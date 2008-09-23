// Copyright 2006 Google Inc.
// All Rights Reserved.
// Author: <renn@google.com> (Marius Renn)
//
// This file conatins the abstract base class EdgeDetection, which provides
// not only a common interface to edge detectors, but also convenience methods
// and initializing for its subclasses.
//
#ifndef HELIUM_EDGEDETECTOR_H__
#define HELIUM_EDGEDETECTOR_H__

// Local includes
#include "color.h"
#include "graymap.h"
#include "helium_image.h"

namespace helium {

// Abstract base class EdgeDetector, which provides a common interface to 
// edge detectors and contains common methods and member variables for its
// subclasses.
class EdgeDetector {
  public:
    // Destrctor.
    virtual ~EdgeDetector() {}

    // Pure abstract method to run edge detection on a given image. Returns
    // a GrayMap in which the values represent the strengths of the edges.
    virtual GrayMap DetectEdges(const Image& image) = 0;
    
  protected:
    // Protected constructor for the convenience of subclasses. 
    EdgeDetector(unsigned kernel_size);

    // Convenience method for subclasses. Applies the given kernel at the
    // position of the given Color pointer. Returns the result of applying
    // the kernel to each color channel in the arguments red, green and blue.
    void ApplyKernel(const int8* mask, 
                     const Color* image_ptr, 
                     int image_width,
                     int& red, 
                     int& green, 
                     int& blue);

    // The size of the kernel
    const unsigned kernel_size_;
    
    // The size of the kernel divided by two. Since this value is needed so
    // often, it is precomputed.
    const unsigned kernel_half_;
};

} // namespace

#endif  // HELIUM_EDGEDETECTOR_H__
