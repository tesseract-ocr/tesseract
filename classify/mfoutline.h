/******************************************************************************
 **	Filename:    mfoutline.h
 **	Purpose:     Interface spec for fx outline structures
 **	Author:      Dan Johnson
 **	History:     Thu May 17 08:55:32 1990, DSJ, Created.
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
#ifndef   MFOUTLINE_H
#define   MFOUTLINE_H

/**----------------------------------------------------------------------------
          Include Files and Type Defines
----------------------------------------------------------------------------**/
#include "general.h"
#include "oldlist.h"
#include "fpoint.h"
#include "fxdefs.h"
#include "baseline.h"
#include "varable.h"

#define NORMAL_X_HEIGHT   (0.5)
#define NORMAL_BASELINE   (0.0)

typedef LIST MFOUTLINE;

typedef enum {
  north, south, east, west, northeast, northwest, southeast, southwest
}


DIRECTION;

typedef struct
{
  FPOINT Point;
  FLOAT32 Slope;
  unsigned Padding:20;
  BOOL8 Hidden:TRUE;
  BOOL8 ExtremityMark:TRUE;
  DIRECTION Direction:4;
  DIRECTION PreviousDirection:4;
}


MFEDGEPT;

typedef enum {
  outer, hole
}


OUTLINETYPE;

typedef struct
{
  FLOAT64 Mx, My;                /* first moment of all outlines */
  FLOAT64 L;                     /* total length of all outlines */
  FLOAT64 x, y;                  /* center of mass of all outlines */
  FLOAT64 Ix, Iy;                /* second moments about center of mass axes */
  FLOAT64 Rx, Ry;                /* radius of gyration about center of mass axes */
}


OUTLINE_STATS;

typedef enum {
  baseline, character
}


NORM_METHOD;

/*----------------------------------------------------------------------------
            Variables
------------------------------------------------------------------------------*/
/* control knobs used to control normalization of outlines */
extern INT_VAR_H(classify_norm_method, character,
                 "Normalization Method   ...");
/* PREV DEFAULT "baseline" */
extern double_VAR_H(classify_char_norm_range, 0.2,
                    "Character Normalization Range ...");
extern double_VAR_H(classify_min_norm_scale_x, 0.0,
                    "Min char x-norm scale ...");
/* PREV DEFAULT 0.1 */
extern double_VAR_H(classify_max_norm_scale_x, 0.325,
                    "Max char x-norm scale ...");
/* PREV DEFAULT 0.3 */
extern double_VAR_H(classify_min_norm_scale_y, 0.0,
                    "Min char y-norm scale ...");
/* PREV DEFAULT 0.1 */
extern double_VAR_H(classify_max_norm_scale_y, 0.325,
                    "Max char y-norm scale ...");
/* PREV DEFAULT 0.3 */

/**----------------------------------------------------------------------------
          Macros
----------------------------------------------------------------------------**/
#define AverageOf(A,B)    (((A) + (B)) / 2)

/* macro for computing the baseline of a row of text at an x position */
#define BaselineAt(L,X) (BASELINE_OFFSET)

/* macro for computing the scale factor to use to normalize characters */
#define ComputeScaleFactor(L)						\
(NORMAL_X_HEIGHT / ((classify_baseline_normalized)?			\
				(BASELINE_SCALE):					\
				((L)->xheight)))

/* macros for manipulating micro-feature outlines */
#define DegenerateOutline(O)  (((O) == NIL) || ((O) == rest(O)))
#define PointAt(O)    ((MFEDGEPT *) first_node (O))
#define NextPointAfter(E) (rest (E))
#define MakeOutlineCircular(O)  (set_rest (last (O), (O)))

/* macros for manipulating micro-feature outline edge points */
#define ClearMark(P)    ((P)->ExtremityMark = FALSE)
#define MarkPoint(P)    ((P)->ExtremityMark = TRUE)

/**----------------------------------------------------------------------------
          Public Function Prototypes
----------------------------------------------------------------------------**/
void ComputeBlobCenter(TBLOB *Blob, TPOINT *BlobCenter);

LIST ConvertBlob(TBLOB *Blob);

MFOUTLINE ConvertOutline(TESSLINE *Outline);

LIST ConvertOutlines(TESSLINE *Outline,
                     LIST ConvertedOutlines,
                     OUTLINETYPE OutlineType);

void ComputeOutlineStats(LIST Outlines, OUTLINE_STATS *OutlineStats);

void FilterEdgeNoise(MFOUTLINE Outline, FLOAT32 NoiseSegmentLength);

void FindDirectionChanges(MFOUTLINE Outline,
                          FLOAT32 MinSlope,
                          FLOAT32 MaxSlope);

void FreeMFOutline(void *agr);  //MFOUTLINE                             Outline);

void FreeOutlines(LIST Outlines);

void MarkDirectionChanges(MFOUTLINE Outline);

MFEDGEPT *NewEdgePoint();

MFOUTLINE NextExtremity(MFOUTLINE EdgePoint);

void NormalizeOutline(MFOUTLINE Outline,
                      LINE_STATS *LineStats,
                      FLOAT32 XOrigin);

void NormalizeOutlines(LIST Outlines,
                       LINE_STATS *LineStats,
                       FLOAT32 *XScale,
                       FLOAT32 *YScale);

void SettupBlobConversion(TBLOB *Blob);

void SmearExtremities(MFOUTLINE Outline, FLOAT32 XScale, FLOAT32 YScale);

/*----------------------------------------------------------------------------
          Private Function Prototypes
-----------------------------------------------------------------------------*/
void ChangeDirection(MFOUTLINE Start, MFOUTLINE End, DIRECTION Direction);

void CharNormalizeOutline(MFOUTLINE Outline,
                          FLOAT32 XCenter,
                          FLOAT32 YCenter,
                          FLOAT32 XScale,
                          FLOAT32 YScale);

void ComputeDirection(MFEDGEPT *Start,
                      MFEDGEPT *Finish,
                      FLOAT32 MinSlope,
                      FLOAT32 MaxSlope);

void FinishOutlineStats(register OUTLINE_STATS *OutlineStats);

void InitOutlineStats(OUTLINE_STATS *OutlineStats);

MFOUTLINE NextDirectionChange(MFOUTLINE EdgePoint);

void UpdateOutlineStats(register OUTLINE_STATS *OutlineStats,
                        register FLOAT32 x1,
                        register FLOAT32 x2,
                        register FLOAT32 y1,
                        register FLOAT32 y2);

#endif
