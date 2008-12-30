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
/**********************************************************************
 * BLOCK::BLOCK
 *
 * Constructor for a simple rectangular block.
 **********************************************************************/
BLOCK::BLOCK (                   //rectangular block
const char *name,                //filename
BOOL8 prop,                      //proportional
inT16 kern,                      //kerning
inT16 space,                     //spacing
inT16 xmin,                      //bottom left
inT16 ymin, inT16 xmax,          //top right
inT16 ymax):
PDBLK (xmin, ymin, xmax, ymax),
filename(name) {  //box(ICOORD(xmin,ymin),ICOORD(xmax,ymax))
                                 //boundaries
  ICOORDELT_IT left_it = &leftside;
  ICOORDELT_IT right_it = &rightside;

  proportional = prop;
  kerning = kern;
  spacing = space;
  font_class = -1;               //not assigned
  cell_over_xheight_ = 2.0f;
  hand_block = NULL;
  hand_poly = NULL;
  left_it.set_to_list (&leftside);
  right_it.set_to_list (&rightside);
                                 //make default box
  left_it.add_to_end (new ICOORDELT (xmin, ymin));
  left_it.add_to_end (new ICOORDELT (xmin, ymax));
  right_it.add_to_end (new ICOORDELT (xmax, ymin));
  right_it.add_to_end (new ICOORDELT (xmax, ymax));
}


/**********************************************************************
 * BLOCK::set_sides
 *
 * Sets left and right vertex lists
 **********************************************************************/

//void                                                          BLOCK::set_sides(                       //set vertex lists
//ICOORDELT_LIST             *left,                        //left vertices
//ICOORDELT_LIST             *right                        //right vertices
//)
//{
//      ICOORDELT_IT                            left_it= &leftside;                             //boundaries
//      ICOORDELT_IT                            right_it= &rightside;

//      leftside.clear();
//      left_it.move_to_first();
//      left_it.add_list_before(left);
//      rightside.clear();
//      right_it.move_to_first();
//      right_it.add_list_before(right);
//}

/**********************************************************************
 * BLOCK::contains
 *
 * Return TRUE if the given point is within the block.
 **********************************************************************/

//BOOL8                                                         BLOCK::contains(                                        //test containment
//ICOORD                                                        pt                                                                              //point to test
//)
//{
//      BLOCK_RECT_IT                           it=this;                                                                //rectangle iterator
//      ICOORD                                          bleft,tright;                                           //corners of rectangle

//      for (it.start_block();!it.cycled_rects();it.forward())
//      {
//              it.bounding_box(bleft,tright);                                                  //get rectangle
//              if (pt.x()>=bleft.x() && pt.x()<=tright.x()                     //inside rect
//              && pt.y()>=bleft.y() && pt.y()<=tright.y())
//                      return TRUE;                                                                                            //is inside
//      }
//      return FALSE;                                                                                                           //not inside
//}

/**********************************************************************
 * BLOCK::move
 *
 * Reposition block
 **********************************************************************/

//void                                                     BLOCK::move(                                         // reposition block
//const ICOORD                                  vec                                                                     // by vector
//)
//{
//   ROW_IT                                             row_it( &rows );
//      ICOORDELT_IT                            it( &leftside );

//      for( row_it.mark_cycle_pt(); !row_it.cycled_list(); row_it.forward() )
//         row_it.data()->move( vec );

//      for( it.mark_cycle_pt(); !it.cycled_list(); it.forward() )
//         *(it.data()) += vec;

//   it.set_to_list( &rightside );

//   for( it.mark_cycle_pt(); !it.cycled_list(); it.forward() )
//         *(it.data()) += vec;

//   box.move( vec );
//}

/**********************************************************************
 * decreasing_top_order
 *
 * Sort Comparator: Return <0 if row1 top < row2 top
 **********************************************************************/

int decreasing_top_order(  //
                         const void *row1,
                         const void *row2) {
  return (*(ROW **) row2)->bounding_box ().top () -
    (*(ROW **) row1)->bounding_box ().top ();
}


/**********************************************************************
 * BLOCK::sort_rows
 *
 * Order rows so that they are in order of decreasing Y coordinate
 **********************************************************************/

void BLOCK::sort_rows() {  // order on "top"
  ROW_IT row_it(&rows);

  row_it.sort (decreasing_top_order);
}


/**********************************************************************
 * BLOCK::compress
 *
 * Delete space between the rows. (And maybe one day, compress the rows)
 * Fill space of block from top down, left aligning rows.
 **********************************************************************/

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


/**********************************************************************
 * BLOCK::check_pitch
 *
 * Check whether the block is fixed or prop, set the flag, and set
 * the pitch if it is fixed.
 **********************************************************************/

void BLOCK::check_pitch() {  // check prop
  //      tprintf("Missing FFT fixed pitch stuff!\n");
  pitch = -1;
}


/**********************************************************************
 * BLOCK::compress
 *
 * Compress and move in a single operation.
 **********************************************************************/

void BLOCK::compress(                  // squash it up
                     const ICOORD vec  // and move
                    ) {
  box.move (vec);
  compress();
}


/**********************************************************************
 * BLOCK::print
 *
 * Print the info on a block
 **********************************************************************/

void BLOCK::print(            //print list of sides
                  FILE *,     //file to print on
                  BOOL8 dump  //print full detail
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


/**********************************************************************
 * BLOCK::plot
 *
 * Plot the outline of a block in the given colour.
 **********************************************************************/

//void                                                          BLOCK::plot(                                            //draw outline
//WINDOW                                                        window,                                                         //window to draw in
//inT32                                                         serial,                                                         //serial number
//COLOUR                                                        colour                                                          //colour to draw in
//)
//{
//      ICOORD                                          startpt;                                                                //start of outline
//      ICOORD                                          endpt;                                                          //end of outline
//      ICOORD                                          prevpt;                                                         //previous point
//      ICOORDELT_IT                            it= &leftside;                                          //iterator
//      char                                                    number[32];                                                     //block id

//      line_color_index(window,colour);                                                                //set the colour
//      text_color_index(window,colour);
//      character_height(window,(float)BLOCK_LABEL_HEIGHT);
//      text_font_index(window,6);

//      if (!leftside.empty())
//      {
//              startpt= *(it.data());                                                                          //bottom left corner
////            fprintf(stderr,"Block %d bottom left is (%d,%d)\n",
////                    serial,startpt.x(),startpt.y());
//              sprintf(number,"%d",serial);
//              text2d(window,startpt.x(),startpt.y(),number,0,FALSE);

//              move2d(window,startpt.x(),startpt.y());
//              do
//              {
//                      prevpt= *(it.data());                                                                   //previous point
//                      it.forward();                                                                                           //move to next point
//                      draw2d(window,prevpt.x(),it.data()->y());                       //draw round corner
//                      draw2d(window,it.data()->x(),it.data()->y());
//              }
//              while (!it.at_last());                                                                          //until end of list
//              endpt= *(it.data());                                                                                    //end point

//              move2d(window,startpt.x(),startpt.y());                         //other side of boundary
//              it.set_to_list(&rightside);
//              prevpt=startpt;
//              for (it.mark_cycle_pt();!it.cycled_list();it.forward())
//              {
//                      draw2d(window,prevpt.x(),it.data()->y());                       //draw round corner
//                      draw2d(window,it.data()->x(),it.data()->y());
//                      prevpt= *(it.data());                                                                   //previous point
//              }
//              draw2d(window,endpt.x(),endpt.y());                                             //close boundary
//              if (hand_block!=NULL)
//                      hand_block->plot(window,colour,serial);
//      }
//}

/**********************************************************************
 * BLOCK::show
 *
 * Show the image corresponding to a block as its set of rectangles.
 **********************************************************************/

//void                                                          BLOCK::show(                                            //show image block
//IMAGE                                                         *image,                                                         //image to show
//WINDOW                                                        window                                                          //window to show in
//)
//{
//      BLOCK_RECT_IT                           it=this;                                                                //rectangle iterator
//      ICOORD                                          bleft,tright;                                           //corners of rectangle

//      for (it.start_block();!it.cycled_rects();it.forward())
//      {
//              it.bounding_box(bleft,tright);                                                  //get rectangle
////            fprintf(stderr,"Drawing a block with a bottom left of (%d,%d)\n",
////                    bleft.x(),bleft.y());
//              show_sub_image(image,bleft.x(),bleft.y(),
//                      tright.x()-bleft.x(),tright.y()-bleft.y(),
//                      window,bleft.x(),bleft.y());                                                    //show it
//      }
//}

/**********************************************************************
 * BLOCK::operator=
 *
 * Assignment - duplicate the block structure, but with an EMPTY row list.
 **********************************************************************/

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
  //   if ( !leftside.empty() )
  //              leftside.clear();
  // if ( !rightside.empty() )
  //              rightside.clear();
  //   leftside.deep_copy( &source.leftside );
  //   rightside.deep_copy( &source.rightside );
  //      box=source.box;
  return *this;
}
