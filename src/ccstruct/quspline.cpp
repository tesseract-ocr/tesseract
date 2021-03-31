/**********************************************************************
 * File:        quspline.cpp  (Formerly qspline.c)
 * Description: Code for the QSPLINE class.
 * Author:      Ray Smith
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

// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#  include "config_auto.h"
#endif

#include "quspline.h"

#include "points.h"   // for ICOORD
#include "quadlsq.h"  // for QLSQ
#include "quadratc.h" // for QUAD_COEFFS

#include <allheaders.h> // for pixRenderPolyline, pixGetDepth, pixGetHeight
#include "pix.h"        // for L_CLEAR_PIXELS, L_SET_PIXELS, Pix (ptr only)

namespace tesseract {

#define QSPLINE_PRECISION 16 // no of steps to draw

/**********************************************************************
 * QSPLINE::QSPLINE
 *
 * Constructor to build a QSPLINE given the components used in the old code.
 **********************************************************************/

QSPLINE::QSPLINE(     // constructor
    int32_t count,    // no of segments
    int32_t *xstarts, // start coords
    double *coeffs    // coefficients
) {
  int32_t index; // segment index

  // get memory
  xcoords = new int32_t[count + 1];
  quadratics = new QUAD_COEFFS[count];
  segments = count;
  for (index = 0; index < segments; index++) {
    // copy them
    xcoords[index] = xstarts[index];
    quadratics[index] =
        QUAD_COEFFS(coeffs[index * 3], coeffs[index * 3 + 1], coeffs[index * 3 + 2]);
  }
  // right edge
  xcoords[index] = xstarts[index];
}

/**********************************************************************
 * QSPLINE::QSPLINE
 *
 * Constructor to build a QSPLINE by appproximation of points.
 **********************************************************************/

QSPLINE::QSPLINE(               // constructor
    int xstarts[],              // spline boundaries
    int segcount,               // no of segments
    int xpts[],                 // points to fit
    int ypts[], int pointcount, // no of pts
    int degree                  // fit required
) {
  int pointindex;    /*no along text line */
  int segment;       /*segment no */
  int32_t *ptcounts; // no in each segment
  QLSQ qlsq;         /*accumulator */

  segments = segcount;
  xcoords = new int32_t[segcount + 1];
  ptcounts = new int32_t[segcount + 1];
  quadratics = new QUAD_COEFFS[segcount];
  memmove(xcoords, xstarts, (segcount + 1) * sizeof(int32_t));
  ptcounts[0] = 0; /*none in any yet */
  for (segment = 0, pointindex = 0; pointindex < pointcount; pointindex++) {
    while (segment < segcount && xpts[pointindex] >= xstarts[segment]) {
      segment++; /*try next segment */
                 /*cumulative counts */
      ptcounts[segment] = ptcounts[segment - 1];
    }
    ptcounts[segment]++; /*no in previous partition */
  }
  while (segment < segcount) {
    segment++;
    /*zero the rest */
    ptcounts[segment] = ptcounts[segment - 1];
  }

  for (segment = 0; segment < segcount; segment++) {
    qlsq.clear();
    /*first blob */
    pointindex = ptcounts[segment];
    if (pointindex > 0 && xpts[pointindex] != xpts[pointindex - 1] &&
        xpts[pointindex] != xstarts[segment]) {
      qlsq.add(xstarts[segment],
               ypts[pointindex - 1] + (ypts[pointindex] - ypts[pointindex - 1]) *
                                          (xstarts[segment] - xpts[pointindex - 1]) /
                                          (xpts[pointindex] - xpts[pointindex - 1]));
    }
    for (; pointindex < ptcounts[segment + 1]; pointindex++) {
      qlsq.add(xpts[pointindex], ypts[pointindex]);
    }
    if (pointindex > 0 && pointindex < pointcount && xpts[pointindex] != xstarts[segment + 1]) {
      qlsq.add(xstarts[segment + 1],
               ypts[pointindex - 1] + (ypts[pointindex] - ypts[pointindex - 1]) *
                                          (xstarts[segment + 1] - xpts[pointindex - 1]) /
                                          (xpts[pointindex] - xpts[pointindex - 1]));
    }
    qlsq.fit(degree);
    quadratics[segment].a = qlsq.get_a();
    quadratics[segment].b = qlsq.get_b();
    quadratics[segment].c = qlsq.get_c();
  }
  delete[] ptcounts;
}

/**********************************************************************
 * QSPLINE::QSPLINE
 *
 * Constructor to build a QSPLINE from another.
 **********************************************************************/

QSPLINE::QSPLINE( // constructor
    const QSPLINE &src) {
  segments = 0;
  xcoords = nullptr;
  quadratics = nullptr;
  *this = src;
}

/**********************************************************************
 * QSPLINE::~QSPLINE
 *
 * Destroy a QSPLINE.
 **********************************************************************/

QSPLINE::~QSPLINE() {
  delete[] xcoords;
  delete[] quadratics;
}

/**********************************************************************
 * QSPLINE::operator=
 *
 * Copy a QSPLINE
 **********************************************************************/

QSPLINE &QSPLINE::operator=( // assignment
    const QSPLINE &source) {
  delete[] xcoords;
  delete[] quadratics;

  segments = source.segments;
  xcoords = new int32_t[segments + 1];
  quadratics = new QUAD_COEFFS[segments];
  memmove(xcoords, source.xcoords, (segments + 1) * sizeof(int32_t));
  memmove(quadratics, source.quadratics, segments * sizeof(QUAD_COEFFS));
  return *this;
}

/**********************************************************************
 * QSPLINE::step
 *
 * Return the total of the step functions between the given coords.
 **********************************************************************/

double QSPLINE::step( // find step functions
    double x1,        // between coords
    double x2) {
  int index1, index2; // indices of coords
  double total;       /*total steps */

  index1 = spline_index(x1);
  index2 = spline_index(x2);
  total = 0;
  while (index1 < index2) {
    total += static_cast<double>(quadratics[index1 + 1].y(static_cast<float>(xcoords[index1 + 1])));
    total -= static_cast<double>(quadratics[index1].y(static_cast<float>(xcoords[index1 + 1])));
    index1++; /*next segment */
  }
  return total; /*total steps */
}

/**********************************************************************
 * QSPLINE::y
 *
 * Return the y value at the given x value.
 **********************************************************************/

double QSPLINE::y( // evaluate
    double x       // coord to evaluate at
    ) const {
  int32_t index; // segment index

  index = spline_index(x);
  return quadratics[index].y(x); // in correct segment
}

/**********************************************************************
 * QSPLINE::spline_index
 *
 * Return the index to the largest xcoord not greater than x.
 **********************************************************************/

int32_t QSPLINE::spline_index( // evaluate
    double x                   // coord to evaluate at
    ) const {
  int32_t index;  // segment index
  int32_t bottom; // bottom of range
  int32_t top;    // top of range

  bottom = 0;
  top = segments;
  while (top - bottom > 1) {
    index = (top + bottom) / 2; // centre of range
    if (x >= xcoords[index]) {
      bottom = index; // new min
    } else {
      top = index; // new max
    }
  }
  return bottom;
}

/**********************************************************************
 * QSPLINE::move
 *
 * Reposition spline by vector
 **********************************************************************/

void QSPLINE::move( // reposition spline
    ICOORD vec      // by vector
) {
  int32_t segment; // index of segment
  int16_t x_shift = vec.x();

  for (segment = 0; segment < segments; segment++) {
    xcoords[segment] += x_shift;
    quadratics[segment].move(vec);
  }
  xcoords[segment] += x_shift;
}

/**********************************************************************
 * QSPLINE::overlap
 *
 * Return true if spline2 overlaps this by no more than fraction less
 * than the bounds of this.
 **********************************************************************/

bool QSPLINE::overlap( // test overlap
    QSPLINE *spline2,  // 2 cannot be smaller
    double fraction    // by more than this
) {
  int leftlimit = xcoords[1];             /*common left limit */
  int rightlimit = xcoords[segments - 1]; /*common right limit */
                                          /*or too non-overlap */
  return !(spline2->segments < 3 ||
           spline2->xcoords[1] > leftlimit + fraction * (rightlimit - leftlimit) ||
           spline2->xcoords[spline2->segments - 1] <
               rightlimit - fraction * (rightlimit - leftlimit));
}

/**********************************************************************
 * extrapolate_spline
 *
 * Extrapolates the spline linearly using the same gradient as the
 * quadratic has at either end.
 **********************************************************************/

void QSPLINE::extrapolate( // linear extrapolation
    double gradient,       // gradient to use
    int xmin,              // new left edge
    int xmax               // new right edge
) {
  int segment;        /*current segment of spline */
  int dest_segment;   // dest index
  int32_t *xstarts;   // new boundaries
  QUAD_COEFFS *quads; // new ones
  int increment;      // in size

  increment = xmin < xcoords[0] ? 1 : 0;
  if (xmax > xcoords[segments]) {
    increment++;
  }
  if (increment == 0) {
    return;
  }
  xstarts = new int32_t[segments + 1 + increment];
  quads = new QUAD_COEFFS[segments + increment];
  if (xmin < xcoords[0]) {
    xstarts[0] = xmin;
    quads[0].a = 0;
    quads[0].b = gradient;
    quads[0].c = y(xcoords[0]) - quads[0].b * xcoords[0];
    dest_segment = 1;
  } else {
    dest_segment = 0;
  }
  for (segment = 0; segment < segments; segment++) {
    xstarts[dest_segment] = xcoords[segment];
    quads[dest_segment] = quadratics[segment];
    dest_segment++;
  }
  xstarts[dest_segment] = xcoords[segment];
  if (xmax > xcoords[segments]) {
    quads[dest_segment].a = 0;
    quads[dest_segment].b = gradient;
    quads[dest_segment].c = y(xcoords[segments]) - quads[dest_segment].b * xcoords[segments];
    dest_segment++;
    xstarts[dest_segment] = xmax + 1;
  }
  segments = dest_segment;
  delete[] xcoords;
  delete[] quadratics;
  xcoords = xstarts;
  quadratics = quads;
}

/**********************************************************************
 * QSPLINE::plot
 *
 * Draw the QSPLINE in the given colour.
 **********************************************************************/

#ifndef GRAPHICS_DISABLED
void QSPLINE::plot(          // draw it
    ScrollView *window,      // window to draw in
    ScrollView::Color colour // colour to draw in
    ) const {
  int32_t segment;  // index of segment
  int16_t step;     // index of poly piece
  double increment; // x increment
  double x;         // x coord

  window->Pen(colour);
  for (segment = 0; segment < segments; segment++) {
    increment = static_cast<double>(xcoords[segment + 1] - xcoords[segment]) / QSPLINE_PRECISION;
    x = xcoords[segment];
    for (step = 0; step <= QSPLINE_PRECISION; step++) {
      if (segment == 0 && step == 0) {
        window->SetCursor(x, quadratics[segment].y(x));
      } else {
        window->DrawTo(x, quadratics[segment].y(x));
      }
      x += increment;
    }
  }
}
#endif

void QSPLINE::plot(Image pix) const {
  if (pix == nullptr) {
    return;
  }

  int32_t segment;  // Index of segment
  int16_t step;     // Index of poly piece
  double increment; // x increment
  double x;         // x coord
  auto height = static_cast<double>(pixGetHeight(pix));
  Pta *points = ptaCreate(QSPLINE_PRECISION * segments);
  const int kLineWidth = 5;

  for (segment = 0; segment < segments; segment++) {
    increment = static_cast<double>((xcoords[segment + 1] - xcoords[segment])) / QSPLINE_PRECISION;
    x = xcoords[segment];
    for (step = 0; step <= QSPLINE_PRECISION; step++) {
      double y = height - quadratics[segment].y(x);
      ptaAddPt(points, x, y);
      x += increment;
    }
  }

  switch (pixGetDepth(pix)) {
    case 1:
      pixRenderPolyline(pix, points, kLineWidth, L_SET_PIXELS, 1);
      break;
    case 32:
      pixRenderPolylineArb(pix, points, kLineWidth, 255, 0, 0, 1);
      break;
    default:
      pixRenderPolyline(pix, points, kLineWidth, L_CLEAR_PIXELS, 1);
      break;
  }
  ptaDestroy(&points);
}

} // namespace tesseract
