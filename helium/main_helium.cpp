// Copyright 2006 Google Inc.
// All Rights Reserved.
// Author: <renn@google.com> (Marius Renn)
//
// This is the main file for the core front-end to Helium. It is rather
// simple, and does not provide any debugging information.
// The detected text is output to a file called "text.dat".
//
// Please note, that for image loading, Leptonica is required!
//
// Local includes
#include "color.h"
#include "debugging.h"
#include "helium_image.h"
#include "heliumbinarizer.h"
#include "heliumtextdetector.h"
#include "leptonica.h"
#include "textareas.h"
#include "textrecognition.h"

// C includes
#include <stdlib.h>

using namespace helium;

void ExitWithError(const char* message) {
  ERROR(message);
  exit(-1);
}

int main(int argc, char** argv) {
  // Usage check
  if (argc != 2) ExitWithError("Usage: text_detect <input_file>");
  
  // Init Tesseract
  TextRecognition::Init("../");
  
  // Read input image
  LOG_MSG("Reading file...");
  Pix* input = pixRead(argv[1]);
  if (!input) ExitWithError("Could not open image file!");
  Image image = Leptonica::PixToImage(input);
  if (!image.Valid()) ExitWithError("Error while loading image file!");
  
  // Run text detector
  LOG_MSG("Running Text Detector...");
  HeliumTextDetector detector;
  detector.SetDefaultParameters();
  detector.DetectText(image);
  
  // Setup binarizer
  LOG_MSG("Setting up Binarizer...");
  HeliumBinarizer binarizer(image);
  binarizer.AddClusters(detector.GetClusters());
  
  // Run OCR
  LOG_MSG("OCRing...");
  TextAreas text;
  TextRecognition::RecognizeUsingBinarizer(&binarizer, text);

  // Output Text
  text.WriteDatFile("text.dat");
  LOG_MSG("Done.");
  
  return 0;
};
