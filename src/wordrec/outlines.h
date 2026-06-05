/******************************************************************************
 *
 * File:         outlines.h
 * Description:  Combinatorial Splitter
 * Author:       Mark Seaman, OCR Technology
 *
 * (c) Copyright 1989, Hewlett-Packard Company.
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
 *****************************************************************************/

#ifndef OUTLINES_H
#define OUTLINES_H

#include <cmath>     // for abs
#include "blobs.h"   // for TPOINT
#include "params.h"  // for IntParam
#include "wordrec.h" // for Wordrec

/*----------------------------------------------------------------------
              C o n s t a n t s
----------------------------------------------------------------------*/
constexpr int LARGE_DISTANCE = 100000; /* Used for closest dist */
constexpr int MIN_BLOB_SIZE = 10;      /* Big units */
constexpr double MAX_ASPECT_RATIO = 2.5; /* Widest character */

/*----------------------------------------------------------------------
              M a c r o s
----------------------------------------------------------------------*/
/**********************************************************************
 * same_point
 *
 * Return true if the point values are the same. The parameters must
 * be of type POINT.
 **********************************************************************/
#define same_point(p1, p2) \
  ((abs(p1.x - p2.x) < chop_same_distance) && (abs(p1.y - p2.y) < chop_same_distance))

/**********************************************************************
 * dist_square
 *
 * Return the square of the distance between these two points.  The
 * parameters must be of type POINT.
 **********************************************************************/

template <typename Point>
inline constexpr auto dist_square(const Point &p1, const Point &p2) {
  return (p2.x - p1.x) * (p2.x - p1.x) + (p2.y - p1.y) * (p2.y - p1.y);
}

/**********************************************************************
 * closest
 *
 * The expression provides the EDGEPT that is closest to the point in
 * question.  All three parameters must be of type EDGEPT.
 **********************************************************************/

template <typename Edgept>
inline Edgept *closest(Edgept *test_p, Edgept *p1, Edgept *p2) {
  if (!p1) return p2;
  if (!p2) return p1;
  return dist_square(test_p->pos, p1->pos) < dist_square(test_p->pos, p2->pos) ? p1 : p2;
}

/**********************************************************************
 * edgept_dist
 *
 * Return the distance (squared) between the two edge points.
 **********************************************************************/

template <typename Edgept>
inline constexpr auto edgept_dist(const Edgept *p1, const Edgept *p2) {
  return dist_square(p1->pos, p2->pos);
}

/**********************************************************************
 * is_exterior_point
 *
 * Return true if the point supplied is an exterior projection from the
 * outline.
 **********************************************************************/

#define is_exterior_point(edge, point)                                                   \
  (same_point(edge->prev->pos, point->pos) || same_point(edge->next->pos, point->pos) || \
   (angle_change(edge->prev, edge, edge->next) - angle_change(edge->prev, edge, point) > 20))

/**********************************************************************
 * is_equal
 *
 * Return true if the POINTs are equal.
 **********************************************************************/

template <typename Point>
inline constexpr bool is_equal(const Point &p1, const Point &p2) {
  return p1.x == p2.x && p1.y == p2.y;
}

/**********************************************************************
 * is_on_line
 *
 * Return true if the point is on the line segment between the two end
 * points.  The two end points are included as part of the  line.  The
 * parameters must be of type POINT.
 **********************************************************************/

template <typename T>
inline constexpr bool within_range(T x, T x0, T x1) {
  return (x0 <= x && x <= x1) || (x1 <= x && x <= x0);
}

template <typename Point>
inline constexpr bool is_on_line(const Point &p, const Point &p0, const Point &p1) {
  return within_range(p.x, p0.x, p1.x) && within_range(p.y, p0.y, p1.y);
}

#endif
