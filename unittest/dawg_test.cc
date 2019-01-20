
#include <set>
#include <string>
#include <vector>

#include "ratngs.h"
#include "unicharset.h"
#include "trie.h"

#include "include_gunit.h"
#include "base/filelinereader.h"
#include "util/process/subprocess.h"

namespace {

void RemoveTrailingLineTerminators(char* line) {
  char* end = line + strlen(line) - 1;
  while (end >= line && ('\n' == *end || '\r' == *end)) {
    *end-- = 0;
  }
}

void AddLineToSet(std::set<std::string>* words, char* line) {
  RemoveTrailingLineTerminators(line);
  words->insert(line);
}

// Test some basic functionality dealing with Dawgs (compressed dictionaries,
// aka Directed Acyclic Word Graphs).
class DawgTest : public testing::Test {
 protected:
  void LoadWordlist(const std::string& filename, std::set<std::string>* words) const {
    FileLineReader::Options options;
    options.set_comment_char(0);
    FileLineReader flr(filename.c_str(), options);
    flr.set_line_callback(NewPermanentCallback(AddLineToSet, words));
    flr.Reload();
  }
  std::string TestDataNameToPath(const std::string& name) const {
    return file::JoinPath(TESTDATA_DIR, "/" + name);
  }
  std::string TessBinaryPath(const std::string& binary_name) const {
    return file::JoinPath(TESS_SRC_DIR,
  }
  std::string OutputNameToPath(const std::string& name) const {
    return file::JoinPath(FLAGS_test_tmpdir, name);
  }
  int RunCommand(const std::string& program, const std::string& arg1, const std::string& arg2,
                 const std::string& arg3) const {
    SubProcess p;
    std::vector<std::string> argv;
    argv.push_back(program);
    argv.push_back(arg1);
    argv.push_back(arg2);
    argv.push_back(arg3);
    p.SetProgram(TessBinaryPath(program), argv);
    p.Start();
    p.Wait();
    return p.exit_code();
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
