/**********************************************************************
 * File:        word_size_model.h
 * Description: Declaration of the Word Size Model Class
 * Author:    Ahmad Abdulkader
 * Created:   2008
 *
 * (C) Copyright 2008, Google Inc.
 ** Licensed under the Apache License, Version 2.0 (the "License");
 ** you may not use this file except in compliance with the License.
 ** You may obtain a copy of the License at
 ** http://www.apache.org/licenses/LICENSE-2.0
 ** Unless required by applicable law or agreed to in writing, software
 ** distributed under the License is distributed on an "AS IS" BASIS,
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 ** See the License for the specific language governing permissions and
 ** limitations under the License.
 *
 **********************************************************************/

// The WordSizeModel class abstracts the geometrical relationships
// between characters/shapes in the same word (presumeably of the same font)
// A non-parametric bigram model describes the three geometrical properties of a
// character pair:
//   1- Normalized Width
//   2- Normalized Top
//   3- Normalized Height
// These dimensions are computed for each character pair in a word. These are
// then compared to the same information for each of the fonts that the size
// model knows about. The WordSizeCost is the cost of the font that matches
// best.

#ifndef WORD_SIZE_MODEL_H
#define WORD_SIZE_MODEL_H

#include <string>
#include "char_samp.h"
#include "char_set.h"

namespace tesseract {
struct PairSizeInfo {
  int delta_top;
  int wid_0;
  int hgt_0;
  int wid_1;
  int hgt_1;
};

struct FontPairSizeInfo {
  string font_name;
  PairSizeInfo **pair_size_info;
};

class WordSizeModel {
 public:
  WordSizeModel(CharSet *, bool contextual);
  virtual ~WordSizeModel();
  static WordSizeModel *Create(const string &data_file_path,
                               const string &lang,
                               CharSet *char_set,
                               bool contextual);
  // Given a word and number of unichars, return the size cost,
  // minimized over all fonts in the size model.
  int Cost(CharSamp **samp_array, int samp_cnt) const;
  // Given dimensions of a pair of character samples and a font size
  // model for that character pair, return the pair's size cost for
  // the font.
  static double PairCost(int width_0, int height_0, int top_0,
                         int width_1, int height_1, int top_1,
                         const PairSizeInfo& pair_info);
  bool Save(string file_name);
  // Number of fonts in size model.
  inline int FontCount() const {
    return font_pair_size_models_.size();
  }
  inline const FontPairSizeInfo *FontInfo() const {
    return &font_pair_size_models_[0];
  }
  // Helper functions to convert between size codes, class id and position
  // codes
  static inline int SizeCode(int cls_id, int start, int end) {
    return (cls_id << 2) + (end << 1) + start;
  }

 private:
  // Scaling constant used to convert floating point ratios in size table
  // to fixed point
  static const int kShapeModelScale = 1000;
  static const int kExpectedTokenCount = 10;

  // Language properties
  bool contextual_;
  CharSet *char_set_;
  // Size ratios table
  vector<FontPairSizeInfo> font_pair_size_models_;

  // Initialize the word size model object
  bool Init(const string &data_file_path, const string &lang);
};
}
#endif  // WORD_SIZE_MODEL_H
