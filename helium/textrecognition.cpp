// Copyright 2006 Google Inc.
// All Rights Reserved.
// Author: <renn@google.com> (Marius Renn)

#include <stdlib.h>
#include "binarizer.h"
#include "box.h"
#include "debugging.h"
#include "mask.h"
#include "perspectivedetection.h"
#include "tesseract.h"
#include "textareas.h"
#include "textrecognition.h"

using namespace helium;

bool TextRecognition::recognizer_initialized_ = false;

void TextRecognition::Init(const char* data_path,
                           const char *lang,
                           const char *configfile) {
  // Allow reinitialization of Tesseract to get around its adaptation.
  Tesseract::Init(data_path, lang, configfile);
  recognizer_initialized_ = true;
}

void TextRecognition::RecognizeUsingBinarizer(Binarizer* binarizer,
                                              TextAreas& text_areas) {
  ASSERT(recognizer_initialized_);
  Mask text_mask;
  Surface surface;
  Box text_mask_bounds;
  while (binarizer->GetNextMask(text_mask, text_mask_bounds)) {
    // Detect perspective
    if (PerspectiveDetection::DetectSurface(text_mask, surface)) {
      // Dewarp mask
      Mask dewarped_mask = surface.Dewarp(text_mask);
      // OCR dewarped mask
      char* text = Tesseract::RecognizeText(dewarped_mask);
      text_areas.AddArea(text_mask_bounds, text);
      // The memory for the OCR text string is created by strdup in
      // Tesseract::RecognizeText().
      free(text);
    }
  }
}
