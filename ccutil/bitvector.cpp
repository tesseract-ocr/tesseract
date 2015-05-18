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
#include "ndminx.h"

namespace tesseract {

// Fast lookup table to get the first least significant set bit in a byte.
// For zero, the table has 255, but since it is a special case, most code
// that uses this table will check for zero before looking up lsb_index_.
const uinT8 BitVector::lsb_index_[256] = {
  255, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
  4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
  5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
  4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
  6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
  4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
  5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
  4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
  7, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
  4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
  5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
  4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
  6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
  4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
  5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
  4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0
};

// Fast lookup table to get the residual bits after zeroing the first (lowest)
// set bit in a byte.
const uinT8 BitVector::lsb_eroded_[256] = {
  0, 0, 0, 0x2, 0, 0x4, 0x4, 0x6,
  0, 0x8, 0x8, 0x0a, 0x08, 0x0c, 0x0c, 0x0e,
  0, 0x10, 0x10, 0x12, 0x10, 0x14, 0x14, 0x16,
  0x10, 0x18, 0x18, 0x1a, 0x18, 0x1c, 0x1c, 0x1e,
  0, 0x20, 0x20, 0x22, 0x20, 0x24, 0x24, 0x26,
  0x20, 0x28, 0x28, 0x2a, 0x28, 0x2c, 0x2c, 0x2e,
  0x20, 0x30, 0x30, 0x32, 0x30, 0x34, 0x34, 0x36,
  0x30, 0x38, 0x38, 0x3a, 0x38, 0x3c, 0x3c, 0x3e,
  0, 0x40, 0x40, 0x42, 0x40, 0x44, 0x44, 0x46,
  0x40, 0x48, 0x48, 0x4a, 0x48, 0x4c, 0x4c, 0x4e,
  0x40, 0x50, 0x50, 0x52, 0x50, 0x54, 0x54, 0x56,
  0x50, 0x58, 0x58, 0x5a, 0x58, 0x5c, 0x5c, 0x5e,
  0x40, 0x60, 0x60, 0x62, 0x60, 0x64, 0x64, 0x66,
  0x60, 0x68, 0x68, 0x6a, 0x68, 0x6c, 0x6c, 0x6e,
  0x60, 0x70, 0x70, 0x72, 0x70, 0x74, 0x74, 0x76,
  0x70, 0x78, 0x78, 0x7a, 0x78, 0x7c, 0x7c, 0x7e,
  0, 0x80, 0x80, 0x82, 0x80, 0x84, 0x84, 0x86,
  0x80, 0x88, 0x88, 0x8a, 0x88, 0x8c, 0x8c, 0x8e,
  0x80, 0x90, 0x90, 0x92, 0x90, 0x94, 0x94, 0x96,
  0x90, 0x98, 0x98, 0x9a, 0x98, 0x9c, 0x9c, 0x9e,
  0x80, 0xa0, 0xa0, 0xa2, 0xa0, 0xa4, 0xa4, 0xa6,
  0xa0, 0xa8, 0xa8, 0xaa, 0xa8, 0xac, 0xac, 0xae,
  0xa0, 0xb0, 0xb0, 0xb2, 0xb0, 0xb4, 0xb4, 0xb6,
  0xb0, 0xb8, 0xb8, 0xba, 0xb8, 0xbc, 0xbc, 0xbe,
  0x80, 0xc0, 0xc0, 0xc2, 0xc0, 0xc4, 0xc4, 0xc6,
  0xc0, 0xc8, 0xc8, 0xca, 0xc8, 0xcc, 0xcc, 0xce,
  0xc0, 0xd0, 0xd0, 0xd2, 0xd0, 0xd4, 0xd4, 0xd6,
  0xd0, 0xd8, 0xd8, 0xda, 0xd8, 0xdc, 0xdc, 0xde,
  0xc0, 0xe0, 0xe0, 0xe2, 0xe0, 0xe4, 0xe4, 0xe6,
  0xe0, 0xe8, 0xe8, 0xea, 0xe8, 0xec, 0xec, 0xee,
  0xe0, 0xf0, 0xf0, 0xf2, 0xf0, 0xf4, 0xf4, 0xf6,
  0xf0, 0xf8, 0xf8, 0xfa, 0xf8, 0xfc, 0xfc, 0xfe
};

// Fast lookup table to give the number of set bits in a byte.
const int BitVector::hamming_table_[256] = {
    0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8
};


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
  if (static_cast<int>(fwrite(array_, sizeof(*array_), wordlen, fp)) != wordlen)
      return false;
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
  if (static_cast<int>(fread(array_, sizeof(*array_), wordlen, fp)) != wordlen)
      return false;
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

// Returns the index of the next set bit after the given index.
// Useful for quickly iterating through the set bits in a sparse vector.
int BitVector::NextSetBit(int prev_bit) const {
  // Move on to the next bit.
  int next_bit = prev_bit + 1;
  if (next_bit >= bit_size_) return -1;
  // Check the remains of the word containing the next_bit first.
  int next_word = WordIndex(next_bit);
  int bit_index = next_word * kBitFactor;
  int word_end = bit_index + kBitFactor;
  uinT32 word = array_[next_word];
  uinT8 byte = word & 0xff;
  while (bit_index < word_end) {
    if (bit_index + 8 > next_bit && byte != 0) {
      while (bit_index + lsb_index_[byte] < next_bit && byte != 0)
        byte = lsb_eroded_[byte];
      if (byte != 0)
        return bit_index + lsb_index_[byte];
    }
    word >>= 8;
    bit_index += 8;
    byte = word & 0xff;
  }
  // next_word didn't contain a 1, so find the next word with set bit.
  ++next_word;
  int wordlen = WordLength();
  while (next_word < wordlen && (word = array_[next_word]) == 0) {
    ++next_word;
    bit_index += kBitFactor;
  }
  if (bit_index >= bit_size_) return -1;
  // Find the first non-zero byte within the word.
  while ((word & 0xff) == 0) {
    word >>= 8;
    bit_index += 8;
  }
  return bit_index + lsb_index_[word & 0xff];
}

// Returns the number of set bits in the vector.
int BitVector::NumSetBits() const {
  int wordlen = WordLength();
  int total_bits = 0;
  for (int w = 0; w < wordlen; ++w) {
    uinT32 word = array_[w];
    for (int i = 0; i < 4; ++i) {
      total_bits += hamming_table_[word & 0xff];
      word >>= 8;
    }
  }
  return total_bits;
}

// Logical in-place operations on whole bit vectors. Tries to do something
// sensible if they aren't the same size, but they should be really.
void BitVector::operator|=(const BitVector& other) {
  int length = MIN(WordLength(), other.WordLength());
  for (int w = 0; w < length; ++w)
    array_[w] |= other.array_[w];
}
void BitVector::operator&=(const BitVector& other) {
  int length = MIN(WordLength(), other.WordLength());
  for (int w = 0; w < length; ++w)
    array_[w] &= other.array_[w];
  for (int w = WordLength() - 1; w >= length; --w)
    array_[w] = 0;
}
void BitVector::operator^=(const BitVector& other) {
  int length = MIN(WordLength(), other.WordLength());
  for (int w = 0; w < length; ++w)
    array_[w] ^= other.array_[w];
}
// Set subtraction *this = v1 - v2.
void BitVector::SetSubtract(const BitVector& v1, const BitVector& v2) {
  Alloc(v1.size());
  int length = MIN(v1.WordLength(), v2.WordLength());
  for (int w = 0; w < length; ++w)
    array_[w] = v1.array_[w] ^ (v1.array_[w] & v2.array_[w]);
  for (int w = WordLength() - 1; w >= length; --w)
    array_[w] = v1.array_[w];
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


