/******************************************************************************
 ** Filename:    fpoint.h
 ** Purpose:     Abstract data type for 2D points (floating point coords)
 ** Author:      Dan Johnson
 **
 ** (c) Copyright Hewlett-Packard Company, 1988.
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

#ifndef FPOINT_H
#define FPOINT_H

/**----------------------------------------------------------------------------
          Include Files and Type Defines
----------------------------------------------------------------------------**/
#include <cmath>
#include <cstdio>

/* define data structure to hold 2D points or vectors using floating point */
struct FPOINT {
  float x, y;
};
using FVECTOR = FPOINT;

/**----------------------------------------------------------------------------
            Macros
----------------------------------------------------------------------------**/
/* functions for computing miscellaneous properties of 2 points */
inline constexpr float XDelta(const FPOINT &A, const FPOINT &B) {
  return B.x - A.x;
}
inline constexpr float YDelta(const FPOINT &A, const FPOINT &B) {
  return B.y - A.y;
}
inline constexpr float SlopeFrom(const FPOINT &A, const FPOINT &B) {
  return YDelta(A, B) / XDelta(A, B);
}
inline double AngleFrom(const FPOINT &A, const FPOINT &B) {
  return std::atan2(static_cast<double>(YDelta(A, B)), static_cast<double>(XDelta(A, B)));
}
inline constexpr float XIntersectionOf(const FPOINT &A, const FPOINT &B, float X) {
  return SlopeFrom(A, B) * (X - A.x) + A.y;
}

/*-------------------------------------------------------------------------
        Public Function Prototypes
---------------------------------------------------------------------------*/

float DistanceBetween(FPOINT A, FPOINT B);

float NormalizedAngleFrom(FPOINT *Point1, FPOINT *Point2, float FullScale);

#endif
