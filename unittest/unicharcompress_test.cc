// (C) Copyright 2017, Google Inc.
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <string>

#include "absl/strings/ascii.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_split.h"
#include "allheaders.h"

#include "include_gunit.h"
#include "log.h"                        // for LOG
#include <tesseract/serialis.h>
#include "tprintf.h"
#include "unicharcompress.h"

namespace tesseract {
namespace {

class UnicharcompressTest : public ::testing::Test {
 protected:
  void SetUp() {
    std::locale::global(std::locale(""));
  }

  // Loads and compresses the given unicharset.
  void LoadUnicharset(const std::string& unicharset_name) {
    std::string radical_stroke_file =
        file::JoinPath(LANGDATA_DIR, "radical-stroke.txt");
    std::string unicharset_file =
        file::JoinPath(TESTDATA_DIR, unicharset_name);
    std::string radical_data;
    CHECK_OK(file::GetContents(radical_stroke_file, &radical_data,
                               file::Defaults()));
    CHECK(unicharset_.load_from_file(unicharset_file.c_str()));
    STRING radical_str(radical_data.c_str());
    null_char_ =
        unicharset_.has_special_codes() ? UNICHAR_BROKEN : unicharset_.size();
    compressed_.ComputeEncoding(unicharset_, null_char_, &radical_str);
    // Get the encoding of the null char.
    RecodedCharID code;
    compressed_.EncodeUnichar(null_char_, &code);
    encoded_null_char_ = code(0);
    std::string output_name = file::JoinPath(
        FLAGS_test_tmpdir, absl::StrCat(unicharset_name, ".encoding.txt"));
    STRING encoding = compressed_.GetEncodingAsString(unicharset_);
    std::string encoding_str(&encoding[0], encoding.size());
    CHECK_OK(file::SetContents(output_name, encoding_str, file::Defaults()));
    LOG(INFO) << "Wrote encoding to:" << output_name;
  }
  // Serializes and de-serializes compressed_ over itself.
  void SerializeAndUndo() {
    GenericVector<char> data;
    TFile wfp;
    wfp.OpenWrite(&data);
    EXPECT_TRUE(compressed_.Serialize(&wfp));
    TFile rfp;
    rfp.Open(&data[0], data.size());
    EXPECT_TRUE(compressed_.DeSerialize(&rfp));
  }
  // Returns true if the lang is in CJK.
  bool IsCJKLang(const std::string& lang) {
    return lang == "chi_sim" || lang == "chi_tra" || lang == "kor" ||
           lang == "jpn";
  }
  // Returns true if the lang is Indic.
  bool IsIndicLang(const std::string& lang) {
    return lang == "asm" || lang == "ben" || lang == "bih" || lang == "hin" ||
           lang == "mar" || lang == "nep" || lang == "san" || lang == "bod" ||
           lang == "dzo" || lang == "guj" || lang == "kan" || lang == "mal" ||
           lang == "ori" || lang == "pan" || lang == "sin" || lang == "tam" ||
           lang == "tel";
  }

  // Expects the appropriate results from the compressed_  unicharset_.
  void ExpectCorrect(const std::string& lang) {
    // Count the number of times each code is used in each element of
    // RecodedCharID.
    RecodedCharID zeros;
    for (int i = 0; i < RecodedCharID::kMaxCodeLen; ++i) zeros.Set(i, 0);
    int code_range = compressed_.code_range();
   std::vector<RecodedCharID> times_seen(code_range, zeros);
    for (int u = 0; u <= unicharset_.size(); ++u) {
      if (u != UNICHAR_SPACE && u != null_char_ &&
          (u == unicharset_.size() || (unicharset_.has_special_codes() &&
                                       u < SPECIAL_UNICHAR_CODES_COUNT))) {
        continue;  // Not used so not encoded.
      }
      RecodedCharID code;
      int len = compressed_.EncodeUnichar(u, &code);
      // Check round-trip encoding.
      int unichar_id;
      GenericVector<UNICHAR_ID> normed_ids;
      if (u == null_char_ || u == unicharset_.size()) {
        unichar_id = null_char_;
      } else {
        unichar_id = u;
      }
      EXPECT_EQ(unichar_id, compressed_.DecodeUnichar(code));
      // Check that the codes are valid.
      for (int i = 0; i < len; ++i) {
        int code_val = code(i);
        EXPECT_GE(code_val, 0);
        EXPECT_LT(code_val, code_range);
        times_seen[code_val].Set(i, times_seen[code_val](i) + 1);
      }
    }
    // Check that each code is used in at least one position.
    for (int c = 0; c < code_range; ++c) {
      int num_used = 0;
      for (int i = 0; i < RecodedCharID::kMaxCodeLen; ++i) {
        if (times_seen[c](i) != 0) ++num_used;
      }
      EXPECT_GE(num_used, 1) << "c=" << c << "/" << code_range;
    }
    // Check that GetNextCodes/GetFinalCodes lists match the times_seen,
    // and create valid codes.
    RecodedCharID code;
    CheckCodeExtensions(code, times_seen);
    // Finally, we achieved all that using a codebook < 10% of the size of
    // the original unicharset, for CK or Indic, and 20% with J, but just
    // no bigger for all others.
    if (IsCJKLang(lang) || IsIndicLang(lang)) {
      EXPECT_LT(code_range, unicharset_.size() / (lang == "jpn" ? 5 : 10));
    } else {
      EXPECT_LE(code_range, unicharset_.size() + 1);
    }
    LOG(INFO) << "Compressed unicharset of " << unicharset_.size() << " to "
              << code_range;
  }
  // Checks for extensions of the current code that either finish a code, or
  // extend it and checks those extensions recursively.
  void CheckCodeExtensions(const RecodedCharID& code,
                           const std::vector<RecodedCharID>& times_seen) {
    RecodedCharID extended = code;
    int length = code.length();
    const GenericVector<int>* final_codes = compressed_.GetFinalCodes(code);
    if (final_codes != nullptr) {
      for (int i = 0; i < final_codes->size(); ++i) {
        int ending = (*final_codes)[i];
        EXPECT_GT(times_seen[ending](length), 0);
        extended.Set(length, ending);
        int unichar_id = compressed_.DecodeUnichar(extended);
        EXPECT_NE(INVALID_UNICHAR_ID, unichar_id);
      }
    }
    const GenericVector<int>* next_codes = compressed_.GetNextCodes(code);
    if (next_codes != nullptr) {
      for (int i = 0; i < next_codes->size(); ++i) {
        int extension = (*next_codes)[i];
        EXPECT_GT(times_seen[extension](length), 0);
        extended.Set(length, extension);
        CheckCodeExtensions(extended, times_seen);
      }
    }
  }

  UnicharCompress compressed_;
  UNICHARSET unicharset_;
  int null_char_;
  // The encoding of the null_char_.
  int encoded_null_char_;
};

TEST_F(UnicharcompressTest, DoesChinese) {
  LOG(INFO) << "Testing chi_tra";
  LoadUnicharset("chi_tra.unicharset");
  ExpectCorrect("chi_tra");
  LOG(INFO) << "Testing chi_sim";
  LoadUnicharset("chi_sim.unicharset");
  ExpectCorrect("chi_sim");
}

TEST_F(UnicharcompressTest, DoesJapanese) {
  LOG(INFO) << "Testing jpn";
  LoadUnicharset("jpn.unicharset");
  ExpectCorrect("jpn");
}

TEST_F(UnicharcompressTest, DoesKorean) {
  LOG(INFO) << "Testing kor";
  LoadUnicharset("kor.unicharset");
  ExpectCorrect("kor");
}

TEST_F(UnicharcompressTest, DoesKannada) {
  LOG(INFO) << "Testing kan";
  LoadUnicharset("kan.unicharset");
  ExpectCorrect("kan");
  SerializeAndUndo();
  ExpectCorrect("kan");
}

TEST_F(UnicharcompressTest, DoesMarathi) {
  LOG(INFO) << "Testing mar";
  LoadUnicharset("mar.unicharset");
  ExpectCorrect("mar");
}

TEST_F(UnicharcompressTest, DoesEnglish) {
  LOG(INFO) << "Testing eng";
  LoadUnicharset("eng.unicharset");
  ExpectCorrect("eng");
}

// Tests that a unicharset that contains double-letter ligatures (eg ff) has
// no null char in the encoding at all.
TEST_F(UnicharcompressTest, DoesLigaturesWithDoubles) {
  LOG(INFO) << "Testing por with ligatures";
  LoadUnicharset("por.unicharset");
  ExpectCorrect("por");
  // Check that any unichar-id that is encoded with multiple codes has the
  // correct encoded_nulll_char_ in between.
  for (int u = 0; u <= unicharset_.size(); ++u) {
    RecodedCharID code;
    int len = compressed_.EncodeUnichar(u, &code);
    if (len > 1) {
      // The should not be any null char in the code.
      for (int i = 0; i < len; ++i) {
        EXPECT_NE(encoded_null_char_, code(i));
      }
    }
  }
}

// Tests that GetEncodingAsString returns the right result for a trivial
// unicharset.
TEST_F(UnicharcompressTest, GetEncodingAsString) {
  LoadUnicharset("trivial.unicharset");
  ExpectCorrect("trivial");
  STRING encoding = compressed_.GetEncodingAsString(unicharset_);
  std::string encoding_str(&encoding[0], encoding.length());
  std::vector<std::string> lines =
      absl::StrSplit(encoding_str, "\n", absl::SkipEmpty());
  EXPECT_EQ(5, lines.size());
  // The first line is always space.
  EXPECT_EQ("0\t ", lines[0]);
  // Next we have i.
  EXPECT_EQ("1\ti", lines[1]);
  // Next we have f.
  EXPECT_EQ("2\tf", lines[2]);
  // Next we have the fi ligature: ﬁ. There are no nulls in it, as there are no
  // repeated letter ligatures in this unicharset, unlike por.unicharset above.
  EXPECT_EQ("2,1\tﬁ", lines[3]);
  // Finally the null character.
  EXPECT_EQ("3\t<nul>", lines[4]);
}

}  // namespace
}  // namespace tesseract
