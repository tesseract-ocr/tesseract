/******************************************************************************
 **	Filename:    fxdefs.h
 **	Purpose:     Generic interface definitions for feature extractors
 **	Author:      Dan Johnson
 **	History:     Fri Jan 19 09:04:14 1990, DSJ, Created.
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
#ifndef   FXDEFS_H
#define   FXDEFS_H

/*-----------------------------------------------------------------------------
          Include Files and Type Defines
-----------------------------------------------------------------------------*/
#include "tessclas.h"
#include "general.h"

/* define different modes for feature extractor - learning vs. classifying */
#define LEARN_MODE      0
#define CLASSIFY_MODE   1

/** define a data structure to hold line statistics.  These line statistics
  are used to normalize character outlines to a standard size and position
  relative to the baseline of the text. */
typedef struct
{
  SPLINE_SPEC *Baseline;         /**< collection of splines describing baseline */
  SPLINE_SPEC *XHeightLine;      /**< collection of splines describing x-height */
  FLOAT32 xheight;               /**< avg. distance from x-height to baseline */
  FLOAT32 AscRise;               /**< avg. distance from ascenders to x-height */
  FLOAT32 DescDrop;              /**< avg. distance from baseline to descenders */
  /* always a negative number */
  TEXTROW *TextRow;              /**< kludge - only needed by fx for fast matcher */
  /* should be removed later */
}


LINE_STATS;

/** define a generic character description as a char pointer.  In reality,
  it will be a pointer to some data structure.  Paired feature
  extractors/matchers need to agree on the data structure to be used,
  however, the high level classifier does not need to know the details
  of this data structure. */
typedef char *CHAR_FEATURES;

/*-----------------------------------------------------------------------------
            Macros
-----------------------------------------------------------------------------*/
/** macro to change and monitor the mode of the feature extractor.
  In general, learn mode smears features which would otherwise be discrete
  in nature; classify mode does not.*/
#define SetExtractMode(M) (ExtractMode = (M))
#define EnterLearnMode    (SetExtractMode (LEARN_MODE))
#define EnterClassifyMode (SetExtractMode (CLASSIFY_MODE))

/*----------------------------------------------------------------------------
          Public Function Prototypes
-----------------------------------------------------------------------------*/
void SetupExtractors();

void GetLineStatsFromRow(TEXTROW *Row, LINE_STATS *LineStats);

/*
#if defined(__STDC__) || defined(__cplusplus)
# define        _ARGS(s) s
#else
# define        _ARGS(s) ()
#endif*/

/* fxdefs.c
void GetLineStatsFromRow
    _ARGS((TEXTROW *Row,
  LINE_STATS *LineStats));

#undef _ARGS
*/

/*-----------------------------------------------------------------------------
        Global Data Definitions and Declarations
-----------------------------------------------------------------------------*/
/** flag to control learn mode vs. classify mode */
extern int ExtractMode;
#endif
