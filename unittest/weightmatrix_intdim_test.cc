///////////////////////////////////////////////////////////////////////
// File:        weightmatrix_intdim_test.cc
// Description: Tests that an int-mode WeightMatrix with a zero first or
//              second dimension is rejected by WeightMatrix::DeSerialize
//              instead of crashing in IntSimdMatrix::Init (which would
//              compute num_in = dim2 - 1 = -1 and read the bias from
//              array_[-1] on an unallocated array).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
//
///////////////////////////////////////////////////////////////////////

#include "include_gunit.h"

#include "intsimdmatrix.h"            // for IntSimdMatrix (active backend)
#include "weightmatrix.h"             // for WeightMatrix
#include "weightmatrix_test_helper.h" // for MakeIntWeightMatrix

#include <cstdint>
#include <string>
#include <vector>

namespace tesseract {
namespace {

using namespace unittest;

// A valid int-mode matrix must still deserialize successfully.
TEST(WeightMatrixIntDimTest, AcceptsValidIntMatrix) {
  WeightMatrix m;
  EXPECT_TRUE(DeserializeWeightMatrix(MakeIntWeightMatrix(2, 2, 2), &m));
}

// A zero second dimension must be rejected instead of crashing in Init.
TEST(WeightMatrixIntDimTest, RejectsZeroDim2) {
  if (IntSimdMatrix::intSimdMatrix == nullptr) {
    GTEST_SKIP() << "no SIMD int backend active; the vulnerable path is unreachable";
  }
  WeightMatrix m;
  EXPECT_FALSE(DeserializeWeightMatrix(MakeIntWeightMatrix(2, 0, 0), &m));
}

// A zero first dimension means that the matrix has no output rows at all,
// so it must be rejected as well.
TEST(WeightMatrixIntDimTest, RejectsZeroDim1) {
  WeightMatrix m;
  EXPECT_FALSE(DeserializeWeightMatrix(MakeIntWeightMatrix(0, 2, 0), &m));
}

} // namespace
} // namespace tesseract
