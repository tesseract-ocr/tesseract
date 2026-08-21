///////////////////////////////////////////////////////////////////////
// File:        plumbing_test.cc
// Description: Tests that a corrupt TESSDATA_LSTM component in a
//              .traineddata file is rejected without crashing. A
//              plumbing layer (Series/Parallel/Reversed) with an
//              empty or undersized network stack would make
//              XScaleFactor/CacheXScaleFactor dereference stack_[0]
//              during engine initialization.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
//
///////////////////////////////////////////////////////////////////////

#include "include_gunit.h"

#include <tesseract/baseapi.h>

#include "network.h"        // for NetworkType
#include "tessdatamanager.h" // for TessdataManager, TESSDATA_LSTM

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

namespace tesseract {
namespace {

// Minimal unicharset with the special codes (space, Joined, Broken) that
// UNICHARSET::load_from_file expects, as embedded in the TESSDATA_LSTM
// component by LSTMRecognizer::Serialize.
const char kMinUnicharset[] =
    "3\n"
    "NULL 0 NULL 0\n"
    "Joined 7 0,69,188,255,486,1218,0,30,486,1188 Latin 26 0 98 Joined\n"
    "|Broken|0|1 f 0,69,186,255,892,2138,0,80,892,2058 Common 84 10 84 |Broken|0|1\n";

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
  void PutString(const char *s) {
    PutU32(static_cast<uint32_t>(std::strlen(s)));
    data_.insert(data_.end(), s, s + std::strlen(s));
  }
  void PutRaw(const char *s) { data_.insert(data_.end(), s, s + std::strlen(s)); }
  const std::vector<char> &data() const { return data_; }

private:
  std::vector<char> data_;
};

// Serialized network header as written by Network::Serialize:
//   int8 type, int8 training, int8 needs_to_backprop, int32 network_flags,
//   int32 ni, int32 no, int32 num_weights, string name.
void AppendNetworkHeader(ByteWriter *w, NetworkType type) {
  w->PutU8(static_cast<uint32_t>(type));
  w->PutU8(0); // training: TS_DISABLED
  w->PutU8(0); // needs_to_backprop
  w->PutU32(0); // network_flags
  w->PutU32(0); // ni
  w->PutU32(0); // no
  w->PutU32(0); // num_weights
  w->PutString(""); // name
}

// A minimal valid child network (NT_INPUT with a 1x1x1x1 shape).
void AppendInputChild(ByteWriter *w) {
  AppendNetworkHeader(w, NT_INPUT);
  w->PutS32(1); // batch
  w->PutS32(1); // height
  w->PutS32(1); // width
  w->PutS32(1); // depth
  w->PutS32(0); // loss type
}

// Builds a TESSDATA_LSTM component whose top-level network is a plumbing
// layer of the given type with the given (corrupt) stack size, followed by
// the remaining fields of LSTMRecognizer::DeSerialize.
std::vector<char> MakeLstmComponent(NetworkType type, uint32_t stack_size) {
  ByteWriter w;
  AppendNetworkHeader(&w, type);
  w.PutU32(stack_size); // Plumbing::DeSerialize reads this as uint32
  for (uint32_t i = 0; i < stack_size; ++i) {
    AppendInputChild(&w);
  }
  w.PutRaw(kMinUnicharset); // unicharset (raw text, no recoder/unicharset components)
  w.PutString("");             // network_str_
  w.PutS32(0);                 // training_flags_
  w.PutS32(0);                 // training_iteration_
  w.PutS32(0);                 // sample_iteration_
  w.PutS32(0);                 // null_char_
  w.PutU32(0);                 // adam_beta_ (float 0.0)
  w.PutU32(0);                 // learning_rate_ (float 0.0)
  w.PutU32(0);                 // momentum_ (float 0.0)
  return w.data();
}

// Writes a traineddata file with the given (corrupt) LSTM component to
// dir/eng.traineddata.
bool WriteCorruptTraineddata(const std::string &dir, const std::vector<char> &lstm) {
  TessdataManager mgr;
  mgr.OverwriteEntry(TESSDATA_LSTM, lstm.data(), static_cast<int>(lstm.size()));
  return mgr.SaveFile((dir + "/eng.traineddata").c_str(), nullptr);
}

class PlumbingTest : public testing::Test {
protected:
  void SetUp() override {
    tmpl_ = "/tmp/tess_plumbing_test_XXXXXX";
    char *dir = mkdtemp(tmpl_.data());
    ASSERT_NE(dir, nullptr);
    dir_ = dir;
  }
  void TearDown() override {
    std::remove((dir_ + "/eng.traineddata").c_str());
    rmdir(dir_.c_str());
  }
  // Expects the LSTM engine to reject the corrupted traineddata
  // gracefully (init failure) instead of crashing.
  void ExpectInitFails(const std::vector<char> &lstm) {
    ASSERT_TRUE(WriteCorruptTraineddata(dir_, lstm));
    tesseract::TessBaseAPI api;
    EXPECT_EQ(api.Init(dir_.c_str(), "eng", tesseract::OEM_LSTM_ONLY), -1);
  }
  std::string dir_;
  std::string tmpl_;
};

// Empty NT_SERIES stack: Series::CacheXScaleFactor would dereference
// stack_[0] on the empty vector during initialization.
TEST_F(PlumbingTest, RejectsEmptySeriesStack) {
  ExpectInitFails(MakeLstmComponent(NT_SERIES, 0));
}

// Empty NT_PARALLEL stack: Plumbing::XScaleFactor would dereference
// stack_[0] on the empty vector during initialization.
TEST_F(PlumbingTest, RejectsEmptyParallelStack) {
  ExpectInitFails(MakeLstmComponent(NT_PARALLEL, 0));
}

// Empty NT_XREVERSED stack: same crash as the parallel case.
TEST_F(PlumbingTest, RejectsEmptyReversedStack) {
  ExpectInitFails(MakeLstmComponent(NT_XREVERSED, 0));
}

// A Series with a single network: Series::Forward requires at least two
// networks, so such a model can never work.
TEST_F(PlumbingTest, RejectsSingleNetworkSeries) {
  ExpectInitFails(MakeLstmComponent(NT_SERIES, 1));
}

} // namespace
} // namespace tesseract
