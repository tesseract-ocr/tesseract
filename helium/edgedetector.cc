// Copyright 2006 Google Inc.
// All Rights Reserved.
// Author: <renn@google.com> (Marius Renn)

// Local includes
#include "edgedetector.h"

using namespace helium;
                              
// We try to do as much initializing in the base constructor as possible, to
// avoid redundancy in the subclasses
EdgeDetector::EdgeDetector(unsigned mask_size) 
  : kernel_size_(mask_size),
    kernel_half_((mask_size - 1) / 2) {
}

void EdgeDetector::ApplyKernel(const int8* mask, 
                               const Color* image_ptr,
                               int image_width,
                               int& red, 
                               int& green, 
                               int& blue) {
  unsigned i = 0;
  for(unsigned y = 0; y < kernel_size_; y++) {
    for(unsigned x = 0; x < kernel_size_; x++) {
      red += Red(*image_ptr) * mask[i];
      green += Green(*image_ptr) * mask[i];
      blue += Blue(*(image_ptr++)) * mask[i++];
    }
    image_ptr += (image_width - kernel_size_);
  }
}
