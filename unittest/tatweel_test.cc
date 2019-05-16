#include "tesseract/ccutil/unicharset.h"
#include "tesseract/dict/dawg.h"
#include "tesseract/dict/trie.h"
#include "util/utf8/public/unicodetext.h"

namespace {

class TatweelTest : public ::testing::Test {
 protected:
  void SetUp() {
    std::locale::global(std::locale(""));
  }

  TatweelTest() {
    string filename = TestDataNameToPath("ara.wordlist");
    string wordlist;
    CHECK_OK(file::GetContents(filename, &wordlist, file::Defaults()));
    // Put all the unicodes in the unicharset_.
    UnicodeText text;
    text.PointToUTF8(wordlist.data(), wordlist.size());
    int num_tatweel = 0;
    for (auto it = text.begin(); it != text.end(); ++it) {
      string utf8 = it.get_utf8_string();
      if (utf8.find(u8"\u0640") != string::npos) ++num_tatweel;
      unicharset_.unichar_insert(utf8.c_str());
    }
    LOG(INFO) << "Num tatweels in source data=" << num_tatweel;
    EXPECT_GT(num_tatweel, 0);
  }

  string TestDataNameToPath(const string& name) {
    return file::JoinPath(FLAGS_test_srcdir, "testdata/" + name);
  }
  UNICHARSET unicharset_;
};

TEST_F(TatweelTest, UnicharsetIgnoresTatweel) {
  // This test verifies that the unicharset ignores the Tatweel character.
  for (int i = 0; i < unicharset_.size(); ++i) {
    const char* utf8 = unicharset_.id_to_unichar(i);
    EXPECT_EQ(strstr(utf8, u8"\u0640"), nullptr);
  }
}

TEST_F(TatweelTest, DictIgnoresTatweel) {
  // This test verifies that the dictionary ignores the Tatweel character.
  tesseract::Trie trie(tesseract::DAWG_TYPE_WORD, "ara", SYSTEM_DAWG_PERM,
                       unicharset_.size(), 0);
  string filename = TestDataNameToPath("ara.wordlist");
  EXPECT_TRUE(trie.read_and_add_word_list(
      filename.c_str(), unicharset_, tesseract::Trie::RRP_REVERSE_IF_HAS_RTL));
  EXPECT_EQ(0, trie.check_for_words(filename.c_str(), unicharset_, false));
}

TEST_F(TatweelTest, UnicharsetLoadKeepsTatweel) {
  // This test verifies that a load of an existing unicharset keeps any
  // existing tatweel for backwards compatibility.
  string filename = TestDataNameToPath("ara.unicharset");
  EXPECT_TRUE(unicharset_.load_from_file(filename.c_str()));
  int num_tatweel = 0;
  for (int i = 0; i < unicharset_.size(); ++i) {
    const char* utf8 = unicharset_.id_to_unichar(i);
    if (strstr(utf8, u8"\u0640") != nullptr) ++num_tatweel;
  }
  LOG(INFO) << "Num tatweels in unicharset=" << num_tatweel;
  EXPECT_EQ(num_tatweel, 4);
}

}  // namespace
