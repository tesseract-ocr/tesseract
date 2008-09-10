// Copyright 2006 Google Inc.
// All Rights Reserved.
// Author: <renn@google.com> (Marius Renn)
//
// This file declares the ImageEnhancer class, used for broadening the color
// range of images, that have a low contrast.
//
#ifndef HELIUM_IMAGEENHANCER_H__
#define HELIUM_IMAGEENHANCER_H__

#include "color.h"

namespace helium {

class Image;

// Enhancing an image with ImageEnhancer is done in two phases. First, the
// 5 and 95 percentiles of the image's histogram are calculated in the scan
// phase. If the image requires enhancement (use EnhancementNeeded() to obtain
// a rating of how much enhancement is required), the white-point and
// black-point of the image are set to their extreme values, thus broadening
// the overall color range, in the second phase.
class ImageEnhancer {
  public:
    // Function pointer that would be an argument to LocalContrast().
    // The 3 arguments are gray value, mean and stddev at given pixel.
    typedef uint8(*FuncPMV)(uint8, double, double);

    // returns mean at neighborhood of x
    static uint8 Func_Mean(uint8, double, double);
    // returns standard deviation of block at x
    static uint8 Func_StdDev(uint8, double, double);
    // binarize pixel at x using mean
    static uint8 Func_ThreshOnMean(uint8, double, double);
    // enhances foreground pixels, leave BG the same
    static uint8 Func_EnhanceFG(uint8, double, double);
    // suppress background pixel values, leaving FG the same
    static uint8 Func_SuppressBG(uint8, double, double);
    // like EnhanceFG, but binarizes pixel at x
    static uint8 Func_BinarizeFG(uint8, double, double);

    static const int kDefaultEdgeThresh = 16;
    static const int kDefaultBlockHalfWidth = 50;
    static const int kDefaultBlockHalfHeight = 50;
    static const int kDefaultMinColorRange = 128;

    // Independently stretch the color channels if average range is less than
    // the given threshold.
    static void EnhanceColors(Image& image, int min_color_ragne);

    // Enhance local contrast using mean/var computed in (2*ws x 2*hs) window.
    // If fg_thresh is given, pixels above the threshold will be excluded from
    // mean/var computation.  Funcpt selects the final pixel value function.
    // One of the predefined Func_*() or a user-defined function can be used.
    // Using fg_thresh=0 and Func_ThreshOnMean implements NiBlack binarization.
    static void LocalContrast(GrayMap& src, int ws, int hs,
                              int fg_thresh, FuncPMV funcpt);

    static void LocalContrast(GrayMap& src, int ws, int hs) {
      LocalContrast(src, ws, hs, kDefaultEdgeThresh, &Func_SuppressBG);
    }
    // Empirically determined best parameters for edge map enhancement
    static void LocalContrast(GrayMap& src) {
      LocalContrast(src, kDefaultBlockHalfWidth, kDefaultBlockHalfHeight,
                    kDefaultEdgeThresh, &Func_SuppressBG);
    }

    // Apply rank filter with specified window size on given graymap and
    // return the new image.  The size is specified by half-width and half
    // height that is added around each pixel.  The rank is a value between
    // 0..1 representing the fraction of pixels in the neighborhood with
    // smaller values.  (See leptonica for details.)
    static GrayMap RankFilterGray(GrayMap& map,
                                  int half_width, int half_height,
                                  float rank_ratio);

    // Implements a non-linear Niblack algorithm to produce a ternary valued
    // component label map based on specified background window size and
    // bg-to-fg window size ratio.  Parameter std_range controls thresholding.
    // Values within std_range of normalized distance will be labeled as
    // don't care.
    static void NLNiblack(GrayMap& src, int bg_half_width, int bg_half_height,
                          int bgfg_ratio, int min_fg_sdev_value,
                          float fg_sdev_range, float fg_sdev_rank);

    static void Binarize(GrayMap& src, int threshold, int minval, int maxval);

    static void ApplyMask(const GrayMap& mask, GrayMap& src);

    static void EnhanceGray(GrayMap& img, int min_gray_range);

    static void GetRange(const GrayMap& src, uint8 *min_v, uint8 *max_v);

  private:
    // Sequentially scan through the image to extract histogram information,
    // that is required for enhancing the image color range.
    static void Scan(const Image& image, Color& min_color, Color& max_color);
};

} // namespace

#endif  // HELIUM_IMAGEENHANCER_H__
