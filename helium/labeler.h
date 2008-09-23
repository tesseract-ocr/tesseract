// Copyright 2006 Google Inc.
// All Rights Reserved.
// Author: <renn@google.com> (Marius Renn)
//
// This file contains the Labeler class, used to label regions defined by a
// given mask. The Labeler will find all regions that are separated by mask
// pixels (high values), and assign them a 32 bit id.
// Note, that currently, Labeling is not used, due to the memory and speed
// requirements. It is however an interesting alternative to tracing.
//
#ifndef HELIUM_LABELER_H__
#define HELIUM_LABELER_H__

#include "helium_image.h"

namespace helium {

// The Labeler class provides a single method to label Mask data. The output
// is returned in a 32 bit image.
class Labeler {
  public:
    // Labels the given mask, and returns an image, that contains the 32 bit
    // label id's for each discovered region.
    static Image Label(const Mask& mask);
    
  private:
    static void LabelAllRegions(Image& labels, const Mask& edges);
};

} // namespace

#endif
