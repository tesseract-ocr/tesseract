
#include <cstdlib>      // for system
#include <fstream>      // for ifstream
#include <set>
#include <string>
#include <vector>

#include "ratngs.h"
#include "unicharset.h"
#include "trie.h"

#include "include_gunit.h"

namespace {

// Test some basic functionality dealing with Dawgs (compressed dictionaries,
// aka Directed Acyclic Word Graphs).
class DawgTest : public testing::Test {
 protected:
  void LoadWordlist(const std::string& filename, std::set<std::string>* words) const {
    std::ifstream file(filename);
    if (file.is_open()) {
      std::string line;
      while (getline(file, line)) {
        // Remove trailing line terminators from line.
        while (!line.empty() && (line.back() == '\n' || line.back() == '\r')) {
          line.resize(line.size() - 1);
        }
        // Add line to set.
        words->insert(line.c_str());
      }
      file.close();
    }
  }
  std::string TestDataNameToPath(const std::string& name) const {
    return file::JoinPath(TESTDATA_DIR, name);
  }
  std::string TessBinaryPath(const std::string& name) const {
    return file::JoinPath(TESSBIN_DIR, "src/training/" + name);
  }
  std::string OutputNameToPath(const std::string& name) const {
    return file::JoinPath(FLAGS_test_tmpdir, name);
  }
  int RunCommand(const std::string& program, const std::string& arg1,
                 const std::string& arg2, const std::string& arg3) const {
    std::string cmdline =
      TessBinaryPath(program) + " " + arg1 + " " + arg2 + " " + arg3;
    return system(cmdline.c_str());
  }
  // Test that we are able to convert a wordlist file (one "word" per line) to
  // a dawg (a compressed format) and then extract the original wordlist back
  // out using the tools "wordlist2dawg" and "dawg2wordlist."
  void TestDawgRoundTrip(const std::string& unicharset_filename,
                         const std::string& wordlist_filename) const {
    std::set<std::string> orig_words, roundtrip_words;
    std::string unicharset = TestDataNameToPath(unicharset_filename);
    std::string orig_wordlist = TestDataNameToPath(wordlist_filename);
    std::string output_dawg = OutputNameToPath(wordlist_filename + ".dawg");
    std::string output_wordlist = OutputNameToPath(wordlist_filename);
    LoadWordlist(orig_wordlist, &orig_words);
    EXPECT_EQ(
        RunCommand("wordlist2dawg", orig_wordlist, output_dawg, unicharset), 0);
    EXPECT_EQ(
        RunCommand("dawg2wordlist", unicharset, output_dawg, output_wordlist),
        0);
    LoadWordlist(output_wordlist, &roundtrip_words);
    EXPECT_EQ(orig_words, roundtrip_words);
  }
};

TEST_F(DawgTest, TestDawgConversion) {
  TestDawgRoundTrip("eng.unicharset", "eng.wordlist.clean.freq");
}

TEST_F(DawgTest, TestMatching) {
  UNICHARSET unicharset;
  unicharset.load_from_file(TestDataNameToPath("eng.unicharset").c_str());
  tesseract::Trie trie(tesseract::DAWG_TYPE_WORD, "basic_dawg", NGRAM_PERM,
                       unicharset.size(), 0);
  WERD_CHOICE space_apos(" '", unicharset);
  trie.add_word_to_dawg(space_apos);

  WERD_CHOICE space(" ", unicharset);

  // partial match ok - then good!
  EXPECT_TRUE(trie.prefix_in_dawg(space, false));
  // require complete match - not present.
  EXPECT_FALSE(trie.word_in_dawg(space));
  EXPECT_FALSE(trie.prefix_in_dawg(space, true));

  // partial or complete match ok for full word:
  EXPECT_TRUE(trie.prefix_in_dawg(space_apos, false));
  EXPECT_TRUE(trie.word_in_dawg(space_apos));
  EXPECT_TRUE(trie.prefix_in_dawg(space_apos, true));
}

}  // namespace
