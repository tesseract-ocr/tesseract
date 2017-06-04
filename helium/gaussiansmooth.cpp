// Copyright 2006 Google Inc.
// All Rights Reserved.
// Author: <renn@google.com> (Marius Renn)

// Local includes
#include "color.h"
#include "debugging.h"
#include "gaussiansmooth.h"
#include "point.h"
#include "mathfunctions.h"

using namespace helium;

GaussianSmooth::GaussianSmooth(unsigned size) : 
  kernel_size_(size + 1), 
  gauss_sum_(0),
  kernel_(new int[size + 1]) {
  
  // Create gaussian mask by calculating the binomial coefficients. We will
  // later divide by the sum of these coefficients to obtain the output value.
  for (unsigned i = 0; i < kernel_size_; i ++) {
    int coeff = Binomi(size + 1, i + 1);
    kernel_[i] = coeff;
    gauss_sum_ += coeff;
  }
}

GaussianSmooth::~GaussianSmooth() {
  delete[] kernel_;
}

Image GaussianSmooth::Smooth(const Image& image) {
  ASSERT(kernel_);
  
  // Create intermediate and destination images
  Image temp(image.width(), image.height());
  Image dest(image.width(), image.height());
  ASSERT(temp.Valid() && dest.Valid());
  
  int s = kernel_size_ / 2;
  Color pixel;
  Point p;
  int r, g, b;
  int row = image.width();
  
  // Setup data pointers
  Color* src_ptr = image.Access(s, s);
  Color* temp_ptr = temp.Access(s, s);
  Color* dest_ptr = dest.Access(s, s);
  
  // First pass (horizontal smoothing)
  for (p.y = s; p.y < image.height() - s; p.y++) {
    for (p.x = s; p.x < image.width() - s; p.x++) {
      int r_gauss = 0;
      int g_gauss = 0;
      int b_gauss = 0;
      for (int i = 0; i < kernel_size_; i++) {
        pixel = *(src_ptr + i - s);
        ColorSeparate(pixel, r, g, b);
        r_gauss += r * kernel_[i];
        g_gauss += g * kernel_[i];
        b_gauss += b * kernel_[i];
      }
      r_gauss /= gauss_sum_;
      g_gauss /= gauss_sum_;
      b_gauss /= gauss_sum_;
      
      *(temp_ptr++) = MakeColor(r_gauss, g_gauss, b_gauss);
      src_ptr++;
    }
    src_ptr += s*2;
    temp_ptr += s*2;
  }
  
  // Second pass (vertical smoothing)
  temp_ptr = temp.Access(s, s);
  for (p.y = s; p.y < image.height() - s; p.y++) {
    for (p.x = s; p.x < image.width() - s; p.x++) {
      int r_gauss = 0;
      int g_gauss = 0;
      int b_gauss = 0;
      for (int i = 0; i < kernel_size_; i++) {
        pixel = *(temp_ptr + (i - s) * row);
        ColorSeparate(pixel, r, g, b);
        
        r_gauss += r * kernel_[i];
        g_gauss += g * kernel_[i];
        b_gauss += b * kernel_[i];
      }
      r_gauss /= gauss_sum_;
      g_gauss /= gauss_sum_;
      b_gauss /= gauss_sum_;
      
      *(dest_ptr++) = MakeColor(r_gauss, g_gauss, b_gauss);
      temp_ptr++;
    }
    dest_ptr += s*2;
    temp_ptr += s*2;
  }
  return dest;
}
