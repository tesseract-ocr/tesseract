// Copyright 2006 Google Inc.
// All Rights Reserved.
// Author: <renn@google.com> (Marius Renn)
//

// Local includes
#include "debugging.h"
#include "sobeledgedetector.h"

// C includes
#include <math.h>

using namespace helium;

SobelEdgeDetector::SobelEdgeDetector() 
  : EdgeDetector(3) {
}

// Runs Sobel edge detection on the given image using two kernels:
//
//  Vertical Edge         Horizontal Edge
//  1   2   1             1   0  -1
//  0   0   0             2   0  -2
// -1  -2  -1             1   0  -1
//
// This implementation does not actually make use of the superclass's 
// ApplyKernel(...) method, as we would like to reuse as many old values as
// possible. This means that for any given pixel, that is not on an
// edge of the image, the following situation for the vertical kernel holds:
//
//  r1   r2   r3
//  s1   s2   s3
//  y1   y2   y3,
//
// where the weighted sums (r1 + 2*r2 + r3) and (s1 + 2*s2 + s3) have been 
// calculated already. Only y1, y2, and y3 need to be fetched from memory.
//
// Likewise for the horizontal kernel, the situation is as follows:
//
//  c1   d1   x1
//  c2   d2   x2
//  c3   d3   x3,
//
// where all values, except x1, x2 and x3 have been calculated already. This
// means that per pixel we need to fetch six values from memory (opposed to 
// 12 or even 18 in a non-optimized version). 
//
// Once we have calculated the required horizontal and vertical sums, all we
// need to do is subtract the bottom sum from the top, the right sum from the
// left, and combine the results to obtain the edge value.
//
// Special cases apply for the edges, which are not discussed here, as they
// can be easily deduced from the standard case.
GrayMap SobelEdgeDetector::DetectEdges(const Image& image) {
  // Create destination map
  GrayMap dest(image.width(), image.height());
  ASSERT(dest.Valid());
  
  bool left = false;
  bool top = false;
  uint8* dest_ptr = dest.Access(kernel_half_, kernel_half_);
  Color* image_ptr = image.Access(kernel_half_, kernel_half_);
  Color top_row = 0;
  Color left_col = 0;
  Color bottom_row = 0;
  Color right_col = 0;
  
  const unsigned row = image.width();
  
  Color rows[2][row];
  Color cols[2];
  
  // First pixel
  top_row = Sum3(image_ptr - row - 1,
                 image_ptr - row,
                 image_ptr - row + 1);
  rows[0][0] = Sum3(image_ptr - 1,
                    image_ptr,
                    image_ptr + 1);
  rows[1][0] = Sum3(image_ptr + row - 1,
                    image_ptr + row,
                    image_ptr + row + 1);
  left_col = Sum3(image_ptr - row - 1,
                  image_ptr - 1,
                  image_ptr + row - 1); 
  cols[0] = Sum3(image_ptr - row,
                 image_ptr,
                 image_ptr + row);
  cols[1] = Sum3(image_ptr - row + 1,
                 image_ptr + 1,
                 image_ptr + row + 1);
  
  *dest_ptr = EdgeValue(left_col, cols[1], top_row, rows[1][0]);
  ++image_ptr;
  ++dest_ptr;
  
  // First row
  for (unsigned x = 1; x < image.width() - 2; ++x) {
    // Add new row and column
    top_row = Sum3(image_ptr - row - 1,
                   image_ptr - row,
                   image_ptr - row + 1);
    rows[0][x] = Sum3(image_ptr - 1,
                      image_ptr,
                      image_ptr + 1);
    rows[1][x] = Sum3(image_ptr + row - 1,
                      image_ptr + row,
                      image_ptr + row + 1);
    right_col = Sum3(image_ptr - row + 1,
                     image_ptr + 1,
                     image_ptr + row + 1);
    // Calculate edge value
    *dest_ptr = EdgeValue(cols[left], right_col, top_row, rows[1][x]);
    
    cols[left] = right_col;
    left = !left;
    ++image_ptr;
    ++dest_ptr;
  }
  image_ptr += 2;
  dest_ptr += 2;
  
  for (unsigned y = 2; y < image.height() - 1; ++y) {
    // First pixel of row
    rows[!top][0] = Sum3(image_ptr + row - 1,
                         image_ptr + row,
                         image_ptr + row + 1);
    left_col = Sum3(image_ptr - row - 1,
                    image_ptr - 1,
                    image_ptr + row - 1); 
    cols[0] = Sum3(image_ptr - row,
                   image_ptr,
                   image_ptr + row);
    cols[1] = Sum3(image_ptr - row + 1,
                   image_ptr + 1,
                   image_ptr + row + 1); 
                 
    *dest_ptr = EdgeValue(left_col, cols[1], rows[top][0], rows[!top][0]);
                 
    ++image_ptr;
    ++dest_ptr;
    rows[top][0] = top_row;
    left = false;
    
    for(unsigned x = 1; x < image.width() - 2; ++x) {
      // Add new row and column
      bottom_row = Sum3(image_ptr + row - 1,
                        image_ptr + row,
                        image_ptr + row + 1);
      right_col = Sum3(image_ptr - row + 1,
                       image_ptr + 1,
                       image_ptr + row + 1);
      
      // Calculate edge value
      *dest_ptr = EdgeValue(cols[left], right_col, rows[top][x], bottom_row);
      
      rows[top][x] = bottom_row;
      cols[left] = right_col;
      
      // Move on
      left = !left;
      ++image_ptr;
      ++dest_ptr;
    }
    image_ptr += 2;
    dest_ptr += 2;
    top = !top;
  }
  return dest;
}

// Calculates the weighted sum (a + 2*b + c), which is either one row or column
// of the kernel.
Color SobelEdgeDetector::Sum3(const Color* a, const Color* b, const Color* c) {
  int sum_red = Red(*a) + 2 * Red(*b) + Red(*c);
  int sum_green = Green(*a) + 2 * Green(*b) + Green(*c);
  int sum_blue = Blue(*a) + 2 * Blue(*b) + Blue(*c);
  return MakeColor(sum_red / 4, sum_green / 4, sum_blue / 4);
}

// Calculates the edge value given the weighted column sums x1, x2 and the 
// weighted row sums y1, y2. The value is calculated as follows:
// |x1 - x2| + |y1 - y2|
// The color channels are then summed up and scaled down to produce the 
// output value. 
uint8 SobelEdgeDetector::EdgeValue(const Color& x1, 
                               const Color& x2,
                               const Color& y1,
                               const Color& y2) {
  Color diff_x = ColorDifference(x1, x2);
  Color diff_y = ColorDifference(y1, y2);
  
  int value = ((Red(diff_x) + Red(diff_y))
            + (Green(diff_x) + Green(diff_y))
            + (Blue(diff_x) + Blue(diff_y))) / 4;
  
  return (value > 255) ? 255 : static_cast<uint8>(value);
}

