// Copyright 2006 Google Inc.
// All Rights Reserved.
// Author: <renn@google.com> (Marius Renn)
// Author: <dsl@google.com> (Dar-Shyang Lee) Basically completely rewritten
//

// Local includes
#include <math.h>
#include "debugging.h"
#include "histogram.h"
#include "image.h"
#include "graymap.h"
#include "imageenhancer.h"

using namespace helium;

class IntegralMatrix {
 public:
  IntegralMatrix() : acc_(NULL), sqr_(NULL), width_(0), height_(0) {}

  explicit IntegralMatrix(const GrayMap& img)
    : acc_(NULL), sqr_(NULL), width_(0), height_(0) {
    Init(img);
  }

  // Prepares integral matrix for a gray image.  Must be called for each image.
  void Init(const GrayMap& img) {
    Release();
    width_ = img.width();
    height_ = img.height();
    uint8 *data = img.data();
    acc_ = new double[width_*height_];
    sqr_ = new double[width_*height_];
    double *accpt = acc_;
    double *sqrpt = sqr_;
    for (int i = 0; i < height_; ++i) {
      for (int j = 0; j < width_; ++j) {
        double sum = static_cast<double>(*data++);           // img[i,j];
        double sqr = sum*sum;                                // img[i,j]^2

        if (i > 0) sum += *(accpt-width_);                   // +acc[i-1,j]
        if (j > 0) sum += *(accpt-1);                        // +acc[i,j-1]
        if (i > 0 && j > 0) sum -= *(accpt-width_-1);        // -acc[i-1,j-1]
        *accpt++ = sum;
        // acc[i,j] = img[i,j]+acc[i-1,j]+acc[i,j-1]-acc[i-1,j-1]

        if (i > 0) sqr += *(sqrpt-width_);                   // sqr[i-1,j]
        if (j > 0) sqr += *(sqrpt-1);                        // sqr[i,j-1]
        if (i > 0 && j > 0) sqr -= *(sqrpt-width_-1);        // -sqr[i-1,j-1]
        *sqrpt++ = sqr;
      }
    }
  }

  void Release() {
    delete [] acc_;
    delete [] sqr_;
    acc_ = NULL;
    sqr_ = NULL;
    width_ = 0;
    height_ = 0;
  }

  ~IntegralMatrix() {
     Release();
  }

  // return a[i,j] with special handle for out-of-range points.
  double GetValue(const double* a, int x, int y) {
    if (x < 0 || y < 0) return 0;     // negative coordinates are filled with 0
    if (y >= height_) y = height_ - 1;
    if (x >= width_) x = width_ - 1;  // clip to image on oversized regions
    return a[y*width_+x];
  }

  // Returns the sum of pixels in window [x1,y1] to [x2,y2], inclusive.
  // and the size of the clipped window.
  double GetWindow(const double* a, int x1, int y1, int x2, int y2, int *size) {
    double sum = GetValue(a, x2, y2) + GetValue(a, x1-1, y1-1)
                 - GetValue(a, x1-1, y2) - GetValue(a, x2, y1-1);
    if (x1 < 0) x1 = 0;
    if (y1 < 0) y1 = 0;
    if (x2 >= width_) x2 = width_ - 1;
    if (y2 >= height_) y2 = height_ - 1;
    *size = (x2 - x1 + 1) * (y2 - y1 + 1);
    return sum;
  }

  // Returns the sum of pixel values in the window, and the actual area size
  // taking boundary situation into consideration.
  double GetWindowSum(int x1, int y1, int x2, int y2, int *size) {
    return GetWindow(acc_, x1, y1, x2, y2, size);
  }

  // Returns the sum of square of pixel values in the window, and window size.
  double GetWindowSumSqr(int x1, int y1, int x2, int y2, int *size) {
    return GetWindow(sqr_, x1, y1, x2, y2, size);
  }

  // Returns the mean and variance of pixel values for the window.
  int GetWindowMeanVar(int x1, int y1, int x2, int y2,
                       double *mean, double *var) {
    int area, area2;
    double sumx = GetWindowSum(x1, y1, x2, y2, &area);
    double sumx2 = GetWindowSumSqr(x1, y1, x2, y2, &area2);
    ASSERT(area == area2);
    double u = 0.0;
    double v = 0.0;
    if (area > 0) {
      u = sumx/static_cast<double>(area);
      v = sumx2/static_cast<double>(area) - u*u;
      ASSERT(v >= 0);
    }
    *mean = u;
    *var = v;
    return area;
  }

  const double* acc() { return acc_; }
  const double* sqr() { return sqr_; }

  GrayMap GetImage(const double* buf) {
    const double *pt = buf;
    double max = -1;
    double min = -1;
    for (int i = 0; i < height_; ++i) {
      for (int j = 0; j < width_; ++j, ++pt) {
        if (*pt > max) max = *pt;
        if (min < 0 || *pt < min) min = *pt;
      }
    }
    double s = (max - min)/256;
    if (s < 1) s = 1.0;
    pt = buf;
    GrayMap img(width_, height_);
    uint8 *data = img.data();
    for (int i = 0; i < img.height(); ++i) {
      for (int j = 0; j < img.width(); ++j) {
        *data++ = static_cast<uint8>((*pt++ - min) / s);
      }
    }
    return img;
  }

 private:
  double *acc_;    // acc(i,j) = sum I(y,x) for y<=i, x<=j, row-major stored
  double *sqr_;    // sqr(i,j) = sum I(y,x)^2 for y<=i, x<=j
  int width_;
  int height_;
};


// Return mean as is
uint8 ImageEnhancer::Func_Mean(uint8 p, double mean, double stddev) {
  return static_cast<uint8>(mean);
}
// Return std, slightly scaled back so they fit in range
uint8 ImageEnhancer::Func_StdDev(uint8 p, double mean, double stddev) {
  stddev *= 0.5;
  if (stddev > 255) stddev = 255;
  return static_cast<uint8>(stddev);
}
// threshold base on mean
uint8 ImageEnhancer::Func_ThreshOnMean(uint8 p, double mean, double stddev) {
  return (p > mean) ? 255 : 0;
}
// conservative thresholding, elevates FG while preserving BG
uint8 ImageEnhancer::Func_EnhanceFG(uint8 p, double mean, double stddev) {
  return (p > mean + 0.5*stddev) ? 255 : p;
}
// conservative binarization, try to capture 95% of BG
uint8 ImageEnhancer::Func_BinarizeFG(uint8 p, double mean, double stddev) {
  return (p > mean + 3*stddev) ? 255 : 0;
}
// Try to zero-out 95% of BG, but keeps FG value for tracing
uint8 ImageEnhancer::Func_SuppressBG(uint8 p, double mean, double stddev) {
  if (p < mean + 3*stddev) return 0;
  double x = (256/8) * (p - mean)/stddev;  // stretch 3..8 stddev over 0..255
  if (x > 255) x = 255;
  return static_cast<uint8>(x);
}


void ImageEnhancer::ApplyMask(const GrayMap& mask, GrayMap& src) {
  uint8 *p1 = src.data();
  uint8 *p2 = mask.data();
  for (int y = 0; y < src.height(); ++y) {
    for (int x = 0; x < src.width(); ++x) {
      int value = *p1 * *p2;
      if (value > 255) value = 255;
      *p1++ = static_cast<uint8>(value);
      p2++;
    }
  }
}


// Apply local contrast enhancement using a window size whose half-width is
// defined by ws and hs.  The modification is done in-place on input image.
void ImageEnhancer::LocalContrast(GrayMap& src, int ws, int hs,
                                  int fg_thresh, FuncPMV funcpt) {
  IntegralMatrix im;
  if (fg_thresh > 0) {
    // Use fg_thresh to mask out strong FG regions, and compute mean/var
    // over only the BG region.  For simplicity, we "zero-out" those FG
    // pixels instead of marking them as "don't-care" state.  So there
    // is a bias towards 0 when computing the mean/var.
    GrayMap mask;
    mask.Copy(src);
    Binarize(mask, fg_thresh, -1, 0);  // keep BG values, zero out FG
    im.Init(mask);    // compute mean/var over background
  } else {
    im.Init(src);
  }

  uint8 *data = src.data();
  for (int y = 0; y < src.height(); ++y) {
    for (int x = 0; x < src.width(); ++x) {
      double mean, var;
      int area = im.GetWindowMeanVar(x-ws, y-hs, x+ws, y+hs, &mean, &var);
      ASSERT(area > 0);  // should never get 0 area by our usage
      double stddev = (var <= 1.0) ? 1.0 : sqrt(var);
      *data = (*funcpt)(*data, mean, stddev);
      data++;
    }
  }
}

// Performs independent pixel operation in-place using given threshold.
// For pixels whose value is below the threshold, the minvalue is used.
// If minvalue==-1, the original value is used.  Similarly for maxvalue.
void ImageEnhancer::Binarize(GrayMap& src,
                             int threshold,
                             int minvalue,
                             int maxvalue) {
  uint8 *data = src.data();
  for (int y = 0; y < src.height(); ++y) {
    for (int x = 0; x < src.width(); ++x) {
      uint8 value;
      if (*data < threshold) {  // use minvalue rule
        value = (minvalue < 0) ? *data : minvalue;
      } else {                  // use maxvalue rule
        value = (maxvalue < 0) ? *data : maxvalue;
      }
      *data++ = value;
    }
  }
}


void ImageEnhancer::GetRange(const GrayMap& img, uint8 *min_v, uint8 *max_v) {
  uint8 *data = img.data();
  *min_v = 255;
  *max_v = 0;
  for (int i = 0; i < img.height(); ++i) {
    for (int j = 0; j < img.width(); ++j) {
      if (*data > *max_v) *max_v = *data;
      if (*data < *min_v) *min_v = *data;
      data++;
    }
  }
}

void ImageEnhancer::EnhanceGray(GrayMap& src, int min_gray_range) {
  uint8 min_v, max_v;
  GetRange(src, &min_v, &max_v);
  int range = max_v - min_v + 1;
  if (range > min_gray_range) return;

  uint8 *src_ptr = src.data();
  int img_size = src.width() * src.height();
  for (int i = 0; i < img_size; ++i) {
    int gray = ((*src_ptr - min_v) << 8) / range;
    if (gray > 255) gray = 255;
    *src_ptr++ = static_cast<uint8>(gray);
  }
}

void ImageEnhancer::EnhanceColors(Image& img, int min_color_range) {
  Color min_color, max_color;  // at lower/upper 5% of channel histogram
  Scan(img, min_color, max_color);
  if (Average(ColorDifference(max_color, min_color)) > min_color_range) return;

  // Calculate how far to offset the colors (black-point)
  int offset_r = -Red(min_color);
  int offset_g = -Green(min_color);
  int offset_b = -Blue(min_color);
  // Calculate how much to stretch the colors (white-point)
  int range_r = Red(max_color) - Red(min_color) + 1;
  int range_g = Green(max_color) - Green(min_color) + 1;
  int range_b = Blue(max_color) - Blue(min_color) + 1;
  ASSERT(range_r > 0 && range_g > 0 && range_b > 0);

  Color* img_ptr = img.data();
  int img_size = img.width() * img.height();
  for (unsigned i = 0; i < img_size; i++) {
    int r, g, b;
    ColorSeparate(*img_ptr, r, g, b);
    r += offset_r;
    g += offset_g;
    b += offset_b;
    r = (r << 8) / range_r;
    g = (g << 8) / range_g;
    b = (b << 8) / range_b;
    ChannelLimit(r, g, b);
    *img_ptr++ = MakeColor(r, g, b);
  }
}

void ImageEnhancer::Scan(const Image& image, Color& minc, Color& maxc) {
  // Create Histogram
  Histogram histogram;
  histogram.AddImage(image);

  // Get 5% and 95% percentile
  minc = histogram.Percentile(5);
  maxc = histogram.Percentile(95);

  // Don't need this anymore
  histogram.Done();
}
