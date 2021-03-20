/******************************************************************************
 ** Filename:    matchdefs.h
 ** Purpose:     Generic interface definitions for feature matchers.
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

#ifndef MATCHDEFS_H
#define MATCHDEFS_H

#include <tesseract/unichar.h>

#include <climits> // INT16_MAX
#include <cstdint> // int16_t

namespace tesseract {

/* define the maximum number of classes defined for any matcher
  and the maximum class id for any matcher. This must be changed
  if more different classes need to be classified */
#define MAX_NUM_CLASSES INT16_MAX

/** a CLASS_ID is the ascii character to be associated with a class */
using CLASS_ID = UNICHAR_ID;
#define NO_CLASS (0)

/** a PROTO_ID is the index of a prototype within it's class.  Valid proto
  id's are 0 to N-1 where N is the number of prototypes that make up the
  class. */
using PROTO_ID = int16_t;
#define NO_PROTO (-1)

/** FEATURE_ID is the index of a feature within a character description
  The feature id ranges from 0 to N-1 where N is the number
  of features in a character description. */
using FEATURE_ID = uint8_t;

} // namespace tesseract

#endif
