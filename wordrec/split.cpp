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
#include "structures.h"
#include "hideedge.h"
#include "callcpp.h"

#ifdef __UNIX__
#include <assert.h>
#endif

/*----------------------------------------------------------------------
              V a r i a b l e s
----------------------------------------------------------------------*/
BOOL_VAR(wordrec_display_splits, 0, "Display splits");

#define SPLITBLOCK 100           /* Cells per block */
makestructure (newsplit, free_split, printsplit, SPLIT,
freesplit, SPLITBLOCK, "SPLIT", splitcount);

/*----------------------------------------------------------------------
              F u n c t i o n s
----------------------------------------------------------------------*/

/**********************************************************************
 * delete_split
 *
 * Remove this split from existance.  Take if off the display list and
 * deallocate its memory.
 **********************************************************************/
void delete_split(SPLIT *split) { 
  if (split) {
    free_split(split); 
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
  this_edgept = newedgept ();
  this_edgept->pos.x = x;
  this_edgept->pos.y = y;
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

  reveal_edge(this_edgept); 
  this_edgept->flags[1] = 0;

  return (this_edgept);
}


/**********************************************************************
 * new_split
 *
 * Create a new split record and initialize it.  Put it on the display
 * list.
 **********************************************************************/
SPLIT *new_split(EDGEPT *point1, EDGEPT *point2) { 
  SPLIT *s;
  s = (SPLIT *) newsplit ();
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
    cprintf ("(%d,%d)--(%d,%d)",
      split->point1->pos.x, split->point1->pos.y,
      split->point2->pos.x, split->point2->pos.y);
  }
}


/**********************************************************************
 * split_outline
 *
 * Split between these two edge points. Apply a split and return a
 * pointer to the other side of the split.
 **********************************************************************/
void split_outline(EDGEPT *join_point1, EDGEPT *join_point2) { 
  EDGEPT *join_point1a;
  EDGEPT *temp2;
  EDGEPT *temp1;

  assert (join_point1 != join_point2);

  temp2 = join_point2->next;
  temp1 = join_point1->next;
  /* Create two new points */
  join_point1a = make_edgept (join_point1->pos.x,
    join_point1->pos.y, temp1, join_point2);

  make_edgept (join_point2->pos.x, join_point2->pos.y, temp2, join_point1);
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

  p1->next = tmp2->next;
  p2->next = tmp1->next;

  oldedgept(tmp1); 
  oldedgept(tmp2); 

  p1->vec.x = p1->next->pos.x - p1->pos.x;
  p1->vec.y = p1->next->pos.y - p1->pos.y;

  p2->vec.x = p2->next->pos.x - p2->pos.x;
  p2->vec.y = p2->next->pos.y - p2->pos.y;
}
