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

#include "export.h"

#include "validator.h"

#include <string>
#include <vector>

namespace tesseract {

// The standard unicode normalizations.
enum class UnicodeNormMode {
  kNFD,
  kNFC,
  kNFKD,
  kNFKC,
};

// To normalize away differences in punctuation that are ambiguous, like
// curly quotes and different widths of dash.
enum class OCRNorm {
  kNone,
  kNormalize,
};

// To validate and normalize away some subtle differences that can occur in
// Indic scripts, eg ensuring that an explicit virama is always followed by
// a zero-width non-joiner.
enum class GraphemeNorm {
  kNone,
  kNormalize,
};

// Normalizes a UTF8 string according to the given modes. Returns true on
// success. If false is returned, some failure or invalidity was present, and
// the result string is produced on a "best effort" basis.
TESS_UNICHARSET_TRAINING_API
bool NormalizeUTF8String(UnicodeNormMode u_mode, OCRNorm ocr_normalize,
                         GraphemeNorm grapheme_normalize, const char *str8,
                         std::string *normalized);

// Normalizes a UTF8 string according to the given modes and splits into
// graphemes according to g_mode. Returns true on success. If false is returned,
// some failure or invalidity was present, and the result string is produced on
// a "best effort" basis.
TESS_UNICHARSET_TRAINING_API
bool NormalizeCleanAndSegmentUTF8(UnicodeNormMode u_mode, OCRNorm ocr_normalize,
                                  GraphemeNormMode g_mode, bool report_errors, const char *str8,
                                  std::vector<std::string> *graphemes);

// Applies just the OCR-specific normalizations and return the normalized char.
char32 OCRNormalize(char32 ch);

// Returns true if the OCRNormalized ch1 and ch2 are the same.
bool IsOCREquivalent(char32 ch1, char32 ch2);

// Returns true if the value lies in the range of valid unicodes.
bool IsValidCodepoint(const char32 ch);

// Returns true a code point has the White_Space Unicode property.
TESS_UNICHARSET_TRAINING_API
bool IsWhitespace(const char32 ch);

// Returns true if every char in the given (null-terminated) string has the
// White_Space Unicode property.
TESS_UNICHARSET_TRAINING_API
bool IsUTF8Whitespace(const char *text);

// Returns the length of bytes of the prefix of 'text' that have the White_Space
// unicode property.
TESS_UNICHARSET_TRAINING_API
unsigned int SpanUTF8Whitespace(const char *text);

// Returns the length of bytes of the prefix of 'text' that DO NOT have the
// White_Space unicode property.
TESS_UNICHARSET_TRAINING_API
unsigned int SpanUTF8NotWhitespace(const char *text);

// Returns true if the char is interchange valid i.e. no C0 or C1 control codes
// (other than CR LF HT FF) and no non-characters.
TESS_UNICHARSET_TRAINING_API
bool IsInterchangeValid(const char32 ch);

// Same as above but restricted to 7-bit ASCII.
TESS_UNICHARSET_TRAINING_API
bool IsInterchangeValid7BitAscii(const char32 ch);

// Convert a full-width UTF-8 string to half-width.
TESS_UNICHARSET_TRAINING_API
char32 FullwidthToHalfwidth(const char32 ch);

} // namespace tesseract

#endif // TESSERACT_CCUTIL_NORMSTRNGS_H_
