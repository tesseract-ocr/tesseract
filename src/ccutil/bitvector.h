///////////////////////////////////////////////////////////////////////
// File:        bitvector.h
// Description: Class replacement for BITVECTOR.
// Author:      Ray Smith
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

#ifndef TESSERACT_CCUTIL_BITVECTOR_H_
#define TESSERACT_CCUTIL_BITVECTOR_H_

#include <tesseract/export.h>

#include <cassert>
#include <cstdint> // for uint8_t
#include <cstdio>
#include <vector>  // for std::vector

namespace tesseract {

// Trivial class to encapsulate a fixed-length array of bits, with
// Serialize/DeSerialize. Replaces the old macros.
class TESS_API BitVector {
public:
  // Fast lookup table to get the first least significant set bit in a byte.
  // For zero, the table has 255, but since it is a special case, most code
  // that uses this table will check for zero before looking up lsb_index_.
  static const uint8_t lsb_index_[256];
  // Fast lookup table to get the residual bits after zeroing the least
  // significant set bit in a byte.
  static const uint8_t lsb_eroded_[256];
  // Fast lookup table to give the number of set bits in a byte.
  static const int hamming_table_[256];

  BitVector() = default;
  // Initializes the array to length * false.
  explicit BitVector(int length) : bit_size_(length), array_(WordLength()) {
  }
  BitVector(const BitVector &src) : bit_size_(src.bit_size_), array_(src.array_) {
  }
  BitVector &operator=(const BitVector &src);
  ~BitVector() = default;

  // Initializes the array to length * false.
  void Init(int length);

  int empty() const {
    return bit_size_ == 0;
  }

  // Returns the number of bits that are accessible in the vector.
  int size() const {
    return bit_size_;
  }

  // Writes to the given file. Returns false in case of error.
  bool Serialize(FILE *fp) const;
  // Reads from the given file. Returns false in case of error.
  // If swap is true, assumes a big/little-endian swap is needed.
  bool DeSerialize(bool swap, FILE *fp);

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
    if (value) {
      SetBit(index);
    } else {
      ResetBit(index);
    }
  }
  bool At(int index) const {
    return (array_[WordIndex(index)] & BitMask(index)) != 0;
  }
  bool operator[](int index) const {
    return (array_[WordIndex(index)] & BitMask(index)) != 0;
  }

  // Returns the index of the next set bit after the given index.
  // Useful for quickly iterating through the set bits in a sparse vector.
  int NextSetBit(int prev_bit) const;

  // Returns the number of set bits in the vector.
  int NumSetBits() const;

  // Logical in-place operations on whole bit vectors. Tries to do something
  // sensible if they aren't the same size, but they should be really.
  void operator|=(const BitVector &other);
  void operator&=(const BitVector &other);
  void operator^=(const BitVector &other);
  // Set subtraction *this = v1 - v2.
  void SetSubtract(const BitVector &v1, const BitVector &v2);

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
  uint32_t BitMask(int index) const {
    return 1 << (index & (kBitFactor - 1));
  }
  // Returns the number of array elements needed to represent the current
  // bit_size_.
  int WordLength() const {
    return (bit_size_ + kBitFactor - 1) / kBitFactor;
  }
  // Returns the number of bytes consumed by the array_.
  int ByteLength() const {
    return WordLength() * sizeof(array_[0]);
  }

  // Number of bits in this BitVector.
  int32_t bit_size_ = 0;
  // Array of words used to pack the bits.
  // Bits are stored little-endian by uint32_t word, ie by word first and then
  // starting with the least significant bit in each word.
  std::vector<uint32_t> array_;
  // Number of bits in an array_ element.
  static const int kBitFactor = sizeof(array_[0]) * 8;
};

} // namespace tesseract.

#endif // TESSERACT_CCUTIL_BITVECTOR_H_
