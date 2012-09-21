/**********************************************************************
 * File:        strngs.c  (Formerly strings.c)
 * Description: STRING class functions.
 * Author:					Ray Smith
 * Created:					Fri Feb 15 09:13:30 GMT 1991
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

#include          "mfcpch.h"     // Precompiled headers
#include          "helpers.h"
#include          "tprintf.h"
#include          "strngs.h"
#include          "genericvector.h"

#include <assert.h>
// Size of buffer needed to host the decimal representation of the maximum
// possible length of an int (in 64 bits, being -<20 digits>.
const int kMaxIntSize = 22;

/**********************************************************************
 * STRING_HEADER provides metadata about the allocated buffer,
 * including total capacity and how much used (strlen with '\0').
 *
 * The implementation hides this header at the start of the data
 * buffer and appends the string on the end to keep sizeof(STRING)
 * unchanged from earlier versions so serialization is not affected.
 *
 * The collection of MACROS provide different implementations depending
 * on whether the string keeps track of its strlen or not so that this
 * feature can be added in later when consumers dont modifify the string
 **********************************************************************/

// Smallest string to allocate by default
const int kMinCapacity = 16;

char* STRING::AllocData(int used, int capacity) {
  data_ = (STRING_HEADER *)alloc_string(capacity + sizeof(STRING_HEADER));

  // header is the metadata for this memory block
  STRING_HEADER* header = GetHeader();
  header->capacity_ = capacity;
  header->used_ = used;
  return GetCStr();
}

void STRING::DiscardData() {
  free_string((char *)data_);
}

// This is a private method; ensure FixHeader is called (or used_ is well defined)
// beforehand
char* STRING::ensure_cstr(inT32 min_capacity) {
  STRING_HEADER* orig_header = GetHeader();
  if (min_capacity <= orig_header->capacity_)
    return ((char *)this->data_) + sizeof(STRING_HEADER);

  // if we are going to grow bigger, than double our existing
  // size, but if that still is not big enough then keep the
  // requested capacity
  if (min_capacity < 2 * orig_header->capacity_)
    min_capacity = 2 * orig_header->capacity_;

  int alloc = sizeof(STRING_HEADER) + min_capacity;
  STRING_HEADER* new_header = (STRING_HEADER*)(alloc_string(alloc));

  memcpy(&new_header[1], GetCStr(), orig_header->used_);
  new_header->capacity_ = min_capacity;
  new_header->used_ = orig_header->used_;

  // free old memory, then rebind to new memory
  DiscardData();
  data_ = new_header;

  assert(InvariantOk());
  return ((char *)data_) + sizeof(STRING_HEADER);
}

// This is const, but is modifying a mutable field
// this way it can be used on const or non-const instances.
void STRING::FixHeader() const {
  const STRING_HEADER* header = GetHeader();
  if (header->used_ < 0)
    header->used_ = strlen(GetCStr()) + 1;
}


STRING::STRING() {
  // Empty STRINGs contain just the "\0".
  memcpy(AllocData(1, kMinCapacity), "", 1);
}

STRING::STRING(const STRING& str) {
  str.FixHeader();
  const STRING_HEADER* str_header  = str.GetHeader();
  int   str_used  = str_header->used_;
  char *this_cstr = AllocData(str_used, str_used);
  memcpy(this_cstr, str.GetCStr(), str_used);
  assert(InvariantOk());
}

STRING::STRING(const char* cstr) {
  if (cstr == NULL) {
    // Empty STRINGs contain just the "\0".
    memcpy(AllocData(1, kMinCapacity), "", 1);
  } else {
    int len = strlen(cstr) + 1;
    char* this_cstr = AllocData(len, len);
    memcpy(this_cstr, cstr, len);
  }
  assert(InvariantOk());
}

STRING::~STRING() {
  DiscardData();
}

// Writes to the given file. Returns false in case of error.
bool STRING::Serialize(FILE* fp) const {
  inT32 len = length();
  if (fwrite(&len, sizeof(len), 1, fp) != 1) return false;
  if (fwrite(GetCStr(), 1, len, fp) != len) return false;
  return true;
}
// Reads from the given file. Returns false in case of error.
// If swap is true, assumes a big/little-endian swap is needed.
bool STRING::DeSerialize(bool swap, FILE* fp) {
  inT32 len;
  if (fread(&len, sizeof(len), 1, fp) != 1) return false;
  if (swap)
    ReverseN(&len, sizeof(len));
  truncate_at(len);
  if (fread(GetCStr(), 1, len, fp) != len) return false;
  return true;
}

BOOL8 STRING::contains(const char c) const {
  return (c != '\0') && (strchr (GetCStr(), c) != NULL);
}

inT32 STRING::length() const {
  FixHeader();
  return GetHeader()->used_ - 1;
}

const char* STRING::string() const {
  const STRING_HEADER* header = GetHeader();
  if (header->used_ == 0)
    return NULL;

  // mark header length unreliable because tesseract might
  // cast away the const and mutate the string directly.
  header->used_ = -1;
  return GetCStr();
}

/******
 * The STRING_IS_PROTECTED interface adds additional support to migrate
 * code that needs to modify the STRING in ways not otherwise supported
 * without violating encapsulation.
 *
 * Also makes the [] operator return a const so it is immutable
 */
#if STRING_IS_PROTECTED
const char& STRING::operator[](inT32 index) const {
  return GetCStr()[index];
}

void STRING::insert_range(inT32 index, const char* str, int len) {
  // if index is outside current range, then also grow size of string
  // to accmodate the requested range.
  STRING_HEADER* this_header = GetHeader();
  int used = this_header->used_;
  if (index > used)
    used = index;

  char* this_cstr = ensure_cstr(used + len + 1);
  if (index < used) {
    // move existing string from index to '\0' inclusive.
    memmove(this_cstr + index + len,
           this_cstr + index,
           this_header->used_ - index);
  } else if (len > 0) {
    // We are going to overwrite previous null terminator, so write the new one.
    this_cstr[this_header->used_ + len - 1] = '\0';

    // If the old header did not have the terminator,
    // then we need to account for it now that we've added it.
    // Otherwise it was already accounted for; we just moved it.
    if (this_header->used_ == 0)
      ++this_header->used_;
  }

  // Write new string to index.
  // The string is already terminated from the conditions above.
  memcpy(this_cstr + index, str, len);
  this_header->used_ += len;

  assert(InvariantOk());
}

void STRING::erase_range(inT32 index, int len) {
  char* this_cstr = GetCStr();
  STRING_HEADER* this_header = GetHeader();

  memcpy(this_cstr+index, this_cstr+index+len,
         this_header->used_ - index - len);
  this_header->used_ -= len;
  assert(InvariantOk());
}

#else
void STRING::truncate_at(inT32 index) {
  char* this_cstr = ensure_cstr(index + 1);
  this_cstr[index] = '\0';
  GetHeader()->used_ = index + 1;
  assert(InvariantOk());
}

char& STRING::operator[](inT32 index) const {
  // Code is casting away this const and mutating the string,
  // so mark used_ as -1 to flag it unreliable.
  GetHeader()->used_ = -1;
  return ((char *)GetCStr())[index];
}
#endif

void STRING::split(const char c, GenericVector<STRING> *splited) {
  int start_index = 0;
  for (int i = 0; i < length(); i++) {
    if ((*this)[i] == c) {
      if (i != start_index) {
        (*this)[i] = '\0';
        STRING tmp = GetCStr() + start_index;
        splited->push_back(tmp);
        (*this)[i] = c;
      }
      start_index = i + 1;
    }
  }

  if (length() != start_index) {
    STRING tmp = GetCStr() + start_index;
    splited->push_back(tmp);
  }
}

BOOL8 STRING::operator==(const STRING& str) const {
  FixHeader();
  str.FixHeader();
  const STRING_HEADER* str_header = str.GetHeader();
  const STRING_HEADER* this_header = GetHeader();
  int this_used = this_header->used_;
  int str_used  = str_header->used_;

  return (this_used == str_used)
          && (memcmp(GetCStr(), str.GetCStr(), this_used) == 0);
}

BOOL8 STRING::operator!=(const STRING& str) const {
  FixHeader();
  str.FixHeader();
  const STRING_HEADER* str_header = str.GetHeader();
  const STRING_HEADER* this_header = GetHeader();
  int this_used = this_header->used_;
  int str_used  = str_header->used_;

  return (this_used != str_used)
         || (memcmp(GetCStr(), str.GetCStr(), this_used) != 0);
}

BOOL8 STRING::operator!=(const char* cstr) const {
  FixHeader();
  const STRING_HEADER* this_header = GetHeader();

  if (cstr == NULL)
    return this_header->used_ > 1;  // either '\0' or NULL
  else {
    inT32 length = strlen(cstr) + 1;
    return (this_header->used_ != length)
            || (memcmp(GetCStr(), cstr, length) != 0);
  }
}

STRING& STRING::operator=(const STRING& str) {
  str.FixHeader();
  const STRING_HEADER* str_header = str.GetHeader();
  int   str_used = str_header->used_;

  GetHeader()->used_ = 0;  // clear since ensure doesnt need to copy data
  char* this_cstr = ensure_cstr(str_used);
  STRING_HEADER* this_header = GetHeader();

  memcpy(this_cstr, str.GetCStr(), str_used);
  this_header->used_ = str_used;

  assert(InvariantOk());
  return *this;
}

STRING & STRING::operator+=(const STRING& str) {
  FixHeader();
  str.FixHeader();
  const STRING_HEADER* str_header = str.GetHeader();
  const char* str_cstr = str.GetCStr();
  int  str_used  = str_header->used_;
  int  this_used = GetHeader()->used_;
  char* this_cstr = ensure_cstr(this_used + str_used);

  STRING_HEADER* this_header = GetHeader();  // after ensure for realloc

  if (this_used > 1) {
    memcpy(this_cstr + this_used - 1, str_cstr, str_used);
    this_header->used_ += str_used - 1;  // overwrite '\0'
  } else {
    memcpy(this_cstr, str_cstr, str_used);
    this_header->used_ = str_used;
  }

  assert(InvariantOk());
  return *this;
}

void STRING::add_str_int(const char* str, int number) {
  if (str != NULL)
    *this += str;
  // Allow space for the maximum possible length of inT64.
  char num_buffer[kMaxIntSize];
  snprintf(num_buffer, kMaxIntSize - 1, "%d", number);
  num_buffer[kMaxIntSize - 1] = '\0';
  *this += num_buffer;
}

STRING & STRING::operator=(const char* cstr) {
  STRING_HEADER* this_header = GetHeader();
  if (cstr) {
    int len = strlen(cstr) + 1;

    this_header->used_ = 0;  // dont bother copying data if need to realloc
    char* this_cstr = ensure_cstr(len);
    this_header = GetHeader();  // for realloc
    memcpy(this_cstr, cstr, len);
    this_header->used_ = len;
  } else {
    // Reallocate to same state as default constructor.
    DiscardData();
    // Empty STRINGs contain just the "\0".
    memcpy(AllocData(1, kMinCapacity), "", 1);
  }

  assert(InvariantOk());
  return *this;
}

void STRING::assign(const char *cstr, int len) {
  STRING_HEADER* this_header = GetHeader();
  this_header->used_ = 0;  // dont bother copying data if need to realloc
  char* this_cstr = ensure_cstr(len + 1);  // +1 for '\0'

  this_header = GetHeader();  // for realloc
  memcpy(this_cstr, cstr, len);
  this_cstr[len] = '\0';
  this_header->used_ = len + 1;

  assert(InvariantOk());
}

STRING STRING::operator+(const STRING& str) const {
  STRING result(*this);
  result += str;

  assert(InvariantOk());
  return result;
}


STRING STRING::operator+(const char ch) const {
  STRING result;
  FixHeader();
  const STRING_HEADER* this_header = GetHeader();
  int this_used = this_header->used_;
  char* result_cstr = result.ensure_cstr(this_used + 1);
  STRING_HEADER* result_header = result.GetHeader();
  int result_used = result_header->used_;

  // copies '\0' but we'll overwrite that
  memcpy(result_cstr, GetCStr(), this_used);
  result_cstr[result_used] = ch;      // overwrite old '\0'
  result_cstr[result_used + 1] = '\0';  // append on '\0'
  ++result_header->used_;

  assert(InvariantOk());
  return result;
}


STRING&  STRING::operator+=(const char *str) {
  if (!str || !*str)  // empty string has no effect
    return *this;

  FixHeader();
  int len = strlen(str) + 1;
  int this_used = GetHeader()->used_;
  char* this_cstr = ensure_cstr(this_used + len);
  STRING_HEADER* this_header = GetHeader();  // after ensure for realloc

  // if we had non-empty string then append overwriting old '\0'
  // otherwise replace
  if (this_used > 0) {
    memcpy(this_cstr + this_used - 1, str, len);
    this_header->used_ += len - 1;
  } else {
    memcpy(this_cstr, str, len);
    this_header->used_ = len;
  }

  assert(InvariantOk());
  return *this;
}


STRING& STRING::operator+=(const char ch) {
  if (ch == '\0')
    return *this;

  FixHeader();
  int   this_used = GetHeader()->used_;
  char* this_cstr = ensure_cstr(this_used + 1);
  STRING_HEADER* this_header = GetHeader();

  if (this_used > 0)
    --this_used; // undo old empty null if there was one

  this_cstr[this_used++] = ch;   // append ch to end
  this_cstr[this_used++] = '\0'; // append '\0' after ch
  this_header->used_ = this_used;

  assert(InvariantOk());
  return *this;
}
