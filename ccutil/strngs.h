/**********************************************************************
 * File:        strngs.h  (Formerly strings.h)
 * Description: STRING class definition.
 * Author:					Ray Smith
 * Created:					Fri Feb 15 09:15:01 GMT 1991
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

#ifndef           STRNGS_H
#define           STRNGS_H

#include          <string.h>
#include          "memry.h"
#include          "serialis.h"

// STRING_IS_PROTECTED means that  string[index] = X is invalid
// because you have to go through strings interface to modify it.
// This allows the string to ensure internal integrity and maintain
// its own string length. Unfortunately this is not possible because
// STRINGS are used as direct-manipulation data buffers for things
// like length arrays and many places cast away the const on string()
// to mutate the string. Turning this off means that internally we
// cannot assume we know the strlen.
#define STRING_IS_PROTECTED  0

#ifdef CCUTIL_EXPORTS
#define CCUTIL_API __declspec(dllexport)
#elif defined(CCUTIL_IMPORTS)
#define CCUTIL_API __declspec(dllimport)
#else
#define CCUTIL_API
#endif

class CCUTIL_API STRING
{
  public:
    STRING();
    STRING(const STRING &string);
    STRING(const char *string);
    ~STRING ();

    BOOL8 contains(const char c) const;
    inT32 length() const;
    const char *string() const;

#if STRING_IS_PROTECTED
    const char &operator[] (inT32 index) const;
    // len is number of chars in s to insert starting at index in this string
    void insert_range(inT32 index, const char*s, int len);
    void erase_range(inT32 index, int len);
    void truncate_at(inT32 index);
#else
    char &operator[] (inT32 index) const;
#endif

    BOOL8 operator== (const STRING & string) const;
    BOOL8 operator!= (const STRING & string) const;
    BOOL8 operator!= (const char *string) const;

    STRING & operator= (const char *string);
    STRING & operator= (const STRING & string);

    STRING operator+ (const STRING & string) const;
    STRING operator+ (const char ch) const;

    STRING & operator+= (const char *string);
    STRING & operator+= (const STRING & string);
    STRING & operator+= (const char ch);

    // Appends the given string and int (as a %d) to this.
    // += cannot be used for ints as there as a char += operator that would
    // be ambiguous, and ints usually need a string before or between them
    // anyway.
    void add_str_int(const char* str, int number);

    // ensure capcaity but keep pointer encapsulated
    inline void ensure(inT32 min_capacity) { ensure_cstr(min_capacity); }

  private:
    typedef struct STRING_HEADER {
      // How much space was allocated in the string buffer for char data.
      int capacity_;

      // used_ is how much of the capacity is currently being used,
      // including a '\0' terminator.
      //
      // If used_ is 0 then string is NULL (not even the '\0')
      // else if used_ > 0 then it is strlen() + 1 (because it includes '\0')
      // else strlen is >= 0 (not NULL) but needs to be computed.
      //      this condition is set when encapsulation is violated because
      //      an API returned a mutable string.
      //
      // capacity_ - used_ = excess capacity that the string can grow
      //                     without reallocating
      mutable int used_;
    } STRING_HEADER;

    // To preserve the behavior of the old serialization, we only have space
    // for one pointer in this structure. So we are embedding a data structure
    // at the start of the storage that will hold additional state variables,
    // then storing the actual string contents immediately after.
    STRING_HEADER* data_;

    // returns the header part of the storage
    inline STRING_HEADER* GetHeader() {
      return data_;
    }
    inline const STRING_HEADER* GetHeader() const {
      return data_;
    }

    // returns the string data part of storage
    inline char* GetCStr() {
      return ((char *)data_) + sizeof(STRING_HEADER);
    };

    inline const char* GetCStr() const {
      return ((const char *)data_) + sizeof(STRING_HEADER);
    };
    inline bool InvariantOk() const {
#if STRING_IS_PROTECTED
      return (GetHeader()->used_ == 0) ?
        (string() == NULL) : (GetHeader()->used_ == (strlen(string()) + 1));
#else
      return true;
#endif
    }

    // Ensure string has requested capacity as optimization
    // to avoid unnecessary reallocations.
    // The return value is a cstr buffer with at least requested capacity
    char* ensure_cstr(inT32 min_capacity);

    void FixHeader() const;  // make used_ non-negative, even if const

    char* AllocData(int used, int capacity);
    void DiscardData();
};
#endif
