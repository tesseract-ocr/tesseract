///////////////////////////////////////////////////////////////////////
// File:        otsuthr.h
// Description: Simple Otsu thresholding for binarizing images.
// Author:      Ray Smith
// Created:     Fri Mar 07 12:14:01 PST 2008
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

#ifndef TESSERACT_CCMAIN_OTSUTHR_H__
#define TESSERACT_CCMAIN_OTSUTHR_H__

namespace tesseract {

const int kHistogramSize = 256;  // The size of a histogram of pixel values.

// Compute the Otsu threshold(s) for the given image rectangle, making one
// for each channel. Each channel is always one byte per pixel.
// Returns an array of threshold values and an array of hi_values, such
// that a pixel value >threshold[channel] is considered foreground if
// hi_values[channel] is 0 or background if 1. A hi_value of -1 indicates
// that there is no apparent foreground. At least one hi_value will not be -1.
// Delete thresholds and hi_values with delete [] after use.
void OtsuThreshold(const unsigned char* imagedata,
                   int bytes_per_pixel, int bytes_per_line,
                   int left, int top, int width, int height,
                   int** thresholds, int** hi_values);

// Compute the histogram for the given image rectangle, and the given
// channel. (Channel pointed to by imagedata.) Each channel is always
// one byte per pixel.
// Bytes per pixel is used to skip channels not being
// counted with this call in a multi-channel (pixel-major) image.
// Histogram is always a 256 element array to count occurrences of
// each pixel value.
void HistogramRect(const unsigned char* imagedata,
                   int bytes_per_pixel, int bytes_per_line,
                   int left, int top, int width, int height,
                   int* histogram);

// Compute the Otsu threshold(s) for the given histogram.
// Also returns H = total count in histogram, and
// omega0 = count of histogram below threshold.
int OtsuStats(const int* histogram, int* H_out, int* omega0_out);

}  // namespace tesseract.

#endif  // TESSERACT_CCMAIN_OTSUTHR_H__
