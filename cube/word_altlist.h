/**********************************************************************
 * File:        word_altlist.h
 * Description: Declaration of the Word Alternate List Class
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

// The WordAltList abstracts a alternate list of words and their corresponding
// costs that result from the word recognition process. The class inherits
// from the AltList class
// It provides methods to add a new word alternate, its corresponding score and
// a tag.

#ifndef WORD_ALT_LIST_H
#define WORD_ALT_LIST_H

#include "altlist.h"

namespace tesseract {
class WordAltList : public AltList {
 public:
  explicit WordAltList(int max_alt);
  ~WordAltList();
  // Sort the list of alternates based on cost
  void Sort();
  // insert an alternate word with the specified cost and tag
  bool Insert(char_32 *char_ptr, int cost, void *tag = NULL);
  // returns the alternate string at the specified position
  inline char_32 * Alt(int alt_idx) { return word_alt_[alt_idx]; }
  // print each entry of the altlist, both UTF8 and unichar ids, and
  // their costs, to stderr
  void PrintDebug();
 private:
  char_32 **word_alt_;
};
}  // namespace tesseract

#endif  // WORD_ALT_LIST_H
