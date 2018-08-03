/**********************************************************************
    * File:        validate_javanese.cpp
 * Description: Text validator for Javanese Script - aksara jawa.
 * Author:      Shree Devi Kumar
 * Created:     August 03, 2018
 *
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
 
 #include "validate_javanese.h"
#include "errcode.h"
#include "tprintf.h"

namespace tesseract {

// Returns whether codes matches the pattern for a Javanese Grapheme.
// Taken from unicode standard:
// http://www.unicode.org/charts/PDF/UA980.pdf
// http://www.unicode.org/versions/Unicode11.0.0/ch17.pdf
// Also the Consonant class here includes independent vowels, as they are
// treated the same anyway.

bool ValidateJavanese::ConsumeGraphemeIfValid() {
  int num_codes = codes_.size();
  if (codes_used_ == num_codes) return false;
  if (codes_[codes_used_].first == CharClass::kOther) {
    UseMultiCode(1);
    return true;
  }
  if (codes_[codes_used_].first != CharClass::kConsonant) {
    if (report_errors_) {
      tprintf("Invalid start of Javanese syllable:0x%x\n",
              codes_[codes_used_].second);
    }
    return false;
  }
  if (UseMultiCode(1)) return true;
  if (      codes_[codes_used_].first == CharClass::kNukta) {
    if (UseMultiCode(1)) return true;
  }
  while (codes_used_ + 1 < num_codes &&
         codes_[codes_used_].first == CharClass::kVirama &&
         codes_[codes_used_ + 1].first == CharClass::kConsonant) {
    ASSERT_HOST(!CodeOnlyToOutput());
    if (UseMultiCode(2)) return true;
    if (codes_[codes_used_].first == CharClass::kRobat) {
      if (UseMultiCode(1)) return true;
    }
  }
  int num_matra_parts = 0;
  if (codes_[codes_used_].second == kZeroWidthJoiner ||
      codes_[codes_used_].second == kZeroWidthNonJoiner) {
    if (CodeOnlyToOutput()) {
      if (report_errors_) {
        tprintf("Unterminated joiner: 0x%x\n", output_.back());
      }
      return false;
    }
    ++num_matra_parts;
  }
  // Not quite as shown by the BNF, the matra piece is allowed as a matra on its
  // own or as an addition to other matras.
  if (codes_[codes_used_].first == CharClass::kMatra) {
    ++num_matra_parts;
    if (UseMultiCode(num_matra_parts)) return true;
  } else if (num_matra_parts) {
    if (report_errors_) {
      tprintf("Joiner with non-dependent vowel after it!:0x%x 0x%x\n",
              output_.back(), codes_[codes_used_].second);
    }
    return false;
  }
  if (codes_[codes_used_].first == CharClass::kMatraPiece &&
      codes_[codes_used_ - 1].first != CharClass::kMatraPiece) {
    if (UseMultiCode(1)) return true;
  }
  if (codes_[codes_used_].first == CharClass::kVowelModifier) {
    if (UseMultiCode(1)) return true;
  }
  if (codes_used_ + 1 < num_codes &&
      codes_[codes_used_].first == CharClass::kVirama &&
      codes_[codes_used_ + 1].first == CharClass::kConsonant) {
    ASSERT_HOST(!CodeOnlyToOutput());
    if (UseMultiCode(2)) return true;
  }
  return true;
}

Validator::CharClass ValidateJavanese::UnicodeToCharClass(char32 ch) const {
  if (IsVedicAccent(ch)) return CharClass::kVedicMark;
  if (ch == kZeroWidthNonJoiner) return CharClass::kZeroWidthNonJoiner;
  if (ch == kZeroWidthJoiner) return CharClass::kZeroWidthJoiner;
  // Offset from the start of the relevant unicode code block aka code page.
  int off = ch - static_cast<char32>(script_);
  // Anything in another code block is other.
  if (off < 0 || off >= kIndicCodePageSize) return CharClass::kOther;
  if (off < 0x4) return CharClass::kVowelModifier;
  if (off <= 0x32) return CharClass::kConsonant; // includes independent vowels
  if (off == 0x33) return CharClass::kNukta; // A9B3 CECAK TELU
  if (off == 0x34) return CharClass::kVowelModifier; // A9B4 TARUNG
  if (off <= 0x3d) return CharClass::kMatra;
  if (off <= 0x3f) return CharClass::kVowelModifier; // A9BE-A9BF PENGKAL-CAKRA
  if (off == 0x40) return CharClass::kVirama; // A9C0 PANGKON
  return CharClass::kOther;
}

}  // namespace tesseract
