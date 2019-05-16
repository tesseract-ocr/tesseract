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

#include <memory>

#include "tabvector.h"

#include "include_gunit.h"

using tesseract::TabVector;

namespace {

class TabVectorTest : public testing::Test {
 protected:
  void SetUp() {
    std::locale::global(std::locale(""));
    vector_.reset();
  }

  void TearDown() {}

  void MakeSimpleTabVector(int x1, int y1, int x2, int y2) {
    vector_.reset(new TabVector());
    vector_->set_startpt(ICOORD(x1, y1));
    vector_->set_endpt(ICOORD(x2, y2));
  }

  std::unique_ptr<TabVector> vector_;
};

TEST_F(TabVectorTest, SetStartEndPointsMatch) {
  vector_.reset(new TabVector());
  ICOORD start(51, 65);
  ICOORD end(7568, 234);
  // Test coordinates individually to avoid adding an ostream operator
  // explicitly to the ICOORD class (Droid doesn't support it).
  vector_->set_startpt(start);
  EXPECT_EQ(start.x(), vector_->startpt().x());
  EXPECT_EQ(start.y(), vector_->startpt().y());
  vector_->set_endpt(end);
  EXPECT_EQ(end.x(), vector_->endpt().x());
  EXPECT_EQ(end.y(), vector_->endpt().y());
}

TEST_F(TabVectorTest, XAtY45DegreeSlopeInRangeExact) {
  MakeSimpleTabVector(0, 0, 100, 100);
  for (int y = 0; y <= 100; ++y) {
    int x = vector_->XAtY(y);
    EXPECT_EQ(y, x);
  }
}

TEST_F(TabVectorTest, XAtYVerticalInRangeExact) {
  const int x = 120;  // Arbitrary choice
  MakeSimpleTabVector(x, 0, x, 100);
  for (int y = 0; y <= 100; ++y) {
    int result_x = vector_->XAtY(y);
    EXPECT_EQ(x, result_x);
  }
}

TEST_F(TabVectorTest, XAtYHorizontal) {
  const int y = 76;  // arbitrary
  MakeSimpleTabVector(0, y, 100, y);
  EXPECT_EQ(0, vector_->XAtY(y));
  // TODO(nbeato): What's the failure condition?
  // Undefined! Should not pass! Allow until resolved answer.
  EXPECT_EQ(0, vector_->XAtY(10));
}

TEST_F(TabVectorTest, XAtYRoundingSimple) {
  MakeSimpleTabVector(0, 0, 2, 10000);
  int x = vector_->XAtY(1);
  EXPECT_EQ(0, x);
  x = vector_->XAtY(4999);
  EXPECT_EQ(0, x);
  x = vector_->XAtY(5001);
  EXPECT_EQ(1, x);
  x = vector_->XAtY(9999);
  EXPECT_EQ(1, x);
}

TEST_F(TabVectorTest, XAtYLargeNumbers) {
  // Assume a document is 800 DPI,
  // the width of a page is 10 inches across (8000 pixels), and
  // the height of the page is 15 inches (12000 pixels).
  MakeSimpleTabVector(7804, 504, 7968, 11768);  // Arbitrary for vertical line
  int x = vector_->XAtY(6136);                  // test mid point
  EXPECT_EQ(7886, x);
}

TEST_F(TabVectorTest, XAtYHorizontalInRangeExact) {
  const int y = 120;  // Arbitrary choice
  MakeSimpleTabVector(50, y, 150, y);

  int x = vector_->XAtY(y);
  EXPECT_EQ(50, x);
}

TEST_F(TabVectorTest, VOverlapInRangeSimple) {
  MakeSimpleTabVector(0, 0, 100, 100);
  int overlap = vector_->VOverlap(90, 10);
  EXPECT_EQ(80, overlap);
  overlap = vector_->VOverlap(100, 0);
  EXPECT_EQ(100, overlap);
}

TEST_F(TabVectorTest, VOverlapOutOfRange) {
  MakeSimpleTabVector(0, 10, 100, 90);
  int overlap = vector_->VOverlap(100, 0);
  EXPECT_EQ(80, overlap);
}

TEST_F(TabVectorTest, XYFlip) {
  MakeSimpleTabVector(1, 2, 3, 4);
  vector_->XYFlip();
  EXPECT_EQ(2, vector_->startpt().x());
  EXPECT_EQ(1, vector_->startpt().y());
  EXPECT_EQ(4, vector_->endpt().x());
  EXPECT_EQ(3, vector_->endpt().y());
}

}  // namespace
