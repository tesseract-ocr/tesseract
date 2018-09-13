///////////////////////////////////////////////////////////////////////
// File:        intsimdmatrix_test.cc
// Author:      rays@google.com (Ray Smith)
//
// Copyright 2017 Google Inc. All Rights Reserved.
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

#include "intsimdmatrix.h"
#include <memory>
#include "genericvector.h"
#include "include_gunit.h"
#include "intsimdmatrixavx2.h"
#include "intsimdmatrixsse.h"
#include "matrix.h"
#include "simddetect.h"
#include "tprintf.h"

namespace tesseract {
namespace {

class IntSimdMatrixTest : public ::testing::Test {
 protected:
  // Makes a random weights matrix of the given size.
  GENERIC_2D_ARRAY<int8_t> InitRandom(int no, int ni) {
    GENERIC_2D_ARRAY<int8_t> a(no, ni, 0);
    for (int i = 0; i < no; ++i) {
      for (int j = 0; j < ni; ++j) {
        a(i, j) = static_cast<int8_t>(random_.SignedRand(INT8_MAX));
      }
    }
    return a;
  }
  // Makes a random input vector of the given size, with rounding up.
  std::vector<int8_t> RandomVector(int size, const IntSimdMatrix& matrix) {
    int rounded_size = matrix.RoundInputs(size);
    std::vector<int8_t> v(rounded_size, 0);
    for (int i = 0; i < size; ++i) {
      v[i] = static_cast<int8_t>(random_.SignedRand(INT8_MAX));
    }
    return v;
  }
  // Makes a random scales vector of the given size.
  GenericVector<double> RandomScales(int size) {
    GenericVector<double> v(size, 0.0);
    for (int i = 0; i < size; ++i) {
      v[i] = 1.0 + random_.SignedRand(1.0);
    }
    return v;
  }
  // Tests a range of sizes and compares the results against the base_ version.
  void ExpectEqualResults(IntSimdMatrix* matrix) {
    for (int num_out = 1; num_out < 130; ++num_out) {
      for (int num_in = 1; num_in < 130; ++num_in) {
        GENERIC_2D_ARRAY<int8_t> w = InitRandom(num_out, num_in + 1);
        matrix->Init(w);
        std::vector<int8_t> u = RandomVector(num_in, *matrix);
        GenericVector<double> scales = RandomScales(num_out);
        std::vector<double> base_result(num_out);
        base_.MatrixDotVector(w, scales, u.data(), base_result.data());
        std::vector<double> test_result(num_out);
        matrix->MatrixDotVector(w, scales, u.data(), test_result.data());
        for (int i = 0; i < num_out; ++i)
          EXPECT_FLOAT_EQ(base_result[i], test_result[i]) << "i=" << i;
      }
    }
  }

  TRand random_;
  IntSimdMatrix base_;
};

// Tests that the SSE implementation gets the same result as the vanilla.
TEST_F(IntSimdMatrixTest, SSE) {
  if (SIMDDetect::IsSSEAvailable()) {
    tprintf("SSE found! Continuing...");
  } else {
    tprintf("No SSE found! Not Tested!");
    return;
  }
  std::unique_ptr<IntSimdMatrix> matrix(new IntSimdMatrixSSE());
  ExpectEqualResults(matrix.get());
}

// Tests that the AVX2 implementation gets the same result as the vanilla.
TEST_F(IntSimdMatrixTest, AVX2) {
  if (SIMDDetect::IsAVX2Available()) {
    tprintf("AVX2 found! Continuing...");
  } else {
    tprintf("No AVX2 found! Not Tested!");
    return;
  }
  std::unique_ptr<IntSimdMatrix> matrix(new IntSimdMatrixAVX2());
  ExpectEqualResults(matrix.get());
}

}  // namespace
}  // namespace tesseract
