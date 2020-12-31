/**********************************************************************
 * File:        strngs.h  (Formerly strings.h)
 * Description: STRING class definition.
 * Author:      Ray Smith
 *
 * (C) Copyright 1991, Hewlett-Packard Ltd.
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

#ifndef STRNGS_H
#define STRNGS_H

#include <tesseract/export.h>  // for TESS_API

#include <cassert>  // for assert
#include <cstdint>  // for uint32_t
#include <cstdio>   // for FILE
#include <cstring>  // for strncpy
#include <string>
#include <vector>

namespace tesseract {

class TFile;

class STRING : public std::string {
 public:
  using std::string::string;
  STRING(const std::string &s) : std::string(s) {}
  STRING(const char *s) : std::string(s ? s : "") {}

  // Writes to the given file. Returns false in case of error.
  TESS_API
  bool Serialize(FILE* fp) const;
  // Reads from the given file. Returns false in case of error.
  // If swap is true, assumes a big/little-endian swap is needed.
  TESS_API
  bool DeSerialize(bool swap, FILE* fp);
  // Writes to the given file. Returns false in case of error.
  TESS_API
  bool Serialize(tesseract::TFile* fp) const;
  // Reads from the given file. Returns false in case of error.
  // If swap is true, assumes a big/little-endian swap is needed.
  TESS_API
  bool DeSerialize(tesseract::TFile* fp);
  // As DeSerialize, but only seeks past the data - hence a static method.
  TESS_API
  static bool SkipDeSerialize(tesseract::TFile* fp);

  TESS_API
  bool contains(char c) const;
  int32_t size() const {
    return length();
  }
  // Workaround to avoid g++ -Wsign-compare warnings.
  uint32_t unsigned_size() const {
    const int32_t len = length();
    assert(0 <= len);
    return static_cast<uint32_t>(len);
  }

  inline char* strdup() const {
    int32_t len = length() + 1;
    return strncpy(new char[len], c_str(), len);
  }

  TESS_API
  void split(char c, std::vector<STRING>* splited);
  TESS_API
  void truncate_at(int32_t index);

  // Appends the given string and int (as a %d) to this.
  // += cannot be used for ints as there as a char += operator that would
  // be ambiguous, and ints usually need a string before or between them
  // anyway.
  TESS_API
  void add_str_int(const char* str, int number);
  // Appends the given string and double (as a %.8g) to this.
  TESS_API
  void add_str_double(const char* str, double number);
};

}  // namespace tesseract.

#endif
