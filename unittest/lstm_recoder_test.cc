///////////////////////////////////////////////////////////////////////
// File:        lstm_recoder_test.cc
// Description: Tests that a corrupt LSTM model is rejected at load time by
//              LSTMRecognizer::Load, instead of being used later as an
//              out-of-bounds index into the network softmax (top_n_flags_ /
//              outputs) during beam search: (1) a recoder whose code range
//              exceeds the network output count, and (2) a null char that is
//              not within the output range.
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

#include <cstdint>
#include <string>
#include <vector>
#include <atomic>
#include <filesystem>
#include <random>
#include <system_error>

namespace tesseract {
namespace {

// Reads an unsigned little-endian value of the given size from data at off.
bool ReadInt(const std::vector<char> &data, size_t off, int bytes, uint64_t *out) {
  if (off + static_cast<size_t>(bytes) > data.size()) {
    return false;
  }
  uint64_t v = 0;
  for (int i = bytes - 1; i >= 0; --i) {
    v = (v << 8) | static_cast<uint8_t>(data[off + i]);
  }
  *out = v;
  return true;
}

// Parses the recoder (a serialized std::vector<RecodedCharID>) and returns
// the byte offset of the first int32 code value (in an entry other than the
// first, i.e. other than space) that is strictly smaller than num_outputs.
// That value can be set to num_outputs, raising code_range above the
// softmax width. Returns false if there is no such code (code_range already
// exceeds num_outputs, or the recoder is empty).
bool FindBumpableCode(const std::vector<char> &rec, int32_t num_outputs, size_t *offset) {
  uint64_t n = 0;
  if (!ReadInt(rec, 0, 4, &n) || n > rec.size()) {
    return false;
  }
  size_t pos = 4;
  for (uint64_t c = 0; c < n; ++c) {
    // RecodedCharID is serialized as int8 self_normalized_, uint32 length_,
    // then length_ int32 code values.
    if (pos + 5 > rec.size()) return false;
    uint64_t length = 0;
    if (!ReadInt(rec, pos + 1, 4, &length)) return false;
    if (5 + 4 * length > rec.size()) return false;
    for (uint64_t i = 0; i < length; ++i) {
      const size_t off = pos + 5 + 4 * i;
      int32_t val = 0;
      for (int k = 3; k >= 0; --k) {
        val = (val << 8) | static_cast<uint8_t>(rec[off + k]);
      }
      // Skip the first entry (space): bumping its code would trip the
      // separate "Space was garbled" check instead of the code-range check.
      if (c > 0 && val < num_outputs) {
        *offset = off;
        return true;
      }
    }
    pos += 5 + 4 * length;
  }
  return false;
}

// Byte offsets within the serialized LSTM component needed to corrupt the
// null char without disturbing anything else.
struct LstmOffsets {
  size_t null_char_offset = 0;
};

// Locates the null char field within the serialized LSTM component. After the
// network (parsed via Network::CreateFromFile, which advances the TFile), the
// layout is: network_str_ (std::string: uint32 len + len bytes), then
// training_flags_, training_iteration_, sample_iteration_ (int32 each), then
// null_char_ (int32).
bool FindLstmOffsets(const std::vector<char> &lstm, LstmOffsets *off) {
  TFile nfp;
  nfp.Open(lstm.data(), lstm.size());
  Network *net = Network::CreateFromFile(&nfp);
  if (net == nullptr) {
    return false;
  }
  delete net;
  // CreateFromFile consumed the network, so the TFile now sits just past it,
  // i.e. at the start of network_str_.
  const size_t base = lstm.size() - nfp.RemainingBytes();
  uint64_t str_len = 0;
  if (!ReadInt(lstm, base, 4, &str_len) || base + 4 + str_len > lstm.size()) {
    return false;
  }
  const size_t fields = base + 4 + str_len;
  if (fields + 4 * 4 > lstm.size()) {
    return false;
  }
  // After network_str_ the layout is: training_flags_, training_iteration_,
  // sample_iteration_ (int32 each), then null_char_ (int32).
  off->null_char_offset = fields + 3 * 4;
  return true;
}

class LstmRecoderTest : public testing::Test {
 protected:
  void SetUp() override {
    namespace fs = std::filesystem;
    // Seed from a random device so the directory name is unique across
    // concurrent test processes (a per-process counter alone is not).
    static std::atomic<unsigned> counter{std::random_device{}()};
    dir_ = (fs::temp_directory_path() /
            ("tess_lstm_recoder_test_" + std::to_string(counter.fetch_add(1))))
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

// Distinguishes the reason a corrupted model was not produced, so a test can
// skip only when the eng fixture is genuinely absent rather than silently
// hiding a regression in the corruption logic.
enum class CorruptResult { kFixtureMissing, kBuilt, kFailed };

// True if the eng.traineddata fixture is available in TESSDATA_DIR.
bool EngModelPresent() {
  return std::filesystem::exists(std::string(TESSDATA_DIR) + "/eng.traineddata");
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
  // Write the value as a little-endian int32.
  uint32_t new_val = static_cast<uint32_t>(num_outputs);
  for (int k = 0; k < 4; ++k) {
    rec[offset + k] = static_cast<char>((new_val >> (8 * k)) & 0xFF);
  }
  mgr.OverwriteEntry(TESSDATA_LSTM_RECODER, rec.data(), static_cast<int>(rec.size()));
  if (!mgr.SaveFile((dir + "/eng.traineddata").c_str(), nullptr)) {
    return CorruptResult::kFailed;
  }
  return CorruptResult::kBuilt;
}

// Loads eng.traineddata, sets the model's null char to the network output
// count (just past the end of the softmax), and writes the result to
// dir/eng.traineddata. The null char is indexed into top_n_flags_/outputs, so
// a corrupt value would read out of bounds.
CorruptResult MakeOversizedNullChar(const std::string &dir) {
  const std::string eng = std::string(TESSDATA_DIR) + "/eng.traineddata";
  TessdataManager mgr;
  if (!mgr.Init(eng.c_str())) {
    return CorruptResult::kFixtureMissing;
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

  LstmOffsets off;
  if (!FindLstmOffsets(lstm, &off)) {
    return CorruptResult::kFailed;
  }
  const uint32_t new_val = static_cast<uint32_t>(num_outputs);
  for (int k = 0; k < 4; ++k) {
    lstm[off.null_char_offset + k] = static_cast<char>((new_val >> (8 * k)) & 0xFF);
  }
  mgr.OverwriteEntry(TESSDATA_LSTM, lstm.data(), static_cast<int>(lstm.size()));
  if (!mgr.SaveFile((dir + "/eng.traineddata").c_str(), nullptr)) {
    return CorruptResult::kFailed;
  }
  return CorruptResult::kBuilt;
}

// A recoder code range wider than the network softmax must be rejected.
TEST_F(LstmRecoderTest, RejectsOversizedRecoderCodeRange) {
  if (!EngModelPresent()) {
    GTEST_SKIP() << "eng.traineddata not found in " << TESSDATA_DIR;
  }
  ASSERT_EQ(MakeOversizedRecoder(dir_), CorruptResult::kBuilt)
      << "failed to build an oversized-recoder model";
  tesseract::TessBaseAPI api;
  EXPECT_EQ(api.Init(dir_.c_str(), "eng", tesseract::OEM_LSTM_ONLY), -1);
}

// A null char outside the network softmax must be rejected.
TEST_F(LstmRecoderTest, RejectsOutOfRangeNullChar) {
  if (!EngModelPresent()) {
    GTEST_SKIP() << "eng.traineddata not found in " << TESSDATA_DIR;
  }
  ASSERT_EQ(MakeOversizedNullChar(dir_), CorruptResult::kBuilt)
      << "failed to build an out-of-range-null-char model";
  tesseract::TessBaseAPI api;
  EXPECT_EQ(api.Init(dir_.c_str(), "eng", tesseract::OEM_LSTM_ONLY), -1);
}

} // namespace
} // namespace tesseract
