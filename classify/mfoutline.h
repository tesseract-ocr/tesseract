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
#include "blobs.h"
#include "host.h"
#include "oldlist.h"
#include "fpoint.h"
#include "params.h"

#define NORMAL_X_HEIGHT   (0.5)
#define NORMAL_BASELINE   (0.0)

typedef LIST MFOUTLINE;

typedef enum {
  north, south, east, west, northeast, northwest, southeast, southwest
} DIRECTION;

typedef struct {
  FPOINT Point;
  FLOAT32 Slope;
  unsigned Padding:20;
  BOOL8 Hidden:TRUE;
  BOOL8 ExtremityMark:TRUE;
  DIRECTION Direction:4;
  DIRECTION PreviousDirection:4;
} MFEDGEPT;

typedef enum {
  outer, hole
} OUTLINETYPE;

typedef struct {
  FLOAT64 Mx, My;                /* first moment of all outlines */
  FLOAT64 L;                     /* total length of all outlines */
  FLOAT64 x, y;                  /* center of mass of all outlines */
  FLOAT64 Ix, Iy;                /* second moments about center of mass axes */
  FLOAT64 Rx, Ry;                /* radius of gyration about center of mass axes */
} OUTLINE_STATS;

typedef enum {
  baseline, character
} NORM_METHOD;

/**----------------------------------------------------------------------------
          Macros
----------------------------------------------------------------------------**/
#define AverageOf(A,B)    (((A) + (B)) / 2)

/* macro for computing the scale factor to use to normalize characters */
#define MF_SCALE_FACTOR  (NORMAL_X_HEIGHT / kBlnXHeight)

/* macros for manipulating micro-feature outlines */
#define DegenerateOutline(O)  (((O) == NIL_LIST) || ((O) == list_rest(O)))
#define PointAt(O)    ((MFEDGEPT *) first_node (O))
#define NextPointAfter(E) (list_rest (E))
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
                      FLOAT32 XOrigin);

/*----------------------------------------------------------------------------
          Private Function Prototypes
-----------------------------------------------------------------------------*/
void ChangeDirection(MFOUTLINE Start, MFOUTLINE End, DIRECTION Direction);

// Normalizes the Outline in-place using cn_denorm's local transformation,
// then converts from the integer feature range [0,255] to the clusterer
// feature range of [-0.5, 0.5].
void CharNormalizeOutline(MFOUTLINE Outline, const DENORM& cn_denorm);

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
