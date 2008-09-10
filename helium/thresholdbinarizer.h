// Copyright 2006 Google Inc.
// All Rights Reserved.
// Author: <renn@google.com> (Marius Renn)
//
// This file contains the ThresholdBinarizer file, that provides binarization
// by threshold functionality for a given image area. It is mainly used for
// external text detectors, that do not provide color information, making a
// binarization by color impossible.
//
#ifndef HELIUM_THRESHOLDBINARIZER_H__
#define HELIUM_THRESHOLDBINARIZER_H__

#include "binarizer.h"

namespace helium {

class TextAreas;

// The ThresholdBinarizer class provides Otsu-threshold binarization on a 
// given image area, and was heavily inspired by Ray Smith's code in the
// Tesseract API.
// Normally, the Binarizer is simply passed to the TextRecognition module.
// However, ThresholdBinarizer also provides a number of static methods to
// manually calculate the threshold and to do binarization on any image.
class ThresholdBinarizer : public Binarizer {
  public:
    // Construct a ThresholdBinarizer that will binarize the given areas on
    // the given Image.
    ThresholdBinarizer(const Image& image, const TextAreas& text_areas);
    
    // Binarizes the next area and outputs the binary mask, and the bounds of
    // the mask in the Image. Returns false, if there was no further text area
    // to binarize.
    bool GetNextMask(Mask& out_mask, Box& out_bounds);
  
    // Static method to binarize a given Box in a given image. This method 
    // first calculates the threshold, using ThresholdValue(...), and then
    // binarizes the requested area with Binarize(...).
    static void BinarizeRect(const Image& image,
                             Mask& mask, 
                             const Box& bounds);
    
    // Calculates the Otsu threshold value for the specified area in the 
    // specified image. 
    static int ThresholdValue(const Image& image, const Box& bounds);
    
    // Binarizes the specified area in the given image, using the specified
    // threshold. Pixels that exceed the threshold will be set to hi_value.
    static void Binarize(const Image& image, 
                         Mask& mask,
                         const Box& bounds,
                         int threshold,
                         bool hi_value);
                         
  private:
    const TextAreas& text_areas_;
    unsigned index_;
};

} // namespace

#endif  // HELIUM_THRESHOLDBINARIZER_H__
