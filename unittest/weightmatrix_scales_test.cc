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

#include "serialis.h"     // for TFile
#include "weightmatrix.h" // for WeightMatrix

#include <cstdint>
#include <string>
#include <vector>

namespace tesseract {
namespace {

// Appends little-endian values to a byte buffer.
class WmWriter {
 public:
  void PutU8(uint8_t v) { data_.push_back(static_cast<char>(v)); }
  void PutI32(int32_t v) {
    for (int i = 0; i < 4; ++i) data_.push_back(static_cast<char>((v >> (8 * i)) & 0xFF));
  }
  void PutU32(uint32_t v) { PutI32(static_cast<int32_t>(v)); }
  const std::vector<char> &data() const { return data_; }

 private:
  std::vector<char> data_;
};

// Builds an int-mode (double/float format, kDoubleFlag | kInt8Flag)
// WeightMatrix blob with the given wi_ dimensions and the given number of
// scales. Layout mirrors WeightMatrix::DeSerialize: mode byte, then the wi_
// (GENERIC_2D_ARRAY<int8_t>) DeSerialize: int32 dim1, int32 dim2, empty_
// (int8) and dim1*dim2 int8 cells, then the scale count (uint32) and the
// scales as doubles.
std::vector<char> MakeIntWeightMatrix(int32_t dim1, int32_t dim2, int32_t num_scales) {
  WmWriter w;
  w.PutU8(128 + 1); // kDoubleFlag | kInt8Flag
  w.PutI32(dim1);
  w.PutI32(dim2);
  w.PutU8(0); // empty_ (unused cell)
  for (int32_t i = 0; i < dim1 * dim2; ++i) {
    w.PutU8(0);
  }
  w.PutU32(static_cast<uint32_t>(num_scales));
  for (int32_t i = 0; i < num_scales; ++i) {
    union {
      double d;
      uint64_t u;
    } conv;
    conv.d = 1.0;
    w.PutU32(static_cast<uint32_t>(conv.u & 0xFFFFFFFF));
    w.PutU32(static_cast<uint32_t>(conv.u >> 32));
  }
  return w.data();
}

class WeightMatrixScalesTest : public testing::Test {
 protected:
  bool DeSerialize(const std::vector<char> &blob, WeightMatrix *m) {
    TFile fp;
    fp.Open(blob.data(), blob.size());
    if (fp.RemainingBytes() != blob.size()) {
      return false;
    }
    return m->DeSerialize(false, &fp);
  }
};

// One scale per output row (a generic-trained model) must be accepted.
TEST_F(WeightMatrixScalesTest, AcceptsScalesMatchingDim1) {
  WeightMatrix m;
  EXPECT_TRUE(DeSerialize(MakeIntWeightMatrix(2, 2, 2), &m));
}

// More scales than rows (a SIMD-trained model stores the rounded-up count)
// must still be accepted.
TEST_F(WeightMatrixScalesTest, AcceptsRoundedScales) {
  WeightMatrix m;
  EXPECT_TRUE(DeSerialize(MakeIntWeightMatrix(2, 2, 4), &m));
}

// No scales at all: the generic MatrixDotVector would read scales_[0] past
// the end, so the load must be rejected.
TEST_F(WeightMatrixScalesTest, RejectsNoScales) {
  WeightMatrix m;
  EXPECT_FALSE(DeSerialize(MakeIntWeightMatrix(2, 2, 0), &m));
}

// Fewer scales than output rows: still out of bounds for the generic
// reader, so the load must be rejected.
TEST_F(WeightMatrixScalesTest, RejectsTooFewScales) {
  WeightMatrix m;
  EXPECT_FALSE(DeSerialize(MakeIntWeightMatrix(2, 2, 1), &m));
}

} // namespace
} // namespace tesseract
