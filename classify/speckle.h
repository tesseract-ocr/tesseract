/******************************************************************************
 **	Filename:    speckle.h
 **	Purpose:     Interface to classifier speckle filtering routines.
 **	Author:      Dan Johnson
 **	History:     Mon Mar 11 10:14:16 1991, DSJ, Created.
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
#ifndef SPECKLE_H
#define SPECKLE_H

/**----------------------------------------------------------------------------
          Include Files and Type Defines
----------------------------------------------------------------------------**/
#include "baseline.h"
#include "choices.h"

/**----------------------------------------------------------------------------
            Macros
----------------------------------------------------------------------------**/
/* macro for getting the height of a row of text */
#define RowHeight(R)	((is_baseline_normalized ())?			\
			(BASELINE_SCALE):				\
			((R)->lineheight))

/**----------------------------------------------------------------------------
          Public Function Prototypes
----------------------------------------------------------------------------**/
LIST AddLargeSpeckleTo(LIST Choices); 

void InitSpeckleVars(); 

BOOL8 LargeSpeckle(TBLOB *Blob, TEXTROW *Row); 

/*
#if defined(__STDC__) || defined(__cplusplus)
# define        _ARGS(s) s
#else
# define        _ARGS(s) ()
#endif*/

/* speckle.c
LIST AddLargeSpeckleTo
    _ARGS((LIST Choices));

void InitSpeckleVars
    _ARGS((void));

BOOL8 LargeSpeckle
    _ARGS((BLOB *Blob,
  TEXTROW *Row));

#undef _ARGS
*/
/**----------------------------------------------------------------------------
        Global Data Definitions and Declarations
----------------------------------------------------------------------------**/
extern float SmallSpecklePenalty;
extern float SmallSpeckleCertainty;
#endif
