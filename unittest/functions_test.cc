///////////////////////////////////////////////////////////////////////
// File:        functions_test.cc
// Description: Tests that the table based Tanh() and Logistic() saturate
//              for huge arguments without an undefined conversion to an
//              integer, and that they still interpolate in range.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
//
///////////////////////////////////////////////////////////////////////

#include "include_gunit.h"

#include "functions.h"

#include "helpers.h" // for TRand

#include <cfenv>
#include <cmath>
#include <vector>

namespace tesseract {

// Regression test for the table based Tanh() and Logistic().
//
// Their argument is the output of WeightMatrix::MatrixDotVector() and is only
// bounded by the weights and scales of the model, which are loaded without a
// range check. The functions used to scale the argument by kScaleFactor and
// convert it to unsigned *before* comparing the index with the table size;
// converting a value which does not fit is undefined, raises FE_INVALID
// (which the tesseract CLI traps when built with g++) and, below 2^63, yields
// a truncated index that can pass the check so that the result was far
// outside the range of the function.

// Arguments that saturate, including ones for which 256 * x does not fit
// into a 32 bit or 64 bit integer.
static const TFloat kLargeArgs[] = {16.0f, 1e3f, 1e6f, 1e10f, 1e16f, 1e20f, 3e38f};

TEST(FunctionsTest, TanhSaturates) {
  std::feclearexcept(FE_ALL_EXCEPT);
  for (TFloat x : kLargeArgs) {
    EXPECT_FLOAT_EQ(1, Tanh(x)) << "x=" << x;
    EXPECT_FLOAT_EQ(-1, Tanh(-x)) << "x=" << -x;
  }
  EXPECT_EQ(0, std::fetestexcept(FE_INVALID));
}

TEST(FunctionsTest, LogisticSaturates) {
  std::feclearexcept(FE_ALL_EXCEPT);
  for (TFloat x : kLargeArgs) {
    EXPECT_FLOAT_EQ(1, Logistic(x)) << "x=" << x;
    EXPECT_FLOAT_EQ(0, Logistic(-x)) << "x=" << -x;
  }
  EXPECT_EQ(0, std::fetestexcept(FE_INVALID));
}

// Arguments inside the table still interpolate.
TEST(FunctionsTest, InterpolatesInRange) {
  for (TFloat x = -15.0f; x <= 15.0f; x += 0.37f) {
    EXPECT_NEAR(std::tanh(x), Tanh(x), 1e-3) << "x=" << x;
    EXPECT_NEAR(1 / (1 + std::exp(-x)), Logistic(x), 1e-3) << "x=" << x;
  }
}

// The functions for whole vectors (TanhVector, LogisticVector and
// FloatToInt8Vector, which use SIMD code if the CPU supports it) must give
// exactly the same results as the scalar functions for all lengths, also for
// huge values and at the limits of the table.
TEST(FunctionsTest, VectorFunctionsMatchScalar) {
  std::vector<TFloat> values = {0.0f,   -0.0f,  1e-30f, -1e-30f, 0.5f / 256,
                                1.0f,   -1.0f,  15.99f, -15.99f, 4095.0f / 256,
                                -4095.0f / 256, 16.0f,  -16.0f,  1e30f,
                                -1e30f, 3e38f,  -3e38f};
  // Values around the entries of the table and the limits of int8_t.
  for (int i = -4100; i <= 4100; i += 7) {
    values.push_back(i / 256.0f);
    values.push_back(std::nextafter(i / 256.0f, 100.0f));
    values.push_back(std::nextafter(i / 256.0f, -100.0f));
  }
  for (int i = -260; i <= 260; ++i) {
    values.push_back(i / 254.0f);
    values.push_back(std::nextafter(i / 254.0f, 100.0f));
    values.push_back(std::nextafter(i / 254.0f, -100.0f));
  }
  TRand random;
  random.set_seed(1);
  for (int i = 0; i < 4000; ++i) {
    values.push_back(static_cast<TFloat>(random.SignedRand(20.0)));
  }
  std::feclearexcept(FE_ALL_EXCEPT);
  const size_t kLengths[] = {values.size(), 1, 2, 7, 8, 9, 15, 16, 17, 31, 48};
  for (size_t n : kLengths) {
    // The results are written to the middle of a larger vector to detect
    // writes outside of the n values.
    const TFloat kGuard = 12345;
    std::vector<TFloat> tanh_result(n + 2, kGuard);
    std::vector<TFloat> logistic_result(n + 2, kGuard);
    std::vector<int8_t> int_result(n + 2, 99);
    TanhVector(n, values.data(), tanh_result.data() + 1);
    LogisticVector(n, values.data(), logistic_result.data() + 1);
    FloatToInt8Vector(n, values.data(), int_result.data() + 1);
    for (size_t i = 0; i < n; ++i) {
      TFloat x = values[i];
      EXPECT_EQ(Tanh(x), tanh_result[i + 1]) << "x=" << x << ", n=" << n;
      EXPECT_EQ(Logistic(x), logistic_result[i + 1]) << "x=" << x << ", n=" << n;
      int expected =
          IntCastRounded(ClipToRange<TFloat>(x * INT8_MAX, -INT8_MAX, INT8_MAX));
      EXPECT_EQ(expected, int_result[i + 1]) << "x=" << x << ", n=" << n;
    }
    EXPECT_EQ(kGuard, tanh_result[0]);
    EXPECT_EQ(kGuard, tanh_result[n + 1]);
    EXPECT_EQ(kGuard, logistic_result[0]);
    EXPECT_EQ(kGuard, logistic_result[n + 1]);
    EXPECT_EQ(99, int_result[0]);
    EXPECT_EQ(99, int_result[n + 1]);
    // The functions can also be used in place.
    std::vector<TFloat> inout(values.begin(), values.begin() + n);
    TanhVector(n, inout.data(), inout.data());
    for (size_t i = 0; i < n; ++i) {
      EXPECT_EQ(tanh_result[i + 1], inout[i]) << "in place, i=" << i;
    }
  }
  EXPECT_EQ(0, std::fetestexcept(FE_INVALID));
}

} // namespace tesseract
