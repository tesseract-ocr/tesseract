/*
 * Copyright 2011, Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// Author: Xiaotao Duan
//
// This library contains image processing method to estimate
// similarity of two given images.
//
// This library is *not* thread safe because static memory is
// used for performance.
//
// Two methods are provided to estimate the similarity of two
// given images. ComputeSignature() is used to compute the
// signature of a given image. Diff() takes two signatures
// as input to compute how similar two given images are.
// The algorithm is based on R. O. Stehling, M. A. Nascimento,
// and A. X. Falc Ìƒao. "A Compact and Efficient Image Retrieval
// Approach Based on Border/Interior Pixel Classification".
//
// For performance consideration, 480x480 of central area of
// a given image is used for signature computation.

#include "similar.h"
#include "utils.h"

// How many quantized colors using to compute signature.
// For performance consideration, it's specified in format of
// number of left shift bits rather than color numbers directly.
// e.g. kShiftColors 4 means (1 << 4 == 16) colors are used.
static const int kShiftColors = 4;
static const int kDesiredWidthForSignature = 480;
static const int kDesiredHeightForSignature = 480;
static uint8 _colors[kDesiredHeightForSignature * kDesiredWidthForSignature];
// The final signature contains both color information of
// inner and outer pixels, and total pixel count at last.
static uint32_t _signature[1 + (1 << kShiftColors) * 2];

uint32_t* ComputeSignatureInner(const uint8* const luminance,
    int width, int height,
    int left, int top, int desired_width, int desired_height, int* size) {
  int num_colors = 1 << kShiftColors;

  memset(_signature, 0, sizeof(int32) * num_colors * 2);

  // Build quantized color map for input image. For each possible lumiance
  // value from 0 to 255, quantize it into more coarse value.
#ifdef HAVE_ARMEABI_V7A
  if (supportsNeon()) {
    for (int i = 0; i < desired_height; ++i) {
      uint8* ptr_colors = _colors + i * desired_width;
      const uint8* ptr_lumi = luminance + (i + top) * width + left;
      int itrs = desired_width / 8;
      for (int j = 0; j < itrs; ++j) {
        // Load 8 bytes from ptr_lumi into neon vector
        uint8x8_t lumix8 = vld1_u8(ptr_lumi);
        // Batch right shift every elements (8 - kShiftColors) bits
        // in the vector
        uint8x8_t colorx8 = vshr_n_u8(lumix8, 8 - kShiftColors);
        // Store above result into our own memory starting from ptr_colors
        vst1_u8(ptr_colors, colorx8);

        ptr_lumi += 8;
        ptr_colors += 8;
      }
    }
  } else {
#endif
    int shift_bits = 8 - kShiftColors;  // equals to 256/num_colors
    for (int i = 0; i < desired_height; ++i) {
      uint8* ptr_colors = &_colors[i * desired_width];
      const uint8* ptr_lumi = luminance + (i + top) * width + left;
      for (int j = 0; j < desired_width; ++j) {
        *ptr_colors++ = *ptr_lumi++ >> shift_bits;
      }
    }
#ifdef HAVE_ARMEABI_V7A
  }
#endif

  // Go through each pixel, decide it is a inner pixel (having same
  // quantized color as its 4 neighbours) or an outer one (at least one of
  // his 4 neighbours has different color), update signature respectively.
  int h = desired_height - 1;
  int w = desired_width - 1;
  for (int i = 1; i < h; ++i) {
    uint8* ptr_colors = _colors + i * desired_width;
    for (int j = 1; j < w; ++j) {
      int inner = 0;
      uint8 y = ptr_colors[j];
      uint8 yleft = ptr_colors[j - 1];
      if (y == yleft) {
        uint8 yright = ptr_colors[j + 1];
        if (y == yright) {
          uint8 yup = ptr_colors[j - desired_width];
          if (y == yup) {
            uint8 ydown = ptr_colors[j + desired_width];
            if (y == ydown) {
              inner = 1;
            }
          }
        }
      }
      _signature[inner * num_colors + y] += 1;
    }
  }

  *size = sizeof(_signature) / sizeof(uint32);
  _signature[*size - 1] = (desired_height - 2) * (desired_width - 2);

  return _signature;
}

uint32_t* ComputeSignature(const uint8* const luminance,
    const int width, const int height, int* size) {
  int desired_width = min(kDesiredWidthForSignature, width);
  int desired_height = min(kDesiredHeightForSignature, height);
  int left = (width - desired_width) >> 1;
  int top = (height - desired_height) >> 1;

  return ComputeSignatureInner(luminance, width, height,
      left, top, desired_width, desired_height, size);
}

int Diff(const int32* const signature1, const int32* const signature2,
    const int size) {
  int total = signature1[size - 1];
  int diff = 0;
  for (int i = 0; i < size; ++i) {
    diff += abs(signature1[i] - signature2[i]);
  }
  return diff * 50 / total;
}
