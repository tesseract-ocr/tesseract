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

#include <string>
#include <vector>

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

} // namespace tesseract
