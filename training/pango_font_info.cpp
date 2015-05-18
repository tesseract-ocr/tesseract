/**********************************************************************
 * File:        pango_font_info.cpp
 * Description: Font-related objects and helper functions
 * Author:      Ranjith Unnikrishnan
 * Created:     Mon Nov 18 2013
 *
 * (C) Copyright 2013, Google Inc.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 **********************************************************************/

// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif

#ifdef MINGW
// workaround for stdlib.h and putenv
#undef __STRICT_ANSI__
#include "strcasestr.h"
#endif  // MINGW
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/param.h>
#include <algorithm>

#include "pango_font_info.h"
#include "commandlineflags.h"
#include "fileio.h"
#include "normstrngs.h"
#include "tlog.h"
#include "unichar.h"
#include "util.h"
#include "pango/pango.h"
#include "pango/pangocairo.h"
#include "pango/pangofc-font.h"

STRING_PARAM_FLAG(fonts_dir, "/auto/ocr-data/tesstraining/fonts",
                  "Overrides system default font location");
STRING_PARAM_FLAG(fontconfig_tmpdir, "/tmp",
                  "Overrides fontconfig default temporary dir");
BOOL_PARAM_FLAG(fontconfig_refresh_cache, false,
                "Does a one-time deletion of cache files from the "
                "fontconfig_tmpdir before initializing fontconfig.");
BOOL_PARAM_FLAG(fontconfig_refresh_config_file, true,
                "Does a one-time reset of the fontconfig config file to point"
                " to fonts_dir before initializing fontconfig. Set to true"
                " if fontconfig_refresh_cache is true. Set it to false to use"
                " multiple instances in separate processes without having to"
                " rescan the fonts_dir, using a previously setup font cache");

#ifndef USE_STD_NAMESPACE
#include "ocr/trainingdata/typesetting/legacy_fonts.h"
BOOL_PARAM_FLAG(use_only_legacy_fonts, false,
                "Overrides --fonts_dir and sets the known universe of fonts to"
                "the list in legacy_fonts.h");
#else
using std::pair;
#endif

namespace tesseract {

// Default assumed output resolution. Required only for providing font metrics
// in pixels.
const int kDefaultResolution = 300;

bool PangoFontInfo::fontconfig_initialized_ = false;

PangoFontInfo::PangoFontInfo() : desc_(NULL), resolution_(kDefaultResolution) {
  Clear();
}

PangoFontInfo::PangoFontInfo(const string& desc)
    : desc_(NULL), resolution_(kDefaultResolution) {
  if (!ParseFontDescriptionName(desc)) {
    tprintf("ERROR: Could not parse %s\n", desc.c_str());
    Clear();
  }
}

void PangoFontInfo::Clear() {
  font_size_ = 0;
  is_bold_ = false;
  is_italic_ = false;
  is_smallcaps_ = false;
  is_monospace_ = false;
  family_name_.clear();
  font_type_ = UNKNOWN;
  if (desc_) {
    pango_font_description_free(desc_);
    desc_ = NULL;
  }
}

string PangoFontInfo::DescriptionName() const {
  if (!desc_) return "";
  char* desc_str = pango_font_description_to_string(desc_);
  string desc_name(desc_str);
  g_free(desc_str);
  return desc_name;
}

// Initializes Fontconfig for use by writing a fake fonts.conf file into the
// FLAGS_fontconfigs_tmpdir directory, that points to the supplied
// fonts_dir, and then overrides the FONTCONFIG_PATH environment variable
// to point to this fonts.conf file. If force_clear, the cache is refreshed
// even if it has already been initialized.
void PangoFontInfo::InitFontConfig(bool force_clear, const string& fonts_dir) {
  if ((fontconfig_initialized_ && !force_clear) || fonts_dir.empty()) {
    fontconfig_initialized_ = true;
    return;
  }
  if (FLAGS_fontconfig_refresh_cache || force_clear) {
    File::DeleteMatchingFiles(File::JoinPath(
        FLAGS_fontconfig_tmpdir.c_str(), "*cache-?").c_str());
  }
  if (FLAGS_fontconfig_refresh_config_file || FLAGS_fontconfig_refresh_cache ||
      force_clear) {
    const int MAX_FONTCONF_FILESIZE = 1024;
    char fonts_conf_template[MAX_FONTCONF_FILESIZE];
    snprintf(fonts_conf_template, MAX_FONTCONF_FILESIZE,
             "<?xml version=\"1.0\"?>\n"
             "<!DOCTYPE fontconfig SYSTEM \"fonts.dtd\">\n"
             "<fontconfig>\n"
             "<dir>%s</dir>\n"
             "<cachedir>%s</cachedir>\n"
             "<config></config>\n"
             "</fontconfig>", fonts_dir.c_str(),
             FLAGS_fontconfig_tmpdir.c_str());
    string fonts_conf_file = File::JoinPath(FLAGS_fontconfig_tmpdir.c_str(),
                                            "fonts.conf");
    File::WriteStringToFileOrDie(fonts_conf_template, fonts_conf_file);
  }
#ifdef _WIN32
  std::string env("FONTCONFIG_PATH=");
  env.append(FLAGS_fontconfig_tmpdir.c_str());
  putenv(env.c_str());
  putenv("LANG=en_US.utf8");
#else
  setenv("FONTCONFIG_PATH", FLAGS_fontconfig_tmpdir.c_str(), true);
  // Fix the locale so that the reported font names are consistent.
  setenv("LANG", "en_US.utf8", true);
#endif  // _WIN32
  if (!fontconfig_initialized_ || force_clear) {
    if (FcInitReinitialize() != FcTrue) {
      tprintf("FcInitiReinitialize failed!!\n");
    }
  }
  fontconfig_initialized_ = true;
  FontUtils::ReInit();
}

static void ListFontFamilies(PangoFontFamily*** families,
                             int* n_families) {
  PangoFontInfo::InitFontConfig(false, FLAGS_fonts_dir.c_str());
  PangoFontMap* font_map = pango_cairo_font_map_get_default();
  DISABLE_HEAP_LEAK_CHECK;
  pango_font_map_list_families(font_map, families, n_families);
}

// Inspects whether a given font family is monospace. If the font is not
// available, it cannot make a decision and returns false by default.
static bool IsMonospaceFontFamily(const char* family_name) {
  PangoFontFamily** families = 0;
  int n_families = 0;
  bool is_monospace = false;
  ListFontFamilies(&families, &n_families);
  ASSERT_HOST(n_families > 0);
  bool found = false;
  for (int i = 0; i < n_families; ++i) {
    if (!strcasecmp(family_name, pango_font_family_get_name(families[i]))) {
      is_monospace = pango_font_family_is_monospace(families[i]);
      found = true;
      break;
    }
  }
  if (!found) {
    tlog(1, "Could not find monospace property of family %s\n", family_name);
  }
  g_free(families);
  return is_monospace;
}

bool PangoFontInfo::ParseFontDescription(const PangoFontDescription *desc) {
  Clear();
  const char* family = pango_font_description_get_family(desc);
  if (!family) {
    char* desc_str = pango_font_description_to_string(desc);
    tprintf("WARNING: Could not parse family name from description: '%s'\n",
            desc_str);
    g_free(desc_str);
    return false;
  }
  family_name_ = string(family);
  desc_ = pango_font_description_copy(desc);
  is_monospace_ = IsMonospaceFontFamily(family);

  // Set font size in points
  font_size_ = pango_font_description_get_size(desc);
  if (!pango_font_description_get_size_is_absolute(desc)) {
    font_size_ /= PANGO_SCALE;
  }

  PangoStyle style = pango_font_description_get_style(desc);
  is_italic_ = (PANGO_STYLE_ITALIC == style ||
                PANGO_STYLE_OBLIQUE == style);
  is_smallcaps_ = (pango_font_description_get_variant(desc)
                   == PANGO_VARIANT_SMALL_CAPS);

  is_bold_ = (pango_font_description_get_weight(desc) >= PANGO_WEIGHT_BOLD);
  // We dont have a way to detect whether a font is of type Fraktur. The fonts
  // we currently use all have "Fraktur" in their family name, so we do a
  // fragile but functional check for that here.
  is_fraktur_ = (strcasestr(family, "Fraktur") != NULL);
  return true;
}

bool PangoFontInfo::ParseFontDescriptionName(const string& name) {
  PangoFontDescription *desc = pango_font_description_from_string(name.c_str());
  bool success = ParseFontDescription(desc);
  pango_font_description_free(desc);
  return success;
}

// Returns the PangoFont structure corresponding to the closest available font
// in the font map. Note that if the font is wholly missing, this could
// correspond to a completely different font family and face.
PangoFont* PangoFontInfo::ToPangoFont() const {
  InitFontConfig(false, FLAGS_fonts_dir.c_str());
  PangoFontMap* font_map = pango_cairo_font_map_get_default();
  PangoContext* context = pango_context_new();
  pango_cairo_context_set_resolution(context, resolution_);
  pango_context_set_font_map(context, font_map);
  PangoFont* font = NULL;
  {
    DISABLE_HEAP_LEAK_CHECK;
    font = pango_font_map_load_font(font_map, context, desc_);
  }
  g_object_unref(context);
  return font;
}

bool PangoFontInfo::CoversUTF8Text(const char* utf8_text, int byte_length) const {
  PangoFont* font = ToPangoFont();
  PangoCoverage* coverage = pango_font_get_coverage(font, NULL);
  for (UNICHAR::const_iterator it = UNICHAR::begin(utf8_text, byte_length);
       it != UNICHAR::end(utf8_text, byte_length);
       ++it) {
    if (IsWhitespace(*it) || pango_is_zero_width(*it))
      continue;
    if (pango_coverage_get(coverage, *it) != PANGO_COVERAGE_EXACT) {
      char tmp[5];
      int len = it.get_utf8(tmp);
      tmp[len] = '\0';
      tlog(2, "'%s' (U+%x) not covered by font\n", tmp, *it);
      return false;
    }
  }
  return true;
}

// This variant of strncpy permits src and dest to overlap. It will copy the
// first byte first.
static char* my_strnmove(char* dest, const char* src, size_t n) {
  char* ret = dest;

  // Copy characters until n reaches zero or the src byte is a nul.
  do {
    *dest = *src;
    --n;
    ++dest;
    ++src;
  } while (n && src[0]);

  // If we reached a nul byte and there are more 'n' left, zero them out.
  while (n) {
    *dest = '\0';
    --n;
    ++dest;
  }
  return ret;
}

int PangoFontInfo::DropUncoveredChars(string* utf8_text) const {
  PangoFont* font = ToPangoFont();
  PangoCoverage* coverage = pango_font_get_coverage(font, NULL);
  int num_dropped_chars = 0;
  // Maintain two iterators that point into the string. For space efficiency, we
  // will repeatedly copy one covered UTF8 character from one to the other, and
  // at the end resize the string to the right length.
  char* out = const_cast<char*>(utf8_text->c_str());
  const UNICHAR::const_iterator it_begin =
      UNICHAR::begin(utf8_text->c_str(), utf8_text->length());
  const UNICHAR::const_iterator it_end =
      UNICHAR::end(utf8_text->c_str(), utf8_text->length());
  for (UNICHAR::const_iterator it = it_begin; it != it_end;) {
    // Skip bad utf-8.
    if (!it.is_legal()) {
      ++it;  // One suitable error message will still be issued.
      continue;
    }
    int unicode = *it;
    int utf8_len = it.utf8_len();
    const char* utf8_char = it.utf8_data();
    // Move it forward before the data gets modified.
    ++it;
    if (!IsWhitespace(unicode) && !pango_is_zero_width(unicode) &&
        pango_coverage_get(coverage, unicode) != PANGO_COVERAGE_EXACT) {
      if (TLOG_IS_ON(2)) {
        UNICHAR unichar(unicode);
        char* str = unichar.utf8_str();
        tlog(2, "'%s' (U+%x) not covered by font\n", str, unicode);
        delete[] str;
      }
      ++num_dropped_chars;
      continue;
    }
    my_strnmove(out, utf8_char, utf8_len);
    out += utf8_len;
  }
  utf8_text->resize(out - utf8_text->c_str());
  return num_dropped_chars;
}

bool PangoFontInfo::GetSpacingProperties(const string& utf8_char,
                                         int* x_bearing, int* x_advance) const {
  // Convert to equivalent PangoFont structure
  PangoFont* font = ToPangoFont();
  // Find the glyph index in the font for the supplied utf8 character.
  int total_advance = 0;
  int min_bearing = 0;
  // Handle multi-unicode strings by reporting the left-most position of the
  // x-bearing, and right-most position of the x-advance if the string were to
  // be rendered.
  const UNICHAR::const_iterator it_begin = UNICHAR::begin(utf8_char.c_str(),
                                                          utf8_char.length());
  const UNICHAR::const_iterator it_end = UNICHAR::end(utf8_char.c_str(),
                                                      utf8_char.length());
  for (UNICHAR::const_iterator it = it_begin; it != it_end; ++it) {
    PangoGlyph glyph_index = pango_fc_font_get_glyph(
        reinterpret_cast<PangoFcFont*>(font), *it);
    if (!glyph_index) {
      // Glyph for given unicode character doesn't exist in font.
      return false;
    }
    // Find the ink glyph extents for the glyph
    PangoRectangle ink_rect, logical_rect;
    pango_font_get_glyph_extents(font, glyph_index, &ink_rect, &logical_rect);
    pango_extents_to_pixels(&ink_rect, NULL);
    pango_extents_to_pixels(&logical_rect, NULL);

    int bearing = total_advance + PANGO_LBEARING(ink_rect);
    if (it == it_begin || bearing < min_bearing) {
      min_bearing = bearing;
    }
    total_advance += PANGO_RBEARING(logical_rect);
  }
  *x_bearing = min_bearing;
  *x_advance = total_advance;
  return true;
}

bool PangoFontInfo::CanRenderString(const char* utf8_word, int len) const {
  vector<string> graphemes;
  return CanRenderString(utf8_word, len, &graphemes);
}

bool PangoFontInfo::CanRenderString(const char* utf8_word, int len,
                                    vector<string>* graphemes) const {
  if (graphemes) graphemes->clear();
  // We check for font coverage of the text first, as otherwise Pango could
  // (undesirably) fall back to another font that does have the required
  // coverage.
  if (!CoversUTF8Text(utf8_word, len)) {
    return false;
  }
  // U+25CC dotted circle character that often (but not always) gets rendered
  // when there is an illegal grapheme sequence.
  const char32 kDottedCircleGlyph = 9676;
  bool bad_glyph = false;
  PangoFontMap* font_map = pango_cairo_font_map_get_default();
  PangoContext* context = pango_context_new();
  pango_context_set_font_map(context, font_map);
  PangoLayout* layout;
  {
    // Pango is not relasing the cached layout.
    DISABLE_HEAP_LEAK_CHECK;
    layout = pango_layout_new(context);
  }
  if (desc_) {
    pango_layout_set_font_description(layout, desc_);
  } else {
    PangoFontDescription *desc = pango_font_description_from_string(
        DescriptionName().c_str());
    pango_layout_set_font_description(layout, desc);
    pango_font_description_free(desc);
  }
  pango_layout_set_text(layout, utf8_word, len);
  PangoLayoutIter* run_iter = NULL;
  { // Fontconfig caches some information here that is not freed before exit.
    DISABLE_HEAP_LEAK_CHECK;
    run_iter = pango_layout_get_iter(layout);
  }
  do {
    PangoLayoutRun* run = pango_layout_iter_get_run_readonly(run_iter);
    if (!run) {
      tlog(2, "Found end of line NULL run marker\n");
      continue;
    }
    PangoGlyph dotted_circle_glyph;
    PangoFont* font = run->item->analysis.font;
    dotted_circle_glyph = pango_fc_font_get_glyph(
        reinterpret_cast<PangoFcFont*>(font), kDottedCircleGlyph);
    if (TLOG_IS_ON(2)) {
      PangoFontDescription* desc = pango_font_describe(font);
      char* desc_str = pango_font_description_to_string(desc);
      tlog(2, "Desc of font in run: %s\n", desc_str);
      g_free(desc_str);
      pango_font_description_free(desc);
    }

    PangoGlyphItemIter cluster_iter;
    gboolean have_cluster;
    for (have_cluster = pango_glyph_item_iter_init_start(&cluster_iter,
                                                         run, utf8_word);
         have_cluster && !bad_glyph;
         have_cluster = pango_glyph_item_iter_next_cluster(&cluster_iter)) {
      const int start_byte_index = cluster_iter.start_index;
      const int end_byte_index = cluster_iter.end_index;
      int start_glyph_index = cluster_iter.start_glyph;
      int end_glyph_index = cluster_iter.end_glyph;
      string cluster_text = string(utf8_word + start_byte_index,
                                   end_byte_index - start_byte_index);
      if (graphemes) graphemes->push_back(cluster_text);
      if (IsUTF8Whitespace(cluster_text.c_str())) {
        tlog(2, "Skipping whitespace\n");
        continue;
      }
      if (TLOG_IS_ON(2)) {
        printf("start_byte=%d end_byte=%d start_glyph=%d end_glyph=%d ",
               start_byte_index, end_byte_index,
               start_glyph_index, end_glyph_index);
      }
      for (int i = start_glyph_index,
               step = (end_glyph_index > start_glyph_index) ? 1 : -1;
           !bad_glyph && i != end_glyph_index; i+= step) {
        const bool unknown_glyph =
            (cluster_iter.glyph_item->glyphs->glyphs[i].glyph &
             PANGO_GLYPH_UNKNOWN_FLAG);
        const bool illegal_glyph =
            (cluster_iter.glyph_item->glyphs->glyphs[i].glyph ==
             dotted_circle_glyph);
        bad_glyph = unknown_glyph || illegal_glyph;
        if (TLOG_IS_ON(2)) {
          printf("(%d=%d)", cluster_iter.glyph_item->glyphs->glyphs[i].glyph,
                 bad_glyph ? 1 : 0);
        }
      }
      if (TLOG_IS_ON(2)) {
        printf("  '%s'\n", cluster_text.c_str());
      }
      if (bad_glyph)
        tlog(1, "Found illegal glyph!\n");
    }
  } while (!bad_glyph && pango_layout_iter_next_run(run_iter));

  pango_layout_iter_free(run_iter);
  g_object_unref(context);
  g_object_unref(layout);
  if (bad_glyph && graphemes) graphemes->clear();
  return !bad_glyph;
}


// ------------------------ FontUtils ------------------------------------
vector<string> FontUtils::available_fonts_;  // cache list

// Returns whether the specified font description is available in the fonts
// directory.
//
// The generated list of font families and faces includes "synthesized" font
// faces that are not truly loadable. Pango versions >=1.18 have a
// pango_font_face_is_synthesized method that can be used to prune the list.
// Until then, we are restricted to using a hack where we try to load the font
// from the font_map, and then check what we loaded to see if it has the
// description we expected. If it is not, then the font is deemed unavailable.
/* static */
bool FontUtils::IsAvailableFont(const char* input_query_desc,
                                string* best_match) {
  string query_desc(input_query_desc);
  if (PANGO_VERSION <= 12005) {
    // Strip commas and any ' Medium' substring in the name.
    query_desc.erase(std::remove(query_desc.begin(), query_desc.end(), ','),
                     query_desc.end());
    const string kMediumStr = " Medium";
    std::size_t found = query_desc.find(kMediumStr);
    if (found != std::string::npos) {
      query_desc.erase(found, kMediumStr.length());
    }
  }

  PangoFontDescription *desc = pango_font_description_from_string(
      query_desc.c_str());
  PangoFont* selected_font = NULL;
  {
    PangoFontInfo::InitFontConfig(false, FLAGS_fonts_dir.c_str());
    PangoFontMap* font_map = pango_cairo_font_map_get_default();
    PangoContext* context = pango_context_new();
    pango_context_set_font_map(context, font_map);
    {
      DISABLE_HEAP_LEAK_CHECK;
      selected_font = pango_font_map_load_font(font_map, context, desc);
    }
    g_object_unref(context);
  }
  if (selected_font == NULL) {
    pango_font_description_free(desc);
    return false;
  }
  PangoFontDescription* selected_desc = pango_font_describe(selected_font);

  bool equal = pango_font_description_equal(desc, selected_desc);
  tlog(3, "query weight = %d \t selected weight =%d\n",
       pango_font_description_get_weight(desc),
       pango_font_description_get_weight(selected_desc));

  char* selected_desc_str = pango_font_description_to_string(selected_desc);
  tlog(2, "query_desc: '%s' Selected: 's'\n", query_desc.c_str(),
       selected_desc_str);
  if (!equal && best_match != NULL) {
    *best_match = selected_desc_str;
    // Clip the ending ' 0' if there is one. It seems that, if there is no
    // point size on the end of the fontname, then Pango always appends ' 0'.
    int len = best_match->size();
    if (len > 2 && best_match->at(len - 1) == '0' &&
        best_match->at(len - 2) == ' ') {
      *best_match = best_match->substr(0, len - 2);
    }
  }
  g_free(selected_desc_str);
  pango_font_description_free(selected_desc);
  g_object_unref(selected_font);
  pango_font_description_free(desc);
  return equal;
}

static bool ShouldIgnoreFontFamilyName(const char* query) {
  static const char* kIgnoredFamilyNames[]
      = { "Sans", "Serif", "Monospace", NULL };
  const char** list = kIgnoredFamilyNames;
  for (; *list != NULL; ++list) {
    if (!strcmp(*list, query))
      return true;
  }
  return false;
}

// Outputs description names of available fonts.
/* static */
const vector<string>& FontUtils::ListAvailableFonts() {
  if (available_fonts_.size()) {
    return available_fonts_;
  }
#ifndef USE_STD_NAMESPACE
  if (FLAGS_use_only_legacy_fonts) {
    // Restrict view to list of fonts in legacy_fonts.h
    tprintf("Using list of legacy fonts only\n");
    const int kNumFontLists = 4;
    for (int i = 0; i < kNumFontLists; ++i) {
      for (int j = 0; kFontlists[i][j] != NULL; ++j) {
        available_fonts_.push_back(kFontlists[i][j]);
      }
    }
    return available_fonts_;
  }
#endif

  PangoFontFamily** families = 0;
  int n_families = 0;
  ListFontFamilies(&families, &n_families);
  for (int i = 0; i < n_families; ++i) {
    const char* family_name = pango_font_family_get_name(families[i]);
    tlog(2, "Listing family %s\n", family_name);
    if (ShouldIgnoreFontFamilyName(family_name)) {
      continue;
    }

    int n_faces;
    PangoFontFace** faces = NULL;
    pango_font_family_list_faces(families[i], &faces, &n_faces);
    for (int j = 0; j < n_faces; ++j) {
      PangoFontDescription* desc = pango_font_face_describe(faces[j]);
      char* desc_str = pango_font_description_to_string(desc);
      if (IsAvailableFont(desc_str)) {
        available_fonts_.push_back(desc_str);
      }
      pango_font_description_free(desc);
      g_free(desc_str);
    }
    g_free(faces);
  }
  g_free(families);
  sort(available_fonts_.begin(), available_fonts_.end());
  return available_fonts_;
}


static void CharCoverageMapToBitmap(PangoCoverage* coverage,
                                    vector<bool>* unichar_bitmap) {
  const int kMinUnicodeValue = 33;
  const int kMaxUnicodeValue = 0x10FFFF;
  unichar_bitmap->resize(kMaxUnicodeValue + 1, false);
  // Mark off characters that the font can render.
  for (int i = kMinUnicodeValue; i <= kMaxUnicodeValue; ++i) {
    if (IsInterchangeValid(i)) {
      (*unichar_bitmap)[i]
          = (pango_coverage_get(coverage, i) == PANGO_COVERAGE_EXACT);
    }
  }
}

/* static */
void FontUtils::GetAllRenderableCharacters(vector<bool>* unichar_bitmap) {
  const vector<string>& all_fonts = ListAvailableFonts();
  return GetAllRenderableCharacters(all_fonts, unichar_bitmap);
}

/* static */
void FontUtils::GetAllRenderableCharacters(const string& font_name,
                                           vector<bool>* unichar_bitmap) {
  PangoFontInfo font_info(font_name);
  PangoCoverage* coverage = pango_font_get_coverage(
      font_info.ToPangoFont(), NULL);
  CharCoverageMapToBitmap(coverage, unichar_bitmap);
}

/* static */
void FontUtils::GetAllRenderableCharacters(const vector<string>& fonts,
                                           vector<bool>* unichar_bitmap) {
  // Form the union of coverage maps from the fonts
  PangoCoverage* all_coverage = pango_coverage_new();
  tlog(1, "Processing %d fonts\n", fonts.size());
  for (int i = 0; i < fonts.size(); ++i) {
    PangoFontInfo font_info(fonts[i]);
    PangoCoverage* coverage = pango_font_get_coverage(
        font_info.ToPangoFont(), NULL);
    // Mark off characters that any font can render.
    pango_coverage_max(all_coverage, coverage);
  }
  CharCoverageMapToBitmap(all_coverage, unichar_bitmap);
  pango_coverage_unref(all_coverage);
}


// Utilities written to be backward compatible with StringRender

/* static */
int FontUtils::FontScore(const unordered_map<char32, inT64>& ch_map,
                         const string& fontname,
                         int* raw_score,
                         vector<bool>* ch_flags) {
  PangoFontInfo font_info;
  if (!font_info.ParseFontDescriptionName(fontname)) {
    tprintf("ERROR: Could not parse %s\n", fontname.c_str());
  }
  PangoFont* font = font_info.ToPangoFont();
  PangoCoverage* coverage = pango_font_get_coverage(font, NULL);

  if (ch_flags) {
    ch_flags->clear();
    ch_flags->reserve(ch_map.size());
  }
  *raw_score = 0;
  int ok_chars = 0;
  for (unordered_map<char32, inT64>::const_iterator it = ch_map.begin();
       it != ch_map.end(); ++it) {
    bool covered = (IsWhitespace(it->first) ||
                    (pango_coverage_get(coverage, it->first)
                     == PANGO_COVERAGE_EXACT));
    if (covered) {
      ++(*raw_score);
      ok_chars += it->second;
    }
    if (ch_flags) {
      ch_flags->push_back(covered);
    }
  }
  return ok_chars;
}


/* static */
string FontUtils::BestFonts(const unordered_map<char32, inT64>& ch_map,
                            vector<pair<const char*, vector<bool> > >* fonts) {
  const double kMinOKFraction = 0.99;
  // Weighted fraction of characters that must be renderable in a font to make
  // it OK even if the raw count is not good.
  const double kMinWeightedFraction = 0.99995;

  fonts->clear();
  vector<vector<bool> > font_flags;
  vector<int> font_scores;
  vector<int> raw_scores;
  int most_ok_chars = 0;
  int best_raw_score = 0;
  const vector<string>& font_names = FontUtils::ListAvailableFonts();
  for (int i = 0; i < font_names.size(); ++i) {
    vector<bool> ch_flags;
    int raw_score = 0;
    int ok_chars = FontScore(ch_map, font_names[i], &raw_score, &ch_flags);
    most_ok_chars = MAX(ok_chars, most_ok_chars);
    best_raw_score = MAX(raw_score, best_raw_score);

    font_flags.push_back(ch_flags);
    font_scores.push_back(ok_chars);
    raw_scores.push_back(raw_score);
  }

  // Now select the fonts with a score above a threshold fraction
  // of both the raw and weighted best scores. To prevent bogus fonts being
  // selected for CJK, we require a high fraction (kMinOKFraction = 0.99) of
  // BOTH weighted and raw scores.
  // In low character-count scripts, the issue is more getting enough fonts,
  // when only 1 or 2 might have all those rare dingbats etc in them, so we
  // allow a font with a very high weighted (coverage) score
  // (kMinWeightedFraction = 0.99995) to be used even if its raw score is poor.
  int least_good_enough = static_cast<int>(most_ok_chars * kMinOKFraction);
  int least_raw_enough = static_cast<int>(best_raw_score * kMinOKFraction);
  int override_enough = static_cast<int>(most_ok_chars * kMinWeightedFraction);

  string font_list;
  for (int i = 0; i < font_names.size(); ++i) {
    int score = font_scores[i];
    int raw_score = raw_scores[i];
    if ((score >= least_good_enough && raw_score >= least_raw_enough) ||
        score >= override_enough) {
      fonts->push_back(make_pair(font_names[i].c_str(), font_flags[i]));
      tlog(1, "OK font %s = %.4f%%, raw = %d = %.2f%%\n",
           font_names[i].c_str(),
           100.0 * score / most_ok_chars,
           raw_score, 100.0 * raw_score / best_raw_score);
      font_list += font_names[i];
      font_list += "\n";
    } else if (score >= least_good_enough || raw_score >= least_raw_enough) {
      tlog(1, "Runner-up font %s = %.4f%%, raw = %d = %.2f%%\n",
           font_names[i].c_str(),
           100.0 * score / most_ok_chars,
           raw_score, 100.0 * raw_score / best_raw_score);
    }
  }
  return font_list;
}

/* static */
bool FontUtils::SelectFont(const char* utf8_word, const int utf8_len,
                           string* font_name, vector<string>* graphemes) {
  return SelectFont(utf8_word, utf8_len, ListAvailableFonts(), font_name,
                    graphemes);
}

/* static */
bool FontUtils::SelectFont(const char* utf8_word, const int utf8_len,
                           const vector<string>& all_fonts,
                           string* font_name, vector<string>* graphemes) {
  if (font_name) font_name->clear();
  if (graphemes) graphemes->clear();
  for (int i = 0; i < all_fonts.size(); ++i) {
    PangoFontInfo font;
    vector<string> found_graphemes;
    ASSERT_HOST_MSG(font.ParseFontDescriptionName(all_fonts[i]),
                    "Could not parse font desc name %s\n",
                    all_fonts[i].c_str());
    if (font.CanRenderString(utf8_word, utf8_len, &found_graphemes)) {
      if (graphemes) graphemes->swap(found_graphemes);
      if (font_name) *font_name = all_fonts[i];
      return true;
    }
  }
  return false;
}

// PangoFontInfo is reinitialized, so clear the static list of fonts.
/* static */
void FontUtils::ReInit() { available_fonts_.clear(); }

}  // namespace tesseract
