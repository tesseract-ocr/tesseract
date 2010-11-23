/**********************************************************************
 * File:        char_altlist.cpp
 * Description: Implementation of a Character Alternate List Class
 * Author:    Ahmad Abdulkader
 * Created:   2007
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

#include "char_altlist.h"

namespace tesseract {

// The CharSet is not class owned and must exist for
// the life time of this class
CharAltList::CharAltList(const CharSet *char_set, int max_alt)
    : AltList(max_alt) {
  char_set_ = char_set;
  max_alt_ = max_alt;
  class_id_alt_ = NULL;
  class_id_cost_ = NULL;
}

CharAltList::~CharAltList() {
  if (class_id_alt_ != NULL) {
    delete []class_id_alt_;
    class_id_alt_ = NULL;
  }

  if (class_id_cost_ != NULL) {
    delete []class_id_cost_;
    class_id_cost_ = NULL;
  }
}

// Insert a new char alternate
bool CharAltList::Insert(int class_id, int cost, void *tag) {
  // validate class ID
  if (class_id < 0 || class_id >= char_set_->ClassCount()) {
    return false;
  }

  // allocate buffers if nedded
  if (class_id_alt_ == NULL || alt_cost_ == NULL) {
    class_id_alt_ = new int[max_alt_];
    alt_cost_ = new int[max_alt_];
    alt_tag_ = new void *[max_alt_];

    if (class_id_alt_ == NULL || alt_cost_ == NULL || alt_tag_ == NULL) {
      return false;
    }

    memset(alt_tag_, 0, max_alt_ * sizeof(*alt_tag_));
  }

  if (class_id_cost_ == NULL) {
    int class_cnt = char_set_->ClassCount();

    class_id_cost_ = new int[class_cnt];
    if (class_id_cost_ == NULL) {
      return false;
    }

    for (int ich = 0; ich < class_cnt; ich++) {
      class_id_cost_[ich] = WORST_COST;
    }
  }

  if (class_id < 0 || class_id >= char_set_->ClassCount()) {
    return false;
  }

  // insert the alternate
  class_id_alt_[alt_cnt_] = class_id;
  alt_cost_[alt_cnt_] = cost;
  alt_tag_[alt_cnt_] = tag;

  alt_cnt_++;

  class_id_cost_[class_id] = cost;

  return true;
}

// sort the alternate Desc. based on prob
void CharAltList::Sort() {
  for (int alt_idx = 0; alt_idx < alt_cnt_; alt_idx++) {
    for (int alt = alt_idx + 1; alt < alt_cnt_; alt++) {
      if (alt_cost_[alt_idx] > alt_cost_[alt]) {
        int temp = class_id_alt_[alt_idx];
        class_id_alt_[alt_idx] = class_id_alt_[alt];
        class_id_alt_[alt] = temp;

        temp = alt_cost_[alt_idx];
        alt_cost_[alt_idx] = alt_cost_[alt];
        alt_cost_[alt] = temp;

        void *tag = alt_tag_[alt_idx];
        alt_tag_[alt_idx] = alt_tag_[alt];
        alt_tag_[alt] = tag;
      }
    }
  }
}
}
