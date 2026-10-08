///////////////////////////////////////////////////////////////////////
// File:        lstm_nullchar_test.cc
// Description: Tests that an out-of-range null_char in the TESSDATA_LSTM
//              component of a .traineddata file is rejected at load time
//              by LSTMRecognizer::DeSerialize, instead of being used later
//              as an out-of-bounds index into the softmax output during
//              recognition.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
//
///////////////////////////////////////////////////////////////////////

#include "include_gunit.h"

#include <tesseract/baseapi.h>

#include "network.h"        // for Network::CreateFromFile, NumOutputs
#include "serialis.h"       // for TFile
#include "tessdatamanager.h" // for TessdataManager

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

namespace tesseract {
namespace {

class LstmNullCharTest : public testing::Test {
 protected:
  void SetUp() override {
    tmpl_ = "/tmp/tess_lstm_nullchar_test_XXXXXX";
    char *dir = mkdtemp(tmpl_.data());
    ASSERT_NE(dir, nullptr);
    dir_ = dir;
  }
  void TearDown() override {
    std::remove((dir_ + "/eng.traineddata").c_str());
    rmdir(dir_.c_str());
  }
  std::string dir_;
  std::string tmpl_;
};

// Loads eng.traineddata, replaces the null_char field of the TESSDATA_LSTM
// component with the given value and writes the result to dir/eng.traineddata.
// Returns false (and skips the caller) when eng or its LSTM component is not
// available. The returned num_outputs is the network softmax width, used to
// confirm the patched value is really out of range.
bool MakePatchedTraineddata(const std::string &dir, int32_t patched_null_char,
                            int *num_outputs) {
  const std::string eng = std::string(TESSDATA_DIR) + "/eng.traineddata";
  TessdataManager mgr;
  if (!mgr.Init(eng.c_str()) || !mgr.IsLSTMAvailable()) {
    return false;
  }
  TFile fp;
  if (!mgr.GetComponent(TESSDATA_LSTM, &fp)) {
    return false;
  }
  const size_t len = fp.RemainingBytes();
  std::vector<char> comp(len);
  if (fp.FRead(comp.data(), 1, len) != len) {
    return false;
  }
  // Parse the network to learn the softmax width, then position the reader
  // exactly at the null_char field (right after the network string and the
  // three training iteration scalars).
  TFile pf;
  pf.Open(comp.data(), comp.size());
  Network *net = Network::CreateFromFile(&pf);
  if (net == nullptr) {
    return false;
  }
  const int no = net->NumOutputs();
  delete net;
  std::string net_str;
  int32_t flags, training_iter, sample_iter;
  if (!pf.DeSerialize(net_str) || !pf.DeSerialize(&flags) ||
      !pf.DeSerialize(&training_iter) || !pf.DeSerialize(&sample_iter)) {
    return false;
  }
  const size_t null_off = comp.size() - pf.RemainingBytes();
  if (null_off + 4 > comp.size()) {
    return false;
  }
  int32_t current_null_char;
  std::memcpy(&current_null_char, comp.data() + null_off, 4);
  // Sanity: the untouched model must carry a valid null_char. This confirms
  // the offset computation rather than corrupting an unrelated field.
  if (current_null_char < 0 || current_null_char >= no) {
    return false;
  }
  if (patched_null_char == current_null_char) {
    return false; // nothing to change
  }
  std::memcpy(comp.data() + null_off, &patched_null_char, 4);
  mgr.OverwriteEntry(TESSDATA_LSTM, comp.data(), static_cast<int>(comp.size()));
  if (!mgr.SaveFile((dir + "/eng.traineddata").c_str(), nullptr)) {
    return false;
  }
  *num_outputs = no;
  return true;
}

// An oversized null_char must be rejected at load time.
TEST_F(LstmNullCharTest, RejectsOversizedNullChar) {
  int num_outputs = 0;
  if (!MakePatchedTraineddata(dir_, 0x40000000, &num_outputs)) {
    GTEST_SKIP();
  }
  tesseract::TessBaseAPI api;
  EXPECT_EQ(api.Init(dir_.c_str(), "eng", tesseract::OEM_LSTM_ONLY), -1);
}

// A negative null_char must be rejected at load time.
TEST_F(LstmNullCharTest, RejectsNegativeNullChar) {
  int num_outputs = 0;
  if (!MakePatchedTraineddata(dir_, -1, &num_outputs)) {
    GTEST_SKIP();
  }
  tesseract::TessBaseAPI api;
  EXPECT_EQ(api.Init(dir_.c_str(), "eng", tesseract::OEM_LSTM_ONLY), -1);
}

} // namespace
} // namespace tesseract
