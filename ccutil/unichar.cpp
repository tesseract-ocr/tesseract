///////////////////////////////////////////////////////////////////////
// File:        unichar.cpp
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

#include "unichar.h"

#define UNI_MAX_LEGAL_UTF32 0x0010FFFF

// Construct from a utf8 string. If len<0 then the string is null terminated.
// If the string is too long to fit in the UNICHAR then it takes only what
// will fit. Checks for illegal input and stops at an illegal sequence.
// The resulting UNICHAR may be empty.
UNICHAR::UNICHAR(const char* utf8_str, int len) {
  int total_len = 0;
  int step = 0;
  if (len < 0) {
    for (len = 0; utf8_str[len] != 0 && len < UNICHAR_LEN; ++len);
  }
  for (total_len = 0; total_len < len; total_len += step) {
    step = utf8_step(utf8_str + total_len);
    if (total_len + step > UNICHAR_LEN)
      break;  // Too long.
    if (step == 0)
      break;  // Illegal first byte.
    int i;
    for (i = 1; i < step; ++i)
      if ((utf8_str[total_len + i] & 0xc0) != 0x80)
        break;
    if (i < step)
      break;  // Illegal surrogate
  }
  memcpy(chars, utf8_str, total_len);
  if (total_len < UNICHAR_LEN) {
    chars[UNICHAR_LEN - 1] = total_len;
    while (total_len < UNICHAR_LEN - 1)
      chars[total_len++] = 0;
  }
}

// Construct from a single UCS4 character. Illegal values are ignored,
// resulting in an empty UNICHAR.
UNICHAR::UNICHAR(int unicode) {
  const int bytemask = 0xBF;
  const int bytemark = 0x80;

  if (unicode < 0x80) {
    chars[UNICHAR_LEN - 1] = 1;
    chars[2] = 0;
    chars[1] = 0;
    chars[0] = static_cast<char>(unicode);
  } else if (unicode < 0x800) {
    chars[UNICHAR_LEN - 1] = 2;
    chars[2] = 0;
    chars[1] = static_cast<char>((unicode | bytemark) & bytemask);
    unicode >>= 6;
    chars[0] = static_cast<char>(unicode | 0xc0);
  } else if (unicode < 0x10000) {
    chars[UNICHAR_LEN - 1] = 3;
    chars[2] = static_cast<char>((unicode | bytemark) & bytemask);
    unicode >>= 6;
    chars[1] = static_cast<char>((unicode | bytemark) & bytemask);
    unicode >>= 6;
    chars[0] = static_cast<char>(unicode | 0xe0);
  } else if (unicode <= UNI_MAX_LEGAL_UTF32) {
    chars[UNICHAR_LEN - 1] = 4;
    chars[3] = static_cast<char>((unicode | bytemark) & bytemask);
    unicode >>= 6;
    chars[2] = static_cast<char>((unicode | bytemark) & bytemask);
    unicode >>= 6;
    chars[1] = static_cast<char>((unicode | bytemark) & bytemask);
    unicode >>= 6;
    chars[0] = static_cast<char>(unicode | 0xf0);
  } else {
    memset(chars, 0, UNICHAR_LEN);
  }
}

// Get the first character as UCS-4.
int UNICHAR::first_uni() const {
  static const int utf8_offsets[5] = {
    0, 0, 0x3080, 0xE2080, 0x3C82080
  };
  int uni = 0;
  int len = utf8_step(chars);
  const char* src = chars;

  switch (len) {
  default:
    break;
  case 4:
    uni += static_cast<unsigned char>(*src++);
    uni <<= 6;
  case 3:
    uni += static_cast<unsigned char>(*src++);
    uni <<= 6;
  case 2:
    uni += static_cast<unsigned char>(*src++);
    uni <<= 6;
  case 1:
    uni += static_cast<unsigned char>(*src++);
  }
  uni -= utf8_offsets[len];
  return uni;
}

// Get a terminated UTF8 string: Must delete[] it after use.
char* UNICHAR::utf8_str() const {
  int len = utf8_len();
  char* str = new char[len + 1];
  memcpy(str, chars, len);
  str[len] = 0;
  return str;
}

// Get the number of bytes in the first character of the given utf8 string.
int UNICHAR::utf8_step(const char* utf8_str) {
  static const char utf8_bytes[256] = {
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
    3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3, 4,4,4,4,4,4,4,4,0,0,0,0,0,0,0,0
  };

  return utf8_bytes[static_cast<unsigned char>(*utf8_str)];
}
