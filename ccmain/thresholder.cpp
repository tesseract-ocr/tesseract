///////////////////////////////////////////////////////////////////////
// File:        thresholder.cpp
// Description: Base API for thresolding images in tesseract.
// Author:      Ray Smith
// Created:     Mon May 12 11:28:15 PDT 2008
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

#include "allheaders.h"

#include "thresholder.h"

#include <string.h>

#include "otsuthr.h"

#include "openclwrapper.h"

namespace tesseract {

ImageThresholder::ImageThresholder()
  : pix_(NULL),
    image_width_(0), image_height_(0),
    pix_channels_(0), pix_wpl_(0),
    scale_(1), yres_(300), estimated_res_(300) {
  SetRectangle(0, 0, 0, 0);
}

ImageThresholder::~ImageThresholder() {
  Clear();
}

// Destroy the Pix if there is one, freeing memory.
void ImageThresholder::Clear() {
  pixDestroy(&pix_);
}

// Return true if no image has been set.
bool ImageThresholder::IsEmpty() const {
  return pix_ == NULL;
}

// SetImage makes a copy of all the image data, so it may be deleted
// immediately after this call.
// Greyscale of 8 and color of 24 or 32 bits per pixel may be given.
// Palette color images will not work properly and must be converted to
// 24 bit.
// Binary images of 1 bit per pixel may also be given but they must be
// byte packed with the MSB of the first byte being the first pixel, and a
// one pixel is WHITE. For binary images set bytes_per_pixel=0.
void ImageThresholder::SetImage(const unsigned char* imagedata,
                                int width, int height,
                                int bytes_per_pixel, int bytes_per_line) {
  int bpp = bytes_per_pixel * 8;
  if (bpp == 0) bpp = 1;
  Pix* pix = pixCreate(width, height, bpp == 24 ? 32 : bpp);
  l_uint32* data = pixGetData(pix);
  int wpl = pixGetWpl(pix);
  switch (bpp) {
  case 1:
    for (int y = 0; y < height; ++y, data += wpl, imagedata += bytes_per_line) {
      for (int x = 0; x < width; ++x) {
        if (imagedata[x / 8] & (0x80 >> (x % 8)))
          CLEAR_DATA_BIT(data, x);
        else
          SET_DATA_BIT(data, x);
      }
    }
    break;

  case 8:
    // Greyscale just copies the bytes in the right order.
    for (int y = 0; y < height; ++y, data += wpl, imagedata += bytes_per_line) {
      for (int x = 0; x < width; ++x)
        SET_DATA_BYTE(data, x, imagedata[x]);
    }
    break;

  case 24:
    // Put the colors in the correct places in the line buffer.
    for (int y = 0; y < height; ++y, imagedata += bytes_per_line) {
      for (int x = 0; x < width; ++x, ++data) {
        SET_DATA_BYTE(data, COLOR_RED, imagedata[3 * x]);
        SET_DATA_BYTE(data, COLOR_GREEN, imagedata[3 * x + 1]);
        SET_DATA_BYTE(data, COLOR_BLUE, imagedata[3 * x + 2]);
      }
    }
    break;

  case 32:
    // Maintain byte order consistency across different endianness.
    for (int y = 0; y < height; ++y, imagedata += bytes_per_line, data += wpl) {
      for (int x = 0; x < width; ++x) {
        data[x] = (imagedata[x * 4] << 24) | (imagedata[x * 4 + 1] << 16) |
                  (imagedata[x * 4 + 2] << 8) | imagedata[x * 4 + 3];
      }
    }
    break;

  default:
    tprintf("Cannot convert RAW image to Pix with bpp = %d\n", bpp);
  }
  pixSetYRes(pix, 300);
  SetImage(pix);
  pixDestroy(&pix);
}

// Store the coordinates of the rectangle to process for later use.
// Doesn't actually do any thresholding.
void ImageThresholder::SetRectangle(int left, int top, int width, int height) {
  rect_left_ = left;
  rect_top_ = top;
  rect_width_ = width;
  rect_height_ = height;
}

// Get enough parameters to be able to rebuild bounding boxes in the
// original image (not just within the rectangle).
// Left and top are enough with top-down coordinates, but
// the height of the rectangle and the image are needed for bottom-up.
void ImageThresholder::GetImageSizes(int* left, int* top,
                                     int* width, int* height,
                                     int* imagewidth, int* imageheight) {
  *left = rect_left_;
  *top = rect_top_;
  *width = rect_width_;
  *height = rect_height_;
  *imagewidth = image_width_;
  *imageheight = image_height_;
}

// Pix vs raw, which to use? Pix is the preferred input for efficiency,
// since raw buffers are copied.
// SetImage for Pix clones its input, so the source pix may be pixDestroyed
// immediately after, but may not go away until after the Thresholder has
// finished with it.
void ImageThresholder::SetImage(const Pix* pix) {
  if (pix_ != NULL)
    pixDestroy(&pix_);
  Pix* src = const_cast<Pix*>(pix);
  int depth;
  pixGetDimensions(src, &image_width_, &image_height_, &depth);
  // Convert the image as necessary so it is one of binary, plain RGB, or
  // 8 bit with no colormap.
  if (depth > 1 && depth < 8) {
    pix_ = pixConvertTo8(src, false);
  } else if (pixGetColormap(src)) {
    pix_ = pixRemoveColormap(src, REMOVE_CMAP_BASED_ON_SRC);
  } else {
    pix_ = pixClone(src);
  }
  depth = pixGetDepth(pix_);
  pix_channels_ = depth / 8;
  pix_wpl_ = pixGetWpl(pix_);
  scale_ = 1;
  estimated_res_ = yres_ = pixGetYRes(src);
  Init();
}

// Threshold the source image as efficiently as possible to the output Pix.
// Creates a Pix and sets pix to point to the resulting pointer.
// Caller must use pixDestroy to free the created Pix.
void ImageThresholder::ThresholdToPix(Pix** pix) {
  if (pix_channels_ == 0) {
    // We have a binary image, so it just has to be cloned.
    *pix = GetPixRect();
  } else {
    OtsuThresholdRectToPix(pix_, pix);
  }
}

// Gets a pix that contains an 8 bit threshold value at each pixel. The
// returned pix may be an integer reduction of the binary image such that
// the scale factor may be inferred from the ratio of the sizes, even down
// to the extreme of a 1x1 pixel thresholds image.
// Ideally the 8 bit threshold should be the exact threshold used to generate
// the binary image in ThresholdToPix, but this is not a hard constraint.
// Returns NULL if the input is binary. PixDestroy after use.
Pix* ImageThresholder::GetPixRectThresholds() {
  if (IsBinary()) return NULL;
  Pix* pix_grey = GetPixRectGrey();
  int width = pixGetWidth(pix_grey);
  int height = pixGetHeight(pix_grey);
  int* thresholds;
  int* hi_values;
  OtsuThreshold(pix_grey, 0, 0, width, height, &thresholds, &hi_values);
  pixDestroy(&pix_grey);
  Pix* pix_thresholds = pixCreate(width, height, 8);
  int threshold = thresholds[0] > 0 ? thresholds[0] : 128;
  pixSetAllArbitrary(pix_thresholds, threshold);
  delete [] thresholds;
  delete [] hi_values;
  return pix_thresholds;
}

// Common initialization shared between SetImage methods.
void ImageThresholder::Init() {
  SetRectangle(0, 0, image_width_, image_height_);
}

// Get a clone/copy of the source image rectangle.
// The returned Pix must be pixDestroyed.
// This function will be used in the future by the page layout analysis, and
// the layout analysis that uses it will only be available with Leptonica,
// so there is no raw equivalent.
Pix* ImageThresholder::GetPixRect() {
  if (IsFullImage()) {
    // Just clone the whole thing.
    return pixClone(pix_);
  } else {
    // Crop to the given rectangle.
    Box* box = boxCreate(rect_left_, rect_top_, rect_width_, rect_height_);
    Pix* cropped = pixClipRectangle(pix_, box, NULL);
    boxDestroy(&box);
    return cropped;
  }
}

// Get a clone/copy of the source image rectangle, reduced to greyscale,
// and at the same resolution as the output binary.
// The returned Pix must be pixDestroyed.
// Provided to the classifier to extract features from the greyscale image.
Pix* ImageThresholder::GetPixRectGrey() {
  Pix* pix = GetPixRect();  // May have to be reduced to grey.
  int depth = pixGetDepth(pix);
  if (depth != 8) {
    Pix* result = depth < 8 ? pixConvertTo8(pix, false)
                            : pixConvertRGBToLuminance(pix);
    pixDestroy(&pix);
    return result;
  }
  return pix;
}

// Otsu thresholds the rectangle, taking the rectangle from *this.
void ImageThresholder::OtsuThresholdRectToPix(Pix* src_pix,
                                              Pix** out_pix) const {
  PERF_COUNT_START("OtsuThresholdRectToPix")
  int* thresholds;
  int* hi_values;

  int num_channels = OtsuThreshold(src_pix, rect_left_, rect_top_, rect_width_,
                                   rect_height_, &thresholds, &hi_values);
  // only use opencl if compiled w/ OpenCL and selected device is opencl
#ifdef USE_OPENCL
  OpenclDevice od;
  if ((num_channels == 4 || num_channels == 1) &&
      od.selectedDeviceIsOpenCL() && rect_top_ == 0 && rect_left_ == 0 ) {
    od.ThresholdRectToPixOCL((const unsigned char*)pixGetData(src_pix),
                             num_channels, pixGetWpl(src_pix) * 4,
                             thresholds, hi_values, out_pix /*pix_OCL*/,
                             rect_height_, rect_width_, rect_top_, rect_left_);
  } else {
#endif
    ThresholdRectToPix(src_pix, num_channels, thresholds, hi_values, out_pix);
#ifdef USE_OPENCL
  }
#endif
  delete [] thresholds;
  delete [] hi_values;

  PERF_COUNT_END
}

/// Threshold the rectangle, taking everything except the src_pix
/// from the class, using thresholds/hi_values to the output pix.
/// NOTE that num_channels is the size of the thresholds and hi_values
// arrays and also the bytes per pixel in src_pix.
void ImageThresholder::ThresholdRectToPix(Pix* src_pix,
                                          int num_channels,
                                          const int* thresholds,
                                          const int* hi_values,
                                          Pix** pix) const {
  PERF_COUNT_START("ThresholdRectToPix")
  *pix = pixCreate(rect_width_, rect_height_, 1);
  uinT32* pixdata = pixGetData(*pix);
  int wpl = pixGetWpl(*pix);
  int src_wpl = pixGetWpl(src_pix);
  uinT32* srcdata = pixGetData(src_pix);
  for (int y = 0; y < rect_height_; ++y) {
    const uinT32* linedata = srcdata + (y + rect_top_) * src_wpl;
    uinT32* pixline = pixdata + y * wpl;
    for (int x = 0; x < rect_width_; ++x) {
      bool white_result = true;
      for (int ch = 0; ch < num_channels; ++ch) {
        int pixel = GET_DATA_BYTE(const_cast<void*>(
                                  reinterpret_cast<const void *>(linedata)),
                                  (x + rect_left_) * num_channels + ch);
        if (hi_values[ch] >= 0 &&
            (pixel > thresholds[ch]) == (hi_values[ch] == 0)) {
          white_result = false;
          break;
        }
      }
      if (white_result)
        CLEAR_DATA_BIT(pixline, x);
      else
        SET_DATA_BIT(pixline, x);
    }
  }

  PERF_COUNT_END
}

}  // namespace tesseract.

