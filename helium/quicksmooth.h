// Copyright 2006 Google Inc.
// All Rights Reserved.
// Author: <renn@google.com> (Marius Renn)
//
// This file conatins the QuickSmooth class, which allows efficient smoothing
// of a given image. It is used for noise reduction, which is crucial to 
// obtaining usable results from the tracer. Although a gaussian smooth 
// produces better results and is more configurable, it is considered to 
// expensive for most uses of Helium.
//
#ifndef HELIUM_QUICKSMOOTH_H__
#define HELIUM_QUICKSMOOTH_H__

// Local includes
#include "helium_image.h"

namespace helium {

class Image;

// The QuickSmooth class provides one static method, Smooth(Image&), which 
// smoothes the given image in-place.
class QuickSmooth {
  public:
    // Smoothes the given image by using a convolution kernel of:
    //
    //  1/9  1/9  1/9
    //  1/9  1/9  1/9
    //  1/9  1/9  1/9
    //
    // This is done almost in-place. Algorithm details are explained in 
    // quicksmooth.cc!
    static void Smooth(Image& image);
    
  private:
    // Divides all color components in the image by 9.
    static void DividePixels(Image& image);
    
    // Sums all pixels in the 8-neighborhood plus the center pixel. 
    static void SumPixels(Image& image);
    
    // Internal methods to calculate the sum of colors
    static inline Color Sum2(const Color& a, const Color& b);
    static inline Color Sum3(const Color& a, const Color& b, const Color& c);
    static inline Color Sum4(const Color& a,
                             const Color& b,
                             const Color& c,
                             const Color& d);
};

} // namespace

#endif  // HELIUM_QUICKSMOOTH_H__
