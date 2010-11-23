/**********************************************************************
 * File:        alt_list.h
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

// The AltList class is the base class for the list of alternate recognition
// results. Each alternate has a cost an an optional tag associated with it

#ifndef ALT_LIST_H
#define ALT_LIST_H

#include <math.h>
#include "cube_utils.h"

namespace tesseract {
class AltList {
 public:
  explicit AltList(int max_alt);
  virtual ~AltList();
  // sort the list of alternates based
  virtual void Sort() = 0;
  // return the best possible cost and index of corresponding alternate
  int BestCost (int *best_alt) const;
  // return the count of alternates
  inline int AltCount() const { return alt_cnt_; }
  // returns the cost (-ve log prob) of an alternate
  inline int AltCost(int alt_idx) const { return alt_cost_[alt_idx]; }
  // returns the prob of an alternate
  inline double AltProb(int alt_idx) const {
    return CubeUtils::Cost2Prob(AltCost(alt_idx));
  }
  // returns the alternate tag
  inline void *AltTag(int alt_idx) const { return alt_tag_[alt_idx]; }

 protected:
  // max number of alternates the list can hold
  int max_alt_;
  // actual alternate count
  int alt_cnt_;
  // array of alternate costs
  int *alt_cost_;
  // array of alternate tags
  void **alt_tag_;
};
}

#endif  // ALT_LIST_H
