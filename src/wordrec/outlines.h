/* -*-C-*-
 ********************************************************************************
 *
 * File:        outlines.h  (Formerly outlines.h)
 * Description:  Combinatorial Splitter
 * Author:       Mark Seaman, OCR Technology
 * Created:      Thu Jul 27 11:27:55 1989
 * Modified:     Wed May 15 17:28:47 1991 (Mark Seaman) marks@hpgrlt
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
 *********************************************************************************/
#ifndef OUTLINES_H
#define OUTLINES_H

#include "blobs.h"
#include "chop.h"

#include <math.h>

/*----------------------------------------------------------------------
              C o n s t a n t s
----------------------------------------------------------------------*/
#define LARGE_DISTANCE   100000  /* Used for closest dist */
#define MIN_BLOB_SIZE    10      /* Big units */
#define MAX_ASPECT_RATIO 2.5     /* Widest character */

/*----------------------------------------------------------------------
              M a c r o s
----------------------------------------------------------------------*/
/**********************************************************************
 * same_point
 *
 * Return TRUE if the point values are the same. The parameters must
 * be of type POINT.
 **********************************************************************/
#define same_point(p1,p2)                    \
	((abs (p1.x - p2.x) < chop_same_distance) && \
	(abs (p1.y - p2.y) < chop_same_distance))

/**********************************************************************
 * dist_square
 *
 * Return the square of the distance between these two points.  The
 * parameters must be of type POINT.
 **********************************************************************/

#define dist_square(p1,p2)                     \
	((p2.x - p1.x) * (p2.x - p1.x) +            \
	(p2.y - p1.y) * (p2.y - p1.y))

/**********************************************************************
 * closest
 *
 * The expression provides the EDGEPT that is closest to the point in
 * question.  All three parameters must be of type EDGEPT.
 **********************************************************************/

#define closest(test_p,p1,p2)                   \
(p1 ?                                         \
	(p2 ?                                        \
	((dist_square (test_p->pos, p1->pos) <      \
		dist_square (test_p->pos, p2->pos)) ?     \
	p1  :                                      \
	p2) :                                      \
	p1)  :                                      \
	p2)

/**********************************************************************
 * edgept_dist
 *
 * Return the distance (squared) between the two edge points.
 **********************************************************************/

#define edgept_dist(p1,p2)  \
(dist_square ((p1)->pos, (p2)->pos))

/**********************************************************************
 * is_exterior_point
 *
 * Return TRUE if the point supplied is an exterior projection from the
 * outline.
 **********************************************************************/

#define is_exterior_point(edge,point)                    \
(same_point (edge->prev->pos, point->pos)  ||          \
	same_point (edge->next->pos, point->pos)  ||          \
	(angle_change (edge->prev, edge, edge->next) -   \
	angle_change (edge->prev, edge, point) > 20))

/**********************************************************************
 * is_equal
 *
 * Return TRUE if the POINTs are equal.
 **********************************************************************/

#define is_equal(p1,p2)  \
(((p1).x == (p2).x) && ((p1).y == (p2).y))

/**********************************************************************
 * is_on_line
 *
 * Return TRUE if the point is on the line segment between the two end
 * points.  The two end points are included as part of the  line.  The
 * parameters must be of type POINT.
 **********************************************************************/

#define is_on_line(p,p0,p1)                  \
	(within_range ((p).x, (p0).x, (p1).x) &&  \
	within_range ((p).y, (p0).y, (p1).y))

/**********************************************************************
 * within_range
 *
 * Return TRUE if the first number is in between the second two numbers.
 * Return FALSE otherwise.
 **********************************************************************/

#define within_range(x,x0,x1) \
	(((x0 <= x) && (x <= x1)) || ((x1 <= x) && (x <= x0)))

#endif
