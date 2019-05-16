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

#include "rect.h"

#include "include_gunit.h"

namespace {

class TBOXTest : public testing::Test {
 public:
  void SetUp() {
    std::locale::global(std::locale(""));
  }

  void TearDown() {}
};

TEST_F(TBOXTest, OverlapInside) {
  TBOX a(10, 10, 20, 20);
  TBOX b(11, 11, 12, 12);

  EXPECT_TRUE(a.overlap(b));
  EXPECT_TRUE(b.overlap(a));
  EXPECT_DOUBLE_EQ(0.01, a.overlap_fraction(b));
  EXPECT_DOUBLE_EQ(1.0, b.overlap_fraction(a));
}

TEST_F(TBOXTest, OverlapBoolCorners) {
  TBOX mid(10, 10, 30, 30);
  TBOX bottom_left(5, 5, 15, 15);
  TBOX top_left(5, 25, 15, 35);
  // other corners covered by symmetry

  EXPECT_TRUE(mid.overlap(bottom_left));
  EXPECT_TRUE(bottom_left.overlap(mid));
  EXPECT_TRUE(mid.overlap(top_left));
  EXPECT_TRUE(top_left.overlap(mid));
}

TEST_F(TBOXTest, OverlapFractionCorners) {
  TBOX mid(10, 10, 30, 30);
  TBOX bottom_left(5, 5, 15, 15);
  TBOX top_left(5, 25, 15, 35);
  // other corners covered by symmetry

  EXPECT_DOUBLE_EQ((5.0 * 5.0) / (20.0 * 20.0),
                   mid.overlap_fraction(bottom_left));
  EXPECT_DOUBLE_EQ((5.0 * 5.0) / (10.0 * 10.0),
                   bottom_left.overlap_fraction(mid));
  EXPECT_DOUBLE_EQ((5.0 * 5.0) / (20.0 * 20.0), mid.overlap_fraction(top_left));
  EXPECT_DOUBLE_EQ((5.0 * 5.0) / (10.0 * 10.0), top_left.overlap_fraction(mid));
}

TEST_F(TBOXTest, OverlapBoolSides) {
  TBOX mid(10, 10, 30, 30);
  TBOX left(5, 15, 15, 25);
  TBOX bottom(15, 5, 25, 15);
  // other sides covered by symmetry

  EXPECT_TRUE(mid.overlap(left));
  EXPECT_TRUE(left.overlap(mid));
  EXPECT_TRUE(mid.overlap(bottom));
  EXPECT_TRUE(bottom.overlap(mid));
}

TEST_F(TBOXTest, OverlapFractionSides) {
  TBOX mid(10, 10, 30, 30);
  TBOX left(5, 15, 15, 25);
  TBOX bottom(15, 5, 25, 15);
  // other sides covered by symmetry

  EXPECT_DOUBLE_EQ((5.0 * 10.0) / (20.0 * 20.0), mid.overlap_fraction(left));
  EXPECT_DOUBLE_EQ((5.0 * 10.0) / (10.0 * 10.0), left.overlap_fraction(mid));
  EXPECT_DOUBLE_EQ((5.0 * 10.0) / (20.0 * 20.0), mid.overlap_fraction(bottom));
  EXPECT_DOUBLE_EQ((5.0 * 10.0) / (10.0 * 10.0), bottom.overlap_fraction(mid));
}

TEST_F(TBOXTest, OverlapBoolSpan) {
  TBOX mid(10, 10, 30, 30);
  TBOX vertical(15, 5, 25, 35);
  TBOX horizontal(5, 15, 35, 25);
  // other sides covered by symmetry in other test cases

  EXPECT_TRUE(mid.overlap(vertical));
  EXPECT_TRUE(vertical.overlap(mid));
  EXPECT_TRUE(mid.overlap(horizontal));
  EXPECT_TRUE(horizontal.overlap(mid));
}

TEST_F(TBOXTest, OverlapFractionSpan) {
  TBOX mid(10, 10, 30, 30);
  TBOX vertical(15, 5, 25, 35);
  TBOX horizontal(5, 15, 35, 25);
  // other sides covered by symmetry in other test cases

  EXPECT_DOUBLE_EQ((10.0 * 20.0) / (20.0 * 20.0),
                   mid.overlap_fraction(vertical));
  EXPECT_DOUBLE_EQ((10.0 * 20.0) / (10.0 * 30.0),
                   vertical.overlap_fraction(mid));
  EXPECT_DOUBLE_EQ((20.0 * 10.0) / (20.0 * 20.0),
                   mid.overlap_fraction(horizontal));
  EXPECT_DOUBLE_EQ((20.0 * 10.0) / (30.0 * 10.0),
                   horizontal.overlap_fraction(mid));
}

// TODO(nbeato): pretty much all cases
TEST_F(TBOXTest, OverlapOutsideTests) {
  TBOX mid(10, 10, 30, 30);
  TBOX left(0, 15, 5, 25);

  EXPECT_FALSE(mid.overlap(left));
  EXPECT_FALSE(left.overlap(mid));
  EXPECT_DOUBLE_EQ(0.0, mid.overlap_fraction(left));
  EXPECT_DOUBLE_EQ(0.0, left.overlap_fraction(mid));
}

TEST_F(TBOXTest, OverlapXFraction) {
  TBOX a(10, 10, 20, 20);
  TBOX b(12, 100, 26, 200);
  TBOX c(0, 0, 100, 100);
  TBOX d(0, 0, 1, 1);

  EXPECT_DOUBLE_EQ(8.0 / 10.0, a.x_overlap_fraction(b));
  EXPECT_DOUBLE_EQ(8.0 / 14.0, b.x_overlap_fraction(a));
  EXPECT_DOUBLE_EQ(1.0, a.x_overlap_fraction(c));
  EXPECT_DOUBLE_EQ(10.0 / 100.0, c.x_overlap_fraction(a));
  EXPECT_DOUBLE_EQ(0.0, a.x_overlap_fraction(d));
  EXPECT_DOUBLE_EQ(0.0, d.x_overlap_fraction(a));
}

TEST_F(TBOXTest, OverlapYFraction) {
  TBOX a(10, 10, 20, 20);
  TBOX b(100, 12, 200, 26);
  TBOX c(0, 0, 100, 100);
  TBOX d(0, 0, 1, 1);

  EXPECT_DOUBLE_EQ(8.0 / 10.0, a.y_overlap_fraction(b));
  EXPECT_DOUBLE_EQ(8.0 / 14.0, b.y_overlap_fraction(a));
  EXPECT_DOUBLE_EQ(1.0, a.y_overlap_fraction(c));
  EXPECT_DOUBLE_EQ(10.0 / 100.0, c.y_overlap_fraction(a));
  EXPECT_DOUBLE_EQ(0.0, a.y_overlap_fraction(d));
  EXPECT_DOUBLE_EQ(0.0, d.y_overlap_fraction(a));
}

TEST_F(TBOXTest, OverlapXFractionZeroSize) {
  TBOX zero(10, 10, 10, 10);
  TBOX big(0, 0, 100, 100);
  TBOX small(0, 0, 1, 1);

  EXPECT_DOUBLE_EQ(1.0, zero.x_overlap_fraction(big));
  EXPECT_DOUBLE_EQ(0.0, big.x_overlap_fraction(zero));
  EXPECT_DOUBLE_EQ(0.0, zero.x_overlap_fraction(small));
  EXPECT_DOUBLE_EQ(0.0, small.x_overlap_fraction(zero));
}

TEST_F(TBOXTest, OverlapYFractionZeroSize) {
  TBOX zero(10, 10, 10, 10);
  TBOX big(0, 0, 100, 100);
  TBOX small(0, 0, 1, 1);

  EXPECT_DOUBLE_EQ(1.0, zero.y_overlap_fraction(big));
  EXPECT_DOUBLE_EQ(0.0, big.y_overlap_fraction(zero));
  EXPECT_DOUBLE_EQ(0.0, zero.y_overlap_fraction(small));
  EXPECT_DOUBLE_EQ(0.0, small.y_overlap_fraction(zero));
}

}  // namespace
