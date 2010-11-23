/**********************************************************************
 * File:        char_altlist.h
 * Description: Declaration of a Character Alternate List Class
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

#ifndef CHAR_ALT_LIST_H
#define CHAR_ALT_LIST_H

// The CharAltList class holds the list of class alternates returned from
// a character classifier. Each alternate represents a class ID.
// It inherits from the AltList class.
// The CharAltList owns a CharSet object that maps a class-id to a string.

#include "altlist.h"
#include "char_set.h"

namespace tesseract {
class CharAltList : public AltList {
 public:
  CharAltList(const CharSet *char_set, int max_alt = kMaxCharAlt);
  ~CharAltList();

  // Sort the alternate list based on cost
  void Sort();
  // insert a new alternate with the specified class-id, cost and tag
  bool Insert(int class_id, int cost, void *tag = NULL);
  // returns the cost of a specific class ID
  inline int ClassCost(int class_id) const {
    if (class_id_cost_ == NULL ||
        class_id < 0 ||
        class_id >= char_set_->ClassCount()) {
      return WORST_COST;
    }
    return class_id_cost_[class_id];
  }
  // returns the alternate class-id corresponding to an alternate index
  inline int Alt(int alt_idx) const { return class_id_alt_[alt_idx]; }
  // set the cost of a certain alternate
  void SetAltCost(int alt_idx, int cost) {
    alt_cost_[alt_idx] = cost;
    class_id_cost_[class_id_alt_[alt_idx]] = cost;
  }

 private:
  // character set object. Passed at construction time
  const CharSet *char_set_;
  // array of alternate class-ids
  int *class_id_alt_;
  // array of alternate costs
  int *class_id_cost_;
  // default max count of alternates
  static const int kMaxCharAlt = 256;
};
}

#endif  // CHAR_ALT_LIST_H
