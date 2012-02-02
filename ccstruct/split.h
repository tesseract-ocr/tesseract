/* -*-C-*-
 ********************************************************************************
 *
 * File:        split.h  (Formerly split.h)
 * Description:
 * Author:       Mark Seaman, SW Productivity
 * Created:      Fri Oct 16 14:37:00 1987
 * Modified:     Mon May 13 10:49:23 1991 (Mark Seaman) marks@hpgrlt
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
 *****************************************************************************/
#ifndef SPLIT_H
#define SPLIT_H

/*----------------------------------------------------------------------
              I n c l u d e s
----------------------------------------------------------------------*/
#include "blobs.h"
#include "oldlist.h"

/*----------------------------------------------------------------------
              T y p e s
----------------------------------------------------------------------*/
typedef struct split_record
{                                /*  SPLIT  */
  EDGEPT *point1;
  EDGEPT *point2;
} SPLIT;

typedef LIST SPLITS;             /*  SPLITS  */

/*----------------------------------------------------------------------
              V a r i a b l e s
----------------------------------------------------------------------*/

extern BOOL_VAR_H(wordrec_display_splits, 0, "Display splits");

/*----------------------------------------------------------------------
              M a c r o s
----------------------------------------------------------------------*/
/**********************************************************************
 * clone_split
 *
 * Create a new split record and set the contents equal to the contents
 * of this record.
 **********************************************************************/

#define clone_split(dest,source)                               \
if (source)                                                  \
	(dest) = new_split ((source)->point1, (source)->point2);  \
else                                                         \
	(dest) = (SPLIT*) NULL                                    \


/*----------------------------------------------------------------------
              F u n c t i o n s
----------------------------------------------------------------------*/
void delete_split(SPLIT *split);

EDGEPT *make_edgept(int x, int y, EDGEPT *next, EDGEPT *prev);

void remove_edgept(EDGEPT *point);

SPLIT *new_split(EDGEPT *point1, EDGEPT *point2);

void print_split(SPLIT *split);

void split_outline(EDGEPT *join_point1, EDGEPT *join_point2);

void unsplit_outlines(EDGEPT *p1, EDGEPT *p2);

#endif
