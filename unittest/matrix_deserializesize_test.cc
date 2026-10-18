///////////////////////////////////////////////////////////////////////
// File:        matrix_deserializesize_test.cc
// Description: Tests that GENERIC_2D_ARRAY::DeSerializeSize rejects
//              negative and oversized dimensions read from a TFile
//              stream. The dimensions are serialized as little-endian
//              int32, so a crafted .traineddata can supply values such
//              as -1 (0xFFFFFFFF) or 65535 that the previous signed
//              "size > UINT16_MAX" check failed to reject. The -1 case
//              made ResizeNoInit compute (-1)*(-1) = 1 and leave the
//              array indexed as dim1_ = dim2_ = -1, while 65535*65535
//              overflowed the 32-bit int used for the element count.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
//
///////////////////////////////////////////////////////////////////////

#include "include_gunit.h"

#include "matrix.h"   // for GENERIC_2D_ARRAY
#include "serialis.h"  // for TFile

#include <cstdint>
#include <vector>

namespace tesseract {
namespace {

// Appends little-endian values to a byte buffer, mirroring the layout
// GENERIC_2D_ARRAY<T>::DeSerialize(TFile*) expects:
// int32 dim1, int32 dim2, T empty_, dim1*dim2 T cells.
class MxWriter {
 public:
  void PutI32(int32_t v) {
    for (int i = 0; i < 4; ++i) data_.push_back(static_cast<char>((v >> (8 * i)) & 0xFF));
  }
  void PutU8(uint8_t v) { data_.push_back(static_cast<char>(v)); }
  const std::vector<char> &data() const { return data_; }

 private:
  std::vector<char> data_;
};

// Builds a GENERIC_2D_ARRAY<int8_t> blob with the given (possibly
// negative or oversized) dimensions and num_cells payload bytes.
std::vector<char> MakeMatrixBlob(int32_t dim1, int32_t dim2, int32_t num_cells) {
  MxWriter w;
  w.PutI32(dim1);
  w.PutI32(dim2);
  w.PutU8(0); // empty_
  for (int32_t i = 0; i < num_cells; ++i) {
    w.PutU8(0);
  }
  return w.data();
}

bool DeSerializeMatrix(const std::vector<char> &blob, GENERIC_2D_ARRAY<int8_t> *m) {
  TFile fp;
  fp.Open(blob.data(), blob.size());
  if (fp.RemainingBytes() != blob.size()) {
    return false;
  }
  return m->DeSerialize(&fp);
}

class MatrixDeserializeSizeTest : public testing::Test {
};

// A valid small matrix must still deserialize successfully.
TEST_F(MatrixDeserializeSizeTest, AcceptsValidMatrix) {
  GENERIC_2D_ARRAY<int8_t> m;
  ASSERT_TRUE(DeSerializeMatrix(MakeMatrixBlob(2, 3, 6), &m));
  EXPECT_EQ(2, m.dim1());
  EXPECT_EQ(3, m.dim2());
}

// A negative dimension must be rejected instead of being passed to
// Resize, where (-1)*(-1) would allocate a 1-element array while the
// dims stay negative (later used as OOB indices).
TEST_F(MatrixDeserializeSizeTest, RejectsNegativeDim) {
  GENERIC_2D_ARRAY<int8_t> m;
  EXPECT_FALSE(DeSerializeMatrix(MakeMatrixBlob(-1, -1, 1), &m));
}

// A negative dimension in either position must be rejected.
TEST_F(MatrixDeserializeSizeTest, RejectsNegativeDim2) {
  GENERIC_2D_ARRAY<int8_t> m;
  EXPECT_FALSE(DeSerializeMatrix(MakeMatrixBlob(3, -1, 0), &m));
}

// Both dimensions at UINT16_MAX must be rejected: the product
// 65535*65535 does not fit in a 32-bit signed int, so it would wrap to
// a negative element count.
TEST_F(MatrixDeserializeSizeTest, RejectsOverflowingProduct) {
  GENERIC_2D_ARRAY<int8_t> m;
  EXPECT_FALSE(DeSerializeMatrix(MakeMatrixBlob(65535, 65535, 0), &m));
}

// A dimension above UINT16_MAX must still be rejected (regression:
// this was already checked before, keep it).
TEST_F(MatrixDeserializeSizeTest, RejectsOversizedDim) {
  GENERIC_2D_ARRAY<int8_t> m;
  EXPECT_FALSE(DeSerializeMatrix(MakeMatrixBlob(65536, 1, 0), &m));
}

// A declared element count that fits in a 32-bit int but is not backed
// by the bytes in the stream must be rejected instead of forcing a
// huge allocation (denial of service from a tiny crafted file).
TEST_F(MatrixDeserializeSizeTest, RejectsDeclaredSizeLargerThanStream) {
  // 32768 * 65535 = 2147450880 < INT32_MAX, but the blob carries only a
  // handful of payload bytes.
  GENERIC_2D_ARRAY<int8_t> m;
  EXPECT_FALSE(DeSerializeMatrix(MakeMatrixBlob(32768, 65535, 8), &m));
}

// A large matrix whose declared size is fully present in the stream
// must still deserialize successfully.
TEST_F(MatrixDeserializeSizeTest, AcceptsLargeFullyBackedMatrix) {
  GENERIC_2D_ARRAY<int8_t> m;
  ASSERT_TRUE(DeSerializeMatrix(MakeMatrixBlob(65535, 1, 65535), &m));
  EXPECT_EQ(65535, m.dim1());
  EXPECT_EQ(1, m.dim2());
}

// ResizeNoInit itself must bound the element count, for callers that
// have no stream to check against. The maximum allowed size must
// still be accepted.
TEST_F(MatrixDeserializeSizeTest, ResizeNoInitAcceptsMaxSize) {
  GENERIC_2D_ARRAY<int8_t> m;
  m.ResizeNoInit(10000, 10000); // exactly kMaxElements
  EXPECT_EQ(10000, m.dim1());
  EXPECT_EQ(10000, m.dim2());
}

// A size above the DoS cap must be rejected even though the product
// fits in a 32-bit int (10001 * 10000 = 100010000).
TEST_F(MatrixDeserializeSizeTest, ResizeNoInitRejectsOverCap) {
  GENERIC_2D_ARRAY<int8_t> m;
  m.ResizeNoInit(10001, 10000);
  EXPECT_EQ(0, m.dim1());
  EXPECT_EQ(0, m.dim2());
}

// A negative size must be rejected (no allocation, no state change).
TEST_F(MatrixDeserializeSizeTest, ResizeNoInitRejectsNegative) {
  GENERIC_2D_ARRAY<int8_t> m;
  m.ResizeNoInit(-1, -1);
  EXPECT_EQ(0, m.dim1());
  EXPECT_EQ(0, m.dim2());
}

// The 65535 x 65535 overflow case must be rejected as well.
TEST_F(MatrixDeserializeSizeTest, ResizeNoInitRejectsOverflowingProduct) {
  GENERIC_2D_ARRAY<int8_t> m;
  m.ResizeNoInit(65535, 65535);
  EXPECT_EQ(0, m.dim1());
  EXPECT_EQ(0, m.dim2());
}

} // namespace
} // namespace tesseract
