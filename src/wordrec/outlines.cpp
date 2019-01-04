/* -*-C-*-
 ********************************************************************************
 *
 * File:         outlines.cpp  (Formerly outlines.c)
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

namespace tesseract {
/*----------------------------------------------------------------------
              F u n c t i o n s
----------------------------------------------------------------------*/
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
    p.x = (int16_t) x0;
    p.y = point->pos.y;
  }
  else {
    /* Slope and intercept */
    slope = (y0 - y1) / (x0 - x1);
    intercept = y1 - x1 * slope;

    /* Find perpendicular */
    p.x = (int16_t) ((point->pos.x + (point->pos.y - intercept) * slope) /
      (slope * slope + 1));
    p.y = (int16_t) (slope * p.x + intercept);
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

}  // namespace tesseract
