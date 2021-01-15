#include "validate_khmer.h"
#include "errcode.h"
#include "tprintf.h"

namespace tesseract {

// Returns whether codes matches the pattern for a Khmer Grapheme.
// Taken from unicode standard:
// http://www.unicode.org/versions/Unicode9.0.0/ch16.pdf.
// where it gives: B {R | C} {S {R}}* {{Z} V} {O} {S}, using different notation
// to the ISCII standard http://varamozhi.sourceforge.net/iscii91.pdf.
// Translated to the codes used by the CharClass enum:
// C {R | N} {HC {R}}* {{Z|z} M{P}} {D} {HC}
// Where R is a new symbol (Robat) and N is repurposed as a consonant shifter.
// Also the Consonant class here includes independent vowels, as they are
// treated the same anyway.
// In the split grapheme mode, the only characters that get grouped are the
// HC and the {Z|z}M The unicode chapter on Khmer only mentions the joiners in
// the BNF syntax, so who knows what they do.
bool ValidateKhmer::ConsumeGraphemeIfValid() {
  const unsigned num_codes = codes_.size();
  if (codes_used_ == num_codes) return false;
  if (codes_[codes_used_].first == CharClass::kOther) {
    UseMultiCode(1);
    return true;
  }
  if (codes_[codes_used_].first != CharClass::kConsonant) {
    if (report_errors_) {
      tprintf("Invalid start of Khmer syllable:0x%x\n",
              codes_[codes_used_].second);
    }
    return false;
  }
  if (UseMultiCode(1)) return true;
  if (codes_[codes_used_].first == CharClass::kRobat ||
      codes_[codes_used_].first == CharClass::kNukta) {
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
  unsigned num_matra_parts = 0;
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
  if (codes_[codes_used_].first == CharClass::kMatra ||
      codes_[codes_used_].first == CharClass::kMatraPiece) {
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

Validator::CharClass ValidateKhmer::UnicodeToCharClass(char32 ch) const {
  if (IsVedicAccent(ch)) return CharClass::kVedicMark;
  if (ch == kZeroWidthNonJoiner) return CharClass::kZeroWidthNonJoiner;
  if (ch == kZeroWidthJoiner) return CharClass::kZeroWidthJoiner;
  // Offset from the start of the relevant unicode code block aka code page.
  int off = ch - static_cast<char32>(script_);
  // Anything in another code block is other.
  if (off < 0 || off >= kIndicCodePageSize) return CharClass::kOther;
  if (off <= 0x33) return CharClass::kConsonant;
  if (off <= 0x45) return CharClass::kMatra;
  if (off == 0x46) return CharClass::kMatraPiece;
  if (off == 0x4c) return CharClass::kRobat;
  if (off == 0x49 || off == 0x4a) return CharClass::kNukta;
  if (off <= 0x51) return CharClass::kVowelModifier;
  if (off == 0x52) return CharClass::kVirama;
  return CharClass::kOther;
}

}  // namespace tesseract
