/**********************************************************************
 * File:        ocrblock.cpp  (Formerly block.c)
 * Description: BLOCK member functions and iterator functions.
 * Author:		Ray Smith
 * Created:		Fri Mar 15 09:41:28 GMT 1991
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

#include "mfcpch.h"
#include          <stdlib.h>
#include          "blckerr.h"
#include          "ocrblock.h"
#include          "tprintf.h"

#define BLOCK_LABEL_HEIGHT  150  //char height of block id

ELISTIZE_S (BLOCK)
/**
 * BLOCK::BLOCK
 *
 * Constructor for a simple rectangular block.
 */
BLOCK::BLOCK(const char *name,                //< filename
             BOOL8 prop,                      //< proportional
             inT16 kern,                      //< kerning
             inT16 space,                     //< spacing
             inT16 xmin,                      //< bottom left
             inT16 ymin, inT16 xmax,          //< top right
             inT16 ymax)
  : PDBLK (xmin, ymin, xmax, ymax),
    filename(name),
    re_rotation_(1.0f, 0.0f),
    classify_rotation_(1.0f, 0.0f),
    skew_(1.0f, 0.0f) {
  ICOORDELT_IT left_it = &leftside;
  ICOORDELT_IT right_it = &rightside;

  proportional = prop;
  kerning = kern;
  spacing = space;
  font_class = -1;               //not assigned
  cell_over_xheight_ = 2.0f;
  hand_poly = NULL;
  left_it.set_to_list (&leftside);
  right_it.set_to_list (&rightside);
                                 //make default box
  left_it.add_to_end (new ICOORDELT (xmin, ymin));
  left_it.add_to_end (new ICOORDELT (xmin, ymax));
  right_it.add_to_end (new ICOORDELT (xmax, ymin));
  right_it.add_to_end (new ICOORDELT (xmax, ymax));
}

/**
 * decreasing_top_order
 *
 * Sort Comparator: Return <0 if row1 top < row2 top
 */

int decreasing_top_order(  //
                         const void *row1,
                         const void *row2) {
  return (*(ROW **) row2)->bounding_box ().top () -
    (*(ROW **) row1)->bounding_box ().top ();
}


/**
 * BLOCK::rotate
 *
 * Rotate the polygon by the given rotation and recompute the bounding_box.
 */
void BLOCK::rotate(const FCOORD& rotation) {
  poly_block()->rotate(rotation);
  box = *poly_block()->bounding_box();
}

/**
 * BLOCK::sort_rows
 *
 * Order rows so that they are in order of decreasing Y coordinate
 */

void BLOCK::sort_rows() {  // order on "top"
  ROW_IT row_it(&rows);

  row_it.sort (decreasing_top_order);
}


/**
 * BLOCK::compress
 *
 * Delete space between the rows. (And maybe one day, compress the rows)
 * Fill space of block from top down, left aligning rows.
 */

void BLOCK::compress() {  // squash it up
  #define           ROW_SPACING 5

  ROW_IT row_it(&rows);
  ROW *row;
  ICOORD row_spacing (0, ROW_SPACING);

  ICOORDELT_IT icoordelt_it;

  sort_rows();

  box = TBOX (box.topleft (), box.topleft ());
  box.move_bottom_edge (ROW_SPACING);
  for (row_it.mark_cycle_pt (); !row_it.cycled_list (); row_it.forward ()) {
    row = row_it.data ();
    row->move (box.botleft () - row_spacing -
      row->bounding_box ().topleft ());
    box += row->bounding_box ();
  }

  leftside.clear ();
  icoordelt_it.set_to_list (&leftside);
  icoordelt_it.add_to_end (new ICOORDELT (box.left (), box.bottom ()));
  icoordelt_it.add_to_end (new ICOORDELT (box.left (), box.top ()));
  rightside.clear ();
  icoordelt_it.set_to_list (&rightside);
  icoordelt_it.add_to_end (new ICOORDELT (box.right (), box.bottom ()));
  icoordelt_it.add_to_end (new ICOORDELT (box.right (), box.top ()));
}


/**
 * BLOCK::check_pitch
 *
 * Check whether the block is fixed or prop, set the flag, and set
 * the pitch if it is fixed.
 */

void BLOCK::check_pitch() {  // check prop
  //      tprintf("Missing FFT fixed pitch stuff!\n");
  pitch = -1;
}


/**
 * BLOCK::compress
 *
 * Compress and move in a single operation.
 */

void BLOCK::compress(                  // squash it up
                     const ICOORD vec  // and move
                    ) {
  box.move (vec);
  compress();
}


/**
 * BLOCK::print
 *
 * Print the info on a block
 */

void BLOCK::print(            //print list of sides
                  FILE *,     //< file to print on
                  BOOL8 dump  //< print full detail
                 ) {
  ICOORDELT_IT it = &leftside;   //iterator

  box.print ();
  tprintf ("Proportional= %s\n", proportional ? "TRUE" : "FALSE");
  tprintf ("Kerning= %d\n", kerning);
  tprintf ("Spacing= %d\n", spacing);
  tprintf ("Fixed_pitch=%d\n", pitch);
  tprintf ("Filename= %s\n", filename.string ());

  if (dump) {
    tprintf ("Left side coords are:\n");
    for (it.mark_cycle_pt (); !it.cycled_list (); it.forward ())
      tprintf ("(%d,%d) ", it.data ()->x (), it.data ()->y ());
    tprintf ("\n");
    tprintf ("Right side coords are:\n");
    it.set_to_list (&rightside);
    for (it.mark_cycle_pt (); !it.cycled_list (); it.forward ())
      tprintf ("(%d,%d) ", it.data ()->x (), it.data ()->y ());
    tprintf ("\n");
  }
}

/**
 * BLOCK::operator=
 *
 * Assignment - duplicate the block structure, but with an EMPTY row list.
 */

BLOCK & BLOCK::operator= (       //assignment
const BLOCK & source             //from this
) {
  this->ELIST_LINK::operator= (source);
  this->PDBLK::operator= (source);
  proportional = source.proportional;
  kerning = source.kerning;
  spacing = source.spacing;
  filename = source.filename;    //STRINGs assign ok
  if (!rows.empty ())
    rows.clear ();
  re_rotation_ = source.re_rotation_;
  classify_rotation_ = source.classify_rotation_;
  skew_ = source.skew_;
  return *this;
}
