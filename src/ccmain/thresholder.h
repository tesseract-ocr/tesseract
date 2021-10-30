///////////////////////////////////////////////////////////////////////
// File:        thresholder.h
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

#ifndef TESSERACT_CCMAIN_THRESHOLDER_H_
#define TESSERACT_CCMAIN_THRESHOLDER_H_

#include <tesseract/export.h>

#include <vector> // for std::vector

struct Pix;

namespace tesseract {

enum class ThresholdMethod {
  Otsu,          // Tesseract's legacy Otsu
  LeptonicaOtsu, // Leptonica's Otsu
  Sauvola,       // Leptonica's Sauvola
  Max,           // Number of Thresholding methods
};

class TessBaseAPI;

/// Base class for all tesseract image thresholding classes.
/// Specific classes can add new thresholding methods by
/// overriding ThresholdToPix.
/// Each instance deals with a single image, but the design is intended to
/// be useful for multiple calls to SetRectangle and ThresholdTo* if
/// desired.
class TESS_API ImageThresholder {
public:
  ImageThresholder();
  virtual ~ImageThresholder();

  /// Destroy the Pix if there is one, freeing memory.
  virtual void Clear();

  /// Return true if no image has been set.
  bool IsEmpty() const;

  /// SetImage makes a copy of all the image data, so it may be deleted
  /// immediately after this call.
  /// Greyscale of 8 and color of 24 or 32 bits per pixel may be given.
  /// Palette color images will not work properly and must be converted to
  /// 24 bit.
  /// Binary images of 1 bit per pixel may also be given but they must be
  /// byte packed with the MSB of the first byte being the first pixel, and a
  /// one pixel is WHITE. For binary images set bytes_per_pixel=0.
  void SetImage(const unsigned char *imagedata, int width, int height, int bytes_per_pixel,
                int bytes_per_line);

  /// Store the coordinates of the rectangle to process for later use.
  /// Doesn't actually do any thresholding.
  void SetRectangle(int left, int top, int width, int height);

  /// Get enough parameters to be able to rebuild bounding boxes in the
  /// original image (not just within the rectangle).
  /// Left and top are enough with top-down coordinates, but
  /// the height of the rectangle and the image are needed for bottom-up.
  virtual void GetImageSizes(int *left, int *top, int *width, int *height, int *imagewidth,
                             int *imageheight);

  /// Return true if the source image is color.
  bool IsColor() const {
    return pix_channels_ >= 3;
  }

  /// Returns true if the source image is binary.
  bool IsBinary() const {
    return pix_channels_ == 0;
  }

  int GetScaleFactor() const {
    return scale_;
  }

  // Set the resolution of the source image in pixels per inch.
  // This should be called right after SetImage(), and will let us return
  // appropriate font sizes for the text.
  void SetSourceYResolution(int ppi) {
    yres_ = ppi;
    estimated_res_ = ppi;
  }
  int GetSourceYResolution() const {
    return yres_;
  }
  int GetScaledYResolution() const {
    return scale_ * yres_;
  }
  // Set the resolution of the source image in pixels per inch, as estimated
  // by the thresholder from the text size found during thresholding.
  // This value will be used to set internal size thresholds during recognition
  // and will not influence the output "point size." The default value is
  // the same as the source resolution. (yres_)
  void SetEstimatedResolution(int ppi) {
    estimated_res_ = ppi;
  }
  // Returns the estimated resolution, including any active scaling.
  // This value will be used to set internal size thresholds during recognition.
  int GetScaledEstimatedResolution() const {
    return scale_ * estimated_res_;
  }

  /// Pix vs raw, which to use? Pix is the preferred input for efficiency,
  /// since raw buffers are copied.
  /// SetImage for Pix clones its input, so the source pix may be pixDestroyed
  /// immediately after, but may not go away until after the Thresholder has
  /// finished with it.
  void SetImage(const Image pix);

  /// Threshold the source image as efficiently as possible to the output Pix.
  /// Creates a Pix and sets pix to point to the resulting pointer.
  /// Caller must use pixDestroy to free the created Pix.
  /// Returns false on error.
  virtual bool ThresholdToPix(Image *pix);

  virtual std::tuple<bool, Image, Image, Image> Threshold(TessBaseAPI *api,
                                                          ThresholdMethod method);

  // Gets a pix that contains an 8 bit threshold value at each pixel. The
  // returned pix may be an integer reduction of the binary image such that
  // the scale factor may be inferred from the ratio of the sizes, even down
  // to the extreme of a 1x1 pixel thresholds image.
  // Ideally the 8 bit threshold should be the exact threshold used to generate
  // the binary image in ThresholdToPix, but this is not a hard constraint.
  // Returns nullptr if the input is binary. PixDestroy after use.
  virtual Image GetPixRectThresholds();

  /// Get a clone/copy of the source image rectangle.
  /// The returned Pix must be pixDestroyed.
  /// This function will be used in the future by the page layout analysis, and
  /// the layout analysis that uses it will only be available with Leptonica,
  /// so there is no raw equivalent.
  Image GetPixRect();

  // Get a clone/copy of the source image rectangle, reduced to greyscale,
  // and at the same resolution as the output binary.
  // The returned Pix must be pixDestroyed.
  // Provided to the classifier to extract features from the greyscale image.
  virtual Image GetPixRectGrey();

protected:
  // ----------------------------------------------------------------------
  // Utility functions that may be useful components for other thresholders.

  /// Common initialization shared between SetImage methods.
  virtual void Init();

  /// Return true if we are processing the full image.
  bool IsFullImage() const {
    return rect_left_ == 0 && rect_top_ == 0 && rect_width_ == image_width_ &&
           rect_height_ == image_height_;
  }

  // Otsu thresholds the rectangle, taking the rectangle from *this.
  void OtsuThresholdRectToPix(Image src_pix, Image *out_pix) const;

  /// Threshold the rectangle, taking everything except the src_pix
  /// from the class, using thresholds/hi_values to the output pix.
  /// NOTE that num_channels is the size of the thresholds and hi_values
  // arrays and also the bytes per pixel in src_pix.
  void ThresholdRectToPix(Image src_pix, int num_channels, const std::vector<int> &thresholds,
                          const std::vector <int> &hi_values, Image *pix) const;

protected:
  /// Clone or other copy of the source Pix.
  /// The pix will always be PixDestroy()ed on destruction of the class.
  Image pix_;

  int image_width_;  ///< Width of source pix_.
  int image_height_; ///< Height of source pix_.
  int pix_channels_; ///< Number of 8-bit channels in pix_.
  int pix_wpl_;      ///< Words per line of pix_.
  // Limits of image rectangle to be processed.
  int scale_;         ///< Scale factor from original image.
  int yres_;          ///< y pixels/inch in source image.
  int estimated_res_; ///< Resolution estimate from text size.
  int rect_left_;
  int rect_top_;
  int rect_width_;
  int rect_height_;
};

} // namespace tesseract.

#endif // TESSERACT_CCMAIN_THRESHOLDER_H_
