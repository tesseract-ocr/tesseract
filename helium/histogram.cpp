// Copyright 2006 Google Inc.
// All Rights Reserved.
// Author: <renn@google.com> (Marius Renn)

// std includes
#include <math.h>
#include <stdio.h>

// Local includes
#include "debugging.h"
#include "histogram.h"
#include "helium_image.h"
#include "mathfunctions.h"

using namespace helium;

Histogram::Histogram()
  : size_(0),
    histogram_red_(new uint32[256]),
    histogram_green_(new uint32[256]),
    histogram_blue_(new uint32[256]),
    expected_(0),
    variance_(0) {
  memset(histogram_red_, 0, 256 * 4);
  memset(histogram_green_, 0, 256 * 4);
  memset(histogram_blue_, 0, 256 * 4);
}

void Histogram::AddImage(const Image& image) {
  ASSERT(histogram_red_ && histogram_green_ && histogram_blue_);

  // Fill histogram with values of image
  Color* image_ptr = image.data();
  for (unsigned i = 0; i < image.width() * image.height(); i++)
    AddColor(*(image_ptr++));
}

bool Histogram::Subtract(const Histogram& other) {
  bool valid = true;
  if (size_ < other.size_) {
    fprintf(stderr, "Warning: size %d - %d\n", size_, other.size_);
    valid = false;
  }
  for (unsigned i = 0; i < 256; i++) {
    // Make sure we can subtract these values
    if (!(histogram_red_[i] >= other.histogram_red_[i]) ||
        !(histogram_green_[i] >= other.histogram_green_[i]) ||
        !(histogram_blue_[i] >= other.histogram_blue_[i])) {
      fprintf(stderr, "Warning: HistogramSubtract "
              "[%d] -- R:%d-%d\tG:%d-%d\tB:%d-%d\n",
              i,
              histogram_red_[i], other.histogram_red_[i],
              histogram_green_[i], other.histogram_green_[i],
              histogram_blue_[i], other.histogram_blue_[i]);
      valid = false;
    }

    histogram_red_[i] -= other.histogram_red_[i];
    histogram_green_[i] -= other.histogram_green_[i];
    histogram_blue_[i] -= other.histogram_blue_[i];
    if (histogram_red_[i] < 0) histogram_red_[i] = 0;
    if (histogram_green_[i] < 0) histogram_green_[i] = 0;
    if (histogram_blue_[i] < 0) histogram_blue_[i] = 0;
  }
  size_ -= other.size_;
  if (size_ < 0) size_ = 0;
  return valid;
}

void Histogram::Done() {
  ASSERT(histogram_red_ && histogram_green_ && histogram_blue_);

  float exp_red, exp_green, exp_blue;
  CalculateExpected(exp_red, exp_green, exp_blue);
  CalculateVariance(exp_red, exp_green, exp_blue);

  // Delete histograms
  delete[] histogram_red_;
  delete[] histogram_green_;
  delete[] histogram_blue_;
  histogram_red_ = histogram_green_ = histogram_blue_ = NULL;
}

void Histogram::CalculateExpected(float& exp_red,
                                  float& exp_green,
                                  float& exp_blue) {
  exp_red = 0.0;
  exp_green = 0.0;
  exp_blue = 0.0;

  float n = static_cast<float>(size_);
  if (n <= 0.0) return;

  for (unsigned i = 0; i < 256; i++) {
    float v = static_cast<float>(i);

    // Calculate probabilities
    float p_r = static_cast<float>(histogram_red_[i]) / n;
    float p_g = static_cast<float>(histogram_green_[i]) / n;
    float p_b = static_cast<float>(histogram_blue_[i]) / n;

    // Sum weighted value
    exp_red += p_r * v;
    exp_green += p_g * v;
    exp_blue += p_b * v;
  }

  expected_ = MakeColor(static_cast<uint8>(exp_red),
                        static_cast<uint8>(exp_green),
                        static_cast<uint8>(exp_blue));
}

void Histogram::CalculateVariance(float exp_red,
                                  float exp_green,
                                  float exp_blue) {
  float n = static_cast<float>(size_);
  if (n <= 0.0) return;

  float r = 0.0, g = 0.0, b = 0.0;
  for (unsigned i = 0; i < 256; i++) {
    float v = static_cast<float>(i);

    // Calculate probabilities
    float p_r = static_cast<float>(histogram_red_[i]) / n;
    float p_g = static_cast<float>(histogram_green_[i]) / n;
    float p_b = static_cast<float>(histogram_blue_[i]) / n;

    // Sum square distances to expected value
    r += Square(v - exp_red) * p_r;
    g += Square(v - exp_green) * p_g;
    b += Square(v - exp_blue) * p_b;
  }

  variance_ = MakeColor(static_cast<int>(sqrt(r)),
                        static_cast<int>(sqrt(g)),
                        static_cast<int>(sqrt(b)));
}

Color Histogram::Percentile(uint8 percentile) const {
  return MakeColor(ChannelPercentile(histogram_red_, percentile),
                   ChannelPercentile(histogram_green_, percentile),
                   ChannelPercentile(histogram_blue_, percentile));
}

uint32 Histogram::ChannelPercentile(uint32* channel, uint8 percentile) const {
  ASSERT(histogram_red_ && histogram_green_ && histogram_blue_);
  ASSERT(percentile <= 100);

  float p = static_cast<float>(percentile) / 100.0;
  uint32 threshold = static_cast<uint32>(static_cast<float>(size_) * p);

  uint32 i = 0;
  for (uint32 sum = 0; sum < threshold; sum += channel[i++]);
  return (i == 0) ? 0 : i - 1;
}
