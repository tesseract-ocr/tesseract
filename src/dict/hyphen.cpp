/******************************************************************************
 * File:         hyphen.cpp  (Formerly hyphen.c)
 * Description:  Functions for maintaining information about hyphenated words.
 * Author:       Mark Seaman, OCR Technology
 * Status:       Reusable Software Component
 *
 * (c) Copyright 1987, Hewlett-Packard Company.
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

namespace tesseract {

// Unless the previous word was the last one on the line, and the current
// one is not (thus it is the first one on the line), erase hyphen_word_,
// clear hyphen_active_dawgs_, hyphen_constraints_ update last_word_on_line_.
void Dict::reset_hyphen_vars(bool last_word_on_line) {
  if (!(last_word_on_line_ == true && last_word_on_line == false)) {
    if (hyphen_word_ != nullptr) {
      delete hyphen_word_;
      hyphen_word_ = nullptr;
      hyphen_active_dawgs_.clear();
    }
  }
  if (hyphen_debug_level) {
    tprintf("reset_hyphen_vars: last_word_on_line %d -> %d\n",
            last_word_on_line_, last_word_on_line);
  }
  last_word_on_line_ = last_word_on_line;
}

// Update hyphen_word_, and copy the given DawgPositionVectors into
// hyphen_active_dawgs_.
void Dict::set_hyphen_word(const WERD_CHOICE &word,
                           const DawgPositionVector &active_dawgs) {
  if (hyphen_word_ == nullptr) {
    hyphen_word_ = new WERD_CHOICE(word.unicharset());
    hyphen_word_->make_bad();
  }
  if (hyphen_word_->rating() > word.rating()) {
    *hyphen_word_ = word;
    // Remove the last unichar id as it is a hyphen, and remove
    // any unichar_string/lengths that are present.
    hyphen_word_->remove_last_unichar_id();
    hyphen_active_dawgs_ = active_dawgs;
  }
  if (hyphen_debug_level) {
    hyphen_word_->print("set_hyphen_word: ");
  }
}
}  // namespace tesseract
