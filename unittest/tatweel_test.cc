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

#include <filesystem>
#include "dawg.h"
#include "include_gunit.h"
#include "trie.h"
#include "unicharset.h"
#include "util/utf8/unicodetext.h" // for UnicodeText

namespace tesseract {

class TatweelTest : public ::testing::Test {
protected:
  void SetUp() override {
    static std::locale system_locale("");
    std::locale::global(system_locale);
  }

  TatweelTest() {
    std::string filename = TestDataNameToPath("ara.wordlist");
    if (std::filesystem::exists(filename)) {
      std::string wordlist("\u0640");
      CHECK_OK(file::GetContents(filename, &wordlist, file::Defaults()));
      // Put all the unicodes in the unicharset_.
      UnicodeText text;
      text.PointToUTF8(wordlist.data(), wordlist.size());
      int num_tatweel = 0;
      for (auto it = text.begin(); it != text.end(); ++it) {
        std::string utf8 = it.get_utf8_string();
        if (utf8.find("\u0640") != std::string::npos)
          ++num_tatweel;
        unicharset_.unichar_insert(utf8.c_str());
      }
      LOG(INFO) << "Num tatweels in source data=" << num_tatweel;
      EXPECT_GT(num_tatweel, 0);
    }
  }

  std::string TestDataNameToPath(const std::string &name) {
    return file::JoinPath(TESTDATA_DIR, name);
  }
  UNICHARSET unicharset_;
};

TEST_F(TatweelTest, UnicharsetIgnoresTatweel) {
  // This test verifies that the unicharset ignores the Tatweel character.
  for (int i = 0; i < unicharset_.size(); ++i) {
    const char *utf8 = unicharset_.id_to_unichar(i);
    EXPECT_EQ(strstr(utf8, reinterpret_cast<const char *>(u8"\u0640")), nullptr);
  }
}

TEST_F(TatweelTest, DictIgnoresTatweel) {
  // This test verifies that the dictionary ignores the Tatweel character.
  tesseract::Trie trie(tesseract::DAWG_TYPE_WORD, "ara", SYSTEM_DAWG_PERM, unicharset_.size(), 0);
  std::string filename = TestDataNameToPath("ara.wordlist");
  if (!std::filesystem::exists(filename)) {
    LOG(INFO) << "Skip test because of missing " << filename;
    GTEST_SKIP();
  } else {
    EXPECT_TRUE(trie.read_and_add_word_list(filename.c_str(), unicharset_,
                                            tesseract::Trie::RRP_REVERSE_IF_HAS_RTL));
    EXPECT_EQ(0, trie.check_for_words(filename.c_str(), unicharset_, false));
  }
}

TEST_F(TatweelTest, UnicharsetLoadKeepsTatweel) {
  // This test verifies that a load of an existing unicharset keeps any
  // existing tatweel for backwards compatibility.
  std::string filename = TestDataNameToPath("ara.unicharset");
  if (!std::filesystem::exists(filename)) {
    LOG(INFO) << "Skip test because of missing " << filename;
    GTEST_SKIP();
  } else {
    EXPECT_TRUE(unicharset_.load_from_file(filename.c_str()));
    int num_tatweel = 0;
    for (int i = 0; i < unicharset_.size(); ++i) {
      const char *utf8 = unicharset_.id_to_unichar(i);
      if (strstr(utf8, reinterpret_cast<const char *>(u8"\u0640")) != nullptr) {
        ++num_tatweel;
      }
    }
    LOG(INFO) << "Num tatweels in unicharset=" << num_tatweel;
    EXPECT_EQ(num_tatweel, 4);
  }
}

} // namespace tesseract
