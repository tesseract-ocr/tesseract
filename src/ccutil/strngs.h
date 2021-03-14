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

#include <tesseract/export.h> // for TESS_API

#include <cassert> // for assert
#include <cstdint> // for uint32_t
#include <cstdio>  // for FILE
#include <cstring> // for strncpy
#include <string>
#include <vector>

namespace tesseract {

class TFile;

TESS_API
const std::vector<std::string> split(const std::string &s, char c);

class STRING : public std::string {
public:
  using std::string::string;
  STRING(const std::string &s) : std::string(s) {}
  STRING(const char *s) : std::string(s ? s : "") {}

  // Writes to the given file. Returns false in case of error.
  TESS_API
  bool Serialize(FILE *fp) const;
  // Reads from the given file. Returns false in case of error.
  // If swap is true, assumes a big/little-endian swap is needed.
  TESS_API
  bool DeSerialize(bool swap, FILE *fp);
  // Writes to the given file. Returns false in case of error.
  TESS_API
  bool Serialize(tesseract::TFile *fp) const;
  // Reads from the given file. Returns false in case of error.
  // If swap is true, assumes a big/little-endian swap is needed.
  TESS_API
  bool DeSerialize(tesseract::TFile *fp);
  // As DeSerialize, but only seeks past the data - hence a static method.
  TESS_API
  static bool SkipDeSerialize(tesseract::TFile *fp);

  TESS_API
  bool contains(char c) const;

  TESS_API
  void split(char c, std::vector<STRING> *splited);
};

} // namespace tesseract.

#endif
