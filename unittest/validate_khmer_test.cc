#include "tesseract/training/normstrngs.h"

#include "tesseract/unittest/normstrngs_test.h"

namespace tesseract {
namespace {

// Test some random Khmer words.
TEST(ValidateKhmerTest, GoodKhmerWords) {
  string str = "ព័ត៏មានប្លែកៗ";
  ExpectGraphemeModeResults(str, UnicodeNormMode::kNFC, 13, 12, 7, str);
  str = "ទំនុកច្រៀង";
  ExpectGraphemeModeResults(str, UnicodeNormMode::kNFC, 10, 9, 5, str);
  str = "កាលីហ្វូញ៉ា";
  ExpectGraphemeModeResults(str, UnicodeNormMode::kNFC, 11, 10, 4, str);
  str = "ចាប់ពីផ្លូវ";
  ExpectGraphemeModeResults(str, UnicodeNormMode::kNFC, 11, 10, 5, str);
}

// Test some random Khmer words with dotted circles.
TEST(ValidateKhmerTest, BadKhmerWords) {
  string result;
  // Multiple dependent vowels not allowed
  string str = "\u1796\u17b6\u17b7";
  EXPECT_FALSE(NormalizeUTF8String(UnicodeNormMode::kNFC, OCRNorm::kNone,
                                   GraphemeNorm::kNormalize, str.c_str(),
                                   &result));
  // Multiple shifters not allowed
  str = "\u1798\u17c9\u17ca";
  EXPECT_FALSE(NormalizeUTF8String(UnicodeNormMode::kNFC, OCRNorm::kNone,
                                   GraphemeNorm::kNormalize, str.c_str(),
                                   &result));
  // Multiple signs not allowed
  str = "\u1780\u17b6\u17cb\u17cd";
  EXPECT_FALSE(NormalizeUTF8String(UnicodeNormMode::kNFC, OCRNorm::kNone,
                                   GraphemeNorm::kNormalize, str.c_str(),
                                   &result));
}

}  // namespace
}  // namespace tesseract
