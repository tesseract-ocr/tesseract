///////////////////////////////////////////////////////////////////////
// File:        hfst_io_utils.cpp
// Description: Useful functions for reading HFST optimized lookup fsts
// Author:      Miikka Silfverberg
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
///////////////////////////////////////////////////////////////////////

#include "hfst_io_utils.h"
#include "tprintf.h"

#include <cassert>
#include <cstring>

namespace hfst {

void readsome(char * target, size_t len, FILE * file) {
  size_t block_count = fread(target, len, 1, file);

  if (block_count != 1) {
    tprintf ("readsome: Error reading file\n");
  }
}

char* read_string(FILE * file) {
  if (feof(file) or ferror(file)) {
    tprintf ("read_string: Error reading file\n");
  }

  char target[MAX_STRING_BUFFER + 1];

  for (size_t i = 0; i < MAX_STRING_BUFFER; ++i) {
    target[i] = static_cast<char>(getc(file));

    if (target[i] == '\0') {
      break;
    }
  }

  target[MAX_STRING_BUFFER] = '\0';

  return strdup((const char *)target);
}

}
