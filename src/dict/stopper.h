/******************************************************************************
 ** Filename:    stopper.h
 ** Purpose:     Stopping criteria for word classifier.
 ** Author:      Dan Johnson
 **
 ** (c) Copyright Hewlett-Packard Company, 1988.
 ** Licensed under the Apache License, Version 2.0 (the "License");
 ** you may not use this file except in compliance with the License.
 ** You may obtain a copy of the License at
 ** http://www.apache.org/licenses/LICENSE-2.0
 ** Unless required by applicable law or agreed to in writing, software
 ** distributed under the License is distributed on an "AS IS" BASIS,
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 ** See the License for the specific language governing permissions and
 ** limitations under the License.
 ******************************************************************************/
#ifndef STOPPER_H
#define STOPPER_H

#include "params.h"
#include "ratngs.h"

#include <tesseract/unichar.h>

namespace tesseract {

class WERD_CHOICE;

using BLOB_WIDTH = uint8_t;

struct DANGERR_INFO {
  DANGERR_INFO()
      : begin(-1)
      , end(-1)
      , dangerous(false)
      , correct_is_ngram(false)
      , leftmost(INVALID_UNICHAR_ID) {}
  DANGERR_INFO(int b, int e, bool d, bool n, UNICHAR_ID l)
      : begin(b), end(e), dangerous(d), correct_is_ngram(n), leftmost(l) {}
  int begin;
  int end;
  bool dangerous;
  bool correct_is_ngram;
  UNICHAR_ID leftmost; // in the replacement, what's the leftmost character?
};

using DANGERR = std::vector<DANGERR_INFO>;

} // namespace tesseract

#endif
