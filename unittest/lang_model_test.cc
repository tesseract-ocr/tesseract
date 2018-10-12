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

#include <string>                       // for std::string

#include "absl/strings/str_cat.h"

#include "gmock/gmock.h"                // for testing::ElementsAreArray

#include "include_gunit.h"
#include "lang_model_helpers.h"
#include "log.h"                        // for LOG
#include "lstmtrainer.h"
#include "unicharset_training_utils.h"

namespace tesseract {
namespace {

std::string TestDataNameToPath(const std::string& name) {
  return file::JoinPath(TESTING_DIR, name);
}

// This is an integration test that verifies that CombineLangModel works to
// the extent that an LSTMTrainer can be initialized with the result, and it
// can encode strings. More importantly, the test verifies that adding an extra
// character to the unicharset does not change the encoding of strings.
TEST(LangModelTest, AddACharacter) {
  constexpr char kTestString[] = "Simple ASCII string to encode !@#$%&";
  constexpr char kTestStringRupees[] = "ASCII string with Rupee symbol ₹";
  // Setup the arguments.
  std::string script_dir = LANGDATA_DIR;
  std::string eng_dir = file::JoinPath(script_dir, "eng");
  std::string unicharset_path = TestDataNameToPath("eng_beam.unicharset");
  UNICHARSET unicharset;
  EXPECT_TRUE(unicharset.load_from_file(unicharset_path.c_str()));
  std::string version_str = "TestVersion";
  std::string output_dir = FLAGS_test_tmpdir;
  LOG(INFO) << "Output dir=" << output_dir;
  std::string lang1 = "eng";
  bool pass_through_recoder = false;
  GenericVector<STRING> words, puncs, numbers;
  // If these reads fail, we get a warning message and an empty list of words.
  ReadFile(file::JoinPath(eng_dir, "eng.wordlist"), nullptr)
      .split('\n', &words);
  EXPECT_GT(words.size(), 0);
  ReadFile(file::JoinPath(eng_dir, "eng.punc"), nullptr).split('\n', &puncs);
  EXPECT_GT(puncs.size(), 0);
  ReadFile(file::JoinPath(eng_dir, "eng.numbers"), nullptr)
      .split('\n', &numbers);
  EXPECT_GT(numbers.size(), 0);
  bool lang_is_rtl = false;
  // Generate the traineddata file.
  EXPECT_EQ(0, CombineLangModel(unicharset, script_dir, version_str, output_dir,
                                lang1, pass_through_recoder, words, puncs,
                                numbers, lang_is_rtl, nullptr, nullptr));
  // Init a trainer with it, and encode a string.
  std::string traineddata1 =
      file::JoinPath(output_dir, lang1, absl::StrCat(lang1, ".traineddata"));
  LSTMTrainer trainer1;
  trainer1.InitCharSet(traineddata1);
  GenericVector<int> labels1;
  EXPECT_TRUE(trainer1.EncodeString(kTestString, &labels1));

  // Add a new character to the unicharset and try again.
  int size_before = unicharset.size();
  unicharset.unichar_insert("₹");
  SetupBasicProperties(/*report_errors*/ true, /*decompose (NFD)*/ false,
                       &unicharset);
  EXPECT_EQ(size_before + 1, unicharset.size());
  // Generate the traineddata file.
  std::string lang2 = "extended";
  EXPECT_EQ(EXIT_SUCCESS,
            CombineLangModel(unicharset, script_dir, version_str, output_dir,
                             lang2, pass_through_recoder, words, puncs, numbers,
                             lang_is_rtl, nullptr, nullptr));
  // Init a trainer with it, and encode a string.
  std::string traineddata2 =
      file::JoinPath(output_dir, lang2, absl::StrCat(lang2, ".traineddata"));
  LSTMTrainer trainer2;
  trainer2.InitCharSet(traineddata2);
  GenericVector<int> labels2;
  EXPECT_TRUE(trainer2.EncodeString(kTestString, &labels2));
  // Copy labels1 to a std::vector, renumbering the null char to match trainer2.
  // Since Tensor Flow's CTC implementation insists on having the null be the
  // last label, and we want to be compatible, null has to be renumbered when
  // we add a class.
  int null1 = trainer1.null_char();
  int null2 = trainer2.null_char();
  EXPECT_EQ(null1 + 1, null2);
  std::vector<int> labels1_v(labels1.size());
  for (int i = 0; i < labels1.size(); ++i) {
    if (labels1[i] == null1)
      labels1_v[i] = null2;
    else
      labels1_v[i] = labels1[i];
  }
  EXPECT_THAT(labels1_v,
              testing::ElementsAreArray(&labels2[0], labels2.size()));
  // To make sure we we are not cheating somehow, we can now encode the Rupee
  // symbol, which we could not do before.
  EXPECT_FALSE(trainer1.EncodeString(kTestStringRupees, &labels1));
  EXPECT_TRUE(trainer2.EncodeString(kTestStringRupees, &labels2));
}

}  // namespace
}  // namespace tesseract
