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

#include "unicharset.h"
#include <string>
#include "gmock/gmock.h" // for testing::ElementsAreArray
#include "include_gunit.h"
#include "log.h" // for LOG

using testing::ElementsAreArray;

namespace tesseract {

class UnicharsetTest : public ::testing::Test {
protected:
  void SetUp() override {
    std::locale::global(std::locale(""));
  }
};

TEST(UnicharsetTest, Basics) {
  // This test verifies basic insertion, unichar_to_id, and encode.
  UNICHARSET u;
  u.unichar_insert("a");
  EXPECT_EQ(u.size(), 4);
  u.unichar_insert("f");
  EXPECT_EQ(u.size(), 5);
  u.unichar_insert("i");
  EXPECT_EQ(u.size(), 6);
  // The fi ligature is NOT added because it can be encoded with a cleanup as f
  // then i.
  u.unichar_insert("\ufb01");
  EXPECT_EQ(u.size(), 6);
  u.unichar_insert("e");
  EXPECT_EQ(u.size(), 7);
  u.unichar_insert("n");
  EXPECT_EQ(u.size(), 8);
  EXPECT_EQ(u.unichar_to_id("f"), 4);
  EXPECT_EQ(u.unichar_to_id("i"), 5);
  // The fi ligature has no valid id.
  EXPECT_EQ(u.unichar_to_id("\ufb01"), INVALID_UNICHAR_ID);
  // The fi pair has no valid id.
  EXPECT_EQ(u.unichar_to_id("fi"), INVALID_UNICHAR_ID);
  std::vector<int> labels;
  EXPECT_TRUE(u.encode_string("affine", true, &labels, nullptr, nullptr));
  std::vector<int> v(&labels[0], &labels[0] + labels.size());
  EXPECT_THAT(v, ElementsAreArray({3, 4, 4, 5, 7, 6}));
  // With the fi ligature encoding fails without a pre-cleanup.
  std::string lig_str = "af\ufb01ne";
  EXPECT_FALSE(u.encode_string(lig_str.c_str(), true, &labels, nullptr, nullptr));
  lig_str = u.CleanupString(lig_str.c_str());
  EXPECT_TRUE(u.encode_string(lig_str.c_str(), true, &labels, nullptr, nullptr));
  v = std::vector<int>(&labels[0], &labels[0] + labels.size());
  EXPECT_THAT(v, ElementsAreArray({3, 4, 4, 5, 7, 6}));
}

TEST(UnicharsetTest, Multibyte) {
  // This test verifies basic insertion, unichar_to_id, and encode.
  // The difference from Basic above is that now we are testing multi-byte
  // unicodes instead of single byte.
  UNICHARSET u;
  // Insert some Arabic letters.
  u.unichar_insert("\u0627");
  EXPECT_EQ(u.size(), 4);
  u.unichar_insert("\u062c");
  EXPECT_EQ(u.size(), 5);
  u.unichar_insert("\u062f");
  EXPECT_EQ(u.size(), 6);
  u.unichar_insert("\ufb01"); // fi ligature is added as fi pair.
  EXPECT_EQ(u.size(), 7);
  u.unichar_insert("\u062b");
  EXPECT_EQ(u.size(), 8);
  u.unichar_insert("\u0635");
  EXPECT_EQ(u.size(), 9);
  EXPECT_EQ(u.unichar_to_id("\u0627"), 3);
  EXPECT_EQ(u.unichar_to_id("\u062c"), 4);
  // The first two bytes of this string is \u0627, which matches id 3;
  EXPECT_EQ(u.unichar_to_id("\u0627\u062c", 2), 3);
  EXPECT_EQ(u.unichar_to_id("\u062f"), 5);
  // Individual f and i are not present, but they are there as a pair.
  EXPECT_EQ(u.unichar_to_id("f"), INVALID_UNICHAR_ID);
  EXPECT_EQ(u.unichar_to_id("i"), INVALID_UNICHAR_ID);
  EXPECT_EQ(u.unichar_to_id("fi"), 6);
  // The fi ligature is findable.
  EXPECT_EQ(u.unichar_to_id("\ufb01"), 6);
  std::vector<int> labels;
  EXPECT_TRUE(
      u.encode_string("\u0627\u062c\u062c\u062f\u0635\u062b", true, &labels, nullptr, nullptr));
  std::vector<int> v(&labels[0], &labels[0] + labels.size());
  EXPECT_THAT(v, ElementsAreArray({3, 4, 4, 5, 8, 7}));
  // With the fi ligature the fi is picked out.
  std::vector<char> lengths;
  unsigned encoded_length;
  std::string src_str = "\u0627\u062c\ufb01\u0635\u062b";
  // src_str has to be pre-cleaned for lengths to be correct.
  std::string cleaned = u.CleanupString(src_str.c_str());
  EXPECT_TRUE(u.encode_string(cleaned.c_str(), true, &labels, &lengths, &encoded_length));
  EXPECT_EQ(encoded_length, cleaned.size());
  std::string len_str(&lengths[0], lengths.size());
  EXPECT_STREQ(len_str.c_str(), "\002\002\002\002\002");
  v = std::vector<int>(&labels[0], &labels[0] + labels.size());
  EXPECT_THAT(v, ElementsAreArray({3, 4, 6, 8, 7}));
}

TEST(UnicharsetTest, MultibyteBigrams) {
  // This test verifies basic insertion, unichar_to_id, and encode.
  // The difference from Basic above is that now we are testing multi-byte
  // unicodes instead of single byte.
  UNICHARSET u;
  // Insert some Arabic letters.
  u.unichar_insert("\u0c9c");
  EXPECT_EQ(u.size(), 4);
  u.unichar_insert("\u0cad");
  EXPECT_EQ(u.size(), 5);
  u.unichar_insert("\u0ccd\u0c9c");
  EXPECT_EQ(u.size(), 6);
  u.unichar_insert("\u0ccd");
  EXPECT_EQ(u.size(), 7);
  // By default the encodable bigram is NOT added.
  u.unichar_insert("\u0ccd\u0cad");
  EXPECT_EQ(u.size(), 7);
  // It is added if we force it to be.
  u.unichar_insert("\u0ccd\u0cad", OldUncleanUnichars::kTrue);
  EXPECT_EQ(u.size(), 8);
  std::vector<char> data;
  tesseract::TFile fp;
  fp.OpenWrite(&data);
  u.save_to_file(&fp);
  fp.Open(&data[0], data.size());
  UNICHARSET v;
  v.load_from_file(&fp, false);
  EXPECT_EQ(v.unichar_to_id("\u0c9c"), 3);
  EXPECT_EQ(v.unichar_to_id("\u0cad"), 4);
  EXPECT_EQ(v.unichar_to_id("\u0ccd\u0c9c"), 5);
  EXPECT_EQ(v.unichar_to_id("\u0ccd"), 6);
  EXPECT_EQ(v.unichar_to_id("\u0ccd\u0cad"), 7);
}

TEST(UnicharsetTest, OldStyle) {
  // This test verifies an old unicharset that contains fi/fl ligatures loads
  // and keeps all the entries.
  std::string filename = file::JoinPath(TESTDATA_DIR, "eng.unicharset");
  UNICHARSET u;
  LOG(INFO) << "Filename=" << filename;
  EXPECT_TRUE(u.load_from_file(filename.c_str()));
  EXPECT_EQ(u.size(), 111);
}

} // namespace tesseract
