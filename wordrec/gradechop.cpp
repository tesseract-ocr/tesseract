/* -*-C-*-
 ********************************************************************************
 *
 * File:        gradechop.c  (Formerly gradechop.c)
 * Description:
 * Author:       Mark Seaman, OCR Technology
 * Created:      Fri Oct 16 14:37:00 1987
 * Modified:     Tue Jul 30 16:06:27 1991 (Mark Seaman) marks@hpgrlt
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
 *********************************************************************************/
/*----------------------------------------------------------------------
              I n c l u d e s
----------------------------------------------------------------------*/
#include "gradechop.h"
#include "wordrec.h"
#include "olutil.h"
#include "chop.h"
#include "ndminx.h"
#include <math.h>

/*----------------------------------------------------------------------
              T y p e s
----------------------------------------------------------------------*/
#define CENTER_GRADE_CAP 25.0

/*----------------------------------------------------------------------
              M a c r o s
----------------------------------------------------------------------*/
/**********************************************************************
 * find_bounds_loop
 *
 * This is a macro to be used by set_outline_bounds.
 **********************************************************************/

#define find_bounds_loop(point1,point2,x_min,x_max)     \
	x_min = point2->pos.x;                               \
	x_max = point2->pos.x;                               \
																		\
	this_point = point1;                                 \
	do {                                                 \
		x_min = MIN (this_point->pos.x, x_min);           \
		x_max = MAX (this_point->pos.x, x_max);           \
		this_point = this_point->next;                    \
	}                                                    \
	while (this_point != point2 && this_point != point1) \


namespace tesseract {

/*----------------------------------------------------------------------
              F u n c t i o n s
----------------------------------------------------------------------*/
/**********************************************************************
 * full_split_priority
 *
 * Assign a priority to this split based on the features that it has.
 * Part of the priority has already been calculated so just return the
 * additional amount for the bounding box type information.
 **********************************************************************/
PRIORITY Wordrec::full_split_priority(SPLIT *split, inT16 xmin, inT16 xmax) {
  BOUNDS_RECT rect;

  set_outline_bounds (split->point1, split->point2, rect);

  if (xmin < MIN (rect[0], rect[2]) && xmax > MAX (rect[1], rect[3]))
    return (999.0);

  return (grade_overlap (rect) +
    grade_center_of_blob (rect) + grade_width_change (rect));
}


/**********************************************************************
 * grade_center_of_blob
 *
 * Return a grade for the a split.  Rank it on closeness to the center
 * of the original blob
 *   0    =  "perfect"
 *   100  =  "no way jay"
 **********************************************************************/
PRIORITY Wordrec::grade_center_of_blob(register BOUNDS_RECT rect) {
  register PRIORITY grade;
  int width1 = rect[1] - rect[0];
  int width2 = rect[3] - rect[2];

  if (width1 > chop_centered_maxwidth &&
      width2 > chop_centered_maxwidth) {
    return 0.0;
  }

  grade = width1 - width2;
  if (grade < 0)
    grade = -grade;

  grade *= chop_center_knob;
  grade = MIN (CENTER_GRADE_CAP, grade);
  return (MAX (0.0, grade));
}


/**********************************************************************
 * grade_overlap
 *
 * Return a grade for this split for the overlap of the resultant blobs.
 *   0    =  "perfect"
 *   100  =  "no way jay"
 **********************************************************************/
PRIORITY Wordrec::grade_overlap(register BOUNDS_RECT rect) {
  register PRIORITY grade;
  register inT16 width1;
  register inT16 width2;
  register inT16 overlap;

  width1 = rect[3] - rect[2];
  width2 = rect[1] - rect[0];

  overlap = MIN (rect[1], rect[3]) - MAX (rect[0], rect[2]);
  width1 = MIN (width1, width2);
  if (overlap == width1)
    return (100.0);              /* Total overlap */

  width1 = 2 * overlap - width1; /* Extra penalty for too */
  overlap += MAX (0, width1);    /* much overlap */

  grade = overlap * chop_overlap_knob;

  return (MAX (0.0, grade));
}


/**********************************************************************
 * grade_split_length
 *
 * Return a grade for the length of this split.
 *   0    =  "perfect"
 *   100  =  "no way jay"
 **********************************************************************/
PRIORITY Wordrec::grade_split_length(register SPLIT *split) {
  register PRIORITY grade;
  register float split_length;

  split_length = weighted_edgept_dist (split->point1, split->point2,
    chop_x_y_weight);

  if (split_length <= 0)
    grade = 0;
  else
    grade = sqrt (split_length) * chop_split_dist_knob;

  return (MAX (0.0, grade));
}


/**********************************************************************
 * grade_sharpness
 *
 * Return a grade for the sharpness of this split.
 *   0    =  "perfect"
 *   100  =  "no way jay"
 **********************************************************************/
PRIORITY Wordrec::grade_sharpness(register SPLIT *split) {
  register PRIORITY grade;

  grade = point_priority (split->point1) + point_priority (split->point2);

  if (grade < -360.0)
    grade = 0;
  else
    grade += 360.0;

  grade *= chop_sharpness_knob;       /* Values 0 to -360 */

  return (grade);
}


/**********************************************************************
 * grade_width_change
 *
 * Return a grade for the change in width of the resultant blobs.
 *   0    =  "perfect"
 *   100  =  "no way jay"
 **********************************************************************/
PRIORITY Wordrec::grade_width_change(register BOUNDS_RECT rect) {
  register PRIORITY grade;
  register inT32 width1;
  register inT32 width2;

  width1 = rect[3] - rect[2];
  width2 = rect[1] - rect[0];

  grade = 20 - (MAX (rect[1], rect[3])
    - MIN (rect[0], rect[2]) - MAX (width1, width2));

  grade *= chop_width_change_knob;

  return (MAX (0.0, grade));
}


/**********************************************************************
 * set_outline_bounds
 *
 * Set up the limits for the x coordinate of the outline.
 **********************************************************************/
void Wordrec::set_outline_bounds(register EDGEPT *point1,
                                 register EDGEPT *point2,
                                 BOUNDS_RECT rect) {
  register EDGEPT *this_point;
  register inT16 x_min;
  register inT16 x_max;

  find_bounds_loop(point1, point2, x_min, x_max);

  rect[0] = x_min;
  rect[1] = x_max;

  find_bounds_loop(point2, point1, x_min, x_max);

  rect[2] = x_min;
  rect[3] = x_max;
}

}  // namespace tesseract
