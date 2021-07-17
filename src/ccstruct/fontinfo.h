///////////////////////////////////////////////////////////////////////
// File:        fontinfo.h
// Description: Font information classes abstracted from intproto.h/cpp.
// Author:      rays@google.com (Ray Smith)
//
// (C) Copyright 2011, Google Inc.
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
///////////////////////////////////////////////////////////////////////

#ifndef TESSERACT_CCSTRUCT_FONTINFO_H_
#define TESSERACT_CCSTRUCT_FONTINFO_H_

#include "errcode.h"

#include <tesseract/unichar.h>
#include "genericvector.h"

#include <cstdint> // for uint16_t, uint32_t
#include <cstdio>  // for FILE
#include <vector>

namespace tesseract {

template <typename T>
class UnicityTable;

// Simple struct to hold a font and a score. The scores come from the low-level
// integer matcher, so they are in the uint16_t range. Fonts are an index to
// fontinfo_table.
// These get copied around a lot, so best to keep them small.
struct ScoredFont {
  ScoredFont() : fontinfo_id(-1), score(0) {}
  ScoredFont(int font_id, uint16_t classifier_score)
      : fontinfo_id(font_id), score(classifier_score) {}

  // Index into fontinfo table, but inside the classifier, may be a shapetable
  // index.
  int32_t fontinfo_id;
  // Raw score from the low-level classifier.
  uint16_t score;
};

// Struct for information about spacing between characters in a particular font.
struct FontSpacingInfo {
  int16_t x_gap_before;
  int16_t x_gap_after;
  std::vector<UNICHAR_ID> kerned_unichar_ids;
  std::vector<int16_t> kerned_x_gaps;
};

/*
 * font_properties contains properties about boldness, italicness, fixed pitch,
 * serif, fraktur
 */
struct FontInfo {
  FontInfo() : name(nullptr), properties(0), universal_id(0), spacing_vec(nullptr) {}
  ~FontInfo() = default;

  bool operator==(const FontInfo &rhs) const {
    return strcmp(name, rhs.name) == 0;
  }

  // Writes to the given file. Returns false in case of error.
  bool Serialize(FILE *fp) const;
  // Reads from the given file. Returns false in case of error.
  // If swap is true, assumes a big/little-endian swap is needed.
  bool DeSerialize(TFile *fp);

  // Reserves unicharset_size spots in spacing_vec.
  void init_spacing(int unicharset_size) {
    spacing_vec = new std::vector<FontSpacingInfo *>(unicharset_size);
  }
  // Adds the given pointer to FontSpacingInfo to spacing_vec member
  // (FontInfo class takes ownership of the pointer).
  // Note: init_spacing should be called before calling this function.
  void add_spacing(UNICHAR_ID uch_id, FontSpacingInfo *spacing_info) {
    ASSERT_HOST(static_cast<size_t>(uch_id) < spacing_vec->size());
    (*spacing_vec)[uch_id] = spacing_info;
  }

  // Returns the pointer to FontSpacingInfo for the given UNICHAR_ID.
  const FontSpacingInfo *get_spacing(UNICHAR_ID uch_id) const {
    return (spacing_vec == nullptr || spacing_vec->size() <= static_cast<size_t>(uch_id)) ? nullptr
                                                                     : (*spacing_vec)[uch_id];
  }

  // Fills spacing with the value of the x gap expected between the two given
  // UNICHAR_IDs. Returns true on success.
  bool get_spacing(UNICHAR_ID prev_uch_id, UNICHAR_ID uch_id, int *spacing) const {
    const FontSpacingInfo *prev_fsi = this->get_spacing(prev_uch_id);
    const FontSpacingInfo *fsi = this->get_spacing(uch_id);
    if (prev_fsi == nullptr || fsi == nullptr) {
      return false;
    }
    size_t i = 0;
    for (; i < prev_fsi->kerned_unichar_ids.size(); ++i) {
      if (prev_fsi->kerned_unichar_ids[i] == uch_id) {
        break;
      }
    }
    if (i < prev_fsi->kerned_unichar_ids.size()) {
      *spacing = prev_fsi->kerned_x_gaps[i];
    } else {
      *spacing = prev_fsi->x_gap_after + fsi->x_gap_before;
    }
    return true;
  }

  bool is_italic() const {
    return properties & 1;
  }
  bool is_bold() const {
    return (properties & 2) != 0;
  }
  bool is_fixed_pitch() const {
    return (properties & 4) != 0;
  }
  bool is_serif() const {
    return (properties & 8) != 0;
  }
  bool is_fraktur() const {
    return (properties & 16) != 0;
  }

  char *name;
  uint32_t properties;
  // The universal_id is a field reserved for the initialization process
  // to assign a unique id number to all fonts loaded for the current
  // combination of languages. This id will then be returned by
  // ResultIterator::WordFontAttributes.
  int32_t universal_id;
  // Horizontal spacing between characters (indexed by UNICHAR_ID).
  std::vector<FontSpacingInfo *> *spacing_vec;
};

// Every class (character) owns a FontSet that represents all the fonts that can
// render this character.
// Since almost all the characters from the same script share the same set of
// fonts, the sets are shared over multiple classes (see
// Classify::fontset_table_). Thus, a class only store an id to a set.
// Because some fonts cannot render just one character of a set, there are a
// lot of FontSet that differ only by one font. Rather than storing directly
// the FontInfo in the FontSet structure, it's better to share FontInfos among
// FontSets (Classify::fontinfo_table_).
using FontSet = std::vector<int>;

// Class that adds a bit of functionality on top of GenericVector to
// implement a table of FontInfo that replaces UniCityTable<FontInfo>.
// TODO(rays) change all references once all existing traineddata files
// are replaced.
class FontInfoTable : public GenericVector<FontInfo> {
public:
  TESS_API // when you remove inheritance from GenericVector, move this on
  // class level
  FontInfoTable();
  TESS_API
  ~FontInfoTable();

  // Writes to the given file. Returns false in case of error.
  TESS_API
  bool Serialize(FILE *fp) const;
  // Reads from the given file. Returns false in case of error.
  // If swap is true, assumes a big/little-endian swap is needed.
  TESS_API
  bool DeSerialize(TFile *fp);

  // Returns true if the given set of fonts includes one with the same
  // properties as font_id.
  TESS_API
  bool SetContainsFontProperties(int font_id, const std::vector<ScoredFont> &font_set) const;
  // Returns true if the given set of fonts includes multiple properties.
  TESS_API
  bool SetContainsMultipleFontProperties(const std::vector<ScoredFont> &font_set) const;

  // Moves any non-empty FontSpacingInfo entries from other to this.
  TESS_API
  void MoveSpacingInfoFrom(FontInfoTable *other);
  // Moves this to the target unicity table.
  TESS_API
  void MoveTo(UnicityTable<FontInfo> *target);
};

// Deletion callbacks for GenericVector.
void FontInfoDeleteCallback(FontInfo f);

// Callbacks used by UnicityTable to read/write FontInfo/FontSet structures.
bool read_info(TFile *f, FontInfo *fi);
bool write_info(FILE *f, const FontInfo &fi);
bool read_spacing_info(TFile *f, FontInfo *fi);
bool write_spacing_info(FILE *f, const FontInfo &fi);
bool write_set(FILE *f, const FontSet &fs);

} // namespace tesseract.

#endif /* THIRD_PARTY_TESSERACT_CCSTRUCT_FONTINFO_H_ */
