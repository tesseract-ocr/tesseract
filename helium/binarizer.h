// Copyright 2006 Google Inc.
// All Rights Reserved.
// Author: <renn@google.com> (Marius Renn)
//
// This file contains the abstract base class Binarizer, which is used to
// separate detected objects (such as text) from the background. The idea is
// to extract mask by mask, sending each one through perspective correction 
// and OCR, before extracting the next. This way, we must keep only one mask
// in memory at all times.
// For text detection in Helium, a Binarizer is passed to the 
// RecognizeUsingBinarizer(...) method of the TextRecognition class, which
// automates the process of extracting masks, apply perspective correction
// and sending it to OCR.
//
#ifndef HELIUM_BINARIZER_H__
#define HELIUM_BINARIZER_H__

namespace helium {

class Box;
class Image;
class Mask;

class Binarizer {
  public:
    // Create a new Binarizer that will extract masks from the given Image,
    Binarizer(const Image& image) : image_(image) {
    }

    // Destructor.
    virtual ~Binarizer() {
    }

    // Extract the next mask from the image. out_bounds will contain the
    // bounds of the area in the image, where the mask was extracted from. 
    // Returns true, if a mask was found.
    virtual bool GetNextMask(Mask& out_mask, Box& out_bounds) = 0;
  
  protected:
    const Image& image_;
};

} // namespace

#endif  // HELIUM_BINARIZER_H__
