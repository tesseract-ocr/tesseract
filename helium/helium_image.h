// Copyright 2006 Google Inc.
// All Rights Reserved.
// Author: <renn@google.com> (Marius Renn)
//
// This file contains the Image class used to hold all color image data in
// Helium.
//
#ifndef HELIUM_IMAGE_H__
#define HELIUM_IMAGE_H__

// Local includes
#include "array.h"
#include "color.h"
#include "graymap.h"
#include "mask.h"

namespace helium {

class Box;

// The Image class is actually a Map of Colors with added functionality. It
// is possible to access the color channels separately to simplify algorithms
// that handle each of them individually.
// For debugging purposes Image has a number of methods for visualization:
// The other Map types (GrayMap and Mask) can be converted to an Image for
// output to a file or display. To visualize clusters or other bounding boxes,
// two methods, DrawBox(...) and DrawBoxes(...), are provided.
// To read or write image files to disk, use on of the Image IO interfaces
// such as Leptonica (leptonica.h).
class Image : public Map<Color> {
  public:
    // Constructor to create an empty image with no data.
    Image();

    // Constructor to allocate an image of the given width and height.
    Image(unsigned width, unsigned height);

    // Constructor to wrap existing image data in an Image object. Make sure
    // the image data is in a compatible format (Color array).
    Image(unsigned width, unsigned height, uint32* data);

    // Convenience method to receive access to the red channel at the
    // specified coordinates. Returns a uint8 pointer to that channel.
    // Note that the data is interleaved, so the next red value has an offset
    // of 4.
    inline uint8* AccessRed(int x, int y) const {
      return reinterpret_cast<uint8*>(Access(x, y)) + kRedOffset / 8;
    }

    // Convenience method to receive access to the green channel at the
    // specified coordinates. Returns a uint8 pointer to that channel.
    // Note that the data is interleaved, so the next green value has an offset
    // of 4.
    inline uint8* AccessGreen(int x, int y) const {
      return reinterpret_cast<uint8*>(Access(x, y)) + kGreenOffset / 8;
    }

    // Convenience method to receive access to the blue channel at the
    // specified coordinates. Returns a uint8 pointer to that channel.
    // Note that the data is interleaved, so the next blue value has an offset
    // of 4.
    inline uint8* AccessBlue(int x, int y) const {
      return reinterpret_cast<uint8*>(Access(x, y)) + kBlueOffset / 8;
    }

    // Convenience method to receive access to the alpha channel at the
    // specified coordinates. Returns a uint8 pointer to that channel.
    // Note that the data is interleaved, so the next alpha value has an offset
    // of 4.
    inline uint8* AccessAlpha(int x, int y) const {
      return reinterpret_cast<uint8*>(Access(x, y)) + kAlphaOffset / 8;
    }

    // Conversion methods ------------------------------------------------------
    // The following conversion methods are expensive operations, so they
    // should not be used, other than to report or debug (which is their
    // intended purpose)!

    // Create an image from a GrayMap.
    static Image FromGrayMap(const GrayMap& map);

    // Converts an image to a GrayMap.
    static GrayMap ToGrayMap(const Image& image);

    // Create an image from a Mask.
    static Image FromMask(const Mask& mask);

    // Create an image from the alpha channel of a given image, where the
    // values represent specific labels. This method treats the label values
    // as the hue component of the output to produce a colorful image, that
    // makes distinguishing the different labels easy.
    static Image FromLabeledImage(const Image& labels);

    // Drawing on an Image -----------------------------------------------------
    // Draw a box of the specified color on the receiver. For debugging
    // and reporting purposes only!
    void DrawBox(const Box& box, Color color);

    // Draw a set of boxes of the specified color on the receiver. For
    // debugging and reporting purposes only!
    void DrawBoxes(const Array<Box>& boxes, Color color, float scale);
};

} // namespace

#endif // HELIUM_IMAGE_H__
