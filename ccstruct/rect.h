/**********************************************************************
 * File:        rect.h  (Formerly box.h)
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

#ifndef           RECT_H
#define           RECT_H

#include <math.h>
#include "points.h"
#include "ndminx.h"
#include "scrollview.h"
#include "strngs.h"
#include "tprintf.h"

class DLLSYM TBOX  {  // bounding box
  public:
    TBOX ():       // empty constructor making a null box
    bot_left (MAX_INT16, MAX_INT16), top_right (-MAX_INT16, -MAX_INT16) {
    }

    TBOX(          // constructor
        const ICOORD pt1,   // one corner
        const ICOORD pt2);  // the other corner

    TBOX(                    // constructor
        inT16 left, inT16 bottom, inT16 right, inT16 top);

    TBOX(  // box around FCOORD
        const FCOORD pt);

    bool null_box() const {  // Is box null
      return ((left () >= right ()) || (top () <= bottom ()));
    }

    bool operator==(const TBOX& other) const {
      return bot_left == other.bot_left && top_right == other.top_right;
    }

    inT16 top() const {  // coord of top
      return top_right.y ();
    }
    void set_top(int y) {
      top_right.set_y(y);
    }

    inT16 bottom() const {  // coord of bottom
      return bot_left.y ();
    }
    void set_bottom(int y) {
      bot_left.set_y(y);
    }

    inT16 left() const {  // coord of left
      return bot_left.x ();
    }
    void set_left(int x) {
      bot_left.set_x(x);
    }

    inT16 right() const {  // coord of right
      return top_right.x ();
    }
    void set_right(int x) {
      top_right.set_x(x);
    }

    const ICOORD &botleft() const {  // access function
      return bot_left;
    }

    ICOORD botright() const {  // ~ access function
      return ICOORD (top_right.x (), bot_left.y ());
    }

    ICOORD topleft() const {  // ~ access function
      return ICOORD (bot_left.x (), top_right.y ());
    }

    const ICOORD &topright() const {  // access function
      return top_right;
    }

    inT16 height() const {  // how high is it?
      if (!null_box ())
        return top_right.y () - bot_left.y ();
      else
        return 0;
    }

    inT16 width() const {  // how high is it?
      if (!null_box ())
        return top_right.x () - bot_left.x ();
      else
        return 0;
    }

    inT32 area() const {  // what is the area?
      if (!null_box ())
        return width () * height ();
      else
        return 0;
    }

    // Pads the box on either side by the supplied x,y pad amounts.
    // NO checks for exceeding any bounds like 0 or an image size.
    void pad(int xpad, int ypad) {
      ICOORD pad(xpad, ypad);
      bot_left -= pad;
      top_right += pad;
    }

    void move_bottom_edge(                  // move one edge
                          const inT16 y) {  // by +/- y
      bot_left += ICOORD (0, y);
    }

    void move_left_edge(                  // move one edge
                        const inT16 x) {  // by +/- x
      bot_left += ICOORD (x, 0);
    }

    void move_right_edge(                  // move one edge
                         const inT16 x) {  // by +/- x
      top_right += ICOORD (x, 0);
    }

    void move_top_edge(                  // move one edge
                       const inT16 y) {  // by +/- y
      top_right += ICOORD (0, y);
    }

    void move(                     // move box
              const ICOORD vec) {  // by vector
      bot_left += vec;
      top_right += vec;
    }

    void move(                     // move box
              const FCOORD vec) {  // by float vector
      bot_left.set_x ((inT16) floor (bot_left.x () + vec.x ()));
      // round left
      bot_left.set_y ((inT16) floor (bot_left.y () + vec.y ()));
      // round down
      top_right.set_x ((inT16) ceil (top_right.x () + vec.x ()));
      // round right
      top_right.set_y ((inT16) ceil (top_right.y () + vec.y ()));
      // round up
    }

    void scale(                  // scale box
               const float f) {  // by multiplier
      bot_left.set_x ((inT16) floor (bot_left.x () * f));  // round left
      bot_left.set_y ((inT16) floor (bot_left.y () * f));  // round down
      top_right.set_x ((inT16) ceil (top_right.x () * f));  // round right
      top_right.set_y ((inT16) ceil (top_right.y () * f));  // round up
    }
    void scale(                     // scale box
               const FCOORD vec) {  // by float vector
      bot_left.set_x ((inT16) floor (bot_left.x () * vec.x ()));
      bot_left.set_y ((inT16) floor (bot_left.y () * vec.y ()));
      top_right.set_x ((inT16) ceil (top_right.x () * vec.x ()));
      top_right.set_y ((inT16) ceil (top_right.y () * vec.y ()));
    }

    // rotate doesn't enlarge the box - it just rotates the bottom-left
    // and top-right corners. Use rotate_large if you want to guarantee
    // that all content is contained within the rotated box.
    void rotate(const FCOORD& vec) {  // by vector
      bot_left.rotate (vec);
      top_right.rotate (vec);
      *this = TBOX (bot_left, top_right);
    }
    // rotate_large constructs the containing bounding box of all 4
    // corners after rotating them. It therefore guarantees that all
    // original content is contained within, but also slightly enlarges the box.
    void rotate_large(const FCOORD& vec);

    bool contains(  // is pt inside box
                   const FCOORD pt) const;

    bool contains(  // is box inside box
                   const TBOX &box) const;

    bool overlap(  // do boxes overlap
                  const TBOX &box) const;

    bool major_overlap(  // do boxes overlap more than half
                        const TBOX &box) const;

    // Do boxes overlap on x axis.
    bool x_overlap(const TBOX &box) const;

    // Return the horizontal gap between the boxes. If the boxes
    // overlap horizontally then the return value is negative, indicating
    // the amount of the overlap.
    int x_gap(const TBOX& box) const {
      return MAX(bot_left.x(), box.bot_left.x()) -
             MIN(top_right.x(), box.top_right.x());
    }

    // Return the vertical gap between the boxes. If the boxes
    // overlap vertically then the return value is negative, indicating
    // the amount of the overlap.
    int y_gap(const TBOX& box) const {
      return MAX(bot_left.y(), box.bot_left.y()) -
             MIN(top_right.y(), box.top_right.y());
    }

    // Do boxes overlap on x axis by more than
    // half of the width of the narrower box.
    bool major_x_overlap(const TBOX &box) const;

    // Do boxes overlap on y axis.
    bool y_overlap(const TBOX &box) const;

    // Do boxes overlap on y axis by more than
    // half of the height of the shorter box.
    bool major_y_overlap(const TBOX &box) const;

    // fraction of current box's area covered by other
    double overlap_fraction(const TBOX &box) const;

    // fraction of the current box's projected area covered by the other's
    double x_overlap_fraction(const TBOX& box) const;

    // fraction of the current box's projected area covered by the other's
    double y_overlap_fraction(const TBOX& box) const;

    // Returns true if the boxes are almost equal on x axis.
    bool x_almost_equal(const TBOX &box, int tolerance) const;

    // Returns true if the boxes are almost equal
    bool almost_equal(const TBOX &box, int tolerance) const;

    TBOX intersection(  // shared area box
                     const TBOX &box) const;

    TBOX bounding_union(  // box enclosing both
                       const TBOX &box) const;

    // Sets the box boundaries to the given coordinates.
    void set_to_given_coords(int x_min, int y_min, int x_max, int y_max) {
      bot_left.set_x(x_min);
      bot_left.set_y(y_min);
      top_right.set_x(x_max);
      top_right.set_y(y_max);
    }

    void print() const {  // print
      tprintf("Bounding box=(%d,%d)->(%d,%d)\n",
              left(), bottom(), right(), top());
    }
    // Appends the bounding box as (%d,%d)->(%d,%d) to a STRING.
    void print_to_str(STRING *str) const;

#ifndef GRAPHICS_DISABLED
    void plot(                    // use current settings
              ScrollView* fd) const {  // where to paint
      fd->Rectangle(bot_left.x (), bot_left.y (), top_right.x (),
        top_right.y ());
    }

    void plot(                              // paint box
              ScrollView* fd,                    // where to paint
              ScrollView::Color fill_colour,           // colour for inside
              ScrollView::Color border_colour) const;  // colour for border
#endif
    // Writes to the given file. Returns false in case of error.
    bool Serialize(FILE* fp) const;
    // Reads from the given file. Returns false in case of error.
    // If swap is true, assumes a big/little-endian swap is needed.
    bool DeSerialize(bool swap, FILE* fp);

    friend TBOX& operator+=(TBOX&, const TBOX&);
    // in place union
    friend TBOX& operator&=(TBOX&, const TBOX&);
    // in place intersection

  private:
    ICOORD bot_left;             // bottom left corner
    ICOORD top_right;            // top right corner
};

/**********************************************************************
 * TBOX::TBOX()  Constructor from 1 FCOORD
 *
 **********************************************************************/

inline TBOX::TBOX(               // construtor
                const FCOORD pt  // floating centre
               ) {
  bot_left = ICOORD ((inT16) floor (pt.x ()), (inT16) floor (pt.y ()));
  top_right = ICOORD ((inT16) ceil (pt.x ()), (inT16) ceil (pt.y ()));
}


/**********************************************************************
 * TBOX::contains()  Is point within box
 *
 **********************************************************************/

inline bool TBOX::contains(const FCOORD pt) const {
  return ((pt.x () >= bot_left.x ()) &&
    (pt.x () <= top_right.x ()) &&
    (pt.y () >= bot_left.y ()) && (pt.y () <= top_right.y ()));
}


/**********************************************************************
 * TBOX::contains()  Is box within box
 *
 **********************************************************************/

inline bool TBOX::contains(const TBOX &box) const {
  return (contains (box.bot_left) && contains (box.top_right));
}


/**********************************************************************
 * TBOX::overlap()  Do two boxes overlap?
 *
 **********************************************************************/

inline bool TBOX::overlap(  // do boxes overlap
                          const TBOX &box) const {
  return ((box.bot_left.x () <= top_right.x ()) &&
    (box.top_right.x () >= bot_left.x ()) &&
    (box.bot_left.y () <= top_right.y ()) &&
    (box.top_right.y () >= bot_left.y ()));
}

/**********************************************************************
 * TBOX::major_overlap()  Do two boxes overlap by at least half of the smallest?
 *
 **********************************************************************/

inline bool TBOX::major_overlap(  // Do boxes overlap more that half.
                                const TBOX &box) const {
  int overlap = MIN(box.top_right.x(), top_right.x());
  overlap -= MAX(box.bot_left.x(), bot_left.x());
  overlap += overlap;
  if (overlap < MIN(box.width(), width()))
    return false;
  overlap = MIN(box.top_right.y(), top_right.y());
  overlap -= MAX(box.bot_left.y(), bot_left.y());
  overlap += overlap;
  if (overlap < MIN(box.height(), height()))
    return false;
  return true;
}

/**********************************************************************
 * TBOX::overlap_fraction()  Fraction of area covered by the other box
 *
 **********************************************************************/

inline double TBOX::overlap_fraction(const TBOX &box) const {
  double fraction = 0.0;
  if (this->area()) {
    fraction = this->intersection(box).area() * 1.0 / this->area();
  }
  return fraction;
}

/**********************************************************************
 * TBOX::x_overlap()  Do two boxes overlap on x-axis
 *
 **********************************************************************/

inline bool TBOX::x_overlap(const TBOX &box) const {
  return ((box.bot_left.x() <= top_right.x()) &&
    (box.top_right.x() >= bot_left.x()));
}

/**********************************************************************
 * TBOX::major_x_overlap()  Do two boxes overlap by more than half the
 *                          width of the narrower box on the x-axis
 *
 **********************************************************************/

inline bool TBOX::major_x_overlap(const TBOX &box) const {
  inT16 overlap = box.width();
  if (this->left() > box.left()) {
    overlap -= this->left() - box.left();
  }
  if (this->right() < box.right()) {
    overlap -= box.right() - this->right();
  }
  return (overlap >= box.width() / 2 || overlap >= this->width() / 2);
}

/**********************************************************************
 * TBOX::y_overlap()  Do two boxes overlap on y-axis
 *
 **********************************************************************/

inline bool TBOX::y_overlap(const TBOX &box) const {
  return ((box.bot_left.y() <= top_right.y()) &&
    (box.top_right.y() >= bot_left.y()));
}

/**********************************************************************
 * TBOX::major_y_overlap()  Do two boxes overlap by more than half the
 *                          height of the shorter box on the y-axis
 *
 **********************************************************************/

inline bool TBOX::major_y_overlap(const TBOX &box) const {
  inT16 overlap = box.height();
  if (this->bottom() > box.bottom()) {
    overlap -= this->bottom() - box.bottom();
  }
  if (this->top() < box.top()) {
    overlap -= box.top() - this->top();
  }
  return (overlap >= box.height() / 2 || overlap >= this->height() / 2);
}

/**********************************************************************
 * TBOX::x_overlap_fraction() Calculates the horizontal overlap of the
 *                            given boxes as a fraction of this boxes
 *                            width.
 *
 **********************************************************************/

inline double TBOX::x_overlap_fraction(const TBOX& other) const {
  int low = MAX(left(), other.left());
  int high = MIN(right(), other.right());
  int width = right() - left();
  if (width == 0) {
    int x = left();
    if (other.left() <= x && x <= other.right())
      return 1.0;
    else
      return 0.0;
  } else {
    return MAX(0, static_cast<double>(high - low) / width);
  }
}

/**********************************************************************
 * TBOX::y_overlap_fraction() Calculates the vertical overlap of the
 *                            given boxes as a fraction of this boxes
 *                            height.
 *
 **********************************************************************/

inline double TBOX::y_overlap_fraction(const TBOX& other) const {
  int low = MAX(bottom(), other.bottom());
  int high = MIN(top(), other.top());
  int height = top() - bottom();
  if (height == 0) {
    int y = bottom();
    if (other.bottom() <= y && y <= other.top())
      return 1.0;
    else
      return 0.0;
  } else {
    return MAX(0, static_cast<double>(high - low) / height);
  }
}

#endif
