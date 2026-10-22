///////////////////////////////////////////////////////////////////////
// File:        weightmatrix_test_helper.h
// Description: Helpers to craft WeightMatrix blobs for the unit tests and
//              to run them through WeightMatrix::DeSerialize.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
//
///////////////////////////////////////////////////////////////////////

#ifndef TESSERACT_UNITTEST_WEIGHTMATRIX_TEST_HELPER_H_
#define TESSERACT_UNITTEST_WEIGHTMATRIX_TEST_HELPER_H_

#include "serialis.h"     // for TFile
#include "weightmatrix.h" // for WeightMatrix

#include <cstdint>
#include <vector>

namespace tesseract {
namespace unittest {

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
inline std::vector<char> MakeIntWeightMatrix(int32_t dim1, int32_t dim2,
                                             int32_t num_scales) {
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

// Builds an int-mode WeightMatrix blob in the old (float) format, i.e. one
// whose mode byte has kInt8Flag but not kDoubleFlag. The old format stores
// the scales as floats instead of doubles.
inline std::vector<char> MakeLegacyIntWeightMatrix(int32_t dim1, int32_t dim2,
                                                    int32_t num_scales) {
  WmWriter w;
  w.PutU8(1); // kInt8Flag only, so the old format is selected
  w.PutI32(dim1);
  w.PutI32(dim2);
  w.PutU8(0); // empty_ (unused cell)
  for (int32_t i = 0; i < dim1 * dim2; ++i) {
    w.PutU8(0);
  }
  w.PutU32(static_cast<uint32_t>(num_scales));
  for (int32_t i = 0; i < num_scales; ++i) {
    union {
      float f;
      uint32_t u;
    } conv;
    conv.f = 1.0f;
    w.PutU32(conv.u);
  }
  return w.data();
}

// Feeds the given blob to WeightMatrix::DeSerialize. Returns false if the
// matrix could not be read at all or if the blob was not consumed
// completely, so that a change of the serialized layout is detected.
inline bool DeserializeWeightMatrix(const std::vector<char> &blob,
                                     WeightMatrix *m) {
  TFile fp;
  fp.Open(blob.data(), blob.size());
  if (!m->DeSerialize(false, &fp)) {
    return false;
  }
  return fp.RemainingBytes() == 0;
}

} // namespace unittest
} // namespace tesseract

#endif // TESSERACT_UNITTEST_WEIGHTMATRIX_TEST_HELPER_H_
