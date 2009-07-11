///////////////////////////////////////////////////////////////////////
// File:        imagefind.h
// Description: Class to find image and drawing regions in an image
//              and create a corresponding list of empty blobs.
// Author:      Ray Smith
// Created:     Fri Aug 01 10:50:01 PDT 2008
//
// (C) Copyright 2008, Google Inc.
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
///////////////////////////////////////////////////////////////////////

#ifndef TESSERACT_TEXTORD_IMAGEFIND_H__
#define TESSERACT_TEXTORD_IMAGEFIND_H__

struct Boxa;
struct Pix;
struct Pixa;

namespace tesseract {

// The ImageFinder class is a simple static function wrapper class that
// exposes the FindImages function and some useful helper functions.
class ImageFinder {
 public:
  // Finds image regions within the source pix (page image) and returns
  // the image regions as a Boxa, Pixa pair, analgous to pixConnComp.
  // The returned boxa, pixa may be NULL, meaning no images found.
  // If not NULL, they must be destroyed by the caller.
  static void FindImages(Pix* pix, Boxa** boxa, Pixa** pixa);

  // Returns true if there is a rectangle in the source pix, such that all
  // pixel rows and column slices outside of it have less than
  // min_fraction of the pixels black, and within max_skew_gradient fraction
  // of the pixels on the inside, there are at least max_fraction of the
  // pixels black. In other words, the inside of the rectangle looks roughly
  // rectangular, and the outside of it looks like extra bits.
  // On return, the rectangle is defined by x_start, y_start, x_end and y_end.
  // Note: the algorithm is iterative, allowing it to slice off pixels from
  // one edge, allowing it to then slice off more pixels from another edge.
  static bool pixNearlyRectangular(Pix* pix,
                                   double min_fraction, double max_fraction,
                                   double max_skew_gradient,
                                   int* x_start, int* y_start,
                                   int* x_end, int* y_end);

  // Given an input pix, and a bounding rectangle, the sides of the rectangle
  // are shrunk inwards until they bound any black pixels found within the
  // original rectangle.
  static void BoundsWithinRect(Pix* pix, int* x_start, int* y_start,
                               int* x_end, int* y_end);
};

}  // namespace tesseract.

#endif  // TESSERACT_TEXTORD_LINEFIND_H__

