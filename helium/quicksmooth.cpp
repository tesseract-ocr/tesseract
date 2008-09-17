// Copyright 2006 Google Inc.
// All Rights Reserved.
// Author: <renn@google.com> (Marius Renn)
//
#include "color.h"
#include "debugging.h"
#include "image.h"
#include "quicksmooth.h"

using namespace helium;

Color QuickSmooth::Sum2(const Color& a, const Color& b) {
  return MakeColor(Red(a) + Red(b),
                   Green(a) + Green(b),
                   Blue(a) + Blue(b));
}

Color QuickSmooth::Sum3(const Color& a, const Color& b, const Color& c) {
  return MakeColor(Red(a) + Red(b) + Red(c),
                   Green(a) + Green(b) + Green(c),
                   Blue(a) + Blue(b) + Blue(c));
}

Color QuickSmooth::Sum4(const Color& a, 
                        const Color& b, 
                        const Color& c, 
                        const Color& d) {
  return MakeColor(Red(a) + Red(b) + Red(c) + Red(d),
                   Green(a) + Green(b) + Green(c) + Green(d),
                   Blue(a) + Blue(b) + Blue(c) + Blue(d));
}

// Smoothes the given image by using a convolution kernel of:
//
//  1/9  1/9  1/9
//  1/9  1/9  1/9
//  1/9  1/9  1/9
//
// This is actually done  in two steps. First all values are divided by
// 9, then in the second pass, all values in the kernel range are summed
// up. 
// The smoothing is done almost in-place, and old values are reused where
// possible. For any pixel, that is not on an edge of the image, the 
// following situation holds:
//
//  r1   r2    r3
//  c1   d1    x1
//  c2   d2    x2,
//
// where (r1 + r2 + r3), (c1 + c2), (d1 + d2) have already been calculated. 
// Only x1 and x2 need to be fetched from memory. (Actually, 3 values must be 
// fetched from memory: x1, x2, and d1. This is due to the algorithm 
// storing only the sums (r1 + r2 + r3), (c1 + c2) and (d1 + d2), not
// the operands themselves. Since we need to store (r1' + r2' + r3') for 
// the next row, we need to  calculate the sum (c1 + d1 + x1). We avoid 
// having to lookup c1 again by caching the last d1). 
// 
// In terms of memory usage, this means we require an extra 3 values for the
// three column sums (c1 + c2), (d1 + d2), (x1 + x2), and an extra n values
// for the row sums at each horizontal offset, where n is the width of the 
// image.
//
// Special cases apply for the edges, which are not discussed here, as they
// can be easily deduced from the standard case.
void QuickSmooth::SumPixels(Image& image) {
  Color* img_ptr = image.Access(1, 1);
  unsigned row = image.width();
  Color last_value = *img_ptr;
  Color row_value = 0;
  
  Color cur_col[3];
  Color cur_row[image.width()];
  
  // First pixel of image
  cur_col[0] = Sum3(*(img_ptr - row - 1),
                    *(img_ptr - 1),
                    *(img_ptr + row - 1));

  cur_col[1] = Sum3(*(img_ptr - row),
                    *(img_ptr),
                    *(img_ptr + row));
                   
  cur_col[2] = Sum3(*(img_ptr - row + 1),
                    *(img_ptr + 1),
                    *(img_ptr + row + 1));
                   
  cur_row[0] = Sum3(*(img_ptr - 1),
                    *(img_ptr),
                    *(img_ptr + 1));
             
  *img_ptr = Sum3(cur_col[0], cur_col[1], cur_col[2]);
  ++img_ptr;
  
  uint8 c = 0;
  
  // First row
  for (unsigned x = 2; x < image.width() - 1; x++) {
    cur_col[c] = Sum3(*(img_ptr - row + 1),
                          *(img_ptr + 1),
                          *(img_ptr + row + 1));
            
    cur_row[x - 1] = Sum3(last_value,
                          *(img_ptr),
                          *(img_ptr + 1));
                       
    last_value = *img_ptr;
    
    *img_ptr = Sum3(cur_col[0], cur_col[1], cur_col[2]);
    
    ++img_ptr;
    c = (c == 2) ? 0 : c + 1;
  }
  img_ptr += 2;
  
  // Rest of image
  for (unsigned y = 2; y < image.height() - 1; y++) {
    last_value = *img_ptr;
    
    // First pixel in row
    cur_col[0] = Sum2(*(img_ptr - 1),
                      *(img_ptr + row - 1));
    cur_col[1] = Sum2(*(img_ptr),
                      *(img_ptr + row));
    cur_col[2] = Sum2(*(img_ptr + 1),
                      *(img_ptr + row + 1));     
    row_value = Sum3(*(img_ptr - 1), 
                     *(img_ptr),
                     *(img_ptr + 1)); 
              
    *img_ptr = Sum4(cur_col[0], cur_col[1], cur_col[2], cur_row[0]);

    cur_row[0] = row_value;
    ++img_ptr;
    c = 0;
    
    // All other pixels in row
    Color d1, x1, x2;
    for (unsigned x = 2; x < image.width() - 1; x++) {
      d1 = *(img_ptr);
      x1 = *(img_ptr + 1);
      x2 = *(img_ptr + row + 1);
      
      cur_col[c] = Sum2(x1, x2);
      row_value = Sum3(last_value, d1, x2);
      
      last_value = d1;
      
      *img_ptr = Sum4(cur_col[0], cur_col[1], cur_col[2], cur_row[x - 1]);
      
      cur_row[x - 1] = row_value;
      ++img_ptr;
      c = (c == 2) ? 0 : c + 1;
    }
    img_ptr += 2;
  }
}

void QuickSmooth::DividePixels(Image& image) {
  Color pixel;
  for (Color* img_ptr = image.data(); img_ptr < image.DataEnd(); img_ptr++) {
    pixel = *img_ptr;
    *img_ptr = MakeColor(Red(pixel) / 9, Green(pixel) / 9, Blue(pixel) / 9);
  }
}

void QuickSmooth::Smooth(Image& image) {
  DividePixels(image);
  SumPixels(image);
}
