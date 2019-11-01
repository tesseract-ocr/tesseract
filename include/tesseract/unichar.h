///////////////////////////////////////////////////////////////////////
// File:        unichar.h
// Description: Unicode character/ligature class.
// Author:      Ray Smith
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

#ifndef TESSERACT_CCUTIL_UNICHAR_H_
#define TESSERACT_CCUTIL_UNICHAR_H_

#include <memory.h>

#include <cstring>
#include <string>
#include <vector>

#include "platform.h"

// Maximum number of characters that can be stored in a UNICHAR. Must be
// at least 4. Must not exceed 31 without changing the coding of length.
#define UNICHAR_LEN 30

// TODO(rays) Move these to the tesseract namespace.
// A UNICHAR_ID is the unique id of a unichar.
using UNICHAR_ID = int;

// A variable to indicate an invalid or uninitialized unichar id.
static const int INVALID_UNICHAR_ID = -1;
// A special unichar that corresponds to INVALID_UNICHAR_ID.
static const char INVALID_UNICHAR[] = "__INVALID_UNICHAR__";

enum StrongScriptDirection {
  DIR_NEUTRAL = 0,        // Text contains only neutral characters.
  DIR_LEFT_TO_RIGHT = 1,  // Text contains no Right-to-Left characters.
  DIR_RIGHT_TO_LEFT = 2,  // Text contains no Left-to-Right characters.
  DIR_MIX = 3,            // Text contains a mixture of left-to-right
                          // and right-to-left characters.
};

namespace tesseract {

using char32 = signed int;

// The UNICHAR class holds a single classification result. This may be
// a single Unicode character (stored as between 1 and 4 utf8 bytes) or
// multiple Unicode characters representing the NFKC expansion of a ligature
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
    return len >= 0 && len < UNICHAR_LEN ? len : UNICHAR_LEN;
  }

  // Get a UTF8 string, but NOT nullptr terminated.
  const char* utf8() const {
    return chars;
  }

  // Get a terminated UTF8 string: Must delete[] it after use.
  char* utf8_str() const;

  // Get the number of bytes in the first character of the given utf8 string.
  static int utf8_step(const char* utf8_str);

  // A class to simplify iterating over and accessing elements of a UTF8
  // string. Note that unlike the UNICHAR class, const_iterator does NOT COPY or
  // take ownership of the underlying byte array. It also does not permit
  // modification of the array (as the name suggests).
  //
  // Example:
  //   for (UNICHAR::const_iterator it = UNICHAR::begin(str, str_len);
  //        it != UNICHAR::end(str, len);
  //        ++it) {
  //     tprintf("UCS-4 symbol code = %d\n", *it);
  //     char buf[5];
  //     int char_len = it.get_utf8(buf); buf[char_len] = '\0';
  //     tprintf("Char = %s\n", buf);
  //   }
  class const_iterator {
    using CI = const_iterator;

   public:
    // Step to the next UTF8 character.
    // If the current position is at an illegal UTF8 character, then print an
    // error message and step by one byte. If the current position is at a
    // nullptr value, don't step past it.
    const_iterator& operator++();

    // Return the UCS-4 value at the current position.
    // If the current position is at an illegal UTF8 value, return a single
    // space character.
    int operator*() const;

    // Store the UTF-8 encoding of the current codepoint into buf, which must be
    // at least 4 bytes long. Return the number of bytes written.
    // If the current position is at an illegal UTF8 value, writes a single
    // space character and returns 1.
    // Note that this method does not null-terminate the buffer.
    int get_utf8(char* buf) const;
    // Returns the number of bytes of the current codepoint. Returns 1 if the
    // current position is at an illegal UTF8 value.
    int utf8_len() const;
    // Returns true if the UTF-8 encoding at the current position is legal.
    bool is_legal() const;

    // Return the pointer into the string at the current position.
    const char* utf8_data() const {
      return it_;
    }

    // Iterator equality operators.
    friend bool operator==(const CI& lhs, const CI& rhs) {
      return lhs.it_ == rhs.it_;
    }
    friend bool operator!=(const CI& lhs, const CI& rhs) {
      return !(lhs == rhs);
    }

   private:
    friend class UNICHAR;
    explicit const_iterator(const char* it) : it_(it) {}

    const char* it_;  // Pointer into the string.
  };

  // Create a start/end iterator pointing to a string. Note that these methods
  // are static and do NOT create a copy or take ownership of the underlying
  // array.
  static const_iterator begin(const char* utf8_str, int byte_length);
  static const_iterator end(const char* utf8_str, int byte_length);

  // Converts a utf-8 string to a vector of unicodes.
  // Returns an empty vector if the input contains invalid UTF-8.
  static std::vector<char32> UTF8ToUTF32(const char* utf8_str);
  // Converts a vector of unicodes to a utf8 string.
  // Returns an empty string if the input contains an invalid unicode.
  static std::string UTF32ToUTF8(const std::vector<char32>& str32);

 private:
  // A UTF-8 representation of 1 or more Unicode characters.
  // The last element (chars[UNICHAR_LEN - 1]) is a length if
  // its value < UNICHAR_LEN, otherwise it is a genuine character.
  char chars[UNICHAR_LEN]{};
};

}  // namespace tesseract

#endif  // TESSERACT_CCUTIL_UNICHAR_H_
