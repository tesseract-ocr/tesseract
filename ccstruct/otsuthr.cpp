/**********************************************************************
 * File:        otsuthr.cpp
 * Description: Simple Otsu thresholding for binarizing images.
 * Author:      Ray Smith
 * Created:     Fri Mar 07 12:31:01 PST 2008
 *
 * (C) Copyright 2008, Google Inc.
 ** Licensed under the Apache License, Version 2.0 (the "License");
 ** you may not use this file except in compliance with the License.
 ** You may obtain a copy of the License at
 ** http://www.apache.org/licenses/LICENSE-2.0
 ** Unless required by applicable law or agreed to in writing, software
 ** distributed under the License is distributed on an "AS IS" BASIS,
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 ** See the License for the specific language governing permissions and
 ** limitations under the License.
 *
 **********************************************************************/

#include <string.h>
#include "otsuthr.h"

namespace tesseract {

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
                   int** thresholds, int** hi_values) {
  // Of all channels with no good hi_value, keep the best so we can always
  // produce at least one answer.
  int best_hi_value = 1;
  int best_hi_index = 0;
  bool any_good_hivalue = false;
  double best_hi_dist = 0.0;
  *thresholds = new int[bytes_per_pixel];
  *hi_values = new int[bytes_per_pixel];

  for (int ch = 0; ch < bytes_per_pixel; ++ch) {
    (*thresholds)[ch] = -1;
    (*hi_values)[ch] = -1;
    // Compute the histogram of the image rectangle.
    int histogram[kHistogramSize];
    HistogramRect(imagedata + ch, bytes_per_pixel, bytes_per_line,
                  left, top, width, height, histogram);
    int H;
    int best_omega_0;
    int best_t = OtsuStats(histogram, &H, &best_omega_0);
    if (best_omega_0 == 0 || best_omega_0 == H) {
       // This channel is empty.
       continue;
     }
    // To be a convincing foreground we must have a small fraction of H
    // or to be a convincing background we must have a large fraction of H.
    // In between we assume this channel contains no thresholding information.
    int hi_value = best_omega_0 < H * 0.5;
    (*thresholds)[ch] = best_t;
    if (best_omega_0 > H * 0.75) {
      any_good_hivalue = true;
      (*hi_values)[ch] = 0;
    } else if (best_omega_0 < H * 0.25) {
      any_good_hivalue = true;
      (*hi_values)[ch] = 1;
    } else {
      // In case all channels are like this, keep the best of the bad lot.
      double hi_dist = hi_value ? (H - best_omega_0) : best_omega_0;
      if (hi_dist > best_hi_dist) {
        best_hi_dist = hi_dist;
        best_hi_value = hi_value;
        best_hi_index = ch;
      }
    }
  }
  if (!any_good_hivalue) {
    // Use the best of the ones that were not good enough.
    (*hi_values)[best_hi_index] = best_hi_value;
  }
}

// Compute the histogram for the given image rectangle, and the given
// channel. (Channel pointed to by imagedata.) Each channel is always
// one byte per pixel.
// Bytes per pixel is used to skip channels not being
// counted with this call in a multi-channel (pixel-major) image.
// Histogram is always a kHistogramSize(256) element array to count
// occurrences of each pixel value.
void HistogramRect(const unsigned char* imagedata,
                   int bytes_per_pixel, int bytes_per_line,
                   int left, int top, int width, int height,
                   int* histogram) {
  int bottom = top + height;
  memset(histogram, 0, sizeof(*histogram) * kHistogramSize);
  const unsigned char* pixels = imagedata +
                                top * bytes_per_line +
                                left * bytes_per_pixel;
  for (int y = top; y < bottom; ++y) {
    for (int x = 0; x < width; ++x) {
      ++histogram[pixels[x * bytes_per_pixel]];
    }
    pixels += bytes_per_line;
  }
}

// Compute the Otsu threshold(s) for the given histogram.
// Also returns H = total count in histogram, and
// omega0 = count of histogram below threshold.
int OtsuStats(const int* histogram, int* H_out, int* omega0_out) {
  int H = 0;
  double mu_T = 0.0;
  for (int i = 0; i < kHistogramSize; ++i) {
    H += histogram[i];
    mu_T += static_cast<double>(i) * histogram[i];
  }

  // Now maximize sig_sq_B over t.
  // http://www.ctie.monash.edu.au/hargreave/Cornall_Terry_328.pdf
  int best_t = -1;
  int omega_0, omega_1;
  int best_omega_0 = 0;
  double best_sig_sq_B = 0.0;
  double mu_0, mu_1, mu_t;
  omega_0 = 0;
  mu_t = 0.0;
  for (int t = 0; t < kHistogramSize - 1; ++t) {
    omega_0 += histogram[t];
    mu_t += t * static_cast<double>(histogram[t]);
    if (omega_0 == 0)
      continue;
    omega_1 = H - omega_0;
    if (omega_1 == 0)
      break;
    mu_0 = mu_t / omega_0;
    mu_1 = (mu_T - mu_t) / omega_1;
    double sig_sq_B = mu_1 - mu_0;
    sig_sq_B *= sig_sq_B * omega_0 * omega_1;
    if (best_t < 0 || sig_sq_B > best_sig_sq_B) {
      best_sig_sq_B = sig_sq_B;
      best_t = t;
      best_omega_0 = omega_0;
    }
  }
  if (H_out != NULL) *H_out = H;
  if (omega0_out != NULL) *omega0_out = best_omega_0;
  return best_t;
}

}  // namespace tesseract.
