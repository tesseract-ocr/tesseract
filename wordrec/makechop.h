/* -*-C-*-
 ********************************************************************************
 *
 * File:        makechop.h  (Formerly makechop.h)
 * Description:
 * Author:	Mark Seaman, SW Productivity
 * Created:	Fri Oct 16 14:37:00 1987
 * Modified:     Mon Jul 29 13:33:23 1991 (Mark Seaman) marks@hpgrlt
 * Language:	C
 * Package:	N/A
 * Status:	Reusable Software Component
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
#ifndef MAKECHOP_H
#define MAKECHOP_H

/*----------------------------------------------------------------------
      I n c l u d e s
----------------------------------------------------------------------*/
#include "chop.h"
#include "olutil.h"

/*----------------------------------------------------------------------
      M a c r o s
---------------------------------------------------------------------*/
/**********************************************************************
 * is_split_outline
 *
 * Check to see if both sides of the split fall within the bounding
 * box of this outline.
 **********************************************************************/

#define is_split_outline(outline,split)          \
(outline->Contains(split->point1->pos) &&  \
	outline->Contains(split->point2->pos))    \


/*----------------------------------------------------------------------
        Public Function Prototypes
----------------------------------------------------------------------*/
void apply_seam(TBLOB *blob, TBLOB *other_blob, bool italic_blob, SEAM *seam);

void form_two_blobs(TBLOB *blob, TBLOB *other_blob, bool italic_blob,
                    const TPOINT& location);

void make_double_split(TBLOB *blob, TBLOB *other_blob, bool italic_blob,
                       SEAM *seam);

void make_single_split(TESSLINE *outlines, SPLIT *split);

void make_split_blobs(TBLOB *blob, TBLOB *other_blob, bool italic_blob,
                      SEAM *seam);

void make_triple_split(TBLOB *blob, TBLOB *other_blob, bool italic_blob,
                       SEAM *seam);

void undo_seam(TBLOB *blob, TBLOB *other_blob, SEAM *seam);

void undo_single_split(TBLOB *blob, SPLIT *split);
#endif
