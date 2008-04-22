/* -*-C-*-
 ********************************************************************************
 *
 * File:        chop.h  (Formerly chop.h)
 * Description:
 * Author:       Mark Seaman, SW Productivity
 * Created:      Fri Oct 16 14:37:00 1987
 * Modified:     Wed Jul 10 14:47:37 1991 (Mark Seaman) marks@hpgrlt
 * Language:     C
 * Package:      N/A
 * Status:       Reusable Software Component
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
 *******************************************************************************/

#ifndef CHOP_H
#define CHOP_H

/*----------------------------------------------------------------------
              I n c l u d e s
----------------------------------------------------------------------*/
#include "oldheap.h"
#include "seam.h"

/*----------------------------------------------------------------------
              T y p e s
---------------------------------------------------------------------*/
#define MAX_NUM_POINTS 50
typedef HEAP *POINT_GROUP;
typedef HEAP *SPLIT_GROUP;

/*----------------------------------------------------------------------
              V a r i a b l e s
----------------------------------------------------------------------*/
extern int chop_enable;
extern int chop_debug;
extern int same_distance;
extern int split_length;
extern float center_knob;
extern float overlap_knob;
extern float split_dist_knob;
extern float width_change_knob;
extern float sharpness_knob;
extern int min_outline_area;
extern int min_outline_points;
extern float good_split;
extern float ok_split;
extern int chop_enable;

/*----------------------------------------------------------------------
              M a c r o s
----------------------------------------------------------------------*/
/**********************************************************************
 * point_bend_angle
 *
 * Measure the angle of bend at this edge point. The argument should
 * be of type EDGEPT.
 **********************************************************************/
#define point_bend_angle(point) \
(angle_change ((point)->prev, (point), (point)->next))

/*----------------------------------------------------------------------
              F u n c t i o n s
----------------------------------------------------------------------*/
PRIORITY point_priority(EDGEPT *point);

void add_point_to_list(POINT_GROUP point_list, EDGEPT *point);

int angle_change(EDGEPT *point1, EDGEPT *point2, EDGEPT *point3);

void init_chop();

int is_little_chunk(EDGEPT *point1, EDGEPT *point2);

int is_small_area(EDGEPT *point1, EDGEPT *point2);

EDGEPT *pick_close_point(EDGEPT *critical_point,
                         EDGEPT *vertical_point,
                         int *best_dist);

void prioritize_points(TESSLINE *outline, POINT_GROUP points);

void new_min_point(EDGEPT *local_min, POINT_GROUP points);

void new_max_point(EDGEPT *local_max, POINT_GROUP points);

void vertical_projection_point(EDGEPT *split_point, EDGEPT *target_point,
                               EDGEPT** best_point);

/*
#if defined(__STDC__) || defined(__cplusplus)
# define	_ARGS(s) s
#else
# define	_ARGS(s) ()
#endif*/

/* chop.c
PRIORITY point_priority
  _ARGS((EDGEPT *point));

void add_point_to_list
  _ARGS((POINT_GROUP point_list,
  EDGEPT *point));

SEAM *create_split
  _ARGS((BLOB *blob,
  POINT_GROUP points));

SPLIT *extended_split
  _ARGS((inT32 location,
  EDGEPT *starting_point));

SEAM *get_best_pair
  _ARGS((SPLIT_GROUP split_queue,
  BLOB *blob));

void init_chop
  _ARGS((void));

EDGEPT *pick_close_point
  _ARGS((EDGEPT *critical_point,
  EDGEPT *vertical_point,
  int *best_dist));

void prioritize_points
  _ARGS((TESSLINE *outline,
  POINT_GROUP points));

void new_min_point
  _ARGS((EDGEPT *local_min,
  POINT_GROUP points));

void new_max_point
  _ARGS((EDGEPT *local_max,
  POINT_GROUP points));

EDGEPT *vertical_projection_point
  _ARGS((EDGEPT *split_point,
  EDGEPT *target_point));

#undef _ARGS
*/
#endif
