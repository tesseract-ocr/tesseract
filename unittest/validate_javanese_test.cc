// (C) Copyright 2026, Stefan Weil
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

// A word which ends with a pangkon (virama) is valid. The validator used to
// read one element beyond the end of its codes for such a word and appended
// that value to the text.
TEST(ValidateJavaneseTest, FinalPangkon) {
  // "anak": HA NA KA PANGKON.
  std::string str = "ꦲꦤꦏ꧀";
  std::vector<std::string> glyphs;
  EXPECT_TRUE(NormalizeCleanAndSegmentUTF8(UnicodeNormMode::kNFC, OCRNorm::kNone,
                                           GraphemeNormMode::kCombined, true, str.c_str(),
                                           &glyphs));
  // The pangkon stays with its consonant.
  ASSERT_EQ(glyphs.size(), 3) << PrintStringVectorWithUnicodes(glyphs);
  EXPECT_EQ(glyphs[0], "ꦲ");
  EXPECT_EQ(glyphs[1], "ꦤ");
  EXPECT_EQ(glyphs[2], "ꦏ꧀");
  std::string result;
  EXPECT_TRUE(NormalizeUTF8String(UnicodeNormMode::kNFC, OCRNorm::kNone, GraphemeNorm::kNormalize,
                                  str.c_str(), &result));
  EXPECT_EQ(str, result);
}

// A pangkon between two consonants makes a conjunct, which is one grapheme.
TEST(ValidateJavaneseTest, Conjunct) {
  // KA PANGKON NA.
  std::string str = "ꦏ꧀ꦤ";
  std::vector<std::string> glyphs;
  EXPECT_TRUE(NormalizeCleanAndSegmentUTF8(UnicodeNormMode::kNFC, OCRNorm::kNone,
                                           GraphemeNormMode::kCombined, true, str.c_str(),
                                           &glyphs));
  ASSERT_EQ(glyphs.size(), 1) << PrintStringVectorWithUnicodes(glyphs);
  EXPECT_EQ(glyphs[0], str);
}

} // namespace tesseract
