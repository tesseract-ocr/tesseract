///////////////////////////////////////////////////////////////////////
// File:        weightmatrix_intdim_test.cc
// Description: Tests that an int-mode WeightMatrix whose second
//              dimension is 0 is rejected by WeightMatrix::DeSerialize
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

#include "intsimdmatrix.h" // for IntSimdMatrix (active backend)
#include "serialis.h"      // for TFile
#include "weightmatrix.h"  // for WeightMatrix

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
// WeightMatrix blob with the given wi_ dimensions and scale vector.
// Layout mirrors WeightMatrix::DeSerialize: mode byte, then the wi_
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

class WeightMatrixIntDimTest : public testing::Test {
 protected:
  // Runs DeSerialize on the given blob. Only meaningful when a SIMD int
  // backend is active, since that is what triggers the vulnerable Init path.
  bool DeSerialize(const std::vector<char> &blob, WeightMatrix *m) {
    TFile fp;
    fp.Open(blob.data(), blob.size());
    if (fp.RemainingBytes() != blob.size()) {
      return false;
    }
    return m->DeSerialize(false, &fp);
  }
};

// A valid int-mode matrix must still deserialize successfully.
TEST_F(WeightMatrixIntDimTest, AcceptsValidIntMatrix) {
  WeightMatrix m;
  EXPECT_TRUE(DeSerialize(MakeIntWeightMatrix(2, 2, 2), &m));
}

// A zero second dimension must be rejected instead of crashing in Init.
TEST_F(WeightMatrixIntDimTest, RejectsZeroDim2) {
  if (IntSimdMatrix::intSimdMatrix == nullptr) {
    GTEST_SKIP() << "no SIMD int backend active; the vulnerable path is unreachable";
  }
  WeightMatrix m;
  EXPECT_FALSE(DeSerialize(MakeIntWeightMatrix(2, 0, 0), &m));
}

} // namespace
} // namespace tesseract
