///////////////////////////////////////////////////////////////////////
// File:        unichar.h
// Description: Unicode character/ligature class.
// Author:      Ray Smith
// Created:     Wed Jun 28 17:05:01 PDT 2006
//
// (C) Copyright 2006, Google Inc.
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

#ifndef TESSERACT_CCUTIL_UNICHAR_H__
#define TESSERACT_CCUTIL_UNICHAR_H__

#include <memory.h>
#include <string.h>

// Maximum number of characters that can be stored in a UNICHAR. Must be
// at least 4. Must not exceed 31 without changing the coding of length.
#define UNICHAR_LEN 24

// A UNICHAR_ID is the unique id of a unichar.
typedef int UNICHAR_ID;

// A variable to indicate an invalid or uninitialized unichar id.
static const int INVALID_UNICHAR_ID = -1;
// A special unichar that corresponds to INVALID_UNICHAR_ID.
static const char INVALID_UNICHAR[] = "__INVALID_UNICHAR__";

// The UNICHAR class holds a single classification result. This may be
// a single Unicode character (stored as between 1 and 4 utf8 bytes) or
// multple Unicode characters representing the NFKC expansion of a ligature
// such as fi, ffl etc. These are also stored as utf8.
class UNICHAR {
 public:
  UNICHAR() {
    memset(chars, 0, UNICHAR_LEN);
  }

  // Construct from a utf8 string. If len<0 then the string is null terminated.
  // If the string is too long to fit in the UNICHAR then it takes only what
  // will fit.
  UNICHAR(const char* utf8_str, int len);

  // Construct from a single UCS4 character.
  explicit UNICHAR(int unicode);

  // Default copy constructor and operator= are OK.

  // Get the first character as UCS-4.
  int first_uni() const;

  // Get the length of the UTF8 string.
  int utf8_len() const {
    int len = chars[UNICHAR_LEN - 1];
    return len >=0 && len < UNICHAR_LEN ? len : UNICHAR_LEN;
  }

  // Get a UTF8 string, but NOT NULL terminated.
  const char* utf8() const {
    return chars;
  }

  // Get a terminated UTF8 string: Must delete[] it after use.
  char* utf8_str() const;

  // Get the number of bytes in the first character of the given utf8 string.
  static int utf8_step(const char* utf8_str);

 private:
  // A UTF-8 representation of 1 or more Unicode characters.
  // The last element (chars[UNICHAR_LEN - 1]) is a length if
  // its value < UNICHAR_LEN, otherwise it is a genuine character.
  char chars[UNICHAR_LEN];
};

#endif  // TESSERACT_CCUTIL_UNICHAR_H__
