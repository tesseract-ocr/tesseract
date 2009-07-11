///////////////////////////////////////////////////////////////////////
// File:        classify.cpp
// Description: classify class.
// Author:      Samuel Charron
//
// (C) Copyright 2006, Google Inc.
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

#include "classify.h"
#include "intproto.h"
#include "unicity_table.h"
#include <string.h>

namespace {

// Compare FontInfo structures.
bool compare_fontinfo(const FontInfo& fi1, const FontInfo& fi2) {
  // The font properties are required to be the same for two font with the same
  // name, so there is no need to test them.
  // Consequently, querying the table with only its font name as information is
  // enough to retrieve its properties.
  return strcmp(fi1.name, fi2.name) == 0;
}
// Compare FontSet structures.
bool compare_font_set(const FontSet& fs1, const FontSet& fs2) {
  if (fs1.size != fs2.size)
    return false;
  for (int i = 0; i < fs1.size; ++i) {
    if (fs1.configs[i] != fs2.configs[i])
      return false;
  }
  return true;
}

void delete_callback(FontInfo f) {
  delete[] f.name;
}
void delete_callback_fs(FontSet fs) {
  delete[] fs.configs;
}

}

namespace tesseract {
Classify::Classify()
  : INT_MEMBER(tessedit_single_match, FALSE, "Top choice only from CP"),
    BOOL_MEMBER(classify_enable_learning, true, "Enable adaptive classifier"),
    BOOL_MEMBER(classify_recog_devanagari, false,
                "Whether recognizing a language with devanagari script."),
    EnableLearning(true),
    dict_(&image_) {
  fontinfo_table_.set_compare_callback(
      NewPermanentCallback(compare_fontinfo));
  fontinfo_table_.set_clear_callback(
      NewPermanentCallback(delete_callback));
  fontset_table_.set_compare_callback(
      NewPermanentCallback(compare_font_set));
  fontset_table_.set_clear_callback(
      NewPermanentCallback(delete_callback_fs));
  AdaptedTemplates = NULL;
  PreTrainedTemplates = NULL;
  inttemp_loaded_ = false;
  AllProtosOn = NULL;
  PrunedProtos = NULL;
  AllConfigsOn = NULL;
  AllProtosOff = NULL;
  AllConfigsOff = NULL;
  TempProtoMask = NULL;
  NormProtos = NULL;
}

Classify::~Classify() {
  EndAdaptiveClassifier();
}

}  // namespace tesseract
