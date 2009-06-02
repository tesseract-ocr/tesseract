/******************************************************************************
 **	Filename:    bitvec.c
 **	Purpose:     Routines for manipulating bit vectors
 **	Author:      Dan Johnson
 **	History:     Thu Mar 15 10:37:27 1990, DSJ, Created.
 **
 **	(c) Copyright Hewlett-Packard Company, 1988.
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

/**----------------------------------------------------------------------------
          Include Files and Type Defines
----------------------------------------------------------------------------**/
#include "bitvec.h"

#include <stdio.h>

#include "emalloc.h"
#include "freelist.h"
#include "tprintf.h"

/**----------------------------------------------------------------------------
        Global Data Definitions and Declarations
----------------------------------------------------------------------------**/
static int BitVectorCount = 0;

/**----------------------------------------------------------------------------
              Public Code
----------------------------------------------------------------------------**/
/*---------------------------------------------------------------------------*/
BIT_VECTOR ExpandBitVector(BIT_VECTOR Vector, int NewNumBits) {
/*
 **	Parameters:
 **		Vector		bit vector to be expanded
 **		NewNumBits	new size of bit vector
 **	Globals: none
 **	Operation: This routine uses realloc to increase the size of
 **		the specified bit vector.
 **	Return: New expanded bit vector.
 **	Exceptions: none
 **	History: Fri Nov 16 10:11:16 1990, DSJ, Created.
 */
  return ((BIT_VECTOR) Erealloc(Vector,
    sizeof(Vector[0]) * WordsInVectorOfSize(NewNumBits)));
}                                /* ExpandBitVector */


/*---------------------------------------------------------------------------*/
void FreeBitVector(BIT_VECTOR BitVector) {
/*
 **	Parameters:
 **		BitVector	bit vector to be freed
 **	Globals:
 **		BitVectorCount	count of number of bit vectors allocated
 **	Operation: This routine frees a bit vector.  It also decrements
 **		the global counter that keeps track of the number of
 **		bit vectors allocated.  If BitVector is NULL, then
 **		the count is printed to stderr.
 **	Return: none
 **	Exceptions: none
 **	History: Tue Oct 23 16:46:09 1990, DSJ, Created.
 */
  if (BitVector) {
    Efree(BitVector);
    BitVectorCount--;
  } else {
    tprintf("%6d BITVECTOR elements in use\n", BitVectorCount);
  }
}                                /* FreeBitVector */


                                 /*hamming_distance(array1,array2,length) computes the hamming distance
   between two bit strings */
/*--------------------------------------------------------------------------*/
int hamming_distance(uinT32* array1, uinT32* array2, int length) {
  register uinT32 diff;   /*bit difference */
  register int dist;             /*total distance */

  dist = 0;
  for (; length > 0; length--) {
    diff = *array1++ ^ *array2++;/*different bits */
    while (diff) {
      diff &= diff - 1;          /*lose a bit */
      dist++;
    }
  }
  return dist;                   /*total distance */
}


/*---------------------------------------------------------------------------*/
BIT_VECTOR NewBitVector(int NumBits) {
/*
 **	Parameters:
 **		NumBits		number of bits in new bit vector
 **	Globals:
 **		BitVectorCount	number of bit vectors allocated
 **	Operation: Allocate and return a new bit vector large enough to
 **		hold the specified number of bits.
 **	Return: New bit vector.
 **	Exceptions: none
 **	History: Tue Oct 23 16:51:27 1990, DSJ, Created.
 */
  BitVectorCount++;
  return ((BIT_VECTOR) Emalloc(sizeof(uinT32) *
    WordsInVectorOfSize(NumBits)));
}                                /* NewBitVector */
