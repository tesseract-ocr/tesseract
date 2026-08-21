///////////////////////////////////////////////////////////////////////
// File:        genericvector_test.cc
// Description: Tests that the callback form of GenericVector::read
//              rejects vectors whose size_used_ exceeds reserved (or
//              whose counts are negative). reserved sizes the buffer
//              while size_used_ is an independent file field driving
//              the element loop, so a crafted .traineddata (e.g. the
//              fontinfo table of a version >= 4 inttemp component)
//              performs a heap out-of-bounds write during legacy
//              engine initialization.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
//
///////////////////////////////////////////////////////////////////////

#include "include_gunit.h"

#include "genericvector.h"
#include "serialis.h" // for TFile

#include <cstdint>
#include <vector>

namespace tesseract {
namespace {

// Appends raw little-endian values to a byte buffer.
class ByteWriter {
public:
  void PutS32(int32_t v) {
    uint32_t u = static_cast<uint32_t>(v);
    for (int i = 0; i < 4; ++i) {
      data_.push_back(static_cast<char>((u >> (8 * i)) & 0xFF));
    }
  }
  const std::vector<char> &data() const { return data_; }

private:
  std::vector<char> data_;
};

// A serialized vector header (reserved, size_used_) followed by the
// given number of int32 elements.
std::vector<char> MakeVector(int32_t reserved, int32_t size_used, int32_t num_elements) {
  ByteWriter w;
  w.PutS32(reserved);
  w.PutS32(size_used);
  for (int32_t i = 0; i < num_elements; ++i) {
    w.PutS32(i);
  }
  return w.data();
}

// reserved=4 but size_used_=0x10000: on unpatched code the callback
// loop writes 65536 ints past the 4-int buffer (heap out-of-bounds
// write).
TEST(GenericVectorTest, RejectsSizeUsedBeyondReserved) {
  std::vector<char> bytes = MakeVector(4, 0x10000, 0x10000);
  TFile fp;
  ASSERT_TRUE(fp.Open(bytes.data(), bytes.size()));
  GenericVector<int> v;
  EXPECT_FALSE(v.read(&fp, [](TFile *f, int *p) { return f->DeSerialize(p); }));
}

// Negative counts must be rejected; on unpatched code the read
// "succeeds" and leaves size_used_ negative.
TEST(GenericVectorTest, RejectsNegativeCounts) {
  std::vector<char> bytes = MakeVector(-1, -1, 0);
  TFile fp;
  ASSERT_TRUE(fp.Open(bytes.data(), bytes.size()));
  GenericVector<int> v;
  EXPECT_FALSE(v.read(&fp, [](TFile *f, int *p) { return f->DeSerialize(p); }));
}

// A consistent vector must still be accepted.
TEST(GenericVectorTest, AcceptsConsistentVector) {
  std::vector<char> bytes = MakeVector(4, 2, 2);
  TFile fp;
  ASSERT_TRUE(fp.Open(bytes.data(), bytes.size()));
  GenericVector<int> v;
  ASSERT_TRUE(v.read(&fp, [](TFile *f, int *p) { return f->DeSerialize(p); }));
  EXPECT_EQ(v.size(), 2);
  EXPECT_EQ(v[0], 0);
  EXPECT_EQ(v[1], 1);
}

} // namespace
} // namespace tesseract
