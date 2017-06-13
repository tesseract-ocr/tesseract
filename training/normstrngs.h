/**********************************************************************
 * File:        normstrngs.h
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

#ifndef TESSERACT_CCUTIL_NORMSTRNGS_H_
#define TESSERACT_CCUTIL_NORMSTRNGS_H_

#include "genericvector.h"
#include "strngs.h"

typedef signed int char32;

namespace tesseract {

// UTF-8 to UTF-32 conversion function.
void UTF8ToUTF32(const char* utf8_str, GenericVector<char32>* str32);

// UTF-32 to UTF-8 convesion function.
void UTF32ToUTF8(const GenericVector<char32>& str32, STRING* utf8_str);

// Normalize a single char32 using NFKC + OCR-specific transformations.
// NOTE that proper NFKC may require multiple characters as input. The
// assumption of this function is that the input is already as fully composed
// as it can be, but may require some compatibility normalizations or just
// OCR evaluation related normalizations.
void NormalizeChar32(char32 ch, bool decompose, GenericVector<char32>* str);

// Normalize a UTF8 string. Same as above, but for UTF8-encoded strings, that
// can contain multiple UTF32 code points.
STRING NormalizeUTF8String(bool decompose, const char* str8);
// Default behavior is to compose, until it is proven that decomposed benefits
// at least one language.
inline STRING NormalizeUTF8String(const char* str8) {
  return NormalizeUTF8String(false, str8);
}

// Apply just the OCR-specific normalizations and return the normalized char.
char32 OCRNormalize(char32 ch);

// Returns true if the OCRNormalized ch1 and ch2 are the same.
bool IsOCREquivalent(char32 ch1, char32 ch2);

// Returns true if the value lies in the range of valid unicodes.
bool IsValidCodepoint(const char32 ch);

// Returns true a code point has the White_Space Unicode property.
bool IsWhitespace(const char32 ch);
// Returns true if every char in the given (null-terminated) string has the
// White_Space Unicode property.
bool IsUTF8Whitespace(const char* text);

// Returns the length of bytes of the prefix of 'text' that have the White_Space
// unicode property.
int SpanUTF8Whitespace(const char* text);

// Returns the length of bytes of the prefix of 'text' that DO NOT have the
// White_Space unicode property.
int SpanUTF8NotWhitespace(const char* text);

// Returns true if the char is interchange valid i.e. no C0 or C1 control codes
// (other than CR LF HT FF) and no non-characters.
bool IsInterchangeValid(const char32 ch);
// Same as above but restricted to 7-bit ASCII.
bool IsInterchangeValid7BitAscii(const char32 ch);

// Convert a full-width UTF-8 string to half-width.
char32 FullwidthToHalfwidth(const char32 ch);

}  // namespace tesseract

#endif  // TESSERACT_CCUTIL_NORMSTRNGS_H_
