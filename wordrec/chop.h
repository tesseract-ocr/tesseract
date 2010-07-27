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
extern INT_VAR_H(chop_debug, 0, "Chop debug");

extern BOOL_VAR_H(chop_enable, 1, "Chop enable");

extern BOOL_VAR_H(chop_vertical_creep, 0, "Vertical creep");

extern INT_VAR_H(chop_split_length, 10000, "Split Length");

extern INT_VAR_H(chop_same_distance, 2, "Same distance");

extern INT_VAR_H(chop_min_outline_points, 6,
                 "Min Number of Points on Outline");

extern INT_VAR_H(chop_inside_angle, -50, "Min Inside Angle Bend");

extern INT_VAR_H(chop_min_outline_area, 2000, "Min Outline Area");

extern double_VAR_H(chop_split_dist_knob, 0.5, "Split length adjustment");

extern double_VAR_H(chop_overlap_knob, 0.9, "Split overlap adjustment");

extern double_VAR_H(chop_center_knob, 0.15, "Split center adjustment");

extern double_VAR_H(chop_sharpness_knob, 0.06, "Split sharpness adjustment");

extern double_VAR_H(chop_width_change_knob, 5.0, "Width change adjustment");

extern double_VAR_H(chop_ok_split, 100.0, "OK split limit");

extern double_VAR_H(chop_good_split, 50.0, "Good split limit");

extern INT_VAR_H(chop_x_y_weight, 3, "X / Y  length weight");

/*----------------------------------------------------------------------
              M a c r o s
----------------------------------------------------------------------*/
/**
 * @name point_bend_angle
 *
 * Measure the angle of bend at this edge point. The argument should
 * be of type EDGEPT.
 */
#define point_bend_angle(point) \
(angle_change ((point)->prev, (point), (point)->next))

/*----------------------------------------------------------------------
              F u n c t i o n s
----------------------------------------------------------------------*/
PRIORITY point_priority(EDGEPT *point);

void add_point_to_list(POINT_GROUP point_list, EDGEPT *point);

int angle_change(EDGEPT *point1, EDGEPT *point2, EDGEPT *point3);

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

#endif
