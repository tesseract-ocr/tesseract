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

#include "network.h"         // for Network::CreateFromFile, NumOutputs
#include "serialis.h"        // for TFile
#include "tessdatamanager.h" // for TessdataManager
#include "util/tempdir.h"    // for CreateTempDir, RemoveTempDir

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

namespace tesseract {
namespace {

class LstmNullCharTest : public testing::Test {
protected:
  void SetUp() override {
    dir_ = CreateTempDir("tess_lstm_nullchar_test_");
    ASSERT_FALSE(dir_.empty());
  }
  void TearDown() override {
    RemoveTempDir(dir_);
  }
  std::string dir_;
};

// Distinguishes why a corrupted model was not produced, so a test can skip
// only for an unsuitable fixture (absent, in a different byte order than the
// host, or in a serialization layout that cannot be patched) and assert on a
// genuine construction failure instead of silently hiding a regression in
// the corruption logic.
enum class FixtureResult {
  kMissing,          // no eng.traineddata, or no LSTM component in it
  kForeignByteOrder, // the fixture is not in the host byte order
  kLegacyLayout,     // the LSTM component stores the unicharset inline
  kOk,               // the fixture was usable
  kFailed,           // a genuine construction failure
};

// Returns the reason why a test has to be skipped, or an empty string if the
// fixture result means that the test can run.
std::string SkipReason(FixtureResult result) {
  switch (result) {
    case FixtureResult::kMissing:
      return "eng.traineddata with an LSTM component not found in " +
             std::string(TESSDATA_DIR);
    case FixtureResult::kForeignByteOrder:
      return "eng.traineddata is not in the host byte order";
    case FixtureResult::kLegacyLayout:
      return "eng.traineddata stores the unicharset inline in the LSTM "
             "component";
    default:
      return "";
  }
}

// Opens the fixture and checks that it can be patched on this host.
FixtureResult OpenFixture(TessdataManager *mgr) {
  const std::string eng = std::string(TESSDATA_DIR) + "/eng.traineddata";
  if (!mgr->Init(eng.c_str()) || !mgr->IsLSTMAvailable()) {
    return FixtureResult::kMissing;
  }
  // TessdataManager detects a fixture stored in a different byte order than
  // the host (the standard eng.traineddata is little-endian). Such a fixture
  // cannot be patched and re-saved consistently: TFile::Open (used below to
  // locate the field) resets the reader's swap flag, the raw byte patch
  // below uses the host byte order, and SaveFile writes the traineddata
  // header in the host byte order, so the result would be a file with mixed
  // byte orders that Tesseract no longer loads.
  if (mgr->swap()) {
    return FixtureResult::kForeignByteOrder;
  }
  // The offset computation in MakePatchedTraineddata assumes that the
  // unicharset and the recoder are stored in their own components, i.e. that
  // the TESSDATA_LSTM component holds only the network and the training
  // parameters. In the older layout the unicharset is stored inline, between
  // the network and the network string, and the field cannot be located this
  // way. This is the same condition under which LSTMRecognizer::DeSerialize
  // includes the charsets in the LSTM component.
  if (!mgr->IsComponentAvailable(TESSDATA_LSTM_RECODER) ||
      !mgr->IsComponentAvailable(TESSDATA_LSTM_UNICHARSET)) {
    return FixtureResult::kLegacyLayout;
  }
  return FixtureResult::kOk;
}

// Copies the TESSDATA_LSTM component of the fixture into comp.
FixtureResult ReadLstmComponent(TessdataManager *mgr, std::vector<char> *comp) {
  const FixtureResult opened = OpenFixture(mgr);
  if (opened != FixtureResult::kOk) {
    return opened;
  }
  TFile fp;
  if (!mgr->GetComponent(TESSDATA_LSTM, &fp)) {
    return FixtureResult::kFailed;
  }
  const size_t len = fp.RemainingBytes();
  comp->resize(len);
  if (fp.FRead(comp->data(), 1, len) != len) {
    return FixtureResult::kFailed;
  }
  return FixtureResult::kOk;
}

// Loads eng.traineddata, replaces the null_char field of the TESSDATA_LSTM
// component with the given value and writes the result to dir/eng.traineddata.
// kFailed indicates a fixture-construction regression (e.g. a
// serialization-layout change moved the null_char field out from under the
// offset computation) and must fail the test rather than skip it.
FixtureResult MakePatchedTraineddata(const std::string &dir,
                                     int32_t patched_null_char) {
  TessdataManager mgr;
  std::vector<char> comp;
  const FixtureResult read = ReadLstmComponent(&mgr, &comp);
  if (read != FixtureResult::kOk) {
    return read;
  }
  // Parse the network to learn the softmax width, then position the reader
  // exactly at the null_char field (right after the network string and the
  // three training iteration scalars).
  TFile pf;
  if (!pf.Open(comp.data(), comp.size())) {
    return FixtureResult::kFailed;
  }
  Network *net = Network::CreateFromFile(&pf);
  if (net == nullptr) {
    return FixtureResult::kFailed;
  }
  const int no = net->NumOutputs();
  delete net;
  std::string net_str;
  int32_t flags, training_iter, sample_iter;
  if (!pf.DeSerialize(net_str) || !pf.DeSerialize(&flags) ||
      !pf.DeSerialize(&training_iter) || !pf.DeSerialize(&sample_iter)) {
    return FixtureResult::kFailed;
  }
  const size_t null_off = comp.size() - pf.RemainingBytes();
  if (null_off + 4 > comp.size()) {
    return FixtureResult::kFailed;
  }
  int32_t current_null_char;
  std::memcpy(&current_null_char, comp.data() + null_off, 4);
  // Sanity: the untouched model must carry a valid null_char. This confirms
  // the offset computation rather than corrupting an unrelated field.
  if (current_null_char < 0 || current_null_char >= no) {
    return FixtureResult::kFailed;
  }
  if (patched_null_char == current_null_char) {
    return FixtureResult::kFailed; // nothing to change
  }
  std::memcpy(comp.data() + null_off, &patched_null_char, 4);
  mgr.OverwriteEntry(TESSDATA_LSTM, comp.data(), static_cast<int>(comp.size()));
  if (!mgr.SaveFile((dir + "/eng.traineddata").c_str(), nullptr)) {
    return FixtureResult::kFailed;
  }
  return FixtureResult::kOk;
}

// An oversized null_char must be rejected at load time.
TEST_F(LstmNullCharTest, RejectsOversizedNullChar) {
  const FixtureResult result = MakePatchedTraineddata(dir_, 0x40000000);
  const std::string skip = SkipReason(result);
  if (!skip.empty()) {
    GTEST_SKIP() << skip;
  }
  ASSERT_EQ(result, FixtureResult::kOk)
      << "failed to build an oversized-null-char model";
  tesseract::TessBaseAPI api;
  EXPECT_EQ(api.Init(dir_.c_str(), "eng", tesseract::OEM_LSTM_ONLY), -1);
}

// A negative null_char must be rejected at load time.
TEST_F(LstmNullCharTest, RejectsNegativeNullChar) {
  const FixtureResult result = MakePatchedTraineddata(dir_, -1);
  const std::string skip = SkipReason(result);
  if (!skip.empty()) {
    GTEST_SKIP() << skip;
  }
  ASSERT_EQ(result, FixtureResult::kOk)
      << "failed to build a negative-null-char model";
  tesseract::TessBaseAPI api;
  EXPECT_EQ(api.Init(dir_.c_str(), "eng", tesseract::OEM_LSTM_ONLY), -1);
}

// A model with a valid null_char must still load: the range check in
// LSTMRecognizer::DeSerialize must not reject legitimate models.
TEST_F(LstmNullCharTest, LoadsModelWithValidNullChar) {
  TessdataManager mgr;
  const FixtureResult opened = OpenFixture(&mgr);
  const std::string skip = SkipReason(opened);
  if (!skip.empty()) {
    GTEST_SKIP() << skip;
  }
  ASSERT_EQ(opened, FixtureResult::kOk);
  // Write the fixture out unchanged.
  ASSERT_TRUE(mgr.SaveFile((dir_ + "/eng.traineddata").c_str(), nullptr));
  tesseract::TessBaseAPI api;
  EXPECT_EQ(api.Init(dir_.c_str(), "eng", tesseract::OEM_LSTM_ONLY), 0);
}

} // namespace
} // namespace tesseract
