/* -*-C-*-
 ********************************************************************************
 *
 * File:        seam.h  (Formerly seam.h)
 * Description:
 * Author:       Mark Seaman, SW Productivity
 * Created:      Fri Oct 16 14:37:00 1987
 * Modified:     Thu May 16 17:05:52 1991 (Mark Seaman) marks@hpgrlt
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
#ifndef SEAM_H
#define SEAM_H

/*----------------------------------------------------------------------
              I n c l u d e s
----------------------------------------------------------------------*/
#include "blobs.h"
#include "split.h"
#include "tessarray.h"

/*----------------------------------------------------------------------
              T y p e s
----------------------------------------------------------------------*/
typedef float PRIORITY;          /*  PRIORITY  */

typedef struct seam_record
{                                /*  SEAM  */
  PRIORITY priority;
  inT8 widthp;
  inT8 widthn;
  TPOINT location;
  SPLIT *split1;
  SPLIT *split2;
  SPLIT *split3;
} SEAM;

typedef ARRAY SEAMS;             /*  SEAMS  */

extern SEAM *newseam();

/*----------------------------------------------------------------------
              M a c r o s
----------------------------------------------------------------------*/
/**
 * @name clone_seam
 *
 * Create a new seam record and copy the contents of this seam into it.
 */

#define clone_seam(dest,source)                    \
if (source) {                                      \
  (dest) = newseam ();                             \
  (dest)->location = (source)->location;           \
  (dest)->widthp = (source)->widthp;               \
  (dest)->widthn = (source)->widthn;               \
  (dest)->priority = (source)->priority;           \
  clone_split ((dest)->split1, (source)->split1);  \
  clone_split ((dest)->split2, (source)->split2);  \
  clone_split ((dest)->split3, (source)->split3);  \
}                                                  \
else {                                             \
  (dest) = (SEAM*) NULL;                           \
}                                                  \


/**
 * exact_point
 *
 * Return TRUE if the point positions are the exactly the same. The
 * parameters must be of type (EDGEPT*).
 */

#define exact_point(p1,p2)                    \
        (! ((p1->pos.x - p2->pos.x) || (p1->pos.y - p2->pos.y)))

/*----------------------------------------------------------------------
              F u n c t i o n s
----------------------------------------------------------------------*/
bool point_in_split(SPLIT *split, EDGEPT *point1, EDGEPT *point2);

bool point_in_seam(SEAM *seam, SPLIT *split);

bool point_used_by_split(SPLIT *split, EDGEPT *point);

bool point_used_by_seam(SEAM *seam, EDGEPT *point);

SEAMS add_seam(SEAMS seam_list, SEAM *seam);

void combine_seams(SEAM *dest_seam, SEAM *source_seam);

void delete_seam(void *arg);  //SEAM  *seam);

SEAMS start_seam_list(TBLOB *blobs);

void free_seam_list(SEAMS seam_list);

bool test_insert_seam(SEAMS seam_list,
                      int index,
                      TBLOB *left_blob,
                      TBLOB *first_blob);

SEAMS insert_seam(SEAMS seam_list,
                  int index,
                  SEAM *seam,
                  TBLOB *left_blob,
                  TBLOB *first_blob);

int account_splits_right(SEAM *seam, TBLOB *blob);

int account_splits_left(SEAM *seam, TBLOB *blob, TBLOB *end_blob);

void account_splits_left_helper(SEAM *seam, TBLOB *blob, TBLOB *end_blob,
                                inT32 *depth, inT8 *width, inT8 *found_em);

bool find_split_in_blob(SPLIT *split, TBLOB *blob);

SEAM *join_two_seams(SEAM *seam1, SEAM *seam2);

SEAM *new_seam(PRIORITY priority,
               const TPOINT& location,
               SPLIT *split1,
               SPLIT *split2,
               SPLIT *split3);

SEAMS new_seam_list();

void print_seam(const char *label, SEAM *seam);

void print_seams(const char *label, SEAMS seams);

int shared_split_points(SEAM *seam1, SEAM *seam2);

void break_pieces(TBLOB *blobs, SEAMS seams, inT16 start, inT16 end);

void join_pieces(TBLOB *piece_blobs, SEAMS seams, inT16 start, inT16 end);

void hide_seam(SEAM *seam);

void hide_edge_pair(EDGEPT *pt1, EDGEPT *pt2);

void reveal_seam(SEAM *seam);

void reveal_edge_pair(EDGEPT *pt1, EDGEPT *pt2);

#endif
