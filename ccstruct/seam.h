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

/*----------------------------------------------------------------------
              T y p e s
----------------------------------------------------------------------*/
typedef float PRIORITY;          /*  PRIORITY  */

struct SEAM {
  // Constructor that was formerly new_seam.
  SEAM(PRIORITY priority0, const TPOINT& location0,
       SPLIT *splita, SPLIT *splitb, SPLIT *splitc)
  : priority(priority0), widthp(0), widthn(0), location(location0),
    split1(splita), split2(splitb), split3(splitc) {}
  // Copy constructor that was formerly clone_seam.
  SEAM(const SEAM& src)
  : priority(src.priority), widthp(src.widthp), widthn(src.widthn),
    location(src.location) {
    clone_split(split1, src.split1);
    clone_split(split2, src.split2);
    clone_split(split3, src.split3);
  }
  // Destructor was delete_seam.
  ~SEAM() {
    if (split1)
      delete_split(split1);
    if (split2)
      delete_split(split2);
    if (split3)
      delete_split(split3);
  }

  PRIORITY priority;
  inT8 widthp;
  inT8 widthn;
  TPOINT location;
  SPLIT *split1;
  SPLIT *split2;
  SPLIT *split3;
};

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

bool point_in_seam(const SEAM *seam, SPLIT *split);

bool point_used_by_split(SPLIT *split, EDGEPT *point);

bool point_used_by_seam(SEAM *seam, EDGEPT *point);

void combine_seams(SEAM *dest_seam, SEAM *source_seam);

void start_seam_list(TWERD *word, GenericVector<SEAM*>* seam_array);

bool test_insert_seam(const GenericVector<SEAM*>& seam_array,
                      TWERD *word, int index);

void insert_seam(const TWERD *word, int index, SEAM *seam,
                 GenericVector<SEAM*>* seam_array);

int account_splits(const SEAM *seam, const TWERD *word, int blob_index,
                   int blob_direction);

bool find_split_in_blob(SPLIT *split, TBLOB *blob);

SEAM *join_two_seams(const SEAM *seam1, const SEAM *seam2);

void print_seam(const char *label, SEAM *seam);

void print_seams(const char *label, const GenericVector<SEAM*>& seams);

int shared_split_points(const SEAM *seam1, const SEAM *seam2);

void break_pieces(const GenericVector<SEAM*>& seams,
                  int first, int last, TWERD *word);

void join_pieces(const GenericVector<SEAM*>& seams,
                 int first, int last, TWERD *word);

void hide_seam(SEAM *seam);

void hide_edge_pair(EDGEPT *pt1, EDGEPT *pt2);

void reveal_seam(SEAM *seam);

void reveal_edge_pair(EDGEPT *pt1, EDGEPT *pt2);

#endif
