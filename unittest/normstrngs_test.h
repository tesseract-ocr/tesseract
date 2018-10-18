#ifndef TESSERACT_UNITTEST_NORMSTRNGS_TEST_H_
#define TESSERACT_UNITTEST_NORMSTRNGS_TEST_H_

#include <string>
#include <vector>
#include "base/stringprintf.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "tesseract/ccutil/unichar.h"

namespace tesseract {

inline string CodepointList(const std::vector<char32>& str32) {
  string result;
  int total_chars = str32.size();
  for (int i = 0; i < total_chars; ++i) {
    StringAppendF(&result, "[%x]", str32[i]);
  }
  return result;
}

inline string PrintString32WithUnicodes(const string& str) {
  std::vector<char32> str32 = UNICHAR::UTF8ToUTF32(str.c_str());
  return absl::StrCat("\"", str, "\" ", CodepointList(str32));
}

inline string PrintStringVectorWithUnicodes(const std::vector<string>& glyphs) {
  string result;
  for (const auto& s : glyphs) {
    absl::StrAppend(&result, "Glyph:", PrintString32WithUnicodes(s), "\n");
  }
  return result;
}

inline void ExpectGraphemeModeResults(const string& str, UnicodeNormMode u_mode,
                                      int unicode_count, int glyph_count,
                                      int grapheme_count,
                                      const string& target_str) {
  std::vector<string> glyphs;
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
  string result;
  EXPECT_TRUE(NormalizeUTF8String(
      u_mode, OCRNorm::kNone, GraphemeNorm::kNormalize, str.c_str(), &result));
  EXPECT_EQ(target_str, result);
}

}  // namespace tesseract

#endif  // TESSERACT_UNITTEST_NORMSTRNGS_TEST_H_
