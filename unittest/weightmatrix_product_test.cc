///////////////////////////////////////////////////////////////////////
// File:        weightmatrix_product_test.cc
// Description: Tests the matrix products of a float-mode WeightMatrix
//              (MatrixDotVector, VectorDotMatrix and SumOuterTransposed)
//              against a straightforward calculation with doubles.
//              The sizes are chosen to cover all the blocks and the
//              remainders of the SIMD implementations.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
//
///////////////////////////////////////////////////////////////////////

#include "include_gunit.h"

#include "helpers.h"      // for TRand
#include "weightmatrix.h" // for WeightMatrix, TransposedArray

#include <cmath>
#include <vector>

namespace tesseract {
namespace {

// The results are sums of n products of floats. Allow an error relative to
// the sum of the absolute values of the products, which is the bound for
// the rounding errors of any order of summation.
const double kRelativeTolerance = 1e-5;
const double kAbsoluteTolerance = 1e-6;

class WeightMatrixProductTest : public ::testing::Test {
protected:
  void SetUp() override {
    randomizer_.set_seed(42);
  }

  std::vector<TFloat> RandomVector(int size) {
    std::vector<TFloat> v(size);
    for (auto &value : v) {
      value = static_cast<TFloat>(randomizer_.SignedRand(1.0));
    }
    return v;
  }

  void FillRandom(int size1, int size2, TransposedArray *array) {
    array->ResizeNoInit(size1, size2);
    for (int i = 0; i < size1; ++i) {
      for (int j = 0; j < size2; ++j) {
        array->put(i, j, static_cast<TFloat>(randomizer_.SignedRand(1.0)));
      }
    }
  }

  static void ExpectClose(double expected, double sum_abs, TFloat actual,
                          const char *what, int no, int ni, int index) {
    EXPECT_NEAR(expected, actual,
                kRelativeTolerance * sum_abs + kAbsoluteTolerance)
        << what << " for no=" << no << ", ni=" << ni << ", index=" << index;
  }

  // Tests the products with vectors for no outputs and ni inputs.
  void TestVectorProducts(int no, int ni) {
    WeightMatrix matrix;
    matrix.InitWeightsFloat(no, ni + 1, false, 1.0f, &randomizer_);

    // Forward: v = W u with the bias in the last column of W.
    // The extra elements detect writes beyond the end of the result.
    const TFloat kGuard = 12345;
    std::vector<TFloat> u = RandomVector(ni);
    std::vector<TFloat> v(no + 1, kGuard);
    matrix.MatrixDotVector(u.data(), v.data());
    for (int i = 0; i < no; ++i) {
      const TFloat *w = matrix.GetWeights(i);
      double expected = w[ni];
      double sum_abs = std::fabs(w[ni]);
      for (int j = 0; j < ni; ++j) {
        double product = static_cast<double>(w[j]) * u[j];
        expected += product;
        sum_abs += std::fabs(product);
      }
      ExpectClose(expected, sum_abs, v[i], "MatrixDotVector", no, ni, i);
    }
    EXPECT_EQ(v[no], kGuard);

    // Backward: v = u W without the bias column.
    u = RandomVector(no);
    v.assign(ni + 1, kGuard);
    matrix.VectorDotMatrix(u.data(), v.data());
    for (int j = 0; j < ni; ++j) {
      double expected = 0;
      double sum_abs = 0;
      for (int i = 0; i < no; ++i) {
        double product = static_cast<double>(matrix.GetWeights(i)[j]) * u[i];
        expected += product;
        sum_abs += std::fabs(product);
      }
      ExpectClose(expected, sum_abs, v[j], "VectorDotMatrix", no, ni, j);
    }
    EXPECT_EQ(v[ni], kGuard);
  }

  // Tests the sum of the outer products for no outputs, ni inputs and
  // the given number of samples (timesteps).
  void TestSumOuter(int no, int ni, int num_samples) {
    WeightMatrix matrix;
    matrix.InitWeightsFloat(no, ni + 1, false, 1.0f, &randomizer_);
    TransposedArray u;
    TransposedArray v;
    FillRandom(no, num_samples, &u);
    FillRandom(ni, num_samples, &v);
    matrix.SumOuterTransposed(u, v, false);
    std::vector<TFloat> serial_result;
    for (int i = 0; i < no; ++i) {
      // The last result is the sum for the bias with an input of 1.
      for (int j = 0; j <= ni; ++j) {
        double expected = 0;
        double sum_abs = 0;
        for (int k = 0; k < num_samples; ++k) {
          double product =
              static_cast<double>(u(i, k)) * (j < ni ? v(j, k) : 1);
          expected += product;
          sum_abs += std::fabs(product);
        }
        TFloat actual = matrix.GetDW(i, j);
        EXPECT_NEAR(expected, actual,
                    kRelativeTolerance * sum_abs + kAbsoluteTolerance)
            << "SumOuterTransposed for no=" << no << ", ni=" << ni
            << ", num_samples=" << num_samples << ", i=" << i << ", j=" << j;
        serial_result.push_back(actual);
      }
    }
    // The parallel calculation must give exactly the same results.
    matrix.SumOuterTransposed(u, v, true);
    size_t index = 0;
    for (int i = 0; i < no; ++i) {
      for (int j = 0; j <= ni; ++j) {
        EXPECT_EQ(serial_result[index++], matrix.GetDW(i, j))
            << "parallel SumOuterTransposed for no=" << no << ", ni=" << ni
            << ", num_samples=" << num_samples << ", i=" << i << ", j=" << j;
      }
    }
  }

  TRand randomizer_;
};

// Sizes around the block sizes of the SIMD code (4 rows, 4 or 8 elements)
// and some sizes of real networks.
const int kSizes[] = {1, 2, 3, 4, 5, 7, 8, 9, 11, 12, 13, 16, 17, 31, 48, 111};

TEST_F(WeightMatrixProductTest, VectorProducts) {
  for (int no : kSizes) {
    for (int ni : kSizes) {
      TestVectorProducts(no, ni);
    }
  }
  TestVectorProducts(192, 288);
}

TEST_F(WeightMatrixProductTest, SumOuterTransposed) {
  const int kOutputs[] = {1, 2, 3, 4, 5, 7, 8, 9, 13, 48};
  const int kInputs[] = {1, 2, 3, 4, 5, 7, 8, 9, 13, 64};
  const int kSamples[] = {1, 2, 3, 4, 5, 7, 8, 9, 15, 16, 17, 100};
  for (int no : kOutputs) {
    for (int ni : kInputs) {
      for (int num_samples : kSamples) {
        TestSumOuter(no, ni, num_samples);
      }
    }
  }
}

} // namespace
} // namespace tesseract
