/******************************************************************************
 **    Filename:    bitvec.h
 **    Purpose:     Routines for manipulating bit vectors
 **    Author:      Dan Johnson
 **    History:     Wed Mar  7 17:52:45 1990, DSJ, Created.
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
#ifndef   BITVEC_H
#define   BITVEC_H

#include "host.h"

/*-----------------------------------------------------------------------------
          Include Files and Type Defines
-----------------------------------------------------------------------------*/
// TODO(rays) Rename BITSINLONG to BITSINuinT32, and use sizeof.
#define BITSINLONG      32       /**< no of bits in a long */
typedef uinT32 *BIT_VECTOR;

/*-----------------------------------------------------------------------------
          Public Function Prototypes
-----------------------------------------------------------------------------*/
#define zero_all_bits(array, length)         \
  {                                          \
    int index; /*temporary index*/           \
                                             \
    for (index = 0; index < length; index++) \
      array[index] = 0; /*zero all bits*/    \
  }

#define set_all_bits(array, length)          \
  {                                          \
    int index; /*temporary index*/           \
                                             \
    for (index = 0; index < length; index++) \
      array[index] = ~0; /*set all bits*/    \
  }

#define copy_all_bits(source, dest, length)          \
  {                                                  \
    int index; /*temporary index*/                   \
                                                     \
    for (index = 0; index < length; index++)         \
      dest[index] = source[index]; /*copy all bits*/ \
  }

#define SET_BIT(array,bit) (array[bit/BITSINLONG]|=1<<(bit&(BITSINLONG-1)))

#define reset_bit(array,bit) (array[bit/BITSINLONG]&=~(1<<(bit&(BITSINLONG-1))))

#define test_bit(array,bit) (array[bit/BITSINLONG] & (1<<(bit&(BITSINLONG-1))))

#define WordsInVectorOfSize(NumBits) \
(((NumBits) + BITSINLONG - 1) / BITSINLONG)

/*--------------------------------------------------------------------------
        Public Function Prototypes
--------------------------------------------------------------------------*/
BIT_VECTOR ExpandBitVector(BIT_VECTOR Vector, int NewNumBits);

void FreeBitVector(BIT_VECTOR BitVector);

BIT_VECTOR NewBitVector(int NumBits);

#endif
