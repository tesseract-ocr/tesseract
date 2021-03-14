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

#include "errcode.h" // for ASSERT_HOST

#include "helpers.h"  // for ReverseN
#include "serialis.h" // for TFile

#include <cassert> // for assert
#include <locale>  // for std::locale::classic
#include <sstream> // for std::stringstream

namespace tesseract {

const std::vector<std::string> split(const std::string &s, char c) {
  std::string buff;
  std::vector<std::string> v;
  for (auto n : s) {
    if (n != c)
      buff += n;
    else if (n == c && !buff.empty()) {
       v.push_back(buff);
       buff.clear();
    }
  }
  if (!buff.empty()) {
    v.push_back(buff);
  }
  return v;
}

// TODO(rays) Change all callers to use TFile and remove the old functions.
// Writes to the given file. Returns false in case of error.
bool STRING::Serialize(FILE *fp) const {
  uint32_t len = length();
  return tesseract::Serialize(fp, &len) && tesseract::Serialize(fp, c_str(), len);
}

// Writes to the given file. Returns false in case of error.
bool STRING::Serialize(TFile *fp) const {
  uint32_t len = length();
  return fp->Serialize(&len) && fp->Serialize(c_str(), len);
}

// Reads from the given file. Returns false in case of error.
// If swap is true, assumes a big/little-endian swap is needed.
bool STRING::DeSerialize(bool swap, FILE *fp) {
  uint32_t len;
  if (!tesseract::DeSerialize(fp, &len))
    return false;
  if (swap)
    ReverseN(&len, sizeof(len));
  // Arbitrarily limit the number of characters to protect against bad data.
  if (len > UINT16_MAX)
    return false;
  resize(len);
  return tesseract::DeSerialize(fp, data(), len);
}

// Reads from the given file. Returns false in case of error.
// If swap is true, assumes a big/little-endian swap is needed.
bool STRING::DeSerialize(TFile *fp) {
  uint32_t len;
  if (!fp->DeSerialize(&len))
    return false;
  resize(len);
  return fp->DeSerialize(data(), len);
}

// As DeSerialize, but only seeks past the data - hence a static method.
bool STRING::SkipDeSerialize(TFile *fp) {
  uint32_t len;
  if (!fp->DeSerialize(&len))
    return false;
  return fp->Skip(len);
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

} // namespace tesseract
