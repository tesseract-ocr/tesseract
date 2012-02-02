// Copyright 2011 Google Inc. All Rights Reserved.
// Author: rays@google.com (Ray Smith)
///////////////////////////////////////////////////////////////////////
// File:        bitvector.cpp
// Description: Class replacement for BITVECTOR.
// Author:      Ray Smith
// Created:     Mon Jan 10 17:45:01 PST 2011
//
// (C) Copyright 2011, Google Inc.
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

#include "bitvector.h"
#include <string.h>
#include "helpers.h"

namespace tesseract {

BitVector::BitVector() : bit_size_(0), array_(NULL) {}

BitVector::BitVector(int length) : bit_size_(length) {
  array_ = new uinT32[WordLength()];
  SetAllFalse();
}

BitVector::BitVector(const BitVector& src) : bit_size_(src.bit_size_) {
  array_ = new uinT32[WordLength()];
  memcpy(array_, src.array_, ByteLength());
}

BitVector& BitVector::operator=(const BitVector& src) {
  Alloc(src.bit_size_);
  memcpy(array_, src.array_, ByteLength());
  return *this;
}

BitVector::~BitVector() {
  delete [] array_;
}

// Initializes the array to length * false.
void BitVector::Init(int length) {
  Alloc(length);
  SetAllFalse();
}

// Writes to the given file. Returns false in case of error.
bool BitVector::Serialize(FILE* fp) const {
  if (fwrite(&bit_size_, sizeof(bit_size_), 1, fp) != 1) return false;
  int wordlen = WordLength();
  if (fwrite(array_, sizeof(*array_), wordlen, fp) != wordlen) return false;
  return true;
}

// Reads from the given file. Returns false in case of error.
// If swap is true, assumes a big/little-endian swap is needed.
bool BitVector::DeSerialize(bool swap, FILE* fp) {
  uinT32 new_bit_size;
  if (fread(&new_bit_size, sizeof(new_bit_size), 1, fp) != 1) return false;
  if (swap) {
    ReverseN(&new_bit_size, sizeof(new_bit_size));
  }
  Alloc(new_bit_size);
  int wordlen = WordLength();
  if (fread(array_, sizeof(*array_), wordlen, fp) != wordlen) return false;
  if (swap) {
    for (int i = 0; i < wordlen; ++i)
      ReverseN(&array_[i], sizeof(array_[i]));
  }
  return true;
}

void BitVector::SetAllFalse() {
  memset(array_, 0, ByteLength());
}
void BitVector::SetAllTrue() {
  memset(array_, ~0, ByteLength());
}

// Allocates memory for a vector of the given length.
// Reallocates if the array is a different size, larger or smaller.
void BitVector::Alloc(int length) {
  int initial_wordlength = WordLength();
  bit_size_ = length;
  int new_wordlength = WordLength();
  if (new_wordlength != initial_wordlength) {
    delete [] array_;
    array_ = new uinT32[new_wordlength];
  }
}


}  // namespace tesseract.


