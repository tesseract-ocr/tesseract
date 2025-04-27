///////////////////////////////////////////////////////////////////////
// File:        thresholder.cpp
// Description: Base API for thresholding images in tesseract.
// Author:      Ray Smith
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

// Include automatically generated configuration file
#ifdef HAVE_CONFIG_H
#  include "config_auto.h"
#endif

#include "otsuthr.h"
#include "thresholder.h"
#include "tprintf.h" // for tprintf

#include <allheaders.h>
#include <tesseract/baseapi.h> // for api->GetIntVariable()

#include <algorithm> // for std::max, std::min
#include <cstdint>   // for uint32_t
#include <cstring>
#include <tuple>

namespace tesseract {

ImageThresholder::ImageThresholder()
    : pix_(nullptr)
    , image_width_(0)
    , image_height_(0)
    , pix_channels_(0)
    , pix_wpl_(0)
    , scale_(1)
    , yres_(300)
    , estimated_res_(300) {
  SetRectangle(0, 0, 0, 0);
}

ImageThresholder::~ImageThresholder() {
  Clear();
}

// Destroy the Pix if there is one, freeing memory.
void ImageThresholder::Clear() {
  pix_.destroy();
}

// Return true if no image has been set.
bool ImageThresholder::IsEmpty() const {
  return pix_ == nullptr;
}

// SetImage makes a copy of all the image data, so it may be deleted
// immediately after this call.
// Greyscale of 8 and color of 24 or 32 bits per pixel may be given.
// Palette color images will not work properly and must be converted to
// 24 bit.
// Binary images of 1 bit per pixel may also be given but they must be
// byte packed with the MSB of the first byte being the first pixel, and a
// one pixel is WHITE. For binary images set bytes_per_pixel=0.
void ImageThresholder::SetImage(const unsigned char *imagedata, int width, int height,
                                int bytes_per_pixel, int bytes_per_line) {
  int bpp = bytes_per_pixel * 8;
  if (bpp == 0) {
    bpp = 1;
  }
  Image pix = pixCreate(width, height, bpp == 24 ? 32 : bpp);
  l_uint32 *data = pixGetData(pix);
  int wpl = pixGetWpl(pix);
  switch (bpp) {
    case 1:
      for (int y = 0; y < height; ++y, data += wpl, imagedata += bytes_per_line) {
        for (int x = 0; x < width; ++x) {
          if (imagedata[x / 8] & (0x80 >> (x % 8))) {
            CLEAR_DATA_BIT(data, x);
          } else {
            SET_DATA_BIT(data, x);
          }
        }
      }
      break;

    case 8:
      // Greyscale just copies the bytes in the right order.
      for (int y = 0; y < height; ++y, data += wpl, imagedata += bytes_per_line) {
        for (int x = 0; x < width; ++x) {
          SET_DATA_BYTE(data, x, imagedata[x]);
        }
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
  SetImage(pix);
  pix.destroy();
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
void ImageThresholder::GetImageSizes(int *left, int *top, int *width, int *height, int *imagewidth,
                                     int *imageheight) {
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
void ImageThresholder::SetImage(const Image pix) {
  if (pix_ != nullptr) {
    pix_.destroy();
  }
  Image src = pix;
  int depth;
  pixGetDimensions(src, &image_width_, &image_height_, &depth);
  // Convert the image as necessary so it is one of binary, plain RGB, or
  // 8 bit with no colormap. Guarantee that we always end up with our own copy,
  // not just a clone of the input.
  if (depth > 1 && depth < 8) {
    pix_ = pixConvertTo8(src, false);
  } else {
    pix_ = src.copy();
  }
  depth = pixGetDepth(pix_);
  pix_channels_ = depth / 8;
  pix_wpl_ = pixGetWpl(pix_);
  scale_ = 1;
  estimated_res_ = yres_ = pixGetYRes(pix_);
  Init();
}

std::tuple<bool, Image, Image, Image> ImageThresholder::Threshold(
                                                      TessBaseAPI *api,
                                                      ThresholdMethod method) {
  Image pix_binary = nullptr;
  Image pix_thresholds = nullptr;

  if (pix_channels_ == 0) {
    // We have a binary image, but it still has to be copied, as this API
    // allows the caller to modify the output.
    Image original = GetPixRect();
    pix_binary = original.copy();
    original.destroy();
    return std::make_tuple(true, nullptr, pix_binary, nullptr);
  }

  auto pix_grey = GetPixRectGrey();

  int r;

  l_int32 pix_w, pix_h;
  pixGetDimensions(pix_grey, &pix_w, &pix_h, nullptr);

  bool thresholding_debug;
  api->GetBoolVariable("thresholding_debug", &thresholding_debug);
  if (thresholding_debug) {
    tprintf("\nimage width: %d  height: %d  ppi: %d\n", pix_w, pix_h, yres_);
  }

  if (method == ThresholdMethod::Sauvola && pix_w > 6 && pix_h > 6) {
    // pixSauvolaBinarizeTiled requires half_window_size >= 2.
    // Therefore window_size must be at least 4 which requires
    // pix_w and pix_h to be at least 7.
    int window_size;
    double window_size_factor;
    api->GetDoubleVariable("thresholding_window_size", &window_size_factor);
    window_size = window_size_factor * yres_;
    window_size = std::max(7, window_size);
    window_size = std::min(pix_w < pix_h ? pix_w - 3 : pix_h - 3, window_size);
    int half_window_size = window_size / 2;

    // factor for image division into tiles; >= 1
    l_int32 nx, ny;
    // tiles size will be approx. 250 x 250 pixels
    nx = std::max(1, (pix_w + 125) / 250);
    ny = std::max(1, (pix_h + 125) / 250);
    auto xrat = pix_w / nx;
    auto yrat = pix_h / ny;
    if (xrat < half_window_size + 2) {
      nx = pix_w / (half_window_size + 2);
    }
    if (yrat < half_window_size + 2) {
      ny = pix_h / (half_window_size + 2);
    }

    double kfactor;
    api->GetDoubleVariable("thresholding_kfactor", &kfactor);
    kfactor = std::max(0.0, kfactor);

    if (thresholding_debug) {
      tprintf("window size: %d  kfactor: %.3f  nx:%d  ny: %d\n", window_size, kfactor, nx, ny);
    }

    r = pixSauvolaBinarizeTiled(pix_grey, half_window_size, kfactor, nx, ny,
                               (PIX**)pix_thresholds,
                                (PIX**)pix_binary);
  } else { // if (method == ThresholdMethod::LeptonicaOtsu)
    int tile_size;
    double tile_size_factor;
    api->GetDoubleVariable("thresholding_tile_size", &tile_size_factor);
    tile_size = tile_size_factor * yres_;
    tile_size = std::max(16, tile_size);

    int smooth_size;
    double smooth_size_factor;
    api->GetDoubleVariable("thresholding_smooth_kernel_size",
                         &smooth_size_factor);
    smooth_size_factor = std::max(0.0, smooth_size_factor);
    smooth_size = smooth_size_factor * yres_;
    int half_smooth_size = smooth_size / 2;

    double score_fraction;
    api->GetDoubleVariable("thresholding_score_fraction", &score_fraction);

    if (thresholding_debug) {
      tprintf("tile size: %d  smooth_size: %d  score_fraction: %.2f\n", tile_size, smooth_size, score_fraction);
    }

    r = pixOtsuAdaptiveThreshold(pix_grey, tile_size, tile_size,
                                 half_smooth_size, half_smooth_size,
                                 score_fraction,
                                 (PIX**)pix_thresholds,
                                 (PIX**)pix_binary);
  }

  bool ok = (r == 0);
  return std::make_tuple(ok, pix_grey, pix_binary, pix_thresholds);
}

// Threshold the source image as efficiently as possible to the output Pix.
// Creates a Pix and sets pix to point to the resulting pointer.
// Caller must use pixDestroy to free the created Pix.
/// Returns false on error.
bool ImageThresholder::ThresholdToPix(Image *pix) {
  if (image_width_ > INT16_MAX || image_height_ > INT16_MAX) {
    tprintf("Image too large: (%d, %d)\n", image_width_, image_height_);
    return false;
  }
  // Handle binary image
  if (pix_channels_ == 0) {
    // We have a binary image, but it still has to be copied, as this API
    // allows the caller to modify the output.
    Image original = GetPixRect();
    *pix = original.copy();
    original.destroy();
    return true;
  }
  // Handle colormaps
  Image src = pix_;
  if (pixGetColormap(src)) {
    src = pixRemoveColormap(src, REMOVE_CMAP_BASED_ON_SRC);
  }
  OtsuThresholdRectToPix(src, pix);
  if (src != pix_) {
    src.destroy();
  }
  return true;
}

// Gets a pix that contains an 8 bit threshold value at each pixel. The
// returned pix may be an integer reduction of the binary image such that
// the scale factor may be inferred from the ratio of the sizes, even down
// to the extreme of a 1x1 pixel thresholds image.
// Ideally the 8 bit threshold should be the exact threshold used to generate
// the binary image in ThresholdToPix, but this is not a hard constraint.
// Returns nullptr if the input is binary. PixDestroy after use.
Image ImageThresholder::GetPixRectThresholds() {
  if (IsBinary()) {
    return nullptr;
  }
  Image pix_grey = GetPixRectGrey();
  int width = pixGetWidth(pix_grey);
  int height = pixGetHeight(pix_grey);
  std::vector<int> thresholds;
  std::vector<int> hi_values;
  OtsuThreshold(pix_grey, 0, 0, width, height, thresholds, hi_values);
  pix_grey.destroy();
  Image pix_thresholds = pixCreate(width, height, 8);
  int threshold = thresholds[0] > 0 ? thresholds[0] : 128;
  pixSetAllArbitrary(pix_thresholds, threshold);
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
Image ImageThresholder::GetPixRect() {
  if (IsFullImage()) {
    // Just clone the whole thing.
    return pix_.clone();
  } else {
    // Crop to the given rectangle.
    Box *box = boxCreate(rect_left_, rect_top_, rect_width_, rect_height_);
    Image cropped = pixClipRectangle(pix_, box, nullptr);
    boxDestroy(&box);
    return cropped;
  }
}

// Get a clone/copy of the source image rectangle, reduced to greyscale,
// and at the same resolution as the output binary.
// The returned Pix must be pixDestroyed.
// Provided to the classifier to extract features from the greyscale image.
Image ImageThresholder::GetPixRectGrey() {
  auto pix = GetPixRect(); // May have to be reduced to grey.
  int depth = pixGetDepth(pix);
  if (depth != 8 || pixGetColormap(pix)) {
    if (depth == 24) {
      auto tmp = pixConvert24To32(pix);
      pix.destroy();
      pix = tmp;
    }
    auto result = pixConvertTo8(pix, false);
    pix.destroy();
    return result;
  }
  return pix;
}

// Otsu thresholds the rectangle, taking the rectangle from *this.
void ImageThresholder::OtsuThresholdRectToPix(Image src_pix, Image *out_pix) const {
  std::vector<int> thresholds;
  std::vector<int> hi_values;

  int num_channels = OtsuThreshold(src_pix, rect_left_, rect_top_, rect_width_, rect_height_,
                                   thresholds, hi_values);
  ThresholdRectToPix(src_pix, num_channels, thresholds, hi_values, out_pix);
}

/// Threshold the rectangle, taking everything except the src_pix
/// from the class, using thresholds/hi_values to the output pix.
/// NOTE that num_channels is the size of the thresholds and hi_values
// arrays and also the bytes per pixel in src_pix.
void ImageThresholder::ThresholdRectToPix(Image src_pix, int num_channels, const std::vector<int> &thresholds,
                                          const std::vector<int> &hi_values, Image *pix) const {
  *pix = pixCreate(rect_width_, rect_height_, 1);
  uint32_t *pixdata = pixGetData(*pix);
  int wpl = pixGetWpl(*pix);
  int src_wpl = pixGetWpl(src_pix);
  uint32_t *srcdata = pixGetData(src_pix);
  pixSetXRes(*pix, pixGetXRes(src_pix));
  pixSetYRes(*pix, pixGetYRes(src_pix));
  for (int y = 0; y < rect_height_; ++y) {
    const uint32_t *linedata = srcdata + (y + rect_top_) * src_wpl;
    uint32_t *pixline = pixdata + y * wpl;
    for (int x = 0; x < rect_width_; ++x) {
      bool white_result = true;
      for (int ch = 0; ch < num_channels; ++ch) {
        int pixel = GET_DATA_BYTE(linedata, (x + rect_left_) * num_channels + ch);
        if (hi_values[ch] >= 0 && (pixel > thresholds[ch]) == (hi_values[ch] == 0)) {
          white_result = false;
          break;
        }
      }
      if (white_result) {
        CLEAR_DATA_BIT(pixline, x);
      } else {
        SET_DATA_BIT(pixline, x);
      }
    }
  }
}

} // namespace tesseract.
