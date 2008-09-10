// Copyright 2006 Google Inc.
// All Rights Reserved.
// Author: <renn@google.com> (Marius Renn)
//

// Local includes
#include "debugging.h"
#include "laplaceedgedetector.h"

using namespace helium;

const int8 kLaplaceMask[25] = { 
  -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1,
  -1, -1, 24, -1, -1,
  -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1 
};

LaplaceEdgeDetector::LaplaceEdgeDetector() 
  : EdgeDetector(5) {
}

// Runs Laplace edge detection on the given image using the kernel:
//
//  -1  -1  -1  -1  -1
//  -1  -1  -1  -1  -1
//  -1  -1  24  -1  -1
//  -1  -1  -1  -1  -1
//  -1  -1  -1  -1  -1
//
// This method uses the ApplyKernel(...) function of the EdgeDetector 
// superclass to obtain the output values. It does not reuse old values in
// the current implementation.
GrayMap LaplaceEdgeDetector::DetectEdges(const Image& image) {
  // Create destination map
  GrayMap dest(image.width(), image.height());
  ASSERT(dest.Valid());
  
  int r, g, b;
  uint8* dest_ptr = dest.Access(kernel_half_, kernel_half_);
  Color* image_ptr = image.Access(kernel_half_, kernel_half_);
  int mask_begin = image.width() + kernel_half_;
  int mask_vstep = image.width() - kernel_size_;
  
  for (unsigned y = kernel_half_; y < image.height() - kernel_half_; y++) {
    for (unsigned x = kernel_half_; x < image.width() - kernel_half_; x++) {
      r = g = b = 0;
      
      ApplyKernel(kLaplaceMask, image_ptr - mask_begin, mask_vstep, r, g, b);
    
      int magnitude = (r + g + b) / 3;
      
      // Normalize and limit range of magnitude
      magnitude += 1024;
      if (magnitude < 0) 
        magnitude = 0;
      else if (magnitude > 2047)
        magnitude = 2047;
      magnitude /= 8;
      
      *(dest_ptr++) = static_cast<uint8>(magnitude);
      image_ptr++;
    }
    dest_ptr += kernel_half_ * 2;
    image_ptr += kernel_half_ * 2;
  }
  return dest;
}
