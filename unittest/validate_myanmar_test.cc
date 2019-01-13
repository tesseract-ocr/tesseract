#include "tesseract/training/normstrngs.h"

#include "tesseract/unittest/normstrngs_test.h"

namespace tesseract {
namespace {

// Test some random Myanmar words.
TEST(ValidateMyanmarTest, GoodMyanmarWords) {
  string str = "လျှာကသိသည် ";  // No viramas in this one.
  ExpectGraphemeModeResults(str, UnicodeNormMode::kNFC, 11, 11, 5, str);
  str = "တုန္လႈပ္မႈ ";
  ExpectGraphemeModeResults(str, UnicodeNormMode::kNFC, 11, 9, 4, str);
}

// Test some random Myanmar words with dotted circles.
TEST(ValidateMyanmarTest, BadMyanmarWords) {
  string str = "က်န္းမာေရး";
  std::vector<string> glyphs;
  EXPECT_FALSE(NormalizeCleanAndSegmentUTF8(
      UnicodeNormMode::kNFC, OCRNorm::kNone, GraphemeNormMode::kCombined, true,
      str.c_str(), &glyphs));
  string result;
  EXPECT_FALSE(NormalizeUTF8String(UnicodeNormMode::kNFC, OCRNorm::kNone,
                                   GraphemeNorm::kNormalize, str.c_str(),
                                   &result));
  // It works if the grapheme normalization is turned off.
  EXPECT_TRUE(NormalizeUTF8String(UnicodeNormMode::kNFC, OCRNorm::kNone,
                                  GraphemeNorm::kNone, str.c_str(), &result));
  EXPECT_EQ(str, result);
  str = "ခုႏွစ္";
  EXPECT_FALSE(NormalizeCleanAndSegmentUTF8(
      UnicodeNormMode::kNFC, OCRNorm::kNone, GraphemeNormMode::kGlyphSplit,
      true, str.c_str(), &glyphs));
  EXPECT_FALSE(NormalizeUTF8String(UnicodeNormMode::kNFC, OCRNorm::kNone,
                                   GraphemeNorm::kNormalize, str.c_str(),
                                   &result));
  // It works if the grapheme normalization is turned off.
  EXPECT_TRUE(NormalizeUTF8String(UnicodeNormMode::kNFC, OCRNorm::kNone,
                                  GraphemeNorm::kNone, str.c_str(), &result));
  EXPECT_EQ(str, result);
}

}  // namespace
}  // namespace tesseract
