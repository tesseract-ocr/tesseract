/* -*-C-*-
 ********************************************************************************
 *
 * File:        seam.c  (Formerly seam.c)
 * Description:
 * Author:       Mark Seaman, OCR Technology
 * Created:      Fri Oct 16 14:37:00 1987
 * Modified:     Fri May 17 16:30:13 1991 (Mark Seaman) marks@hpgrlt
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
/*----------------------------------------------------------------------
              I n c l u d e s
----------------------------------------------------------------------*/
#include "seam.h"
#include "blobs.h"
#include "freelist.h"
#include "tprintf.h"

#ifdef __UNIX__
#include <assert.h>
#endif

/*----------------------------------------------------------------------
              V a r i a b l e s
----------------------------------------------------------------------*/
#define NUM_STARTING_SEAMS  20

/*----------------------------------------------------------------------
        Public Function Code
----------------------------------------------------------------------*/
/**
 * @name point_in_split
 *
 * Check to see if either of these points are present in the current
 * split.
 * @returns TRUE if one of them is split.
 */
bool point_in_split(SPLIT *split, EDGEPT *point1, EDGEPT *point2) {
  return ((split) ? ((exact_point (split->point1, point1) ||
                      exact_point (split->point1, point2) ||
                      exact_point (split->point2, point1) ||
                      exact_point (split->point2, point2)) ? TRUE : FALSE)
                  : FALSE);
}


/**
 * @name point_in_seam
 *
 * Check to see if either of these points are present in the current
 * seam.
 * @returns TRUE if one of them is.
 */
bool point_in_seam(const SEAM *seam, SPLIT *split) {
  return (point_in_split(seam->split1, split->point1, split->point2) ||
          point_in_split(seam->split2, split->point1, split->point2) ||
          point_in_split(seam->split3, split->point1, split->point2));
}

/**
 * @name point_used_by_split
 *
 * Return whether this particular EDGEPT * is used in a given split.
 * @returns TRUE if the edgept is used by the split.
 */
bool point_used_by_split(SPLIT *split, EDGEPT *point) {
  if (split == NULL) return false;
  return point == split->point1 || point == split->point2;
}

/**
 * @name point_used_by_seam
 *
 * Return whether this particular EDGEPT * is used in a given seam.
 * @returns TRUE if the edgept is used by the seam.
 */
bool point_used_by_seam(SEAM *seam, EDGEPT *point) {
  if (seam == NULL) return false;
  return point_used_by_split(seam->split1, point) ||
      point_used_by_split(seam->split2, point) ||
      point_used_by_split(seam->split3, point);
}

/**
 * @name combine_seam
 *
 * Combine two seam records into a single seam.  Move the split
 * references from the second seam to the first one.  The argument
 * convention is patterned after strcpy.
 */
void combine_seams(SEAM *dest_seam, SEAM *source_seam) {
  dest_seam->priority += source_seam->priority;
  dest_seam->location += source_seam->location;
  dest_seam->location /= 2;

  if (source_seam->split1) {
    if (!dest_seam->split1)
      dest_seam->split1 = source_seam->split1;
    else if (!dest_seam->split2)
      dest_seam->split2 = source_seam->split1;
    else if (!dest_seam->split3)
      dest_seam->split3 = source_seam->split1;
    else
      delete source_seam->split1;  // Wouldn't have fitted.
    source_seam->split1 = NULL;
  }
  if (source_seam->split2) {
    if (!dest_seam->split2)
      dest_seam->split2 = source_seam->split2;
    else if (!dest_seam->split3)
      dest_seam->split3 = source_seam->split2;
    else
      delete source_seam->split2;  // Wouldn't have fitted.
    source_seam->split2 = NULL;
  }
  if (source_seam->split3) {
    if (!dest_seam->split3)
      dest_seam->split3 = source_seam->split3;
    else
      delete source_seam->split3;  // Wouldn't have fitted.
    source_seam->split3 = NULL;
  }
  delete source_seam;
}

/**
 * @name start_seam_list
 *
 * Initialize a list of seams that match the original number of blobs
 * present in the starting segmentation.  Each of the seams created
 * by this routine have location information only.
 */
void start_seam_list(TWERD *word, GenericVector<SEAM*>* seam_array) {
  seam_array->truncate(0);
  TPOINT location;

  for (int b = 1; b < word->NumBlobs(); ++b) {
    TBOX bbox = word->blobs[b - 1]->bounding_box();
    TBOX nbox = word->blobs[b]->bounding_box();
    location.x = (bbox.right() + nbox.left()) / 2;
    location.y = (bbox.bottom() + bbox.top() + nbox.bottom() + nbox.top()) / 4;
    seam_array->push_back(new SEAM(0.0f, location, NULL, NULL, NULL));
  }
}


/**
 * @name test_insert_seam
 *
 * @returns true if insert_seam will succeed.
 */
bool test_insert_seam(const GenericVector<SEAM*>& seam_array,
                      TWERD *word, int index) {
  SEAM *test_seam;
  TBLOB *blob;
  int test_index;
  int list_length;

  list_length = seam_array.size();
  for (int test_index = 0; test_index < index; ++test_index) {
    test_seam = seam_array[test_index];
    if (test_index + test_seam->widthp < index &&
        test_seam->widthp + test_index == index - 1 &&
        account_splits(test_seam, word, test_index + 1, 1) < 0)
      return false;
  }
  for (int test_index = index; test_index < list_length; test_index++) {
    test_seam = seam_array[test_index];
    if (test_index - test_seam->widthn >= index &&
        test_index - test_seam->widthn == index &&
        account_splits(test_seam, word, test_index + 1, -1) < 0)
      return false;
  }
  return true;
}

/**
 * @name insert_seam
 *
 * Add another seam to a collection of seams at a particular location
 * in the seam array.
 */
void insert_seam(const TWERD* word, int index, SEAM *seam,
                 GenericVector<SEAM*>* seam_array) {
  SEAM *test_seam;
  int test_index;
  int list_length;

  list_length = seam_array->size();
  for (int test_index = 0; test_index < index; ++test_index) {
    test_seam = seam_array->get(test_index);
    if (test_index + test_seam->widthp >= index) {
      test_seam->widthp++;       /*got in the way */
    } else if (test_seam->widthp + test_index == index - 1) {
      test_seam->widthp = account_splits(test_seam, word, test_index + 1, 1);
      if (test_seam->widthp < 0) {
        tprintf("Failed to find any right blob for a split!\n");
        print_seam("New dud seam", seam);
        print_seam("Failed seam", test_seam);
      }
    }
  }
  for (int test_index = index; test_index < list_length; test_index++) {
    test_seam = seam_array->get(test_index);
    if (test_index - test_seam->widthn < index) {
      test_seam->widthn++;       /*got in the way */
    } else if (test_index - test_seam->widthn == index) {
      test_seam->widthn = account_splits(test_seam, word, test_index + 1, -1);
      if (test_seam->widthn < 0) {
        tprintf("Failed to find any left blob for a split!\n");
        print_seam("New dud seam", seam);
        print_seam("Failed seam", test_seam);
      }
    }
  }
  seam_array->insert(seam, index);
}


/**
 * @name account_splits
 *
 * Account for all the splits by looking to the right (blob_direction == 1),
 * or to the left (blob_direction == -1) in the word.
 */
int account_splits(const SEAM *seam, const TWERD *word, int blob_index,
                   int blob_direction) {
  inT8 found_em[3];
  inT8 width;

  found_em[0] = seam->split1 == NULL;
  found_em[1] = seam->split2 == NULL;
  found_em[2] = seam->split3 == NULL;
  if (found_em[0] && found_em[1] && found_em[2])
    return 0;
  width = 0;
  do {
    TBLOB* blob = word->blobs[blob_index];
    if (!found_em[0])
      found_em[0] = find_split_in_blob(seam->split1, blob);
    if (!found_em[1])
      found_em[1] = find_split_in_blob(seam->split2, blob);
    if (!found_em[2])
      found_em[2] = find_split_in_blob(seam->split3, blob);
    if (found_em[0] && found_em[1] && found_em[2]) {
      return width;
    }
    width++;
    blob_index += blob_direction;
  } while (0 <= blob_index && blob_index < word->NumBlobs());
  return -1;
}


/**
 * @name find_split_in_blob
 *
 * @returns TRUE if the split is somewhere in this blob.
 */
bool find_split_in_blob(SPLIT *split, TBLOB *blob) {
  TESSLINE *outline;

  for (outline = blob->outlines; outline != NULL; outline = outline->next)
    if (outline->Contains(split->point1->pos))
      break;
  if (outline == NULL)
    return FALSE;
  for (outline = blob->outlines; outline != NULL; outline = outline->next)
    if (outline->Contains(split->point2->pos))
      return TRUE;
  return FALSE;
}


/**
 * @name join_two_seams
 *
 * Merge these two seams into a new seam.  Duplicate the split records
 * in both of the input seams.  Return the resultant seam.
 */
SEAM *join_two_seams(const SEAM *seam1, const SEAM *seam2) {
  SEAM *result = NULL;
  SEAM *temp;

  assert(seam1 &&seam2);

  if (((seam1->split3 == NULL && seam2->split2 == NULL) ||
       (seam1->split2 == NULL && seam2->split3 == NULL) ||
        seam1->split1 == NULL || seam2->split1 == NULL) &&
      (!shared_split_points(seam1, seam2))) {
    result = new SEAM(*seam1);
    temp = new SEAM(*seam2);
    combine_seams(result, temp);
  }
  return (result);
}

/**
 * @name print_seam
 *
 * Print a list of splits.  Show the coordinates of both points in
 * each split.
 */
void print_seam(const char *label, SEAM *seam) {
  if (seam) {
    tprintf(label);
    tprintf(" %6.2f @ (%d,%d), p=%d, n=%d ",
            seam->priority, seam->location.x, seam->location.y,
            seam->widthp, seam->widthn);
    print_split(seam->split1);

    if (seam->split2) {
      tprintf(",   ");
      print_split (seam->split2);
      if (seam->split3) {
        tprintf(",   ");
        print_split (seam->split3);
      }
    }
    tprintf("\n");
  }
}


/**
 * @name print_seams
 *
 * Print a list of splits.  Show the coordinates of both points in
 * each split.
 */
void print_seams(const char *label, const GenericVector<SEAM*>& seams) {
  char number[CHARS_PER_LINE];

  if (!seams.empty()) {
    tprintf("%s\n", label);
    for (int x = 0; x < seams.size(); ++x) {
      sprintf(number, "%2d:   ", x);
      print_seam(number, seams[x]);
    }
    tprintf("\n");
  }
}


/**
 * @name shared_split_points
 *
 * Check these two seams to make sure that neither of them have two
 * points in common. Return TRUE if any of the same points are present
 * in any of the splits of both seams.
 */
int shared_split_points(const SEAM *seam1, const SEAM *seam2) {
  if (seam1 == NULL || seam2 == NULL)
    return (FALSE);

  if (seam2->split1 == NULL)
    return (FALSE);
  if (point_in_seam(seam1, seam2->split1))
    return (TRUE);

  if (seam2->split2 == NULL)
    return (FALSE);
  if (point_in_seam(seam1, seam2->split2))
    return (TRUE);

  if (seam2->split3 == NULL)
    return (FALSE);
  if (point_in_seam(seam1, seam2->split3))
    return (TRUE);

  return (FALSE);
}

/**********************************************************************
 * break_pieces
 *
 * Break up the blobs in this chain so that they are all independent.
 * This operation should undo the affect of join_pieces.
 **********************************************************************/
void break_pieces(const GenericVector<SEAM*>& seams, int first, int last,
                  TWERD *word) {
  for (int x = first; x < last; ++x)
    reveal_seam(seams[x]);

  TESSLINE *outline = word->blobs[first]->outlines;
  int next_blob = first + 1;

  while (outline != NULL && next_blob <= last) {
    if (outline->next == word->blobs[next_blob]->outlines) {
      outline->next = NULL;
      outline = word->blobs[next_blob]->outlines;
      ++next_blob;
    } else {
      outline = outline->next;
    }
  }
}


/**********************************************************************
 * join_pieces
 *
 * Join a group of base level pieces into a single blob that can then
 * be classified.
 **********************************************************************/
void join_pieces(const GenericVector<SEAM*>& seams, int first, int last,
                 TWERD *word) {
  TESSLINE *outline = word->blobs[first]->outlines;
  if (!outline)
    return;

  for (int x = first; x < last; ++x) {
    SEAM *seam = seams[x];
    if (x - seam->widthn >= first && x + seam->widthp < last)
      hide_seam(seam);
    while (outline->next)
      outline = outline->next;
    outline->next = word->blobs[x + 1]->outlines;
  }
}


/**********************************************************************
 * hide_seam
 *
 * Change the edge points that are referenced by this seam to make
 * them hidden edges.
 **********************************************************************/
void hide_seam(SEAM *seam) {
  if (seam == NULL || seam->split1 == NULL)
    return;
  hide_edge_pair (seam->split1->point1, seam->split1->point2);

  if (seam->split2 == NULL)
    return;
  hide_edge_pair (seam->split2->point1, seam->split2->point2);

  if (seam->split3 == NULL)
    return;
  hide_edge_pair (seam->split3->point1, seam->split3->point2);
}


/**********************************************************************
 * hide_edge_pair
 *
 * Change the edge points that are referenced by this seam to make
 * them hidden edges.
 **********************************************************************/
void hide_edge_pair(EDGEPT *pt1, EDGEPT *pt2) {
  EDGEPT *edgept;

  edgept = pt1;
  do {
    edgept->Hide();
    edgept = edgept->next;
  }
  while (!exact_point (edgept, pt2) && edgept != pt1);
  if (edgept == pt1) {
    /*              tprintf("Hid entire outline at (%d,%d)!!\n",
       edgept->pos.x,edgept->pos.y);                                */
  }
  edgept = pt2;
  do {
    edgept->Hide();
    edgept = edgept->next;
  }
  while (!exact_point (edgept, pt1) && edgept != pt2);
  if (edgept == pt2) {
    /*              tprintf("Hid entire outline at (%d,%d)!!\n",
       edgept->pos.x,edgept->pos.y);                                */
  }
}


/**********************************************************************
 * reveal_seam
 *
 * Change the edge points that are referenced by this seam to make
 * them hidden edges.
 **********************************************************************/
void reveal_seam(SEAM *seam) {
  if (seam == NULL || seam->split1 == NULL)
    return;
  reveal_edge_pair (seam->split1->point1, seam->split1->point2);

  if (seam->split2 == NULL)
    return;
  reveal_edge_pair (seam->split2->point1, seam->split2->point2);

  if (seam->split3 == NULL)
    return;
  reveal_edge_pair (seam->split3->point1, seam->split3->point2);
}


/**********************************************************************
 * reveal_edge_pair
 *
 * Change the edge points that are referenced by this seam to make
 * them hidden edges.
 **********************************************************************/
void reveal_edge_pair(EDGEPT *pt1, EDGEPT *pt2) {
  EDGEPT *edgept;

  edgept = pt1;
  do {
    edgept->Reveal();
    edgept = edgept->next;
  }
  while (!exact_point (edgept, pt2) && edgept != pt1);
  if (edgept == pt1) {
    /*              tprintf("Hid entire outline at (%d,%d)!!\n",
       edgept->pos.x,edgept->pos.y);                                */
  }
  edgept = pt2;
  do {
    edgept->Reveal();
    edgept = edgept->next;
  }
  while (!exact_point (edgept, pt1) && edgept != pt2);
  if (edgept == pt2) {
    /*              tprintf("Hid entire outline at (%d,%d)!!\n",
       edgept->pos.x,edgept->pos.y);                                */
  }
}
