/**********************************************************************
 * File:        char_samp_enum.h
 * Description: Declaration of a Character Sample Enumerator Class
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

// The CharSampEnum class provides the base class for CharSamp class
// Enumerators. This is typically used to implement dump file readers

#ifndef CHARSAMP_ENUM_H
#define CHARSAMP_ENUM_H

#include "char_samp.h"

namespace tesseract {

class CharSampEnum {
 public:
  CharSampEnum();
  virtual ~CharSampEnum();
  virtual bool EnumCharSamp(CharSamp *char_samp, float progress) = 0;
};
}

#endif  // CHARSAMP_ENUM_H
