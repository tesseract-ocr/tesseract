// Copyright 2006 Google Inc.
// All Rights Reserved.
// Author: <renn@google.com> (Marius Renn)
//
// This file declares the Box class, along with a number of Box related 
// functions, that range from very simple to rather complex. 
// Boxes are used throughout Helium to specify any rectangular area in an
// image. 
//
#ifndef HELIUM_BOX_H__
#define HELIUM_BOX_H__

#include "array.h"
#include "debugging.h"
#include "mathfunctions.h"
#include "point.h"

namespace helium {

// The Box class consists of a Point (origin) and two unsigned integers
// (width and height). A number of simple methods reveal a variety of 
// attributes associated with the Box.
class Box {
  public:
    // Constructor for a Box with zero width and height (this is considered
    // an invalid Box).
    Box() : origin_(0, 0), width_(0), height_(0) {
    }
    
    // Constructor to initialize a Box with the given origin and size.
    Box(int left, int top, unsigned width, unsigned height)
      : origin_(left, top), width_(width), height_(height) {
    }

    // Getters -----------------------------------------------------------------
    inline const Point& origin() const {
      return origin_;
    }
    
    inline Point center() const {
      return Point(origin_.x + width_/2, origin_.y + height_/2);
    }
    
    inline int left() const {
      return origin_.x;
    }

    inline int top() const {
      return origin_.y;
    }

    inline unsigned width() const {
      return width_;
    }

    inline unsigned height() const {
      return height_;
    }

    inline int right() const {
      return origin_.x + width_ - 1;
    }
    
    inline int bottom() const {
      return origin_.y + height_ - 1;
    }
    
    inline Point mid_left() const {
      return Point(origin_.x, origin_.y + height_/2);
    }
    
    inline Point mid_right() const {
      return Point(origin_.x + width_, origin_.y + height_/2);
    }
    
    // Setters -----------------------------------------------------------------
    inline void set_origin(const Point& origin) {
      origin_ = origin;
    }
    
    inline void set_left(int left) {
      origin_.x = left;
    }
    
    inline void set_top(int top) {
      origin_.y = top;
    }
    
    inline void set_width(unsigned width) {
      width_ = width;
    }
    
    inline void set_height(unsigned height) {
      height_ = height;
    }
    
    inline void set_right(int right) {
      ASSERT_IN_DEBUG_MODE(right >= origin_.x);
      width_ = (right - origin_.x + 1);
    }
    
    inline void set_bottom(int bottom) {
      ASSERT_IN_DEBUG_MODE(bottom >= origin_.y);
      height_ = (bottom - origin_.y + 1);
    }
    
    // This method sets the x-origin of the Box without affecting the position
    // of the right bound, thus potentially changing the overall width.
    inline void set_left_resize(int left) {
      width_ += (origin_.x - left);
      origin_.x = left;
    }
    
    // This method sets the y-origin of the Box without affecting the position
    // of the bottom bound, thus potentially changing the overall height.
    inline void set_top_resize(int top) {
      height_ += (origin_.y - top);
      origin_.y = top;
    }
    
    // Basic properties --------------------------------------------------------
    // Returns the area of the Box.
    inline unsigned Area() const {
      return width() * height();
    }
    
    // Returns the perimeter of the Box.
    inline unsigned Perimeter() const {
      return 2 * width() + 2 * height();
    }
    
  private:
    Point origin_;
    unsigned width_, height_;
};

// Box related functions -------------------------------------------------------
// Returns a box that is scaled by the given factor. Note that also the origin
// is scaled!
Box ScaleBox(const Box& box, float scale);

// Returns true, if the inner Box lies within the bounds of the outer Box.
bool Encloses(const Box& outer, const Box& inner);

// Returns true, if the inner Box lies within the bounds of the outer Box, and
// the outer Box is larger than the inner Box.
bool Surrounds(const Box& outer, const Box& inner);

// Returns true, if the given Boxes intersect.
bool Intersect(const Box& a, const Box& b);

// Returns the minimum box that encloses the two specified boxes.
Box MinEnclosingBox(const Box& a, const Box& b);

// Returns the minimum box that encloses the specified boxes.
Box MinEnclosingBox(const Array<Box>& boxes);

// Returns the intersection of the specified boxes.
Box Intersection(const Box& a, const Box& b);

// Returns the intersection of the Boxes in Array boxes_a with the Boxes in
// boxes_b. Note that the result may contain overlapping boxes. This function
// has O(n*m) complexity.
Box Intersection(const Array<Box>& boxes_a, const Array<Box>& boxes_b);

// Returns the area of the given Array of boxes. This function has O(n)
// complexity.
unsigned Area(const Array<Box>& boxes);

// Cut the given box out of another. The non-overlapping resulting boxes, that 
// make up the area minus the cut, will be added to the given result Array. 
void CutBox(const Box& area, const Box& cut, Array<Box>& result);

// Calculates the set of boxes that make up the intersection of the two given
// Box Arrays boxes_a and boxes_b. This function has O(n*m) complexity.
void BoxSetsIntersection(const Array<Box>& boxes_a,
                         const Array<Box>& boxes_b,
                         Array<Box>& result);

// Calculates the set of non-overlapping Boxes, that make up the minimum
// enclosing box of the input minus the input. 
void BoxesInvert(const Array<Box>& boxes, Array<Box>& inversion);


} // namespace

#endif  // HELIUM_BOX_H__
