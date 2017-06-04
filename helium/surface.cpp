// Copyright 2006 Google Inc.
// All Rights Reserved.
// Author: <renn@google.com> (Marius Renn)

// local includes
#include "debugging.h"
#include "mask.h"
#include "mathfunctions.h"
#include "surface.h"

// C includes
#include <stdlib.h>

using namespace helium;

// Helper functions ------------------------------------------------------------
inline float XProject(int x, int y, float* c) {
  return (c[0] * x + c[1] * y + c[2]) / (c[6] * x + c[7] * y + 1.0);
}

inline float YProject(int x, int y, float* c) {
  return (c[3] * x + c[4] * y + c[5]) / (c[6] * x + c[7] * y + 1.0);
}

// Surface implementation ------------------------------------------------------
const int kMargin = 8;

Surface::Surface() : top_left_(0, 0),
                     top_right_(0, 0),
                     bottom_left_(0, 0),
                     bottom_right_(0, 0) {
}

Surface::Surface(Point top_left, 
                 Point top_right, 
                 Point bottom_left, 
                 Point bottom_right) : top_left_(top_left),
                                       top_right_(top_right),
                                       bottom_left_(bottom_left),
                                       bottom_right_(bottom_right) {
}

void Surface::CalculateDestinationRect(unsigned& width, unsigned& height) {
  // Calculate middle points (for width / height evaluation)
  Point mid_left = Middle(top_left_, bottom_left_);
  Point mid_right = Middle(top_right_, bottom_right_);
  Point mid_top = Middle(top_left_, top_right_);
  Point mid_bottom = Middle(bottom_left_, bottom_right_);
  
  // Calculate width and height (without perspective)
  float w = Distance(mid_left, mid_right);
  float h = Distance(mid_top, mid_bottom);
  
  // Add perspective factors to width and height
  float dis_left = Distance(top_left_, bottom_left_);
  float dis_right = Distance(top_right_, bottom_right_);
  
  w *= (dis_left > dis_right) 
            ? (dis_left / dis_right)
            : (dis_right / dis_left);
  
  float dis_top = Distance(top_left_, top_right_);
  float dis_bottom = Distance(bottom_left_, bottom_right_);
  
  h *= (dis_top > dis_bottom)
            ? (dis_top / dis_bottom)
            : (dis_bottom / dis_top);
            
  width = static_cast<unsigned>(w);
  height = static_cast<unsigned>(h);
}

Mask Surface::Dewarp(const Mask& mask) {
  unsigned width, height;
  
  // Calculate destination rectangle
  CalculateDestinationRect(width, height);
  
  // Dewarp to it
  return DewarpMaskTo(mask, width, height);
}

Mask Surface::DewarpMaskTo(const Mask& mask, unsigned width, unsigned height) {
  // Heavily inspired by the perspective projection code in Leptonica.
  // Thanks to Dan Bloomberg for the help!
  Mask dest(width + kMargin * 2, height + kMargin * 2);
  dest.Clear();
  
  bool* dest_ptr = dest.Access(kMargin, kMargin);
  
  // Find the coefficients for the inverse projection (dest -> source)
  float coeffs[8];
  if (!CalcProjectionCoefficients(width, height, coeffs)) {
    // If we can't dewarp, then return the input in hopes that it might still
    // get recognized.
    bool* mask_ptr = mask.data();
    for (int y = 0; y < height; y++) {
      for (int x = 0; x < width; x++) *(dest_ptr++) = *(mask_ptr++);
        dest_ptr += kMargin * 2;
    }
    return dest;
  }
  
  int src_x = 0;
  int src_y = 0;
  int max_x = mask.width() - 1;
  int max_y = mask.height() - 1;
  
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      // Calculate source coordinates
      src_x = static_cast<int>(XProject(x, y, coeffs));
      src_y = static_cast<int>(YProject(x, y, coeffs));
      
      // Project onto flat surface
      *dest_ptr = ((src_x >= 0) && (src_y >= 0)
                  && (src_x <= max_x) && (src_y <= max_y))
                ? mask.ValueAt(src_x, src_y)
                : 0;
                
      ++dest_ptr;
    }
    dest_ptr += kMargin * 2;
  }
  return dest;
}

bool Surface::CalcProjectionCoefficients(unsigned width, 
                                         unsigned height,
                                         float* coeffs) {
  // Convert geometrical values to float and give them shorter names
  float x1 = top_left_.x;
  float y1 = top_left_.y;
  float x2 = top_right_.x;
  float y2 = top_right_.y;
  float x3 = bottom_right_.x;
  float y3 = bottom_right_.y;
  float x4 = bottom_left_.x;
  float y4 = bottom_left_.y;
  float w = width;
  float h = height;
  
  // Given is the transformation A*C = B, where B is a vector of the 
  // destination points and the C's are the unknown coefficients. 
  // This is the 8x8 matrix A:
  float transform_matrix[8 * 8] = {
    0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0,     0.0    ,
    0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0,     0.0    ,
    w,   0.0, 1.0, 0.0, 0.0, 0.0, -w * x2, 0.0    ,
    0.0, 0.0, 0.0, w,   0.0, 1.0, -w * y2, 0.0    ,
    w,   h,   1.0, 0.0, 0.0, 0.0, -w * x3, -h * x3,
    0.0, 0.0, 0.0, w,   h,   1.0, -w * y3, -h * y2,
    0.0, h,   1.0, 0.0, 0.0, 0.0, 0.0,     -h * x4,
    0.0, 0.0, 0.0, 0.0, h,   1.0, 0.0,     -h * y4
  };
  
  // These are the destination coordinates B
  float dest[8] = { x1, y1, x2, y2, x3, y3, x4, y4 };
  
  // Run Gauss to solve the linear system
  return Gauss(transform_matrix, dest, coeffs, 8);
}
