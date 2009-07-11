/* -*-C-*-
 ********************************************************************************
 *
 * File:        gradechop.h  (Formerly gradechop.h)
 * Description:
 * Author:       Mark Seaman, SW Productivity
 * Created:      Fri Oct 16 14:37:00 1987
 * Modified:     Tue Jul  9 16:40:39 1991 (Mark Seaman) marks@hpgrlt
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

#ifndef GRADECHOP_H
#define GRADECHOP_H

/*----------------------------------------------------------------------
              I n c l u d e s
----------------------------------------------------------------------*/
#include "seam.h"
#include "ndminx.h"

/*----------------------------------------------------------------------
              T y p e s
----------------------------------------------------------------------*/
typedef inT16 BOUNDS_RECT[4];

/*----------------------------------------------------------------------
              M a c r o s
----------------------------------------------------------------------*/
/**********************************************************************
 * partial_split_priority
 *
 * Assign a priority to this split based on the features that it has.
 * Grade it according to the different rating schemes and return the
 * value of its goodness.
 **********************************************************************/

#define partial_split_priority(split)  \
(grade_split_length   (split) +      \
	grade_sharpness      (split))       \


/**********************************************************************
 * split_bounds_overlap
 *
 * Check to see if this split might overlap with this outline.  Return
 * TRUE if there is a positive overlap in the bounding boxes of the two.
 **********************************************************************/

#define split_bounds_overlap(split,outline)  \
(outline->topleft.x  <= MAX (split->point1->pos.x,split->point2->pos.x) && \
	outline->botright.x >= MIN (split->point1->pos.x,split->point2->pos.x) && \
	outline->botright.y <= MAX (split->point1->pos.y,split->point2->pos.y) && \
	outline->topleft.y  >= MIN (split->point1->pos.y,split->point2->pos.y))

/*----------------------------------------------------------------------
              F u n c t i o n s
----------------------------------------------------------------------*/
PRIORITY full_split_priority(SPLIT *split, inT16 xmin, inT16 xmax);

PRIORITY grade_center_of_blob(register BOUNDS_RECT rect);

PRIORITY grade_overlap(register BOUNDS_RECT rect);

PRIORITY grade_split_length(register SPLIT *split);

PRIORITY grade_sharpness(register SPLIT *split);

PRIORITY grade_width_change(register BOUNDS_RECT rect);

void set_outline_bounds(register EDGEPT *point1,
                        register EDGEPT *point2,
                        BOUNDS_RECT rect);
#endif
