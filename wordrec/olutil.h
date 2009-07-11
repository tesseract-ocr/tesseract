/* -*-C-*-
 ********************************************************************************
 *
 * File:        olutil.h  (Formerly olutil.h)
 * Description:
 * Author:       Mark Seaman, SW Productivity
 * Created:      Fri Oct 16 14:37:00 1987
 * Modified:     Wed Jul 10 14:21:55 1991 (Mark Seaman) marks@hpgrlt
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
#ifndef OLUTIL_H
#define OLUTIL_H

/*----------------------------------------------------------------------
              I n c l u d e s
----------------------------------------------------------------------*/
#include "tessclas.h"
#include "general.h"

/*----------------------------------------------------------------------
              M a c r o s
----------------------------------------------------------------------*/
/**********************************************************************
 * is_inside_angle
 *
 * Return true if the edgept supplied as input is an inside angle.  This
 * is determined by the angular change of the vectors from point to
 * point.

 **********************************************************************/

#define is_inside_angle(pt)                                  \
(angle_change ((pt)->prev, (pt), (pt)->next) < chop_inside_angle)

/**********************************************************************
 * point_in_outline
 *
 * Check to see if this point falls within the bounding box of this
 * outline.  Note that this does not totally ensure that the edge
 * point falls on this outline.
 **********************************************************************/

#define point_in_outline(p,o)         \
((p)->pos.x >= (o)->topleft.x  &&   \
	(p)->pos.y <= (o)->topleft.y  &&   \
	(p)->pos.x <= (o)->botright.x &&   \
	(p)->pos.y >= (o)->botright.y)     \


/**********************************************************************
 * same_outline_bounds
 *
 * Return TRUE if these two outlines have the same bounds.
 **********************************************************************/

#define same_outline_bounds(outline,other_outline)     \
(outline->topleft.x  == other_outline->topleft.x  && \
	outline->topleft.y  == other_outline->topleft.y  && \
	outline->botright.x == other_outline->botright.x && \
	outline->botright.y == other_outline->botright.y)   \


/**********************************************************************
 * weighted_edgept_dist
 *
 * Return the distance (squared) between the two edge points.
 **********************************************************************/

#define weighted_edgept_dist(p1,p2,chop_x_y_weight)  \
(((p1)->pos.x - (p2)->pos.x) *                \
	((p1)->pos.x - (p2)->pos.x) * chop_x_y_weight +  \
	((p1)->pos.y - (p2)->pos.y) *               \
	((p1)->pos.y - (p2)->pos.y))

/*----------------------------------------------------------------------
              F u n c t i o n s
----------------------------------------------------------------------*/
void check_outline_mem(); 

void correct_blob_order(TBLOB *blob1, TBLOB *blob2); 

void eliminate_duplicate_outlines(TBLOB *blob); 

void setup_outline(TESSLINE *outline); 

void setup_blob_outlines(TBLOB *blob); 

#endif
