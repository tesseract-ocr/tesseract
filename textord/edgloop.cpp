/**********************************************************************
 * File:        edgloop.c  (Formerly edgeloop.c)
 * Description: Functions to clean up an outline before approximation.
 * Author:					Ray Smith
 * Created:					Tue Mar 26 16:56:25 GMT 1991
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

#include          "scanedg.h"
#include          "drawedg.h"
#include          "edgloop.h"

// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif

#define MINEDGELENGTH   8        // min decent length

/**********************************************************************
 * complete_edge
 *
 * Complete the edge by cleaning it up.
 **********************************************************************/

void complete_edge(CRACKEDGE *start,  //start of loop
                   C_OUTLINE_IT* outline_it) {
  ScrollView::Color colour;                 //colour to draw in
  inT16 looplength;              //steps in loop
  ICOORD botleft;                //bounding box
  ICOORD topright;
  C_OUTLINE *outline;            //new outline

                                 //check length etc.
  colour = check_path_legal (start);

  if (colour == ScrollView::RED || colour == ScrollView::BLUE) {
    looplength = loop_bounding_box (start, botleft, topright);
    outline = new C_OUTLINE (start, botleft, topright, looplength);
                                 //add to list
    outline_it->add_after_then_move (outline);
  }
}


/**********************************************************************
 * check_path_legal
 *
 * Check that the outline is legal for length and for chaincode sum.
 * The return value is RED for a normal black-inside outline,
 * BLUE for a white-inside outline, MAGENTA if it is too short,
 * YELLOW if it is too long, and GREEN if it is illegal.
 * These colours are used to draw the raw outline.
 **********************************************************************/

ScrollView::Color check_path_legal(                  //certify outline
                        CRACKEDGE *start  //start of loop
                       ) {
  int lastchain;              //last chain code
  int chaindiff;               //chain code diff
  inT32 length;                  //length of loop
  inT32 chainsum;                //sum of chain diffs
  CRACKEDGE *edgept;             //current point
  const ERRCODE ED_ILLEGAL_SUM = "Illegal sum of chain codes";

  length = 0;
  chainsum = 0;                  //sum of chain codes
  edgept = start;
  lastchain = edgept->prev->stepdir; //previous chain code
  do {
    length++;
    if (edgept->stepdir != lastchain) {
                                 //chain code difference
      chaindiff = edgept->stepdir - lastchain;
      if (chaindiff > 2)
        chaindiff -= 4;
      else if (chaindiff < -2)
        chaindiff += 4;
      chainsum += chaindiff;     //sum differences
      lastchain = edgept->stepdir;
    }
    edgept = edgept->next;
  }
  while (edgept != start && length < C_OUTLINE::kMaxOutlineLength);

  if ((chainsum != 4 && chainsum != -4)
  || edgept != start || length < MINEDGELENGTH) {
    if (edgept != start) {
     return ScrollView::YELLOW;
    } else if (length < MINEDGELENGTH) {
     return ScrollView::MAGENTA;
    } else {
      ED_ILLEGAL_SUM.error ("check_path_legal", TESSLOG, "chainsum=%d",
        chainsum);
      return ScrollView::GREEN;
    }
  }
                                 //colour on inside
  return chainsum < 0 ? ScrollView::BLUE : ScrollView::RED;
}

/**********************************************************************
 * loop_bounding_box
 *
 * Find the bounding box of the edge loop.
 **********************************************************************/

inT16 loop_bounding_box(                    //get bounding box
                        CRACKEDGE *&start,  //edge loop
                        ICOORD &botleft,    //bounding box
                        ICOORD &topright) {
  inT16 length;                  //length of loop
  inT16 leftmost;                //on top row
  CRACKEDGE *edgept;             //current point
  CRACKEDGE *realstart;          //topleft start

  edgept = start;
  realstart = start;
  botleft = topright = ICOORD (edgept->pos.x (), edgept->pos.y ());
  leftmost = edgept->pos.x ();
  length = 0;                    //coutn length
  do {
    edgept = edgept->next;
    if (edgept->pos.x () < botleft.x ())
                                 //get bounding box
      botleft.set_x (edgept->pos.x ());
    else if (edgept->pos.x () > topright.x ())
      topright.set_x (edgept->pos.x ());
    if (edgept->pos.y () < botleft.y ())
                                 //get bounding box
      botleft.set_y (edgept->pos.y ());
    else if (edgept->pos.y () > topright.y ()) {
      realstart = edgept;
      leftmost = edgept->pos.x ();
      topright.set_y (edgept->pos.y ());
    }
    else if (edgept->pos.y () == topright.y ()
    && edgept->pos.x () < leftmost) {
                                 //leftmost on line
      leftmost = edgept->pos.x ();
      realstart = edgept;
    }
    length++;                    //count elements
  }
  while (edgept != start);
  start = realstart;             //shift it to topleft
  return length;
}
