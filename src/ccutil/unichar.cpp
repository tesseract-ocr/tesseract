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
#include "errcode.h"
#include "genericvector.h"
#include "tprintf.h"

#define UNI_MAX_LEGAL_UTF32 0x0010FFFF

namespace tesseract {

// Construct from a utf8 string. If len<0 then the string is null terminated.
// If the string is too long to fit in the UNICHAR then it takes only what
// will fit. Checks for illegal input and stops at an illegal sequence.
// The resulting UNICHAR may be empty.
UNICHAR::UNICHAR(const char* utf8_str, int len) {
  int total_len = 0;
  int step = 0;
  if (len < 0) {
    for (len = 0; len < UNICHAR_LEN && utf8_str[len] != 0; ++len);
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

UNICHAR::const_iterator& UNICHAR::const_iterator::operator++() {
  ASSERT_HOST(it_ != nullptr);
  int step = utf8_step(it_);
  if (step == 0) {
    tprintf("ERROR: Illegal UTF8 encountered.\n");
    for (int i = 0; i < 5 && it_[i] != '\0'; ++i) {
      tprintf("Index %d char = 0x%x\n", i, it_[i]);
    }
    step = 1;
  }
  it_ += step;
  return *this;
}

int UNICHAR::const_iterator::operator*() const {
  ASSERT_HOST(it_ != nullptr);
  const int len = utf8_step(it_);
  if (len == 0) {
    tprintf("WARNING: Illegal UTF8 encountered\n");
    return ' ';
  }
  UNICHAR uch(it_, len);
  return uch.first_uni();
}

int UNICHAR::const_iterator::get_utf8(char* utf8_output) const {
  ASSERT_HOST(it_ != nullptr);
  const int len = utf8_step(it_);
  if (len == 0) {
    tprintf("WARNING: Illegal UTF8 encountered\n");
    utf8_output[0] = ' ';
    return 1;
  }
  strncpy(utf8_output, it_, len);
  return len;
}

int UNICHAR::const_iterator::utf8_len() const {
  ASSERT_HOST(it_ != nullptr);
  const int len = utf8_step(it_);
  if (len == 0) {
    tprintf("WARNING: Illegal UTF8 encountered\n");
    return 1;
  }
  return len;
}

bool UNICHAR::const_iterator::is_legal() const {
  return utf8_step(it_) > 0;
}

UNICHAR::const_iterator UNICHAR::begin(const char* utf8_str, const int len) {
  return UNICHAR::const_iterator(utf8_str);
}

UNICHAR::const_iterator UNICHAR::end(const char* utf8_str, const int len) {
  return UNICHAR::const_iterator(utf8_str + len);
}

// Converts a utf-8 string to a vector of unicodes.
// Returns an empty vector if the input contains invalid UTF-8.
/* static */
std::vector<char32> UNICHAR::UTF8ToUTF32(const char* utf8_str) {
  const int utf8_length = strlen(utf8_str);
  std::vector<char32> unicodes;
  unicodes.reserve(utf8_length);
  const_iterator end_it(end(utf8_str, utf8_length));
  for (const_iterator it(begin(utf8_str, utf8_length)); it != end_it; ++it) {
    if (it.is_legal()) {
      unicodes.push_back(*it);
    } else {
      unicodes.clear();
      return unicodes;
    }
  }
  return unicodes;
}

// Returns an empty string if the input contains an invalid unicode.
std::string UNICHAR::UTF32ToUTF8(const std::vector<char32>& str32) {
  std::string utf8_str;
  for (char32 ch : str32) {
    UNICHAR uni_ch(ch);
    int step;
    if (uni_ch.utf8_len() > 0 && (step = utf8_step(uni_ch.utf8())) > 0) {
      utf8_str.append(uni_ch.utf8(), step);
    } else {
      return "";
    }
  }
  return utf8_str;
}

}  // namespace tesseract
