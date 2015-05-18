/**********************************************************************
 * File:        pdblock.c  (Formerly pdblk.c)
 * Description: PDBLK member functions and iterator functions.
 * Author:					Ray Smith
 * Created:					Fri Mar 15 09:41:28 GMT 1991
 *
 * (C) Copyright 1991, Hewlett-Packard Ltd.
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

#include          <stdlib.h>
#include          "allheaders.h"
#include          "blckerr.h"
#include          "pdblock.h"

// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif

#define BLOCK_LABEL_HEIGHT  150  //char height of block id

CLISTIZE (PDBLK)
/**********************************************************************
 * PDBLK::PDBLK
 *
 * Constructor for a simple rectangular block.
 **********************************************************************/
PDBLK::PDBLK (                   //rectangular block
inT16 xmin,                      //bottom left
inT16 ymin, inT16 xmax,          //top right
inT16 ymax):    box (ICOORD (xmin, ymin), ICOORD (xmax, ymax)) {
                                 //boundaries
  ICOORDELT_IT left_it = &leftside;
  ICOORDELT_IT right_it = &rightside;

  hand_poly = NULL;
  left_it.set_to_list (&leftside);
  right_it.set_to_list (&rightside);
                                 //make default box
  left_it.add_to_end (new ICOORDELT (xmin, ymin));
  left_it.add_to_end (new ICOORDELT (xmin, ymax));
  right_it.add_to_end (new ICOORDELT (xmax, ymin));
  right_it.add_to_end (new ICOORDELT (xmax, ymax));
  index_ = 0;
}


/**********************************************************************
 * PDBLK::set_sides
 *
 * Sets left and right vertex lists
 **********************************************************************/

void PDBLK::set_sides(                       //set vertex lists
                      ICOORDELT_LIST *left,  //left vertices
                      ICOORDELT_LIST *right  //right vertices
                     ) {
                                 //boundaries
  ICOORDELT_IT left_it = &leftside;
  ICOORDELT_IT right_it = &rightside;

  leftside.clear ();
  left_it.move_to_first ();
  left_it.add_list_before (left);
  rightside.clear ();
  right_it.move_to_first ();
  right_it.add_list_before (right);
}

/**********************************************************************
 * PDBLK::contains
 *
 * Return TRUE if the given point is within the block.
 **********************************************************************/

BOOL8 PDBLK::contains(           //test containment
                      ICOORD pt  //point to test
                     ) {
  BLOCK_RECT_IT it = this;       //rectangle iterator
  ICOORD bleft, tright;          //corners of rectangle

  for (it.start_block (); !it.cycled_rects (); it.forward ()) {
                                 //get rectangle
    it.bounding_box (bleft, tright);
                                 //inside rect
    if (pt.x () >= bleft.x () && pt.x () <= tright.x ()
      && pt.y () >= bleft.y () && pt.y () <= tright.y ())
      return TRUE;               //is inside
  }
  return FALSE;                  //not inside
}


/**********************************************************************
 * PDBLK::move
 *
 * Reposition block
 **********************************************************************/

void PDBLK::move(                  // reposition block
                 const ICOORD vec  // by vector
                ) {
  ICOORDELT_IT it(&leftside);

  for (it.mark_cycle_pt (); !it.cycled_list (); it.forward ())
    *(it.data ()) += vec;

  it.set_to_list (&rightside);

  for (it.mark_cycle_pt (); !it.cycled_list (); it.forward ())
    *(it.data ()) += vec;

  box.move (vec);
}

// Returns a binary Pix mask with a 1 pixel for every pixel within the
// block. Rotates the coordinate system by rerotation prior to rendering.
Pix* PDBLK::render_mask(const FCOORD& rerotation, TBOX* mask_box) {
  TBOX rotated_box(box);
  rotated_box.rotate(rerotation);
  Pix* pix = pixCreate(rotated_box.width(), rotated_box.height(), 1);
  if (hand_poly != NULL) {
    // We are going to rotate, so get a deep copy of the points and
    // make a new POLY_BLOCK with it.
    ICOORDELT_LIST polygon;
    polygon.deep_copy(hand_poly->points(), ICOORDELT::deep_copy);
    POLY_BLOCK image_block(&polygon, hand_poly->isA());
    image_block.rotate(rerotation);
    // Block outline is a polygon, so use a PB_LINE_IT to get the
    // rasterized interior. (Runs of interior pixels on a line.)
    PB_LINE_IT *lines = new PB_LINE_IT(&image_block);
    for (int y = box.bottom(); y < box.top(); ++y) {
      ICOORDELT_LIST* segments = lines->get_line(y);
      if (!segments->empty()) {
        ICOORDELT_IT s_it(segments);
        // Each element of segments is a start x and x size of the
        // run of interior pixels.
        for (s_it.mark_cycle_pt(); !s_it.cycled_list(); s_it.forward()) {
          int start = s_it.data()->x();
          int xext = s_it.data()->y();
          // Set the run of pixels to 1.
          pixRasterop(pix, start - rotated_box.left(),
                      rotated_box.height() - 1 - (y - rotated_box.bottom()),
                      xext, 1, PIX_SET, NULL, 0, 0);
        }
      }
      delete segments;
    }
    delete lines;
  } else {
    // Just fill the whole block as there is only a bounding box.
    pixRasterop(pix, 0, 0, rotated_box.width(), rotated_box.height(),
                PIX_SET, NULL, 0, 0);
  }
  if (mask_box != NULL) *mask_box = rotated_box;
  return pix;
}


/**********************************************************************
 * PDBLK::plot
 *
 * Plot the outline of a block in the given colour.
 **********************************************************************/

#ifndef GRAPHICS_DISABLED
void PDBLK::plot(                //draw outline
                 ScrollView* window,  //window to draw in
                 inT32 serial,   //serial number
                 ScrollView::Color colour   //colour to draw in
                ) {
  ICOORD startpt;                //start of outline
  ICOORD endpt;                  //end of outline
  ICOORD prevpt;                 //previous point
  ICOORDELT_IT it = &leftside;   //iterator

                                 //set the colour
  window->Pen(colour);
  window->TextAttributes("Times", BLOCK_LABEL_HEIGHT, false, false, false);

  if (hand_poly != NULL) {
    hand_poly->plot(window, serial);
  } else if (!leftside.empty ()) {
    startpt = *(it.data ());     //bottom left corner
    //              tprintf("Block %d bottom left is (%d,%d)\n",
    //                      serial,startpt.x(),startpt.y());
    char temp_buff[34];
    #if defined(__UNIX__) || defined(MINGW)
    sprintf(temp_buff, INT32FORMAT, serial);
    #else
    ultoa (serial, temp_buff, 10);
    #endif
    window->Text(startpt.x (), startpt.y (), temp_buff);

    window->SetCursor(startpt.x (), startpt.y ());
    do {
      prevpt = *(it.data ());    //previous point
      it.forward ();             //move to next point
                                 //draw round corner
    window->DrawTo(prevpt.x (), it.data ()->y ());
    window->DrawTo(it.data ()->x (), it.data ()->y ());
    }
    while (!it.at_last ());      //until end of list
    endpt = *(it.data ());       //end point

                                 //other side of boundary
    window->SetCursor(startpt.x (), startpt.y ());
    it.set_to_list (&rightside);
    prevpt = startpt;
    for (it.mark_cycle_pt (); !it.cycled_list (); it.forward ()) {
                                 //draw round corner
    window->DrawTo(prevpt.x (), it.data ()->y ());
    window->DrawTo(it.data ()->x (), it.data ()->y ());
      prevpt = *(it.data ());    //previous point
    }
                                 //close boundary
    window->DrawTo(endpt.x(), endpt.y());
  }
}
#endif

/**********************************************************************
 * PDBLK::operator=
 *
 * Assignment - duplicate the block structure, but with an EMPTY row list.
 **********************************************************************/

PDBLK & PDBLK::operator= (       //assignment
const PDBLK & source             //from this
) {
  //      this->ELIST_LINK::operator=(source);
  if (!leftside.empty ())
    leftside.clear ();
  if (!rightside.empty ())
    rightside.clear ();
  leftside.deep_copy(&source.leftside, &ICOORDELT::deep_copy);
  rightside.deep_copy(&source.rightside, &ICOORDELT::deep_copy);
  box = source.box;
  return *this;
}


/**********************************************************************
 * BLOCK_RECT_IT::BLOCK_RECT_IT
 *
 * Construct a block rectangle iterator.
 **********************************************************************/

BLOCK_RECT_IT::BLOCK_RECT_IT (
//iterate rectangles
PDBLK * blkptr                   //from block
):left_it (&blkptr->leftside), right_it (&blkptr->rightside) {
  block = blkptr;                //remember block
                                 //non empty list
  if (!blkptr->leftside.empty ()) {
    start_block();  //ready for iteration
  }
}


/**********************************************************************
 * BLOCK_RECT_IT::set_to_block
 *
 * Start a new block.
 **********************************************************************/

void BLOCK_RECT_IT::set_to_block(                  //start (new) block
                                 PDBLK *blkptr) {  //block to start
  block = blkptr;                //remember block
                                 //set iterators
  left_it.set_to_list (&blkptr->leftside);
  right_it.set_to_list (&blkptr->rightside);
  if (!blkptr->leftside.empty ())
    start_block();  //ready for iteration
}


/**********************************************************************
 * BLOCK_RECT_IT::start_block
 *
 * Restart a block.
 **********************************************************************/

void BLOCK_RECT_IT::start_block() {  //start (new) block
  left_it.move_to_first ();
  right_it.move_to_first ();
  left_it.mark_cycle_pt ();
  right_it.mark_cycle_pt ();
  ymin = left_it.data ()->y ();  //bottom of first box
  ymax = left_it.data_relative (1)->y ();
  if (right_it.data_relative (1)->y () < ymax)
                                 //smallest step
    ymax = right_it.data_relative (1)->y ();
}


/**********************************************************************
 * BLOCK_RECT_IT::forward
 *
 * Move to the next rectangle in the block.
 **********************************************************************/

void BLOCK_RECT_IT::forward() {  //next rectangle
  if (!left_it.empty ()) {       //non-empty list
    if (left_it.data_relative (1)->y () == ymax)
      left_it.forward ();        //move to meet top
    if (right_it.data_relative (1)->y () == ymax)
      right_it.forward ();
                                 //last is special
    if (left_it.at_last () || right_it.at_last ()) {
      left_it.move_to_first ();  //restart
      right_it.move_to_first ();
                                 //now at bottom
      ymin = left_it.data ()->y ();
    }
    else {
      ymin = ymax;               //new bottom
    }
                                 //next point
    ymax = left_it.data_relative (1)->y ();
    if (right_it.data_relative (1)->y () < ymax)
                                 //least step forward
      ymax = right_it.data_relative (1)->y ();
  }
}


/**********************************************************************
 * BLOCK_LINE_IT::get_line
 *
 * Get the the start and width of a line in the block.
 **********************************************************************/

inT16 BLOCK_LINE_IT::get_line(             //get a line
                              inT16 y,     //line to get
                              inT16 &xext  //output extent
                             ) {
  ICOORD bleft;                  //bounding box
  ICOORD tright;                 //of block & rect

                                 //get block box
  block->bounding_box (bleft, tright);
  if (y < bleft.y () || y >= tright.y ()) {
    //              block->print(stderr,FALSE);
    BADBLOCKLINE.error ("BLOCK_LINE_IT::get_line", ABORT, "Y=%d", y);
  }

                                 //get rectangle box
  rect_it.bounding_box (bleft, tright);
                                 //inside rectangle
  if (y >= bleft.y () && y < tright.y ()) {
                                 //width of line
    xext = tright.x () - bleft.x ();
    return bleft.x ();           //start of line
  }
  for (rect_it.start_block (); !rect_it.cycled_rects (); rect_it.forward ()) {
                                 //get rectangle box
    rect_it.bounding_box (bleft, tright);
                                 //inside rectangle
    if (y >= bleft.y () && y < tright.y ()) {
                                 //width of line
      xext = tright.x () - bleft.x ();
      return bleft.x ();         //start of line
    }
  }
  LOSTBLOCKLINE.error ("BLOCK_LINE_IT::get_line", ABORT, "Y=%d", y);
  return 0;                      //dummy to stop warning
}
