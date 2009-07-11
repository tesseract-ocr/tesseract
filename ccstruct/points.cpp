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

#include          "mfcpch.h"     //precompiled headers
#include          <stdlib.h>
#include          "ndminx.h"
#include          "serialis.h"
#include          "points.h"

ELISTIZE_S (ICOORDELT)           //turn to list
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


void ICOORD::serialise_asc(         //convert to ascii
                           FILE *f  //file to write
                          ) {
  serialise_INT32(f, xcoord);
  serialise_INT32(f, ycoord);
}


void ICOORD::de_serialise_asc(         //convert from ascii
                              FILE *f  //file to write
                             ) {
  xcoord = (inT16) de_serialise_INT32 (f);
  ycoord = (inT16) de_serialise_INT32 (f);
}


void ICOORDELT::serialise_asc(         //convert to ascii
                              FILE *f  //file to write
                             ) {
  ((ICOORD *) this)->serialise_asc (f);
}


void ICOORDELT::de_serialise_asc(         //convert from ascii
                                 FILE *f  //file to write
                                ) {
  ((ICOORD *) this)->de_serialise_asc (f);
}
