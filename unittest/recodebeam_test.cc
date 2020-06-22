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

#include "recodebeam.h"
#include "matrix.h"
#include "pageres.h"
#include "ratngs.h"
#include <tesseract/genericvector.h>
#include <tesseract/helpers.h>
#include "unicharcompress.h"
#include "normstrngs.h"
#include "unicharset_training_utils.h"

#include "include_gunit.h"
#include "log.h"                        // for LOG
#include "absl/strings/str_format.h"        // for absl::StrFormat

using tesseract::CCUtil;
using tesseract::Dict;
using tesseract::PointerVector;
using tesseract::RecodeBeamSearch;
using tesseract::RecodedCharID;
using tesseract::RecodeNode;
using tesseract::TRand;
using tesseract::UnicharCompress;

namespace {

// Number of characters to test beam search with.
const int kNumChars = 100;
// Amount of extra random data to pad with after.
const int kPadding = 64;
// Dictionary test data.
// The top choice is: "Gef s wordsright.".
// The desired phrase is "Gets words right.".
// There is a competing dictionary phrase: "Get swords right.".
// ... due to the following errors from the network:
// f stronger than t in "Get".
// weak space between Gef and s and between s and words.
// weak space between words and right.
const char* kGWRTops[] = {"G", "e", "f", " ", "s", " ", "w", "o", "r",    "d",
                          "s", "",  "r", "i", "g", "h", "t", ".", nullptr};
const float kGWRTopScores[] = {0.99, 0.85, 0.87, 0.55, 0.99, 0.65,
                               0.89, 0.99, 0.99, 0.99, 0.99, 0.95,
                               0.99, 0.90, 0.90, 0.90, 0.95, 0.75};
const char* kGWR2nds[] = {"C", "c", "t", "",  "S", "",  "W", "O", "t",    "h",
                          "S", " ", "t", "I", "9", "b", "f", ",", nullptr};
const float kGWR2ndScores[] = {0.01, 0.10, 0.12, 0.42, 0.01, 0.25,
                               0.10, 0.01, 0.01, 0.01, 0.01, 0.05,
                               0.01, 0.09, 0.09, 0.09, 0.05, 0.25};

const char* kZHTops[] = {"实", "学", "储", "啬", "投", "学", "生", nullptr};
const float kZHTopScores[] = {0.98, 0.98, 0.98, 0.98, 0.98, 0.98, 0.98};
const char* kZH2nds[] = {"学", "储", "投", "生", "学", "生", "实", nullptr};
const float kZH2ndScores[] = {0.01, 0.01, 0.01, 0.01, 0.01, 0.01, 0.01};

const char* kViTops[] = {"v", "ậ", "y", " ", "t", "ộ", "i", nullptr};
const float kViTopScores[] = {0.98, 0.98, 0.98, 0.98, 0.98, 0.98, 0.97};
const char* kVi2nds[] = {"V", "a", "v", "", "l", "o", "", nullptr};
const float kVi2ndScores[] = {0.01, 0.01, 0.01, 0.01, 0.01, 0.01, 0.01};

class RecodeBeamTest : public ::testing::Test {
 protected:
  void SetUp() {
    std::locale::global(std::locale(""));
  }

  RecodeBeamTest() : lstm_dict_(&ccutil_) {}
  ~RecodeBeamTest() { lstm_dict_.End(); }

  // Loads and compresses the given unicharset.
  void LoadUnicharset(const std::string& unicharset_name) {
    std::string radical_stroke_file = file::JoinPath(LANGDATA_DIR,
                                                "radical-stroke.txt");
    std::string unicharset_file =
        file::JoinPath(TESTDATA_DIR, unicharset_name);
    std::string radical_data;
    CHECK_OK(file::GetContents(radical_stroke_file, &radical_data,
                               file::Defaults()));
    CHECK(ccutil_.unicharset.load_from_file(unicharset_file.c_str()));
    unichar_null_char_ = ccutil_.unicharset.has_special_codes()
                             ? UNICHAR_BROKEN
                             : ccutil_.unicharset.size();
    STRING radical_str(radical_data.c_str());
    EXPECT_TRUE(recoder_.ComputeEncoding(ccutil_.unicharset, unichar_null_char_,
                                         &radical_str));
    RecodedCharID code;
    recoder_.EncodeUnichar(unichar_null_char_, &code);
    encoded_null_char_ = code(0);
    // Space should encode as itself.
    recoder_.EncodeUnichar(UNICHAR_SPACE, &code);
    EXPECT_EQ(UNICHAR_SPACE, code(0));
    std::string output_name = file::JoinPath(FLAGS_test_tmpdir, "testenc.txt");
    STRING encoding = recoder_.GetEncodingAsString(ccutil_.unicharset);
    std::string encoding_str(&encoding[0], encoding.size());
    CHECK_OK(file::SetContents(output_name, encoding_str, file::Defaults()));
    LOG(INFO) << "Wrote encoding to:" << output_name << "\n";
  }
  // Loads the dictionary.
  void LoadDict(const std::string& lang) {
    std::string traineddata_name = lang + ".traineddata";
    std::string traineddata_file =
        file::JoinPath(TESTDATA_DIR, traineddata_name);
    lstm_dict_.SetupForLoad(nullptr);
    tesseract::TessdataManager mgr;
    mgr.Init(traineddata_file.c_str());
    lstm_dict_.LoadLSTM(lang.c_str(), &mgr);
    lstm_dict_.FinishLoad();
  }

  // Expects the appropriate results from the compressed_  ccutil_.unicharset.
  void ExpectCorrect(const GENERIC_2D_ARRAY<float>& output,
                     const GenericVector<int>& transcription) {
    // Get the utf8 string of the transcription.
    std::string truth_utf8;
    for (int i = 0; i < transcription.size(); ++i) {
      truth_utf8 += ccutil_.unicharset.id_to_unichar(transcription[i]);
    }
    PointerVector<WERD_RES> words;
    ExpectCorrect(output, truth_utf8, nullptr, &words);
  }
  void ExpectCorrect(const GENERIC_2D_ARRAY<float>& output,
                     const std::string& truth_utf8, Dict* dict,
                     PointerVector<WERD_RES>* words) {
    RecodeBeamSearch beam_search(recoder_, encoded_null_char_, false, dict);
    beam_search.Decode(output, 3.5, -0.125, -25.0, nullptr);
    // Uncomment and/or change nullptr above to &ccutil_.unicharset to debug:
    // beam_search.DebugBeams(ccutil_.unicharset);
    GenericVector<int> labels, xcoords;
    beam_search.ExtractBestPathAsLabels(&labels, &xcoords);
    LOG(INFO) << "Labels size = " << labels.size() << " coords "
              << xcoords.size() << "\n";
    // Now decode using recoder_.
    std::string decoded;
    int end = 1;
    for (int start = 0; start < labels.size(); start = end) {
      RecodedCharID code;
      int index = start;
      int uni_id = INVALID_UNICHAR_ID;
      do {
        code.Set(code.length(), labels[index++]);
        uni_id = recoder_.DecodeUnichar(code);
      } while (index < labels.size() &&
               code.length() < RecodedCharID::kMaxCodeLen &&
               (uni_id == INVALID_UNICHAR_ID ||
                !recoder_.IsValidFirstCode(labels[index])));
      EXPECT_NE(INVALID_UNICHAR_ID, uni_id)
          << "index=" << index << "/" << labels.size();
      // To the extent of truth_utf8, we expect decoded to match, but if
      // transcription is shorter, that is OK too, as we may just be testing
      // that we get a valid sequence when padded with random data.
      if (uni_id != unichar_null_char_ && decoded.size() < truth_utf8.size())
        decoded += ccutil_.unicharset.id_to_unichar(uni_id);
      end = index;
    }
    EXPECT_EQ(truth_utf8, decoded);

    // Check that ExtractBestPathAsUnicharIds does the same thing.
    GenericVector<int> unichar_ids;
    GenericVector<float> certainties, ratings;
    beam_search.ExtractBestPathAsUnicharIds(false, &ccutil_.unicharset,
                                            &unichar_ids, &certainties,
                                            &ratings, &xcoords);
    std::string u_decoded;
    float total_rating = 0.0f;
    for (int u = 0; u < unichar_ids.size(); ++u) {
      // To the extent of truth_utf8, we expect decoded to match, but if
      // transcription is shorter, that is OK too, as we may just be testing
      // that we get a valid sequence when padded with random data.
      if (u_decoded.size() < truth_utf8.size()) {
        const char* str = ccutil_.unicharset.id_to_unichar(unichar_ids[u]);
        total_rating += ratings[u];
        LOG(INFO) << absl::StrFormat("%d:u_id=%d=%s, c=%g, r=%g, r_sum=%g @%d", u,
                                  unichar_ids[u], str, certainties[u],
                                  ratings[u], total_rating, xcoords[u]) << "\n";
        if (str[0] == ' ') total_rating = 0.0f;
        u_decoded += str;
      }
    }
    EXPECT_EQ(truth_utf8, u_decoded);

    // Check that ExtractBestPathAsWords does the same thing.
    TBOX line_box(0, 0, 100, 10);
    for (int i = 0; i < 2; ++i) {
      beam_search.ExtractBestPathAsWords(line_box, 1.0f, false,
                                         &ccutil_.unicharset, words);
      std::string w_decoded;
      for (int w = 0; w < words->size(); ++w) {
        const WERD_RES* word = (*words)[w];
        if (w_decoded.size() < truth_utf8.size()) {
          if (!w_decoded.empty() && word->word->space()) w_decoded += " ";
          w_decoded += word->best_choice->unichar_string().c_str();
        }
        LOG(INFO) << absl::StrFormat("Word:%d = %s, c=%g, r=%g, perm=%d", w,
                                  word->best_choice->unichar_string().c_str(),
                                  word->best_choice->certainty(),
                                  word->best_choice->rating(),
                                  word->best_choice->permuter()) << "\n";
      }
      std::string w_trunc(w_decoded.data(), truth_utf8.size());
      if (truth_utf8 != w_trunc) {
        tesseract::NormalizeUTF8String(
            tesseract::UnicodeNormMode::kNFKD, tesseract::OCRNorm::kNormalize,
            tesseract::GraphemeNorm::kNone, w_decoded.c_str(), &w_decoded);
        w_trunc.assign(w_decoded.data(), truth_utf8.size());
      }
      EXPECT_EQ(truth_utf8, w_trunc);
    }
  }
  // Generates easy encoding of the given unichar_ids, and pads with at least
  // padding of random data.
  GENERIC_2D_ARRAY<float> GenerateRandomPaddedOutputs(
      const GenericVector<int>& unichar_ids, int padding) {
    int width = unichar_ids.size() * 2 * RecodedCharID::kMaxCodeLen;
    int num_codes = recoder_.code_range();
    GENERIC_2D_ARRAY<float> outputs(width + padding, num_codes, 0.0f);
    // Fill with random data.
    TRand random;
    for (int t = 0; t < width; ++t) {
      for (int i = 0; i < num_codes; ++i)
        outputs(t, i) = random.UnsignedRand(0.25);
    }
    int t = 0;
    for (int i = 0; i < unichar_ids.size(); ++i) {
      RecodedCharID code;
      int len = recoder_.EncodeUnichar(unichar_ids[i], &code);
      EXPECT_NE(0, len);
      for (int j = 0; j < len; ++j) {
        // Make the desired answer a clear winner.
        if (j > 0 && code(j) == code(j - 1)) {
          // We will collapse adjacent equal codes so put a null in between.
          outputs(t++, encoded_null_char_) = 1.0f;
        }
        outputs(t++, code(j)) = 1.0f;
      }
      // Put a 0 as a null char in between.
      outputs(t++, encoded_null_char_) = 1.0f;
    }
    // Normalize the probs.
    for (int t = 0; t < width; ++t) {
      double sum = 0.0;
      for (int i = 0; i < num_codes; ++i) sum += outputs(t, i);
      for (int i = 0; i < num_codes; ++i) outputs(t, i) /= sum;
    }

    return outputs;
  }
  // Encodes a utf8 string (character) as unichar_id, then recodes, and sets
  // the score for the appropriate sequence of codes, returning the ending t.
  int EncodeUTF8(const char* utf8_str, float score, int start_t, TRand* random,
                 GENERIC_2D_ARRAY<float>* outputs) {
    int t = start_t;
    GenericVector<int> unichar_ids;
    EXPECT_TRUE(ccutil_.unicharset.encode_string(utf8_str, true, &unichar_ids,
                                                 nullptr, nullptr));
    if (unichar_ids.empty() || utf8_str[0] == '\0') {
      unichar_ids.clear();
      unichar_ids.push_back(unichar_null_char_);
    }
    int num_ids = unichar_ids.size();
    for (int u = 0; u < num_ids; ++u) {
      RecodedCharID code;
      int len = recoder_.EncodeUnichar(unichar_ids[u], &code);
      EXPECT_NE(0, len);
      for (int i = 0; i < len; ++i) {
        // Apply the desired score.
        (*outputs)(t++, code(i)) = score;
        if (random != nullptr &&
            t + (num_ids - u) * RecodedCharID::kMaxCodeLen < outputs->dim1()) {
          int dups = static_cast<int>(random->UnsignedRand(3.0));
          for (int d = 0; d < dups; ++d) {
            // Duplicate the desired score.
            (*outputs)(t++, code(i)) = score;
          }
        }
      }
      if (random != nullptr &&
          t + (num_ids - u) * RecodedCharID::kMaxCodeLen < outputs->dim1()) {
        int dups = static_cast<int>(random->UnsignedRand(3.0));
        for (int d = 0; d < dups; ++d) {
          // Add a random number of nulls as well.
          (*outputs)(t++, encoded_null_char_) = score;
        }
      }
    }
    return t;
  }
  // Generates an encoding of the given 4 arrays as synthetic network scores.
  // uses scores1 for chars1 and scores2 for chars2, and everything else gets
  // the leftovers shared out equally. Note that empty string encodes as the
  // null_char_.
  GENERIC_2D_ARRAY<float> GenerateSyntheticOutputs(const char* chars1[],
                                                   const float scores1[],
                                                   const char* chars2[],
                                                   const float scores2[],
                                                   TRand* random) {
    int width = 0;
    while (chars1[width] != nullptr) ++width;
    int padding = width * RecodedCharID::kMaxCodeLen;
    int num_codes = recoder_.code_range();
    GENERIC_2D_ARRAY<float> outputs(width + padding, num_codes, 0.0f);
    int t = 0;
    for (int i = 0; i < width; ++i) {
      // In case there is overlap in the codes between 1st and 2nd choice, it
      // is better to encode the 2nd choice first.
      int end_t2 = EncodeUTF8(chars2[i], scores2[i], t, random, &outputs);
      int end_t1 = EncodeUTF8(chars1[i], scores1[i], t, random, &outputs);
      // Advance t to the max end, setting everything else to the leftovers.
      int max_t = std::max(end_t1, end_t2);
      while (t < max_t) {
        double total_score = 0.0;
        for (int j = 0; j < num_codes; ++j) total_score += outputs(t, j);
        double null_remainder = (1.0 - total_score) / 2.0;
        double remainder = null_remainder / (num_codes - 2);
        if (outputs(t, encoded_null_char_) < null_remainder) {
          outputs(t, encoded_null_char_) += null_remainder;
        } else {
          remainder += remainder;
        }
        for (int j = 0; j < num_codes; ++j) {
          if (outputs(t, j) == 0.0f) outputs(t, j) = remainder;
        }
        ++t;
      }
    }
    // Fill the rest with null chars.
    while (t < width + padding) {
      outputs(t++, encoded_null_char_) = 1.0f;
    }
    return outputs;
  }
  UnicharCompress recoder_;
  int unichar_null_char_ = 0;
  int encoded_null_char_ = 0;
  CCUtil ccutil_;
  Dict lstm_dict_;
};

TEST_F(RecodeBeamTest, DoesChinese) {
  LOG(INFO) << "Testing chi_tra" << "\n";
  LoadUnicharset("chi_tra.unicharset");
  // Correctly reproduce the first kNumchars characters from easy output.
  GenericVector<int> transcription;
  for (int i = SPECIAL_UNICHAR_CODES_COUNT; i < kNumChars; ++i)
    transcription.push_back(i);
  GENERIC_2D_ARRAY<float> outputs =
      GenerateRandomPaddedOutputs(transcription, kPadding);
  ExpectCorrect(outputs, transcription);
  LOG(INFO) << "Testing chi_sim" << "\n";
  LoadUnicharset("chi_sim.unicharset");
  // Correctly reproduce the first kNumchars characters from easy output.
  transcription.clear();
  for (int i = SPECIAL_UNICHAR_CODES_COUNT; i < kNumChars; ++i)
    transcription.push_back(i);
  outputs = GenerateRandomPaddedOutputs(transcription, kPadding);
  ExpectCorrect(outputs, transcription);
}

TEST_F(RecodeBeamTest, DoesJapanese) {
  LOG(INFO) << "Testing jpn" << "\n";
  LoadUnicharset("jpn.unicharset");
  // Correctly reproduce the first kNumchars characters from easy output.
  GenericVector<int> transcription;
  for (int i = SPECIAL_UNICHAR_CODES_COUNT; i < kNumChars; ++i)
    transcription.push_back(i);
  GENERIC_2D_ARRAY<float> outputs =
      GenerateRandomPaddedOutputs(transcription, kPadding);
  ExpectCorrect(outputs, transcription);
}

TEST_F(RecodeBeamTest, DoesKorean) {
  LOG(INFO) << "Testing kor" << "\n";
  LoadUnicharset("kor.unicharset");
  // Correctly reproduce the first kNumchars characters from easy output.
  GenericVector<int> transcription;
  for (int i = SPECIAL_UNICHAR_CODES_COUNT; i < kNumChars; ++i)
    transcription.push_back(i);
  GENERIC_2D_ARRAY<float> outputs =
      GenerateRandomPaddedOutputs(transcription, kPadding);
  ExpectCorrect(outputs, transcription);
}

TEST_F(RecodeBeamTest, DoesKannada) {
  LOG(INFO) << "Testing kan" << "\n";
  LoadUnicharset("kan.unicharset");
  // Correctly reproduce the first kNumchars characters from easy output.
  GenericVector<int> transcription;
  for (int i = SPECIAL_UNICHAR_CODES_COUNT; i < kNumChars; ++i)
    transcription.push_back(i);
  GENERIC_2D_ARRAY<float> outputs =
      GenerateRandomPaddedOutputs(transcription, kPadding);
  ExpectCorrect(outputs, transcription);
}

TEST_F(RecodeBeamTest, DoesMarathi) {
  LOG(INFO) << "Testing mar" << "\n";
  LoadUnicharset("mar.unicharset");
  // Correctly reproduce the first kNumchars characters from easy output.
  GenericVector<int> transcription;
  for (int i = SPECIAL_UNICHAR_CODES_COUNT; i < kNumChars; ++i)
    transcription.push_back(i);
  GENERIC_2D_ARRAY<float> outputs =
      GenerateRandomPaddedOutputs(transcription, kPadding);
  ExpectCorrect(outputs, transcription);
}

TEST_F(RecodeBeamTest, DoesEnglish) {
  LOG(INFO) << "Testing eng" << "\n";
  LoadUnicharset("eng.unicharset");
  // Correctly reproduce the first kNumchars characters from easy output.
  GenericVector<int> transcription;
  for (int i = SPECIAL_UNICHAR_CODES_COUNT; i < kNumChars; ++i)
    transcription.push_back(i);
  GENERIC_2D_ARRAY<float> outputs =
      GenerateRandomPaddedOutputs(transcription, kPadding);
  ExpectCorrect(outputs, transcription);
}

TEST_F(RecodeBeamTest, DISABLED_EngDictionary) {
  LOG(INFO) << "Testing eng dictionary" << "\n";
  LoadUnicharset("eng_beam.unicharset");
  GENERIC_2D_ARRAY<float> outputs = GenerateSyntheticOutputs(
      kGWRTops, kGWRTopScores, kGWR2nds, kGWR2ndScores, nullptr);
  std::string default_str;
  for (int i = 0; kGWRTops[i] != nullptr; ++i) default_str += kGWRTops[i];
  PointerVector<WERD_RES> words;
  ExpectCorrect(outputs, default_str, nullptr, &words);
  // Now try again with the dictionary.
  LoadDict("eng_beam");
  ExpectCorrect(outputs, "Gets words right.", &lstm_dict_, &words);
}

TEST_F(RecodeBeamTest, DISABLED_ChiDictionary) {
  LOG(INFO) << "Testing zh_hans dictionary" << "\n";
  LoadUnicharset("zh_hans.unicharset");
  GENERIC_2D_ARRAY<float> outputs = GenerateSyntheticOutputs(
      kZHTops, kZHTopScores, kZH2nds, kZH2ndScores, nullptr);
  PointerVector<WERD_RES> words;
  ExpectCorrect(outputs, "实学储啬投学生", nullptr, &words);
  // Each is an individual word, with permuter = top choice.
  EXPECT_EQ(7, words.size());
  for (int w = 0; w < words.size(); ++w) {
    EXPECT_EQ(TOP_CHOICE_PERM, words[w]->best_choice->permuter());
  }
  // Now try again with the dictionary.
  LoadDict("zh_hans");
  ExpectCorrect(outputs, "实学储啬投学生", &lstm_dict_, &words);
  // Number of words expected.
  const int kNumWords = 5;
  // Content of the words.
  const char* kWords[kNumWords] = {"实学", "储", "啬", "投", "学生"};
  // Permuters of the words.
  const int kWordPerms[kNumWords] = {SYSTEM_DAWG_PERM, TOP_CHOICE_PERM,
                                     TOP_CHOICE_PERM, TOP_CHOICE_PERM,
                                     SYSTEM_DAWG_PERM};
  EXPECT_EQ(kNumWords, words.size());
  for (int w = 0; w < kNumWords && w < words.size(); ++w) {
    EXPECT_STREQ(kWords[w], words[w]->best_choice->unichar_string().c_str());
    EXPECT_EQ(kWordPerms[w], words[w]->best_choice->permuter());
  }
}

// Tests that a recoder built with decomposed unicode allows true ctc
// arbitrary duplicates and inserted nulls inside the multicode sequence.
TEST_F(RecodeBeamTest, DISABLED_MultiCodeSequences) {
  LOG(INFO) << "Testing duplicates in multi-code sequences"  << "\n";
  LoadUnicharset("vie.d.unicharset");
  tesseract::SetupBasicProperties(false, true, &ccutil_.unicharset);
  TRand random;
  GENERIC_2D_ARRAY<float> outputs = GenerateSyntheticOutputs(
      kViTops, kViTopScores, kVi2nds, kVi2ndScores, &random);
  PointerVector<WERD_RES> words;
  std::string truth_str;
  tesseract::NormalizeUTF8String(
      tesseract::UnicodeNormMode::kNFKC, tesseract::OCRNorm::kNormalize,
      tesseract::GraphemeNorm::kNone, "vậy tội", &truth_str);
  ExpectCorrect(outputs, truth_str, nullptr, &words);
}

}  // namespace
