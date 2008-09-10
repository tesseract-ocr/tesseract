// Copyright 2006 Google Inc.
// All Rights Reserved.
// Author: <renn@google.com> (Marius Renn)
//
// This file contains the Surface class which is used to specify the 
// perspective distortion of a certain object (such as text) on an image.
// It provides means to project a mask, that lies on such a surface onto a 
// flat rectangle, removing any perspective distortion. 
// The Surface of text in Helium is calculated with the PerspectiveDetection
// class. See the corresponding header files for details on this.
//
#ifndef HELIUM_SURFACE_H__
#define HELIUM_SURFACE_H__

#include "point.h"

namespace helium {

class Mask;

// The Surface class contains four member variables, one for each point:
// top-left, top-right, bottom-left, bottom-right. The Dewarp(const Mask&)
// method takes a given input mask, assumes that the high-values lie on its
// surface, and projects the mask onto a flat surface.
class Surface {
  public:
    // Initialize surface with all points at (0, 0).
    Surface();
    
    // Initialize surface with the given four points.
    Surface(Point top_left, 
            Point top_right, 
            Point bottom_left, 
            Point bottom_right);
    
    // Set the top left point.
    inline void set_top_left(Point top_left) {
      top_left_ = top_left;
    }
    
    // Set the top right point
    inline void set_top_right(Point top_right) {
      top_right_ = top_right;
    }
    
    // Set the bottom left point
    inline void set_bottom_left(Point bottom_left) {
      bottom_left_ = bottom_left;
    }
    
    // Set the bottom right point
    inline void set_bottom_right(Point bottom_right) {
      bottom_right_ = bottom_right;
    }
    
    // Dewarp the given mask, that is assumed to be projected on the receiver's
    // four points, and return the dewarped mask.
    Mask Dewarp(const Mask& mask);
    
  private:
    // This method takes the four Surface points and calculates the width
    // and height of the dewarped rectangle.
    void CalculateDestinationRect(unsigned& width, unsigned& height);
    
    // Dewarps the given mask to a rectangle of the given width and height.
    Mask DewarpMaskTo(const Mask& mask, unsigned width, unsigned height);
    
    // Calculates the coefficients used for the projection.
    // The projective transform is given by the following equations:
    //
    //  x' = (ax + by + c) / (gx + hy + 1)
    //  y' = (dx + ey + f) / (gx + hy + 1)
    //
    // This method solves the linear equation system (8 unknowns, 2 equations
    // for each of the four points) and outputs the 8 coefficients.
    //
    // Thanks to Dan Bloomberg for the help on this!
    bool CalcProjectionCoefficients(unsigned width, 
                                    unsigned height,
                                    float* coeffs);
    Point top_left_;
    Point top_right_;
    Point bottom_left_;
    Point bottom_right_;
};

} // namespace

#endif  // HELIUM_SURFACE_H__
