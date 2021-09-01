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
#include <gtest/gtest.h>
#include <gtest/internal/gtest-port.h>
#include <memory>
#include <vector>
#include "include_gunit.h"
#include "matrix.h"
#include "simddetect.h"
#include "tprintf.h"

namespace tesseract {

class IntSimdMatrixTest : public ::testing::Test {
protected:
  void SetUp() override {
    std::locale::global(std::locale(""));
  }

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
  std::vector<int8_t> RandomVector(int size, const IntSimdMatrix &matrix) {
    int rounded_size = matrix.RoundInputs(size);
    std::vector<int8_t> v(rounded_size, 0);
    for (int i = 0; i < size; ++i) {
      v[i] = static_cast<int8_t>(random_.SignedRand(INT8_MAX));
    }
    return v;
  }
  // Makes a random scales vector of the given size.
  std::vector<TFloat> RandomScales(int size) {
    std::vector<TFloat> v(size);
    for (int i = 0; i < size; ++i) {
      v[i] = (1.0 + random_.SignedRand(1.0)) / INT8_MAX;
    }
    return v;
  }
  // Tests a range of sizes and compares the results against the generic version.
  void ExpectEqualResults(const IntSimdMatrix &matrix) {
    TFloat total = 0.0;
    for (int num_out = 1; num_out < 130; ++num_out) {
      for (int num_in = 1; num_in < 130; ++num_in) {
        GENERIC_2D_ARRAY<int8_t> w = InitRandom(num_out, num_in + 1);
        std::vector<int8_t> u = RandomVector(num_in, matrix);
        std::vector<TFloat> scales = RandomScales(num_out);
        int ro = num_out;
        if (IntSimdMatrix::intSimdMatrix) {
          ro = IntSimdMatrix::intSimdMatrix->RoundOutputs(ro);
        }
        std::vector<TFloat> base_result(num_out);
        IntSimdMatrix::MatrixDotVector(w, scales, u.data(), base_result.data());
        std::vector<TFloat> test_result(ro);
        std::vector<int8_t> shaped_wi;
        int32_t rounded_num_out;
        matrix.Init(w, shaped_wi, rounded_num_out);
        scales.resize(rounded_num_out);
        if (matrix.matrixDotVectorFunction) {
          matrix.matrixDotVectorFunction(w.dim1(), w.dim2(), &shaped_wi[0], &scales[0], &u[0],
                                         &test_result[0]);
        } else {
          IntSimdMatrix::MatrixDotVector(w, scales, u.data(), test_result.data());
        }
        for (int i = 0; i < num_out; ++i) {
          EXPECT_FLOAT_EQ(base_result[i], test_result[i]) << "i=" << i;
          total += base_result[i];
        }
      }
    }
    // Compare sum of all results with expected value.
#ifdef FAST_FLOAT
    EXPECT_FLOAT_EQ(total, 337852.16f);
#else
    EXPECT_FLOAT_EQ(total, 337849.39354684710);
#endif
  }

  TRand random_;
};

// Test the C++ implementation without SIMD.
TEST_F(IntSimdMatrixTest, C) {
  static const IntSimdMatrix matrix = {nullptr, 1, 1, 1, 1};
  ExpectEqualResults(matrix);
}

// Tests that the SSE implementation gets the same result as the vanilla.
TEST_F(IntSimdMatrixTest, SSE) {
#if defined(HAVE_SSE4_1)
  if (!SIMDDetect::IsSSEAvailable()) {
    GTEST_LOG_(INFO) << "No SSE found! Not tested!";
    GTEST_SKIP();
  }
  ExpectEqualResults(IntSimdMatrix::intSimdMatrixSSE);
#else
  GTEST_LOG_(INFO) << "SSE unsupported! Not tested!";
  GTEST_SKIP();
#endif
}

// Tests that the AVX2 implementation gets the same result as the vanilla.
TEST_F(IntSimdMatrixTest, AVX2) {
#if defined(HAVE_AVX2)
  if (!SIMDDetect::IsAVX2Available()) {
    GTEST_LOG_(INFO) << "No AVX2 found! Not tested!";
    GTEST_SKIP();
  }
  ExpectEqualResults(IntSimdMatrix::intSimdMatrixAVX2);
#else
  GTEST_LOG_(INFO) << "AVX2 unsupported! Not tested!";
  GTEST_SKIP();
#endif
}

} // namespace tesseract
