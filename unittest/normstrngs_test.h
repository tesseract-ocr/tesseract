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

#ifndef TESSERACT_UNITTEST_NORMSTRNGS_TEST_H_
#define TESSERACT_UNITTEST_NORMSTRNGS_TEST_H_

#include <sstream>  // for std::stringstream
#include <string>
#include <vector>
#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "unichar.h"

namespace tesseract {

inline std::string CodepointList(const std::vector<char32>& str32) {
  std::stringstream result;
  int total_chars = str32.size();
  result << std::hex;
  for (int i = 0; i < total_chars; ++i) {
    result << "[" << str32[i] << "]";
  }
  return result.str();
}

inline std::string PrintString32WithUnicodes(const std::string& str) {
  std::vector<char32> str32 = UNICHAR::UTF8ToUTF32(str.c_str());
  return absl::StrCat("\"", str, "\" ", CodepointList(str32));
}

inline std::string PrintStringVectorWithUnicodes(const std::vector<std::string>& glyphs) {
  std::string result;
  for (const auto& s : glyphs) {
    result += "Glyph:";
    result += PrintString32WithUnicodes(s) + "\n";
  }
  return result;
}

inline void ExpectGraphemeModeResults(const std::string& str, UnicodeNormMode u_mode,
                                      int unicode_count, int glyph_count,
                                      int grapheme_count,
                                      const std::string& target_str) {
  std::vector<std::string> glyphs;
  EXPECT_TRUE(NormalizeCleanAndSegmentUTF8(
      u_mode, OCRNorm::kNone, GraphemeNormMode::kIndividualUnicodes, true,
      str.c_str(), &glyphs));
  EXPECT_EQ(glyphs.size(), unicode_count)
      << PrintStringVectorWithUnicodes(glyphs);
  EXPECT_EQ(target_str, absl::StrJoin(glyphs.begin(), glyphs.end(), ""));
  EXPECT_TRUE(NormalizeCleanAndSegmentUTF8(u_mode, OCRNorm::kNone,
                                           GraphemeNormMode::kGlyphSplit, true,
                                           str.c_str(), &glyphs));
  EXPECT_EQ(glyphs.size(), glyph_count)
      << PrintStringVectorWithUnicodes(glyphs);
  EXPECT_EQ(target_str, absl::StrJoin(glyphs.begin(), glyphs.end(), ""));
  EXPECT_TRUE(NormalizeCleanAndSegmentUTF8(u_mode, OCRNorm::kNone,
                                           GraphemeNormMode::kCombined, true,
                                           str.c_str(), &glyphs));
  EXPECT_EQ(glyphs.size(), grapheme_count)
      << PrintStringVectorWithUnicodes(glyphs);
  EXPECT_EQ(target_str, absl::StrJoin(glyphs.begin(), glyphs.end(), ""));
  EXPECT_TRUE(NormalizeCleanAndSegmentUTF8(u_mode, OCRNorm::kNone,
                                           GraphemeNormMode::kSingleString,
                                           true, str.c_str(), &glyphs));
  EXPECT_EQ(glyphs.size(), 1) << PrintStringVectorWithUnicodes(glyphs);
  EXPECT_EQ(target_str, glyphs[0]);
  std::string result;
  EXPECT_TRUE(NormalizeUTF8String(
      u_mode, OCRNorm::kNone, GraphemeNorm::kNormalize, str.c_str(), &result));
  EXPECT_EQ(target_str, result);
}

}  // namespace tesseract

#endif  // TESSERACT_UNITTEST_NORMSTRNGS_TEST_H_
