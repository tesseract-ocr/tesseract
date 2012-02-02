/* -*-C-*-
 ********************************************************************************
 *
 * File:        outlines.c  (Formerly outlines.c)
 * Description:  Combinatorial Splitter
 * Author:       Mark Seaman, OCR Technology
 * Created:      Thu Jul 27 08:59:01 1989
 * Modified:     Wed Jul 10 14:56:49 1991 (Mark Seaman) marks@hpgrlt
 * Language:     C
 * Package:      N/A
 * Status:       Experimental (Do Not Distribute)
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
 ********************************************************************************
 * Revision 1.2  89/09/15  09:24:41  09:24:41  marks (Mark Seaman)
 * First released version of Combinatorial splitter code
**/
/*----------------------------------------------------------------------
              I n c l u d e s
----------------------------------------------------------------------*/
#include "outlines.h"
#include "wordrec.h"

#ifdef __UNIX__
#include <assert.h>
#endif

namespace tesseract {
/*----------------------------------------------------------------------
              F u n c t i o n s
----------------------------------------------------------------------*/
/**********************************************************************
 * crosses_outline
 *
 * Check to see if this line crosses over this outline.  If it does
 * return TRUE.
 **********************************************************************/
int Wordrec::crosses_outline(EDGEPT *p0,         /* Start of line */
                             EDGEPT *p1,         /* End of line */
                             EDGEPT *outline) {  /* Outline to check */
  EDGEPT *pt = outline;
  do {
    if (is_crossed (p0->pos, p1->pos, pt->pos, pt->next->pos))
      return (TRUE);
    pt = pt->next;
  }
  while (pt != outline);
  return (FALSE);
}


/**********************************************************************
 * is_crossed
 *
 * Return TRUE when the two line segments cross each other.  Find out
 * where the projected lines would cross and then check to see if the
 * point of intersection lies on both of the line segments. If it does
 * then these two segments cross.
 **********************************************************************/
int Wordrec::is_crossed(TPOINT a0, TPOINT a1, TPOINT b0, TPOINT b1) {
  int b0a1xb0b1, b0b1xb0a0;
  int a1b1xa1a0, a1a0xa1b0;

  TPOINT b0a1, b0a0, a1b1, b0b1, a1a0;

  b0a1.x = a1.x - b0.x;
  b0a0.x = a0.x - b0.x;
  a1b1.x = b1.x - a1.x;
  b0b1.x = b1.x - b0.x;
  a1a0.x = a0.x - a1.x;
  b0a1.y = a1.y - b0.y;
  b0a0.y = a0.y - b0.y;
  a1b1.y = b1.y - a1.y;
  b0b1.y = b1.y - b0.y;
  a1a0.y = a0.y - a1.y;

  b0a1xb0b1 = CROSS (b0a1, b0b1);
  b0b1xb0a0 = CROSS (b0b1, b0a0);
  a1b1xa1a0 = CROSS (a1b1, a1a0);
                                 /*a1a0xa1b0=CROSS(a1a0,a1b0); */
  a1a0xa1b0 = -CROSS (a1a0, b0a1);

  return ((b0a1xb0b1 > 0 && b0b1xb0a0 > 0)
    || (b0a1xb0b1 < 0 && b0b1xb0a0 < 0))
    && ((a1b1xa1a0 > 0 && a1a0xa1b0 > 0) || (a1b1xa1a0 < 0 && a1a0xa1b0 < 0));
}


/**********************************************************************
 * is_same_edgept
 *
 * Return true if the points are identical.
 **********************************************************************/
int Wordrec::is_same_edgept(EDGEPT *p1, EDGEPT *p2) {
  return (p1 == p2);
}


/**********************************************************************
 * near_point
 *
 * Find the point on a line segment that is closest to a point not on
 * the line segment.  Return that point in near_pt.  Returns whether
 * near_pt was newly created.
 **********************************************************************/
bool Wordrec::near_point(EDGEPT *point,
                         EDGEPT *line_pt_0, EDGEPT *line_pt_1,
                         EDGEPT **near_pt) {
  TPOINT p;

  float slope;
  float intercept;

  float x0 = line_pt_0->pos.x;
  float x1 = line_pt_1->pos.x;
  float y0 = line_pt_0->pos.y;
  float y1 = line_pt_1->pos.y;

  if (x0 == x1) {
                                 /* Handle vertical line */
    p.x = (inT16) x0;
    p.y = point->pos.y;
  }
  else {
    /* Slope and intercept */
    slope = (y0 - y1) / (x0 - x1);
    intercept = y1 - x1 * slope;

    /* Find perpendicular */
    p.x = (inT16) ((point->pos.x + (point->pos.y - intercept) * slope) /
      (slope * slope + 1));
    p.y = (inT16) (slope * p.x + intercept);
  }

  if (is_on_line (p, line_pt_0->pos, line_pt_1->pos) &&
    (!same_point (p, line_pt_0->pos)) && (!same_point (p, line_pt_1->pos))) {
    /* Intersection on line */
    *near_pt = make_edgept(p.x, p.y, line_pt_1, line_pt_0);
    return true;
  } else {                           /* Intersection not on line */
    *near_pt = closest(point, line_pt_0, line_pt_1);
    return false;
  }
}


/**********************************************************************
 * reverse_outline
 *
 * Change the direction of the outline.  If it was clockwise make it
 * counter-clockwise and vice versa.  Do this by swapping each of the
 * next and prev fields of each edge point.
 **********************************************************************/
void Wordrec::reverse_outline(EDGEPT *outline) {
  EDGEPT *edgept = outline;
  EDGEPT *temp;

  do {
                                 /* Swap next and prev */
    temp = edgept->prev;
    edgept->prev = edgept->next;
    edgept->next = temp;
    /* Set up vec field */
    edgept->vec.x = edgept->next->pos.x - edgept->pos.x;
    edgept->vec.y = edgept->next->pos.y - edgept->pos.y;

    edgept = edgept->prev;       /* Go to next point */
  }
  while (edgept != outline);
}

}  // namespace tesseract
