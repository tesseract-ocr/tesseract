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

/*-----------------------------------------------------------------------------
          Include Files and Type Defines
-----------------------------------------------------------------------------*/

#include "baseline.h"
#include "ratngs.h"

/*-----------------------------------------------------------------------------
            Macros
-----------------------------------------------------------------------------*/
/** macro for getting the height of a row of text */
#define RowHeight(R)	((classify_baseline_normalized)?			\
			(BASELINE_SCALE):				\
			((R)->lineheight))

/*-----------------------------------------------------------------------------
          Public Function Prototypes
-----------------------------------------------------------------------------*/
void AddLargeSpeckleTo(BLOB_CHOICE_LIST *Choices);

BOOL8 LargeSpeckle(TBLOB *Blob, TEXTROW *Row);

#endif
