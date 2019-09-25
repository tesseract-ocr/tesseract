/* -*-C-*-
 ******************************************************************************
 *
 * File:        chop.cpp  (Formerly chop.c)
 * Author:      Mark Seaman, OCR Technology
 *
 * (c) Copyright 1987, Hewlett-Packard Company.
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

/*----------------------------------------------------------------------
              I n c l u d e s
----------------------------------------------------------------------*/

#define _USE_MATH_DEFINES       // for M_PI
#include <cmath>                // for M_PI
#include "chop.h"
#include "outlines.h"
#include "callcpp.h"
#include "plotedges.h"
#include "wordrec.h"

// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif

namespace tesseract {

// Show if the line is going in the positive or negative X direction.
static int direction(const EDGEPT* point) {
  //* direction to return
  int dir = 0;
  //* prev point
  const EDGEPT* prev = point->prev;
  //* next point
  const EDGEPT* next = point->next;

  if (((prev->pos.x <= point->pos.x) && (point->pos.x < next->pos.x)) ||
      ((prev->pos.x < point->pos.x) && (point->pos.x <= next->pos.x))) {
    dir = 1;
  }
  if (((prev->pos.x >= point->pos.x) && (point->pos.x > next->pos.x)) ||
      ((prev->pos.x > point->pos.x) && (point->pos.x >= next->pos.x))) {
    dir = -1;
  }

  return dir;
}

/**
 * @name point_priority
 *
 * Assign a priority to and edge point that might be used as part of a
 * split. The argument should be of type EDGEPT.
 */
PRIORITY Wordrec::point_priority(EDGEPT *point) {
  return static_cast<PRIORITY>(angle_change(point->prev, point, point->next));
}


/**
 * @name add_point_to_list
 *
 * Add an edge point to a POINT_GROUP containing a list of other points.
 */
void Wordrec::add_point_to_list(PointHeap* point_heap, EDGEPT *point) {
  if (point_heap->size() < MAX_NUM_POINTS - 2) {
    PointPair pair(point_priority(point), point);
    point_heap->Push(&pair);
  }

#ifndef GRAPHICS_DISABLED
  if (chop_debug > 2)
    mark_outline(point);
#endif
}

// Returns true if the edgept supplied as input is an inside angle.  This
// is determined by the angular change of the vectors from point to point.
bool Wordrec::is_inside_angle(EDGEPT *pt) {
  return angle_change(pt->prev, pt, pt->next) < chop_inside_angle;
}

/**
 * @name angle_change
 *
 * Return the change in angle (degrees) of the line segments between
 * points one and two, and two and three.
 */
int Wordrec::angle_change(EDGEPT *point1, EDGEPT *point2, EDGEPT *point3) {
  VECTOR vector1;
  VECTOR vector2;

  int angle;

  /* Compute angle */
  vector1.x = point2->pos.x - point1->pos.x;
  vector1.y = point2->pos.y - point1->pos.y;
  vector2.x = point3->pos.x - point2->pos.x;
  vector2.y = point3->pos.y - point2->pos.y;
  /* Use cross product */
  float length = std::sqrt(static_cast<float>(vector1.length()) * vector2.length());
  if (static_cast<int>(length) == 0)
    return (0);
  angle = static_cast<int>(floor(asin(vector1.cross(vector2) /
                                      length) / M_PI * 180.0 + 0.5));

  /* Use dot product */
  if (vector1.dot(vector2) < 0)
    angle = 180 - angle;
  /* Adjust angle */
  if (angle > 180)
    angle -= 360;
  if (angle <= -180)
    angle += 360;
  return (angle);
}

/**
 * @name pick_close_point
 *
 * Choose the edge point that is closest to the critical point.  This
 * point may not be exactly vertical from the critical point.
 */
EDGEPT *Wordrec::pick_close_point(EDGEPT *critical_point,
                                  EDGEPT *vertical_point,
                                  int *best_dist) {
  EDGEPT *best_point = nullptr;
  int this_distance;
  int found_better;

  do {
    found_better = false;

    this_distance = edgept_dist (critical_point, vertical_point);
    if (this_distance <= *best_dist) {

      if (!(same_point (critical_point->pos, vertical_point->pos) ||
        same_point (critical_point->pos, vertical_point->next->pos) ||
        (best_point && same_point (best_point->pos, vertical_point->pos)) ||
      is_exterior_point (critical_point, vertical_point))) {
        *best_dist = this_distance;
        best_point = vertical_point;
        if (chop_vertical_creep)
          found_better = true;
      }
    }
    vertical_point = vertical_point->next;
  }
  while (found_better == true);

  return (best_point);
}


/**
 * @name prioritize_points
 *
 * Find a list of edge points from the outer outline of this blob.  For
 * each of these points assign a priority.  Sort these points using a
 * heap structure so that they can be visited in order.
 */
void Wordrec::prioritize_points(TESSLINE *outline, PointHeap* points) {
  EDGEPT *this_point;
  EDGEPT *local_min = nullptr;
  EDGEPT *local_max = nullptr;

  this_point = outline->loop;
  local_min = this_point;
  local_max = this_point;
  do {
    if (this_point->vec.y < 0) {
                                 /* Look for minima */
      if (local_max != nullptr)
        new_max_point(local_max, points);
      else if (is_inside_angle (this_point))
        add_point_to_list(points, this_point);
      local_max = nullptr;
      local_min = this_point->next;
    }
    else if (this_point->vec.y > 0) {
                                 /* Look for maxima */
      if (local_min != nullptr)
        new_min_point(local_min, points);
      else if (is_inside_angle (this_point))
        add_point_to_list(points, this_point);
      local_min = nullptr;
      local_max = this_point->next;
    }
    else {
      /* Flat area */
      if (local_max != nullptr) {
        if (local_max->prev->vec.y != 0) {
          new_max_point(local_max, points);
        }
        local_max = this_point->next;
        local_min = nullptr;
      }
      else {
        if (local_min->prev->vec.y != 0) {
          new_min_point(local_min, points);
        }
        local_min = this_point->next;
        local_max = nullptr;
      }
    }

                                 /* Next point */
    this_point = this_point->next;
  }
  while (this_point != outline->loop);
}


/**
 * @name new_min_point
 *
 * Found a new minimum point try to decide whether to save it or not.
 * Return the new value for the local minimum.  If a point is saved then
 * the local minimum is reset to nullptr.
 */
void Wordrec::new_min_point(EDGEPT *local_min, PointHeap* points) {
  int16_t dir;

  dir = direction (local_min);

  if (dir < 0) {
    add_point_to_list(points, local_min);
    return;
  }

  if (dir == 0 && point_priority (local_min) < 0) {
    add_point_to_list(points, local_min);
    return;
  }
}


/**
 * @name new_max_point
 *
 * Found a new minimum point try to decide whether to save it or not.
 * Return the new value for the local minimum.  If a point is saved then
 * the local minimum is reset to nullptr.
 */
void Wordrec::new_max_point(EDGEPT *local_max, PointHeap* points) {
  int16_t dir;

  dir = direction (local_max);

  if (dir > 0) {
    add_point_to_list(points, local_max);
    return;
  }

  if (dir == 0 && point_priority (local_max) < 0) {
    add_point_to_list(points, local_max);
    return;
  }
}


/**
 * @name vertical_projection_point
 *
 * For one point on the outline, find the corresponding point on the
 * other side of the outline that is a likely projection for a split
 * point.  This is done by iterating through the edge points until the
 * X value of the point being looked at is greater than the X value of
 * the split point.  Ensure that the point being returned is not right
 * next to the split point.  Return the edge point in *best_point as
 * a result, and any points that were newly created are also saved on
 * the new_points list.
 */
void Wordrec::vertical_projection_point(EDGEPT *split_point, EDGEPT *target_point,
                                        EDGEPT** best_point,
                                        EDGEPT_CLIST *new_points) {
  EDGEPT *p;                     /* Iterator */
  EDGEPT *this_edgept;           /* Iterator */
  EDGEPT_C_IT new_point_it(new_points);
  int x = split_point->pos.x;    /* X value of vertical */
  int best_dist = LARGE_DISTANCE;/* Best point found */

  if (*best_point != nullptr)
    best_dist = edgept_dist(split_point, *best_point);

  p = target_point;
  /* Look at each edge point */
  do {
    if (((p->pos.x <= x && x <= p->next->pos.x) ||
         (p->next->pos.x <= x && x <= p->pos.x)) &&
        !same_point(split_point->pos, p->pos) &&
        !same_point(split_point->pos, p->next->pos) &&
        !p->IsChopPt() &&
        (*best_point == nullptr || !same_point((*best_point)->pos, p->pos))) {

      if (near_point(split_point, p, p->next, &this_edgept)) {
        new_point_it.add_before_then_move(this_edgept);
      }

      if (*best_point == nullptr)
        best_dist = edgept_dist (split_point, this_edgept);

      this_edgept =
        pick_close_point(split_point, this_edgept, &best_dist);
      if (this_edgept)
        *best_point = this_edgept;
    }

    p = p->next;
  }
  while (p != target_point);
}

}  // namespace tesseract
