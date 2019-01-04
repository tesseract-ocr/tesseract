#include "tesseract/training/normstrngs.h"

#include "tesseract/unittest/normstrngs_test.h"

namespace tesseract {
namespace {

TEST(ValidateGraphemeTest, MultipleSyllablesAreNotASingleGrapheme) {
  string str = "\u0c15\u0c3f\u0c15\u0c0e";  // KA - dep I - KA - ind E.
  std::vector<string> glyphs;
  EXPECT_TRUE(NormalizeCleanAndSegmentUTF8(
      UnicodeNormMode::kNFC, OCRNorm::kNone, GraphemeNormMode::kCombined, true,
      str.c_str(), &glyphs))
      << PrintString32WithUnicodes(str);
  // It made 3 graphemes.
  EXPECT_EQ(glyphs.size(), 3);
  EXPECT_EQ(glyphs[0], string("\u0c15\u0c3f"));
  EXPECT_EQ(glyphs[1], string("\u0c15"));
  EXPECT_EQ(glyphs[2], string("\u0c0e"));
}

TEST(ValidateGraphemeTest, SingleConsonantOK) {
  string str = "\u0cb9";  // HA
  std::vector<string> glyphs;
  EXPECT_TRUE(NormalizeCleanAndSegmentUTF8(
      UnicodeNormMode::kNFC, OCRNorm::kNone, GraphemeNormMode::kCombined, true,
      str.c_str(), &glyphs))
      << PrintString32WithUnicodes(str);
  EXPECT_EQ(glyphs.size(), 1);
  EXPECT_EQ(glyphs[0], str);
}

TEST(ValidateGraphemeTest, SimpleCV) {
  string str = "\u0cb9\u0cbf";  // HA I
  std::vector<string> glyphs;
  EXPECT_TRUE(NormalizeCleanAndSegmentUTF8(
      UnicodeNormMode::kNFC, OCRNorm::kNone, GraphemeNormMode::kCombined, true,
      str.c_str(), &glyphs))
      << PrintString32WithUnicodes(str);
  EXPECT_EQ(glyphs.size(), 1);
  EXPECT_EQ(glyphs[0], str);
}

TEST(ValidateGraphemeTest, SubscriptConjunct) {
  string str = "\u0cb9\u0ccd\u0c95\u0cbf";  // HA Virama KA I
  std::vector<string> glyphs;
  EXPECT_TRUE(NormalizeCleanAndSegmentUTF8(
      UnicodeNormMode::kNFC, OCRNorm::kNone, GraphemeNormMode::kCombined, true,
      str.c_str(), &glyphs))
      << PrintString32WithUnicodes(str);
  EXPECT_EQ(glyphs.size(), 1);
  EXPECT_EQ(glyphs[0], str);
  EXPECT_TRUE(NormalizeCleanAndSegmentUTF8(
      UnicodeNormMode::kNFC, OCRNorm::kNone, GraphemeNormMode::kGlyphSplit,
      true, str.c_str(), &glyphs))
      << PrintString32WithUnicodes(str);
  EXPECT_EQ(glyphs.size(), 3);
  EXPECT_EQ(glyphs[1], string("\u0ccd\u0c95"));
}

TEST(ValidateGraphemeTest, HalfFormJoiner) {
  string str = "\u0d15\u0d4d\u200d\u0d24";  // KA Virama ZWJ Ta
  std::vector<string> glyphs;
  EXPECT_TRUE(NormalizeCleanAndSegmentUTF8(
      UnicodeNormMode::kNFC, OCRNorm::kNone, GraphemeNormMode::kCombined, true,
      str.c_str(), &glyphs))
      << PrintString32WithUnicodes(str);
  EXPECT_EQ(glyphs.size(), 1);
  EXPECT_EQ(glyphs[0], str);
  EXPECT_TRUE(NormalizeCleanAndSegmentUTF8(
      UnicodeNormMode::kNFC, OCRNorm::kNone, GraphemeNormMode::kGlyphSplit,
      true, str.c_str(), &glyphs))
      << PrintString32WithUnicodes(str);
  EXPECT_EQ(glyphs.size(), 2) << PrintStringVectorWithUnicodes(glyphs);
  EXPECT_EQ(glyphs[0], string("\u0d15\u0d4d\u200d"));
}

TEST(ValidateGraphemeTest, TraditionalConjunctJoiner) {
  string str = "\u0d15\u200d\u0d4d\u0d24";  // KA ZWI Virama Ta
  std::vector<string> glyphs;
  EXPECT_TRUE(NormalizeCleanAndSegmentUTF8(
      UnicodeNormMode::kNFC, OCRNorm::kNone, GraphemeNormMode::kCombined, true,
      str.c_str(), &glyphs))
      << PrintString32WithUnicodes(str);
  EXPECT_EQ(glyphs.size(), 1);
  EXPECT_EQ(glyphs[0], str);
  EXPECT_TRUE(NormalizeCleanAndSegmentUTF8(
      UnicodeNormMode::kNFC, OCRNorm::kNone, GraphemeNormMode::kGlyphSplit,
      true, str.c_str(), &glyphs))
      << PrintString32WithUnicodes(str);
  EXPECT_EQ(glyphs.size(), 3);
  EXPECT_EQ(glyphs[1], string("\u200d\u0d4d"));
}

TEST(ValidateGraphemeTest, OpenConjunctNonJoiner) {
  string str = "\u0d15\u200c\u0d4d\u0d24";  // KA ZWNJ Virama Ta
  std::vector<string> glyphs;
  EXPECT_TRUE(NormalizeCleanAndSegmentUTF8(
      UnicodeNormMode::kNFC, OCRNorm::kNone, GraphemeNormMode::kCombined, true,
      str.c_str(), &glyphs))
      << PrintString32WithUnicodes(str);
  EXPECT_EQ(glyphs.size(), 1);
  EXPECT_EQ(glyphs[0], str);
  EXPECT_TRUE(NormalizeCleanAndSegmentUTF8(
      UnicodeNormMode::kNFC, OCRNorm::kNone, GraphemeNormMode::kGlyphSplit,
      true, str.c_str(), &glyphs))
      << PrintString32WithUnicodes(str);
  EXPECT_EQ(glyphs.size(), 3);
  EXPECT_EQ(glyphs[1], string("\u200c\u0d4d"));
  // Malaylam only, so not allowed in Telugu.
  str = "\u0c15\u200c\u0c4d\u0c24";  // KA ZWNJ Virama Ta
  EXPECT_FALSE(NormalizeCleanAndSegmentUTF8(
      UnicodeNormMode::kNFC, OCRNorm::kNone, GraphemeNormMode::kCombined, true,
      str.c_str(), &glyphs))
      << PrintString32WithUnicodes(str);
}

TEST(ValidateGraphemeTest, ExplicitViramaNonJoiner) {
  string str = "\u0d15\u0d4d\u200c\u0d24";  // KA Virama ZWNJ Ta
  std::vector<string> glyphs;
  EXPECT_TRUE(NormalizeCleanAndSegmentUTF8(
      UnicodeNormMode::kNFC, OCRNorm::kNone, GraphemeNormMode::kCombined, true,
      str.c_str(), &glyphs))
      << PrintString32WithUnicodes(str);
  EXPECT_EQ(glyphs.size(), 2);
  EXPECT_EQ(glyphs[1], string("\u0d24"));
  EXPECT_TRUE(NormalizeCleanAndSegmentUTF8(
      UnicodeNormMode::kNFC, OCRNorm::kNone, GraphemeNormMode::kGlyphSplit,
      true, str.c_str(), &glyphs))
      << PrintString32WithUnicodes(str);
  EXPECT_EQ(glyphs.size(), 3);
  EXPECT_EQ(glyphs[1], string("\u0d4d\u200c"));
}

TEST(ValidateGraphemeTest, ThaiGraphemes) {
  // This is a single grapheme unless in glyph split mode
  string str = "\u0e14\u0e38\u0e4a";
  std::vector<string> glyphs;
  EXPECT_TRUE(NormalizeCleanAndSegmentUTF8(
      UnicodeNormMode::kNFC, OCRNorm::kNone, GraphemeNormMode::kCombined, true,
      str.c_str(), &glyphs))
      << PrintString32WithUnicodes(str);
  EXPECT_EQ(glyphs.size(), 1);
  EXPECT_EQ(glyphs[0], str);
  EXPECT_TRUE(NormalizeCleanAndSegmentUTF8(
      UnicodeNormMode::kNFC, OCRNorm::kNone, GraphemeNormMode::kGlyphSplit,
      true, str.c_str(), &glyphs))
      << PrintString32WithUnicodes(str);
  EXPECT_EQ(glyphs.size(), 3);
  EXPECT_EQ(glyphs[0], string("\u0e14"));
}

TEST(ValidateGraphemeTest, NoLonelyJoinersQuote) {
  string str = "'\u0d24\u0d23\u0d32\u0d4d'\u200d";
  std::vector<string> glyphs;
  // Returns true, but the joiner is gone.
  EXPECT_TRUE(NormalizeCleanAndSegmentUTF8(
      UnicodeNormMode::kNFC, OCRNorm::kNone, GraphemeNormMode::kCombined, true,
      str.c_str(), &glyphs))
      << PrintString32WithUnicodes(str);
  EXPECT_EQ(glyphs.size(), 5);
  EXPECT_EQ(glyphs[0], string("'"));
  EXPECT_EQ(glyphs[1], string("\u0d24"));
  EXPECT_EQ(glyphs[2], string("\u0d23"));
  EXPECT_EQ(glyphs[3], string("\u0d32\u0d4d\u200c"));
  EXPECT_EQ(glyphs[4], string("'"));
}

}  // namespace
}  // namespace tesseract
