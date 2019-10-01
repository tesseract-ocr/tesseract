/******************************************************************************
 ** Filename:    mfoutline.h
 ** Purpose:     Interface spec for fx outline structures
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

#ifndef MFOUTLINE_H
#define MFOUTLINE_H

/**----------------------------------------------------------------------------
          Include Files and Type Defines
----------------------------------------------------------------------------**/
#include "blobs.h"
#include "fpoint.h"
#include "oldlist.h"
#include "params.h"

using MFOUTLINE = LIST;

enum DIRECTION : uint8_t {
  north,
  south,
  east,
  west,
  northeast,
  northwest,
  southeast,
  southwest
};

struct MFEDGEPT {
  // Inline functions for manipulating micro-feature outline edge points.

  void ClearMark() {
    ExtremityMark = false;
  }

  void MarkPoint() {
    ExtremityMark = true;
  }

  FPOINT Point;
  float Slope;
  bool Hidden;
  bool ExtremityMark;
  DIRECTION Direction;
  DIRECTION PreviousDirection;
};

enum OUTLINETYPE { outer, hole };

enum NORM_METHOD { baseline, character };

/**----------------------------------------------------------------------------
          Macros
----------------------------------------------------------------------------**/
#define AverageOf(A, B) (((A) + (B)) / 2)

// Constant for computing the scale factor to use to normalize characters.
const float MF_SCALE_FACTOR = 0.5f / kBlnXHeight;

// Inline functions for manipulating micro-feature outlines.

static inline bool DegenerateOutline(MFOUTLINE Outline) {
  return (Outline == NIL_LIST) || (Outline == list_rest(Outline));
}

static inline MFEDGEPT* PointAt(MFOUTLINE Outline) {
  return reinterpret_cast<MFEDGEPT*>first_node(Outline);
}

static inline MFOUTLINE NextPointAfter(MFOUTLINE Outline) {
  return list_rest(Outline);
}

static inline void MakeOutlineCircular(MFOUTLINE Outline) {
  set_rest(last(Outline), Outline);
}

/**----------------------------------------------------------------------------
          Public Function Prototypes
----------------------------------------------------------------------------**/
void ComputeBlobCenter(TBLOB* Blob, TPOINT* BlobCenter);

LIST ConvertBlob(TBLOB* Blob);

MFOUTLINE ConvertOutline(TESSLINE* Outline);

LIST ConvertOutlines(TESSLINE* Outline, LIST ConvertedOutlines,
                     OUTLINETYPE OutlineType);

void FilterEdgeNoise(MFOUTLINE Outline, float NoiseSegmentLength);

void FindDirectionChanges(MFOUTLINE Outline, float MinSlope, float MaxSlope);

void FreeMFOutline(void* agr);  // MFOUTLINE Outline);

void FreeOutlines(LIST Outlines);

void MarkDirectionChanges(MFOUTLINE Outline);

MFEDGEPT* NewEdgePoint();

MFOUTLINE NextExtremity(MFOUTLINE EdgePoint);

void NormalizeOutline(MFOUTLINE Outline, float XOrigin);

/*----------------------------------------------------------------------------
          Private Function Prototypes
-----------------------------------------------------------------------------*/
void ChangeDirection(MFOUTLINE Start, MFOUTLINE End, DIRECTION Direction);

// Normalizes the Outline in-place using cn_denorm's local transformation,
// then converts from the integer feature range [0,255] to the clusterer
// feature range of [-0.5, 0.5].
void CharNormalizeOutline(MFOUTLINE Outline, const DENORM& cn_denorm);

void ComputeDirection(MFEDGEPT* Start, MFEDGEPT* Finish, float MinSlope,
                      float MaxSlope);

MFOUTLINE NextDirectionChange(MFOUTLINE EdgePoint);

#endif
