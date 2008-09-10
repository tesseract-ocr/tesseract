// Copyright 2006 Google Inc.
// All Rights Reserved.
// Author: <renn@google.com> (Marius Renn)
//
// This file contains the Leptonica class, used to convert Helium Map objects
// to Pix pointers and vice versa.
// This file uses Google3 formatted includes!
//
#ifndef HELIUM_LEPTONICA_H__
#define HELIUM_LEPTONICA_H__

// Local includes
#include "image.h"

// Leptonica include
#include "third_party/leptonica/include/allheaders.h"

namespace helium {

class Image;

// This class is used for converting images from the Helium format to the
// Leptonica Pix format, and vice versa. Note, that these methods are
// expensive, and should only be used when necessary (i.e. for debugging,
// or file input).
class Leptonica {
  public:
    // Converts the given Pix pointer to an image. The Pix pointer must be
    // a 32 bit color image.
    static Image PixToImage(const Pix* pix);

    // Converts an Image to a Pix pointer.
    static Pix* ImageToPix(const Image& image);

    // Converts a 8-bit pix to a GrayMap.
    static GrayMap PixToGrayMap(const Pix* pix);

    // Converts a GrayMap to a 8-bit Pix image.
    static Pix* GrayMapToPix(const GrayMap& graymap);

    // Converts a Pix pointer to a Mask, where every value that is equal to 0,
    // is converted to true, otherwise false.
    static Mask* PixToMask(const Pix* pix);

    // Converts a Mask to a Pix pointer.
    static Pix* MaskToPix(const Mask& mask);

    // Displays given image on screen
    static void DisplayImage(const Image& image);

    static void DisplayGrayMap(const GrayMap& graymap);

    static void DisplayMask(const Mask& mask);

    // Saves given image to file
    static void SaveImage(const Image& image, const char* filename);
};

} // namespace

#endif  // HELIUM_LEPTONICA_H__
