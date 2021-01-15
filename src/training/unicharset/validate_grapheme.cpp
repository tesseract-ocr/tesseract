#include "validate_grapheme.h"
#include "tprintf.h"
#include "unicode/uchar.h"  // From libicu

namespace tesseract {

bool ValidateGrapheme::ConsumeGraphemeIfValid() {
  const unsigned num_codes = codes_.size();
  char32 prev_prev_ch = ' ';
  char32 prev_ch = ' ';
  CharClass prev_cc = CharClass::kWhitespace;
  int num_codes_in_grapheme = 0;
  while (codes_used_ < num_codes) {
    CharClass cc = codes_[codes_used_].first;
    char32 ch = codes_[codes_used_].second;
    const bool is_combiner =
        cc == CharClass::kCombiner || cc == CharClass::kVirama;
  // TODO: Make this code work well with RTL text.
  // See https://github.com/tesseract-ocr/tesseract/pull/2266#issuecomment-467114751
  #if 0
    // Reject easily detected badly formed sequences.
    if (prev_cc == CharClass::kWhitespace && is_combiner) {
      if (report_errors_) tprintf("Word started with a combiner:0x%x\n", ch);
     return false;
    }
  #endif
    if (prev_cc == CharClass::kVirama && cc == CharClass::kVirama) {
      if (report_errors_)
        tprintf("Two grapheme links in a row:0x%x 0x%x\n", prev_ch, ch);
      return false;
    }
    if (prev_cc != CharClass::kWhitespace && cc != CharClass::kWhitespace &&
        IsBadlyFormed(prev_ch, ch)) {
      return false;
    }
    bool prev_is_fwd_combiner =
        prev_ch == kZeroWidthJoiner || prev_cc == CharClass::kVirama ||
        (prev_ch == kZeroWidthNonJoiner &&
         (cc == CharClass::kVirama || prev_prev_ch == kZeroWidthJoiner));
    if (num_codes_in_grapheme > 0 && !is_combiner && !prev_is_fwd_combiner)
      break;
    CodeOnlyToOutput();
    ++num_codes_in_grapheme;
    prev_prev_ch = prev_ch;
    prev_ch = ch;
    prev_cc = cc;
  }
  if (num_codes_in_grapheme > 0) MultiCodePart(num_codes_in_grapheme);
  return true;
}

Validator::CharClass ValidateGrapheme::UnicodeToCharClass(char32 ch) const {
  if (IsVedicAccent(ch)) return CharClass::kVedicMark;
  // The ZeroWidth[Non]Joiner characters are mapped to kCombiner as they
  // always combine with the previous character.
  if (u_hasBinaryProperty(ch, UCHAR_GRAPHEME_LINK)) return CharClass::kVirama;
  if (u_isUWhiteSpace(ch)) return CharClass::kWhitespace;
  // Workaround for Javanese Aksara's Taling, do not label it as a combiner
  if (ch == 0xa9ba) return CharClass::kConsonant;
  int char_type = u_charType(ch);
  if (char_type == U_NON_SPACING_MARK || char_type == U_ENCLOSING_MARK ||
      char_type == U_COMBINING_SPACING_MARK || ch == kZeroWidthNonJoiner ||
      ch == kZeroWidthJoiner)
    return CharClass::kCombiner;
  return CharClass::kOther;
}

// Helper returns true if the sequence prev_ch,ch is invalid.
bool ValidateGrapheme::IsBadlyFormed(char32 prev_ch, char32 ch) {
  // Reject badly formed Indic vowels.
  if (IsBadlyFormedIndicVowel(prev_ch, ch)) {
    if (report_errors_)
      tprintf("Badly formed Indic vowel sequence:0x%x 0x%x\n", prev_ch, ch);
    return true;
  }
  if (IsBadlyFormedThai(prev_ch, ch)) {
    if (report_errors_) tprintf("Badly formed Thai:0x%x 0x%x\n", prev_ch, ch);
    return true;
  }
  return false;
}

// Helper returns true if the sequence prev_ch,ch is an invalid Indic vowel.
// Some vowels in Indic scripts may be analytically decomposed into atomic pairs
// of components that are themselves valid unicode symbols. (See Table 12-1 in
// http://www.unicode.org/versions/Unicode9.0.0/ch12.pdf
// for examples in Devanagari). The Unicode standard discourages specifying
// vowels this way, but they are sometimes encountered in text, probably because
// some editors still permit it. Renderers however dislike such pairs, and so
// this function may be used to detect their occurrence for removal.
// TODO(rays) This function only covers a subset of Indic languages and doesn't
// include all rules. Add rules as appropriate to support other languages or
// find a way to generalize these existing rules that makes use of the
// regularity of the mapping from ISCII to Unicode.
/* static */
bool ValidateGrapheme::IsBadlyFormedIndicVowel(char32 prev_ch, char32 ch) {
  return ((prev_ch == 0x905 && (ch == 0x946 || ch == 0x93E)) ||
          (prev_ch == 0x909 && ch == 0x941) ||
          (prev_ch == 0x90F && (ch >= 0x945 && ch <= 0x947)) ||
          (prev_ch == 0x905 && (ch >= 0x949 && ch <= 0x94C)) ||
          (prev_ch == 0x906 && (ch >= 0x949 && ch <= 0x94C)) ||
          // Illegal combinations of two dependent Devanagari vowels.
          (prev_ch == 0x93E && (ch >= 0x945 && ch <= 0x948)) ||
          // Dependent Devanagari vowels following a virama.
          (prev_ch == 0x94D && (ch >= 0x93E && ch <= 0x94C)) ||
          // Bengali vowels (Table 9-5, pg 313)
          (prev_ch == 0x985 && ch == 0x9BE) ||
          // Telugu vowels (Table 9-19, pg 331)
          (prev_ch == 0xC12 && (ch == 0xC55 || ch == 0xC4C)) ||
          // Kannada vowels (Table 9-20, pg 332)
          (prev_ch == 0xC92 && ch == 0xCCC));
}

// Helper returns true if ch is a Thai consonant.
static bool IsThaiConsonant(char32 ch) { return 0xe01 <= ch && ch <= 0xe2e; }

// Helper returns true is ch is a before-consonant vowel.
static bool IsThaiBeforeConsonantVowel(char32 ch) {
  return 0xe40 <= ch && ch <= 0xe44;
}

// Helper returns true if ch is a Thai tone mark.
static bool IsThaiToneMark(char32 ch) { return 0xe48 <= ch && ch <= 0xe4b; }

// Helper returns true if ch is a Thai vowel that may be followed by a tone
// mark.
static bool IsThaiTonableVowel(char32 ch) {
  return (0xe34 <= ch && ch <= 0xe39) || ch == 0xe31;
}

// Helper returns true if the sequence prev_ch,ch is invalid Thai.
// These rules come from a native Thai speaker, and are not covered by the
// Thai section in the unicode book:
// http://www.unicode.org/versions/Unicode9.0.0/ch16.pdf
// Comments below added by Ray interpreting the code ranges.
/* static */
bool ValidateGrapheme::IsBadlyFormedThai(char32 prev_ch, char32 ch) {
  // Tone marks must follow consonants or specific vowels.
  if (IsThaiToneMark(ch) &&
      !(IsThaiConsonant(prev_ch) || IsThaiTonableVowel(prev_ch))) {
    return true;
  }
  // Tonable vowels must follow consonants.
  if ((IsThaiTonableVowel(ch) || ch == 0xe47) && !IsThaiConsonant(prev_ch)) {
    return true;
  }
  // Thanthakhat must follow consonant or specific vowels.
  if (ch == 0xe4c &&
      !(IsThaiConsonant(prev_ch) || prev_ch == 0xe38 || prev_ch == 0xe34)) {
    return true;
  }
  // Nikkhahit must follow a consonant ?or certain markers?.
  // TODO(rays) confirm this, but there were so many in the ground truth of the
  // validation set that it seems reasonable to assume it is valid.
  if (ch == 0xe4d &&
      !(IsThaiConsonant(prev_ch) || prev_ch == 0xe48 || prev_ch == 0xe49)) {
    return true;
  }
  // The vowels e30, e32, e33 can be used more liberally.
  if ((ch == 0xe30 || ch == 0xe32 || ch == 0xe33) &&
      !(IsThaiConsonant(prev_ch) || IsThaiToneMark(prev_ch)) &&
      !(prev_ch == 0xe32 && ch == 0xe30) &&
      !(prev_ch == 0xe4d && ch == 0xe32)) {
    return true;
  }
  // Some vowels come before consonants, and therefore cannot follow things
  // that cannot end a syllable.
  if (IsThaiBeforeConsonantVowel(ch) &&
      (IsThaiBeforeConsonantVowel(prev_ch) || prev_ch == 0xe31 ||
       prev_ch == 0xe37)) {
    return true;
  }
  // Don't allow the standalone vowel U+0e24 to be followed by other vowels.
  if ((0xe30 <= ch && ch <= 0xe4D) && prev_ch == 0xe24) {
    return true;
  }
  return false;
}

}  // namespace tesseract
