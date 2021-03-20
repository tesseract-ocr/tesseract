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

// Test some random Myanmar words.
TEST(ValidateMyanmarTest, GoodMyanmarWords) {
  std::string str = "လျှာကသိသည် "; // No viramas in this one.
  ExpectGraphemeModeResults(str, UnicodeNormMode::kNFC, 11, 11, 5, str);
  str = "တုန္လႈပ္မႈ ";
  ExpectGraphemeModeResults(str, UnicodeNormMode::kNFC, 11, 9, 4, str);
}

// Test some random Myanmar words with dotted circles.
TEST(ValidateMyanmarTest, BadMyanmarWords) {
  std::string str = "က်န္းမာေရး";
  std::vector<std::string> glyphs;
  EXPECT_FALSE(NormalizeCleanAndSegmentUTF8(UnicodeNormMode::kNFC, OCRNorm::kNone,
                                            GraphemeNormMode::kCombined, true, str.c_str(),
                                            &glyphs));
  std::string result;
  EXPECT_FALSE(NormalizeUTF8String(UnicodeNormMode::kNFC, OCRNorm::kNone, GraphemeNorm::kNormalize,
                                   str.c_str(), &result));
  // It works if the grapheme normalization is turned off.
  EXPECT_TRUE(NormalizeUTF8String(UnicodeNormMode::kNFC, OCRNorm::kNone, GraphemeNorm::kNone,
                                  str.c_str(), &result));
  EXPECT_EQ(str, result);
  str = "ခုႏွစ္";
  EXPECT_FALSE(NormalizeCleanAndSegmentUTF8(UnicodeNormMode::kNFC, OCRNorm::kNone,
                                            GraphemeNormMode::kGlyphSplit, true, str.c_str(),
                                            &glyphs));
  EXPECT_FALSE(NormalizeUTF8String(UnicodeNormMode::kNFC, OCRNorm::kNone, GraphemeNorm::kNormalize,
                                   str.c_str(), &result));
  // It works if the grapheme normalization is turned off.
  EXPECT_TRUE(NormalizeUTF8String(UnicodeNormMode::kNFC, OCRNorm::kNone, GraphemeNorm::kNone,
                                  str.c_str(), &result));
  EXPECT_EQ(str, result);
}

} // namespace tesseract
