/* -*-C-*-
 ********************************************************************************
 *
 * File:        split.c  (Formerly split.c)
 * Description:
 * Author:       Mark Seaman, OCR Technology
 * Created:      Fri Oct 16 14:37:00 1987
 * Modified:     Fri May 17 16:27:49 1991 (Mark Seaman) marks@hpgrlt
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
 *************************************************************************/
/*----------------------------------------------------------------------
              I n c l u d e s
----------------------------------------------------------------------*/
#include "split.h"
#include "coutln.h"
#include "tprintf.h"

#ifdef __UNIX__
#include <assert.h>
#endif

/*----------------------------------------------------------------------
              V a r i a b l e s
----------------------------------------------------------------------*/
BOOL_VAR(wordrec_display_splits, 0, "Display splits");

/*----------------------------------------------------------------------
              F u n c t i o n s
----------------------------------------------------------------------*/

/**********************************************************************
 * delete_split
 *
 * Remove this split from existence.
 **********************************************************************/
void delete_split(SPLIT *split) { 
  if (split) {
    delete split;
  }
}


/**********************************************************************
 * make_edgept
 *
 * Create an EDGEPT and hook it into an existing list of edge points.
 **********************************************************************/
EDGEPT *make_edgept(int x, int y, EDGEPT *next, EDGEPT *prev) {
  EDGEPT *this_edgept;
  /* Create point */
  this_edgept = new EDGEPT;
  this_edgept->pos.x = x;
  this_edgept->pos.y = y;
  // Now deal with the src_outline steps.
  C_OUTLINE* prev_ol = prev->src_outline;
  if (prev_ol != NULL && prev->next == next) {
    // Compute the fraction of the segment that is being cut.
    FCOORD segment_vec(next->pos.x - prev->pos.x, next->pos.y - prev->pos.y);
    FCOORD target_vec(x - prev->pos.x, y - prev->pos.y);
    double cut_fraction = target_vec.length() / segment_vec.length();
    // Get the start and end at the step level.
    ICOORD step_start = prev_ol->position_at_index(prev->start_step);
    int end_step = prev->start_step + prev->step_count;
    int step_length = prev_ol->pathlength();
    ICOORD step_end = prev_ol->position_at_index(end_step % step_length);
    ICOORD step_vec = step_end - step_start;
    double target_length = step_vec.length() * cut_fraction;
    // Find the point on the segment that gives the length nearest to target.
    int best_step = prev->start_step;
    ICOORD total_step(0, 0);
    double best_dist = target_length;
    for (int s = prev->start_step; s < end_step; ++s) {
      total_step += prev_ol->step(s % step_length);
      double dist = fabs(target_length - total_step.length());
      if (dist < best_dist) {
        best_dist = dist;
        best_step = s + 1;
      }
    }
    // The new point is an intermediate point.
    this_edgept->src_outline = prev_ol;
    this_edgept->step_count = end_step - best_step;
    this_edgept->start_step = best_step % step_length;
    prev->step_count = best_step - prev->start_step;
  } else {
    // The new point is poly only.
    this_edgept->src_outline = NULL;
    this_edgept->step_count = 0;
    this_edgept->start_step = 0;
  }
  /* Hook it up */
  this_edgept->next = next;
  this_edgept->prev = prev;
  prev->next = this_edgept;
  next->prev = this_edgept;
  /* Set up vec entries */
  this_edgept->vec.x = this_edgept->next->pos.x - x;
  this_edgept->vec.y = this_edgept->next->pos.y - y;
  this_edgept->prev->vec.x = x - this_edgept->prev->pos.x;
  this_edgept->prev->vec.y = y - this_edgept->prev->pos.y;
  return this_edgept;
}

/**********************************************************************
 * remove_edgept
 *
 * Remove a given EDGEPT from its list and delete it.
 **********************************************************************/
void remove_edgept(EDGEPT *point) {
  EDGEPT *prev = point->prev;
  EDGEPT *next = point->next;
  // Add point's steps onto prev's steps if they are from the same outline.
  if (prev->src_outline == point->src_outline && prev->src_outline != NULL) {
    prev->step_count += point->step_count;
  }
  prev->next = next;
  next->prev = prev;
  prev->vec.x = next->pos.x - prev->pos.x;
  prev->vec.y = next->pos.y - prev->pos.y;
  delete point;
}

/**********************************************************************
 * new_split
 *
 * Create a new split record and initialize it.  Put it on the display
 * list.
 **********************************************************************/
SPLIT *new_split(EDGEPT *point1, EDGEPT *point2) { 
  SPLIT *s = new SPLIT;
  s->point1 = point1;
  s->point2 = point2;
  return (s);
}


/**********************************************************************
 * print_split
 *
 * Print a list of splits.  Show the coordinates of both points in
 * each split.
 **********************************************************************/
void print_split(SPLIT *split) { 
  if (split) {
    tprintf("(%d,%d)--(%d,%d)",
            split->point1->pos.x, split->point1->pos.y,
            split->point2->pos.x, split->point2->pos.y);
  }
}


/**********************************************************************
 * split_outline
 *
 * Split between these two edge points.
 **********************************************************************/
void split_outline(EDGEPT *join_point1, EDGEPT *join_point2) { 
  assert(join_point1 != join_point2);

  EDGEPT* temp2 = join_point2->next;
  EDGEPT* temp1 = join_point1->next;
  /* Create two new points */
  EDGEPT* new_point1 = make_edgept(join_point1->pos.x, join_point1->pos.y,
                                   temp1, join_point2);
  EDGEPT* new_point2 = make_edgept(join_point2->pos.x, join_point2->pos.y,
                                   temp2, join_point1);
  // Join_point1 and 2 are now cross-over points, so they must have NULL
  // src_outlines and give their src_outline information their new
  // replacements.
  new_point1->src_outline = join_point1->src_outline;
  new_point1->start_step = join_point1->start_step;
  new_point1->step_count = join_point1->step_count;
  new_point2->src_outline = join_point2->src_outline;
  new_point2->start_step = join_point2->start_step;
  new_point2->step_count = join_point2->step_count;
  join_point1->src_outline = NULL;
  join_point1->start_step = 0;
  join_point1->step_count = 0;
  join_point2->src_outline = NULL;
  join_point2->start_step = 0;
  join_point2->step_count = 0;
  join_point1->MarkChop();
  join_point2->MarkChop();
}


/**********************************************************************
 * unsplit_outlines
 *
 * Remove the split that was put between these two points.
 **********************************************************************/
void unsplit_outlines(EDGEPT *p1, EDGEPT *p2) { 
  EDGEPT *tmp1 = p1->next;
  EDGEPT *tmp2 = p2->next;

  assert (p1 != p2);

  tmp1->next->prev = p2;
  tmp2->next->prev = p1;

  // tmp2 is coincident with p1. p1 takes tmp2's place as tmp2 is deleted.
  p1->next = tmp2->next;
  p1->src_outline = tmp2->src_outline;
  p1->start_step = tmp2->start_step;
  p1->step_count = tmp2->step_count;
  // Likewise p2 takes tmp1's place.
  p2->next = tmp1->next;
  p2->src_outline = tmp1->src_outline;
  p2->start_step = tmp1->start_step;
  p2->step_count = tmp1->step_count;
  p1->UnmarkChop();
  p2->UnmarkChop();

  delete tmp1;
  delete tmp2;

  p1->vec.x = p1->next->pos.x - p1->pos.x;
  p1->vec.y = p1->next->pos.y - p1->pos.y;

  p2->vec.x = p2->next->pos.x - p2->pos.x;
  p2->vec.y = p2->next->pos.y - p2->pos.y;
}
