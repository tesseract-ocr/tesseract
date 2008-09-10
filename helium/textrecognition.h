// Copyright 2006 Google Inc.
// All Rights Reserved.
// Author: <renn@google.com> (Marius Renn)
//
// This file contains the TextRecognition class, which uses a given Binarizer
// to extract, dewarp and OCR text found in images. This is called after
// the text detection phase.
//
#ifndef HELIUM_TEXTRECOGNITION_H__
#define HELIUM_TEXTRECOGNITION_H__

namespace helium {

class Binarizer;

// The TextRecognition class simplifies the process of binarizing, dewarping 
// and sending the text masks to OCR. This is done by calling the
// RecognizeUsingBinarizer(...) method.
class TextRecognition {
  public:
    // Init the TextRecognition system with the data found at the specified
    // path. This should be the path of the directory, that contains the
    // tessdata directory. 
    // This must be called before using RecognizeUsingBinarizer(...), but it
    // can be callled multiple times to clear OCR's internal adaptation.
    static void Init(const char* data_path);
    
    // This method passes all the binarized masks, that were extracted by the
    // specified Binarizer, through perspective correction, and
    // on to the OCR engine. The bounding boxes and recognized text are added
    // to the given TextAreas object.
    static void RecognizeUsingBinarizer(Binarizer* binarizer,
                                        TextAreas& text_areas);
  
  private:
    // Static flag to make sure Init(...) was called at least once, before
    // text recognition.
    static bool recognizer_initialized_;
};

} // namespace

#endif  // HELIUM_TEXTRECOGNITION_H__
