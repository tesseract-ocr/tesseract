///////////////////////////////////////////////////////////////////////
// File:        weightmatrix_deserialize_test.cc
// Description: Tests that a WeightMatrix in the old (float) serialization
//              format, i.e. one whose mode byte lacks kDoubleFlag, is
//              rejected by WeightMatrix::DeSerialize with a false return
//              instead of aborting the process via ASSERT_HOST when built
//              with FAST_FLOAT (the default).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
//
///////////////////////////////////////////////////////////////////////

#include "include_gunit.h"

#include "serialis.h"                // for TFile
#include "weightmatrix.h"             // for WeightMatrix
#include "weightmatrix_test_helper.h" // for DeserializeWeightMatrix

#include <cstdint>
#include <vector>

namespace tesseract {
namespace {

using namespace unittest;

// Serializes a valid float-mode WeightMatrix with WeightMatrix::Serialize,
// then patches the mode byte (the first byte of the blob) to the given
// value, emulating a model file whose weight matrix declares the old
// (float) format. The rest of the blob is a valid serialization, so the
// only thing that differs between the test cases is the mode byte.
std::vector<char> MakeWeightMatrixBlob(uint8_t mode) {
  WeightMatrix m;
  m.InitWeightsFloat(3, 4, false, 1.0f, nullptr);
  std::vector<char> blob;
  TFile fp;
  fp.OpenWrite(&blob);
  EXPECT_TRUE(m.Serialize(false, &fp));
  EXPECT_FALSE(blob.empty());
  blob[0] = static_cast<char>(mode);
  return blob;
}

// Feeds a blob with the given mode byte to WeightMatrix::DeSerialize,
// which reads the mode byte first, exactly like the layer DeSerialize
// functions do.
bool DeSerializeWithMode(uint8_t mode, WeightMatrix *m) {
  return DeserializeWeightMatrix(MakeWeightMatrixBlob(mode), m);
}

// A current-format (kDoubleFlag) weight matrix must still deserialize
// successfully.
TEST(WeightMatrixDeSerializeTest, AcceptsDoubleFlagMode) {
  WeightMatrix m;
  EXPECT_TRUE(DeSerializeWithMode(128, &m)); // kDoubleFlag
  EXPECT_EQ(3, m.Dim1());
  EXPECT_EQ(4, m.Dim2());
}

// A mode byte without kDoubleFlag selects the old (float) format, which is
// not supported with FAST_FLOAT. It must be rejected with a false return
// instead of aborting the process.
TEST(WeightMatrixDeSerializeTest, RejectsOldFormatMode) {
#ifdef FAST_FLOAT
  WeightMatrix m;
  EXPECT_FALSE(DeSerializeWithMode(0, &m));
#else
  // Without FAST_FLOAT the old format is supported, so this test only
  // guards the FAST_FLOAT code path.
  GTEST_SKIP() << "FAST_FLOAT not enabled; the old format is supported";
#endif
}

// Same, with the other mode flag bits set, as in a crafted file.
TEST(WeightMatrixDeSerializeTest, RejectsOldFormatModeWithFlags) {
#ifdef FAST_FLOAT
  WeightMatrix m;
  EXPECT_FALSE(DeSerializeWithMode(1, &m)); // kInt8Flag only
  EXPECT_FALSE(DeSerializeWithMode(4, &m)); // kAdamFlag only
  EXPECT_FALSE(DeSerializeWithMode(5, &m)); // kInt8Flag | kAdamFlag
#else
  GTEST_SKIP() << "FAST_FLOAT not enabled; the old format is supported";
#endif
}

} // namespace
} // namespace tesseract
