/**********************************************************************
 * File:        blobbox.cpp  (Formerly blobnbox.c)
 * Description: Code for the textord blob class.
 * Author:					Ray Smith
 * Created:					Thu Jul 30 09:08:51 BST 1992
 *
 * (C) Copyright 1992, Hewlett-Packard Ltd.
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
 **********************************************************************/

#include "mfcpch.h"
#include          "blobbox.h"

#define PROJECTION_MARGIN 10     //arbitrary
#define EXTERN

EXTERN double_VAR (textord_error_weight, 3,
"Weighting for error in believability");
EXTERN BOOL_VAR (pitsync_projection_fix, TRUE,
"Fix bug in projection profile");

ELISTIZE (BLOBNBOX) ELIST2IZE (TO_ROW) ELISTIZE (TO_BLOCK)
/**********************************************************************
 * BLOBNBOX::merge
 *
 * Merge this blob with the given blob, which should be after this.
 **********************************************************************/
void BLOBNBOX::merge(                    //merge blobs
                     BLOBNBOX *nextblob  //blob to join with
                    ) {
  box += nextblob->box;          //merge boxes
  nextblob->joined = TRUE;
}


/**********************************************************************
 * BLOBNBOX::chop
 *
 * Chop this blob into equal sized pieces using the x height as a guide.
 * The blob is not actually chopped. Instead, fake blobs are inserted
 * with the relevant bounding boxes.
 **********************************************************************/

void BLOBNBOX::chop(                        //chop blobs
                    BLOBNBOX_IT *start_it,  //location of this
                    BLOBNBOX_IT *end_it,    //iterator
                    FCOORD rotation,        //for landscape
                    float xheight           //of line
                   ) {
  inT16 blobcount;               //no of blobs
  BLOBNBOX *newblob;             //fake blob
  BLOBNBOX *blob;                //current blob
  inT16 blobindex;               //number of chop
  inT16 leftx;                   //left edge of blob
  float blobwidth;               //width of each
  float rightx;                  //right edge to scan
  float ymin, ymax;              //limits of new blob
  float test_ymin, test_ymax;    //limits of part blob
  ICOORD bl, tr;                 //corners of box
  BLOBNBOX_IT blob_it;           //blob iterator

                                 //get no of chops
  blobcount = (inT16) floor (box.width () / xheight);
  if (blobcount > 1 && (blob_ptr != NULL || cblob_ptr != NULL)) {
                                 //width of each
    blobwidth = (float) (box.width () + 1) / blobcount;
    for (blobindex = blobcount - 1, rightx = box.right ();
    blobindex >= 0; blobindex--, rightx -= blobwidth) {
      ymin = (float) MAX_INT32;
      ymax = (float) -MAX_INT32;
      blob_it = *start_it;
      do {
        blob = blob_it.data ();
        if (blob->blob_ptr != NULL)
          find_blob_limits (blob->blob_ptr, rightx - blobwidth, rightx,
            rotation, test_ymin, test_ymax);
        else
          find_cblob_vlimits (blob->cblob_ptr, rightx - blobwidth,
            rightx,
            /*rotation, */ test_ymin, test_ymax);
        blob_it.forward ();
        if (test_ymin < ymin)
          ymin = test_ymin;
        if (test_ymax > ymax)
          ymax = test_ymax;
      }
      while (blob != end_it->data ());
      if (ymin < ymax) {
        leftx = (inT16) floor (rightx - blobwidth);
        if (leftx < box.left ())
          leftx = box.left ();   //clip to real box
        bl = ICOORD (leftx, (inT16) floor (ymin));
        tr = ICOORD ((inT16) ceil (rightx), (inT16) ceil (ymax));
        if (blobindex == 0)
          box = TBOX (bl, tr);    //change box
        else {
          newblob = new BLOBNBOX;
                                 //box is all it has
          newblob->box = TBOX (bl, tr);
                                 //stay on current
          end_it->add_after_stay_put (newblob);
        }
      }
    }
  }
}


/**********************************************************************
 * find_blob_limits
 *
 * Scan the outlines of the blob to locate the y min and max
 * between the given x limits.
 **********************************************************************/

void find_blob_limits(                  //get y limits
                      PBLOB *blob,      //blob to search
                      float leftx,      //x limits
                      float rightx,
                      FCOORD rotation,  //for landscape
                      float &ymin,      //output y limits
                      float &ymax) {
  float testy;                   //y intercept
  FCOORD pos;                    //rotated
  FCOORD vec;
  POLYPT *polypt;                //current point
                                 //outlines
  OUTLINE_IT out_it = blob->out_list ();
  POLYPT_IT poly_it;             //outline pts

  ymin = (float) MAX_INT32;
  ymax = (float) -MAX_INT32;
  for (out_it.mark_cycle_pt (); !out_it.cycled_list (); out_it.forward ()) {
                                 //get points
    poly_it.set_to_list (out_it.data ()->polypts ());
    for (poly_it.mark_cycle_pt (); !poly_it.cycled_list ();
    poly_it.forward ()) {
      polypt = poly_it.data ();
      pos = polypt->pos;
      pos.rotate (rotation);
      vec = polypt->vec;
      vec.rotate (rotation);
      if ((pos.x () < leftx && pos.x () + vec.x () > leftx)
      || (pos.x () > leftx && pos.x () + vec.x () < leftx)) {
        testy = pos.y () + vec.y () * (leftx - pos.x ()) / vec.x ();
        //intercept of boundary
        if (testy < ymin)
          ymin = testy;
        if (testy > ymax)
          ymax = testy;
      }
      if (pos.x () >= leftx && pos.x () <= rightx) {
        if (pos.y () > ymax)
          ymax = pos.y ();
        if (pos.y () < ymin)
          ymin = pos.y ();
      }
      if ((pos.x () > rightx && pos.x () + vec.x () < rightx)
      || (pos.x () < rightx && pos.x () + vec.x () > rightx)) {
        testy = pos.y () + vec.y () * (rightx - pos.x ()) / vec.x ();
        //intercept of boundary
        if (testy < ymin)
          ymin = testy;
        if (testy > ymax)
          ymax = testy;
      }
    }
  }
}


/**********************************************************************
 * find_cblob_limits
 *
 * Scan the outlines of the cblob to locate the y min and max
 * between the given x limits.
 **********************************************************************/

void find_cblob_limits(                  //get y limits
                       C_BLOB *blob,     //blob to search
                       float leftx,      //x limits
                       float rightx,
                       FCOORD rotation,  //for landscape
                       float &ymin,      //output y limits
                       float &ymax) {
  inT16 stepindex;               //current point
  ICOORD pos;                    //current coords
  ICOORD vec;                    //rotated step
  C_OUTLINE *outline;            //current outline
                                 //outlines
  C_OUTLINE_IT out_it = blob->out_list ();

  ymin = (float) MAX_INT32;
  ymax = (float) -MAX_INT32;
  for (out_it.mark_cycle_pt (); !out_it.cycled_list (); out_it.forward ()) {
    outline = out_it.data ();
    pos = outline->start_pos (); //get coords
    pos.rotate (rotation);
    for (stepindex = 0; stepindex < outline->pathlength (); stepindex++) {
                                 //inside
      if (pos.x () >= leftx && pos.x () <= rightx) {
        if (pos.y () > ymax)
          ymax = pos.y ();
        if (pos.y () < ymin)
          ymin = pos.y ();
      }
      vec = outline->step (stepindex);
      vec.rotate (rotation);
      pos += vec;                //move to next
    }
  }
}


/**********************************************************************
 * find_cblob_vlimits
 *
 * Scan the outlines of the cblob to locate the y min and max
 * between the given x limits.
 **********************************************************************/

void find_cblob_vlimits(               //get y limits
                        C_BLOB *blob,  //blob to search
                        float leftx,   //x limits
                        float rightx,
                        float &ymin,   //output y limits
                        float &ymax) {
  inT16 stepindex;               //current point
  ICOORD pos;                    //current coords
  ICOORD vec;                    //rotated step
  C_OUTLINE *outline;            //current outline
                                 //outlines
  C_OUTLINE_IT out_it = blob->out_list ();

  ymin = (float) MAX_INT32;
  ymax = (float) -MAX_INT32;
  for (out_it.mark_cycle_pt (); !out_it.cycled_list (); out_it.forward ()) {
    outline = out_it.data ();
    pos = outline->start_pos (); //get coords
    for (stepindex = 0; stepindex < outline->pathlength (); stepindex++) {
                                 //inside
      if (pos.x () >= leftx && pos.x () <= rightx) {
        if (pos.y () > ymax)
          ymax = pos.y ();
        if (pos.y () < ymin)
          ymin = pos.y ();
      }
      vec = outline->step (stepindex);
      pos += vec;                //move to next
    }
  }
}


/**********************************************************************
 * find_cblob_hlimits
 *
 * Scan the outlines of the cblob to locate the x min and max
 * between the given y limits.
 **********************************************************************/

void find_cblob_hlimits(                //get x limits
                        C_BLOB *blob,   //blob to search
                        float bottomy,  //y limits
                        float topy,
                        float &xmin,    //output x limits
                        float &xmax) {
  inT16 stepindex;               //current point
  ICOORD pos;                    //current coords
  ICOORD vec;                    //rotated step
  C_OUTLINE *outline;            //current outline
                                 //outlines
  C_OUTLINE_IT out_it = blob->out_list ();

  xmin = (float) MAX_INT32;
  xmax = (float) -MAX_INT32;
  for (out_it.mark_cycle_pt (); !out_it.cycled_list (); out_it.forward ()) {
    outline = out_it.data ();
    pos = outline->start_pos (); //get coords
    for (stepindex = 0; stepindex < outline->pathlength (); stepindex++) {
                                 //inside
      if (pos.y () >= bottomy && pos.y () <= topy) {
        if (pos.x () > xmax)
          xmax = pos.x ();
        if (pos.x () < xmin)
          xmin = pos.x ();
      }
      vec = outline->step (stepindex);
      pos += vec;                //move to next
    }
  }
}


/**********************************************************************
 * rotate_blob
 *
 * Poly copy the blob and rotate the copy by the given vector.
 **********************************************************************/

PBLOB *rotate_blob(                 //get y limits
                   PBLOB *blob,     //blob to search
                   FCOORD rotation  //vector to rotate by
                  ) {
  PBLOB *copy;                   //copy of blob
  POLYPT *polypt;                //current point
  OUTLINE_IT out_it;
  POLYPT_IT poly_it;             //outline pts

  copy = new PBLOB;
  *copy = *blob;                 //deep copy
  out_it.set_to_list (copy->out_list ());
  for (out_it.mark_cycle_pt (); !out_it.cycled_list (); out_it.forward ()) {
                                 //get points
    poly_it.set_to_list (out_it.data ()->polypts ());
    for (poly_it.mark_cycle_pt (); !poly_it.cycled_list ();
    poly_it.forward ()) {
      polypt = poly_it.data ();
                                 //rotate it
      polypt->pos.rotate (rotation);
      polypt->vec.rotate (rotation);
    }
    out_it.data ()->compute_bb ();
  }
  return copy;
}


/**********************************************************************
 * rotate_cblob
 *
 * Poly copy the blob and rotate the copy by the given vector.
 **********************************************************************/

PBLOB *rotate_cblob(                 //rotate it
                    C_BLOB *blob,    //blob to search
                    float xheight,   //for poly approx
                    FCOORD rotation  //for landscape
                   ) {
  PBLOB *copy;                   //copy of blob
  POLYPT *polypt;                //current point
  OUTLINE_IT out_it;
  POLYPT_IT poly_it;             //outline pts

  copy = new PBLOB (blob, xheight);
  out_it.set_to_list (copy->out_list ());
  for (out_it.mark_cycle_pt (); !out_it.cycled_list (); out_it.forward ()) {
                                 //get points
    poly_it.set_to_list (out_it.data ()->polypts ());
    for (poly_it.mark_cycle_pt (); !poly_it.cycled_list ();
    poly_it.forward ()) {
      polypt = poly_it.data ();
                                 //rotate it
      polypt->pos.rotate (rotation);
      polypt->vec.rotate (rotation);
    }
    out_it.data ()->compute_bb ();
  }
  return copy;
}


/**********************************************************************
 * crotate_cblob
 *
 * Rotate the copy by the given vector and return a C_BLOB.
 **********************************************************************/

C_BLOB *crotate_cblob(                 //rotate it
                      C_BLOB *blob,    //blob to search
                      FCOORD rotation  //for landscape
                     ) {
  C_OUTLINE_LIST out_list;       //output outlines
                                 //input outlines
  C_OUTLINE_IT in_it = blob->out_list ();
                                 //output outlines
  C_OUTLINE_IT out_it = &out_list;

  for (in_it.mark_cycle_pt (); !in_it.cycled_list (); in_it.forward ()) {
    out_it.add_after_then_move (new C_OUTLINE (in_it.data (), rotation));
  }
  return new C_BLOB (&out_list);
}


/**********************************************************************
 * box_next
 *
 * Compute the bounding box of this blob with merging of x overlaps
 * but no pre-chopping.
 * Then move the iterator on to the start of the next blob.
 **********************************************************************/

TBOX box_next(                 //get bounding box
             BLOBNBOX_IT *it  //iterator to blobds
            ) {
  BLOBNBOX *blob;                //current blob
  TBOX result;                    //total box

  blob = it->data ();
  result = blob->bounding_box ();
  do {
    it->forward ();
    blob = it->data ();
    if (blob->blob () == NULL && blob->cblob () == NULL)
                                 //was pre-chopped
      result += blob->bounding_box ();
  }
                                 //until next real blob
  while ((blob->blob () == NULL && blob->cblob () == NULL) || blob->joined_to_prev ());
  return result;
}


/**********************************************************************
 * box_next_pre_chopped
 *
 * Compute the bounding box of this blob with merging of x overlaps
 * but WITH pre-chopping.
 * Then move the iterator on to the start of the next pre-chopped blob.
 **********************************************************************/

TBOX box_next_pre_chopped(                 //get bounding box
                         BLOBNBOX_IT *it  //iterator to blobds
                        ) {
  BLOBNBOX *blob;                //current blob
  TBOX result;                    //total box

  blob = it->data ();
  result = blob->bounding_box ();
  do {
    it->forward ();
    blob = it->data ();
  }
                                 //until next real blob
  while (blob->joined_to_prev ());
  return result;
}


/**********************************************************************
 * TO_ROW::TO_ROW
 *
 * Constructor to make a row from a blob.
 **********************************************************************/

TO_ROW::TO_ROW (                 //constructor
BLOBNBOX * blob,                 //first blob
float top,                       //corrected top
float bottom,                    //of row
float row_size                   //ideal
): y_min(bottom), y_max(top), initial_y_min(bottom), num_repeated_sets_(-1) {
  float diff;                    //in size
  BLOBNBOX_IT it = &blobs;       //list of blobs

  it.add_to_end (blob);
  diff = top - bottom - row_size;
  if (diff > 0) {
    y_max -= diff / 2;
    y_min += diff / 2;
  }
                                 //very small object
  else if ((top - bottom) * 3 < row_size) {
    diff = row_size / 3 + bottom - top;
    y_max += diff / 2;
    y_min -= diff / 2;
  }
}


/**********************************************************************
 * TO_ROW:add_blob
 *
 * Add the blob to the end of the row.
 **********************************************************************/

void TO_ROW::add_blob(                 //constructor
                      BLOBNBOX *blob,  //first blob
                      float top,       //corrected top
                      float bottom,    //of row
                      float row_size   //ideal
                     ) {
  float allowed;                 //allowed expansion
  float available;               //expansion
  BLOBNBOX_IT it = &blobs;       //list of blobs

  it.add_to_end (blob);
  allowed = row_size + y_min - y_max;
  if (allowed > 0) {
    available = top > y_max ? top - y_max : 0;
    if (bottom < y_min)
                                 //total available
        available += y_min - bottom;
    if (available > 0) {
      available += available;    //do it gradually
      if (available < allowed)
        available = allowed;
      if (bottom < y_min)
        y_min -= (y_min - bottom) * allowed / available;
      if (top > y_max)
        y_max += (top - y_max) * allowed / available;
    }
  }
}


/**********************************************************************
 * TO_ROW:insert_blob
 *
 * Add the blob to the row in the correct position.
 **********************************************************************/

void TO_ROW::insert_blob(                //constructor
                         BLOBNBOX *blob  //first blob
                        ) {
  BLOBNBOX_IT it = &blobs;       //list of blobs

  if (it.empty ())
    it.add_before_then_move (blob);
  else {
    it.mark_cycle_pt ();
    while (!it.cycled_list ()
      && it.data ()->bounding_box ().left () <=
      blob->bounding_box ().left ())
      it.forward ();
    if (it.cycled_list ())
      it.add_to_end (blob);
    else
      it.add_before_stay_put (blob);
  }
}


/**********************************************************************
 * TO_ROW::compute_vertical_projection
 *
 * Compute the vertical projection of a TO_ROW from its blobs.
 **********************************************************************/

void TO_ROW::compute_vertical_projection() {  //project whole row
  TBOX row_box;                   //bound of row
  BLOBNBOX *blob;                //current blob
  TBOX blob_box;                  //bounding box
  BLOBNBOX_IT blob_it = blob_list ();

  if (blob_it.empty ())
    return;
  row_box = blob_it.data ()->bounding_box ();
  for (blob_it.mark_cycle_pt (); !blob_it.cycled_list (); blob_it.forward ())
    row_box += blob_it.data ()->bounding_box ();

  projection.set_range (row_box.left () - PROJECTION_MARGIN,
    row_box.right () + PROJECTION_MARGIN);
  projection_left = row_box.left () - PROJECTION_MARGIN;
  projection_right = row_box.right () + PROJECTION_MARGIN;
  for (blob_it.mark_cycle_pt (); !blob_it.cycled_list (); blob_it.forward ()) {
    blob = blob_it.data ();
    if (blob->blob () != NULL)
      vertical_blob_projection (blob->blob (), &projection);
    else if (blob->cblob () != NULL)
      vertical_cblob_projection (blob->cblob (), &projection);
  }
}


/**********************************************************************
 * vertical_blob_projection
 *
 * Compute the vertical projection of a blob from its outlines
 * and add to the given STATS.
 **********************************************************************/

void vertical_blob_projection(              //project outlines
                              PBLOB *blob,  //blob to project
                              STATS *stats  //output
                             ) {
                                 //outlines of blob
  OUTLINE_IT out_it = blob->out_list ();

  for (out_it.mark_cycle_pt (); !out_it.cycled_list (); out_it.forward ()) {
    vertical_outline_projection (out_it.data (), stats);
  }
}


/**********************************************************************
 * vertical_outline_projection
 *
 * Compute the vertical projection of a outline from its outlines
 * and add to the given STATS.
 **********************************************************************/

void vertical_outline_projection(                   //project outlines
                                 OUTLINE *outline,  //outline to project
                                 STATS *stats       //output
                                ) {
  POLYPT *polypt;                //current point
  inT32 xcoord;                  //current pixel coord
  float end_x;                   //end of vec
  POLYPT_IT poly_it = outline->polypts ();
  OUTLINE_IT out_it = outline->child ();
  float ymean;                   //amount to add
  float width;                   //amount of x

  for (poly_it.mark_cycle_pt (); !poly_it.cycled_list (); poly_it.forward ()) {
    polypt = poly_it.data ();
    end_x = polypt->pos.x () + polypt->vec.x ();
    if (polypt->vec.x () > 0) {
      for (xcoord = (inT32) floor (polypt->pos.x ());
      xcoord < end_x; xcoord++) {
        if (polypt->pos.x () < xcoord) {
          width = (float) xcoord;
          ymean =
            polypt->vec.y () * (xcoord -
            polypt->pos.x ()) / polypt->vec.x () +
            polypt->pos.y ();
        }
        else {
          width = polypt->pos.x ();
          ymean = polypt->pos.y ();
        }
        if (end_x > xcoord + 1) {
          width -= xcoord + 1;
          ymean +=
            polypt->vec.y () * (xcoord + 1 -
            polypt->pos.x ()) / polypt->vec.x () +
            polypt->pos.y ();
        }
        else {
          width -= end_x;
          ymean += polypt->pos.y () + polypt->vec.y ();
        }
        ymean = ymean * width / 2;
        stats->add (xcoord, (inT32) floor (ymean + 0.5));
      }
    }
    else if (polypt->vec.x () < 0) {
      for (xcoord = (inT32) floor (end_x);
      xcoord < polypt->pos.x (); xcoord++) {
        if (polypt->pos.x () > xcoord + 1) {
          width = xcoord + 1.0f;
          ymean =
            polypt->vec.y () * (xcoord + 1 -
            polypt->pos.x ()) / polypt->vec.x () +
            polypt->pos.y ();
        }
        else {
          width = polypt->pos.x ();
          ymean = polypt->pos.y ();
        }
        if (end_x < xcoord) {
          width -= xcoord;
          ymean +=
            polypt->vec.y () * (xcoord -
            polypt->pos.x ()) / polypt->vec.x () +
            polypt->pos.y ();
        }
        else {
          width -= end_x;
          ymean += polypt->pos.y () + polypt->vec.y ();
        }
        ymean = ymean * width / 2;
        stats->add (xcoord, (inT32) floor (ymean + 0.5));
      }
    }
  }

  for (out_it.mark_cycle_pt (); !out_it.cycled_list (); out_it.forward ()) {
    vertical_outline_projection (out_it.data (), stats);
  }
}


/**********************************************************************
 * vertical_cblob_projection
 *
 * Compute the vertical projection of a cblob from its outlines
 * and add to the given STATS.
 **********************************************************************/

void vertical_cblob_projection(               //project outlines
                               C_BLOB *blob,  //blob to project
                               STATS *stats   //output
                              ) {
                                 //outlines of blob
  C_OUTLINE_IT out_it = blob->out_list ();

  for (out_it.mark_cycle_pt (); !out_it.cycled_list (); out_it.forward ()) {
    vertical_coutline_projection (out_it.data (), stats);
  }
}


/**********************************************************************
 * vertical_coutline_projection
 *
 * Compute the vertical projection of a outline from its outlines
 * and add to the given STATS.
 **********************************************************************/

void vertical_coutline_projection(                     //project outlines
                                  C_OUTLINE *outline,  //outline to project
                                  STATS *stats         //output
                                 ) {
  ICOORD pos;                    //current point
  ICOORD step;                   //edge step
  inT32 length;                  //of outline
  inT16 stepindex;               //current step
  C_OUTLINE_IT out_it = outline->child ();

  pos = outline->start_pos ();
  length = outline->pathlength ();
  for (stepindex = 0; stepindex < length; stepindex++) {
    step = outline->step (stepindex);
    if (step.x () > 0) {
      if (pitsync_projection_fix)
        stats->add (pos.x (), -pos.y ());
      else
        stats->add (pos.x (), pos.y ());
    }
    else if (step.x () < 0) {
      if (pitsync_projection_fix)
        stats->add (pos.x () - 1, pos.y ());
      else
        stats->add (pos.x () - 1, -pos.y ());
    }
    pos += step;
  }

  for (out_it.mark_cycle_pt (); !out_it.cycled_list (); out_it.forward ()) {
    vertical_coutline_projection (out_it.data (), stats);
  }
}


/**********************************************************************
 * TO_BLOCK::TO_BLOCK
 *
 * Constructor to make a TO_BLOCK from a real block.
 **********************************************************************/

TO_BLOCK::TO_BLOCK(                  //make a block
                   BLOCK *src_block  //real block
                  ) {
  block = src_block;
}

static void clear_blobnboxes(BLOBNBOX_LIST* boxes) {
  BLOBNBOX_IT it = boxes;
  // A BLOBNBOX generally doesn't own its blobs, so if they do, you
  // have to delete them explicitly.
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    BLOBNBOX* box = it.data();
    if (box->blob() != NULL)
      delete box->blob();
    if (box->cblob() != NULL)
      delete box->cblob();
  }
}

TO_BLOCK::~TO_BLOCK() {
  // Any residual BLOBNBOXes at this stage own their blobs, so delete them.
  clear_blobnboxes(&blobs);
  clear_blobnboxes(&underlines);
  clear_blobnboxes(&noise_blobs);
  clear_blobnboxes(&small_blobs);
  clear_blobnboxes(&large_blobs);
}

#ifndef GRAPHICS_DISABLED
// Draw the blobs on the various lists in the block in different colors.
void TO_BLOCK::plot_graded_blobs(ScrollView* to_win) {
  plot_blob_list(to_win, &noise_blobs, ScrollView::CORAL, ScrollView::BLUE);
  plot_blob_list(to_win, &small_blobs,
                 ScrollView::GOLDENROD, ScrollView::YELLOW);
  plot_blob_list(to_win, &large_blobs,
                 ScrollView::DARK_GREEN, ScrollView::YELLOW);
  plot_blob_list(to_win, &blobs, ScrollView::WHITE, ScrollView::BROWN);
}

/**********************************************************************
 * plot_blob_list
 *
 * Draw a list of blobs.
 **********************************************************************/

void plot_blob_list(ScrollView* win,                   // window to draw in
                    BLOBNBOX_LIST *list,               // blob list
                    ScrollView::Color body_colour,     // colour to draw
                    ScrollView::Color child_colour) {  // colour of child
  BLOBNBOX_IT it = list;
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    it.data()->plot(win, body_colour, child_colour);
  }
}

#endif //GRAPHICS_DISABLED

