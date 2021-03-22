/**********************************************************************
 * File:        normstrngs.cpp
 * Description: Utilities to normalize and manipulate UTF-32 and
 *              UTF-8 strings.
 * Author:      Ranjith Unnikrishnan
 * Created:     Thu July 4 2013
 *
 * (C) Copyright 2013, Google Inc.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 **********************************************************************/

#include "normstrngs.h"

#include <string>
#include <unordered_map>
#include <vector>

#include <tesseract/unichar.h>
#include "errcode.h"
#include "icuerrorcode.h"
#include "unicode/normalizer2.h" // From libicu
#include "unicode/translit.h"    // From libicu
#include "unicode/uchar.h"       // From libicu
#include "unicode/unorm2.h"      // From libicu
#include "unicode/uscript.h"     // From libicu

namespace tesseract {

static bool is_hyphen_punc(const char32 ch) {
  static const int kNumHyphenPuncUnicodes = 13;
  static const char32 kHyphenPuncUnicodes[kNumHyphenPuncUnicodes] = {
      '-',    0x2010, 0x2011, 0x2012, 0x2013, 0x2014, 0x2015, // hyphen..horizontal bar
      0x207b,                                                 // superscript minus
      0x208b,                                                 // subscript minus
      0x2212,                                                 // minus sign
      0xfe58,                                                 // small em dash
      0xfe63,                                                 // small hyphen-minus
      0xff0d,                                                 // fullwidth hyphen-minus
  };
  for (int kHyphenPuncUnicode : kHyphenPuncUnicodes) {
    if (kHyphenPuncUnicode == ch) {
      return true;
    }
  }
  return false;
}

static bool is_single_quote(const char32 ch) {
  static const int kNumSingleQuoteUnicodes = 8;
  static const char32 kSingleQuoteUnicodes[kNumSingleQuoteUnicodes] = {
      '\'', '`',
      0x2018, // left single quotation mark (English, others)
      0x2019, // right single quotation mark (Danish, Finnish, Swedish, Norw.)
              // We may have to introduce a comma set with 0x201a
      0x201B, // single high-reveresed-9 quotation mark (PropList.txt)
      0x2032, // prime
      0x300C, // left corner bracket (East Asian languages)
      0xFF07, // fullwidth apostrophe
  };
  for (int kSingleQuoteUnicode : kSingleQuoteUnicodes) {
    if (kSingleQuoteUnicode == ch) {
      return true;
    }
  }
  return false;
}

static bool is_double_quote(const char32 ch) {
  static const int kNumDoubleQuoteUnicodes = 8;
  static const char32 kDoubleQuoteUnicodes[kNumDoubleQuoteUnicodes] = {
      '"',
      0x201C, // left double quotation mark (English, others)
      0x201D, // right double quotation mark (Danish, Finnish, Swedish, Norw.)
      0x201F, // double high-reversed-9 quotation mark (PropList.txt)
      0x2033, // double prime
      0x301D, // reversed double prime quotation mark (East Asian langs,
              // horiz.)
      0x301E, // close double prime (East Asian languages written horizontally)
      0xFF02, // fullwidth quotation mark
  };
  for (int kDoubleQuoteUnicode : kDoubleQuoteUnicodes) {
    if (kDoubleQuoteUnicode == ch) {
      return true;
    }
  }
  return false;
}

// Helper runs a standard unicode normalization, optional OCR normalization,
// and leaves the result as char32 for subsequent processing.
static void NormalizeUTF8ToUTF32(UnicodeNormMode u_mode, OCRNorm ocr_normalize, const char *str8,
                                 std::vector<char32> *normed32) {
  // Convert to ICU string for unicode normalization.
  icu::UnicodeString uch_str(str8, "UTF-8");
  IcuErrorCode error_code;
  // Convert the enum to the new weird icu representation.
  const char *norm_type =
      u_mode == UnicodeNormMode::kNFKD || u_mode == UnicodeNormMode::kNFKC ? "nfkc" : "nfc";
  UNormalization2Mode compose = u_mode == UnicodeNormMode::kNFC || u_mode == UnicodeNormMode::kNFKC
                                    ? UNORM2_COMPOSE
                                    : UNORM2_DECOMPOSE;
  // Pointer to singleton does not require deletion.
  const icu::Normalizer2 *normalizer =
      icu::Normalizer2::getInstance(nullptr, norm_type, compose, error_code);
  error_code.assertSuccess();
  error_code.reset();
  icu::UnicodeString norm_str = normalizer->normalize(uch_str, error_code);
  error_code.assertSuccess();
  // Convert to char32 for output. OCR normalization if required.
  normed32->reserve(norm_str.length()); // An approximation.
  for (int offset = 0; offset < norm_str.length(); offset = norm_str.moveIndex32(offset, 1)) {
    char32 ch = norm_str.char32At(offset);
    // Skip all ZWS, RTL and LTR marks.
    if (Validator::IsZeroWidthMark(ch)) {
      continue;
    }
    if (ocr_normalize == OCRNorm::kNormalize) {
      ch = OCRNormalize(ch);
    }
    normed32->push_back(ch);
  }
}

// Helper removes joiners from strings that contain no letters.
static void StripJoiners(std::vector<char32> *str32) {
  for (char32 ch : *str32) {
    if (u_isalpha(ch)) {
      return;
    }
  }
  int len = 0;
  for (char32 ch : *str32) {
    if (ch != Validator::kZeroWidthJoiner && ch != Validator::kZeroWidthNonJoiner) {
      (*str32)[len++] = ch;
    }
  }
  str32->resize(len);
}

// Normalizes a UTF8 string according to the given modes. Returns true on
// success. If false is returned, some failure or invalidity was present, and
// the result string is produced on a "best effort" basis.
bool NormalizeUTF8String(UnicodeNormMode u_mode, OCRNorm ocr_normalize,
                         GraphemeNorm grapheme_normalize, const char *str8,
                         std::string *normalized) {
  std::vector<char32> normed32;
  NormalizeUTF8ToUTF32(u_mode, ocr_normalize, str8, &normed32);
  if (grapheme_normalize == GraphemeNorm::kNormalize) {
    StripJoiners(&normed32);
    std::vector<std::vector<char32>> graphemes;
    bool success = Validator::ValidateCleanAndSegment(GraphemeNormMode::kSingleString, false,
                                                      normed32, &graphemes);
    if (graphemes.empty() || graphemes[0].empty()) {
      success = false;
    } else if (normalized != nullptr) {
      *normalized = UNICHAR::UTF32ToUTF8(graphemes[0]);
    }
    return success;
  }
  if (normalized != nullptr) {
    *normalized = UNICHAR::UTF32ToUTF8(normed32);
  }
  return true;
}

// Normalizes a UTF8 string according to the given modes and splits into
// graphemes according to g_mode. Returns true on success. If false is returned,
// some failure or invalidity was present, and the result string is produced on
// a "best effort" basis.
bool NormalizeCleanAndSegmentUTF8(UnicodeNormMode u_mode, OCRNorm ocr_normalize,
                                  GraphemeNormMode g_mode, bool report_errors, const char *str8,
                                  std::vector<std::string> *graphemes) {
  std::vector<char32> normed32;
  NormalizeUTF8ToUTF32(u_mode, ocr_normalize, str8, &normed32);
  StripJoiners(&normed32);
  std::vector<std::vector<char32>> graphemes32;
  bool success = Validator::ValidateCleanAndSegment(g_mode, report_errors, normed32, &graphemes32);
  if (g_mode != GraphemeNormMode::kSingleString && success) {
    // If we modified the string to clean it up, the segmentation may not be
    // correct, so check for changes and do it again.
    std::vector<char32> cleaned32;
    for (const auto &g : graphemes32) {
      cleaned32.insert(cleaned32.end(), g.begin(), g.end());
    }
    if (cleaned32 != normed32) {
      graphemes32.clear();
      success = Validator::ValidateCleanAndSegment(g_mode, report_errors, cleaned32, &graphemes32);
    }
  }
  graphemes->clear();
  graphemes->reserve(graphemes32.size());
  for (const auto &grapheme : graphemes32) {
    graphemes->push_back(UNICHAR::UTF32ToUTF8(grapheme));
  }
  return success;
}

// Apply just the OCR-specific normalizations and return the normalized char.
char32 OCRNormalize(char32 ch) {
  if (is_hyphen_punc(ch)) {
    return '-';
  } else if (is_single_quote(ch)) {
    return '\'';
  } else if (is_double_quote(ch)) {
    return '"';
  }
  return ch;
}

bool IsOCREquivalent(char32 ch1, char32 ch2) {
  return OCRNormalize(ch1) == OCRNormalize(ch2);
}

bool IsValidCodepoint(const char32 ch) {
  // In the range [0, 0xD800) or [0xE000, 0x10FFFF]
  return (static_cast<uint32_t>(ch) < 0xD800) || (ch >= 0xE000 && ch <= 0x10FFFF);
}

bool IsWhitespace(const char32 ch) {
  ASSERT_HOST_MSG(IsValidCodepoint(ch), "Invalid Unicode codepoint: 0x%x\n", ch);
  return u_isUWhiteSpace(static_cast<UChar32>(ch));
}

bool IsUTF8Whitespace(const char *text) {
  return SpanUTF8Whitespace(text) == strlen(text);
}

unsigned int SpanUTF8Whitespace(const char *text) {
  int n_white = 0;
  for (UNICHAR::const_iterator it = UNICHAR::begin(text, strlen(text));
       it != UNICHAR::end(text, strlen(text)); ++it) {
    if (!IsWhitespace(*it)) {
      break;
    }
    n_white += it.utf8_len();
  }
  return n_white;
}

unsigned int SpanUTF8NotWhitespace(const char *text) {
  int n_notwhite = 0;
  for (UNICHAR::const_iterator it = UNICHAR::begin(text, strlen(text));
       it != UNICHAR::end(text, strlen(text)); ++it) {
    if (IsWhitespace(*it)) {
      break;
    }
    n_notwhite += it.utf8_len();
  }
  return n_notwhite;
}

bool IsInterchangeValid(const char32 ch) {
  return IsValidCodepoint(ch) && !(ch >= 0xFDD0 && ch <= 0xFDEF) && // Noncharacters.
         !(ch >= 0xFFFE && ch <= 0xFFFF) && !(ch >= 0x1FFFE && ch <= 0x1FFFF) &&
         !(ch >= 0x2FFFE && ch <= 0x2FFFF) && !(ch >= 0x3FFFE && ch <= 0x3FFFF) &&
         !(ch >= 0x4FFFE && ch <= 0x4FFFF) && !(ch >= 0x5FFFE && ch <= 0x5FFFF) &&
         !(ch >= 0x6FFFE && ch <= 0x6FFFF) && !(ch >= 0x7FFFE && ch <= 0x7FFFF) &&
         !(ch >= 0x8FFFE && ch <= 0x8FFFF) && !(ch >= 0x9FFFE && ch <= 0x9FFFF) &&
         !(ch >= 0xAFFFE && ch <= 0xAFFFF) && !(ch >= 0xBFFFE && ch <= 0xBFFFF) &&
         !(ch >= 0xCFFFE && ch <= 0xCFFFF) && !(ch >= 0xDFFFE && ch <= 0xDFFFF) &&
         !(ch >= 0xEFFFE && ch <= 0xEFFFF) && !(ch >= 0xFFFFE && ch <= 0xFFFFF) &&
         !(ch >= 0x10FFFE && ch <= 0x10FFFF) &&
         (!u_isISOControl(static_cast<UChar32>(ch)) || ch == '\n' || ch == '\f' || ch == '\t' ||
          ch == '\r');
}

bool IsInterchangeValid7BitAscii(const char32 ch) {
  return IsValidCodepoint(ch) && ch <= 128 &&
         (!u_isISOControl(static_cast<UChar32>(ch)) || ch == '\n' || ch == '\f' || ch == '\t' ||
          ch == '\r');
}

char32 FullwidthToHalfwidth(const char32 ch) {
  // Return unchanged if not in the fullwidth-halfwidth Unicode block.
  if (ch < 0xFF00 || ch > 0xFFEF || !IsValidCodepoint(ch)) {
    if (ch != 0x3000) {
      return ch;
    }
  }
  // Special case for fullwidth left and right "white parentheses".
  if (ch == 0xFF5F) {
    return 0x2985;
  }
  if (ch == 0xFF60) {
    return 0x2986;
  }
  // Construct a full-to-half width transliterator.
  IcuErrorCode error_code;
  icu::UnicodeString uch_str(static_cast<UChar32>(ch));
  const icu::Transliterator *fulltohalf =
      icu::Transliterator::createInstance("Fullwidth-Halfwidth", UTRANS_FORWARD, error_code);
  error_code.assertSuccess();
  error_code.reset();

  fulltohalf->transliterate(uch_str);
  delete fulltohalf;
  ASSERT_HOST(uch_str.length() != 0);
  return uch_str[0];
}

} // namespace tesseract
