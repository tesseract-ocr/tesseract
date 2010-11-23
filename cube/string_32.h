/**********************************************************************
 * File:        string_32.h
 * Description: Declaration of a 32 Bit string class
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

// the string_32 class provides the functionality needed
// for a 32-bit string class

#ifndef STRING_32_H
#define STRING_32_H

#include <string.h>
#include <string>
#include <algorithm>
#include <vector>

#ifdef USE_STD_NAMESPACE
using std::basic_string;
using std::string;
using std::vector;
#endif

namespace tesseract {

// basic definitions
typedef signed int char_32;
typedef basic_string<char_32> string_32;
}

#endif  // STRING_32_H
