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

#include <cfenv>
#include <cmath>

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

} // namespace tesseract
