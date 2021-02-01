/**********************************************************************
 * File:        strngs.cpp  (Formerly strings.c)
 * Description: STRING class functions.
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

#include "strngs.h"

#include "errcode.h"        // for ASSERT_HOST

#include "helpers.h"        // for ReverseN
#include "serialis.h"       // for TFile

#include <cassert>          // for assert
#include <cstdlib>          // for malloc, free
#include <locale>           // for std::locale::classic
#include <sstream>          // for std::stringstream

namespace tesseract {

// Size of buffer needed to host the decimal representation of the maximum
// possible length of an int (in 64 bits), being -<20 digits>.
const int kMaxIntSize = 22;

// TODO(rays) Change all callers to use TFile and remove the old functions.
// Writes to the given file. Returns false in case of error.
bool STRING::Serialize(FILE* fp) const {
  uint32_t len = length();
  return tesseract::Serialize(fp, &len) &&
         tesseract::Serialize(fp, c_str(), len);
}

// Writes to the given file. Returns false in case of error.
bool STRING::Serialize(TFile* fp) const {
  uint32_t len = length();
  return fp->Serialize(&len) &&
         fp->Serialize(c_str(), len);
}

// Reads from the given file. Returns false in case of error.
// If swap is true, assumes a big/little-endian swap is needed.
bool STRING::DeSerialize(bool swap, FILE* fp) {
  uint32_t len;
  if (!tesseract::DeSerialize(fp, &len)) return false;
  if (swap)
    ReverseN(&len, sizeof(len));
  // Arbitrarily limit the number of characters to protect against bad data.
  if (len > UINT16_MAX) return false;
  truncate_at(len);
  return tesseract::DeSerialize(fp, (char *)data(), len);
}

// Reads from the given file. Returns false in case of error.
// If swap is true, assumes a big/little-endian swap is needed.
bool STRING::DeSerialize(TFile* fp) {
  uint32_t len;
  if (!fp->DeSerialize(&len)) return false;
  truncate_at(len);
  return fp->DeSerialize((char *)data(), len);
}

// As DeSerialize, but only seeks past the data - hence a static method.
bool STRING::SkipDeSerialize(TFile* fp) {
  uint32_t len;
  if (!fp->DeSerialize(&len)) return false;
  return fp->Skip(len);
}

bool STRING::contains(const char c) const {
  return (c != '\0') && (strchr (c_str(), c) != nullptr);
}

void STRING::truncate_at(int32_t index) {
  resize(index);
}

void STRING::split(const char c, std::vector<STRING> *splited) {
  int start_index = 0;
  const int len = length();
  for (int i = 0; i < len; i++) {
    if ((*this)[i] == c) {
      if (i != start_index) {
        (*this)[i] = '\0';
        splited->push_back(STRING(c_str() + start_index, i - start_index));
        (*this)[i] = c;
      }
      start_index = i + 1;
    }
  }

  if (len != start_index) {
    splited->push_back(STRING(c_str() + start_index, len - start_index));
  }
}

void STRING::add_str_int(const char* str, int number) {
  if (str != nullptr)
    *this += str;
  // Allow space for the maximum possible length of int64_t.
  char num_buffer[kMaxIntSize];
  snprintf(num_buffer, kMaxIntSize - 1, "%d", number);
  num_buffer[kMaxIntSize - 1] = '\0';
  *this += num_buffer;
}

// Appends the given string and double (as a %.8g) to this.
void STRING::add_str_double(const char* str, double number) {
  if (str != nullptr)
    *this += str;
  std::stringstream stream;
  // Use "C" locale (needed for double value).
  stream.imbue(std::locale::classic());
  // Use 8 digits for double value.
  stream.precision(8);
  stream << number;
  *this += stream.str().c_str();
}

} // namespace tesseract
