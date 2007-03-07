///////////////////////////////////////////////////////////////////////
// File:        baseapi.h
// Description: Simple API for calling tesseract.
// Author:      Ray Smith
// Created:     Fri Oct 06 15:35:01 PDT 2006
//
// (C) Copyright 2006, Google Inc.
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

#ifndef THIRD_PARTY_TESSERACT_CCMAIN_BASEAPI_H__
#define THIRD_PARTY_TESSERACT_CCMAIN_BASEAPI_H__

#include <string>

#include "host.h"
#include "ocrclass.h"

class PAGE_RES;
class BLOCK_LIST;

// Base class for all tesseract APIs.
// Specific classes can add ability to work on different inputs or produce
// different outputs.

class TessBaseAPI {
 public:
  // Start tesseract.
  // The datapath must be the name of the data directory or some other file
  // in which the data directory resides (for instance argv[0].)
  // The configfile is the name of a file in the tessconfigs directory
  // (eg batch) or NULL to run on defaults.
  // Outputbase may also be NULL, and is the basename of various output files.
  // If the output of any of these files is enabled, then a name must be given.
  // If numeric_mode is true, only possible digits and roman numbers are
  // returned. Returns 0 if successful. Crashes if not.
  // The argc and argv may be 0 and NULL respectively. They are used for
  // providing config files for debug/display purposes.
  // TODO(rays) get the facts straight. Is it OK to call
  // it more than once? Make it properly check for errors and return them.
  static int Init(const char* datapath, const char* outputbase,
                  const char* configfile, bool numeric_mode,
                  int argc, char* argv[]);

  // Recognize a rectangle from an image and return the result as a string.
  // May be called many times for a single Init.
  // Currently has no error checking.
  // Greyscale of 8 and color of 24 or 32 bits per pixel may be given.
  // Palette color images will not work properly and must be converted to
  // 24 bit.
  // Binary images of 1 bit per pixel may also be given but they must be
  // byte packed with the MSB of the first byte being the first pixel, and a
  // 1 represents WHITE. For binary images set bytes_per_pixel=0.
  // The recognized text is returned as a char* which (in future will be coded
  // as UTF8 and) must be freed with the delete [] operator.
  static char* TesseractRect(const UINT8* imagedata,
                             int bytes_per_pixel,
                             int bytes_per_line,
                             int left, int top, int width, int height);

  // Call between pages or documents etc to free up memory and forget
  // adaptive data.
  static void ClearAdaptiveClassifier();

  // Close down tesseract and free up memory.
  static void End();

  // Dump the internal binary image to a PGM file.
  static void DumpPGM(const char* filename);

 protected:
  // Copy the given image rectangle to Tesseract, with adaptive thresholding
  // if the image is not already binary.
  static void CopyImageToTesseract(const UINT8* imagedata,
                                   int bytes_per_pixel,
                                   int bytes_per_line,
                                   int left, int top, int width, int height);

  // Compute the Otsu threshold(s) for the given image rectangle, making one
  // for each channel. Each channel is always one byte per pixel.
  // Returns an array of threshold values and an array of hi_values, such
  // that a pixel value >threshold[channel] is considered foreground if
  // hi_values[channel] is 0 or background if 1. A hi_value of -1 indicates
  // that there is no apparent foreground. At least one hi_value will not be -1.
  // thresholds and hi_values are assumed to be of bytes_per_pixel size.
  static void OtsuThreshold(const UINT8* imagedata,
                           int bytes_per_pixel,
                           int bytes_per_line,
                           int left, int top, int right, int bottom,
                           int* thresholds,
                           int* hi_values);

  // Compute the histogram for the given image rectangle, and the given
  // channel. (Channel pointed to by imagedata.) Each channel is always
  // one byte per pixel.
  // Bytes per pixel is used to skip channels not being
  // counted with this call in a multi-channel (pixel-major) image.
  // Histogram is always a 256 element array to count occurrences of
  // each pixel value.
  static void HistogramRect(const UINT8* imagedata,
                            int bytes_per_pixel,
                            int bytes_per_line,
                            int left, int top, int right, int bottom,
                            int* histogram);

  // Compute the Otsu threshold(s) for the given histogram.
  // Also returns H = total count in histogram, and
  // omega0 = count of histogram below threshold.
  static int OtsuStats(const int* histogram,
                       int* H_out,
                       int* omega0_out);

  // Threshold the given grey or color image into the tesseract global
  // image ready for recognition. Requires thresholds and hi_value
  // produced by OtsuThreshold above.
  static void ThresholdRect(const UINT8* imagedata,
                            int bytes_per_pixel,
                            int bytes_per_line,
                            int left, int top,
                            int width, int height,
                            const int* thresholds,
                            const int* hi_values);

  // Cut out the requested rectangle of the binary image to the
  // tesseract global image ready for recognition.
  static void CopyBinaryRect(const UINT8* imagedata,
                             int bytes_per_line,
                             int left, int top,
                             int width, int height);

  // Low-level function to recognize the current global image to a string.
  static char* RecognizeToString();

  // Find lines from the image making the BLOCK_LIST.
  static void FindLines(BLOCK_LIST* block_list);

  // Recognize the tesseract global image and return the result as Tesseract
  // internal structures.
  static PAGE_RES* Recognize(BLOCK_LIST* block_list, ETEXT_DESC* monitor);

  // Convert (and free) the internal data structures into a text string.
  static char* TesseractToText(PAGE_RES* page_res);
};

#endif  // THIRD_PARTY_TESSERACT_CCMAIN_BASEAPI_H__
