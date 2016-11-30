/**********************************************************************
 * File:        pango_font_info.h
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

#ifndef TESSERACT_TRAINING_PANGO_FONT_INFO_H_
#define TESSERACT_TRAINING_PANGO_FONT_INFO_H_

#include <string>
#include <utility>
#include <vector>

#include "commandlineflags.h"
#include "hashfn.h"
#include "host.h"
#include "pango/pango-font.h"
#include "pango/pango.h"
#include "pango/pangocairo.h"
#include "util.h"

DECLARE_STRING_PARAM_FLAG(fonts_dir);
DECLARE_STRING_PARAM_FLAG(fontconfig_tmpdir);

typedef signed int char32;

namespace tesseract {

// Data holder class for a font, intended to avoid having to work with Pango or
// FontConfig-specific objects directly.
class PangoFontInfo {
 public:
  enum FontTypeEnum {
    UNKNOWN,
    SERIF,
    SANS_SERIF,
    DECORATIVE,
  };
  PangoFontInfo();
  ~PangoFontInfo();
  // Initialize from parsing a font description name, defined as a string of the
  // format:
  //   "FamilyName [FaceName] [PointSize]"
  // where a missing FaceName implies the default regular face.
  // eg. "Arial Italic 12", "Verdana"
  //
  // FaceName is a combination of:
  //   [StyleName] [Variant] [Weight] [Stretch]
  // with (all optional) Pango-defined values of:
  // StyleName: Oblique, Italic
  // Variant  : Small-Caps
  // Weight   : Ultra-Light, Light, Medium, Semi-Bold, Bold, Ultra-Bold, Heavy
  // Stretch  : Ultra-Condensed, Extra-Condensed, Condensed, Semi-Condensed,
  //            Semi-Expanded, Expanded, Extra-Expanded, Ultra-Expanded.
  explicit PangoFontInfo(const string& name);
  bool ParseFontDescriptionName(const string& name);

  // Returns true if the font have codepoint coverage for the specified text.
  bool CoversUTF8Text(const char* utf8_text, int byte_length) const;
  // Modifies string to remove unicode points that are not covered by the
  // font. Returns the number of characters dropped.
  int DropUncoveredChars(string* utf8_text) const;

  // Returns true if the entire string can be rendered by the font with full
  // character coverage and no unknown glyph or dotted-circle glyph
  // substitutions on encountering a badly formed unicode sequence.
  // If true, returns individual graphemes. Any whitespace characters in the
  // original string are also included in the list.
  bool CanRenderString(const char* utf8_word, int len,
                       std::vector<string>* graphemes) const;
  bool CanRenderString(const char* utf8_word, int len) const;

  // Retrieves the x_bearing and x_advance for the given utf8 character in the
  // font. Returns false if the glyph for the character could not be found in
  // the font.
  // Ref: http://freetype.sourceforge.net/freetype2/docs/glyphs/glyphs-3.html
  bool GetSpacingProperties(const string& utf8_char,
                            int* x_bearing, int* x_advance) const;

  // If not already initialized, initializes FontConfig by setting its
  // environment variable and creating a fonts.conf file that points to the
  // FLAGS_fonts_dir and the cache to FLAGS_fontconfig_tmpdir.
  static void SoftInitFontConfig();
  // Re-initializes font config, whether or not already initialized.
  // If already initialized, any existing cache is deleted, just to be sure.
  static void HardInitFontConfig(const string& fonts_dir,
                                 const string& cache_dir);

  // Accessors
  string DescriptionName() const;
  // Font Family name eg. "Arial"
  const string& family_name() const    { return family_name_; }
  // Size in points (1/72"), rounded to the nearest integer.
  int font_size() const { return font_size_; }
  bool is_bold() const { return is_bold_; }
  bool is_italic() const { return is_italic_; }
  bool is_smallcaps() const { return is_smallcaps_; }
  bool is_monospace() const { return is_monospace_; }
  bool is_fraktur() const { return is_fraktur_; }
  FontTypeEnum font_type() const { return font_type_; }

  int resolution() const { return resolution_; }
  void set_resolution(const int resolution) {
    resolution_ = resolution;
  }

 private:
  friend class FontUtils;
  void Clear();
  bool ParseFontDescription(const PangoFontDescription* desc);
  // Returns the PangoFont structure corresponding to the closest available font
  // in the font map.
  PangoFont* ToPangoFont() const;

  // Font properties set automatically from parsing the font description name.
  string family_name_;
  int font_size_;
  bool is_bold_;
  bool is_italic_;
  bool is_smallcaps_;
  bool is_monospace_;
  bool is_fraktur_;
  FontTypeEnum font_type_;
  // The Pango description that was used to initialize the instance.
  PangoFontDescription* desc_;
  // Default output resolution to assume for GetSpacingProperties() and any
  // other methods that returns pixel values.
  int resolution_;
  // Fontconfig operates through an environment variable, so it intrinsically
  // cannot be thread-friendly, but you can serialize multiple independent
  // font configurations by calling HardInitFontConfig(fonts_dir, cache_dir).
  // These hold the last initialized values set by HardInitFontConfig or
  // the first call to SoftInitFontConfig.
  // Directory to be scanned for font files.
  static string fonts_dir_;
  // Directory to store the cache of font information. (Can be the same as
  // fonts_dir_)
  static string cache_dir_;

 private:
  PangoFontInfo(const PangoFontInfo&);
  void operator=(const PangoFontInfo&);
};

// Static utility methods for querying font availability and font-selection
// based on codepoint coverage.
class FontUtils {
 public:
  // Returns true if the font of the given description name is available in the
  // target directory specified by --fonts_dir
  static bool IsAvailableFont(const char* font_desc) {
    return IsAvailableFont(font_desc, NULL);
  }
  // Returns true if the font of the given description name is available in the
  // target directory specified by --fonts_dir. If false is returned, and
  // best_match is not NULL, the closest matching font is returned there.
  static bool IsAvailableFont(const char* font_desc, string* best_match);
  // Outputs description names of available fonts.
  static const std::vector<string>& ListAvailableFonts();

  // Picks font among available fonts that covers and can render the given word,
  // and returns the font description name and the decomposition of the word to
  // graphemes. Returns false if no suitable font was found.
  static bool SelectFont(const char* utf8_word, const int utf8_len,
                         string* font_name, std::vector<string>* graphemes);

  // Picks font among all_fonts that covers and can render the given word,
  // and returns the font description name and the decomposition of the word to
  // graphemes. Returns false if no suitable font was found.
  static bool SelectFont(const char* utf8_word, const int utf8_len,
                         const std::vector<string>& all_fonts,
                         string* font_name, std::vector<string>* graphemes);

  // Returns a bitmask where the value of true at index 'n' implies that unicode
  // value 'n' is renderable by at least one available font.
  static void GetAllRenderableCharacters(std::vector<bool>* unichar_bitmap);
  // Variant of the above function that inspects only the provided font names.
  static void GetAllRenderableCharacters(const std::vector<string>& font_names,
                                         std::vector<bool>* unichar_bitmap);
  static void GetAllRenderableCharacters(const string& font_name,
                                         std::vector<bool>* unichar_bitmap);

  // NOTE: The following utilities were written to be backward compatible with
  // StringRender.

  // BestFonts returns a font name and a bit vector of the characters it
  // can render for the fonts that score within some fraction of the best
  // font on the characters in the given hash map.
  // In the flags vector, each flag is set according to whether the
  // corresponding character (in order of iterating ch_map) can be rendered.
  // The return string is a list of the acceptable fonts that were used.
  static string BestFonts(
      const TessHashMap<char32, inT64>& ch_map,
      std::vector<std::pair<const char*, std::vector<bool> > >* font_flag);

  // FontScore returns the weighted renderability score of the given
  // hash map character table in the given font. The unweighted score
  // is also returned in raw_score.
  // The values in the bool vector ch_flags correspond to whether the
  // corresponding character (in order of iterating ch_map) can be rendered.
  static int FontScore(const TessHashMap<char32, inT64>& ch_map,
                       const string& fontname, int* raw_score,
                       std::vector<bool>* ch_flags);

  // PangoFontInfo is reinitialized, so clear the static list of fonts.
  static void ReInit();

 private:
  static std::vector<string> available_fonts_;  // cache list
};
}  // namespace tesseract

#endif  // TESSERACT_TRAINING_PANGO_FONT_INFO_H_
