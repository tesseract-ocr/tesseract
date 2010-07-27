///////////////////////////////////////////////////////////////////////
// File:        thresholder.h
// Description: Base API for thresolding images in tesseract.
// Author:      Ray Smith
// Created:     Mon May 12 11:00:15 PDT 2008
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

#ifndef TESSERACT_CCMAIN_THRESHOLDER_H__
#define TESSERACT_CCMAIN_THRESHOLDER_H__

class IMAGE;
struct Pix;

namespace tesseract {

/// Base class for all tesseract image thresholding classes.
/// Specific classes can add new thresholding methods by
/// overriding ThresholdToIMAGE and/or ThresholdToPix.
/// Each instance deals with a single image, but the design is intended to
/// be useful for multiple calls to SetRectangle and ThresholdTo* if
/// desired.
class ImageThresholder {
 public:
  ImageThresholder();
  virtual ~ImageThresholder();

  /// Destroy the Pix if there is one, freeing memory.
  virtual void Clear();

  /// Return true if no image has been set.
  bool IsEmpty() const;

  /// SetImage makes a copy of only the metadata, not the underlying
  /// image buffer. It promises to treat the source as read-only in either case,
  /// but in return assumes that the Pix or image buffer remain valid
  /// throughout the life of the ImageThresholder.
  /// Greyscale of 8 and color of 24 or 32 bits per pixel may be given.
  /// Palette color images will not work properly and must be converted to
  /// 24 bit.
  /// Binary images of 1 bit per pixel may also be given but they must be
  /// byte packed with the MSB of the first byte being the first pixel, and a
  /// one pixel is WHITE. For binary images set bytes_per_pixel=0.
  void SetImage(const unsigned char* imagedata, int width, int height,
                int bytes_per_pixel, int bytes_per_line);

  /// Store the coordinates of the rectangle to process for later use.
  /// Doesn't actually do any thresholding.
  void SetRectangle(int left, int top, int width, int height);

  /// Get enough parameters to be able to rebuild bounding boxes in the
  /// original image (not just within the rectangle).
  /// Left and top are enough with top-down coordinates, but
  /// the height of the rectangle and the image are needed for bottom-up.
  virtual void GetImageSizes(int* left, int* top, int* width, int* height,
                             int* imagewidth, int* imageheight);

  /// Return true if HAVE_LIBLEPT and this thresholder implements the Pix
  /// interface.
  virtual bool HasThresholdToPix() const;

  /// Return true if the source image is color.
  bool IsColor() const {
    return image_bytespp_ >= 3;
  }

  /// Threshold the source image as efficiently as possible to the output
  /// tesseract IMAGE class.
  virtual void ThresholdToIMAGE(IMAGE* image);

#ifdef HAVE_LIBLEPT
  /// Pix vs raw, which to use?
  /// Implementations should provide the ability to source and target Pix
  /// where possible. A future version of Tesseract may choose to use Pix
  /// as its internal representation and discard IMAGE altogether.
  /// Because of that, an implementation that sources and targets Pix may end up
  /// with less copies than an implementation that does not.
  /// NOTE: Opposite to SetImage for raw images, SetImage for Pix clones its
  /// input, so the source pix may be pixDestroyed immediately after.
  void SetImage(const Pix* pix);

  /// Threshold the source image as efficiently as possible to the output Pix.
  /// Creates a Pix and sets pix to point to the resulting pointer.
  /// Caller must use pixDestroy to free the created Pix.
  virtual void ThresholdToPix(Pix** pix);

  /// Get a clone/copy of the source image rectangle.
  /// The returned Pix must be pixDestroyed.
  /// This function will be used in the future by the page layout analysis, and
  /// the layout analysis that uses it will only be available with Leptonica,
  /// so there is no raw equivalent.
  Pix* GetPixRect();
#endif

 protected:
  // ----------------------------------------------------------------------
  // Utility functions that may be useful components for other thresholders.

  /// Common initialization shared between SetImage methods.
  virtual void Init();

  /// Return true if we are processing the full image.
  bool IsFullImage() const {
    return rect_left_ == 0 && rect_top_ == 0 &&
           rect_width_ == image_width_ && rect_height_ == image_height_;
  }

  /// Otsu threshold the rectangle, taking everything except the image buffer
  /// pointer from the class, to the output IMAGE.
  void OtsuThresholdRectToIMAGE(const unsigned char* imagedata,
                                int bytes_per_pixel, int bytes_per_line,
                                IMAGE* image) const;

  /// Threshold the rectangle, taking everything except the image buffer pointer
  /// from the class, using thresholds/hi_values to the output IMAGE.
  void ThresholdRectToIMAGE(const unsigned char* imagedata,
                            int bytes_per_pixel, int bytes_per_line,
                            const int* thresholds, const int* hi_values,
                            IMAGE* image) const;

  /// Cut out the requested rectangle of the source raw binary image to the
  /// output IMAGE.
  void CopyBinaryRectRawToIMAGE(IMAGE* image) const;

#ifdef HAVE_LIBLEPT
  /// Otsu threshold the rectangle, taking everything except the image buffer
  /// pointer from the class, to the output Pix.
  void OtsuThresholdRectToPix(const unsigned char* imagedata,
                              int bytes_per_pixel, int bytes_per_line,
                              Pix** pix) const;

  /// Threshold the rectangle, taking everything except the image buffer pointer
  /// from the class, using thresholds/hi_values to the output IMAGE.
  void ThresholdRectToPix(const unsigned char* imagedata,
                          int bytes_per_pixel, int bytes_per_line,
                          const int* thresholds, const int* hi_values,
                          Pix** pix) const;

  /// Copy the raw image rectangle, taking all data from the class, to the Pix.
  void RawRectToPix(Pix** pix) const;

  /// Cut out the requested rectangle of the binary image to the output IMAGE.
  void CopyBinaryRectPixToIMAGE(IMAGE* image) const;
#endif

 protected:
#ifdef HAVE_LIBLEPT
  /// Clone or other copy of the source Pix.
  /// The pix will always be PixDestroy()ed on destruction of the class.
  Pix*                 pix_;
#endif
  /// Exactly one of pix_ and image_data_ is not NULL.
  const unsigned char* image_data_;     //< Raw source image.

  int                  image_width_;    //< Width of source image/pix.
  int                  image_height_;   //< Height of source image/pix.
  int                  image_bytespp_;  //< Bytes per pixel of source image/pix.
  int                  image_bytespl_;  //< Bytes per line of source image/pix.
  // Limits of image rectangle to be processed.
  int                  rect_left_;
  int                  rect_top_;
  int                  rect_width_;
  int                  rect_height_;
};

}  // namespace tesseract.

#endif  // TESSERACT_CCMAIN_THRESHOLDER_H__


