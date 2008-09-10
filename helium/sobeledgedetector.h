// Copyright 2006 Google Inc.
// All Rights Reserved.
// Author: <renn@google.com> (Marius Renn)
//
// This file contains the SobelEdgeDetector class to run Sobel edge detection
// on any given color image.
// The detected edges can be used in the tracing phase, where the tracer 
// attempts to follow along these edges to find closed contours.
//
// The result of a Sobel edge detection, which can be though of as the first
// derivative of the input signal, gives the edge strength for any point in the
// image. The values can be thresholded to receive a binary map of which pixels
// are considered edges, and which not. Although Sobel edge detection is very
// robust against noise, the actual edge is difficult to evaluate, as 
// thresholding can produce very think edges. This makes it difficult for a
// tracer to decide on where to go next.
//
#ifndef HELIUM_SOBELEDGEDETECTOR_H__
#define HELIUM_SOBELEDGEDETECTOR_H__

#include "edgedetector.h"

namespace helium {

// The SobelEdgeDetector class runs an optimized Sobel edge detection algorithm
// on a given color image. The result is returned in a GrayMap, which can
// either be thresholded (with one of the methods in binarization.h), or used
// directly, like in the MaxTracer.
class SobelEdgeDetector : public EdgeDetector {
  public:
    SobelEdgeDetector();
    
    // Applies sobel edge detection on the given image, and returns the 
    // strength of the edges in a GrayMap. Algorithm details are explained
    // in sobeledgedetector.cc!
    GrayMap DetectEdges(const Image& image);
  
  private:
    // Return the sum of the three colors found at the specified pointers
    static inline Color Sum3(const Color* a, const Color* b, const Color* c);
    
    // Calculate the output value for the given color sums. See the 
    // implementation file for details.
    static inline uint8 EdgeValue(const Color& x1, 
                                  const Color& x2,
                                  const Color& y1,
                                  const Color& y2);
};

} // namespace

#endif // HELIUM_SOBELEDGEDETECTOR_H__
