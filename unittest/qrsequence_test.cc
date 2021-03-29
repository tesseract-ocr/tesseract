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

#include <algorithm>
#include <vector>

#include "cycletimer.h"
#include "include_gunit.h"
#include "log.h"
#include "qrsequence.h"

namespace tesseract {

class TestableQRSequenceGenerator : public QRSequenceGenerator {
public:
  explicit TestableQRSequenceGenerator(const int &N) : QRSequenceGenerator(N) {}
  // Overriding scope for testing
  using QRSequenceGenerator::GetBinaryReversedInteger;
};

// Verifies binary inversion for a small range.
TEST(QRSequenceGenerator, GetBinaryReversedInteger) {
  const int kRangeSize = 8;
  TestableQRSequenceGenerator generator(kRangeSize);
  int reversed_vals[kRangeSize] = {0, 4, 2, 6, 1, 5, 3, 7};
  for (int i = 0; i < kRangeSize; ++i) {
    EXPECT_EQ(reversed_vals[i], generator.GetBinaryReversedInteger(i));
  }
}

// Trivial test fixture for a parameterized test.
class QRSequenceGeneratorTest : public ::testing::TestWithParam<int> {
protected:
  void SetUp() override {
    std::locale::global(std::locale(""));
  }
};

TEST_P(QRSequenceGeneratorTest, GeneratesValidSequence) {
  const int kRangeSize = GetParam();
  TestableQRSequenceGenerator generator(kRangeSize);
  std::vector<int> vals(kRangeSize);
  CycleTimer timer;
  timer.Restart();
  for (int i = 0; i < kRangeSize; ++i) {
    vals[i] = generator.GetVal();
  }
  LOG(INFO) << kRangeSize << "-length sequence took " << timer.GetInMs() << "ms";
  // Sort the numbers to verify that we've covered the range without repetition.
  std::sort(vals.begin(), vals.end());
  for (int i = 0; i < kRangeSize; ++i) {
    EXPECT_EQ(i, vals[i]);
    if (i != vals[i]) {
      LOG(INFO) << "Aborting remaining comparisons";
      break;
    }
  }
}

// Run a parameterized test using the following range sizes.
INSTANTIATE_TEST_SUITE_P(RangeTest, QRSequenceGeneratorTest,
                         ::testing::Values(2, 7, 8, 9, 16, 1e2, 1e4, 1e6));
} // namespace tesseract
