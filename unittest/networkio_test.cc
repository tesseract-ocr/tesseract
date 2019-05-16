#include "tesseract/lstm/networkio.h"
#include "tesseract/lstm/stridemap.h"

using tesseract::FD_BATCH;
using tesseract::FD_HEIGHT;
using tesseract::FD_WIDTH;
using tesseract::FlexDimensions;
using tesseract::NetworkIO;
using tesseract::StrideMap;

namespace {

class NetworkioTest : public ::testing::Test {
 protected:
  void SetUp() override {
    std::locale::global(std::locale(""));
  }

  // Sets up an Array2d object of the given size, initialized to increasing
  // values starting with start.
  std::unique_ptr<Array2D<int>> SetupArray(int ysize, int xsize, int start) {
    std::unique_ptr<Array2D<int>> a(new Array2D<int>(ysize, xsize));
    int value = start;
    for (int y = 0; y < ysize; ++y) {
      for (int x = 0; x < xsize; ++x) {
        (*a)(y, x) = value++;
      }
    }
    return a;
  }
  // Sets up a NetworkIO with a batch of 2 "images" of known values.
  void SetupNetworkIO(NetworkIO* nio) {
    std::vector<std::unique_ptr<Array2D<int>>> arrays;
    arrays.push_back(SetupArray(3, 4, 0));
    arrays.push_back(SetupArray(4, 5, 12));
    std::vector<std::pair<int, int>> h_w_sizes;
    for (int i = 0; i < arrays.size(); ++i) {
      h_w_sizes.emplace_back(arrays[i].get()->height(),
                             arrays[i].get()->width());
    }
    StrideMap stride_map;
    stride_map.SetStride(h_w_sizes);
    nio->ResizeToMap(true, stride_map, 2);
    // Iterate over the map, setting nio's contents from the arrays.
    StrideMap::Index index(stride_map);
    do {
      int value = (*arrays[index.index(FD_BATCH)])(index.index(FD_HEIGHT),
                                                   index.index(FD_WIDTH));
      nio->SetPixel(index.t(), 0, 128 + value, 0.0f, 128.0f);
      nio->SetPixel(index.t(), 1, 128 - value, 0.0f, 128.0f);
    } while (index.Increment());
  }
};

// Tests that the initialization via SetPixel works and the resize correctly
// fills with zero where image sizes don't match.
TEST_F(NetworkioTest, InitWithZeroFill) {
  NetworkIO nio;
  nio.Resize2d(true, 32, 2);
  int width = nio.Width();
  for (int t = 0; t < width; ++t) {
    nio.SetPixel(t, 0, 0, 0.0f, 128.0f);
    nio.SetPixel(t, 1, 0, 0.0f, 128.0f);
  }
  // The initialization will wipe out all previously set values.
  SetupNetworkIO(&nio);
  nio.ZeroInvalidElements();
  StrideMap::Index index(nio.stride_map());
  int next_t = 0;
  int pos = 0;
  do {
    int t = index.t();
    // The indexed values just increase monotonically.
    int value = nio.i(t)[0];
    EXPECT_EQ(value, pos);
    value = nio.i(t)[1];
    EXPECT_EQ(value, -pos);
    // When we skip t values, the data is always 0.
    while (next_t < t) {
      EXPECT_EQ(nio.i(next_t)[0], 0);
      EXPECT_EQ(nio.i(next_t)[1], 0);
      ++next_t;
    }
    ++pos;
    ++next_t;
  } while (index.Increment());
  EXPECT_EQ(pos, 32);
  EXPECT_EQ(next_t, 40);
}

// Tests that CopyWithYReversal works.
TEST_F(NetworkioTest, CopyWithYReversal) {
  NetworkIO nio;
  SetupNetworkIO(&nio);
  NetworkIO copy;
  copy.CopyWithYReversal(nio);
  StrideMap::Index index(copy.stride_map());
  int next_t = 0;
  int pos = 0;
  std::vector<int> expected_values = {
      8,  9,  10, 11, 4,  5,  6,  7,  0,  1,  2,  3,  27, 28, 29, 30,
      31, 22, 23, 24, 25, 26, 17, 18, 19, 20, 21, 12, 13, 14, 15, 16};
  do {
    int t = index.t();
    // The indexed values match the expected values.
    int value = copy.i(t)[0];
    EXPECT_EQ(value, expected_values[pos]);
    value = copy.i(t)[1];
    EXPECT_EQ(value, -expected_values[pos]);
    // When we skip t values, the data is always 0.
    while (next_t < t) {
      EXPECT_EQ(copy.i(next_t)[0], 0) << "Failure t = " << next_t;
      EXPECT_EQ(copy.i(next_t)[1], 0) << "Failure t = " << next_t;
      ++next_t;
    }
    ++pos;
    ++next_t;
  } while (index.Increment());
  EXPECT_EQ(pos, 32);
  EXPECT_EQ(next_t, 40);
}

// Tests that CopyWithXReversal works.
TEST_F(NetworkioTest, CopyWithXReversal) {
  NetworkIO nio;
  SetupNetworkIO(&nio);
  NetworkIO copy;
  copy.CopyWithXReversal(nio);
  StrideMap::Index index(copy.stride_map());
  int next_t = 0;
  int pos = 0;
  std::vector<int> expected_values = {
      3,  2,  1,  0,  7,  6,  5,  4,  11, 10, 9,  8,  16, 15, 14, 13,
      12, 21, 20, 19, 18, 17, 26, 25, 24, 23, 22, 31, 30, 29, 28, 27};
  do {
    int t = index.t();
    // The indexed values match the expected values.
    int value = copy.i(t)[0];
    EXPECT_EQ(value, expected_values[pos]);
    value = copy.i(t)[1];
    EXPECT_EQ(value, -expected_values[pos]);
    // When we skip t values, the data is always 0.
    while (next_t < t) {
      EXPECT_EQ(copy.i(next_t)[0], 0) << "Failure t = " << next_t;
      EXPECT_EQ(copy.i(next_t)[1], 0) << "Failure t = " << next_t;
      ++next_t;
    }
    ++pos;
    ++next_t;
  } while (index.Increment());
  EXPECT_EQ(pos, 32);
  EXPECT_EQ(next_t, 40);
}

// Tests that CopyWithXYTranspose works.
TEST_F(NetworkioTest, CopyWithXYTranspose) {
  NetworkIO nio;
  SetupNetworkIO(&nio);
  NetworkIO copy;
  copy.CopyWithXYTranspose(nio);
  StrideMap::Index index(copy.stride_map());
  int next_t = 0;
  int pos = 0;
  std::vector<int> expected_values = {
      0,  4,  8,  1,  5,  9,  2,  6,  10, 3,  7,  11, 12, 17, 22, 27,
      13, 18, 23, 28, 14, 19, 24, 29, 15, 20, 25, 30, 16, 21, 26, 31};
  do {
    int t = index.t();
    // The indexed values match the expected values.
    int value = copy.i(t)[0];
    EXPECT_EQ(value, expected_values[pos]);
    value = copy.i(t)[1];
    EXPECT_EQ(value, -expected_values[pos]);
    // When we skip t values, the data is always 0.
    while (next_t < t) {
      EXPECT_EQ(copy.i(next_t)[0], 0);
      EXPECT_EQ(copy.i(next_t)[1], 0);
      ++next_t;
    }
    ++pos;
    ++next_t;
  } while (index.Increment());
  EXPECT_EQ(pos, 32);
  EXPECT_EQ(next_t, 40);
}

}  // namespace
