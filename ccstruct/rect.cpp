/**********************************************************************
 * File:        rect.c  (Formerly box.c)
 * Description: Bounding box class definition.
 * Author:					Phil Cheatle
 * Created:					Wed Oct 16 15:18:45 BST 1991
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

#include          "mfcpch.h"     // precompiled headers
#include          "rect.h"

// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif

/**********************************************************************
 * TBOX::TBOX()  Constructor from 2 ICOORDS
 *
 **********************************************************************/

TBOX::TBOX(                   //construtor
         const ICOORD pt1,  //one corner
         const ICOORD pt2   //the other corner
        ) {
  if (pt1.x () <= pt2.x ()) {
    if (pt1.y () <= pt2.y ()) {
      bot_left = pt1;
      top_right = pt2;
    }
    else {
      bot_left = ICOORD (pt1.x (), pt2.y ());
      top_right = ICOORD (pt2.x (), pt1.y ());
    }
  }
  else {
    if (pt1.y () <= pt2.y ()) {
      bot_left = ICOORD (pt2.x (), pt1.y ());
      top_right = ICOORD (pt1.x (), pt2.y ());
    }
    else {
      bot_left = pt2;
      top_right = pt1;
    }
  }
}

/**********************************************************************
 * TBOX::TBOX()  Constructor from 4 integer values.
 *  Note: It is caller's responsibility to provide values in the right
 *        order.
 **********************************************************************/

TBOX::TBOX(                    //constructor
    inT16 left, inT16 bottom, inT16 right, inT16 top)
    : bot_left(left, bottom), top_right(right, top) {
}

// rotate_large constructs the containing bounding box of all 4
// corners after rotating them. It therefore guarantees that all
// original content is contained within, but also slightly enlarges the box.
void TBOX::rotate_large(const FCOORD& vec) {
  ICOORD top_left(bot_left.x(), top_right.y());
  ICOORD bottom_right(top_right.x(), bot_left.y());
  top_left.rotate(vec);
  bottom_right.rotate(vec);
  rotate(vec);
  TBOX box2(top_left, bottom_right);
  *this += box2;
}

/**********************************************************************
 * TBOX::intersection()  Build the largest box contained in both boxes
 *
 **********************************************************************/

TBOX TBOX::intersection(  //shared area box
                      const TBOX &box) const {
  inT16 left;
  inT16 bottom;
  inT16 right;
  inT16 top;
  if (overlap (box)) {
    if (box.bot_left.x () > bot_left.x ())
      left = box.bot_left.x ();
    else
      left = bot_left.x ();

    if (box.top_right.x () < top_right.x ())
      right = box.top_right.x ();
    else
      right = top_right.x ();

    if (box.bot_left.y () > bot_left.y ())
      bottom = box.bot_left.y ();
    else
      bottom = bot_left.y ();

    if (box.top_right.y () < top_right.y ())
      top = box.top_right.y ();
    else
      top = top_right.y ();
  }
  else {
    left = MAX_INT16;
    bottom = MAX_INT16;
    top = -MAX_INT16;
    right = -MAX_INT16;
  }
  return TBOX (left, bottom, right, top);
}


/**********************************************************************
 * TBOX::bounding_union()  Build the smallest box containing both boxes
 *
 **********************************************************************/

TBOX TBOX::bounding_union(  //box enclosing both
                        const TBOX &box) const {
  ICOORD bl;                     //bottom left
  ICOORD tr;                     //top right

  if (box.bot_left.x () < bot_left.x ())
    bl.set_x (box.bot_left.x ());
  else
    bl.set_x (bot_left.x ());

  if (box.top_right.x () > top_right.x ())
    tr.set_x (box.top_right.x ());
  else
    tr.set_x (top_right.x ());

  if (box.bot_left.y () < bot_left.y ())
    bl.set_y (box.bot_left.y ());
  else
    bl.set_y (bot_left.y ());

  if (box.top_right.y () > top_right.y ())
    tr.set_y (box.top_right.y ());
  else
    tr.set_y (top_right.y ());
  return TBOX (bl, tr);
}


/**********************************************************************
 * TBOX::plot()  Paint a box using specified settings
 *
 **********************************************************************/

#ifndef GRAPHICS_DISABLED
void TBOX::plot(                      //paint box
               ScrollView* fd,       //where to paint
               ScrollView::Color fill_colour,   //colour for inside
               ScrollView::Color border_colour  //colour for border
              ) const {
  fd->Brush(fill_colour);
  fd->Pen(border_colour);
  plot(fd);
}
#endif

// Writes to the given file. Returns false in case of error.
bool TBOX::Serialize(FILE* fp) const {
  if (!bot_left.Serialize(fp)) return false;
  if (!top_right.Serialize(fp)) return false;
  return true;
}
// Reads from the given file. Returns false in case of error.
// If swap is true, assumes a big/little-endian swap is needed.
bool TBOX::DeSerialize(bool swap, FILE* fp) {
  if (!bot_left.DeSerialize(swap, fp)) return false;
  if (!top_right.DeSerialize(swap, fp)) return false;
  return true;
}

/**********************************************************************
 * operator+=
 *
 * Extend one box to include the other  (In place union)
 **********************************************************************/

DLLSYM TBOX &
operator+= (                     //bounding bounding bx
TBOX & op1,                       //operands
const TBOX & op2) {
  if (op2.bot_left.x () < op1.bot_left.x ())
    op1.bot_left.set_x (op2.bot_left.x ());

  if (op2.top_right.x () > op1.top_right.x ())
    op1.top_right.set_x (op2.top_right.x ());

  if (op2.bot_left.y () < op1.bot_left.y ())
    op1.bot_left.set_y (op2.bot_left.y ());

  if (op2.top_right.y () > op1.top_right.y ())
    op1.top_right.set_y (op2.top_right.y ());

  return op1;
}


/**********************************************************************
 * operator&=
 *
 * Reduce one box to intersection with the other  (In place intersection)
 **********************************************************************/

TBOX& operator&=(TBOX& op1, const TBOX& op2) {
  if (op1.overlap (op2)) {
    if (op2.bot_left.x () > op1.bot_left.x ())
      op1.bot_left.set_x (op2.bot_left.x ());

    if (op2.top_right.x () < op1.top_right.x ())
      op1.top_right.set_x (op2.top_right.x ());

    if (op2.bot_left.y () > op1.bot_left.y ())
      op1.bot_left.set_y (op2.bot_left.y ());

    if (op2.top_right.y () < op1.top_right.y ())
      op1.top_right.set_y (op2.top_right.y ());
  }
  else {
    op1.bot_left.set_x (MAX_INT16);
    op1.bot_left.set_y (MAX_INT16);
    op1.top_right.set_x (-MAX_INT16);
    op1.top_right.set_y (-MAX_INT16);
  }
  return op1;
}

bool TBOX::x_almost_equal(const TBOX &box, int tolerance) const {
  return (abs(left() - box.left()) <= tolerance &&
           abs(right() - box.right()) <= tolerance);
}

bool TBOX::almost_equal(const TBOX &box, int tolerance) const {
  return (abs(left() - box.left()) <= tolerance &&
          abs(right() - box.right()) <= tolerance &&
          abs(top() - box.top()) <= tolerance &&
          abs(bottom() - box.bottom()) <= tolerance);
}
