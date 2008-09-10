// Copyright 2006 Google Inc.
// All Rights Reserved.
// Author: <renn@google.com> (Marius Renn)
//
// This file conatins the GaussianSmooth class, which allows quality smoothing
// of a given image at the price of complexity.
//
#ifndef HELIUM_GAUSSIANSMOOTH_H__
#define HELIUM_GAUSSIANSMOOTH_H__

// Local includes
#include "image.h"

namespace helium {

// The GaussianSmooth class allows gaussian smoothing with a kernel of any
// desired size. Although this produces high quality results, the algorithm
// is fairly complex, and to make matters worse, requires an additional two
// image buffers, which each are the size of the input image.
class GaussianSmooth {
  public:
    // Constructor to create a GaussianSmooth with a kernel of the given size.
    GaussianSmooth(unsigned size);
    
    // Destructor to deallocate kernel.
    ~GaussianSmooth();

    // Smooth the given image, using a standard gaussian blur algorithm.
    // The smoothed image is returned. 
    Image Smooth(const Image& image);
  
  private:
    // Do not allow copying
    GaussianSmooth(const GaussianSmooth&);
    void operator=(const GaussianSmooth&);
    
    int kernel_size_;   // The size of the kernel
    int gauss_sum_;     // The sum of the kernel values (for normalizing)
    int* kernel_;       // The kernel itself
};

} // namespace

#endif  // HELIUM_GAUSSIANSMOOTH_H__
