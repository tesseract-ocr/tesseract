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

#include "genericvector.h"
#include "kdpair.h"
#include "statistc.h"

#include "include_gunit.h"

namespace {

const int kTestData[] = {2, 0, 12, 1, 1, 2, 10, 1, 0, 0, 0, 2, 0, 4, 1, 1};

class STATSTest : public testing::Test {
 public:
  void SetUp() {
    stats_.set_range(0, 16);
    for (int i = 0; i < ABSL_ARRAYSIZE(kTestData); ++i)
      stats_.add(i, kTestData[i]);
  }

  void TearDown() {}

  STATS stats_;
};

// Tests some basic numbers from the stats_.
TEST_F(STATSTest, BasicStats) {
  EXPECT_EQ(37, stats_.get_total());
  EXPECT_EQ(2, stats_.mode());
  EXPECT_EQ(12, stats_.pile_count(2));
}

// Tests the top_n_modes function.
TEST_F(STATSTest, TopNModes) {
  GenericVector<tesseract::KDPairInc<float, int> > modes;
  int num_modes = stats_.top_n_modes(3, &modes);
  EXPECT_EQ(3, num_modes);
  // Mode0 is 12 1 1 = 14 total count with a mean of 2 3/14.
  EXPECT_FLOAT_EQ(2.0f + 3.0f / 14, modes[0].key);
  EXPECT_EQ(14, modes[0].data);
  // Mode 1 is 2 10 1 = 13 total count with a mean of 5 12/13.
  EXPECT_FLOAT_EQ(5.0f + 12.0f / 13, modes[1].key);
  EXPECT_EQ(13, modes[1].data);
  // Mode 2 is 4 1 1 = 6 total count with a mean of 13.5.
  EXPECT_FLOAT_EQ(13.5f, modes[2].key);
  EXPECT_EQ(6, modes[2].data);
}

}  // namespace.
