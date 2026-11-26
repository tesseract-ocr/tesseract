///////////////////////////////////////////////////////////////////////
// File:        fullyconnected_test.cc
// Description: Tests that a FullyConnected (softmax) network layer with
//              weight-matrix dimensions that do not match the declared
//              ni/no is rejected at load. Without the check,
//              MatrixDotVector writes w.dim1() results into a scratch
//              buffer sized from no_ and reads w.dim2()-1 inputs from
//              a buffer sized from ni_ (heap out-of-bounds write/read)
//              on the first recognition step.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
//
///////////////////////////////////////////////////////////////////////

#include "include_gunit.h"

#include "network.h" // for Network, NetworkType
#include "networkio.h"
#include "networkscratch.h"
#include "serialis.h" // for TFile

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

void PutDoubleLE(ByteWriter *w, double d) {
  union {
    double d;
    uint64_t u;
  } conv;
  conv.d = d;
  w->PutU32(static_cast<uint32_t>(conv.u & 0xFFFFFFFF));
  w->PutU32(static_cast<uint32_t>(conv.u >> 32));
}

// Builds a serialized NT_SOFTMAX network: header with the given ni/no,
// then a float-mode WeightMatrix with the given (corrupt) dimensions.
// The matrix is stored as doubles on disk (see WeightMatrix::DeSerialize).
std::vector<char> MakeSoftmaxNetwork(int ni, int no, int32_t dim1, int32_t dim2) {
  ByteWriter w;
  w.PutU8(static_cast<uint32_t>(NT_SOFTMAX));
  w.PutU8(0); // training: TS_DISABLED
  w.PutU8(0); // needs_to_backprop
  w.PutU32(0); // network_flags
  w.PutU32(static_cast<uint32_t>(ni));
  w.PutU32(static_cast<uint32_t>(no));
  w.PutU32(0); // num_weights (not cross-checked, kept consistent anyway)
  w.PutU32(0); // name (empty string)
  // WeightMatrix::DeSerialize:
  w.PutU8(128); // mode: kDoubleFlag, float mode
  w.PutS32(dim1);
  w.PutS32(dim2);
  PutDoubleLE(&w, 0.0); // empty_ cell
  for (int32_t i = 0; i < dim1 * dim2; ++i) {
    PutDoubleLE(&w, 0.0); // weight data
  }
  return w.data();
}

// A FullyConnected layer whose weight matrix does not match the declared
// sizes must be rejected by CreateFromFile; on unpatched code the test
// reaches Forward, where MatrixDotVector performs the out-of-bounds
// write this regression test guards against.
TEST(FullyconnectedTest, RejectsWeightMatrixDimensionMismatch) {
  // ni_=no_=1 but dim1=3 (OOB write of 3 results into a 1-result buffer)
  // and dim2=5 (OOB read of 4 inputs from a 1-input buffer).
  std::vector<char> bytes = MakeSoftmaxNetwork(1, 1, 3, 5);
  TFile fp;
  ASSERT_TRUE(fp.Open(bytes.data(), bytes.size()));
  Network *net = Network::CreateFromFile(&fp);
  if (net == nullptr) {
    return; // Fixed: the mismatched layer is rejected at load.
  }
  NetworkIO input;
  input.Resize2d(false, /*width=*/1, /*num_features=*/1);
  NetworkScratch scratch;
  NetworkIO output;
  net->Forward(false, input, nullptr, &scratch, &output);
  delete net;
  FAIL() << "crafted FullyConnected layer with mismatched weight matrix was accepted";
}

// Consistent dimensions must still be accepted.
TEST(FullyconnectedTest, AcceptsMatchingDimensions) {
  std::vector<char> bytes = MakeSoftmaxNetwork(1, 2, 2, 2);
  TFile fp;
  ASSERT_TRUE(fp.Open(bytes.data(), bytes.size()));
  Network *net = Network::CreateFromFile(&fp);
  ASSERT_NE(net, nullptr);
  delete net;
}

} // namespace
} // namespace tesseract
