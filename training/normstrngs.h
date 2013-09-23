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
void NormalizeChar32(char32 ch, GenericVector<char32>* str);

// Normalize a UTF8 string. Same as above, but for UTF8-encoded strings, that
// can contain multiple UTF32 code points.
STRING NormalizeUTF8String(const char* str8);

// Apply just the OCR-specific normalizations and return the normalized char.
char32 OCRNormalize(char32 ch);

// Returns true if the OCRNormalized ch1 and ch2 are the same.
bool IsOCREquivalent(char32 ch1, char32 ch2);

}  // namespace tesseract

#endif  // TESSERACT_CCUTIL_NORMSTRNGS_H_
