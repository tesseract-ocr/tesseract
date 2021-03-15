/******************************************************************************
 **    Filename:    bitvec.h
 **    Purpose:     Routines for manipulating bit vectors
 **    Author:      Dan Johnson
 **
 **    (c) Copyright Hewlett-Packard Company, 1988.
 ** Licensed under the Apache License, Version 2.0 (the "License");
 ** you may not use this file except in compliance with the License.
 ** You may obtain a copy of the License at
 ** http://www.apache.org/licenses/LICENSE-2.0
 ** Unless required by applicable law or agreed to in writing, software
 ** distributed under the License is distributed on an "AS IS" BASIS,
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 ** See the License for the specific language governing permissions and
 ** limitations under the License.
 ******************************************************************************/

#ifndef BITVEC_H
#define BITVEC_H

#include <cstddef> // for size_t
#include <cstdint> // for uint32_t

/*-----------------------------------------------------------------------------
          Include Files and Type Defines
-----------------------------------------------------------------------------*/

using BIT_VECTOR = uint32_t *;

//< no of bits in a BIT_VECTOR element
const size_t BITSINLONG = 8 * sizeof(uint32_t);

/*-----------------------------------------------------------------------------
          Public Function Prototypes
-----------------------------------------------------------------------------*/

static inline void zero_all_bits(BIT_VECTOR array, size_t length) {
  for (size_t index = 0; index < length; index++) {
    array[index] = 0;
  }
}

static inline void set_all_bits(BIT_VECTOR array, size_t length) {
  for (size_t index = 0; index < length; index++) {
    array[index] = ~0;
  }
}

static inline void copy_all_bits(BIT_VECTOR source, BIT_VECTOR dest, size_t length) {
  for (size_t index = 0; index < length; index++) {
    dest[index] = source[index];
  }
}

#define SET_BIT(array, bit) (array[bit / BITSINLONG] |= 1 << (bit & (BITSINLONG - 1)))

#define reset_bit(array, bit) (array[bit / BITSINLONG] &= ~(1 << (bit & (BITSINLONG - 1))))

#define test_bit(array, bit) (array[bit / BITSINLONG] & (1 << (bit & (BITSINLONG - 1))))

static inline size_t WordsInVectorOfSize(size_t NumBits) {
  return (NumBits + BITSINLONG - 1) / BITSINLONG;
}

/**
 * This routine frees a bit vector.
 *
 * @param BitVector bit vector to be freed
 *
 */
static inline void FreeBitVector(BIT_VECTOR BitVector) {
  delete[] BitVector;
}

/*---------------------------------------------------------------------------*/
/**
 * Allocate and return a new bit vector large enough to
 * hold the specified number of bits.
 *
 * @param NumBits number of bits in new bit vector
 *
 * @return New bit vector.
 */
static inline BIT_VECTOR NewBitVector(size_t NumBits) {
  return new uint32_t[WordsInVectorOfSize(NumBits)];
}

#endif
