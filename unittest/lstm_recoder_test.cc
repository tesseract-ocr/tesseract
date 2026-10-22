///////////////////////////////////////////////////////////////////////
// File:        lstm_recoder_test.cc
// Description: Tests that a corrupt LSTM model is rejected at load time by
//              LSTMRecognizer::Load, instead of being used later as an
//              out-of-bounds index into the network softmax (top_n_flags_ /
//              outputs) during beam search: a recoder whose code range
//              exceeds the network output count.
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
#include <filesystem>
#include <string>
#include <vector>

namespace tesseract {
namespace {

// Reads a 32-bit value stored in host byte order from data at off. Host
// order matches how TFile reads the in-memory component (TFile::Open(memory)
// does not swap), so the fixture must be in the host byte order; the caller
// has verified that via TessdataManager::swap.
bool ReadInt(const std::vector<char> &data, size_t off, uint32_t *out) {
  if (off + 4 > data.size()) {
    return false;
  }
  std::memcpy(out, data.data() + off, 4);
  return true;
}

// Parses the recoder (a serialized std::vector<RecodedCharID>) and returns
// the byte offset of the first int32 code value (in an entry other than the
// first, i.e. other than space) that is strictly smaller than num_outputs.
// That value can be set to num_outputs, raising code_range above the
// softmax width. Returns false if there is no such code (code_range already
// exceeds num_outputs, or the recoder is empty).
bool FindBumpableCode(const std::vector<char> &rec, int32_t num_outputs, size_t *offset) {
  uint32_t n = 0;
  if (!ReadInt(rec, 0, &n) || static_cast<size_t>(n) > rec.size()) {
    return false;
  }
  size_t pos = 4;
  for (uint32_t c = 0; c < n; ++c) {
    // RecodedCharID is serialized as int8 self_normalized_, uint32 length_,
    // then length_ int32 code values.
    if (pos + 5 > rec.size()) return false;
    uint32_t length = 0;
    if (!ReadInt(rec, pos + 1, &length)) return false;
    const size_t len = static_cast<size_t>(length);
    if (5 + 4 * len > rec.size()) return false;
    for (size_t i = 0; i < len; ++i) {
      const size_t off = pos + 5 + 4 * i;
      int32_t val = 0;
      if (!ReadInt(rec, off, reinterpret_cast<uint32_t *>(&val))) return false;
      // Skip the first entry (space): bumping its code would trip the
      // separate "Space was garbled" check instead of the code-range check.
      if (c > 0 && val < num_outputs) {
        *offset = off;
        return true;
      }
    }
    pos += 5 + 4 * len;
  }
  return false;
}

class LstmRecoderTest : public testing::Test {
 protected:
  void SetUp() override {
    dir_ = CreateTempDir("tess_lstm_recoder_test_");
    ASSERT_FALSE(dir_.empty());
  }
  void TearDown() override {
    RemoveTempDir(dir_);
  }
  std::string dir_;
};

// Distinguishes the reason a corrupted model was not produced, so a test can
// skip only when the eng fixture is genuinely absent or stored in a foreign
// byte order (and therefore unpatchable) rather than silently hiding a
// regression in the corruption logic.
enum class CorruptResult { kFixtureMissing, kForeignByteOrder, kBuilt, kFailed };

// Returns the reason to skip the test for a given CorruptResult, or an empty
// string if the test can run.
std::string SkipReason(CorruptResult result) {
  switch (result) {
    case CorruptResult::kFixtureMissing:
      return "eng.traineddata not found in " + std::string(TESSDATA_DIR);
    case CorruptResult::kForeignByteOrder:
      return "eng.traineddata is not in the host byte order";
    default:
      return "";
  }
}

// Loads eng.traineddata, finds the network softmax width, then raises the
// first recoder code that is below that width up to the width so that the
// recoder's code range exceeds the softmax. Writes the result to
// dir/eng.traineddata.
CorruptResult MakeOversizedRecoder(const std::string &dir) {
  const std::string eng = std::string(TESSDATA_DIR) + "/eng.traineddata";
  TessdataManager mgr;
  if (!mgr.Init(eng.c_str())) {
    return CorruptResult::kFixtureMissing;
  }
  // The raw byte patch below reads and writes in host byte order, and TFile
  // reads the in-memory component without swapping, so a fixture stored in a
  // foreign byte order cannot be patched and re-saved consistently.
  if (mgr.swap()) {
    return CorruptResult::kForeignByteOrder;
  }
  TFile lfp;
  if (!mgr.GetComponent(TESSDATA_LSTM, &lfp)) {
    return CorruptResult::kFailed;
  }
  std::vector<char> lstm(lfp.RemainingBytes());
  if (lfp.FRead(lstm.data(), 1, lstm.size()) != lstm.size()) {
    return CorruptResult::kFailed;
  }
  TFile nfp;
  nfp.Open(lstm.data(), lstm.size());
  Network *net = Network::CreateFromFile(&nfp);
  if (net == nullptr) {
    return CorruptResult::kFailed;
  }
  const int32_t num_outputs = net->NumOutputs();
  delete net;

  TFile rfp;
  if (!mgr.GetComponent(TESSDATA_LSTM_RECODER, &rfp)) {
    return CorruptResult::kFailed;
  }
  std::vector<char> rec(rfp.RemainingBytes());
  if (rfp.FRead(rec.data(), 1, rec.size()) != rec.size()) {
    return CorruptResult::kFailed;
  }
  size_t offset;
  if (!FindBumpableCode(rec, num_outputs, &offset)) {
    return CorruptResult::kFailed;
  }
  // Write the value in host byte order, matching how the component was read
  // and how SaveFile writes it back.
  const uint32_t new_val = static_cast<uint32_t>(num_outputs);
  std::memcpy(rec.data() + offset, &new_val, 4);
  mgr.OverwriteEntry(TESSDATA_LSTM_RECODER, rec.data(), static_cast<int>(rec.size()));
  if (!mgr.SaveFile((dir + "/eng.traineddata").c_str(), nullptr)) {
    return CorruptResult::kFailed;
  }
  return CorruptResult::kBuilt;
}

// A recoder code range wider than the network softmax must be rejected.
TEST_F(LstmRecoderTest, RejectsOversizedRecoderCodeRange) {
  if (!std::filesystem::exists(std::string(TESSDATA_DIR) + "/eng.traineddata")) {
    GTEST_SKIP() << "eng.traineddata not found in " << TESSDATA_DIR;
  }
  const CorruptResult result = MakeOversizedRecoder(dir_);
  const std::string skip = SkipReason(result);
  if (!skip.empty()) {
    GTEST_SKIP() << skip;
  }
  ASSERT_EQ(result, CorruptResult::kBuilt) << "failed to build an oversized-recoder model";
  tesseract::TessBaseAPI api;
  EXPECT_EQ(api.Init(dir_.c_str(), "eng", tesseract::OEM_LSTM_ONLY), -1);
}

} // namespace
} // namespace tesseract
