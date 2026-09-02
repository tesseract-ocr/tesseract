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

#include <tesseract/unichar.h>
#include "gmock/gmock.h" // for testing::ElementsAreArray
#include "include_gunit.h"

namespace tesseract {

TEST(UnicharTest, Conversion) {
  // This test verifies that Unichar::UTF8ToUTF32 and Unichar::UTF32ToUTF8
  // show the required conversion properties.
  // Test for round-trip utf8-32-8 for 1, 2, 3 and 4 byte codes.
  const char *kUTF8Src = "a\u05d0\u0ca4\U0002a714";
  const std::vector<char32> kUTF32Src = {'a', 0x5d0, 0xca4, 0x2a714};
  // Check for round-trip conversion.
  std::vector<char32> utf32 = UNICHAR::UTF8ToUTF32(kUTF8Src);
  EXPECT_THAT(utf32, testing::ElementsAreArray(kUTF32Src));
  std::string utf8 = UNICHAR::UTF32ToUTF8(utf32);
  EXPECT_STREQ(kUTF8Src, utf8.c_str());
}

TEST(UnicharTest, InvalidText) {
  // This test verifies that Unichar correctly deals with invalid text.
  const char *kInvalidUTF8 = "a b\200d string";
  const std::vector<char32> kInvalidUTF32 = {'a', ' ', 0x200000, 'x'};
  // Invalid utf8 produces an empty vector.
  std::vector<char32> utf32 = UNICHAR::UTF8ToUTF32(kInvalidUTF8);
  EXPECT_TRUE(utf32.empty());
  // Invalid utf32 produces an empty string.
  std::string utf8 = UNICHAR::UTF32ToUTF8(kInvalidUTF32);
  EXPECT_TRUE(utf8.empty());
}

TEST(UnicharTest, TruncatedUtf8) {
  // This test verifies that UTF8ToUTF32 does not read past the end of a
  // string that ends with a truncated multibyte prefix (issue #4495).
  // A truncated multibyte prefix is invalid UTF-8, so the conversion
  // must return an empty vector instead of reading past the NUL.
  // Keep the explicit NULs to make the truncation boundary visible in each
  // fixture; without them, the literal terminator is implicit.
  const char *kTruncated2 = "\xC2\0";
  const char *kTruncated3 = "\xE8\0";
  const char *kTruncated4 = "\xF0\0";
  const char *kTruncatedMid = "ab\xE8\0";
  const char *kIllegalLeading = "\x80\0";
  EXPECT_TRUE(UNICHAR::UTF8ToUTF32(kTruncated2).empty());
  EXPECT_TRUE(UNICHAR::UTF8ToUTF32(kTruncated3).empty());
  EXPECT_TRUE(UNICHAR::UTF8ToUTF32(kTruncated4).empty());
  EXPECT_TRUE(UNICHAR::UTF8ToUTF32(kTruncatedMid).empty());
  EXPECT_TRUE(UNICHAR::UTF8ToUTF32(kIllegalLeading).empty());
}

} // namespace tesseract
