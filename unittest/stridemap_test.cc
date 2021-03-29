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

#ifdef INCLUDE_TENSORFLOW
#  include <tensorflow/compiler/xla/array2d.h> // for xla::Array2D
#else
#  include <array> // std::array
#endif
#include "include_gunit.h"
#include "stridemap.h"

namespace tesseract {

#if !defined(INCLUDE_TENSORFLOW) && 0
namespace xla {

template <typename T>
class Array2D : public std::vector<T> {
public:
  Array2D() : std::vector<T>(std::vector<int64_t>{0, 0}) {}

  Array2D(const int64_t n1, const int64_t n2) : std::vector<T>(std::vector<int64_t>{n1, n2}) {}

  Array2D(const int64_t n1, const int64_t n2, const T value) : std::vector<T>({n1, n2}, value) {}
};
} // namespace xla
#endif

class StridemapTest : public ::testing::Test {
protected:
  void SetUp() override {
    std::locale::global(std::locale(""));
  }

#ifdef INCLUDE_TENSORFLOW
  // Sets up an Array2d object of the given size, initialized to increasing
  // values starting with start.
  std::unique_ptr<xla::Array2D<int>> SetupArray(int ysize, int xsize, int start) {
    std::unique_ptr<xla::Array2D<int>> a(new xla::Array2D<int>(ysize, xsize));
    int value = start;
    for (int y = 0; y < ysize; ++y) {
      for (int x = 0; x < xsize; ++x) {
#  ifdef INCLUDE_TENSORFLOW
        (*a)(y, x) = value++;
#  else
        a[y][x] = value++;
#  endif
      }
    }
    return a;
  }
#endif
};

TEST_F(StridemapTest, Indexing) {
  // This test verifies that with a batch of arrays of different sizes, the
  // iteration index each of them in turn, without going out of bounds.
#ifdef INCLUDE_TENSORFLOW
  std::vector<std::unique_ptr<xla::Array2D<int>>> arrays;
  arrays.push_back(SetupArray(3, 4, 0));
  arrays.push_back(SetupArray(4, 5, 12));
  arrays.push_back(SetupArray(4, 4, 32));
  arrays.push_back(SetupArray(3, 5, 48));
  std::vector<std::pair<int, int>> h_w_sizes;
  for (size_t i = 0; i < arrays.size(); ++i) {
    h_w_sizes.emplace_back(arrays[i].get()->height(), arrays[i].get()->width());
  }
  StrideMap stride_map;
  stride_map.SetStride(h_w_sizes);
  StrideMap::Index index(stride_map);
  int pos = 0;
  do {
    EXPECT_GE(index.t(), pos);
    EXPECT_EQ((*arrays.at(index.index(FD_BATCH)))(index.index(FD_HEIGHT), index.index(FD_WIDTH)),
              pos);
    EXPECT_EQ(index.IsLast(FD_BATCH), index.index(FD_BATCH) == arrays.size() - 1);
    EXPECT_EQ(index.IsLast(FD_HEIGHT),
              index.index(FD_HEIGHT) == arrays[index.index(FD_BATCH)]->height() - 1);
    EXPECT_EQ(index.IsLast(FD_WIDTH),
              index.index(FD_WIDTH) == arrays[index.index(FD_BATCH)]->width() - 1);
    EXPECT_TRUE(index.IsValid());
    ++pos;
  } while (index.Increment());
  LOG(INFO) << "pos=" << pos;
  index.InitToLast();
  do {
    --pos;
    EXPECT_GE(index.t(), pos);
    EXPECT_EQ((*arrays.at(index.index(FD_BATCH)))(index.index(FD_HEIGHT), index.index(FD_WIDTH)),
              pos);
    StrideMap::Index copy(index);
    // Since a change in batch index changes the height and width, it isn't
    // necessarily true that the position is still valid, even when changing
    // to another valid batch index.
    if (index.IsLast(FD_BATCH)) {
      EXPECT_FALSE(copy.AddOffset(1, FD_BATCH));
    }
    copy = index;
    EXPECT_EQ(index.IsLast(FD_HEIGHT), !copy.AddOffset(1, FD_HEIGHT));
    copy = index;
    EXPECT_EQ(index.IsLast(FD_WIDTH), !copy.AddOffset(1, FD_WIDTH));
    copy = index;
    if (index.index(FD_BATCH) == 0) {
      EXPECT_FALSE(copy.AddOffset(-1, FD_BATCH));
    }
    copy = index;
    EXPECT_EQ(index.index(FD_HEIGHT) == 0, !copy.AddOffset(-1, FD_HEIGHT));
    copy = index;
    EXPECT_EQ(index.index(FD_WIDTH) == 0, !copy.AddOffset(-1, FD_WIDTH));
    copy = index;
    EXPECT_FALSE(copy.AddOffset(10, FD_WIDTH));
    copy = index;
    EXPECT_FALSE(copy.AddOffset(-10, FD_HEIGHT));
    EXPECT_TRUE(index.IsValid());
  } while (index.Decrement());
#else
  LOG(INFO) << "Skip test because of missing xla::Array2D";
  GTEST_SKIP();
#endif
}

TEST_F(StridemapTest, Scaling) {
  // This test verifies that with a batch of arrays of different sizes, the
  // scaling/reduction functions work as expected.
#ifdef INCLUDE_TENSORFLOW
  std::vector<std::unique_ptr<xla::Array2D<int>>> arrays;
  arrays.push_back(SetupArray(3, 4, 0));  // 0-11
  arrays.push_back(SetupArray(4, 5, 12)); // 12-31
  arrays.push_back(SetupArray(4, 4, 32)); // 32-47
  arrays.push_back(SetupArray(3, 5, 48)); // 48-62
  std::vector<std::pair<int, int>> h_w_sizes;
  for (size_t i = 0; i < arrays.size(); ++i) {
    h_w_sizes.emplace_back(arrays[i].get()->height(), arrays[i].get()->width());
  }
  StrideMap stride_map;
  stride_map.SetStride(h_w_sizes);

  // Scale x by 2, keeping y the same.
  std::vector<int> values_x2 = {0,  1,  4,  5,  8,  9,  12, 13, 17, 18, 22, 23, 27, 28,
                                32, 33, 36, 37, 40, 41, 44, 45, 48, 49, 53, 54, 58, 59};
  StrideMap test_map(stride_map);
  test_map.ScaleXY(2, 1);
  StrideMap::Index index(test_map);
  int pos = 0;
  do {
    int expected_value = values_x2[pos++];
    EXPECT_EQ((*arrays.at(index.index(FD_BATCH)))(index.index(FD_HEIGHT), index.index(FD_WIDTH)),
              expected_value);
  } while (index.Increment());
  EXPECT_EQ(pos, values_x2.size());

  test_map = stride_map;
  // Scale y by 2, keeping x the same.
  std::vector<int> values_y2 = {0,  1,  2,  3,  12, 13, 14, 15, 16, 17, 18, 19, 20, 21,
                                32, 33, 34, 35, 36, 37, 38, 39, 48, 49, 50, 51, 52};
  test_map.ScaleXY(1, 2);
  index.InitToFirst();
  pos = 0;
  do {
    int expected_value = values_y2[pos++];
    EXPECT_EQ((*arrays.at(index.index(FD_BATCH)))(index.index(FD_HEIGHT), index.index(FD_WIDTH)),
              expected_value);
  } while (index.Increment());
  EXPECT_EQ(pos, values_y2.size());

  test_map = stride_map;
  // Scale x and y by 2.
  std::vector<int> values_xy2 = {0, 1, 12, 13, 17, 18, 32, 33, 36, 37, 48, 49};
  test_map.ScaleXY(2, 2);
  index.InitToFirst();
  pos = 0;
  do {
    int expected_value = values_xy2[pos++];
    EXPECT_EQ((*arrays.at(index.index(FD_BATCH)))(index.index(FD_HEIGHT), index.index(FD_WIDTH)),
              expected_value);
  } while (index.Increment());
  EXPECT_EQ(pos, values_xy2.size());

  test_map = stride_map;
  // Reduce Width to 1.
  std::vector<int> values_x_to_1 = {0, 4, 8, 12, 17, 22, 27, 32, 36, 40, 44, 48, 53, 58};
  test_map.ReduceWidthTo1();
  index.InitToFirst();
  pos = 0;
  do {
    int expected_value = values_x_to_1[pos++];
    EXPECT_EQ((*arrays.at(index.index(FD_BATCH)))(index.index(FD_HEIGHT), index.index(FD_WIDTH)),
              expected_value);
  } while (index.Increment());
  EXPECT_EQ(pos, values_x_to_1.size());
#else
  LOG(INFO) << "Skip test because of missing xla::Array2D";
  GTEST_SKIP();
#endif
}

} // namespace tesseract
