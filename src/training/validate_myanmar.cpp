#include "validate_myanmar.h"
#include "errcode.h"
#include "icuerrorcode.h"
#include "tprintf.h"
#include "unicode/uchar.h"    // From libicu
#include "unicode/uscript.h"  // From libicu

namespace tesseract {

// Returns whether codes matches the pattern for a Myanmar Grapheme.
// Taken directly from the unicode table 16-3.
// See http://www.unicode.org/versions/Unicode9.0.0/ch16.pdf
bool ValidateMyanmar::ConsumeGraphemeIfValid() {
  int num_codes = codes_.size();
  if (codes_used_ == num_codes) return true;
  // Other.
  if (IsMyanmarOther(codes_[codes_used_].second)) {
    UseMultiCode(1);
    return true;
  }
  // Kinzi.
  if (codes_used_ + 2 < num_codes && codes_[codes_used_].second == 0x1004 &&
      codes_[codes_used_ + 1].second == kMyanmarAsat &&
      codes_[codes_used_ + 2].second == kMyanmarVirama) {
    ASSERT_HOST(!CodeOnlyToOutput());
    ASSERT_HOST(!CodeOnlyToOutput());
    if (UseMultiCode(3)) return true;
  }
  // Base consonant/vowel. NOTE that since everything in Myanmar appears to be
  // optional, except the base, this is the only place where invalid input can
  // be detected and false returned.
  if (IsMyanmarLetter(codes_[codes_used_].second)) {
    if (UseMultiCode(1)) return true;
  } else {
    if (report_errors_) {
      tprintf("Invalid start of Myanmar syllable:0x%x\n",
              codes_[codes_used_].second);
    }
    return false;  // One of these is required.
  }
  if (ConsumeSubscriptIfPresent()) return true;
  ConsumeOptionalSignsIfPresent();
  // What we have consumed so far is a valid syllable.
  return true;
}

// TODO(rays) Doesn't use intermediate coding like the other scripts, as there
// is little correspondence between the content of table 16-3 and the char
// classes of the Indic languages. (Experts may disagree and improve!)
// In unicode table 16-3 there is basically a long list of optional characters,
// which can be coded quite easily.
// Unfortunately, table 16-3 doesn't include even half the Myanmar unicodes!!
// The table also allows sequences that still result in dotted circles!!
// So with a lot of guesswork the rest have been added in a reasonable place.
Validator::CharClass ValidateMyanmar::UnicodeToCharClass(char32 ch) const {
  if (IsMyanmarLetter(ch)) return CharClass::kConsonant;
  return CharClass::kOther;
}

// Helper consumes/copies a virama and any subscript consonant.
// Returns true if the end of input is reached.
bool ValidateMyanmar::ConsumeSubscriptIfPresent() {
  // Subscript consonant. It appears there can be only one.
  int num_codes = codes_.size();
  if (codes_used_ + 1 < num_codes &&
      codes_[codes_used_].second == kMyanmarVirama) {
    if (IsMyanmarLetter(codes_[codes_used_ + 1].second)) {
      ASSERT_HOST(!CodeOnlyToOutput());
      if (UseMultiCode(2)) return true;
    }
  }
  return false;
}

// Helper consumes/copies a series of optional signs.
// Returns true if the end of input is reached.
bool ValidateMyanmar::ConsumeOptionalSignsIfPresent() {
  // The following characters are allowed, all optional, and in sequence.
  // An exception is kMyanmarMedialYa, which can include kMyanmarAsat.
  const std::vector<char32> kMedials({kMyanmarAsat, kMyanmarMedialYa, 0x103c,
                                      0x103d, 0x103e, 0x105e, 0x105f, 0x1060,
                                      0x1081, 0x1031});
  for (char32 ch : kMedials) {
    if (codes_[codes_used_].second == ch) {
      if (UseMultiCode(1)) return true;
      if (ch == kMyanmarMedialYa &&
          codes_[codes_used_].second == kMyanmarAsat) {
        if (UseMultiCode(1)) return true;
      }
    }
  }
  // Vowel sign i, ii, ai.
  char32 ch = codes_[codes_used_].second;
  if (ch == 0x102d || ch == 0x102e || ch == 0x1032) {
    if (UseMultiCode(1)) return true;
  }
  // Vowel sign u, uu, and extensions.
  ch = codes_[codes_used_].second;
  if (ch == 0x102f || ch == 0x1030 || (0x1056 <= ch && ch <= 0x1059) ||
      ch == 0x1062 || ch == 0x1067 || ch == 0x1068 ||
      (0x1071 <= ch && ch <= 0x1074) || (0x1083 <= ch && ch <= 0x1086) ||
      ch == 0x109c || ch == 0x109d) {
    if (UseMultiCode(1)) return true;
  }
  // Tall aa, aa with optional asat.
  if (codes_[codes_used_].second == 0x102b ||
      codes_[codes_used_].second == 0x102c) {
    if (UseMultiCode(1)) return true;
    if (codes_[codes_used_].second == kMyanmarAsat) {
      if (UseMultiCode(1)) return true;
    }
  }
  // The following characters are allowed, all optional, and in sequence.
  const std::vector<char32> kSigns({0x1036, 0x1037});
  for (char32 ch : kSigns) {
    if (codes_[codes_used_].second == ch) {
      if (UseMultiCode(1)) return true;
    }
  }
  // Tone mark extensions.
  ch = codes_[codes_used_].second;
  if (ch == 0x1038 || ch == kMyanmarAsat || ch == 0x1063 || ch == 0x1064 ||
      (0x1069 <= ch && ch <= 0x106d) || (0x1087 <= ch && ch <= 0x108d) ||
      ch == 0x108f || ch == 0x109a || ch == 0x109b ||
      (0xaa7b <= ch && ch <= 0xaa7d)) {
    if (UseMultiCode(1)) return true;
  }
  return false;
}

// Returns true if the unicode is a Myanmar "letter" including consonants
// and independent vowels. Although table 16-3 distinguishes between some
// base consonants and vowels, the extensions make no such distinction, so we
// put them all into a single bucket.
/* static */
bool ValidateMyanmar::IsMyanmarLetter(char32 ch) {
  return (0x1000 <= ch && ch <= 0x102a) || ch == 0x103f ||
         (0x1050 <= ch && ch <= 0x1055) || (0x105a <= ch && ch <= 0x105d) ||
         ch == 0x1061 || ch == 0x1065 || ch == 0x1066 ||
         (0x106e <= ch && ch <= 0x1070) || (0x1075 <= ch && ch <= 0x1080) ||
         ch == 0x108e || (0xa9e0 <= ch && ch <= 0xa9ef) ||
         (0xa9fa <= ch && ch <= 0xa9ff) || (0xaa60 <= ch && ch <= 0xaa73) ||
         ch == 0xaa7a || ch == 0xaa7e || ch == 0xaa7f;
}

// Returns true if ch is a Myanmar digit or other symbol that does not take
// part in being a syllable.
/* static */
bool ValidateMyanmar::IsMyanmarOther(char32 ch) {
  IcuErrorCode err;
  UScriptCode script_code = uscript_getScript(ch, err);
  if (script_code != USCRIPT_MYANMAR && ch != Validator::kZeroWidthJoiner &&
      ch != Validator::kZeroWidthNonJoiner)
    return true;
  return (0x1040 <= ch && ch <= 0x1049) || (0x1090 <= ch && ch <= 0x1099) ||
         (0x109c <= ch && ch <= 0x109d) || (0xa9f0 <= ch && ch <= 0xa9f9) ||
         (0xaa74 <= ch && ch <= 0xaa79);
}

}  // namespace tesseract
