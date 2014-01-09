/**********************************************************************
 * File:        stringrenderer.cpp
 * Description: Class for rendering UTF-8 text to an image, and retrieving
 *              bounding boxes around each grapheme cluster.
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

#include "stringrenderer.h"

#include <stdio.h>
#include <string.h>
#include <algorithm>
#include <map>
#include <utility>
#include <vector>

#include "allheaders.h"     // from leptonica
#include "boxchar.h"
#include "ligature_table.h"
#include "normstrngs.h"
#include "pango/pango-font.h"
#include "pango/pango-glyph-item.h"
#include "tlog.h"
#include "unichar.h"
#include "unicode/uchar.h"  // from libicu
#include "util.h"

#ifndef USE_STD_NAMESPACE
// Compatability with pango 1.20.
#include "pango/pango-glyph-item-private.h"
#define pango_glyph_item_iter_init_start _pango_glyph_item_iter_init_start
#define pango_glyph_item_iter_next_cluster _pango_glyph_item_iter_next_cluster
#else
using std::map;
using std::max;
using std::min;
using std::swap;
#endif

namespace tesseract {

static const int kDefaultOutputResolution = 300;

// Word joiner (U+2060) inserted after letters in ngram mode, as per
// recommendation in http://unicode.org/reports/tr14/ to avoid line-breaks at
// hyphens and other non-alpha characters.
static const char* kWordJoinerUTF8 = "\u2060";
static const char32 kWordJoiner = 0x2060;

static bool IsCombiner(int ch) {
  const int char_type = u_charType(ch);
  return ((char_type == U_NON_SPACING_MARK) ||
          (char_type == U_ENCLOSING_MARK) ||
          (char_type == U_COMBINING_SPACING_MARK));
}

static string EncodeAsUTF8(const char32 ch32) {
  UNICHAR uni_ch(ch32);
  return string(uni_ch.utf8(), uni_ch.utf8_len());
}


/* static */
Pix* CairoARGB32ToPixFormat(cairo_surface_t *surface) {
  if (cairo_image_surface_get_format(surface) != CAIRO_FORMAT_ARGB32) {
    printf("Unexpected surface format %d\n",
           cairo_image_surface_get_format(surface));
    return NULL;
  }
  const int width = cairo_image_surface_get_width(surface);
  const int height = cairo_image_surface_get_height(surface);
  Pix* pix = pixCreate(width, height, 32);
  int byte_stride = cairo_image_surface_get_stride(surface);

  for (int i = 0; i < height; ++i) {
    memcpy(reinterpret_cast<unsigned char*>(pix->data + i * pix->wpl) + 1,
           cairo_image_surface_get_data(surface) + i * byte_stride,
           byte_stride - ((i == height - 1) ? 1 : 0));
  }
  return pix;
}

StringRenderer::StringRenderer(const string& font_desc, int page_width,
                               int page_height) :
    page_width_(page_width), page_height_(page_height),
    h_margin_(50), v_margin_(50), char_spacing_(0), leading_(0),
    vertical_text_(false), gravity_hint_strong_(false),
    render_fullwidth_latin_(false) ,drop_uncovered_chars_(true),
    strip_unrenderable_words_(false), add_ligatures_(false),
    output_word_boxes_(false), surface_(NULL), cr_(NULL),
    layout_(NULL), start_box_(0), page_(0), box_padding_(0),
    total_chars_(0), font_index_(0), last_offset_(0) {
  pen_color_[0] = 0.0;
  pen_color_[1] = 0.0;
  pen_color_[2] = 0.0;
  set_font(font_desc);
  set_resolution(kDefaultOutputResolution);
  page_boxes_ = NULL;
}

bool StringRenderer::set_font(const string& desc) {
  bool success = font_.ParseFontDescriptionName(desc);
  font_.set_resolution(resolution_);
  return success;
}

void StringRenderer::set_resolution(const int resolution) {
  resolution_ = resolution;
  font_.set_resolution(resolution);
}

StringRenderer::~StringRenderer() {
  ClearBoxes();
  FreePangoCairo();
}

void StringRenderer::InitPangoCairo() {
  FreePangoCairo();
  surface_ = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, page_width_,
                                        page_height_);
  cr_ = cairo_create(surface_);
  layout_ = pango_cairo_create_layout(cr_);

  if (vertical_text_) {
    PangoContext* context = pango_layout_get_context(layout_);
    pango_context_set_base_gravity(context, PANGO_GRAVITY_EAST);
    if (gravity_hint_strong_) {
      pango_context_set_gravity_hint(context, PANGO_GRAVITY_HINT_STRONG);
    }
    pango_layout_context_changed(layout_);
  }

  SetLayoutProperties();
}

void StringRenderer::SetLayoutProperties() {
  string font_desc = font_.DescriptionName();
  // Specify the font via a description name
  PangoFontDescription *desc =
      pango_font_description_from_string(font_desc.c_str());
  // Assign the font description to the layout
  pango_layout_set_font_description(layout_, desc);
  pango_font_description_free(desc);  // free the description
  pango_cairo_context_set_resolution(pango_layout_get_context(layout_),
                                     resolution_);

  int max_width = page_width_ - 2 * h_margin_;
  int max_height = page_height_ - 2 * v_margin_;
  tlog(3, "max_width = %d, max_height = %d\n", max_width, max_height);
  if (vertical_text_) {
    swap(max_width, max_height);
  }
  pango_layout_set_width(layout_, max_width * PANGO_SCALE);
  pango_layout_set_wrap(layout_, PANGO_WRAP_WORD);

  // Adjust character spacing
  PangoAttrList* attr_list = pango_attr_list_new();
  if (char_spacing_) {
    PangoAttribute* spacing_attr = pango_attr_letter_spacing_new(
        static_cast<int>(char_spacing_ * PANGO_SCALE + 0.5));
    spacing_attr->start_index = 0;
    spacing_attr->end_index = static_cast<guint>(-1);
    pango_attr_list_change(attr_list, spacing_attr);
  }
  pango_layout_set_attributes(layout_, attr_list);
  pango_attr_list_unref(attr_list);
  // Adjust line spacing
  if (leading_) {
    pango_layout_set_spacing(layout_, leading_ * PANGO_SCALE);
  }
}

void StringRenderer::FreePangoCairo() {
  if (layout_) {
    g_object_unref(layout_);
    layout_ = NULL;
  }
  if (cr_) {
    cairo_destroy(cr_);
    cr_ = NULL;
  }
  if (surface_) {
    cairo_surface_destroy(surface_);
    surface_ = NULL;
  }
}


// Returns offset in utf8 bytes to first page.
int StringRenderer::FindFirstPageBreakOffset(const char* text,
                                             int text_length) {
  if (!text_length) return 0;
  const int max_height = (page_height_ - 2 * v_margin_);
  const int max_width = (page_width_ - 2 * h_margin_);
  const int max_layout_height = vertical_text_ ? max_width : max_height;

  UNICHAR::const_iterator it = UNICHAR::begin(text, text_length);
  const UNICHAR::const_iterator it_end = UNICHAR::end(text, text_length);
  const int kMaxUnicodeBufLength = 15000;
  for (int i = 0; i < kMaxUnicodeBufLength && it != it_end; ++it, ++i);
  int buf_length = it.utf8_data() - text;
  tlog(1, "len = %d  buf_len = %d\n", text_length, buf_length);
  pango_layout_set_text(layout_, text, buf_length);

  PangoLayoutIter* line_iter = NULL;
  { // Fontconfig caches some info here that is not freed before exit.
    DISABLE_HEAP_LEAK_CHECK;
    line_iter = pango_layout_get_iter(layout_);
  }
  bool first_page = true;
  int page_top = 0;
  int offset = buf_length;
  do {
    // Get bounding box of the current line
    PangoRectangle line_ink_rect;
    pango_layout_iter_get_line_extents(line_iter, &line_ink_rect, NULL);
    pango_extents_to_pixels(&line_ink_rect, NULL);
    PangoLayoutLine* line = pango_layout_iter_get_line_readonly(line_iter);
    if (first_page) {
      page_top = line_ink_rect.y;
      first_page = false;
    }
    int line_bottom = line_ink_rect.y + line_ink_rect.height;
    if (line_bottom - page_top > max_layout_height) {
      offset = line->start_index;
      tlog(1, "Found offset = %d\n", offset);
      break;
    }
  } while (pango_layout_iter_next_line(line_iter));
  pango_layout_iter_free(line_iter);
  return offset;
}

const vector<BoxChar*>& StringRenderer::GetBoxes() const {
    return boxchars_;
}

Boxa* StringRenderer::GetPageBoxes() const {
    return page_boxes_;
}

void StringRenderer::RotatePageBoxes(float rotation) {
  BoxChar::RotateBoxes(rotation, page_width_ / 2, page_height_ / 2,
                       start_box_, boxchars_.size(), &boxchars_);
}


void StringRenderer::ClearBoxes() {
  for (int i = 0; i < boxchars_.size(); ++i)
    delete boxchars_[i];
  boxchars_.clear();
  boxaDestroy(&page_boxes_);
}

void StringRenderer::WriteAllBoxes(const string& filename) const {
  BoxChar::WriteTesseractBoxFile(filename, page_height_, boxchars_);
}

// Returns cluster strings in logical order.
bool StringRenderer::GetClusterStrings(vector<string>* cluster_text) {
  map<int, string> start_byte_to_text;
  PangoLayoutIter* run_iter = pango_layout_get_iter(layout_);
  const char* full_text = pango_layout_get_text(layout_);
  do {
    PangoLayoutRun* run = pango_layout_iter_get_run_readonly(run_iter);
    if (!run) {
      // End of line NULL run marker
      tlog(2, "Found end of line marker\n");
      continue;
    }
    PangoGlyphItemIter cluster_iter;
    gboolean have_cluster;
    for (have_cluster = pango_glyph_item_iter_init_start(&cluster_iter,
                                                          run, full_text);
         have_cluster;
         have_cluster = pango_glyph_item_iter_next_cluster(&cluster_iter)) {
      const int start_byte_index = cluster_iter.start_index;
      const int end_byte_index = cluster_iter.end_index;
      string text = string(full_text + start_byte_index,
                           end_byte_index - start_byte_index);
      if (IsUTF8Whitespace(text.c_str())) {
        tlog(2, "Found whitespace\n");
        text = " ";
      }
      tlog(2, "start_byte=%d end_byte=%d : '%s'\n", start_byte_index,
           end_byte_index, text.c_str());
      if (add_ligatures_) {
        // Make sure the output box files have ligatured text in case the font
        // decided to use an unmapped glyph.
        text = LigatureTable::Get()->AddLigatures(text, NULL);
      }
      start_byte_to_text[start_byte_index] = text;
    }
  } while (pango_layout_iter_next_run(run_iter));
  pango_layout_iter_free(run_iter);

  cluster_text->clear();
  for (map<int, string>::const_iterator it = start_byte_to_text.begin();
       it != start_byte_to_text.end(); ++it) {
    cluster_text->push_back(it->second);
  }
  return cluster_text->size();
}

// Merges an array of BoxChars into words based on the identification of
// BoxChars containing the space character as inter-word separators.
//
// Sometime two adjacent characters in the sequence may be detected as lying on
// different lines based on their spatial positions. This may be the result of a
// newline character at end of the last word on a line in the source text, or of
// a discretionary line-break created by Pango at intra-word locations like
// hyphens. When this is detected the word is split at that location into
// multiple BoxChars. Otherwise, each resulting BoxChar will contain a word and
// its bounding box.
static void MergeBoxCharsToWords(vector<BoxChar*>* boxchars) {
  vector<BoxChar*> result;
  bool started_word = false;
  for (int i = 0; i < boxchars->size(); ++i) {
    if (boxchars->at(i)->ch() == " " ||
        boxchars->at(i)->box() == NULL) {
      result.push_back(boxchars->at(i));
      boxchars->at(i) = NULL;
      started_word = false;
      continue;
    }

    if (!started_word) {
      // Begin new word
      started_word = true;
      result.push_back(boxchars->at(i));
      boxchars->at(i) = NULL;
    } else {
      BoxChar* last_boxchar = result.back();
      // Compute bounding box union
      const Box* box = boxchars->at(i)->box();
      Box* last_box = last_boxchar->mutable_box();
      int left = min(last_box->x, box->x);
      int right = max(last_box->x + last_box->w, box->x + box->w);
      int top = min(last_box->y, box->y);
      int bottom = max(last_box->y + last_box->h, box->y + box->h);
      // Conclude that the word was broken to span multiple lines based on the
      // size of the merged bounding box in relation to those of the individual
      // characters seen so far.
      if (right - left > last_box->w + 5 * box->w) {
        tlog(1, "Found line break after '%s'", last_boxchar->ch().c_str());
        // Insert a fake interword space and start a new word with the current
        // boxchar.
        result.push_back(new BoxChar(" ", 1));
        result.push_back(boxchars->at(i));
        boxchars->at(i) = NULL;
        continue;
      }
      // Append to last word
      last_boxchar->mutable_ch()->append(boxchars->at(i)->ch());
      last_box->x = left;
      last_box->w = right - left;
      last_box->y = top;
      last_box->h = bottom - top;
      delete boxchars->at(i);
      boxchars->at(i) = NULL;
    }
  }
  boxchars->swap(result);
}


void StringRenderer::ComputeClusterBoxes() {
  const char* text = pango_layout_get_text(layout_);
  PangoLayoutIter* cluster_iter = pango_layout_get_iter(layout_);

  // Do a first pass to store cluster start indexes.
  vector<int> cluster_start_indices;
  do {
    cluster_start_indices.push_back(pango_layout_iter_get_index(cluster_iter));
    tlog(3, "Added %d\n", cluster_start_indices.back());
  } while (pango_layout_iter_next_cluster(cluster_iter));
  pango_layout_iter_free(cluster_iter);
  cluster_start_indices.push_back(strlen(text));
  tlog(3, "Added last index %d\n", cluster_start_indices.back());
  // Sort the indices and create a map from start to end indices.
  sort(cluster_start_indices.begin(), cluster_start_indices.end());
  map<int, int> cluster_start_to_end_index;
  for (int i = 0; i < cluster_start_indices.size() - 1; ++i) {
    cluster_start_to_end_index[cluster_start_indices[i]]
        = cluster_start_indices[i + 1];
  }

  // Iterate again to compute cluster boxes and their text with the obtained
  // cluster extent information.
  cluster_iter = pango_layout_get_iter(layout_);
  // Store BoxChars* sorted by their byte start positions
  map<int, BoxChar*> start_byte_to_box;
  do {
    PangoRectangle cluster_rect;
    pango_layout_iter_get_cluster_extents(cluster_iter, &cluster_rect,
                                          NULL);
    pango_extents_to_pixels(&cluster_rect, NULL);
    const int start_byte_index = pango_layout_iter_get_index(cluster_iter);
    const int end_byte_index = cluster_start_to_end_index[start_byte_index];
    string cluster_text = string(text + start_byte_index,
                                 end_byte_index - start_byte_index);
    if (cluster_text.size() && cluster_text[0] == '\n') {
      tlog(2, "Skipping newlines at start of text.\n");
      continue;
    }
    if (!cluster_rect.width || !cluster_rect.height ||
        IsUTF8Whitespace(cluster_text.c_str())) {
      tlog(2, "Skipping whitespace with boxdim (%d,%d) '%s'\n",
           cluster_rect.width, cluster_rect.height, cluster_text.c_str());
      BoxChar* boxchar = new BoxChar(" ", 1);
      boxchar->set_page(page_);
      start_byte_to_box[start_byte_index] = boxchar;
      continue;
    }
    // Prepare a boxchar for addition at this byte position.
    tlog(2, "[%d %d], %d, %d : start_byte=%d end_byte=%d : '%s'\n",
         cluster_rect.x, cluster_rect.y,
         cluster_rect.width, cluster_rect.height,
         start_byte_index, end_byte_index,
         cluster_text.c_str());
    ASSERT_HOST_MSG(cluster_rect.width,
                    "cluster_text:%s  start_byte_index:%d\n",
                    cluster_text.c_str(), start_byte_index);
    ASSERT_HOST_MSG(cluster_rect.height,
                    "cluster_text:%s  start_byte_index:%d\n",
                    cluster_text.c_str(), start_byte_index);
    if (box_padding_) {
      cluster_rect.x = max(0, cluster_rect.x - box_padding_);
      cluster_rect.width += 2 * box_padding_;
      cluster_rect.y = max(0, cluster_rect.y - box_padding_);
      cluster_rect.height += 2 * box_padding_;
    }
    if (add_ligatures_) {
      // Make sure the output box files have ligatured text in case the font
      // decided to use an unmapped glyph.
      cluster_text = LigatureTable::Get()->AddLigatures(cluster_text, NULL);
    }
    BoxChar* boxchar = new BoxChar(cluster_text.c_str(), cluster_text.size());
    boxchar->set_page(page_);
    boxchar->AddBox(cluster_rect.x, cluster_rect.y,
                    cluster_rect.width, cluster_rect.height);
    start_byte_to_box[start_byte_index] = boxchar;
  } while (pango_layout_iter_next_cluster(cluster_iter));
  pango_layout_iter_free(cluster_iter);

  // There is a subtle bug in the cluster text reported by the PangoLayoutIter
  // on ligatured characters (eg. The word "Lam-Aliph" in arabic). To work
  // around this, we use text reported using the PangoGlyphIter which is
  // accurate.
  // TODO(ranjith): Revisit whether this is still needed in newer versions of
  // pango.
  vector<string> cluster_text;
  if (GetClusterStrings(&cluster_text)) {
    ASSERT_HOST(cluster_text.size() == start_byte_to_box.size());
    int ind = 0;
    for (map<int, BoxChar*>::iterator it = start_byte_to_box.begin();
         it != start_byte_to_box.end(); ++it, ++ind) {
      it->second->mutable_ch()->swap(cluster_text[ind]);
    }
  }

  // Append to the boxchars list in byte order.
  vector<BoxChar*> page_boxchars;
  page_boxchars.reserve(start_byte_to_box.size());
  string last_ch;
  for (map<int, BoxChar*>::const_iterator it = start_byte_to_box.begin();
       it != start_byte_to_box.end(); ++it) {
    if (it->second->ch() == kWordJoinerUTF8) {
      // Skip zero-width joiner characters (ZWJs) here.
      delete it->second;
    } else {
      page_boxchars.push_back(it->second);
    }
  }
  CorrectBoxPositionsToLayout(&page_boxchars);

  if (render_fullwidth_latin_) {
    for (map<int, BoxChar*>::iterator it = start_byte_to_box.begin();
         it != start_byte_to_box.end(); ++it) {
      // Convert fullwidth Latin characters to their halfwidth forms.
      string half(ConvertFullwidthLatinToBasicLatin(it->second->ch()));
      it->second->mutable_ch()->swap(half);
    }
  }

  // Merge the character boxes into word boxes if we are rendering n-grams.
  if (output_word_boxes_) {
    MergeBoxCharsToWords(&page_boxchars);
  }

  boxchars_.insert(boxchars_.end(), page_boxchars.begin(), page_boxchars.end());

  // Compute the page bounding box
  Box* page_box = NULL;
  Boxa* all_boxes = NULL;
  for (int i = 0; i < page_boxchars.size(); ++i) {
    if (page_boxchars[i]->box() == NULL) continue;
    if (all_boxes == NULL)
      all_boxes = boxaCreate(0);
    boxaAddBox(all_boxes, page_boxchars[i]->mutable_box(), L_CLONE);
  }
  boxaGetExtent(all_boxes, NULL, NULL, &page_box);
  boxaDestroy(&all_boxes);
  if (page_boxes_ == NULL)
    page_boxes_ = boxaCreate(0);
  boxaAddBox(page_boxes_, page_box, L_INSERT);
}


void StringRenderer::CorrectBoxPositionsToLayout(vector<BoxChar*>* boxchars) {
  if (vertical_text_) {
    const double rotation = - pango_gravity_to_rotation(
        pango_context_get_base_gravity(pango_layout_get_context(layout_)));
    BoxChar::TranslateBoxes(page_width_ - h_margin_, v_margin_, boxchars);
    BoxChar::RotateBoxes(rotation, page_width_ - h_margin_, v_margin_,
                         0, boxchars->size(), boxchars);
  } else {
    BoxChar::TranslateBoxes(h_margin_, v_margin_, boxchars);
  }
}

int StringRenderer::StripUnrenderableWords(string* utf8_text) const {
  string output_text;
  const char* text = utf8_text->c_str();
  int offset = 0;
  int num_dropped = 0;
  while (offset < utf8_text->length()) {
    int space_len = SpanUTF8Whitespace(text + offset);
    output_text.append(text + offset, space_len);
    offset += space_len;
    if (offset == utf8_text->length()) break;

    int word_len = SpanUTF8NotWhitespace(text + offset);
    if (font_.CanRenderString(text + offset, word_len)) {
      output_text.append(text + offset, word_len);
    } else {
      ++num_dropped;
    }
    offset += word_len;
  }
  utf8_text->swap(output_text);

  if (num_dropped > 0) {
    tprintf("Stripped %d unrenderable words\n", num_dropped);
  }
  return num_dropped;
}

int StringRenderer::RenderToBinaryImage(const char* text, int text_length,
                                        int threshold, Pix** pix) {
  Pix *orig_pix = NULL;
  int offset = RenderToImage(text, text_length, &orig_pix);
  if (orig_pix) {
    Pix* gray_pix = pixConvertTo8(orig_pix, false);
    pixDestroy(&orig_pix);
    *pix = pixThresholdToBinary(gray_pix, threshold);
    pixDestroy(&gray_pix);
  } else {
    *pix = orig_pix;
  }
  return offset;
}

// Add word joiner (WJ) characters between adjacent non-space characters except
// immediately before a combiner.
/* static */
string StringRenderer::InsertWordJoiners(const string& text) {
  string out_str;
  const UNICHAR::const_iterator it_end = UNICHAR::end(text.c_str(),
                                                      text.length());
  for (UNICHAR::const_iterator it = UNICHAR::begin(text.c_str(), text.length());
       it != it_end; ++it) {
    // Add the symbol to the output string.
    out_str.append(it.utf8_data(), it.utf8_len());
    // Check the next symbol.
    UNICHAR::const_iterator next_it = it;
    ++next_it;
    bool next_char_is_boundary = (next_it == it_end || *next_it == ' ');
    bool next_char_is_combiner = (next_it == it_end) ?
        false : IsCombiner(*next_it);
    if (*it != ' ' && *it != '\n' && !next_char_is_boundary &&
        !next_char_is_combiner) {
      out_str += kWordJoinerUTF8;
    }
  }
  return out_str;
}

// Convert halfwidth Basic Latin characters to their fullwidth forms.
string StringRenderer::ConvertBasicLatinToFullwidthLatin(const string& str) {
  string full_str;
  const UNICHAR::const_iterator it_end = UNICHAR::end(str.c_str(),
                                                      str.length());
  for (UNICHAR::const_iterator it = UNICHAR::begin(str.c_str(), str.length());
       it != it_end; ++it) {
    // Convert printable and non-space 7-bit ASCII characters to
    // their fullwidth forms.
    if (IsInterchangeValid7BitAscii(*it) && isprint(*it) && !isspace(*it)) {
      // Convert by adding 0xFEE0 to the codepoint of 7-bit ASCII.
      char32 full_char = *it + 0xFEE0;
      full_str.append(EncodeAsUTF8(full_char));
    } else {
      full_str.append(it.utf8_data(), it.utf8_len());
    }
  }
  return full_str;
}

// Convert fullwidth Latin characters to their halfwidth forms.
string StringRenderer::ConvertFullwidthLatinToBasicLatin(const string& str) {
  string half_str;
  UNICHAR::const_iterator it_end = UNICHAR::end(str.c_str(), str.length());
  for (UNICHAR::const_iterator it = UNICHAR::begin(str.c_str(), str.length());
       it != it_end; ++it) {
    char32 half_char = FullwidthToHalfwidth(*it);
    // Convert fullwidth Latin characters to their halfwidth forms
    // only if halfwidth forms are printable and non-space 7-bit ASCII.
    if (IsInterchangeValid7BitAscii(half_char) &&
        isprint(half_char) && !isspace(half_char)) {
      half_str.append(EncodeAsUTF8(half_char));
    } else {
      half_str.append(it.utf8_data(), it.utf8_len());
    }
  }
  return half_str;
}

// Returns offset to end of text substring rendered in this method.
int StringRenderer::RenderToImage(const char* text, int text_length,
                                  Pix** pix) {
  if (pix && *pix) pixDestroy(pix);
  InitPangoCairo();

  const int page_offset = FindFirstPageBreakOffset(text, text_length);
  if (!page_offset) {
    return 0;
  }
  start_box_ = boxchars_.size();

  if (!vertical_text_) {
    // Translate by the specified margin
    cairo_translate(cr_, h_margin_, v_margin_);
  } else {
    // Vertical text rendering is achieved by a two-step process of first
    // performing regular horizontal layout with character orientation set to
    // EAST, and then translating and rotating the layout before rendering onto
    // the desired image surface. The settings required for the former step are
    // done within InitPangoCairo().
    //
    // Translate to the top-right margin of page
    cairo_translate(cr_, page_width_ - h_margin_, v_margin_);
    // Rotate the layout
    double rotation = - pango_gravity_to_rotation(
        pango_context_get_base_gravity(pango_layout_get_context(layout_)));
    tlog(2, "Rotating by %f radians\n", rotation);
    cairo_rotate(cr_, rotation);
    pango_cairo_update_layout(cr_, layout_);
  }
  string page_text(text, page_offset);
  if (render_fullwidth_latin_) {
    // Convert Basic Latin to their fullwidth forms.
    page_text = ConvertBasicLatinToFullwidthLatin(page_text);
  }
  if (strip_unrenderable_words_) {
    StripUnrenderableWords(&page_text);
  }
  if (drop_uncovered_chars_ &&
      !font_.CoversUTF8Text(page_text.c_str(), page_text.length())) {
    int num_dropped = font_.DropUncoveredChars(&page_text);
    if (num_dropped) {
      tprintf("WARNING: Dropped %d uncovered characters\n", num_dropped);
    }
  }
  if (add_ligatures_) {
    // Add ligatures wherever possible, including custom ligatures.
    page_text = LigatureTable::Get()->AddLigatures(page_text, &font_);
  }

  pango_layout_set_text(layout_, page_text.c_str(), page_text.length());

  if (pix) {
    // Set a white background for the target image surface.
    cairo_set_source_rgb(cr_, 1.0, 1.0, 1.0);  // sets drawing colour to white
    // Fill the surface with the active colour (if you don't do this, you will
    // be given a surface with a transparent background to draw on)
    cairo_paint(cr_);
    // Set the ink color to black
    cairo_set_source_rgb(cr_, pen_color_[0], pen_color_[1], pen_color_[2]);
    // If the target surface or transformation properties of the cairo instance
    // have changed, update the pango layout to reflect this
    pango_cairo_update_layout(cr_, layout_);
    // Draw the pango layout onto the cairo surface
    pango_cairo_show_layout(cr_, layout_);
    *pix = CairoARGB32ToPixFormat(surface_);
  }
  ComputeClusterBoxes();
  FreePangoCairo();
  // Update internal state variables.
  ++page_;
  return page_offset;
}

// Render a string to an image, returning it as an 8 bit pix.  Behaves as
// RenderString, except that it ignores the font set at construction and works
// through all the fonts, returning 0 until they are exhausted, at which point
// it returns the value it should have returned all along, but no pix this time.
// Fonts that don't contain a large proportion of the characters in the string
// get skipped.
// Fonts that work each get rendered and the font name gets added
// to the image.
// NOTE that no boxes are produced by this function.
//
// Example usage: To render a null terminated char-array "txt"
//
// int offset = 0;
// do {
//   Pix *pix;
//   offset += renderer.RenderAllFontsToImage(txt + offset,
//                                            strlen(txt + offset), NULL, &pix);
//   ...
// } while (offset < strlen(text));
//
int StringRenderer::RenderAllFontsToImage(const char* text, int text_length,
                                          string* font_used, Pix** image) {
  // Select a suitable font to render the title with.
  const char kTitleTemplate[] = "%s : %d hits = %.2f%%, raw = %d = %.2f%%";
  string title_font;
  if (!FontUtils::SelectFont(kTitleTemplate, strlen(kTitleTemplate),
                             &title_font, NULL)) {
    tprintf("WARNING: Could not find a font to render image title with!\n");
    title_font = "Arial";
  }
  title_font += " 8";
  tlog(1, "Selected title font: %s\n", title_font.c_str());
  if (font_used) font_used->clear();

  string orig_font = font_.DescriptionName();
  if (char_map_.empty()) {
    total_chars_ = 0;
    // Fill the hash table and use that for computing which fonts to use.
    for (UNICHAR::const_iterator it = UNICHAR::begin(text, text_length);
         it != UNICHAR::end(text, text_length); ++it) {
      ++total_chars_;
      ++char_map_[*it];
    }
    tprintf("Total chars = %d\n", total_chars_);
  }
  const vector<string>& all_fonts = FontUtils::ListAvailableFonts();
  for (int i = font_index_; i < all_fonts.size(); ++i) {
    ++font_index_;
    int raw_score = 0;
    int ok_chars = FontUtils::FontScore(char_map_, all_fonts[i], &raw_score,
                                        NULL);
    if (ok_chars > 0 && ok_chars == total_chars_) {
      set_font(all_fonts[i]);
      int offset = RenderToBinaryImage(text, text_length, 128, image);
      ClearBoxes();  // Get rid of them as they are garbage.
      const int kMaxTitleLength = 1024;
      char title[kMaxTitleLength];
      snprintf(title, kMaxTitleLength, kTitleTemplate,
               all_fonts[i].c_str(), ok_chars,
               100.0 * ok_chars / total_chars_, raw_score,
               100.0 * raw_score / char_map_.size());
      tprintf("%s\n", title);
      // This is a good font! Store the offset to return once we've tried all
      // the fonts.
      if (offset) {
        last_offset_ = offset;
        if (font_used) *font_used = all_fonts[i];
      }
      // Add the font to the image.
      set_font(title_font);
      v_margin_ /= 8;
      Pix* title_image = NULL;
      RenderToBinaryImage(title, strlen(title), 128, &title_image);
      pixOr(*image, *image, title_image);
      pixDestroy(&title_image);

      v_margin_ *= 8;
      set_font(orig_font);
      // We return the real offset only after cycling through the list of fonts.
      return 0;
    } else {
      tprintf("Font %s failed with %d hits = %.2f%%\n",
              all_fonts[i].c_str(), ok_chars, 100.0 * ok_chars / total_chars_);
    }
  }
  *image = NULL;
  font_index_ = 0;
  char_map_.clear();
  return last_offset_;
}

}  // namespace tesseract
