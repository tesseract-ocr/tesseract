/* -*-C-*-
 ********************************************************************************
 *
 * File:        array.h  (Formerly array.h)
 * Description:  Dynamic Array of String
 * Author:       Mark Seaman, SW Productivity
 * Created:      Fri Oct 16 14:37:00 1987
 * Modified:     Mon Sep 24 14:15:59 1990 (Mark Seaman) marks@hpgrlt
 * Language:     C
 * Package:      N/A
 * Status:       Reusable Software Component
 *
 * (c) Copyright 1987, Hewlett-Packard Company.
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
 *****************************************************************************

This file contains a set of general purpose dynamic array of string routines.
These routines can be used in a wide variety of ways to provide several
different popular data structures. A new "das" can be created by declaring
a variable of type 'DAS'
******************************************************************************/

#ifndef TESSARRAY_H
#define TESSARRAY_H

/*
----------------------------------------------------------------------
              I n c l u d e s
----------------------------------------------------------------------
*/

#include <stdio.h>

/*
----------------------------------------------------------------------
              T y p e s
----------------------------------------------------------------------
*/

typedef struct array_record
{
  size_t limit;
  size_t top;
  void *base[2];
} *ARRAY;

typedef void (*voidProc) ();

typedef int (*intProc) ();

/*
----------------------------------------------------------------------
              M a c r o s
----------------------------------------------------------------------
*/

#define DEFAULT_SIZE 2

/**********************************************************************
 * array_count
 *
 * Return the value of the number of elements currently in the array.
 **********************************************************************/

#define array_count(a)  \
((a)->top)

/**********************************************************************
 * array_free
 *
 * Free the memory allocated to this array.
 **********************************************************************/

#define array_free  \
memfree

/**********************************************************************
 * array_index
 *
 * Check to make sure that the index value is valid. Return the
 * value of the nth element currently in the array.
 **********************************************************************/

#define array_index(a,i)   \
((i<array_count(a)) ? (a)->base[i] : 0)

/**********************************************************************
 * array_limit
 *
 * Return the maximum number of elements that could be currently held
 * in this array without further expansion.
 **********************************************************************/

#define array_limit(a)     \
((a)->limit)

/**********************************************************************
 * array_loop
 *
 * Iterate through each of the array elements.  Each value can then be
 * accessed by:
 *    array_index (a, x)
 **********************************************************************/

#define array_loop(a,x)    \
for (x=0; x < array_count (a); x++)

/**********************************************************************
 * array_top
 *
 * Return the last element that was pushed on this array.
 **********************************************************************/

#define array_top(a)       \
((a)->base[array_count (a) - 1])

/**********************************************************************
 * array_value
 *
 * Return the nth element of the array.  Don't do range checking.
 **********************************************************************/

#define array_value(a,i)   \
((a)->base[i])

/*----------------------------------------------------------------------
              F u n c t i o n s
----------------------------------------------------------------------*/
ARRAY array_insert(ARRAY array, int index, void *value);

ARRAY array_new(int num);

ARRAY array_push(ARRAY array, void *value);

/*
#if defined(__STDC__) || defined(__cplusplus)
# define	_ARGS(s) s
#else
# define	_ARGS(s) ()
#endif*/

/* array.c
ARRAY array_insert
  _ARGS((ARRAY array,
  int index,
  char *value));

ARRAY array_new
  _ARGS((int num));

ARRAY array_push
  _ARGS((ARRAY array,
  char *value));

#undef _ARGS
*/
#endif
