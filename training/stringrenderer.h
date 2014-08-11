/**********************************************************************
 * File:        stringrenderer.h
 * Description: Class for rendering UTF-8 text to an image, and retrieving
 *              bounding boxes around each grapheme cluster.
 *
 *              Instances are created using a font description string
 *              (eg. "Arial Italic 12"; see pango_font_info.h for the format)
 *              and the page dimensions. Other renderer properties such as
 *              spacing, ligaturization, as well a preprocessing behavior such
 *              as removal of unrenderable words and a special n-gram mode may
 *              be set using respective set_* methods.
 *
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

#ifndef TESSERACT_TRAINING_STRINGRENDERER_H_
#define TESSERACT_TRAINING_STRINGRENDERER_H_

#include <string>
#include <vector>

#include "hashfn.h"
#include "host.h"
#include "pango_font_info.h"
#include "pango/pango-layout.h"
#include "pango/pangocairo.h"

struct Boxa;
struct Pix;

namespace tesseract {

class BoxChar;

class StringRenderer {
 public:
  StringRenderer(const string& font_desc, int page_width, int page_height);
  ~StringRenderer();

  // Renders the text with the chosen font and returns the byte offset upto
  // which the text could be rendered so as to fit the specified page
  // dimensions.
  int RenderToImage(const char* text, int text_length, Pix** pix);
  int RenderToGrayscaleImage(const char* text, int text_length, Pix** pix);
  int RenderToBinaryImage(const char* text, int text_length, int threshold,
                          Pix** pix);
  // Renders a line of text with all available fonts that were able to render
  // at least min_coverage fraction of the input text. Use 1.0 to require that
  // a font be able to render all the text.
  int RenderAllFontsToImage(double min_coverage, const char* text,
                            int text_length, string* font_used, Pix** pix);

  bool set_font(const string& desc);
  void set_char_spacing(double char_spacing) {
    char_spacing_ = char_spacing;
  }
  void set_leading(int leading) {
    leading_ = leading;
  }
  void set_resolution(const int resolution);
  void set_vertical_text(bool vertical_text) {
    vertical_text_ = vertical_text;
  }
  void set_gravity_hint_strong(bool gravity_hint_strong) {
    gravity_hint_strong_ = gravity_hint_strong;
  }
  void set_render_fullwidth_latin(bool render_fullwidth_latin) {
    render_fullwidth_latin_ = render_fullwidth_latin;
  }
  // Sets the probability (value in [0, 1]) of starting to render a word with an
  // underline. This implementation consider words to be space-delimited
  // sequences of characters.
  void set_underline_start_prob(const double frac) {
    underline_start_prob_ = min(max(frac, 0.0), 1.0);
  }
  // Set the probability (value in [0, 1]) of continuing a started underline to
  // the next word.
  void set_underline_continuation_prob(const double frac) {
    underline_continuation_prob_ = min(max(frac, 0.0), 1.0);
  }
  void set_underline_style(const PangoUnderline style) {
    underline_style_ = style;
  }
  void set_page(int page) {
    page_ = page;
  }
  void set_box_padding(int val) {
    box_padding_ = val;
  }
  void set_drop_uncovered_chars(bool val) {
    drop_uncovered_chars_ = val;
  }
  void set_strip_unrenderable_words(bool val) {
    strip_unrenderable_words_ = val;
  }
  void set_output_word_boxes(bool val) {
    output_word_boxes_ = val;
  }
  // Before rendering the string, replace latin characters with their optional
  // ligatured forms (such as "fi", "ffi" etc.) if the font_ covers those
  // unicodes.
  void set_add_ligatures(bool add_ligatures) {
    add_ligatures_ = add_ligatures;
  }
  // Set the rgb value of the text ink. Values range in [0, 1.0]
  void set_pen_color(double r, double g, double b) {
    pen_color_[0] = r;
    pen_color_[1] = g;
    pen_color_[2] = b;
  }
  void set_h_margin(const int h_margin) {
    h_margin_ = h_margin;
  }
  void set_v_margin(const int v_margin) {
    v_margin_ = v_margin;
  }
  const PangoFontInfo& font() const {
    return font_;
  }
  const int h_margin() const {
    return h_margin_;
  }
  const int v_margin() const {
    return v_margin_;
  }

  // Get the boxchars of all clusters rendered thus far (or since the last call
  // to ClearBoxes()).
  const vector<BoxChar*>& GetBoxes() const;
  // Get the rendered page bounding boxes of all pages created thus far (or
  // since last call to ClearBoxes()).
  Boxa* GetPageBoxes() const;

  // Rotate the boxes on the most recent page by the given rotation.
  void RotatePageBoxes(float rotation);
  // Delete all boxes.
  void ClearBoxes();
  void WriteAllBoxes(const string& filename) const;
  // Removes space-delimited words from the string that are not renderable by
  // the current font and returns the count of such words.
  int StripUnrenderableWords(string* utf8_text) const;

  // Insert a Word Joiner symbol (U+2060) between adjacent characters, excluding
  // spaces and combining types, in each word before rendering to ensure words
  // are not broken across lines. The output boxchars will not contain the
  // joiner.
  static string InsertWordJoiners(const string& text);

  // Helper functions to convert fullwidth Latin and halfwidth Basic Latin.
  static string ConvertBasicLatinToFullwidthLatin(const string& text);
  static string ConvertFullwidthLatinToBasicLatin(const string& text);

 protected:
  // Init and free local renderer objects.
  void InitPangoCairo();
  void FreePangoCairo();
  // Set rendering properties.
  void SetLayoutProperties();
  void SetWordUnderlineAttributes(const string& page_text);
  // Compute bounding boxes around grapheme clusters.
  void ComputeClusterBoxes();
  void CorrectBoxPositionsToLayout(vector<BoxChar*>* boxchars);
  bool GetClusterStrings(vector<string>* cluster_text);
  int FindFirstPageBreakOffset(const char* text, int text_length);

  PangoFontInfo font_;
  // Page properties
  int page_width_, page_height_, h_margin_, v_margin_;
  // Text rendering properties
  int pen_color_[3];
  double char_spacing_;
  int leading_, resolution_;
  bool vertical_text_;
  bool gravity_hint_strong_;
  bool render_fullwidth_latin_;
  double underline_start_prob_;
  double underline_continuation_prob_;
  PangoUnderline underline_style_;
  // Text filtering options
  bool drop_uncovered_chars_;
  bool strip_unrenderable_words_;
  bool add_ligatures_;
  bool output_word_boxes_;
  // Pango and cairo specific objects
  cairo_surface_t* surface_;
  cairo_t* cr_;
  PangoLayout* layout_;
  // Internal state of current page number, updated on successive calls to
  // RenderToImage()
  int start_box_;
  int page_;
  // Boxes and associated text for all pages rendered with RenderToImage() since
  // the last call to ClearBoxes().
  vector<BoxChar*> boxchars_;
  int box_padding_;
  // Bounding boxes for pages since the last call to ClearBoxes().
  Boxa* page_boxes_;

  // Objects cached for subsequent calls to RenderAllFontsToImage()
  hash_map<char32, inT64> char_map_;  // Time-saving char histogram.
  int total_chars_;   // Number in the string to be rendered.
  int font_index_;    // Index of next font to use in font list.
  int last_offset_;   // Offset returned from last successful rendering

 private:
  StringRenderer(const StringRenderer&);
  void operator=(const StringRenderer&);
};
}  // namespace tesseract

#endif  // THIRD_PARTY_TESSERACT_TRAINING_STRINGRENDERER_H_
