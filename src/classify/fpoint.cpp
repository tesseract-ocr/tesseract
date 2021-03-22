/******************************************************************************
 ** Filename:    fpoint.cpp
 ** Purpose:     Abstract data type for a 2D point (floating point coords)
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
/*----------------------------------------------------------------------------
          Include Files and Type Defines
----------------------------------------------------------------------------*/
#define _USE_MATH_DEFINES // for M_PI
#include "fpoint.h"
#include <cmath> // for M_PI
#include <cstdio>

/*----------------------------------------------------------------------------
              Public Code
----------------------------------------------------------------------------*/

float DistanceBetween(FPOINT A, FPOINT B) {
  const double xd = XDelta(A, B);
  const double yd = YDelta(A, B);
  return sqrt(static_cast<double>(xd * xd + yd * yd));
}

/**
 * Return the angle from Point1 to Point2 normalized to
 * lie in the range 0 to FullScale (where FullScale corresponds
 * to 2*pi or 360 degrees).
 * @param Point1 points to compute angle between
 * @param Point2 points to compute angle between
 * @param FullScale value to associate with 2*pi
 * @return angle
 */
float NormalizedAngleFrom(FPOINT *Point1, FPOINT *Point2, float FullScale) {
  float NumRadsInCircle = 2.0 * M_PI;

  float Angle = AngleFrom(*Point1, *Point2);
  if (Angle < 0.0) {
    Angle += NumRadsInCircle;
  }
  Angle *= FullScale / NumRadsInCircle;
  if (Angle < 0.0 || Angle >= FullScale) {
    Angle = 0.0;
  }
  return (Angle);
}
