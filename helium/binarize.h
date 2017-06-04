// Copyright 2006 Google Inc.
// All Rights Reserved.
// Author: <renn@google.com> (Marius Renn)
//
// This file contains the Binarize class, which is a collection of static
// methods, used for binarizing GrayMaps.
//
#ifndef HELIUM_BINARIZE_H__
#define HELIUM_BINARIZE_H__

#include "graymap.h"
#include "mask.h"

namespace helium {

// The Binarize class provides a number of static methods to binarize a given
// GrayMap.
class Binarize {
  public:
    // Returns a Mask, that specifies for each pixel, whether the value in the
    // input GrayMap exceeded the specified threshold at that pixel.
    static Mask Threshold(const GrayMap& map, uint8 threshold);
    
    // Returns a Mask, that specifies for each pixel, whether the value in the
    // input GrayMap is a local maxima (in any direction) at that pixel.
    static Mask Maximum(const GrayMap& map, uint8 threshold);
    
    // Returns a Mask, that specifies for each pixel, whether the value in the
    // input GrayMap crosses the zero point (in any direction) at that pixel.
    // This is used for binarizing the output of the Laplace edge detector.
    static Mask ZeroCrossings(const GrayMap& map, uint8 min_slope);
};

} // namespace

#endif  // HELIUM_BINARIZE_H__
