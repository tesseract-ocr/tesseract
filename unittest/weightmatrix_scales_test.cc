///////////////////////////////////////////////////////////////////////
// File:        weightmatrix_scales_test.cc
// Description: Tests that an int-mode WeightMatrix whose scale vector is
//              shorter than the number of output rows (wi_.dim1()) is
//              rejected by WeightMatrix::DeSerialize instead of being
//              read out of bounds later by the generic
//              (non-SIMD) IntSimdMatrix::MatrixDotVector.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
//
///////////////////////////////////////////////////////////////////////

#include "include_gunit.h"

#include "weightmatrix.h"             // for WeightMatrix
#include "weightmatrix_test_helper.h" // for MakeIntWeightMatrix

#include <cstdint>
#include <string>
#include <vector>

namespace tesseract {
namespace {

using namespace unittest;

// One scale per output row (a generic-trained model) must be accepted.
TEST(WeightMatrixScalesTest, AcceptsScalesMatchingDim1) {
  WeightMatrix m;
  EXPECT_TRUE(DeserializeWeightMatrix(MakeIntWeightMatrix(2, 2, 2), &m));
}

// More scales than rows (a SIMD-trained model stores the rounded-up count)
// must still be accepted.
TEST(WeightMatrixScalesTest, AcceptsRoundedScales) {
  WeightMatrix m;
  EXPECT_TRUE(DeserializeWeightMatrix(MakeIntWeightMatrix(2, 2, 4), &m));
}

// No scales at all: the generic MatrixDotVector would read scales_[0] past
// the end, so the load must be rejected.
TEST(WeightMatrixScalesTest, RejectsNoScales) {
  WeightMatrix m;
  EXPECT_FALSE(DeserializeWeightMatrix(MakeIntWeightMatrix(2, 2, 0), &m));
}

// Fewer scales than output rows: still out of bounds for the generic
// reader, so the load must be rejected.
TEST(WeightMatrixScalesTest, RejectsTooFewScales) {
  WeightMatrix m;
  EXPECT_FALSE(DeserializeWeightMatrix(MakeIntWeightMatrix(2, 2, 1), &m));
}

// The old (float) format carries the scales as floats and has the same
// requirement of one scale per output row.
TEST(WeightMatrixScalesTest, RejectsLegacyShortScales) {
  WeightMatrix m;
  EXPECT_FALSE(DeserializeWeightMatrix(MakeLegacyIntWeightMatrix(2, 2, 1), &m));
}

// A legacy matrix with a full scale vector must still load. With
// FAST_FLOAT the old format is rejected outright, so this test only
// guards the double precision code path.
#ifndef FAST_FLOAT
TEST(WeightMatrixScalesTest, AcceptsLegacyFullScales) {
  WeightMatrix m;
  EXPECT_TRUE(DeserializeWeightMatrix(MakeLegacyIntWeightMatrix(2, 2, 2), &m));
}
#endif

} // namespace
} // namespace tesseract
