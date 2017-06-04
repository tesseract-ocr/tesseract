// Copyright 2006 Google Inc.
// All Rights Reserved.
// Author: <renn@google.com> (Marius Renn)
//
// This file contains the LaplaceEdgeDetector class to run Laplace edge 
// detection on any given color image.
// The detected edges can be used in the tracing phase, where the tracer 
// attempts to follow along these edges to find closed contours.
//
// The result of a Laplace edge detection, which can be thought of as the
// second derivative of the input signal, is in itself not directly usable. 
// However, since the points, where values cross 0, represent local maximums in 
// the edge strength signal, applying a ZeroCrossing binarization produces very 
// thin edges. The downside is that the result is extremely sensitive to noise.
//
#ifndef HELIUM_LAPLACEEDGEDETECTOR_H__
#define HELIUM_LAPLACEEDGEDETECTOR_H__

#include "edgedetector.h"

namespace helium {

// The LaplaceEdgeDetector class runs a standard Laplace edge detection 
// algorithm on a given color image. The result is returned in a GrayMap, which
// can either be binarized using a ZeroCrossing detector (found in 
// binarization.h), or used directly (however, currently no algorithm in Helium 
// uses the direct result of the laplace detector).
class LaplaceEdgeDetector : public EdgeDetector {
  public:
    LaplaceEdgeDetector();
    
    // Applies Laplace edge detection on the given image, and returns the
    // obtained values for each pixel in a GrayMap. Note that these can be
    // regarded as the second derivative of the input signal, so that these
    // values do not represent the strength of the edges!
    // Algorithm details are explained in laplaceedgedetector.cc!
    GrayMap DetectEdges(const Image& image);
};

} // namespace

#endif // HELIUM_LAPLACEEDGEDETECTOR_H__
