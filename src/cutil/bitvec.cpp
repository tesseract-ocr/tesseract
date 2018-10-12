/******************************************************************************
 ** Filename:    bitvec.c
 ** Purpose:     Routines for manipulating bit vectors
 ** Author:      Dan Johnson
 **
 ** (c) Copyright Hewlett-Packard Company, 1988.
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

/*-----------------------------------------------------------------------------
          Include Files and Type Defines
-----------------------------------------------------------------------------*/
#include "bitvec.h"

#include <cstdio>

#include "emalloc.h"
#include "tprintf.h"

/*-----------------------------------------------------------------------------
              Public Code
-----------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/**
 * This routine uses realloc to increase the size of
 * the specified bit vector.
 *
 * Globals:
 * - none
 *
 * @param Vector bit vector to be expanded
 * @param NewNumBits new size of bit vector
 *
 * @return New expanded bit vector.
 */
BIT_VECTOR ExpandBitVector(BIT_VECTOR Vector, int NewNumBits) {
  return ((BIT_VECTOR) Erealloc(Vector,
    sizeof(Vector[0]) * WordsInVectorOfSize(NewNumBits)));
}                                /* ExpandBitVector */


/*---------------------------------------------------------------------------*/
void FreeBitVector(BIT_VECTOR BitVector) {
/**
 * This routine frees a bit vector.  It also decrements
 * the global counter that keeps track of the number of
 * bit vectors allocated.  If BitVector is nullptr, then
 * the count is printed to stderr.
 *
 * Globals:
 * - BitVectorCount count of number of bit vectors allocated
 *
 * @param BitVector bit vector to be freed
 *
 */
  if (BitVector) {
    Efree(BitVector);
  }
}                                /* FreeBitVector */


/*---------------------------------------------------------------------------*/
/**
 * Allocate and return a new bit vector large enough to
 * hold the specified number of bits.
 *
 * Globals:
 * - BitVectorCount number of bit vectors allocated
 *
 * @param NumBits number of bits in new bit vector
 *
 * @return New bit vector.
 */
BIT_VECTOR NewBitVector(int NumBits) {
  return ((BIT_VECTOR) Emalloc(sizeof(uint32_t) *
    WordsInVectorOfSize(NumBits)));
}                                /* NewBitVector */
