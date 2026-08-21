///////////////////////////////////////////////////////////////////////
// File:        lstm_layer_test.cc
// Description: Tests that an NT_LSTM network layer with mutually
//              inconsistent deserialized dimensions is rejected at
//              load. The forward pass sizes its buffers from na_, no_
//              and ns_ while the gate weight matrices drive their own
//              dimensions, so a crafted .traineddata performs heap
//              out-of-bounds writes/reads during the first
//              recognition step (e.g. WriteTimeStepPart writing ns_
//              floats into a source_ buffer sized from na_).
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
#include "stridemap.h"

#include <cstdint>
#include <utility>
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

// A serialized float-mode WeightMatrix with the given dimensions,
// all weight data zeroed.
void PutGateMatrix(ByteWriter *w, int32_t dim1, int32_t dim2) {
  w->PutU8(128); // mode: kDoubleFlag, float mode
  w->PutS32(dim1);
  w->PutS32(dim2);
  PutDoubleLE(w, 0.0); // empty_ cell
  for (int32_t i = 0; i < dim1 * dim2; ++i) {
    PutDoubleLE(w, 0.0);
  }
}

// A serialized 1-D NT_LSTM network: header with the given ni/no, na_,
// then the four gates CI, GI, GF1, GO (GFS is not serialized for 1-D).
std::vector<char> MakeLstmNetwork(int ni, int no, int32_t na,
                                  const int32_t gate_dim1[4], const int32_t gate_dim2[4]) {
  ByteWriter w;
  w.PutU8(static_cast<uint32_t>(NT_LSTM));
  w.PutU8(0); // training: TS_DISABLED
  w.PutU8(0); // needs_to_backprop
  w.PutU32(0); // network_flags
  w.PutU32(static_cast<uint32_t>(ni));
  w.PutU32(static_cast<uint32_t>(no));
  w.PutU32(0); // num_weights
  w.PutU32(0); // name (empty string)
  w.PutS32(na);
  for (int g = 0; g < 4; ++g) {
    PutGateMatrix(&w, gate_dim1[g], gate_dim2[g]);
  }
  return w.data();
}

// Builds the input a standalone LSTM layer would receive: one row of
// the given width with ni features.
NetworkIO MakeInput(int ni, int width) {
  StrideMap stride_map;
  stride_map.SetStride({{1, width}});
  NetworkIO input;
  input.ResizeToMap(false, stride_map, ni);
  return input;
}

// Runs Forward on the loaded network; on unpatched code the out-of-
// bounds access this regression test guards against fires here.
void RunForward(Network *net, int ni, int width) {
  NetworkIO input = MakeInput(ni, width);
  NetworkScratch scratch;
  NetworkIO output;
  net->Forward(false, input, nullptr, &scratch, &output);
  delete net;
}

// na_ must equal ni_ + nf_ + ns_ for a 1-D LSTM; here na_=2 but the
// CI matrix makes ns_=64, so the layer must be rejected. On unpatched
// code Forward writes 64 floats at offset ni_=1 into a source_ buffer
// sized for na_=2 (heap out-of-bounds write).
TEST(LstmLayerTest, RejectsInconsistentNa) {
  const int32_t dim1[4] = {64, 64, 64, 64};
  const int32_t dim2[4] = {3, 3, 3, 3};
  std::vector<char> bytes = MakeLstmNetwork(1, 1, 2, dim1, dim2);
  TFile fp;
  ASSERT_TRUE(fp.Open(bytes.data(), bytes.size()));
  Network *net = Network::CreateFromFile(&fp);
  if (net == nullptr) {
    return; // Fixed: the inconsistent layer is rejected at load.
  }
  RunForward(net, 1, 2);
  FAIL() << "crafted LSTM layer with inconsistent na_ was accepted";
}

// All gate matrices must have dim1 == ns_; here the GI matrix has
// dim1=9 while ns_=5. On unpatched code the GI gate dot product writes
// 9 results into a temp line sized for 5 (heap out-of-bounds write).
TEST(LstmLayerTest, RejectsGateDim1Mismatch) {
  const int32_t dim1[4] = {5, 9, 5, 5};
  const int32_t dim2[4] = {7, 7, 7, 7};
  std::vector<char> bytes = MakeLstmNetwork(1, 5, 6, dim1, dim2);
  TFile fp;
  ASSERT_TRUE(fp.Open(bytes.data(), bytes.size()));
  Network *net = Network::CreateFromFile(&fp);
  if (net == nullptr) {
    return; // Fixed: the inconsistent layer is rejected at load.
  }
  RunForward(net, 1, 2);
  FAIL() << "crafted LSTM layer with inconsistent gate dim1 was accepted";
}

// All gate matrices must have dim2 == na_ + 1; here the GI matrix has
// dim2=9 while na_=6. On unpatched code the GI gate dot product reads
// 8 inputs from a buffer sized for 6 (heap out-of-bounds read).
TEST(LstmLayerTest, RejectsGateDim2Mismatch) {
  const int32_t dim1[4] = {5, 5, 5, 5};
  const int32_t dim2[4] = {7, 9, 7, 7};
  std::vector<char> bytes = MakeLstmNetwork(1, 5, 6, dim1, dim2);
  TFile fp;
  ASSERT_TRUE(fp.Open(bytes.data(), bytes.size()));
  Network *net = Network::CreateFromFile(&fp);
  if (net == nullptr) {
    return; // Fixed: the inconsistent layer is rejected at load.
  }
  RunForward(net, 1, 2);
  FAIL() << "crafted LSTM layer with inconsistent gate dim2 was accepted";
}

// A fully consistent 1-D LSTM layer must still be accepted and usable.
TEST(LstmLayerTest, AcceptsConsistentLayer) {
  const int32_t dim1[4] = {5, 5, 5, 5};
  const int32_t dim2[4] = {7, 7, 7, 7};
  std::vector<char> bytes = MakeLstmNetwork(1, 5, 6, dim1, dim2);
  TFile fp;
  ASSERT_TRUE(fp.Open(bytes.data(), bytes.size()));
  Network *net = Network::CreateFromFile(&fp);
  ASSERT_NE(net, nullptr);
  RunForward(net, 1, 2);
}

} // namespace
} // namespace tesseract
