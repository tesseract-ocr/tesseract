// Copyright 2006 Google Inc.
// All Rights Reserved.
// Author: <renn@google.com> (Marius Renn)
//
// Ths file contains the Tesseract class, which provides a static interface
// to the Tesseract OCR engine. It is not only used to OCR text, but also to
// find the baseline for a given text-line, which is used for perspective
// detection.
//
#ifndef HELIUM_TESSERACT_H__
#define HELIUM_TESSERACT_H__

#include "ccmain/baseapi.h"
#include "ccmain/thresholder.h"

namespace helium {

class Mask;

class MaskThresholder : public ::tesseract::ImageThresholder {
 public:
  MaskThresholder(const Mask& mask,
                  bool flipped);

  void ThresholdToIMAGE(IMAGE* image);

 private:
  const Mask& mask_;
  bool flipped_;
};


// The Tesseract class provides a simplified interface to the Tesseract OCR
// engine. It can be used to OCR text (given in a Mask), or to just detect
// the baseline, that runs through it.
class Tesseract {
  public:
    // Call this method exactly once to initialize the Tesseract engine with
    // the data files at the specified path (This should be the path, that 
    // contains the 'tessdata' folder).
    static int Init(const char* datapath,
                    const char *lang = NULL,
                    const char *configfile = NULL);
                
    // Find the baseline, specified by the offset and slope, for the given
    // Mask. If flipped is true, this method will flip the image vertically
    // first, to find the line that goes through the top of the letters.
    static bool DetectBaseline(const Mask& mask,
                              int& out_offset, 
                              float& out_slope,
                              bool flipped);
                            
    // OCRs the given mask, and returns the text that was recognized by 
    // Tesseract.
    static char* RecognizeText(const Mask& mask);
    
    // Call this to deallocate memory, that is used by Tesseract.
    static void End();
  
  private:
    // Transfers a Mask directly to Tesseract. If flipped is true, the mask
    // is transferred from bottom to top, resulting in a vertically flipped
    // representation.
    static void ReadMask(const Mask& mask, bool flipped);

    static ::tesseract::TessBaseAPI api_;
};

} // namespace

#endif  // HELIUM_TESSERACT_H__
