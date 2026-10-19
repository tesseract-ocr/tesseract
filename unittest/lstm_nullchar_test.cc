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

#include <atomic>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <random>
#include <string>
#include <system_error>
#include <vector>

namespace tesseract {
namespace {

class LstmNullCharTest : public testing::Test {
 protected:
  void SetUp() override {
    namespace fs = std::filesystem;
    // Seed from a random device so the directory name is unique across
    // concurrent test processes (a per-process counter alone is not).
    static std::atomic<unsigned> counter{std::random_device{}()};
    dir_ = (fs::temp_directory_path() /
            ("tess_lstm_nullchar_test_" + std::to_string(counter.fetch_add(1))))
        .string();
    std::error_code ec;
    const bool created = fs::create_directory(dir_, ec);
    ASSERT_TRUE(created)
        << "could not create temp dir " << dir_ << ": " << ec.message();
  }
  void TearDown() override {
    std::error_code ec;
    std::filesystem::remove_all(dir_, ec);
  }
  std::string dir_;
};

// Distinguishes why a corrupted model was not produced, so a test can skip
// only for an unsuitable fixture (absent, or in a different byte order than
// the host) and assert on a genuine construction failure instead of
// silently hiding a regression in the corruption logic.
enum class CorruptResult {
  kFixtureMissing,   // no eng.traineddata, or no LSTM component in it
  kForeignByteOrder, // the fixture is not in the host byte order
  kBuilt,
  kFailed,
};

// Loads eng.traineddata, replaces the null_char field of the TESSDATA_LSTM
// component with the given value and writes the result to dir/eng.traineddata.
// kFailed indicates a fixture-construction regression (e.g. a
// serialization-layout change moved the null_char field out from under the
// offset computation) and must fail the test rather than skip it.
CorruptResult MakePatchedTraineddata(const std::string &dir,
                                     int32_t patched_null_char) {
  const std::string eng = std::string(TESSDATA_DIR) + "/eng.traineddata";
  TessdataManager mgr;
  if (!mgr.Init(eng.c_str()) || !mgr.IsLSTMAvailable()) {
    return CorruptResult::kFixtureMissing;
  }
  // TessdataManager detects a fixture stored in a different byte order than
  // the host (the standard eng.traineddata is little-endian). Such a fixture
  // cannot be patched and re-saved consistently: TFile::Open (used below to
  // locate the field) resets the reader's swap flag, the raw byte patch
  // below uses the host byte order, and SaveFile writes the traineddata
  // header in the host byte order, so the result would be a file with mixed
  // byte orders that Tesseract no longer loads.
  if (mgr.swap()) {
    return CorruptResult::kForeignByteOrder;
  }
  TFile fp;
  if (!mgr.GetComponent(TESSDATA_LSTM, &fp)) {
    return CorruptResult::kFailed;
  }
  const size_t len = fp.RemainingBytes();
  std::vector<char> comp(len);
  if (fp.FRead(comp.data(), 1, len) != len) {
    return CorruptResult::kFailed;
  }
  // Parse the network to learn the softmax width, then position the reader
  // exactly at the null_char field (right after the network string and the
  // three training iteration scalars).
  TFile pf;
  if (!pf.Open(comp.data(), comp.size())) {
    return CorruptResult::kFailed;
  }
  Network *net = Network::CreateFromFile(&pf);
  if (net == nullptr) {
    return CorruptResult::kFailed;
  }
  const int no = net->NumOutputs();
  delete net;
  std::string net_str;
  int32_t flags, training_iter, sample_iter;
  if (!pf.DeSerialize(net_str) || !pf.DeSerialize(&flags) ||
      !pf.DeSerialize(&training_iter) || !pf.DeSerialize(&sample_iter)) {
    return CorruptResult::kFailed;
  }
  const size_t null_off = comp.size() - pf.RemainingBytes();
  if (null_off + 4 > comp.size()) {
    return CorruptResult::kFailed;
  }
  int32_t current_null_char;
  std::memcpy(&current_null_char, comp.data() + null_off, 4);
  // Sanity: the untouched model must carry a valid null_char. This confirms
  // the offset computation rather than corrupting an unrelated field.
  if (current_null_char < 0 || current_null_char >= no) {
    return CorruptResult::kFailed;
  }
  if (patched_null_char == current_null_char) {
    return CorruptResult::kFailed; // nothing to change
  }
  std::memcpy(comp.data() + null_off, &patched_null_char, 4);
  mgr.OverwriteEntry(TESSDATA_LSTM, comp.data(), static_cast<int>(comp.size()));
  if (!mgr.SaveFile((dir + "/eng.traineddata").c_str(), nullptr)) {
    return CorruptResult::kFailed;
  }
  return CorruptResult::kBuilt;
}

// An oversized null_char must be rejected at load time.
TEST_F(LstmNullCharTest, RejectsOversizedNullChar) {
  const CorruptResult result = MakePatchedTraineddata(dir_, 0x40000000);
  if (result == CorruptResult::kFixtureMissing) {
    GTEST_SKIP() << "eng.traineddata with an LSTM component not found in "
                 << TESSDATA_DIR;
  }
  if (result == CorruptResult::kForeignByteOrder) {
    GTEST_SKIP() << "eng.traineddata is not in the host byte order";
  }
  ASSERT_EQ(result, CorruptResult::kBuilt)
      << "failed to build an oversized-null-char model";
  tesseract::TessBaseAPI api;
  EXPECT_EQ(api.Init(dir_.c_str(), "eng", tesseract::OEM_LSTM_ONLY), -1);
}

// A negative null_char must be rejected at load time.
TEST_F(LstmNullCharTest, RejectsNegativeNullChar) {
  const CorruptResult result = MakePatchedTraineddata(dir_, -1);
  if (result == CorruptResult::kFixtureMissing) {
    GTEST_SKIP() << "eng.traineddata with an LSTM component not found in "
                 << TESSDATA_DIR;
  }
  if (result == CorruptResult::kForeignByteOrder) {
    GTEST_SKIP() << "eng.traineddata is not in the host byte order";
  }
  ASSERT_EQ(result, CorruptResult::kBuilt)
      << "failed to build a negative-null-char model";
  tesseract::TessBaseAPI api;
  EXPECT_EQ(api.Init(dir_.c_str(), "eng", tesseract::OEM_LSTM_ONLY), -1);
}

} // namespace
} // namespace tesseract
