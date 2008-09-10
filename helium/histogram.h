// Copyright 2006 Google Inc.
// All Rights Reserved.
// Author: <renn@google.com> (Marius Renn)
//
// This file contains the Histogram class, which is used to store histogram
// information of color images. Internally Histogram uses three separate
// histograms: one for each color channel.
//
#ifndef HELIUM_HISTOGRAM_H__
#define HELIUM_HISTOGRAM_H__

#include "color.h"
#include "debugging.h"

namespace helium {

class Image;

// The histogram class contains 3 x 256 "bins", one for each value of each
// channel red, green, and blue. It is updated by adding a color to the
// histogram via AddColor(Color), or an entire image via
// AddImage(const Image&). After having added all the colors, a call to
// Done() calculates the expected color value, and the variance of each channel.
// Done() also deallocates the three internal histograms, so it is important
// that it is called exactly once. Obviously, you cannot add colors to the
// histogram, once Done() has been called.
// Histograms may be copied, as long as Done() is called only once. (An
// assertion will fail, if it is called more than once).
class Histogram {
  public:
    Histogram();

    // Update the values in the histogram to reflect an addition of the
    // specified color. You must not call this method after calling Done()!
    inline void AddColor(Color color) {
      ASSERT(histogram_red_ && histogram_green_ && histogram_blue_);
      histogram_red_[Red(color)]++;
      histogram_green_[Green(color)]++;
      histogram_blue_[Blue(color)]++;
      size_++;
    }

    // Update the valeus in the histogram to reflect an addition of every
    // color in the specified image. You must not call this method after
    // calling Done()!
    void AddImage(const Image& image);

    // Call this when no more colors will be added to the histogram. This
    // method then calculates the expected color and variance, and deallocates
    // the histogram data for the color channels. Done() must be called exactly
    // once for each Histogram!
    void Done();

    // Returns true if Done() was called already, and the expected color and
    // variance are valid. This is done by checking if the internal histograms
    // have been deallocated already.
    inline bool IsDone() const {
      return (!histogram_red_ && !histogram_green_ && !histogram_blue_);
    }

    // Returns the expected color of the histogram. You must have called Done()
    // before calling this.
    inline Color Expected() const {
      return expected_;
    }

    // Returns the variance of the histogram. You must have called Done()
    // before calling this.
    inline Color Variance() const {
      return variance_;
    }

    // Returns the number of color values that have contributed to the
    // histogram.
    inline unsigned size() const {
      return size_;
    }

    // Subtract another histogram from the receiver. This only produces
    // meaningful results, if the colors that made up the other histogram, are
    // a subset of the colors that made up the receiver. This is used for
    // processing holes in traces.
    bool Subtract(const Histogram& other);

    // For each color channel, this method calculates the value that is greater
    // than )or equal to) the specified percentage of values in that channel.
    // The three values are combined to a color, and returned. percentile
    // must be between 0 and 100.
    Color Percentile(uint8 percentile) const;

  private:
    // The number of values that have contributed to this histogram
    unsigned size_;

    // Three histograms, one for each color channel.
    uint32* histogram_red_;
    uint32* histogram_green_;
    uint32* histogram_blue_;

    // The expected color and the variance of each channel.
    Color expected_, variance_;

    // Calculates the expected value, and stores it in expected_. Returns the
    // exact values in the three arguments exp_red, exp_green, exp_blue. These
    // are required for calculating the variance.
    void CalculateExpected(float& exp_red, float& exp_green, float& exp_blue);

    // Calculates the variance, and stores it in variance_. The exact values
    // of the expected color are passed in exp_red, exp_green, exp_blue.
    void CalculateVariance(float exp_red, float exp_green, float exp_blue);

    // Returns the value that is greater than (or equal to) the specified
    // percentage of values in the specified channel.
    uint32 ChannelPercentile(uint32* channel, uint8 percentile) const;
};

} // namespace

#endif // HELIUM_HISTOGRAM_H__
