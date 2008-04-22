/* -*-C-*-
 ********************************************************************************
 *
 * File:        pieces.c  (Formerly pieces.c)
 * Description:
 * Author:       Mark Seaman, OCR Technology
 * Created:      Fri Oct 16 14:37:00 1987
 * Modified:     Mon May 20 12:12:35 1991 (Mark Seaman) marks@hpgrlt
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
#include "pieces.h"
#include "plotseg.h"
#include "hideedge.h"
#include "wordclass.h"
#include "freelist.h"
#include "blobs.h"
#include "matchtab.h"

/*----------------------------------------------------------------------
          M a c r o s
----------------------------------------------------------------------*/
/**********************************************************************
 * set_bounds_entry
 *
 * Set the value of the entry in an array of bounds elements.
 **********************************************************************/

#define set_bounds_entry(array,index,top_left,bot_right)  \
((array)[index].topleft  = (top_left),                  \
(array)[index].botright = (bot_right))                 \


/**********************************************************************
 * get_bounds_entry
 *
 * Get the value of the entry in an array of bounds elements.
 **********************************************************************/

#define get_bounds_entry(array,index,top_left,bot_right)  \
((top_left)  = (array)[index].topleft,                  \
(bot_right) = (array)[index].botright)                 \


/*----------------------------------------------------------------------
          F u n c t i o n s
----------------------------------------------------------------------*/
/**********************************************************************
 * break_pieces
 *
 * Break up the blobs in this chain so that they are all independent.
 * This operation should undo the affect of join_pieces.
 **********************************************************************/
void break_pieces(TBLOB *blobs, SEAMS seams, inT16 start, inT16 end) {
  TESSLINE *outline = blobs->outlines;
  TBLOB *next_blob;
  inT16 x;

  for (x = start; x < end; x++)
    reveal_seam ((SEAM *) array_value (seams, x));

  next_blob = blobs->next;

  while (outline && next_blob) {
    if (outline->next == next_blob->outlines) {
      outline->next = NULL;
      outline = next_blob->outlines;
      next_blob = next_blob->next;
    }
    else {
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
void join_pieces(TBLOB *piece_blobs, SEAMS seams, inT16 start, inT16 end) {
  TBLOB *next_blob;
  TBLOB *blob;
  inT16 x;
  TESSLINE *outline;
  SEAM *seam;

  for (x = 0, blob = piece_blobs; x < start; x++)
    blob = blob->next;
  next_blob = blob->next;
  outline = blob->outlines;
  if (!outline)
    return;

  while (x < end) {
    seam = (SEAM *) array_value (seams, x);
    if (x - seam->widthn >= start && x + seam->widthp < end)
      hide_seam(seam);
    while (outline->next)
      outline = outline->next;
    outline->next = next_blob->outlines;
    next_blob = next_blob->next;

    x++;
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
    hide_edge(edgept);
    edgept = edgept->next;
  }
  while (!exact_point (edgept, pt2) && edgept != pt1);
  if (edgept == pt1) {
    /*              cprintf("Hid entire outline at (%d,%d)!!\n",
       edgept->pos.x,edgept->pos.y);                                */
  }
  edgept = pt2;
  do {
    hide_edge(edgept);
    edgept = edgept->next;
  }
  while (!exact_point (edgept, pt1) && edgept != pt2);
  if (edgept == pt2) {
    /*              cprintf("Hid entire outline at (%d,%d)!!\n",
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
    reveal_edge(edgept);
    edgept = edgept->next;
  }
  while (!exact_point (edgept, pt2) && edgept != pt1);
  if (edgept == pt1) {
    /*              cprintf("Hid entire outline at (%d,%d)!!\n",
       edgept->pos.x,edgept->pos.y);                                */
  }
  edgept = pt2;
  do {
    reveal_edge(edgept);
    edgept = edgept->next;
  }
  while (!exact_point (edgept, pt1) && edgept != pt2);
  if (edgept == pt2) {
    /*              cprintf("Hid entire outline at (%d,%d)!!\n",
       edgept->pos.x,edgept->pos.y);                                */
  }
}


/**********************************************************************
 * bounds_of_piece
 *
 * Find the bounds of the piece that will be created by joining the
 * requested collection of pieces together.
 **********************************************************************/
void bounds_of_piece(BOUNDS_LIST bounds,
                     inT16 start,
                     inT16 end,
                     TPOINT *extreme_tl,
                     TPOINT *extreme_br) {
  TPOINT topleft;
  TPOINT botright;
  inT16 x;

  get_bounds_entry(bounds, start, *extreme_tl, *extreme_br);

  for (x = start + 1; x <= end; x++) {
    get_bounds_entry(bounds, x, topleft, botright);

    extreme_tl->x = min (topleft.x, extreme_tl->x);
    extreme_tl->y = max (topleft.y, extreme_tl->y);
    extreme_br->x = max (botright.x, extreme_br->x);
    extreme_br->y = min (botright.y, extreme_br->y);
  }
}


/**********************************************************************
 * classify_piece
 *
 * Create a larger piece from a collection of smaller ones.  Classify
 * it and return the results.  Take the large piece apart to leave
 * the collection of small pieces un modified.
 **********************************************************************/
CHOICES classify_piece(TBLOB *pieces,
                       SEAMS seams,
                       inT16 start,
                       inT16 end,
                       inT32 fx,
                       STATE *this_state,
                       STATE *best_state,
                       inT32 pass,
                       inT32 blob_index) {
  STATE current_state;
  CHOICES choices;
  TBLOB *pblob;
  TBLOB *blob;
  TBLOB *nblob;
  inT16 x;
  SEARCH_STATE chunk_groups;

  set_n_ones (&current_state, array_count (seams));

  join_pieces(pieces, seams, start, end);
  for (blob = pieces, pblob = NULL, x = 0; x < start; x++) {
    pblob = blob;
    blob = blob->next;
  }
  for (nblob = blob->next; x < end; x++)
    nblob = nblob->next;
  choices = classify_blob (pblob, blob, nblob, NULL, fx, "pieces:", White,
    this_state, best_state, pass, blob_index);

  break_pieces(blob, seams, start, end);
#ifndef GRAPHICS_DISABLED
  if (display_segmentations > 2) {
    chunk_groups = bin_to_chunks (&current_state, array_count (seams));
    display_segmentation(pieces, chunk_groups);
    window_wait(segm_window);
    memfree(chunk_groups);
  }
#endif

  return (choices);
}


/**********************************************************************
 * get_piece_rating
 *
 * Check to see if this piece has already been classified.  If it has
 * return that rating.  Otherwise build the piece from the smaller
 * pieces, classify it, store the rating for later, and take the piece
 * apart again.
 **********************************************************************/
CHOICES get_piece_rating(MATRIX ratings,
                         TBLOB *blobs,
                         SEAMS seams,
                         inT16 start,
                         inT16 end,
                         inT32 fx,
                         STATE *this_state,
                         STATE *best_state,
                         inT32 pass,
                         inT32 blob_index) {
  CHOICES choices;

  choices = matrix_get (ratings, start, end);
  if (choices == NOT_CLASSIFIED) {
    choices =
      classify_piece(blobs,
                     seams,
                     start,
                     end,
                     fx,
                     this_state,
                     best_state,
                     pass,
                     blob_index);
    matrix_put(ratings, start, end, choices);
  }
  return (choices);
}


/**********************************************************************
 * record_blob_bounds
 *
 * Set up and initialize an array that holds the bounds of a set of
 * blobs.
 **********************************************************************/
BOUNDS_LIST record_blob_bounds(TBLOB *blobs) {
  TBLOB *blob;
  BOUNDS_LIST bounds;
  TPOINT topleft;
  TPOINT botright;
  inT16 x = 0;

  bounds = (BOUNDS_LIST) memalloc (count_blobs (blobs) * sizeof (BOUNDS));

  iterate_blobs(blob, blobs) {
    blob_bounding_box(blob, &topleft, &botright);
    set_bounds_entry(bounds, x, topleft, botright);
    x++;
  }
  return (bounds);
}


/**********************************************************************
 * record_piece_ratings
 *
 * Save the choices for all the pieces that have been classified into
 * a matrix that can be used to look them up later.  A two dimensional
 * matrix is created.  The indices correspond to the starting and
 * ending initial piece number.
 **********************************************************************/
MATRIX record_piece_ratings(TBLOB *blobs) {
  BOUNDS_LIST bounds;
  inT16 num_blobs;
  inT16 x;
  inT16 y;
  TPOINT tp_topleft;
  TPOINT tp_botright;
  unsigned int topleft;
  unsigned int botright;
  MATRIX ratings;
  CHOICES choices;

  bounds = record_blob_bounds (blobs);
  num_blobs = count_blobs (blobs);
  ratings = create_matrix (num_blobs);

  for (x = 0; x < num_blobs; x++) {
    for (y = x; y < num_blobs; y++) {
      bounds_of_piece(bounds, x, y, &tp_topleft, &tp_botright);
      topleft = *(unsigned int *) &tp_topleft;
      botright = *(unsigned int *) &tp_botright;
      choices = get_match_by_bounds (topleft, botright);
      if (choices != NIL) {
        matrix_put(ratings, x, y, choices);
      }
    }
  }
  memfree(bounds);
  return (ratings);
}
