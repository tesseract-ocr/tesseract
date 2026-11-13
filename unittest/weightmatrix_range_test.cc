///////////////////////////////////////////////////////////////////////
// File:        weightmatrix_range_test.cc
// Description: Tests that a WeightMatrix whose serialized doubles do not
//              fit a float (beyond FLT_MAX, infinite or NaN) is rejected
//              by WeightMatrix::DeSerialize when built with FAST_FLOAT
//              (the default), instead of being narrowed to float with a
//              floating-point exception that the tesseract program traps.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
//
///////////////////////////////////////////////////////////////////////

#include "include_gunit.h"

#include "serialis.h"                 // for TFile
#include "weightmatrix.h"             // for WeightMatrix
#include "weightmatrix_test_helper.h" // for DeserializeWeightMatrix

#include <cfloat>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <vector>

namespace tesseract {
namespace {

using namespace unittest;

// Overwrites the double stored at the given byte offset of a blob.
void PutDouble(std::vector<char> &blob, size_t offset, double value) {
  ASSERT_LE(offset + sizeof(value), blob.size());
  std::memcpy(&blob[offset], &value, sizeof(value));
}

// Serializes a valid float-mode WeightMatrix with WeightMatrix::Serialize
// and replaces its first weight. Layout as written by Serialize: the mode
// byte, then the wf_ (GENERIC_2D_ARRAY<double>) Serialize: int32 dim1,
// int32 dim2, empty_ (double) and dim1*dim2 doubles.
std::vector<char> MakeFloatWeightMatrix(double first_weight) {
  WeightMatrix m;
  m.InitWeightsFloat(3, 4, false, 1.0f, nullptr);
  std::vector<char> blob;
  TFile fp;
  fp.OpenWrite(&blob);
  EXPECT_TRUE(m.Serialize(false, &fp));
  PutDouble(blob, 1 + 4 + 4 + 8, first_weight);
  return blob;
}

// Builds an int-mode WeightMatrix blob (2 x 2, two scales) and replaces
// its first scale. Layout as in MakeIntWeightMatrix: the mode byte, int32
// dim1, int32 dim2, empty_ (int8), dim1*dim2 int8 cells and the scale
// count (uint32) before the scales.
std::vector<char> MakeIntWeightMatrixWithScale(double first_scale) {
  std::vector<char> blob = MakeIntWeightMatrix(2, 2, 2);
  PutDouble(blob, 1 + 4 + 4 + 1 + 2 * 2 + 4, first_scale);
  return blob;
}

// Weights that fit a float must still be accepted.
TEST(WeightMatrixRangeTest, AcceptsWeightsInFloatRange) {
  WeightMatrix m;
  EXPECT_TRUE(DeserializeWeightMatrix(MakeFloatWeightMatrix(1.0), &m));
  EXPECT_TRUE(DeserializeWeightMatrix(MakeFloatWeightMatrix(-1.0), &m));
  EXPECT_TRUE(DeserializeWeightMatrix(MakeFloatWeightMatrix(0.0), &m));
  EXPECT_TRUE(DeserializeWeightMatrix(MakeFloatWeightMatrix(FLT_MAX), &m));
  EXPECT_TRUE(DeserializeWeightMatrix(MakeFloatWeightMatrix(-FLT_MAX), &m));
  EXPECT_TRUE(DeserializeWeightMatrix(MakeFloatWeightMatrix(1e-300), &m));
}

// A finite weight beyond FLT_MAX overflows when narrowed to float.
TEST(WeightMatrixRangeTest, RejectsWeightsBeyondFloatRange) {
#ifdef FAST_FLOAT
  WeightMatrix m;
  EXPECT_FALSE(DeserializeWeightMatrix(MakeFloatWeightMatrix(1e300), &m));
  EXPECT_FALSE(DeserializeWeightMatrix(MakeFloatWeightMatrix(-1e300), &m));
  EXPECT_FALSE(DeserializeWeightMatrix(MakeFloatWeightMatrix(3.5e38), &m));
  EXPECT_FALSE(DeserializeWeightMatrix(
      MakeFloatWeightMatrix(std::nextafter(static_cast<double>(FLT_MAX), 1e300)), &m));
#else
  GTEST_SKIP() << "FAST_FLOAT not enabled; weights are not narrowed to float";
#endif
}

// Infinities and NaNs, quiet or signaling, are not usable weights either.
TEST(WeightMatrixRangeTest, RejectsNonFiniteWeights) {
#ifdef FAST_FLOAT
  const uint64_t snan_bits = UINT64_C(0x7ff4000000000000);
  double snan;
  std::memcpy(&snan, &snan_bits, sizeof(snan));
  WeightMatrix m;
  EXPECT_FALSE(DeserializeWeightMatrix(MakeFloatWeightMatrix(INFINITY), &m));
  EXPECT_FALSE(DeserializeWeightMatrix(MakeFloatWeightMatrix(-INFINITY), &m));
  EXPECT_FALSE(DeserializeWeightMatrix(MakeFloatWeightMatrix(NAN), &m));
  EXPECT_FALSE(DeserializeWeightMatrix(MakeFloatWeightMatrix(snan), &m));
#else
  GTEST_SKIP() << "FAST_FLOAT not enabled; weights are not narrowed to float";
#endif
}

// The scales of an int-mode matrix are narrowed to float the same way.
TEST(WeightMatrixRangeTest, RejectsScalesBeyondFloatRange) {
  WeightMatrix m;
  EXPECT_TRUE(DeserializeWeightMatrix(MakeIntWeightMatrixWithScale(1.0), &m));
#ifdef FAST_FLOAT
  EXPECT_FALSE(DeserializeWeightMatrix(MakeIntWeightMatrixWithScale(1e300), &m));
  EXPECT_FALSE(DeserializeWeightMatrix(MakeIntWeightMatrixWithScale(INFINITY), &m));
  EXPECT_FALSE(DeserializeWeightMatrix(MakeIntWeightMatrixWithScale(NAN), &m));
#endif
}

} // namespace
} // namespace tesseract
