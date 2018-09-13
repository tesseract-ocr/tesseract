//

#include "tesseract/ccutil/qrsequence.h"

#include <algorithm>
#include <vector>

namespace {

class TestableQRSequenceGenerator : public QRSequenceGenerator {
 public:
  explicit TestableQRSequenceGenerator(const int& N) : QRSequenceGenerator(N) {}
  // Overriding scope for testing
  using QRSequenceGenerator::GetBinaryReversedInteger;
};

// Verifies binary inversion for a small range.
TEST(QRSequenceGenerator, GetBinaryReversedInteger) {
  const int kRangeSize = 8;
  TestableQRSequenceGenerator generator(kRangeSize);
  int reversed_vals[kRangeSize] = { 0, 4, 2, 6, 1, 5, 3, 7};
  for (int i = 0; i < kRangeSize; ++i)
    EXPECT_EQ(reversed_vals[i], generator.GetBinaryReversedInteger(i));
}

// Trivial test fixture for a parameterized test.
class QRSequenceGeneratorTest : public ::testing::TestWithParam<int> {
};

TEST_P(QRSequenceGeneratorTest, GeneratesValidSequence) {
  const int kRangeSize = GetParam();
  TestableQRSequenceGenerator generator(kRangeSize);
  std::vector<int> vals(kRangeSize);
  CycleTimer timer;
  timer.Restart();
  for (int i = 0; i < kRangeSize; ++i)
    vals[i] = generator.GetVal();
  LOG(INFO) << kRangeSize << "-length sequence took " << timer.Get() * 1e3
            << "ms";
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
INSTANTIATE_TEST_CASE_P(RangeTest, QRSequenceGeneratorTest,
                        ::testing::Values(2, 7, 8, 9, 16, 1e2, 1e4, 1e6));
}  // namespace
