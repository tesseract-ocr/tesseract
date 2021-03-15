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

// Though the unicode example for Telugu in section 12.7:
// http://www.unicode.org/versions/Unicode9.0.0/ch12.pdf
// shows using ZWNJ to force an explicit virama, in practice a ZWNJ is used to
// suppress a conjugate that would otherwise occur.  If a consonant is followed
// by a virama and then by a non-Indic character, OpenType will presume that
// the user simply meant to suppress the inherent vowel of the consonant
// and render it as the consonant with an explicit virama, the same as if
// a ZWNJ had followed. Since this is confusing to an OCR engine, the
// normalizer always puts a termninating ZWNJ on the end if not present,
// and accepts the string as valid.
TEST(ValidateIndicTest, AddsJoinerToTerminalVirama) {
  std::string str = "\u0c15\u0c4d";              // KA - virama
  std::string target_str = "\u0c15\u0c4d\u200c"; // KA - virama - ZWNJ
  ExpectGraphemeModeResults(str, UnicodeNormMode::kNFC, 3, 2, 1, target_str);
  // Same result if we started with the normalized string.
  ExpectGraphemeModeResults(target_str, UnicodeNormMode::kNFC, 3, 2, 1, target_str);
}

// Only one dependent vowel is allowed.
TEST(ValidateIndicTest, OnlyOneDependentVowel) {
  std::string str = "\u0d15\u0d3e\u0d42"; // KA AA UU
  std::string dest;
  EXPECT_FALSE(NormalizeUTF8String(UnicodeNormMode::kNFC, OCRNorm::kNone, GraphemeNorm::kNormalize,
                                   str.c_str(), &dest))
      << PrintString32WithUnicodes(str);
}

//  [c26][c4d][c01]
//     A consonant (DA) followed by the virama followed by a bindu
//     Syllable modifiers [c01][c02][c03] all modify the pronunciation of
//     the vowel in a syllable, as does the virama [c04].  You can only
//     have one of these on a syllable.
//
//  References:
//    http://www.omniglot.com/writing/telugu.htm
TEST(ValidateIndicTest, OnlyOneVowelModifier) {
  std::string str = "\u0c26\u0c4d\u0c01"; // DA virama candrabindu
  std::string result;
  EXPECT_TRUE(NormalizeUTF8String(UnicodeNormMode::kNFC, OCRNorm::kNone, GraphemeNorm::kNormalize,
                                  str.c_str(), &result));
  // It made 1 grapheme of 4 chars, by terminating the explicit virama.
  EXPECT_EQ(std::string("\u0c26\u0c4d\u200c\u0c01"), result);

  str = "\u0995\u0983\u0981"; // KA visarga candrabindu
  EXPECT_FALSE(NormalizeUTF8String(UnicodeNormMode::kNFC, OCRNorm::kNone, GraphemeNorm::kNormalize,
                                   str.c_str(), &result));

  // Exception: Malayalam allows multiple anusvara.
  str = "\u0d15\u0d02\u0d02"; // KA Anusvara Anusvara
  EXPECT_TRUE(NormalizeUTF8String(UnicodeNormMode::kNFC, OCRNorm::kNone, GraphemeNorm::kNormalize,
                                  str.c_str(), &result));
  EXPECT_EQ(str, result);
}

//  [c28][c02][c3f]
//    A consonant (NA) followed by the Anusvara/sunna and another matra (I).
// The anusvara [c02] is a pronunciation directive
//    for a whole syllable and only appears at the end of the syllable
//  References:
//    + Unicode v9, 12.1 "Modifier Mark Rules R10,"
//       and the Microsoft page
//       http://www.microsoft.com/typography/otfntdev/teluguot/shaping.aspx
TEST(ValidateIndicTest, VowelModifierMustBeLast) {
  std::string str = "\u0c28\u0c02\u0c3f"; // NA Sunna I
  std::string dest;
  EXPECT_FALSE(NormalizeUTF8String(UnicodeNormMode::kNFC, OCRNorm::kNone, GraphemeNorm::kNormalize,
                                   str.c_str(), &dest))
      << PrintString32WithUnicodes(str);
  // Swap c02/c3f and all is ok.
  str = "\u0c28\u0c3f\u0c02"; // NA I Sunna
  EXPECT_TRUE(NormalizeUTF8String(UnicodeNormMode::kNFC, OCRNorm::kNone, GraphemeNorm::kNormalize,
                                  str.c_str(), &dest))
      << PrintString32WithUnicodes(str);
  EXPECT_EQ(dest, str);
}

//  [c05][c47]
//    A Vowel (A) followed by a combining vowel/matra (EE).
//    In Telugu, matras are only put on consonants, not independent
//    vowels.
//  References:
//  + Unicode v9, 12.1:
//     Principles of the Devanagari Script: Dependent Vowel Signs (Matras).
//  + http://varamozhi.sourceforge.net/iscii91.pdf
TEST(ValidateIndicTest, MatrasFollowConsonantsNotVowels) {
  std::string str = "\u0c05\u0c47"; // A EE
  std::string dest;
  EXPECT_FALSE(NormalizeUTF8String(UnicodeNormMode::kNFC, OCRNorm::kNone, GraphemeNorm::kNormalize,
                                   str.c_str(), &dest))
      << PrintString32WithUnicodes(str);
  str = "\u0c1e\u0c3e"; // NYA AA
  EXPECT_TRUE(NormalizeUTF8String(UnicodeNormMode::kNFC, OCRNorm::kNone, GraphemeNorm::kNormalize,
                                  str.c_str(), &dest))
      << PrintString32WithUnicodes(str);
  EXPECT_EQ(dest, str);
}

// Sub-graphemes are allowed if GraphemeNorm is turned off.
TEST(ValidateIndicTest, SubGraphemes) {
  std::string str = "\u0d3e"; // AA
  std::string dest;
  EXPECT_FALSE(NormalizeUTF8String(UnicodeNormMode::kNFC, OCRNorm::kNone, GraphemeNorm::kNormalize,
                                   str.c_str(), &dest))
      << PrintString32WithUnicodes(str);
  EXPECT_TRUE(NormalizeUTF8String(UnicodeNormMode::kNFC, OCRNorm::kNone, GraphemeNorm::kNone,
                                  str.c_str(), &dest))
      << PrintString32WithUnicodes(str);
  EXPECT_EQ(dest, str);
}

TEST(ValidateIndicTest, Nukta) {
  std::string str = "\u0c95\u0cbc\u0ccd\u0cb9"; // KA Nukta Virama HA
  std::vector<std::string> glyphs;
  EXPECT_TRUE(NormalizeCleanAndSegmentUTF8(UnicodeNormMode::kNFC, OCRNorm::kNone,
                                           GraphemeNormMode::kGlyphSplit, true, str.c_str(),
                                           &glyphs));
  EXPECT_EQ(glyphs.size(), 3);
  EXPECT_EQ(glyphs[2], std::string("\u0ccd\u0cb9"));
  // Swapped Nukta and Virama are not allowed, but NFC normalization fixes it.
  std::string str2 = "\u0c95\u0ccd\u0cbc\u0cb9"; // KA Virama Nukta HA
  ExpectGraphemeModeResults(str2, UnicodeNormMode::kNFC, 4, 3, 1, str);
}

// Sinhala has some of its own specific rules. See www.macciato.com/sinhala
TEST(ValidateIndicTest, SinhalaRakaransaya) {
  std::string str = "\u0d9a\u0dca\u200d\u0dbb"; // KA Virama ZWJ Rayanna
  std::string dest;
  EXPECT_TRUE(NormalizeUTF8String(UnicodeNormMode::kNFC, OCRNorm::kNone, GraphemeNorm::kNormalize,
                                  str.c_str(), &dest))
      << PrintString32WithUnicodes(str);
  EXPECT_EQ(dest, str);
  std::vector<std::string> glyphs;
  EXPECT_TRUE(NormalizeCleanAndSegmentUTF8(UnicodeNormMode::kNFC, OCRNorm::kNone,
                                           GraphemeNormMode::kGlyphSplit, true, str.c_str(),
                                           &glyphs));
  EXPECT_EQ(glyphs.size(), 2);
  EXPECT_EQ(glyphs[1], std::string("\u0dca\u200d\u0dbb"));
  // Can be followed by a dependent vowel.
  str += "\u0dd9"; // E
  EXPECT_TRUE(NormalizeUTF8String(UnicodeNormMode::kNFC, OCRNorm::kNone, GraphemeNorm::kNormalize,
                                  str.c_str(), &dest))
      << PrintString32WithUnicodes(str);
  EXPECT_EQ(dest, str);
}

TEST(ValidateIndicTest, SinhalaYansaya) {
  std::string str = "\u0d9a\u0dca\u200d\u0dba"; // KA Virama ZWJ Yayanna
  std::string dest;
  EXPECT_TRUE(NormalizeUTF8String(UnicodeNormMode::kNFC, OCRNorm::kNone, GraphemeNorm::kNormalize,
                                  str.c_str(), &dest))
      << PrintString32WithUnicodes(str);
  EXPECT_EQ(dest, str);
  // Can be followed by a dependent vowel.
  str += "\u0ddd"; // OO
  EXPECT_TRUE(NormalizeUTF8String(UnicodeNormMode::kNFC, OCRNorm::kNone, GraphemeNorm::kNormalize,
                                  str.c_str(), &dest))
      << PrintString32WithUnicodes(str);
  EXPECT_EQ(dest, str);
  std::vector<std::string> glyphs;
  EXPECT_TRUE(NormalizeCleanAndSegmentUTF8(UnicodeNormMode::kNFC, OCRNorm::kNone,
                                           GraphemeNormMode::kGlyphSplit, true, str.c_str(),
                                           &glyphs));
  EXPECT_EQ(glyphs.size(), 3);
  EXPECT_EQ(glyphs[1], std::string("\u0dca\u200d\u0dba"));
}

TEST(ValidateIndicTest, SinhalaRepaya) {
  std::string str = "\u0d9a\u0dbb\u0dca\u200d\u0db8"; // KA Rayanna Virama ZWJ MA
  std::vector<std::string> glyphs;
  EXPECT_TRUE(NormalizeCleanAndSegmentUTF8(UnicodeNormMode::kNFC, OCRNorm::kNone,
                                           GraphemeNormMode::kCombined, true, str.c_str(),
                                           &glyphs));
  EXPECT_EQ(glyphs.size(), 2);
  EXPECT_EQ(glyphs[1], std::string("\u0dbb\u0dca\u200d\u0db8"));
  EXPECT_TRUE(NormalizeCleanAndSegmentUTF8(UnicodeNormMode::kNFC, OCRNorm::kNone,
                                           GraphemeNormMode::kGlyphSplit, true, str.c_str(),
                                           &glyphs));
  EXPECT_EQ(glyphs.size(), 3);
  EXPECT_EQ(glyphs[1], std::string("\u0dbb\u0dca\u200d"));
}

TEST(ValidateIndicTest, SinhalaSpecials) {
  // Sinhala has some exceptions from the usual rules.
  std::string str = "\u0dc0\u0d9c\u0dca\u200d\u0dbb\u0dca\u200d\u0dbb\u0dca\u200d";
  std::vector<std::string> glyphs;
  EXPECT_TRUE(NormalizeCleanAndSegmentUTF8(UnicodeNormMode::kNFC, OCRNorm::kNone,
                                           GraphemeNormMode::kGlyphSplit, true, str.c_str(),
                                           &glyphs));
  EXPECT_EQ(glyphs.size(), 5) << PrintStringVectorWithUnicodes(glyphs);
  EXPECT_EQ(glyphs[0], std::string("\u0dc0"));
  EXPECT_EQ(glyphs[1], std::string("\u0d9c"));
  EXPECT_EQ(glyphs[2], std::string("\u0dca\u200d\u0dbb"));
  EXPECT_EQ(glyphs[3], std::string("\u0dca\u200d"));
  EXPECT_EQ(glyphs[4], std::string("\u0dbb\u0dca\u200d"));
  str = "\u0dc3\u0dbb\u0dca\u200d\u0dbb\u0dca\u200d\u0dcf";
  EXPECT_TRUE(NormalizeCleanAndSegmentUTF8(UnicodeNormMode::kNFC, OCRNorm::kNone,
                                           GraphemeNormMode::kGlyphSplit, true, str.c_str(),
                                           &glyphs));
  EXPECT_EQ(glyphs.size(), 4) << PrintStringVectorWithUnicodes(glyphs);
  EXPECT_EQ(glyphs[0], std::string("\u0dc3"));
  EXPECT_EQ(glyphs[1], std::string("\u0dbb\u0dca\u200d"));
  EXPECT_EQ(glyphs[2], std::string("\u0dbb\u0dca\u200d"));
  EXPECT_EQ(glyphs[3], std::string("\u0dcf"));
}

} // namespace tesseract
