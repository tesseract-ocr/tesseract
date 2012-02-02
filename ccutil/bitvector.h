// Copyright 2011 Google Inc. All Rights Reserved.
// Author: rays@google.com (Ray Smith)
///////////////////////////////////////////////////////////////////////
// File:        bitvector.h
// Description: Class replacement for BITVECTOR.
// Author:      Ray Smith
// Created:     Mon Jan 10 17:44:01 PST 2011
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


#ifndef TESSERACT_CCUTIL_BITVECTOR_H__
#define TESSERACT_CCUTIL_BITVECTOR_H__

#include <assert.h>
#include <stdio.h>
#include "host.h"

namespace tesseract {

// Trivial class to encapsulate a fixed-length array of bits, with
// Serialize/DeSerialize. Replaces the old macros.
class BitVector {
 public:
  BitVector();
  // Initializes the array to length * false.
  explicit BitVector(int length);
  BitVector(const BitVector& src);
  BitVector& operator=(const BitVector& src);
  ~BitVector();

  // Initializes the array to length * false.
  void Init(int length);

  // Returns the number of bits that are accessible in the vector.
  int size() const {
    return bit_size_;
  }

  // Writes to the given file. Returns false in case of error.
  bool Serialize(FILE* fp) const;
  // Reads from the given file. Returns false in case of error.
  // If swap is true, assumes a big/little-endian swap is needed.
  bool DeSerialize(bool swap, FILE* fp);

  void SetAllFalse();
  void SetAllTrue();

  // Accessors to set/reset/get bits.
  // The range of index is [0, size()-1].
  // There is debug-only bounds checking.
  void SetBit(int index) {
    array_[WordIndex(index)] |= BitMask(index);
  }
  void ResetBit(int index) {
    array_[WordIndex(index)] &= ~BitMask(index);
  }
  void SetValue(int index, bool value) {
    if (value)
      SetBit(index);
    else
      ResetBit(index);
  }
  bool At(int index) const {
    return (array_[WordIndex(index)] & BitMask(index)) != 0;
  }
  bool operator[](int index) const {
    return (array_[WordIndex(index)] & BitMask(index)) != 0;
  }

 private:
  // Allocates memory for a vector of the given length.
  void Alloc(int length);

  // Computes the index to array_ for the given index, with debug range
  // checking.
  int WordIndex(int index) const {
    assert(0 <= index && index < bit_size_);
    return index / kBitFactor;
  }
  // Returns a mask to select the appropriate bit for the given index.
  uinT32 BitMask(int index) const {
    return 1 << (index & (kBitFactor - 1));
  }
  // Returns the number of array elements needed to represent the current
  // bit_size_.
  int WordLength() const {
    return (bit_size_ + kBitFactor - 1) / kBitFactor;
  }
  // Returns the number of bytes consumed by the array_.
  int ByteLength() const {
    return WordLength() * sizeof(*array_);
  }

  // Number of bits in this BitVector.
  uinT32 bit_size_;
  // Array of words used to pack the bits.
  uinT32* array_;
  // Number of bits in an array_ element.
  static const int kBitFactor = sizeof(uinT32) * 8;
};

}  // namespace tesseract.

#endif  // TESSERACT_CCUTIL_BITVECTOR_H__
