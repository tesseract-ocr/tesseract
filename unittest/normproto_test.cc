///////////////////////////////////////////////////////////////////////
// File:        normproto_test.cc
// Description: Tests that Classify::ReadNormProtos handles a normproto
//              line whose first (unichar) token exceeds the
//              unichar[2 * UNICHAR_LEN + 1] stack buffer. The
//              istream extraction has no intrinsic length limit, so a
//              crafted NORMPROTO component in a .traineddata file
//              could overflow the stack buffer during legacy engine
//              initialization.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
//
///////////////////////////////////////////////////////////////////////

#include "include_gunit.h"

#include "classify.h"
#include "serialis.h" // for TFile

#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

namespace tesseract {
namespace {

// Minimal unicharset (space and 'a').
const char kMinUnicharset[] =
    "2\n"
    "NULL 1 0,255,0,255,0,0,0,0,0,0 Latin 2 0 2\n"
    "a 1 0,255,0,255,0,0,0,0,0,0 Latin 2 0 2\n";

// Builds a normproto component: a sample-size line (5), five parameter
// description lines, then the given raw proto lines.
std::vector<char> MakeNormproto(const std::string &lines) {
  std::string data = "5\n";
  for (int i = 0; i < 5; ++i) {
    data += "e e 0 1\n";
  }
  data += lines;
  return std::vector<char>(data.begin(), data.end());
}

class NormprotoTest : public testing::Test {
protected:
  void SetUp() override {
    tmpl_ = "/tmp/tess_normproto_test_XXXXXX";
    char *dir = mkdtemp(tmpl_.data());
    ASSERT_NE(dir, nullptr);
    dir_ = dir;
    std::string uc_path = dir_ + "/eng.unicharset";
    FILE *f = fopen(uc_path.c_str(), "w");
    ASSERT_NE(f, nullptr);
    ASSERT_EQ(fwrite(kMinUnicharset, 1, sizeof(kMinUnicharset) - 1, f),
              sizeof(kMinUnicharset) - 1);
    fclose(f);
    // Load the minimal unicharset into the classifier's inherited
    // unicharset member.
    ASSERT_TRUE(classifier_.unicharset.load_from_file(uc_path.c_str()));
  }
  void TearDown() override {
    std::remove((dir_ + "/eng.unicharset").c_str());
    rmdir(dir_.c_str());
  }
  std::string dir_;
  std::string tmpl_;
  Classify classifier_;
};

// A 99-character first token (the maximum FGets can return) overflows
// unichar[2 * UNICHAR_LEN + 1] on unpatched code; the width-limited
// extraction must reject the line instead.
TEST_F(NormprotoTest, ToleratesOverlongUnicharToken) {
  std::vector<char> bytes = MakeNormproto(std::string(99, 'A') + "\n");
  TFile fp;
  ASSERT_TRUE(fp.Open(bytes.data(), bytes.size()));
  classifier_.NormProtos = classifier_.ReadNormProtos(&fp);
  ASSERT_NE(classifier_.NormProtos, nullptr);
  classifier_.FreeNormProtos();
  EXPECT_EQ(classifier_.NormProtos, nullptr);
}

// A token of exactly 2 * UNICHAR_LEN characters is the maximum legitimate
// size; it must not be truncated or rejected by the width limit.
TEST_F(NormprotoTest, ToleratesMaxLenUnicharToken) {
  std::vector<char> bytes = MakeNormproto(std::string(2 * UNICHAR_LEN, 'A') + " 0\n");
  TFile fp;
  ASSERT_TRUE(fp.Open(bytes.data(), bytes.size()));
  classifier_.NormProtos = classifier_.ReadNormProtos(&fp);
  ASSERT_NE(classifier_.NormProtos, nullptr);
  classifier_.FreeNormProtos();
  EXPECT_EQ(classifier_.NormProtos, nullptr);
}

// A well-formed normproto component must still parse.
TEST_F(NormprotoTest, ReadsValidNormprotos) {
  std::vector<char> bytes = MakeNormproto("a 0\n");
  TFile fp;
  ASSERT_TRUE(fp.Open(bytes.data(), bytes.size()));
  classifier_.NormProtos = classifier_.ReadNormProtos(&fp);
  ASSERT_NE(classifier_.NormProtos, nullptr);
  classifier_.FreeNormProtos();
  EXPECT_EQ(classifier_.NormProtos, nullptr);
}

} // namespace
} // namespace tesseract
