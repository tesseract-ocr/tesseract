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

#include "include_gunit.h"
#include "normstrngs.h"
#include "normstrngs_test.h"

namespace tesseract {

TEST(ValidateGraphemeTest, MultipleSyllablesAreNotASingleGrapheme) {
  std::string str = "\u0c15\u0c3f\u0c15\u0c0e"; // KA - dep I - KA - ind E.
  std::vector<std::string> glyphs;
  EXPECT_TRUE(NormalizeCleanAndSegmentUTF8(UnicodeNormMode::kNFC, OCRNorm::kNone,
                                           GraphemeNormMode::kCombined, true, str.c_str(), &glyphs))
      << PrintString32WithUnicodes(str);
  // It made 3 graphemes.
  EXPECT_EQ(glyphs.size(), 3);
  EXPECT_EQ(glyphs[0], std::string("\u0c15\u0c3f"));
  EXPECT_EQ(glyphs[1], std::string("\u0c15"));
  EXPECT_EQ(glyphs[2], std::string("\u0c0e"));
}

TEST(ValidateGraphemeTest, SingleConsonantOK) {
  std::string str = "\u0cb9"; // HA
  std::vector<std::string> glyphs;
  EXPECT_TRUE(NormalizeCleanAndSegmentUTF8(UnicodeNormMode::kNFC, OCRNorm::kNone,
                                           GraphemeNormMode::kCombined, true, str.c_str(), &glyphs))
      << PrintString32WithUnicodes(str);
  EXPECT_EQ(glyphs.size(), 1);
  EXPECT_EQ(glyphs[0], str);
}

TEST(ValidateGraphemeTest, SimpleCV) {
  std::string str = "\u0cb9\u0cbf"; // HA I
  std::vector<std::string> glyphs;
  EXPECT_TRUE(NormalizeCleanAndSegmentUTF8(UnicodeNormMode::kNFC, OCRNorm::kNone,
                                           GraphemeNormMode::kCombined, true, str.c_str(), &glyphs))
      << PrintString32WithUnicodes(str);
  EXPECT_EQ(glyphs.size(), 1);
  EXPECT_EQ(glyphs[0], str);
}

TEST(ValidateGraphemeTest, SubscriptConjunct) {
  std::string str = "\u0cb9\u0ccd\u0c95\u0cbf"; // HA Virama KA I
  std::vector<std::string> glyphs;
  EXPECT_TRUE(NormalizeCleanAndSegmentUTF8(UnicodeNormMode::kNFC, OCRNorm::kNone,
                                           GraphemeNormMode::kCombined, true, str.c_str(), &glyphs))
      << PrintString32WithUnicodes(str);
  EXPECT_EQ(glyphs.size(), 1);
  EXPECT_EQ(glyphs[0], str);
  EXPECT_TRUE(NormalizeCleanAndSegmentUTF8(UnicodeNormMode::kNFC, OCRNorm::kNone,
                                           GraphemeNormMode::kGlyphSplit, true, str.c_str(),
                                           &glyphs))
      << PrintString32WithUnicodes(str);
  EXPECT_EQ(glyphs.size(), 3);
  EXPECT_EQ(glyphs[1], std::string("\u0ccd\u0c95"));
}

TEST(ValidateGraphemeTest, HalfFormJoiner) {
  std::string str = "\u0d15\u0d4d\u200d\u0d24"; // KA Virama ZWJ Ta
  std::vector<std::string> glyphs;
  EXPECT_TRUE(NormalizeCleanAndSegmentUTF8(UnicodeNormMode::kNFC, OCRNorm::kNone,
                                           GraphemeNormMode::kCombined, true, str.c_str(), &glyphs))
      << PrintString32WithUnicodes(str);
  EXPECT_EQ(glyphs.size(), 1);
  EXPECT_EQ(glyphs[0], str);
  EXPECT_TRUE(NormalizeCleanAndSegmentUTF8(UnicodeNormMode::kNFC, OCRNorm::kNone,
                                           GraphemeNormMode::kGlyphSplit, true, str.c_str(),
                                           &glyphs))
      << PrintString32WithUnicodes(str);
  EXPECT_EQ(glyphs.size(), 2) << PrintStringVectorWithUnicodes(glyphs);
  EXPECT_EQ(glyphs[0], std::string("\u0d15\u0d4d\u200d"));
}

TEST(ValidateGraphemeTest, TraditionalConjunctJoiner) {
  std::string str = "\u0d15\u200d\u0d4d\u0d24"; // KA ZWI Virama Ta
  std::vector<std::string> glyphs;
  EXPECT_TRUE(NormalizeCleanAndSegmentUTF8(UnicodeNormMode::kNFC, OCRNorm::kNone,
                                           GraphemeNormMode::kCombined, true, str.c_str(), &glyphs))
      << PrintString32WithUnicodes(str);
  EXPECT_EQ(glyphs.size(), 1);
  EXPECT_EQ(glyphs[0], str);
  EXPECT_TRUE(NormalizeCleanAndSegmentUTF8(UnicodeNormMode::kNFC, OCRNorm::kNone,
                                           GraphemeNormMode::kGlyphSplit, true, str.c_str(),
                                           &glyphs))
      << PrintString32WithUnicodes(str);
  EXPECT_EQ(glyphs.size(), 3);
  EXPECT_EQ(glyphs[1], std::string("\u200d\u0d4d"));
}

TEST(ValidateGraphemeTest, OpenConjunctNonJoiner) {
  std::string str = "\u0d15\u200c\u0d4d\u0d24"; // KA ZWNJ Virama Ta
  std::vector<std::string> glyphs;
  EXPECT_TRUE(NormalizeCleanAndSegmentUTF8(UnicodeNormMode::kNFC, OCRNorm::kNone,
                                           GraphemeNormMode::kCombined, true, str.c_str(), &glyphs))
      << PrintString32WithUnicodes(str);
  EXPECT_EQ(glyphs.size(), 1);
  EXPECT_EQ(glyphs[0], str);
  EXPECT_TRUE(NormalizeCleanAndSegmentUTF8(UnicodeNormMode::kNFC, OCRNorm::kNone,
                                           GraphemeNormMode::kGlyphSplit, true, str.c_str(),
                                           &glyphs))
      << PrintString32WithUnicodes(str);
  EXPECT_EQ(glyphs.size(), 3);
  EXPECT_EQ(glyphs[1], std::string("\u200c\u0d4d"));
  // Malaylam only, so not allowed in Telugu.
  str = "\u0c15\u200c\u0c4d\u0c24"; // KA ZWNJ Virama Ta
  EXPECT_FALSE(NormalizeCleanAndSegmentUTF8(UnicodeNormMode::kNFC, OCRNorm::kNone,
                                            GraphemeNormMode::kCombined, true, str.c_str(),
                                            &glyphs))
      << PrintString32WithUnicodes(str);
}

TEST(ValidateGraphemeTest, ExplicitViramaNonJoiner) {
  std::string str = "\u0d15\u0d4d\u200c\u0d24"; // KA Virama ZWNJ Ta
  std::vector<std::string> glyphs;
  EXPECT_TRUE(NormalizeCleanAndSegmentUTF8(UnicodeNormMode::kNFC, OCRNorm::kNone,
                                           GraphemeNormMode::kCombined, true, str.c_str(), &glyphs))
      << PrintString32WithUnicodes(str);
  EXPECT_EQ(glyphs.size(), 2);
  EXPECT_EQ(glyphs[1], std::string("\u0d24"));
  EXPECT_TRUE(NormalizeCleanAndSegmentUTF8(UnicodeNormMode::kNFC, OCRNorm::kNone,
                                           GraphemeNormMode::kGlyphSplit, true, str.c_str(),
                                           &glyphs))
      << PrintString32WithUnicodes(str);
  EXPECT_EQ(glyphs.size(), 3);
  EXPECT_EQ(glyphs[1], std::string("\u0d4d\u200c"));
}

TEST(ValidateGraphemeTest, ThaiGraphemes) {
  // This is a single grapheme unless in glyph split mode
  std::string str = "\u0e14\u0e38\u0e4a";
  std::vector<std::string> glyphs;
  EXPECT_TRUE(NormalizeCleanAndSegmentUTF8(UnicodeNormMode::kNFC, OCRNorm::kNone,
                                           GraphemeNormMode::kCombined, true, str.c_str(), &glyphs))
      << PrintString32WithUnicodes(str);
  EXPECT_EQ(glyphs.size(), 1);
  EXPECT_EQ(glyphs[0], str);
  EXPECT_TRUE(NormalizeCleanAndSegmentUTF8(UnicodeNormMode::kNFC, OCRNorm::kNone,
                                           GraphemeNormMode::kGlyphSplit, true, str.c_str(),
                                           &glyphs))
      << PrintString32WithUnicodes(str);
  EXPECT_EQ(glyphs.size(), 3);
  EXPECT_EQ(glyphs[0], std::string("\u0e14"));
}

TEST(ValidateGraphemeTest, NoLonelyJoinersQuote) {
  std::string str = "'\u0d24\u0d23\u0d32\u0d4d'\u200d";
  std::vector<std::string> glyphs;
  // Returns true, but the joiner is gone.
  EXPECT_TRUE(NormalizeCleanAndSegmentUTF8(UnicodeNormMode::kNFC, OCRNorm::kNone,
                                           GraphemeNormMode::kCombined, true, str.c_str(), &glyphs))
      << PrintString32WithUnicodes(str);
  EXPECT_EQ(glyphs.size(), 5);
  EXPECT_EQ(glyphs[0], std::string("'"));
  EXPECT_EQ(glyphs[1], std::string("\u0d24"));
  EXPECT_EQ(glyphs[2], std::string("\u0d23"));
  EXPECT_EQ(glyphs[3], std::string("\u0d32\u0d4d\u200c"));
  EXPECT_EQ(glyphs[4], std::string("'"));
}

} // namespace tesseract
