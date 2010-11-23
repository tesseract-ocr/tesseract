/**********************************************************************
 * File:        alt_list.cpp
 * Description: Class to abstarct a list of alternate results
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

#include "altlist.h"
#include <stdlib.h>

namespace tesseract {

AltList::AltList(int max_alt) {
  max_alt_ = max_alt;
  alt_cnt_ = 0;
  alt_cost_ = NULL;
  alt_tag_ = NULL;
}

AltList::~AltList() {
  if (alt_cost_ != NULL) {
    delete []alt_cost_;
    alt_cost_ = NULL;
  }

  if (alt_tag_ != NULL) {
    delete []alt_tag_;
    alt_tag_ = NULL;
  }
}

// return the best possible cost and index of corresponding alternate
int AltList::BestCost(int *best_alt) const {
  if (alt_cnt_ <= 0) {
    (*best_alt) = -1;
    return -1;
  }

  int best_alt_idx = 0;
  for (int alt_idx = 1; alt_idx < alt_cnt_; alt_idx++) {
    if (alt_cost_[alt_idx] < alt_cost_[best_alt_idx]) {
      best_alt_idx = alt_idx;
    }
  }
  (*best_alt) = best_alt_idx;
  return alt_cost_[best_alt_idx];
}
}
