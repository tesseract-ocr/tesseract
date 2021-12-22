/******************************************************************************
 *
 * File:         context.cpp  (Formerly context.c)
 * Description:  Context checking functions
 * Author:       Mark Seaman, OCR Technology
 *
 * (c) Copyright 1990, Hewlett-Packard Company.
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
 *****************************************************************************/

#include "dict.h"
#include "unicharset.h"

namespace tesseract {

static const int kMinAbsoluteGarbageWordLength = 10;
static const float kMinAbsoluteGarbageAlphanumFrac = 0.5f;

const int case_state_table[6][4] = {
    {/*  0. Beginning of word       */
     /*    P   U   L   D                                          */
     /* -1. Error on case           */
     0, 1, 5, 4},
    {/*  1. After initial capital    */
     0, 3, 2, 4},
    {/*  2. After lower case         */
     0, -1, 2, -1},
    {/*  3. After upper case         */
     0, 3, -1, 4},
    {/*  4. After a digit            */
     0, -1, -1, 4},
    {/*  5. After initial lower case */
     5, -1, 2, -1},
};

int Dict::case_ok(const WERD_CHOICE &word) const {
  int state = 0;
  const UNICHARSET *unicharset = word.unicharset();
  for (unsigned x = 0; x < word.length(); ++x) {
    UNICHAR_ID ch_id = word.unichar_id(x);
    if (unicharset->get_isupper(ch_id)) {
      state = case_state_table[state][1];
    } else if (unicharset->get_islower(ch_id)) {
      state = case_state_table[state][2];
    } else if (unicharset->get_isdigit(ch_id)) {
      state = case_state_table[state][3];
    } else {
      state = case_state_table[state][0];
    }
    if (state == -1) {
      return false;
    }
  }
  return state != 5; // single lower is bad
}

bool Dict::absolute_garbage(const WERD_CHOICE &word, const UNICHARSET &unicharset) {
  if (word.length() < kMinAbsoluteGarbageWordLength) {
    return false;
  }
  int num_alphanum = 0;
  for (unsigned x = 0; x < word.length(); ++x) {
    num_alphanum +=
        (unicharset.get_isalpha(word.unichar_id(x)) || unicharset.get_isdigit(word.unichar_id(x)));
  }
  return (static_cast<float>(num_alphanum) / static_cast<float>(word.length()) <
          kMinAbsoluteGarbageAlphanumFrac);
}

} // namespace tesseract
