// (C) Copyright 2017, Google Inc.
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "kdpair.h"
#include "statistc.h"

#include "include_gunit.h"

#include <algorithm> // for std::max
#include <cstdlib>   // for std::abs

namespace tesseract {

const int kTestData[] = {2, 0, 12, 1, 1, 2, 10, 1, 0, 0, 0, 2, 0, 4, 1, 1};

class STATSTest : public testing::Test {
public:
  void SetUp() override {
    std::locale::global(std::locale(""));
    stats_.set_range(0, 15);
    for (size_t i = 0; i < countof(kTestData); ++i) {
      stats_.add(i, kTestData[i]);
    }
  }

  void TearDown() override {}

  STATS stats_;
};

// Tests some basic numbers from the stats_.
TEST_F(STATSTest, BasicStats) {
  EXPECT_EQ(37, stats_.get_total());
  EXPECT_EQ(2, stats_.mode());
  EXPECT_EQ(12, stats_.pile_count(2));
}

TEST_F(STATSTest, InitStats) {
  STATS stats;
  EXPECT_EQ(0, stats.get_total());
  EXPECT_EQ(0, stats.mode());
  EXPECT_EQ(0, stats.pile_count(2));
}

// Tests the top_n_modes function.
TEST_F(STATSTest, TopNModes) {
  std::vector<tesseract::KDPairInc<float, int> > modes;
  int num_modes = stats_.top_n_modes(3, modes);
  EXPECT_EQ(3, num_modes);
  // Mode0 is 12 1 1 = 14 total count with a mean of 2 3/14.
  EXPECT_FLOAT_EQ(2.0f + 3.0f / 14, modes[0].key());
  EXPECT_EQ(14, modes[0].data());
  // Mode 1 is 2 10 1 = 13 total count with a mean of 5 12/13.
  EXPECT_FLOAT_EQ(5.0f + 12.0f / 13, modes[1].key());
  EXPECT_EQ(13, modes[1].data());
  // Mode 2 is 4 1 1 = 6 total count with a mean of 13.5.
  EXPECT_FLOAT_EQ(13.5f, modes[2].key());
  EXPECT_EQ(6, modes[2].data());
}

// Tests that smooth applies a triangular filter of the given height, also
// when the factor is larger than the histogram.
TEST_F(STATSTest, Smooth) {
  const int kNumBuckets = countof(kTestData);
  for (int factor : {2, 3, 7, kNumBuckets, 3 * kNumBuckets}) {
    STATS stats(0, kNumBuckets - 1);
    for (int i = 0; i < kNumBuckets; ++i) {
      stats.add(i, kTestData[i]);
    }
    stats.smooth(factor);
    int total = 0;
    for (int i = 0; i < kNumBuckets; ++i) {
      int expected = 0;
      for (int k = 0; k < kNumBuckets; ++k) {
        expected += kTestData[k] * std::max(factor - std::abs(i - k), 0);
      }
      EXPECT_EQ(expected, stats.pile_count(i)) << "factor " << factor << ", bucket " << i;
      total += expected;
    }
    EXPECT_EQ(total, stats.get_total()) << "factor " << factor;
  }
}

// Tests that top_n_modes copes with a big histogram that has many peaks.
TEST_F(STATSTest, TopNModesManyPeaks) {
  const int kNumPeaks = 30000;
  const int kPeakSpacing = 8;
  STATS stats(0, kNumPeaks * kPeakSpacing);
  for (int i = 0; i < kNumPeaks; ++i) {
    stats.add(i * kPeakSpacing, 1);
  }
  // Widen one peak, so it has the biggest total, but not the biggest pile.
  stats.add(100 * kPeakSpacing + 1, 1);
  std::vector<tesseract::KDPairInc<float, int> > modes;
  int num_modes = stats.top_n_modes(3, modes);
  EXPECT_EQ(3, num_modes);
  EXPECT_FLOAT_EQ(100 * kPeakSpacing + 0.5f, modes[0].key());
  EXPECT_EQ(2, modes[0].data());
  // Of the equal peaks, the first ones are kept.
  EXPECT_FLOAT_EQ(0.0f, modes[1].key());
  EXPECT_EQ(1, modes[1].data());
  EXPECT_FLOAT_EQ(1.0f * kPeakSpacing, modes[2].key());
  EXPECT_EQ(1, modes[2].data());
}

} // namespace tesseract
