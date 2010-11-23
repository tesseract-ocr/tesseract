/**********************************************************************
 * File:        word_altlist.cpp
 * Description: Implementation of the Word Alternate List Class
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

#include "word_altlist.h"

namespace tesseract {
WordAltList::WordAltList(int max_alt)
    : AltList(max_alt) {
  word_alt_ = NULL;
}

WordAltList::~WordAltList() {
  if (word_alt_ != NULL) {
    for (int alt_idx = 0; alt_idx < alt_cnt_; alt_idx++) {
      if (word_alt_[alt_idx] != NULL) {
        delete []word_alt_[alt_idx];
      }
    }
    delete []word_alt_;
    word_alt_ = NULL;
  }
}

// insert an alternate word with the specified cost and tag
bool WordAltList::Insert(char_32 *word_str, int cost, void *tag) {
  if (word_alt_ == NULL || alt_cost_ == NULL) {
    word_alt_ = new char_32*[max_alt_];
    alt_cost_ = new int[max_alt_];
    alt_tag_ = new void *[max_alt_];

    if (word_alt_ == NULL || alt_cost_ == NULL || alt_tag_ == NULL) {
      return false;
    }

    memset(alt_tag_, 0, max_alt_ * sizeof(*alt_tag_));
  } else {
    // check if alt already exists
    for (int alt_idx = 0; alt_idx < alt_cnt_; alt_idx++) {
      if (CubeUtils::StrCmp(word_str, word_alt_[alt_idx]) == 0) {
        // update the cost if we have a lower one
        if (cost < alt_cost_[alt_idx]) {
          alt_cost_[alt_idx] = cost;
          alt_tag_[alt_idx] = tag;
        }
        return true;
      }
    }
  }

  // determine length of alternate
  int len = CubeUtils::StrLen(word_str);

  word_alt_[alt_cnt_] = new char_32[len + 1];
  if (word_alt_[alt_cnt_] == NULL) {
    return false;
  }

  if (len > 0) {
    memcpy(word_alt_[alt_cnt_], word_str, len * sizeof(*word_str));
  }

  word_alt_[alt_cnt_][len] = 0;
  alt_cost_[alt_cnt_] = cost;
  alt_tag_[alt_cnt_] = tag;

  alt_cnt_++;

  return true;
}

// sort the alternate in descending order based on the cost
void WordAltList::Sort() {
  for (int alt_idx = 0; alt_idx < alt_cnt_; alt_idx++) {
    for (int alt = alt_idx + 1; alt < alt_cnt_; alt++) {
      if (alt_cost_[alt_idx] > alt_cost_[alt]) {
        char_32 *pchTemp = word_alt_[alt_idx];
        word_alt_[alt_idx] = word_alt_[alt];
        word_alt_[alt] = pchTemp;

        int temp = alt_cost_[alt_idx];
        alt_cost_[alt_idx] = alt_cost_[alt];
        alt_cost_[alt] = temp;

        void *tag = alt_tag_[alt_idx];
        alt_tag_[alt_idx] = alt_tag_[alt];
        alt_tag_[alt] = tag;
      }
    }
  }
}

void WordAltList::PrintDebug() {
  for (int alt_idx = 0; alt_idx < alt_cnt_; alt_idx++) {
    char_32 *word_32 = word_alt_[alt_idx];
    string word_str;
    CubeUtils::UTF32ToUTF8(word_32, &word_str);
    int num_unichars = CubeUtils::StrLen(word_32);
    fprintf(stderr, "Alt[%d]=%s (cost=%d, num_unichars=%d); unichars=", alt_idx,
            word_str.c_str(), alt_cost_[alt_idx], num_unichars);
    for (int i = 0; i < num_unichars; ++i)
      fprintf(stderr, "%d ", word_32[i]);
    fprintf(stderr, "\n");
  }
}
}  // namespace tesseract
