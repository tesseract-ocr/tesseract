// Copyright 2006 Google Inc.
// All Rights Reserved.
// Author: <renn@google.com> (Marius Renn)
//
// This file contains the PerspectiveDetection class, which can detect the
// surface points a given mask lies on. Currently, this has been tested for
// text masks only, so the usefulness for other objects remains to be
// evaluated. 
// The obtained surface is used for dewarping the text, using the Dewarp(...)
// method of the Surface class. See there for details.
//
#ifndef HELIUM_PERSPECTIVEDETECTION_H__
#define HELIUM_PERSPECTIVEDETECTION_H__

#include "surface.h"

namespace helium {

class Mask;

// This class contains one static method, that returns the Surface a given mask
// lies on.
class PerspectiveDetection {
  public:
    // Using the given mask, attempts to find the four points, that specify
    // the surface, that the mask lies on. 
    // The implementation sends the mask to Tesseract, which detects the bottom
    // and top baseline. 
    // If Tesseract returns bogus information (no baseline, or too many), this
    // method returns false.
    // TODO: Detect the left and right bounding lines by analyzing the italics
    // of the characters. Currently, this method assumes straight vertical 
    // lines.
    static bool DetectSurface(const Mask& mask, Surface& surface);
};

} // namespace

#endif  // HELIUM_PERSPECTIVEDETECTION_H__
