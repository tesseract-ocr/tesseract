/**********************************************************************
 * File:        pango_font_info.cpp
 * Description: Font-related objects and helper functions
 * Author:      Ranjith Unnikrishnan
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
#  include "config_auto.h"
#endif

#if (defined __CYGWIN__)
// workaround for stdlib.h and putenv
#  undef __STRICT_ANSI__
#endif

#include "commandlineflags.h"
#include "fileio.h"
#include "normstrngs.h"
#include "pango_font_info.h"
#include "tlog.h"

#include <tesseract/unichar.h>

#include "pango/pango.h"
#include "pango/pangocairo.h"
#include "pango/pangofc-font.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#ifndef _MSC_VER
#  include <sys/param.h>
#endif

#define DISABLE_HEAP_LEAK_CHECK

using namespace tesseract;

namespace tesseract {

// Default assumed output resolution. Required only for providing font metrics
// in pixels.
const int kDefaultResolution = 300;

std::string PangoFontInfo::fonts_dir_;
std::string PangoFontInfo::cache_dir_;

static PangoGlyph get_glyph(PangoFont *font, gunichar wc) {
#if PANGO_VERSION_CHECK(1, 44, 0)
  // pango_font_get_hb_font requires Pango 1.44 or newer.
  hb_font_t *hb_font = pango_font_get_hb_font(font);
  hb_codepoint_t glyph;
  hb_font_get_nominal_glyph(hb_font, wc, &glyph);
#else
  // Use deprecated pango_fc_font_get_glyph for older Pango versions.
  PangoGlyph glyph = pango_fc_font_get_glyph(PANGO_FC_FONT(font), wc);
#endif
  return glyph;
}

PangoFontInfo::PangoFontInfo() : desc_(nullptr), resolution_(kDefaultResolution) {
  Clear();
}

PangoFontInfo::PangoFontInfo(const std::string &desc)
    : desc_(nullptr), resolution_(kDefaultResolution) {
  if (!ParseFontDescriptionName(desc)) {
    tprintf("ERROR: Could not parse %s\n", desc.c_str());
    Clear();
  }
}

void PangoFontInfo::Clear() {
  font_size_ = 0;
  family_name_.clear();
  font_type_ = UNKNOWN;
  if (desc_) {
    pango_font_description_free(desc_);
    desc_ = nullptr;
  }
}

PangoFontInfo::~PangoFontInfo() {
  pango_font_description_free(desc_);
}

std::string PangoFontInfo::DescriptionName() const {
  if (!desc_) {
    return "";
  }
  char *desc_str = pango_font_description_to_string(desc_);
  std::string desc_name(desc_str);
  g_free(desc_str);
  return desc_name;
}

// If not already initialized, initializes FontConfig by setting its
// environment variable and creating a fonts.conf file that points to the
// FLAGS_fonts_dir and the cache to FLAGS_fontconfig_tmpdir.
/* static */
void PangoFontInfo::SoftInitFontConfig() {
  if (fonts_dir_.empty()) {
    HardInitFontConfig(FLAGS_fonts_dir.c_str(), FLAGS_fontconfig_tmpdir.c_str());
  }
}

// Re-initializes font config, whether or not already initialized.
// If already initialized, any existing cache is deleted, just to be sure.
/* static */
void PangoFontInfo::HardInitFontConfig(const char *fonts_dir, const char *cache_dir) {
  if (!cache_dir_.empty()) {
    File::DeleteMatchingFiles(File::JoinPath(cache_dir_.c_str(), "*cache-?").c_str());
  }
  const int MAX_FONTCONF_FILESIZE = 1024;
  char fonts_conf_template[MAX_FONTCONF_FILESIZE];
  cache_dir_ = cache_dir;
  fonts_dir_ = fonts_dir;
  snprintf(fonts_conf_template, MAX_FONTCONF_FILESIZE,
           "<?xml version=\"1.0\"?>\n"
           "<!DOCTYPE fontconfig SYSTEM \"fonts.dtd\">\n"
           "<fontconfig>\n"
           "<dir>%s</dir>\n"
           "<cachedir>%s</cachedir>\n"
           "<config></config>\n"
           "</fontconfig>\n",
           fonts_dir, cache_dir);
  std::string fonts_conf_file = File::JoinPath(cache_dir, "fonts.conf");
  File::WriteStringToFileOrDie(fonts_conf_template, fonts_conf_file);
#ifdef _WIN32
  std::string env("FONTCONFIG_PATH=");
  env.append(cache_dir);
  _putenv(env.c_str());
  _putenv("LANG=en_US.utf8");
#else
  setenv("FONTCONFIG_PATH", cache_dir, true);
  // Fix the locale so that the reported font names are consistent.
  setenv("LANG", "en_US.utf8", true);
#endif // _WIN32

  if (FcInitReinitialize() != FcTrue) {
    tprintf("FcInitiReinitialize failed!!\n");
  }
  FontUtils::ReInit();
  // Clear Pango's font cache too.
  pango_cairo_font_map_set_default(nullptr);
}

static void ListFontFamilies(PangoFontFamily ***families, int *n_families) {
  PangoFontInfo::SoftInitFontConfig();
  PangoFontMap *font_map = pango_cairo_font_map_get_default();
  DISABLE_HEAP_LEAK_CHECK;
  pango_font_map_list_families(font_map, families, n_families);
}

bool PangoFontInfo::ParseFontDescription(const PangoFontDescription *desc) {
  Clear();
  const char *family = pango_font_description_get_family(desc);
  if (!family) {
    char *desc_str = pango_font_description_to_string(desc);
    tprintf("WARNING: Could not parse family name from description: '%s'\n", desc_str);
    g_free(desc_str);
    return false;
  }
  family_name_ = std::string(family);
  desc_ = pango_font_description_copy(desc);

  // Set font size in points
  font_size_ = pango_font_description_get_size(desc);
  if (!pango_font_description_get_size_is_absolute(desc)) {
    font_size_ /= PANGO_SCALE;
  }

  return true;
}

bool PangoFontInfo::ParseFontDescriptionName(const std::string &name) {
  PangoFontDescription *desc = pango_font_description_from_string(name.c_str());
  bool success = ParseFontDescription(desc);
  pango_font_description_free(desc);
  return success;
}

// Returns the PangoFont structure corresponding to the closest available font
// in the font map. Note that if the font is wholly missing, this could
// correspond to a completely different font family and face.
PangoFont *PangoFontInfo::ToPangoFont() const {
  SoftInitFontConfig();
  PangoFontMap *font_map = pango_cairo_font_map_get_default();
  PangoContext *context = pango_context_new();
  pango_cairo_context_set_resolution(context, resolution_);
  pango_context_set_font_map(context, font_map);
  PangoFont *font = nullptr;
  {
    DISABLE_HEAP_LEAK_CHECK;
    font = pango_font_map_load_font(font_map, context, desc_);
  }
  g_object_unref(context);
  return font;
}

bool PangoFontInfo::CoversUTF8Text(const char *utf8_text, int byte_length) const {
  PangoFont *font = ToPangoFont();
  if (font == nullptr) {
    // Font not found.
    return false;
  }
  PangoCoverage *coverage = pango_font_get_coverage(font, nullptr);
  for (UNICHAR::const_iterator it = UNICHAR::begin(utf8_text, byte_length);
       it != UNICHAR::end(utf8_text, byte_length); ++it) {
    if (IsWhitespace(*it) || pango_is_zero_width(*it)) {
      continue;
    }
    if (pango_coverage_get(coverage, *it) != PANGO_COVERAGE_EXACT) {
      char tmp[5];
      int len = it.get_utf8(tmp);
      tmp[len] = '\0';
      tlog(2, "'%s' (U+%x) not covered by font\n", tmp, *it);
#if PANGO_VERSION_CHECK(1, 52, 0)
      g_object_unref(coverage);
#else
      pango_coverage_unref(coverage);
#endif
      g_object_unref(font);
      return false;
    }
  }
#if PANGO_VERSION_CHECK(1, 52, 0)
  g_object_unref(coverage);
#else
  pango_coverage_unref(coverage);
#endif
  g_object_unref(font);
  return true;
}

// This variant of strncpy permits src and dest to overlap. It will copy the
// first byte first.
static char *my_strnmove(char *dest, const char *src, size_t n) {
  char *ret = dest;

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

int PangoFontInfo::DropUncoveredChars(std::string *utf8_text) const {
  int num_dropped_chars = 0;
  PangoFont *font = ToPangoFont();
  if (font == nullptr) {
    // Font not found, drop all characters.
    num_dropped_chars = utf8_text->length();
    utf8_text->clear();
    return num_dropped_chars;
  }
  PangoCoverage *coverage = pango_font_get_coverage(font, nullptr);
  // Maintain two iterators that point into the string. For space efficiency, we
  // will repeatedly copy one covered UTF8 character from one to the other, and
  // at the end resize the string to the right length.
  char *out = const_cast<char *>(utf8_text->c_str());
  const UNICHAR::const_iterator it_begin = UNICHAR::begin(utf8_text->c_str(), utf8_text->length());
  const UNICHAR::const_iterator it_end = UNICHAR::end(utf8_text->c_str(), utf8_text->length());
  for (UNICHAR::const_iterator it = it_begin; it != it_end;) {
    // Skip bad utf-8.
    if (!it.is_legal()) {
      ++it; // One suitable error message will still be issued.
      continue;
    }
    int unicode = *it;
    int utf8_len = it.utf8_len();
    const char *utf8_char = it.utf8_data();
    // Move it forward before the data gets modified.
    ++it;
    if (!IsWhitespace(unicode) && !pango_is_zero_width(unicode) &&
        pango_coverage_get(coverage, unicode) != PANGO_COVERAGE_EXACT) {
      if (TLOG_IS_ON(2)) {
        UNICHAR unichar(unicode);
        char *str = unichar.utf8_str();
        tlog(2, "'%s' (U+%x) not covered by font\n", str, unicode);
        delete[] str;
      }
      ++num_dropped_chars;
      continue;
    }
    my_strnmove(out, utf8_char, utf8_len);
    out += utf8_len;
  }
#if PANGO_VERSION_CHECK(1, 52, 0)
  g_object_unref(coverage);
#else
  pango_coverage_unref(coverage);
#endif
  g_object_unref(font);
  utf8_text->resize(out - utf8_text->c_str());
  return num_dropped_chars;
}

bool PangoFontInfo::GetSpacingProperties(const std::string &utf8_char, int *x_bearing,
                                         int *x_advance) const {
  // Convert to equivalent PangoFont structure
  PangoFont *font = ToPangoFont();
  if (!font) {
    return false;
  }
  // Find the glyph index in the font for the supplied utf8 character.
  int total_advance = 0;
  int min_bearing = 0;
  // Handle multi-unicode strings by reporting the left-most position of the
  // x-bearing, and right-most position of the x-advance if the string were to
  // be rendered.
  const UNICHAR::const_iterator it_begin = UNICHAR::begin(utf8_char.c_str(), utf8_char.length());
  const UNICHAR::const_iterator it_end = UNICHAR::end(utf8_char.c_str(), utf8_char.length());
  for (UNICHAR::const_iterator it = it_begin; it != it_end; ++it) {
    PangoGlyph glyph_index = get_glyph(font, *it);
    if (!glyph_index) {
      // Glyph for given unicode character doesn't exist in font.
      g_object_unref(font);
      return false;
    }
    // Find the ink glyph extents for the glyph
    PangoRectangle ink_rect, logical_rect;
    pango_font_get_glyph_extents(font, glyph_index, &ink_rect, &logical_rect);
    pango_extents_to_pixels(&ink_rect, nullptr);
    pango_extents_to_pixels(&logical_rect, nullptr);

    int bearing = total_advance + PANGO_LBEARING(ink_rect);
    if (it == it_begin || bearing < min_bearing) {
      min_bearing = bearing;
    }
    total_advance += PANGO_RBEARING(logical_rect);
  }
  *x_bearing = min_bearing;
  *x_advance = total_advance;
  g_object_unref(font);
  return true;
}

bool PangoFontInfo::CanRenderString(const char *utf8_word, int len) const {
  std::vector<std::string> graphemes;
  return CanRenderString(utf8_word, len, &graphemes);
}

bool PangoFontInfo::CanRenderString(const char *utf8_word, int len,
                                    std::vector<std::string> *graphemes) const {
  if (graphemes) {
    graphemes->clear();
  }
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
  PangoFontMap *font_map = pango_cairo_font_map_get_default();
  PangoContext *context = pango_context_new();
  pango_context_set_font_map(context, font_map);
  PangoLayout *layout;
  {
    // Pango is not releasing the cached layout.
    DISABLE_HEAP_LEAK_CHECK;
    layout = pango_layout_new(context);
  }
  if (desc_) {
    pango_layout_set_font_description(layout, desc_);
  } else {
    PangoFontDescription *desc = pango_font_description_from_string(DescriptionName().c_str());
    pango_layout_set_font_description(layout, desc);
    pango_font_description_free(desc);
  }
  pango_layout_set_text(layout, utf8_word, len);
  PangoLayoutIter *run_iter = nullptr;
  { // Fontconfig caches some information here that is not freed before exit.
    DISABLE_HEAP_LEAK_CHECK;
    run_iter = pango_layout_get_iter(layout);
  }
  do {
    PangoLayoutRun *run = pango_layout_iter_get_run_readonly(run_iter);
    if (!run) {
      tlog(2, "Found end of line nullptr run marker\n");
      continue;
    }
    PangoGlyph dotted_circle_glyph;
    PangoFont *font = run->item->analysis.font;

    dotted_circle_glyph = get_glyph(font, kDottedCircleGlyph);

    if (TLOG_IS_ON(2)) {
      PangoFontDescription *desc = pango_font_describe(font);
      char *desc_str = pango_font_description_to_string(desc);
      tlog(2, "Desc of font in run: %s\n", desc_str);
      g_free(desc_str);
      pango_font_description_free(desc);
    }

    PangoGlyphItemIter cluster_iter;
    gboolean have_cluster;
    for (have_cluster = pango_glyph_item_iter_init_start(&cluster_iter, run, utf8_word);
         have_cluster && !bad_glyph;
         have_cluster = pango_glyph_item_iter_next_cluster(&cluster_iter)) {
      const int start_byte_index = cluster_iter.start_index;
      const int end_byte_index = cluster_iter.end_index;
      int start_glyph_index = cluster_iter.start_glyph;
      int end_glyph_index = cluster_iter.end_glyph;
      std::string cluster_text =
          std::string(utf8_word + start_byte_index, end_byte_index - start_byte_index);
      if (graphemes) {
        graphemes->push_back(cluster_text);
      }
      if (IsUTF8Whitespace(cluster_text.c_str())) {
        tlog(2, "Skipping whitespace\n");
        continue;
      }
      if (TLOG_IS_ON(2)) {
        printf("start_byte=%d end_byte=%d start_glyph=%d end_glyph=%d ", start_byte_index,
               end_byte_index, start_glyph_index, end_glyph_index);
      }
      for (int i = start_glyph_index, step = (end_glyph_index > start_glyph_index) ? 1 : -1;
           !bad_glyph && i != end_glyph_index; i += step) {
        const bool unknown_glyph =
            (cluster_iter.glyph_item->glyphs->glyphs[i].glyph & PANGO_GLYPH_UNKNOWN_FLAG);
        const bool illegal_glyph =
            (cluster_iter.glyph_item->glyphs->glyphs[i].glyph == dotted_circle_glyph);
        bad_glyph = unknown_glyph || illegal_glyph;
        if (TLOG_IS_ON(2)) {
          printf("(%d=%d)", cluster_iter.glyph_item->glyphs->glyphs[i].glyph, bad_glyph ? 1 : 0);
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
  if (bad_glyph && graphemes) {
    graphemes->clear();
  }
  return !bad_glyph;
}

// ------------------------ FontUtils ------------------------------------
std::vector<std::string> FontUtils::available_fonts_; // cache list

// Returns whether the specified font description is available in the fonts
// directory.
//
// The generated list of font families and faces includes "synthesized" font
// faces that are not truly loadable. Pango versions >=1.18 have a
// pango_font_face_is_synthesized method that can be used to prune the list.
// Until then, we are restricted to using a hack where we try to load the font
// from the font_map, and then check what we loaded to see if it has the
// description we expected. If it is not, then the font is deemed unavailable.
//
// TODO: This function reports also some not synthesized fonts as not available
// e.g. 'Bitstream Charter Medium Italic', 'LMRoman17', so we need this hack
// until  other solution is found.
/* static */
bool FontUtils::IsAvailableFont(const char *input_query_desc, std::string *best_match) {
  std::string query_desc(input_query_desc);
  PangoFontDescription *desc = pango_font_description_from_string(query_desc.c_str());
  PangoFont *selected_font = nullptr;
  {
    PangoFontInfo::SoftInitFontConfig();
    PangoFontMap *font_map = pango_cairo_font_map_get_default();
    PangoContext *context = pango_context_new();
    pango_context_set_font_map(context, font_map);
    {
      DISABLE_HEAP_LEAK_CHECK;
      selected_font = pango_font_map_load_font(font_map, context, desc);
    }
    g_object_unref(context);
  }
  if (selected_font == nullptr) {
    pango_font_description_free(desc);
    tlog(4, "** Font '%s' failed to load from font map!\n", input_query_desc);
    return false;
  }
  PangoFontDescription *selected_desc = pango_font_describe(selected_font);

  bool equal = pango_font_description_equal(desc, selected_desc);
  tlog(3, "query weight = %d \t selected weight =%d\n", pango_font_description_get_weight(desc),
       pango_font_description_get_weight(selected_desc));

  char *selected_desc_str = pango_font_description_to_string(selected_desc);
  tlog(2, "query_desc: '%s' Selected: '%s'\n", query_desc.c_str(), selected_desc_str);
  if (!equal && best_match != nullptr) {
    *best_match = selected_desc_str;
    // Clip the ending ' 0' if there is one. It seems that, if there is no
    // point size on the end of the fontname, then Pango always appends ' 0'.
    auto len = best_match->size();
    if (len > 2 && best_match->at(len - 1) == '0' && best_match->at(len - 2) == ' ') {
      best_match->resize(len - 2);
    }
  }
  g_free(selected_desc_str);
  pango_font_description_free(selected_desc);
  g_object_unref(selected_font);
  pango_font_description_free(desc);
  if (!equal)
    tlog(4, "** Font '%s' failed pango_font_description_equal!\n", input_query_desc);
  return equal;
}

static bool ShouldIgnoreFontFamilyName(const char *query) {
  static const char *kIgnoredFamilyNames[] = {"Sans", "Serif", "Monospace", nullptr};
  const char **list = kIgnoredFamilyNames;
  for (; *list != nullptr; ++list) {
    if (!strcmp(*list, query)) {
      return true;
    }
  }
  return false;
}

// Outputs description names of available fonts.
/* static */
const std::vector<std::string> &FontUtils::ListAvailableFonts() {
  if (!available_fonts_.empty()) {
    return available_fonts_;
  }

  PangoFontFamily **families = nullptr;
  int n_families = 0;
  ListFontFamilies(&families, &n_families);
  for (int i = 0; i < n_families; ++i) {
    const char *family_name = pango_font_family_get_name(families[i]);
    tlog(2, "Listing family %s\n", family_name);
    if (ShouldIgnoreFontFamilyName(family_name)) {
      continue;
    }

    int n_faces;
    PangoFontFace **faces = nullptr;
    pango_font_family_list_faces(families[i], &faces, &n_faces);
    for (int j = 0; j < n_faces; ++j) {
      PangoFontDescription *desc = pango_font_face_describe(faces[j]);
      char *desc_str = pango_font_description_to_string(desc);
      // "synthesized" font faces that are not truly loadable, so we skip it
      if (!pango_font_face_is_synthesized(faces[j]) && IsAvailableFont(desc_str)) {
        available_fonts_.emplace_back(desc_str);
      }
      pango_font_description_free(desc);
      g_free(desc_str);
    }
    g_free(faces);
  }
  g_free(families);
  std::sort(available_fonts_.begin(), available_fonts_.end());
  return available_fonts_;
}

// Utilities written to be backward compatible with StringRender

/* static */
int FontUtils::FontScore(const std::unordered_map<char32, int64_t> &ch_map,
                         const std::string &fontname, int *raw_score, std::vector<bool> *ch_flags) {
  PangoFontInfo font_info;
  if (!font_info.ParseFontDescriptionName(fontname)) {
    tprintf("ERROR: Could not parse %s\n", fontname.c_str());
  }
  PangoFont *font = font_info.ToPangoFont();
  PangoCoverage *coverage = nullptr;
  if (font != nullptr) {
    coverage = pango_font_get_coverage(font, nullptr);
  }
  if (ch_flags) {
    ch_flags->clear();
    ch_flags->reserve(ch_map.size());
  }
  *raw_score = 0;
  int ok_chars = 0;
  for (auto &&it : ch_map) {
    bool covered =
        (coverage != nullptr) && (IsWhitespace(it.first) ||
                                  (pango_coverage_get(coverage, it.first) == PANGO_COVERAGE_EXACT));
    if (covered) {
      ++(*raw_score);
      ok_chars += it.second;
    }
    if (ch_flags) {
      ch_flags->push_back(covered);
    }
  }
#if PANGO_VERSION_CHECK(1, 52, 0)
  g_object_unref(coverage);
#else
  pango_coverage_unref(coverage);
#endif
  g_object_unref(font);
  return ok_chars;
}

/* static */
std::string FontUtils::BestFonts(const std::unordered_map<char32, int64_t> &ch_map,
                                 std::vector<std::pair<const char *, std::vector<bool>>> *fonts) {
  const double kMinOKFraction = 0.99;
  // Weighted fraction of characters that must be renderable in a font to make
  // it OK even if the raw count is not good.
  const double kMinWeightedFraction = 0.99995;

  fonts->clear();
  std::vector<std::vector<bool>> font_flags;
  std::vector<int> font_scores;
  std::vector<int> raw_scores;
  int most_ok_chars = 0;
  int best_raw_score = 0;
  const std::vector<std::string> &font_names = FontUtils::ListAvailableFonts();
  for (const auto &font_name : font_names) {
    std::vector<bool> ch_flags;
    int raw_score = 0;
    int ok_chars = FontScore(ch_map, font_name, &raw_score, &ch_flags);
    most_ok_chars = std::max(ok_chars, most_ok_chars);
    best_raw_score = std::max(raw_score, best_raw_score);

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

  std::string font_list;
  for (unsigned i = 0; i < font_names.size(); ++i) {
    int score = font_scores[i];
    int raw_score = raw_scores[i];
    if ((score >= least_good_enough && raw_score >= least_raw_enough) || score >= override_enough) {
      fonts->push_back(std::make_pair(font_names[i].c_str(), font_flags[i]));
      tlog(1, "OK font %s = %.4f%%, raw = %d = %.2f%%\n", font_names[i].c_str(),
           100.0 * score / most_ok_chars, raw_score, 100.0 * raw_score / best_raw_score);
      font_list += font_names[i];
      font_list += "\n";
    } else if (score >= least_good_enough || raw_score >= least_raw_enough) {
      tlog(1, "Runner-up font %s = %.4f%%, raw = %d = %.2f%%\n", font_names[i].c_str(),
           100.0 * score / most_ok_chars, raw_score, 100.0 * raw_score / best_raw_score);
    }
  }
  return font_list;
}

/* static */
bool FontUtils::SelectFont(const char *utf8_word, const int utf8_len, std::string *font_name,
                           std::vector<std::string> *graphemes) {
  return SelectFont(utf8_word, utf8_len, ListAvailableFonts(), font_name, graphemes);
}

/* static */
bool FontUtils::SelectFont(const char *utf8_word, const int utf8_len,
                           const std::vector<std::string> &all_fonts, std::string *font_name,
                           std::vector<std::string> *graphemes) {
  if (font_name) {
    font_name->clear();
  }
  if (graphemes) {
    graphemes->clear();
  }
  for (const auto &all_font : all_fonts) {
    PangoFontInfo font;
    std::vector<std::string> found_graphemes;
    ASSERT_HOST_MSG(font.ParseFontDescriptionName(all_font), "Could not parse font desc name %s\n",
                    all_font.c_str());
    if (font.CanRenderString(utf8_word, utf8_len, &found_graphemes)) {
      if (graphemes) {
        graphemes->swap(found_graphemes);
      }
      if (font_name) {
        *font_name = all_font;
      }
      return true;
    }
  }
  return false;
}

// PangoFontInfo is reinitialized, so clear the static list of fonts.
/* static */
void FontUtils::ReInit() {
  available_fonts_.clear();
}

// Print info about used font backend
/* static */
void FontUtils::PangoFontTypeInfo() {
  PangoFontMap *font_map = pango_cairo_font_map_get_default();
  if (pango_cairo_font_map_get_font_type(reinterpret_cast<PangoCairoFontMap *>(font_map)) ==
      CAIRO_FONT_TYPE_TOY) {
    printf("Using CAIRO_FONT_TYPE_TOY.\n");
  } else if (pango_cairo_font_map_get_font_type(reinterpret_cast<PangoCairoFontMap *>(font_map)) ==
             CAIRO_FONT_TYPE_FT) {
    printf("Using CAIRO_FONT_TYPE_FT.\n");
  } else if (pango_cairo_font_map_get_font_type(reinterpret_cast<PangoCairoFontMap *>(font_map)) ==
             CAIRO_FONT_TYPE_WIN32) {
    printf("Using CAIRO_FONT_TYPE_WIN32.\n");
  } else if (pango_cairo_font_map_get_font_type(reinterpret_cast<PangoCairoFontMap *>(font_map)) ==
             CAIRO_FONT_TYPE_QUARTZ) {
    printf("Using CAIRO_FONT_TYPE_QUARTZ.\n");
  } else if (pango_cairo_font_map_get_font_type(reinterpret_cast<PangoCairoFontMap *>(font_map)) ==
             CAIRO_FONT_TYPE_USER) {
    printf("Using CAIRO_FONT_TYPE_USER.\n");
  } else if (!font_map) {
    printf("Cannot create pango cairo font map!\n");
  }
}

} // namespace tesseract
