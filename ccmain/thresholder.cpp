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

#include "img.h"
#include "otsuthr.h"

namespace tesseract {

ImageThresholder::ImageThresholder()
  : pix_(NULL),
    image_data_(NULL),
    image_width_(0), image_height_(0),
    image_bytespp_(0), image_bytespl_(0),
    scale_(1), yres_(300) {
  SetRectangle(0, 0, 0, 0);
}

ImageThresholder::~ImageThresholder() {
  Clear();
}

// Destroy the Pix if there is one, freeing memory.
void ImageThresholder::Clear() {
  if (pix_ != NULL) {
    pixDestroy(&pix_);
    pix_ = NULL;
  }
  image_data_ = NULL;
}

// Return true if no image has been set.
bool ImageThresholder::IsEmpty() const {
  if (pix_ != NULL)
    return false;
  return image_data_ == NULL;
}

// SetImage makes a copy of only the metadata, not the underlying
// image buffer. It promises to treat the source as read-only in either case,
// but in return assumes that the Pix or image buffer remain valid
// throughout the life of the ImageThresholder.
// Greyscale of 8 and color of 24 or 32 bits per pixel may be given.
// Palette color images will not work properly and must be converted to
// 24 bit.
// Binary images of 1 bit per pixel may also be given but they must be
// byte packed with the MSB of the first byte being the first pixel, and a
// one pixel is WHITE. For binary images set bytes_per_pixel=0.
void ImageThresholder::SetImage(const unsigned char* imagedata,
                                int width, int height,
                                int bytes_per_pixel, int bytes_per_line) {
  if (pix_ != NULL)
    pixDestroy(&pix_);
  pix_ = NULL;
  image_data_ = imagedata;
  image_width_ = width;
  image_height_ = height;
  image_bytespp_ = bytes_per_pixel;
  image_bytespl_ = bytes_per_line;
  scale_ = 1;
  yres_ = 300;
  Init();
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

// NOTE: Opposite to SetImage for raw images, SetImage for Pix clones its
// input, so the source pix may be pixDestroyed immediately after.
void ImageThresholder::SetImage(const Pix* pix) {
  image_data_ = NULL;
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
  image_bytespp_ = depth / 8;
  image_bytespl_ = pixGetWpl(pix_) * sizeof(l_uint32);
  scale_ = 1;
  yres_ = pixGetYRes(src);
  Init();
}

// Threshold the source image as efficiently as possible to the output Pix.
// Creates a Pix and sets pix to point to the resulting pointer.
// Caller must use pixDestroy to free the created Pix.
void ImageThresholder::ThresholdToPix(Pix** pix) {
  if (pix_ != NULL) {
    if (image_bytespp_ == 0) {
      // We have a binary image, so it just has to be cloned.
      *pix = GetPixRect();
    } else {
      if (image_bytespp_ == 4) {
        // Color data can just be passed direct.
        const uinT32* data = pixGetData(pix_);
        OtsuThresholdRectToPix(reinterpret_cast<const uinT8*>(data),
                               image_bytespp_, image_bytespl_, pix);
      } else {
        // Convert 8-bit to IMAGE and then pass its
        // buffer to the raw interface to complete the conversion.
        IMAGE temp_image;
        temp_image.FromPix(pix_);
        OtsuThresholdRectToPix(temp_image.get_buffer(),
                               image_bytespp_,
                               COMPUTE_IMAGE_XDIM(temp_image.get_xsize(),
                                                  temp_image.get_bpp()),
                               pix);
      }
    }
    return;
  }
  if (image_bytespp_ > 0) {
    // Threshold grey or color.
    OtsuThresholdRectToPix(image_data_, image_bytespp_, image_bytespl_, pix);
  } else {
    RawRectToPix(pix);
  }
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
  if (pix_ != NULL) {
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
  // The input is raw, so we have to make a copy of it.
  Pix* raw_pix;
  RawRectToPix(&raw_pix);
  return raw_pix;
}

// Get a clone/copy of the source image rectangle, reduced to greyscale.
// The returned Pix must be pixDestroyed.
// This function will be used in the future by the page layout analysis, and
// the layout analysis that uses it will only be available with Leptonica,
// so there is no raw equivalent.
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

// Otsu threshold the rectangle, taking everything except the image buffer
// pointer from the class, to the output Pix.
void ImageThresholder::OtsuThresholdRectToPix(const unsigned char* imagedata,
                                              int bytes_per_pixel,
                                              int bytes_per_line,
                                              Pix** pix) const {
  int* thresholds;
  int* hi_values;
  OtsuThreshold(imagedata, bytes_per_pixel, bytes_per_line,
                rect_left_, rect_top_, rect_width_, rect_height_,
                &thresholds, &hi_values);

  // Threshold the image to the given IMAGE.
  ThresholdRectToPix(imagedata, bytes_per_pixel, bytes_per_line,
                     thresholds, hi_values, pix);
  delete [] thresholds;
  delete [] hi_values;
}

// Threshold the rectangle, taking everything except the image buffer pointer
// from the class, using thresholds/hi_values to the output IMAGE.
void ImageThresholder::ThresholdRectToPix(const unsigned char* imagedata,
                                          int bytes_per_pixel,
                                          int bytes_per_line,
                                          const int* thresholds,
                                          const int* hi_values,
                                          Pix** pix) const {
  *pix = pixCreate(rect_width_, rect_height_, 1);
  uinT32* pixdata = pixGetData(*pix);
  int wpl = pixGetWpl(*pix);
  const unsigned char* srcdata = imagedata + rect_top_* bytes_per_line +
                                 rect_left_ * bytes_per_pixel;
  for (int y = 0; y < rect_height_; ++y) {
    const uinT8* linedata = srcdata;
    uinT32* pixline = pixdata + y * wpl;
    for (int x = 0; x < rect_width_; ++x, linedata += bytes_per_pixel) {
      bool white_result = true;
      for (int ch = 0; ch < bytes_per_pixel; ++ch) {
        if (hi_values[ch] >= 0 &&
            (linedata[ch] > thresholds[ch]) == (hi_values[ch] == 0)) {
          white_result = false;
          break;
        }
      }
      if (white_result)
        CLEAR_DATA_BIT(pixline, x);
      else
        SET_DATA_BIT(pixline, x);
    }
    srcdata += bytes_per_line;
  }
}

// Copy the raw image rectangle, taking all data from the class, to the Pix.
void ImageThresholder::RawRectToPix(Pix** pix) const {
  if (image_bytespp_ < 4) {
    // Go via a tesseract image structure (doesn't copy the data)
    // and use ToPix.
    IMAGE image;
    int bits_per_pixel = image_bytespp_ * 8;
    if (image_bytespp_ == 0)
      bits_per_pixel = 1;
    image.capture(const_cast<uinT8*>(image_data_),
                  image_width_, rect_top_ + rect_height_, bits_per_pixel);
    if (IsFullImage()) {
      *pix = image.ToPix();
    } else {
      IMAGE rect;
      rect.create(rect_width_, rect_height_, bits_per_pixel);
      // The capture chopped the image off at top+height, so copy
      // the rectangle with y = 0 to get a rectangle of height
      // starting at the bottom, since copy_sub_image uses bottom-up coords.
      copy_sub_image(&image, rect_left_, 0, rect_width_, rect_height_,
                     &rect, 0, 0, true);
      *pix = rect.ToPix();
    }
  } else {
    *pix = pixCreate(rect_width_, rect_height_, 32);
    uinT32* data = pixGetData(*pix);
    int wpl = pixGetWpl(*pix);
    const uinT8* imagedata = image_data_ + rect_top_ * image_bytespl_ +
                             rect_left_ * image_bytespp_;
    for (int y = 0; y < rect_height_; ++y) {
      const uinT8* linedata = imagedata;
      uinT32* line = data + y * wpl;
      for (int x = 0; x < rect_width_; ++x) {
        line[x] = (linedata[0] << 24) | (linedata[1] << 16) |
                  (linedata[2] << 8) | linedata[3];
        linedata += 4;
      }
      imagedata += image_bytespl_;
    }
  }
}

}  // namespace tesseract.

