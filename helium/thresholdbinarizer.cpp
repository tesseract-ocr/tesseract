// Copyright 2006 Google Inc.
// All Rights Reserved.
// Author: <renn@google.com> (Marius Renn)

#include "box.h"
#include "helium_image.h"
#include "textareas.h"
#include "thresholdbinarizer.h"

using namespace helium;

// The static method BinarizeRect implicitly expects a mask to be allocated
// with 2*kMargin padding, but does not expose kMargin.  It is cleaner to
// let the calling function pad the bounding box instead.
const unsigned kMargin = 0;

ThresholdBinarizer::ThresholdBinarizer(const Image& image, 
                                       const TextAreas& text_areas) 
  : Binarizer(image),
    text_areas_(text_areas),
    index_(0) {
}

bool ThresholdBinarizer::GetNextMask(Mask& out_mask, Box& out_bounds) {
  if (index_ >= text_areas_.Size()) return false;
  
  out_bounds = text_areas_.boxes().ValueAt(index_);
  out_mask = Mask(out_bounds.width() + kMargin * 2, 
                  out_bounds.height() + kMargin * 2);
  
  out_mask.Clear();
  
  BinarizeRect(image_, out_mask, out_bounds);

  index_++;
  
  return true;
}

void ThresholdBinarizer::BinarizeRect(const Image& image,
                                      Mask& mask, 
                                      const Box& bounds) {
  // Get threshold
  int threshold = ThresholdValue(image, bounds);
  bool hi_value = (threshold < 0);
  if (threshold < 0) threshold = -threshold;
  
  // Binarize by threshold
  Binarize(image, mask, bounds, threshold, hi_value);
}

// Taken from the Tesseract API and modified.
int ThresholdBinarizer::ThresholdValue(const Image& image, const Box& bounds) {
  int histogram[256];
  memset(histogram, 0, sizeof(histogram));
  
  Color* image_ptr = image.Access(bounds.origin());
  int y_step = image.width() - bounds.width();
  for (int y = 0; y < bounds.height(); ++y) {
    for (int x = 0; x < bounds.width(); ++x) {
      ++histogram[Average(*image_ptr)];
      ++image_ptr;
    }
    image_ptr += y_step;
  }
  
  int H = 0;
  double mu_T = 0.0;
  for (int i = 0; i < 256; ++i) {
    H += histogram[i];
    mu_T += i * histogram[i];
  }

  // Now maximize sig_sq_B over t.
  double best_sig_sq_B = 0.0;
  int best_t = -1;
  double best_omega_0 = 0.0;
  double omega_0, omega_1, mu_0, mu_1, mu_t, sig_sq_B;
  omega_0 = 0.0;
  mu_t = 0.0;
  for (int t = 0; t < 255; ++t) {
    omega_0 += static_cast<double>(histogram[t]);
    mu_t += t * static_cast<double>(histogram[t]);
    if (omega_0 == 0.0)
      continue;
    omega_1 = H - omega_0;
    mu_0 = mu_t / omega_0;
    mu_1 = (mu_T - mu_t) / omega_1;
    double sqdiff_0 = mu_0 - mu_T/H;
    double sqdiff_1 = mu_1 - mu_T/H;
    sqdiff_0 *= sqdiff_0;
    sqdiff_1 *= sqdiff_1;
    sig_sq_B = omega_0 * sqdiff_0 + omega_1 * sqdiff_1;
    if (best_t < 0 || sig_sq_B > best_sig_sq_B) {
      best_sig_sq_B = sig_sq_B;
      best_t = t;
      best_omega_0 = omega_0;
    }
  }
  return best_omega_0 > H / 2.0 ? -best_t : best_t;
}

void ThresholdBinarizer::Binarize(const Image& image, 
                                  Mask& mask,
                                  const Box& bounds,
                                  int threshold,
                                  bool hi_value) {
  Color* img_ptr = image.Access(bounds.origin());
  bool* mask_ptr = mask.Access(kMargin, kMargin);
  
  int y_step_img = image.width() - bounds.width();
  int y_step_mask = mask.width() - bounds.width();
  for (int y = 0; y < bounds.height(); ++y) {
    for (int x = 0; x < bounds.width(); ++x) {
      if (Average(*img_ptr) > threshold) 
        *(mask_ptr++) = hi_value;
      else
        *(mask_ptr++) = !hi_value;
      ++img_ptr;
    }
    img_ptr += y_step_img;
    mask_ptr += y_step_mask;
  }
}
