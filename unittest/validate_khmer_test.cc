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
namespace {

// Test some random Khmer words.
TEST(ValidateKhmerTest, GoodKhmerWords) {
  std::string str = "ព័ត៏មានប្លែកៗ";
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
  std::string result;
  // Multiple dependent vowels not allowed
  std::string str = "\u1796\u17b6\u17b7";
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
