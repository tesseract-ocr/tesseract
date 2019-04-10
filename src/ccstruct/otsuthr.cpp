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

#include "otsuthr.h"

#include <cstring>
#include "allheaders.h"
#include "helpers.h"
#if defined(USE_OPENCL)
#include "openclwrapper.h" // for OpenclDevice
#endif

namespace tesseract {

// Computes the Otsu threshold(s) for the given image rectangle, making one
// for each channel. Each channel is always one byte per pixel.
// Returns an array of threshold values and an array of hi_values, such
// that a pixel value >threshold[channel] is considered foreground if
// hi_values[channel] is 0 or background if 1. A hi_value of -1 indicates
// that there is no apparent foreground. At least one hi_value will not be -1.
// Delete thresholds and hi_values with delete [] after use.
// The return value is the number of channels in the input image, being
// the size of the output thresholds and hi_values arrays.
int OtsuThreshold(Pix* src_pix, int left, int top, int width, int height,
                  int** thresholds, int** hi_values) {
  int num_channels = pixGetDepth(src_pix) / 8;
  // Of all channels with no good hi_value, keep the best so we can always
  // produce at least one answer.
  int best_hi_value = 1;
  int best_hi_index = 0;
  bool any_good_hivalue = false;
  double best_hi_dist = 0.0;
  *thresholds = new int[num_channels];
  *hi_values = new int[num_channels];

  // only use opencl if compiled w/ OpenCL and selected device is opencl
#ifdef USE_OPENCL
  // all of channel 0 then all of channel 1...
  int* histogramAllChannels = new int[kHistogramSize * num_channels];

  // Calculate Histogram on GPU
  OpenclDevice od;
  if (od.selectedDeviceIsOpenCL() && (num_channels == 1 || num_channels == 4) &&
      top == 0 && left == 0) {
    od.HistogramRectOCL(pixGetData(src_pix), num_channels,
                        pixGetWpl(src_pix) * 4, left, top, width, height,
                        kHistogramSize, histogramAllChannels);

    // Calculate Threshold from Histogram on cpu
    for (int ch = 0; ch < num_channels; ++ch) {
      (*thresholds)[ch] = -1;
      (*hi_values)[ch] = -1;
      int *histogram = &histogramAllChannels[kHistogramSize * ch];
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
  } else {
#endif
    for (int ch = 0; ch < num_channels; ++ch) {
      (*thresholds)[ch] = -1;
      (*hi_values)[ch] = -1;
      // Compute the histogram of the image rectangle.
      int histogram[kHistogramSize];
      HistogramRect(src_pix, ch, left, top, width, height, histogram);
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
#ifdef USE_OPENCL
  }
  delete[] histogramAllChannels;
#endif  // USE_OPENCL

  if (!any_good_hivalue) {
    // Use the best of the ones that were not good enough.
    (*hi_values)[best_hi_index] = best_hi_value;
  }
  return num_channels;
}

// Computes the histogram for the given image rectangle, and the given
// single channel. Each channel is always one byte per pixel.
// Histogram is always a kHistogramSize(256) element array to count
// occurrences of each pixel value.
void HistogramRect(Pix* src_pix, int channel,
                   int left, int top, int width, int height,
                   int* histogram) {
  int num_channels = pixGetDepth(src_pix) / 8;
  channel = ClipToRange(channel, 0, num_channels - 1);
  int bottom = top + height;
  memset(histogram, 0, sizeof(*histogram) * kHistogramSize);
  int src_wpl = pixGetWpl(src_pix);
  l_uint32* srcdata = pixGetData(src_pix);
  for (int y = top; y < bottom; ++y) {
    const l_uint32* linedata = srcdata + y * src_wpl;
    for (int x = 0; x < width; ++x) {
      int pixel = GET_DATA_BYTE(linedata, (x + left) * num_channels + channel);
      ++histogram[pixel];
    }
  }
}

// Computes the Otsu threshold(s) for the given histogram.
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
  if (H_out != nullptr) *H_out = H;
  if (omega0_out != nullptr) *omega0_out = best_omega_0;
  return best_t;
}

}  // namespace tesseract.
