///////////////////////////////////////////////////////////////////////
// File:        matrix_test.cc
// Author:      rays@google.com (Ray Smith)
//
// Copyright 2016 Google Inc. All Rights Reserved.
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
///////////////////////////////////////////////////////////////////////

#include "matrix.h"
#include "genericvector.h"
#include "include_gunit.h"

namespace {

class MatrixTest : public ::testing::Test {
 protected:
  void SetUp() override {
    std::locale::global(std::locale(""));
  }

  // Fills src_ with data so it can pretend to be a tensor thus:
  //  dims_=[5, 4, 3, 2]
  //  array_=[0, 1, 2, ....119]
  //  tensor=[[[[0, 1][2, 3][4, 5]]
  //           [[6, 7][8, 9][10, 11]]
  //           [[12, 13][14, 15][16, 17]]
  //           [[18, 19][20, 21][22, 23]]]
  //          [[[24, 25]...
  MatrixTest() {
    src_.Resize(1, kInputSize_, 0);
    for (int i = 0; i < kInputSize_; ++i) {
      src_.put(0, i, i);
    }
    for (int i = 0; i < kNumDims_; ++i) dims_[i] = 5 - i;
  }
  // Number of dimensions in src_.
  static const int kNumDims_ = 4;
  // Number of elements in src_.
  static const int kInputSize_ = 120;
  // Size of each dimension in src_;
  int dims_[kNumDims_];
  // Input array filled with [0,kInputSize).
  GENERIC_2D_ARRAY<int> src_;
};

// Tests that the RotatingTranspose function does the right thing for various
// transformations.
// dims=[5, 4, 3, 2]->[5, 2, 4, 3]
TEST_F(MatrixTest, RotatingTranspose_3_1) {
  GENERIC_2D_ARRAY<int> m;
  src_.RotatingTranspose(dims_, kNumDims_, 3, 1, &m);
  m.ResizeNoInit(kInputSize_ / 3, 3);
  // Verify that the result is:
  // output tensor=[[[[0, 2, 4][6, 8, 10][12, 14, 16][18, 20, 22]]
  //                 [[1, 3, 5][7, 9, 11][13, 15, 17][19, 21, 23]]]
  //                [[[24, 26, 28]...
  EXPECT_EQ(0, m(0, 0));
  EXPECT_EQ(2, m(0, 1));
  EXPECT_EQ(4, m(0, 2));
  EXPECT_EQ(6, m(1, 0));
  EXPECT_EQ(1, m(4, 0));
  EXPECT_EQ(24, m(8, 0));
  EXPECT_EQ(26, m(8, 1));
  EXPECT_EQ(25, m(12, 0));
}

// dims=[5, 4, 3, 2]->[3, 5, 4, 2]
TEST_F(MatrixTest, RotatingTranspose_2_0) {
  GENERIC_2D_ARRAY<int> m;
  src_.RotatingTranspose(dims_, kNumDims_, 2, 0, &m);
  m.ResizeNoInit(kInputSize_ / 2, 2);
  // Verify that the result is:
  // output tensor=[[[[0, 1][6, 7][12, 13][18, 19]]
  //                 [[24, 25][30, 31][36, 37][42, 43]]
  //                 [[48, 49][54, 55][60, 61][66, 67]]
  //                 [[72, 73][78, 79][84, 85][90, 91]]
  //                 [[96, 97][102, 103][108, 109][114, 115]]]
  //                [[[2,3]...
  EXPECT_EQ(0, m(0, 0));
  EXPECT_EQ(1, m(0, 1));
  EXPECT_EQ(6, m(1, 0));
  EXPECT_EQ(7, m(1, 1));
  EXPECT_EQ(24, m(4, 0));
  EXPECT_EQ(25, m(4, 1));
  EXPECT_EQ(30, m(5, 0));
  EXPECT_EQ(2, m(20, 0));
}

// dims=[5, 4, 3, 2]->[5, 3, 2, 4]
TEST_F(MatrixTest, RotatingTranspose_1_3) {
  GENERIC_2D_ARRAY<int> m;
  src_.RotatingTranspose(dims_, kNumDims_, 1, 3, &m);
  m.ResizeNoInit(kInputSize_ / 4, 4);
  // Verify that the result is:
  // output tensor=[[[[0, 6, 12, 18][1, 7, 13, 19]]
  //                 [[2, 8, 14, 20][3, 9, 15, 21]]
  //                 [[4, 10, 16, 22][5, 11, 17, 23]]]
  //                [[[24, 30, 36, 42]...
  EXPECT_EQ(0, m(0, 0));
  EXPECT_EQ(6, m(0, 1));
  EXPECT_EQ(1, m(1, 0));
  EXPECT_EQ(2, m(2, 0));
  EXPECT_EQ(3, m(3, 0));
  EXPECT_EQ(4, m(4, 0));
  EXPECT_EQ(5, m(5, 0));
  EXPECT_EQ(24, m(6, 0));
  EXPECT_EQ(30, m(6, 1));
}

// dims=[5, 4, 3, 2]->[4, 3, 5, 2]
TEST_F(MatrixTest, RotatingTranspose_0_2) {
  GENERIC_2D_ARRAY<int> m;
  src_.RotatingTranspose(dims_, kNumDims_, 0, 2, &m);
  m.ResizeNoInit(kInputSize_ / 2, 2);
  // Verify that the result is:
  // output tensor=[[[[0, 1][24, 25][48, 49][72, 73][96, 97]]
  //                 [[2, 3][26, 27][50, 51][74, 75][98, 99]]
  //                 [[4, 5][28, 29][52, 53][76, 77][100, 101]]]
  //                [[[6, 7]...
  EXPECT_EQ(0, m(0, 0));
  EXPECT_EQ(1, m(0, 1));
  EXPECT_EQ(24, m(1, 0));
  EXPECT_EQ(25, m(1, 1));
  EXPECT_EQ(96, m(4, 0));
  EXPECT_EQ(97, m(4, 1));
  EXPECT_EQ(2, m(5, 0));
  EXPECT_EQ(6, m(15, 0));
}

}  // namespace
