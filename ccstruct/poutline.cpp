/**********************************************************************
 * File:        poutline.cpp  (Formerly outline.c)
 * Description: Code for OUTLINE class.
 * Author:		Ray Smith
 * Created:		Wed Oct 23 10:52:04 BST 1991
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
#include          "poutline.h"

// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif

ELISTIZE_S (OUTLINE)
/**********************************************************************
 * OUTLINE::OUTLINE
 *
 * Constructor to build a OUTLINE from a compact LOOP.
 **********************************************************************/
OUTLINE::OUTLINE (               //constructor
const ICOORD & startpt,          //start position
inT8 * compactloop,              //from Tess format
BOOL8 invert,                    //reverse it
ICOORD bot_left,                 //bounding box
ICOORD top_right):
box (bot_left, top_right),
start(startpt) {
  ICOORD pos;                    //current point
  ICOORD vec;                    //vector to next
  POLYPT *polypt;                //new point
  inT8 *vector;                  //compact loop
  POLYPT_IT it = &outline;       //iterator

  pos = startpt;
  vector = compactloop;
  do {
                                 //vector to next
    vec = ICOORD (*vector, *(vector + 1));
                                 //make a new one
    polypt = new POLYPT (FCOORD (pos), FCOORD (vec));
                                 //add to list
    it.add_after_then_move (polypt);
    pos += vec;                  //move to next
    vector += 2;
  }
  while (pos != startpt);
  if (invert)
    reverse();  //now reverse it
}


/**********************************************************************
 * OUTLINE::OUTLINE
 *
 * Constructor to build an OUTLINE from a list of POLYPTs.
 **********************************************************************/

OUTLINE::OUTLINE(                    //constructor
                 POLYPT_IT *polypts  //input list
                ) {
  POLYPT_IT other_it = *polypts; //end of list

  polypts->move_to_first ();
  other_it.move_to_last ();
                                 //put in outline
  outline.assign_to_sublist (polypts, &other_it);
  compute_bb();
}


/**********************************************************************
 * OUTLINE::compute_bb
 *
 * Compute the bounding box from the outline points.
 **********************************************************************/

void OUTLINE::compute_bb() {  //constructor
  ICOORD ibl, itr;               //integer bb
  FCOORD botleft;                //bounding box
  FCOORD topright;
  FCOORD pos;                    //current pos;
  POLYPT_IT polypts = &outline;  //iterator

  botleft = polypts.data ()->pos;
  topright = botleft;
  start = ICOORD ((inT16) botleft.x (), (inT16) botleft.y ());
  do {
    pos = polypts.data ()->pos;
    if (pos.x () < botleft.x ())
                                 //get bounding box
      botleft = FCOORD (pos.x (), botleft.y ());
    if (pos.y () < botleft.y ())
      botleft = FCOORD (botleft.x (), pos.y ());
    if (pos.x () > topright.x ())
      topright = FCOORD (pos.x (), topright.y ());
    if (pos.y () > topright.y ())
      topright = FCOORD (topright.x (), pos.y ());
    polypts.forward ();
  }
  while (!polypts.at_first ());
  ibl = ICOORD ((inT16) botleft.x (), (inT16) botleft.y ());
  itr = ICOORD ((inT16) topright.x () + 1, (inT16) topright.y () + 1);
  box = TBOX (ibl, itr);
}


/**********************************************************************
 * OUTLINE::area
 *
 * Compute the area from the outline points.
 **********************************************************************/

float OUTLINE::area() {  //constructor
  FCOORD origin;                 //startpt
  FCOORD prev_vec;               //previous value of vec
  FCOORD vec;                    //from start to current
  float total;                   //total area
  POLYPT_IT poly_it = polypts ();//iterator
                                 //child outline itertr
  OUTLINE_IT child_it(&children);

  origin = poly_it.data ()->pos;
  poly_it.forward ();
  vec = poly_it.data ()->pos - origin;
  poly_it.forward ();
  total = 0.0f;
  while (!poly_it.at_first ()) {
    prev_vec = vec;
    vec = poly_it.data ()->pos - origin;
    total += prev_vec * vec;
    poly_it.forward ();
  }
  total /= 2;
  for (child_it.mark_cycle_pt (); !child_it.cycled_list ();
  child_it.forward ()) {
                                 //add ares of childrein
    total += child_it.data ()->area ();
  }
  return total;
}


/**********************************************************************
 * OUTLINE::operator<
 *
 * Return TRUE if the left operand is inside the right one.
 **********************************************************************/

BOOL8
OUTLINE::operator< (             //winding number
OUTLINE & other                  //other outline
) {
  inT16 count;                   //winding count
  POLYPT_IT it = &outline;       //iterator

  if (!box.overlap (other.box))
    return FALSE;                //can't be contained

  do {
    count = other.winding_number (FCOORD (it.data ()->pos));
    //get winding number
    if (count != INTERSECTING)
      return count != 0;
    it.forward ();
  }
  while (!it.at_first ());

                                 //switch lists
  it.set_to_list (&other.outline);
  do {
                                 //try other way round
    count = winding_number (FCOORD (it.data ()->pos));
    if (count != INTERSECTING)
      return count == 0;
    it.forward ();
  }
  while (!it.at_first ());
  return TRUE;
}


/**********************************************************************
 * OUTLINE::winding_number
 *
 * Return the winding number of the outline around the given point.
 **********************************************************************/

inT16 OUTLINE::winding_number(                     //winding number
                              const FCOORD &point  //point to wind around
                             ) {
  inT16 count;                   //winding count
  POLYPT *polypt;                //current point
  FCOORD vec;                    //to current point
  float cross;                   //cross product
  POLYPT_IT it = &outline;       //iterator

  count = 0;
  do {
    polypt = it.data ();
    vec = polypt->pos - point;
                                 //crossing the line
    if (vec.y () <= 0 && vec.y () + polypt->vec.y () > 0) {
      cross = vec * polypt->vec; //cross product
      if (cross > 0)
        count++;                 //crossing right half
      else if (cross == 0)
        return INTERSECTING;     //going through point
    }
    else if (vec.y () > 0 && vec.y () + polypt->vec.y () <= 0) {
      cross = vec * polypt->vec;
      if (cross < 0)
        count--;                 //crossing back
      else if (cross == 0)
        return INTERSECTING;     //illegal
    }
    it.forward ();
  }
  while (!it.at_first ());
  return count;                  //winding number
}


/**********************************************************************
 * OUTLINE::reverse
 *
 * Reverse the direction of an outline.
 **********************************************************************/

void OUTLINE::reverse() {  //reverse direction
  POLYPT_LIST back_list;         //reversed list
  POLYPT_IT dest_it = &back_list;//destination
  POLYPT_IT src_it = &outline;   //source list
  POLYPT *polypt;                //current point

  do {
    polypt = src_it.extract ();
                                 //copy in reverse
    dest_it.add_after_then_move (polypt);
    src_it.backward ();
  }
  while (!src_it.empty ());
  dest_it.move_to_first ();
  do {
    polypt = dest_it.data ();
    polypt->vec = dest_it.data_relative (1)->pos - polypt->pos;
    //vector to next
    dest_it.forward ();
  }
  while (!dest_it.at_first ());
  dest_it.backward ();
  src_it.set_to_list (&back_list);
                                 //put it back
  outline.assign_to_sublist (&src_it, &dest_it);
}


/**********************************************************************
 * OUTLINE::move
 *
 * Move OUTLINE by vector
 **********************************************************************/

void OUTLINE::move(                  // reposition OUTLINE
                   const FCOORD vec  // by vector
                  ) {
                                 //child outline itertr
  OUTLINE_IT child_it(&children);
  POLYPT_IT poly_it(&outline);  //outline point itertr

  box.move (vec);

  start.set_x ((inT16) floor (start.x () + vec.x () + 0.5));
  // ?? Why ICOORD?
  start.set_y ((inT16) floor (start.y () + vec.y () + 0.5));
  // ?? Why ICOORD?

  for (poly_it.mark_cycle_pt (); !poly_it.cycled_list (); poly_it.forward ())
    poly_it.data ()->pos += vec;

  for (child_it.mark_cycle_pt (); !child_it.cycled_list ();
    child_it.forward ())
  child_it.data ()->move (vec);  // move child outlines
}


/**********************************************************************
 * OUTLINE::scale
 *
 * Scale OUTLINE by vector
 **********************************************************************/

void OUTLINE::scale(               // scale OUTLINE
                    const float f  // by multiplier
                   ) {
                                 //child outline itertr
  OUTLINE_IT child_it(&children);
  POLYPT_IT poly_it(&outline);  //outline point itertr
  POLYPT *pt;

  box.scale (f);

                                 // ?? Why ICOORD?
  start.set_x ((inT16) floor (start.x () * f + 0.5));
                                 // ?? Why ICOORD?
  start.set_y ((inT16) floor (start.y () * f + 0.5));

  for (poly_it.mark_cycle_pt (); !poly_it.cycled_list (); poly_it.forward ()) {
    pt = poly_it.data ();
    pt->pos *= f;
    pt->vec *= f;
  }

  for (child_it.mark_cycle_pt (); !child_it.cycled_list ();
    child_it.forward ())
  child_it.data ()->scale (f);   //scale child outlines
}


/**********************************************************************
 * OUTLINE::scale
 *
 * Scale OUTLINE by vector
 **********************************************************************/

void OUTLINE::scale(                     // scale OUTLINE
                    const FCOORD vector  //by fcoord
                   ) {
                                 //child outline itertr
  OUTLINE_IT child_it(&children);
  POLYPT_IT poly_it(&outline);  //outline point itertr
  POLYPT *pt;

  box.scale (vector);

  start.set_x ((inT16) floor (start.x () * vector.x () + 0.5));
  // ?? Why ICOORD?
  start.set_y ((inT16) floor (start.y () * vector.y () + 0.5));
  // ?? Why ICOORD?

  for (poly_it.mark_cycle_pt (); !poly_it.cycled_list (); poly_it.forward ()) {
    pt = poly_it.data ();
    pt->pos =
      FCOORD (pt->pos.x () * vector.x (), pt->pos.y () * vector.y ());
    pt->vec =
      FCOORD (pt->vec.x () * vector.x (), pt->vec.y () * vector.y ());
  }

  for (child_it.mark_cycle_pt (); !child_it.cycled_list ();
    child_it.forward ())
                                 //scale child outlines
  child_it.data ()->scale (vector);
}

/**********************************************************************
 * OUTLINE::rotate
 *
 * Rotate OUTLINE by the given vector
 **********************************************************************/

void OUTLINE::rotate(
                     const FCOORD vector  //by fcoord
                    ) {
                                 //child outline itertr
  OUTLINE_IT child_it(&children);
  POLYPT_IT poly_it(&outline);  //outline point itertr
  POLYPT *pt;
  box.rotate(vector);

  start.rotate(vector);

  for (poly_it.mark_cycle_pt (); !poly_it.cycled_list (); poly_it.forward ()) {
    pt = poly_it.data ();
    pt->pos.rotate(vector);
    pt->vec.rotate(vector);
  }

  for (child_it.mark_cycle_pt (); !child_it.cycled_list ();
    child_it.forward ())
                                 //scale child outlines
    child_it.data ()->rotate(vector);
}


/**********************************************************************
 * OUTLINE::plot
 *
 * Draw the outline in the given colour.
 **********************************************************************/

#ifndef GRAPHICS_DISABLED
void OUTLINE::plot(                //draw it
                   ScrollView* window,  //window to draw in
                   ScrollView::Color colour   //colour to draw in
                  ) {
  POLYPT *polypt;                //current point
  POLYPT_IT it = &outline;       //iterator

  window->Pen(colour);
  polypt = it.data ();
  int startx = polypt->pos.x ();
  int starty = polypt->pos.y ();
  do {
    it.forward ();
    polypt = it.data ();
    window->Line(startx,starty,polypt->pos.x (),polypt->pos.y ());
    startx = polypt->pos.x ();
    starty = polypt->pos.y ();
  }
  while (!it.at_first ());
}
#endif


/**********************************************************************
 * OUTLINE::operator=
 *
 * Assignment - deep copy data
 **********************************************************************/

OUTLINE & OUTLINE::operator= (   //assignment
const OUTLINE & source           //from this
) {
  box = source.box;
  start = source.start;
  if (!outline.empty())
    outline.clear();
  outline.deep_copy(&source.outline, &POLYPT::deep_copy);
  if (!children.empty())
    children.clear();
  children.deep_copy(&source.children, &OUTLINE::deep_copy);
  return *this;
}
