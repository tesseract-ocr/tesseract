/* -*-C-*-
 ********************************************************************************
 *
 * File:        makechop.c  (Formerly makechop.c)
 * Description:
 * Author:   Mark Seaman, OCR Technology
 * Created:  Fri Oct 16 14:37:00 1987
 * Modified:     Mon Jul 29 15:50:42 1991 (Mark Seaman) marks@hpgrlt
 * Language: C
 * Package:  N/A
 * Status:   Reusable Software Component
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

#include "makechop.h"
#include "blobs.h"
#include "render.h"
#include "structures.h"
#ifdef __UNIX__
#include <assert.h>
#include <unistd.h>
#endif

// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif

/*----------------------------------------------------------------------
        Public Function Code
----------------------------------------------------------------------*/
/**********************************************************************
 * apply_seam
 *
 * Split this blob into two blobs by applying the splits included in
 * the seam description.
 **********************************************************************/
void apply_seam(TBLOB *blob, TBLOB *other_blob, bool italic_blob, SEAM *seam) {
  if (seam->split1 == NULL) {
    divide_blobs(blob, other_blob, italic_blob, seam->location);
  }
  else if (seam->split2 == NULL) {
    make_split_blobs(blob, other_blob, italic_blob, seam);
  }
  else if (seam->split3 == NULL) {
    make_double_split(blob, other_blob, italic_blob, seam);
  }
  else {
    make_triple_split(blob, other_blob, italic_blob, seam);
  }
}


/**********************************************************************
 * form_two_blobs
 *
 * Group the outlines from the first blob into both of them. Do so
 * according to the information about the split.
 **********************************************************************/
void form_two_blobs(TBLOB *blob, TBLOB *other_blob, bool italic_blob,
                    const TPOINT& location) {
  setup_blob_outlines(blob);

  divide_blobs(blob, other_blob, italic_blob, location);

  eliminate_duplicate_outlines(blob);
  eliminate_duplicate_outlines(other_blob);

  correct_blob_order(blob, other_blob);
}


/**********************************************************************
 * make_double_split
 *
 * Create two blobs out of one by splitting the original one in half.
 * Return the resultant blobs for classification.
 **********************************************************************/
void make_double_split(TBLOB *blob, TBLOB *other_blob, bool italic_blob,
                       SEAM *seam) {
  make_single_split(blob->outlines, seam->split1);
  make_single_split(blob->outlines, seam->split2);
  form_two_blobs(blob, other_blob, italic_blob, seam->location);
}


/**********************************************************************
 * make_single_split
 *
 * Create two outlines out of one by splitting the original one in half.
 * Return the resultant outlines.
 **********************************************************************/
void make_single_split(TESSLINE *outlines, SPLIT *split) {
  assert (outlines != NULL);

  split_outline (split->point1, split->point2);

  while (outlines->next != NULL)
    outlines = outlines->next;

  outlines->next = new TESSLINE;
  outlines->next->loop = split->point1;
  outlines->next->ComputeBoundingBox();

  outlines = outlines->next;

  outlines->next = new TESSLINE;
  outlines->next->loop = split->point2;
  outlines->next->ComputeBoundingBox();

  outlines->next->next = NULL;
}


/**********************************************************************
 * make_split_blobs
 *
 * Create two blobs out of one by splitting the original one in half.
 * Return the resultant blobs for classification.
 **********************************************************************/
void make_split_blobs(TBLOB *blob, TBLOB *other_blob, bool italic_blob,
                      SEAM *seam) {
  make_single_split(blob->outlines, seam->split1);

  form_two_blobs (blob, other_blob, italic_blob, seam->location);
}


/**********************************************************************
 * make_triple_split
 *
 * Create two blobs out of one by splitting the original one in half.
 * This splitting is accomplished by applying three separate splits on
 * the outlines. Three of the starting outlines will produce two ending
 * outlines. Return the resultant blobs for classification.
 **********************************************************************/
void make_triple_split(TBLOB *blob, TBLOB *other_blob, bool italic_blob,
                       SEAM *seam) {
  make_single_split(blob->outlines, seam->split1);
  make_single_split(blob->outlines, seam->split2);
  make_single_split(blob->outlines, seam->split3);

  form_two_blobs(blob, other_blob, italic_blob, seam->location);
}


/**********************************************************************
 * undo_seam
 *
 * Remove the seam between these two blobs.  Produce one blob as a
 * result.  The seam may consist of one, two, or three splits.  Each
 * of these split must be removed from the outlines.
 **********************************************************************/
void undo_seam(TBLOB *blob, TBLOB *other_blob, SEAM *seam) {
  TESSLINE *outline;

  if (!seam)
    return;                      /* Append other blob outlines */
  if (blob->outlines == NULL) {
    blob->outlines = other_blob->outlines;
    other_blob->outlines = NULL;
  }

  outline = blob->outlines;
  while (outline->next)
    outline = outline->next;
  outline->next = other_blob->outlines;
  other_blob->outlines = NULL;
  delete other_blob;

  if (seam->split1 == NULL) {
  }
  else if (seam->split2 == NULL) {
    undo_single_split (blob, seam->split1);
  }
  else if (seam->split3 == NULL) {
    undo_single_split (blob, seam->split1);
    undo_single_split (blob, seam->split2);
  }
  else {
    undo_single_split (blob, seam->split3);
    undo_single_split (blob, seam->split2);
    undo_single_split (blob, seam->split1);
  }

  setup_blob_outlines(blob);
  eliminate_duplicate_outlines(blob);
}


/**********************************************************************
 * undo_single_split
 *
 * Undo a seam that is made by a single split.  Perform the correct
 * magic to reconstruct the appropriate set of outline data structures.
 **********************************************************************/
void undo_single_split(TBLOB *blob, SPLIT *split) {
  TESSLINE *outline1;
  TESSLINE *outline2;
  /* Modify edge points */
  unsplit_outlines (split->point1, split->point2);

  outline1 = new TESSLINE;
  outline1->next = blob->outlines;
  blob->outlines = outline1;
  outline1->loop = split->point1;

  outline2 = new TESSLINE;
  outline2->next = blob->outlines;
  blob->outlines = outline2;
  outline2->loop = split->point2;
}
