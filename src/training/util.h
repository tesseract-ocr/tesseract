/**********************************************************************
 * File:        util.h
 * Description: Misc STL string utility functions.
 * Author:      Samuel Charron
 *
 * (C) Copyright 2013, Google Inc.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 **********************************************************************/

#ifndef TESSERACT_TRAINING_UTIL_H_
#define TESSERACT_TRAINING_UTIL_H_

#include <cstddef>
#include <cstdlib>
#include <string>
#include <vector>

#include <tesseract/platform.h>

// StringHash is the hashing functor needed by the stl hash map.
#ifndef COMPILER_MSVC
struct StringHash {
  size_t operator()(const std::string& s) const {
    size_t hash_code = 0;
    const uint8_t* str = reinterpret_cast<const uint8_t*>(s.c_str());
    for (unsigned ch = 0; str[ch] != 0; ++ch) {
      hash_code += str[ch] << (ch % 24);
    }
    return hash_code;
  }
};
#else  // COMPILER_MSVC
struct StringHash : public stdext::hash_compare <std::string> {
  size_t operator()(const std::string& s) const {
    size_t hash_code = 0;
    const uint8_t* str = reinterpret_cast<const uint8_t*>(s.c_str());
    for (unsigned ch = 0; str[ch] != 0; ++ch) {
      hash_code += str[ch] << (ch % 24);
    }
    return hash_code;
  }
  bool operator()(const std::string& s1, const std::string& s2) const {
    return s1 == s2;
  }
};
#endif  // !COMPILER_MSVC

#ifdef GOOGLE_TESSERACT
#include "base/heap-checker.h"
#define DISABLE_HEAP_LEAK_CHECK HeapLeakChecker::Disabler disabler
#else
#define DISABLE_HEAP_LEAK_CHECK {}
#endif

#endif  // TESSERACT_TRAINING_UTIL_H_
