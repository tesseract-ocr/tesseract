/******************************************************************************
 **	Filename:    fpoint.c
 **	Purpose:     Abstract data type for a 2D point (floating point coords)
 **	Author:      Dan Johnson
 **	History:     Thu Apr 12 10:44:15 1990, DSJ, Created.
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
#include "const.h"
#include "fpoint.h"
#include <stdio.h>
#include <math.h>

/**----------------------------------------------------------------------------
              Public Code
----------------------------------------------------------------------------**/
/*---------------------------------------------------------------------------*/

FLOAT32 DistanceBetween(FPOINT A, FPOINT B) {
  double xd = XDelta(A, B);
  double yd = YDelta(A, B);
  return sqrt(static_cast<double>(xd * xd + yd * yd));
}



FLOAT32 NormalizedAngleFrom(FPOINT *Point1,
                            FPOINT *Point2,
                            FLOAT32 FullScale) {
/*
 **	Parameters:
 **		Point1, Point2	points to compute angle between
 **		FullScale	value to associate with 2*pi
 **	Globals: none
 **	Operation: Return the angle from Point1 to Point2 normalized to
 **		lie in the range 0 to FullScale (where FullScale corresponds
 **		to 2*pi or 360 degrees).
 **	Return: none
 **	Exceptions: none
 **	History: Wed Mar 28 14:27:25 1990, DSJ, Created.
 */
  FLOAT32 Angle;
  FLOAT32 NumRadsInCircle = 2.0 * PI;

  Angle = AngleFrom (*Point1, *Point2);
  if (Angle < 0.0)
    Angle += NumRadsInCircle;
  Angle *= FullScale / NumRadsInCircle;
  if (Angle < 0.0 || Angle >= FullScale)
    Angle = 0.0;
  return (Angle);

}                                /* NormalizedAngleFrom */
