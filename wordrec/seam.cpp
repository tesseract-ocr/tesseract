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
#include "callcpp.h"
#include "structures.h"
#include "makechop.h"

#ifdef __UNIX__
#include <assert.h>
#endif

/*----------------------------------------------------------------------
              V a r i a b l e s
----------------------------------------------------------------------*/
#define NUM_STARTING_SEAMS  20

#define SEAMBLOCK 100            /* Cells per block */
makestructure (newseam, free_seam, printseam, SEAM,
freeseam, SEAMBLOCK, "SEAM", seamcount);

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
  return ((split) ?
    ((exact_point (split->point1, point1) ||
    exact_point (split->point1, point2) ||
    exact_point (split->point2, point1) ||
    exact_point (split->point2, point2)) ? TRUE : FALSE) : FALSE);
}


/**
 * @name point_in_seam
 *
 * Check to see if either of these points are present in the current
 * seam.  
 * @returns TRUE if one of them is.
 */
bool point_in_seam(SEAM *seam, SPLIT *split) {
  return (point_in_split (seam->split1, split->point1, split->point2) ||
    point_in_split (seam->split2, split->point1, split->point2) ||
    point_in_split (seam->split3, split->point1, split->point2));
}


/**
 * @name add_seam
 *
 * Add another seam to a collection of seams.
 */
SEAMS add_seam(SEAMS seam_list, SEAM *seam) {
  return (array_push (seam_list, seam));
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
      cprintf ("combine_seam: Seam is too crowded, can't be combined !\n");
  }
  if (source_seam->split2) {
    if (!dest_seam->split2)
      dest_seam->split2 = source_seam->split2;
    else if (!dest_seam->split3)
      dest_seam->split3 = source_seam->split2;
    else
      cprintf ("combine_seam: Seam is too crowded, can't be combined !\n");
  }
  if (source_seam->split3) {
    if (!dest_seam->split3)
      dest_seam->split3 = source_seam->split3;
    else
      cprintf ("combine_seam: Seam is too crowded, can't be combined !\n");
  }
  free_seam(source_seam);
}


/**
 * @name delete_seam
 *
 * Free this seam record and the splits that are attached to it.
 */
void delete_seam(void *arg) {  //SEAM  *seam)
  SEAM *seam = (SEAM *) arg;

  if (seam) {
    if (seam->split1)
      delete_split (seam->split1);
    if (seam->split2)
      delete_split (seam->split2);
    if (seam->split3)
      delete_split (seam->split3);
    free_seam(seam);
  }
}


/**
 * @name free_seam_list
 *
 * Free all the seams that have been allocated in this list.  Reclaim
 * the memory for each of the splits as well.
 */
void free_seam_list(SEAMS seam_list) {
  int x;

  array_loop (seam_list, x) delete_seam (array_value (seam_list, x));
  array_free(seam_list);
}


/**
 * @name test_insert_seam
 *
 * @returns true if insert_seam will succeed.
 */
bool test_insert_seam(SEAMS seam_list,
                      int index,
                      TBLOB *left_blob,
                      TBLOB *first_blob) {
  SEAM *test_seam;
  TBLOB *blob;
  int test_index;
  int list_length;

  list_length = array_count (seam_list);
  for (test_index = 0, blob = first_blob->next;
  test_index < index; test_index++, blob = blob->next) {
    test_seam = (SEAM *) array_value (seam_list, test_index);
    if (test_index + test_seam->widthp < index &&
        test_seam->widthp + test_index == index - 1 &&
        account_splits_right(test_seam, blob) < 0)
      return false;
  }
  for (test_index = index, blob = left_blob->next;
  test_index < list_length; test_index++, blob = blob->next) {
    test_seam = (SEAM *) array_value (seam_list, test_index);
    if (test_index - test_seam->widthn >= index &&
        test_index - test_seam->widthn == index &&
        account_splits_left(test_seam, first_blob, blob) < 0)
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
SEAMS insert_seam(SEAMS seam_list,
                  int index,
                  SEAM *seam,
                  TBLOB *left_blob,
                  TBLOB *first_blob) {
  SEAM *test_seam;
  TBLOB *blob;
  int test_index;
  int list_length;

  list_length = array_count (seam_list);
  for (test_index = 0, blob = first_blob->next;
  test_index < index; test_index++, blob = blob->next) {
    test_seam = (SEAM *) array_value (seam_list, test_index);
    if (test_index + test_seam->widthp >= index) {
      test_seam->widthp++;       /*got in the way */
    }
    else if (test_seam->widthp + test_index == index - 1) {
      test_seam->widthp = account_splits_right(test_seam, blob);
      if (test_seam->widthp < 0) {
        cprintf ("Failed to find any right blob for a split!\n");
        print_seam("New dud seam", seam);
        print_seam("Failed seam", test_seam);
      }
    }
  }
  for (test_index = index, blob = left_blob->next;
  test_index < list_length; test_index++, blob = blob->next) {
    test_seam = (SEAM *) array_value (seam_list, test_index);
    if (test_index - test_seam->widthn < index) {
      test_seam->widthn++;       /*got in the way */
    }
    else if (test_index - test_seam->widthn == index) {
      test_seam->widthn = account_splits_left(test_seam, first_blob, blob);
      if (test_seam->widthn < 0) {
        cprintf ("Failed to find any left blob for a split!\n");
        print_seam("New dud seam", seam);
        print_seam("Failed seam", test_seam);
      }
    }
  }
  return (array_insert (seam_list, index, seam));
}


/**
 * @name account_splits_right
 *
 * Account for all the splits by looking to the right.
 * in the blob list.
 */
int account_splits_right(SEAM *seam, TBLOB *blob) {
  inT8 found_em[3];
  inT8 width;

  found_em[0] = seam->split1 == NULL;
  found_em[1] = seam->split2 == NULL;
  found_em[2] = seam->split3 == NULL;
  if (found_em[0] && found_em[1] && found_em[2])
    return 0;
  width = 0;
  do {
    if (!found_em[0])
      found_em[0] = find_split_in_blob (seam->split1, blob);
    if (!found_em[1])
      found_em[1] = find_split_in_blob (seam->split2, blob);
    if (!found_em[2])
      found_em[2] = find_split_in_blob (seam->split3, blob);
    if (found_em[0] && found_em[1] && found_em[2]) {
      return width;
    }
    width++;
    blob = blob->next;
  }
  while (blob != NULL);
  return -1;
}


/**
 * @name account_splits_left
 *
 * Account for all the splits by looking to the left.
 * in the blob list.
 */
int account_splits_left(SEAM *seam, TBLOB *blob, TBLOB *end_blob) {
  static inT32 depth = 0;
  static inT8 width;
  static inT8 found_em[3];

  if (blob != end_blob) {
    depth++;
    account_splits_left (seam, blob->next, end_blob);
    depth--;
  }
  else {
    found_em[0] = seam->split1 == NULL;
    found_em[1] = seam->split2 == NULL;
    found_em[2] = seam->split3 == NULL;
    width = 0;
  }
  if (!found_em[0])
    found_em[0] = find_split_in_blob (seam->split1, blob);
  if (!found_em[1])
    found_em[1] = find_split_in_blob (seam->split2, blob);
  if (!found_em[2])
    found_em[2] = find_split_in_blob (seam->split3, blob);
  if (!found_em[0] || !found_em[1] || !found_em[2]) {
    width++;
    if (depth == 0) {
      width = -1;
    }
  }
  return width;
}


/**
 * @name find_split_in_blob
 *
 * @returns TRUE if the split is somewhere in this blob.
 */
bool find_split_in_blob(SPLIT *split, TBLOB *blob) {
  TESSLINE *outline;

#if 0
  for (outline = blob->outlines; outline != NULL; outline = outline->next)
    if (is_split_outline (outline, split))
      return TRUE;
  return FALSE;
#endif
  for (outline = blob->outlines; outline != NULL; outline = outline->next)
    if (point_in_outline(split->point1, outline))
      break;
  if (outline == NULL)
    return FALSE;
  for (outline = blob->outlines; outline != NULL; outline = outline->next)
    if (point_in_outline(split->point2, outline))
      return TRUE;
  return FALSE;
}


/**
 * @name join_two_seams
 *
 * Merge these two seams into a new seam.  Duplicate the split records
 * in both of the input seams.  Return the resultant seam.
 */
SEAM *join_two_seams(SEAM *seam1, SEAM *seam2) {
  SEAM *result = NULL;
  SEAM *temp;

  assert(seam1 &&seam2);

  if (((seam1->split3 == NULL && seam2->split2 == NULL) ||
    (seam1->split2 == NULL && seam2->split3 == NULL) ||
    seam1->split1 == NULL ||
  seam2->split1 == NULL) && (!shared_split_points (seam1, seam2))) {
    clone_seam(result, seam1);
    clone_seam(temp, seam2);
    combine_seams(result, temp);
  }
  return (result);
}


/**
 * @name new_seam
 *
 * Create a structure for a "seam" between two blobs.  This data
 * structure may actually hold up to three different splits.
 * Initailization of this record is done by this routine.
 */
SEAM *new_seam(PRIORITY priority,
               int x_location,
               SPLIT *split1,
               SPLIT *split2,
               SPLIT *split3) {
  SEAM *seam;

  seam = newseam ();

  seam->priority = priority;
  seam->location = x_location;
  seam->widthp = 0;
  seam->widthn = 0;
  seam->split1 = split1;
  seam->split2 = split2;
  seam->split3 = split3;

  return (seam);
}


/**
 * @name new_seam_list
 *
 * Create a collection of seam records in an array.
 */
SEAMS new_seam_list() {
  return (array_new (NUM_STARTING_SEAMS));
}


/**
 * @name print_seam
 *
 * Print a list of splits.  Show the coordinates of both points in
 * each split.
 */
void print_seam(const char *label, SEAM *seam) {
  if (seam) {
    cprintf(label);
    cprintf (" %6.2f @ %5d, p=%d, n=%d ",
      seam->priority, seam->location, seam->widthp, seam->widthn);

    print_split (seam->split1);

    if (seam->split2) {
      cprintf (",   ");
      print_split (seam->split2);

      if (seam->split3) {
        cprintf (",   ");
        print_split (seam->split3);
      }
    }
    cprintf ("\n");
  }
}


/**
 * @name print_seams
 *
 * Print a list of splits.  Show the coordinates of both points in
 * each split.
 */
void print_seams(const char *label, SEAMS seams) {
  int x;
  char number[CHARS_PER_LINE];

  if (seams) {
    cprintf ("%s\n", label);
    array_loop(seams, x) {
      sprintf (number, "%2d:   ", x);
      print_seam (number, (SEAM *) array_value (seams, x));
    }
    cprintf ("\n");
  }
}


/**
 * @name shared_split_points
 *
 * Check these two seams to make sure that neither of them have two
 * points in common. Return TRUE if any of the same points are present
 * in any of the splits of both seams.
 */
int shared_split_points(SEAM *seam1, SEAM *seam2) {
  if (seam1 == NULL || seam2 == NULL)
    return (FALSE);

  if (seam2->split1 == NULL)
    return (FALSE);
  if (point_in_seam (seam1, seam2->split1))
    return (TRUE);

  if (seam2->split2 == NULL)
    return (FALSE);
  if (point_in_seam (seam1, seam2->split2))
    return (TRUE);

  if (seam2->split3 == NULL)
    return (FALSE);
  if (point_in_seam (seam1, seam2->split3))
    return (TRUE);

  return (FALSE);
}
