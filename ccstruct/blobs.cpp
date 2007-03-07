/* -*-C-*-
 ********************************************************************************
 *
 * File:        blobs.c  (Formerly blobs.c)
 * Description:  Blob definition
 * Author:       Mark Seaman, OCR Technology
 * Created:      Fri Oct 27 15:39:52 1989
 * Modified:     Thu Mar 28 15:33:26 1991 (Mark Seaman) marks@hpgrlt
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

/*----------------------------------------------------------------------
              I n c l u d e s
----------------------------------------------------------------------*/
#include "mfcpch.h"
#include "blobs.h"
#include "cutil.h"
#include "emalloc.h"
#include "structures.h"

/*----------------------------------------------------------------------
              F u n c t i o n s
----------------------------------------------------------------------*/
/**********************************************************************
 * blob_origin
 *
 * Compute the origin of a compound blob, define to be the centre
 * of the bounding box.
 **********************************************************************/
void blob_origin(TBLOB *blob,       /*blob to compute on */
                 TPOINT *origin) {  /*return value */
  TPOINT topleft;                /*bounding box */
  TPOINT botright;

                                 /*find bounding box */
  blob_bounding_box(blob, &topleft, &botright); 
                                 /*centre of box */
  origin->x = (topleft.x + botright.x) / 2;
  origin->y = (topleft.y + botright.y) / 2;
}


/**********************************************************************
 * blob_bounding_box
 *
 * Compute the bounding_box of a compound blob, define to be the
 * max coordinate value of the bounding boxes of all the top-level
 * outlines in the box.
 **********************************************************************/
void blob_bounding_box(TBLOB *blob,               /*blob to compute on */
                       register TPOINT *topleft,  /*bounding box */
                       register TPOINT *botright) {
  register TESSLINE *outline;    /*current outline */

  if (blob == NULL || blob->outlines == NULL) {
    topleft->x = topleft->y = 0;
    *botright = *topleft;        /*default value */
  }
  else {
    outline = blob->outlines;
    *topleft = outline->topleft;
    *botright = outline->botright;
    for (outline = outline->next; outline != NULL; outline = outline->next) {
      if (outline->topleft.x < topleft->x)
                                 /*find extremes */
        topleft->x = outline->topleft.x;
      if (outline->botright.x > botright->x)
                                 /*find extremes */
        botright->x = outline->botright.x;
      if (outline->topleft.y > topleft->y)
                                 /*find extremes */
        topleft->y = outline->topleft.y;
      if (outline->botright.y < botright->y)
                                 /*find extremes */
        botright->y = outline->botright.y;
    }
  }
}


/**********************************************************************
 * blobs_bounding_box
 *
 * Return the smallest extreme point that contain this word.
 **********************************************************************/
void blobs_bounding_box(TBLOB *blobs, TPOINT *topleft, TPOINT *botright) { 
  TPOINT tl;
  TPOINT br;
  TBLOB *blob;
  /* Start with first blob */
  blob_bounding_box(blobs, topleft, botright); 

  iterate_blobs(blob, blobs) { 
    blob_bounding_box(blob, &tl, &br); 

    if (tl.x < topleft->x)
      topleft->x = tl.x;
    if (tl.y > topleft->y)
      topleft->y = tl.y;
    if (br.x > botright->x)
      botright->x = br.x;
    if (br.y < botright->y)
      botright->y = br.y;
  }
}


/**********************************************************************
 * blobs_origin
 *
 * Compute the origin of a compound blob, define to be the centre
 * of the bounding box.
 **********************************************************************/
void blobs_origin(TBLOB *blobs,      /*blob to compute on */
                  TPOINT *origin) {  /*return value */
  TPOINT topleft;                /*bounding box */
  TPOINT botright;

                                 /*find bounding box */
  blobs_bounding_box(blobs, &topleft, &botright); 
                                 /*center of box */
  origin->x = (topleft.x + botright.x) / 2;
  origin->y = (topleft.y + botright.y) / 2;
}


/**********************************************************************
 * blobs_widths
 *
 * Compute the widths of a list of blobs. Return an array of the widths
 * and gaps.
 **********************************************************************/
WIDTH_RECORD *blobs_widths(TBLOB *blobs) {  /*blob to compute on */
  WIDTH_RECORD *width_record;
  TPOINT topleft;                /*bounding box */
  TPOINT botright;
  TBLOB *blob;                   /*blob to compute on */
  int i = 0;
  int blob_end;
  int num_blobs = count_blobs (blobs);

  /* Get memory */
  width_record = (WIDTH_RECORD *) memalloc (sizeof (int) * num_blobs * 2);
  width_record->num_chars = num_blobs;

  blob_bounding_box(blobs, &topleft, &botright); 
  width_record->widths[i++] = botright.x - topleft.x;
  /* First width */
  blob_end = botright.x;

  iterate_blobs (blob, blobs->next) {
    blob_bounding_box(blob, &topleft, &botright); 
    width_record->widths[i++] = topleft.x - blob_end;
    width_record->widths[i++] = botright.x - topleft.x;
    blob_end = botright.x;
  }
  return (width_record);
}


/**********************************************************************
 * count_blobs
 *
 * Return a count of the number of blobs attached to this one.
 **********************************************************************/
int count_blobs(TBLOB *blobs) { 
  TBLOB *b;
  int x = 0;

  iterate_blobs (b, blobs) x++;
  return (x);
}


/**********************************************************************
 * delete_word
 *
 * Reclaim the memory taken by this word structure and all of its
 * lower level structures.
 **********************************************************************/
void delete_word(TWERD *word) { 
  TBLOB *blob;
  TBLOB *nextblob;
  TESSLINE *outline;
  TESSLINE *nextoutline;
  TESSLINE *child;
  TESSLINE *nextchild;

  for (blob = word->blobs; blob; blob = nextblob) {
    nextblob = blob->next;

    for (outline = blob->outlines; outline; outline = nextoutline) {
      nextoutline = outline->next;

      delete_edgepts (outline->loop);

      for (child = outline->child; child; child = nextchild) {
        nextchild = child->next;

        delete_edgepts (child->loop);

        oldoutline(child); 
      }
      oldoutline(outline); 
    }
    oldblob(blob); 
  }
  if (word->correct != NULL)
    strfree (word->correct);     /* Reclaim memory */
  oldword(word); 
}


/**********************************************************************
 * delete_edgepts
 *
 * Delete a list of EDGEPT structures.
 **********************************************************************/
void delete_edgepts(register EDGEPT *edgepts) { 
  register EDGEPT *this_edge;
  register EDGEPT *next_edge;

  if (edgepts == NULL)
    return;

  this_edge = edgepts;
  do {
    next_edge = this_edge->next;
    oldedgept(this_edge); 
    this_edge = next_edge;
  }
  while (this_edge != edgepts);
}
