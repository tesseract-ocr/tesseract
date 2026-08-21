///////////////////////////////////////////////////////////////////////
// File:        recoder_test.cc
// Description: Tests that a UnicharCompress (LSTM recoder) with code
//              values outside the sane range is rejected at load.
//              Negative code values leave code_range_ at zero, so
//              SetupDecoder writes is_valid_start_[code(0)] out of
//              bounds on a size-0 vector<bool>; huge code values wrap
//              code_range_ and make resize() throw.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
//
///////////////////////////////////////////////////////////////////////

#include "include_gunit.h"

#include "serialis.h" // for TFile
#include "unicharcompress.h"

#include <cstdint>
#include <vector>

namespace tesseract {
namespace {

// Appends raw little-endian values to a byte buffer.
class ByteWriter {
public:
  void PutU8(uint32_t v) { data_.push_back(static_cast<char>(v & 0xFF)); }
  void PutU32(uint32_t v) {
    for (int i = 0; i < 4; ++i) {
      data_.push_back(static_cast<char>((v >> (8 * i)) & 0xFF));
    }
  }
  void PutS32(int32_t v) { PutU32(static_cast<uint32_t>(v)); }
  const std::vector<char> &data() const { return data_; }

private:
  std::vector<char> data_;
};

// A serialized UnicharCompress with one length-1 RecodedCharID per
// given code value (self-normalizing).
std::vector<char> MakeRecoder(const std::vector<int32_t> &codes) {
  ByteWriter w;
  w.PutU32(codes.size());
  for (int32_t code : codes) {
    w.PutU8(1); // self_normalized_
    w.PutU32(1); // length_
    w.PutS32(code); // code_[0]
  }
  return w.data();
}

// A recoder code of -1 keeps code_range_ at 0, so on unpatched code
// SetupDecoder performs an out-of-bounds write into the size-0
// is_valid_start_ vector<bool>.
TEST(RecoderTest, RejectsNegativeCode) {
  std::vector<char> bytes = MakeRecoder({-1});
  TFile fp;
  ASSERT_TRUE(fp.Open(bytes.data(), bytes.size()));
  UnicharCompress recoder;
  EXPECT_FALSE(recoder.DeSerialize(&fp));
}

// A recoder code of INT32_MAX wraps code_range_ to a negative value,
// so on unpatched code SetupDecoder's resize() throws.
TEST(RecoderTest, RejectsHugeCode) {
  std::vector<char> bytes = MakeRecoder({INT32_MAX});
  TFile fp;
  ASSERT_TRUE(fp.Open(bytes.data(), bytes.size()));
  UnicharCompress recoder;
  EXPECT_FALSE(recoder.DeSerialize(&fp));
}

// A valid recoder must still be accepted and usable.
TEST(RecoderTest, AcceptsValidCodes) {
  std::vector<char> bytes = MakeRecoder({0, 1});
  TFile fp;
  ASSERT_TRUE(fp.Open(bytes.data(), bytes.size()));
  UnicharCompress recoder;
  ASSERT_TRUE(recoder.DeSerialize(&fp));
  EXPECT_EQ(recoder.code_range(), 2);
  EXPECT_TRUE(recoder.IsValidFirstCode(0));
  EXPECT_TRUE(recoder.IsValidFirstCode(1));
}

} // namespace
} // namespace tesseract
