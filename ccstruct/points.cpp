/**********************************************************************
 * File:        points.c  (Formerly coords.c)
 * Description: Member functions for coordinate classes.
 * Author:					Ray Smith
 * Created:					Fri Mar 15 08:58:17 GMT 1991
 *
 * (C) Copyright 1991, Hewlett-Packard Ltd.
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
 **********************************************************************/

#ifdef _MSC_VER
#define _USE_MATH_DEFINES
#endif  // _MSC_VER

#include          <stdlib.h>
#include          "helpers.h"
#include          "ndminx.h"
#include          "serialis.h"
#include          "points.h"

ELISTIZE (ICOORDELT)           //turn to list
bool FCOORD::normalise() {  //Convert to unit vec
  float len = length ();

  if (len < 0.0000000001) {
    return false;
  }
  xcoord /= len;
  ycoord /= len;
  return true;
}

// Set from the given x,y, shrinking the vector to fit if needed.
void ICOORD::set_with_shrink(int x, int y) {
  // Fit the vector into an ICOORD, which is 16 bit.
  int factor = 1;
  int max_extent = MAX(abs(x), abs(y));
  if (max_extent > MAX_INT16)
    factor = max_extent / MAX_INT16 + 1;
  xcoord = x / factor;
  ycoord = y / factor;
}

// The fortran/basic sgn function returns -1, 0, 1 if x < 0, x == 0, x > 0
// respectively.
static int sign(int x) {
  if (x < 0)
    return -1;
  else
    return x > 0 ? 1 : 0;
}

// Writes to the given file. Returns false in case of error.
bool ICOORD::Serialize(FILE* fp) const {
  if (fwrite(&xcoord, sizeof(xcoord), 1, fp) != 1) return false;
  if (fwrite(&ycoord, sizeof(ycoord), 1, fp) != 1) return false;
  return true;
}
// Reads from the given file. Returns false in case of error.
// If swap is true, assumes a big/little-endian swap is needed.
bool ICOORD::DeSerialize(bool swap, FILE* fp) {
  if (fread(&xcoord, sizeof(xcoord), 1, fp) != 1) return false;
  if (fread(&ycoord, sizeof(ycoord), 1, fp) != 1) return false;
  if (swap) {
    ReverseN(&xcoord, sizeof(xcoord));
    ReverseN(&ycoord, sizeof(ycoord));
  }
  return true;
}

// Setup for iterating over the pixels in a vector by the well-known
// Bresenham rendering algorithm.
// Starting with major/2 in the accumulator, on each step add major_step,
// and then add minor to the accumulator. When the accumulator >= major
// subtract major and step a minor step.

void ICOORD::setup_render(ICOORD* major_step, ICOORD* minor_step,
                          int* major, int* minor) const {
  int abs_x = abs(xcoord);
  int abs_y = abs(ycoord);
  if (abs_x >= abs_y) {
    // X-direction is major.
    major_step->xcoord = sign(xcoord);
    major_step->ycoord = 0;
    minor_step->xcoord = 0;
    minor_step->ycoord = sign(ycoord);
    *major = abs_x;
    *minor = abs_y;
  } else {
    // Y-direction is major.
    major_step->xcoord = 0;
    major_step->ycoord = sign(ycoord);
    minor_step->xcoord = sign(xcoord);
    minor_step->ycoord = 0;
    *major = abs_y;
    *minor = abs_x;
  }
}

// Returns the standard feature direction corresponding to this.
// See binary_angle_plus_pi below for a description of the direction.
uinT8 FCOORD::to_direction() const {
  return binary_angle_plus_pi(angle());
}
// Sets this with a unit vector in the given standard feature direction.
void FCOORD::from_direction(uinT8 direction) {
  double radians = angle_from_direction(direction);
  xcoord = cos(radians);
  ycoord = sin(radians);
}

// Converts an angle in radians (from ICOORD::angle or FCOORD::angle) to a
// standard feature direction as an unsigned angle in 256ths of a circle
// measured anticlockwise from (-1, 0).
uinT8 FCOORD::binary_angle_plus_pi(double radians) {
  return Modulo(IntCastRounded((radians + M_PI) * 128.0 / M_PI), 256);
}
// Inverse of binary_angle_plus_pi returns an angle in radians for the
// given standard feature direction.
double FCOORD::angle_from_direction(uinT8 direction) {
  return direction * M_PI / 128.0 - M_PI;
}

// Returns the point on the given line nearest to this, ie the point such
// that the vector point->this is perpendicular to the line.
// The line is defined as a line_point and a dir_vector for its direction.
FCOORD FCOORD::nearest_pt_on_line(const FCOORD& line_point,
                                  const FCOORD& dir_vector) const {
  FCOORD point_vector(*this - line_point);
  // The dot product (%) is |dir_vector||point_vector|cos theta, so dividing by
  // the square of the length of dir_vector gives us the fraction of dir_vector
  // to add to line1 to get the appropriate point, so
  // result = line1 + lambda dir_vector.
  double lambda = point_vector % dir_vector / dir_vector.sqlength();
  return line_point + (dir_vector * lambda);
}
