#include "validate_indic.h"
#include "errcode.h"
#include "tprintf.h"

namespace tesseract {

// Returns whether codes matches the pattern for an Indic Grapheme.
// The ISCII standard http://varamozhi.sourceforge.net/iscii91.pdf
// has a BNF for valid syllables (Graphemes) which is modified slightly
// for Unicode.  Notably U+200C and U+200D are used before/after the
// virama/virama to express explicit or soft viramas.
// Also the unicode v.9 Malayalam entry states that CZHC can be used in several
// Indic languages to request traditional ligatures, and CzHC is Malayalam-
// specific for requesting open conjuncts.
//
//  + vowel Grapheme:  V[D](v)*
//  + consonant Grapheme: (C[N](H|HZ|Hz|ZH)?)*C[N](H|Hz)?[M[P]][D](v)*
bool ValidateIndic::ConsumeGraphemeIfValid() {
  switch (codes_[codes_used_].first) {
    case CharClass::kConsonant:
      return ConsumeConsonantHeadIfValid() && ConsumeConsonantTailIfValid();
    case CharClass::kVowel:
    case CharClass::kVedicMark:
      return ConsumeVowelIfValid();
    case CharClass::kZeroWidthJoiner:
    case CharClass::kZeroWidthNonJoiner:
      // Apart from within an aksara, joiners are silently dropped.
      if (report_errors_)
        tprintf("Dropping isolated joiner: 0x%x\n", codes_[codes_used_].second);
      ++codes_used_;
      return true;
    case CharClass::kOther:
      UseMultiCode(1);
      return true;
    default:
      if (report_errors_) {
        tprintf("Invalid start of grapheme sequence:%c=0x%x\n",
                codes_[codes_used_].first, codes_[codes_used_].second);
      }
      return false;
  }
}

Validator::CharClass ValidateIndic::UnicodeToCharClass(char32 ch) const {
  if (IsVedicAccent(ch)) return CharClass::kVedicMark;
  if (ch == kZeroWidthNonJoiner) return CharClass::kZeroWidthNonJoiner;
  if (ch == kZeroWidthJoiner) return CharClass::kZeroWidthJoiner;
  // Offset from the start of the relevant unicode code block aka code page.
  int base = static_cast<char32>(script_);
  int off = ch - base;
  // Anything in another code block is other.
  if (off < 0 || off >= kIndicCodePageSize) return CharClass::kOther;
  // Exception for Tamil. The aytham character is considered a letter.
  if (script_ == ViramaScript::kTamil && off == 0x03) return CharClass::kVowel;
  if (off < 0x4) return CharClass::kVowelModifier;
  if (script_ == ViramaScript::kSinhala) {
    // Sinhala is an exception.
    if (off <= 0x19) return CharClass::kVowel;
    if (off <= 0x49) return CharClass::kConsonant;
    if (off == 0x4a) return CharClass::kVirama;
    if (off <= 0x5f) return CharClass::kMatra;
  } else {
    if (off <= 0x14 || off == 0x50) return CharClass::kVowel;
    if (off <= 0x3b || (0x58 <= off && off <= 0x5f))
      return CharClass::kConsonant;
    // Sinhala doesn't have Nukta or Avagraha.
    if (off == 0x3c) return CharClass::kNukta;
    if (off == 0x3d) return CharClass::kVowel; // avagraha
    if (off <= 0x4c || (0x51 <= off && off <= 0x54)) return CharClass::kMatra;
    if (0x55 <= off && off <= 0x57) return CharClass::kMatraPiece;
    if (off == 0x4d) return CharClass::kVirama;
  }
  if (off == 0x60 || off == 0x61) return CharClass::kVowel;
  if (off == 0x62 || off == 0x63) return CharClass::kMatra;
  // Danda and digits up to 6f are OK as other.
  // 70-7f are script-specific.
  if (script_ == ViramaScript::kBengali && (off == 0x70 || off == 0x71))
    return CharClass::kConsonant;
  if (script_ == ViramaScript::kGurmukhi && (off == 0x72 || off == 0x73))
    return CharClass::kConsonant;
  if (script_ == ViramaScript::kSinhala && off == 0x70)
    return CharClass::kConsonant;
  if (script_ == ViramaScript::kDevanagari && off == 0x70)
    return CharClass::kOther;
  if (0x70 <= off && off <= 0x73) return CharClass::kVowelModifier;
  // Non Indic, Digits, Measures, danda, etc.
  return CharClass::kOther;
}

// Helper consumes/copies a virama and any associated post-virama joiners.
// A linking virama (with either type of pre-virama joiner, post-virama ZWJ, or
// no joiner at all) must be followed by a consonant.
// A non-linking (explicit) virama is indicated by a ZWNJ after it, or a non
// consonant, space, or character from a different script. We clean up the
// representation to make it consistent by adding a ZWNJ if missing from a
// non-linking virama. Returns false with an invalid sequence.
bool ValidateIndic::ConsumeViramaIfValid(IndicPair joiner, bool post_matra) {
  int num_codes = codes_.size();
  if (joiner.first == CharClass::kOther) {
    CodeOnlyToOutput();
    if (codes_used_ < num_codes &&
        codes_[codes_used_].second == kZeroWidthJoiner) {
      // Post-matra viramas must be explicit, so no joiners allowed here.
      if (post_matra) {
        if (report_errors_) tprintf("ZWJ after a post-matra virama!!\n");
        return false;
      }
      if (codes_used_ + 1 < num_codes &&
          codes_[codes_used_ - 2].second != kRayana &&
          (codes_[codes_used_ + 1].second == kZeroWidthNonJoiner ||
           codes_[codes_used_ + 1].second == kYayana ||
           codes_[codes_used_ + 1].second == kRayana)) {
        // This combination will be picked up later.
        ASSERT_HOST(!CodeOnlyToOutput());
      } else {
        // Half-form with optional Nukta.
        int len = output_.size() + 1 - output_used_;
        if (UseMultiCode(len)) return true;
      }
      if (codes_used_ < num_codes &&
          codes_[codes_used_].second == kZeroWidthNonJoiner) {
        if (output_used_ == output_.size() ||
            output_[output_used_] != kRayana) {
          if (report_errors_) {
            tprintf("Virama ZWJ ZWNJ in non-Sinhala: base=0x%x!\n",
                    static_cast<int>(script_));
          }
          return false;
        }
        // Special Sinhala case of Stand-alone Repaya. ['RA' H Z z]
        if (UseMultiCode(4)) return true;
      }
    } else if (codes_used_ == num_codes ||
               codes_[codes_used_].first != CharClass::kConsonant ||
               post_matra) {
      if (codes_used_ == num_codes ||
          codes_[codes_used_].second != kZeroWidthNonJoiner) {
        // It is valid to have an unterminated virama at the end of a word, but
        // for consistency, we will always add ZWNJ if not present.
        output_.push_back(kZeroWidthNonJoiner);
      } else {
        CodeOnlyToOutput();
      }
      // Explicit virama [H z]
      MultiCodePart(2);
    }
  } else {
    // Pre-virama joiner [{Z|z} H] requests specific conjunct.
    if (UseMultiCode(2)) {
      if (report_errors_)
        tprintf("Invalid pre-virama joiner with no 2nd consonant!!\n");
      return false;
    }
    if (codes_[codes_used_].second == kZeroWidthJoiner ||
        codes_[codes_used_].second == kZeroWidthNonJoiner) {
      if (report_errors_) {
        tprintf("JHJ!!: 0x%x 0x%x 0x%x\n", joiner.second, output_.back(),
                codes_[codes_used_].second);
      }
      return false;
    }
  }
  // It is good so far as it goes.
  return true;
}

// Helper consumes/copies a series of consonants separated by viramas while
// valid, but not any vowel or other modifiers.
bool ValidateIndic::ConsumeConsonantHeadIfValid() {
  const int num_codes = codes_.size();
  // Consonant aksara
  do {
    CodeOnlyToOutput();
    // Special Sinhala case of [H Z Yayana/Rayana].
    int index = output_.size() - 3;
    if (output_used_ <= index &&
        (output_.back() == kYayana || output_.back() == kRayana) &&
        IsVirama(output_[index]) && output_[index + 1] == kZeroWidthJoiner) {
      MultiCodePart(3);
    }
    bool have_nukta = false;
    if (codes_used_ < num_codes &&
        codes_[codes_used_].first == CharClass::kNukta) {
      have_nukta = true;
      CodeOnlyToOutput();
    }
    // Test for subscript conjunct.
    index = output_.size() - 2 - have_nukta;
    if (output_used_ <= index && IsSubscriptScript() &&
        IsVirama(output_[index])) {
      // Output previous virama, consonant + optional nukta.
      MultiCodePart(2 + have_nukta);
    }
    IndicPair joiner(CharClass::kOther, 0);
    if (codes_used_ < num_codes &&
        (codes_[codes_used_].second == kZeroWidthJoiner ||
         (codes_[codes_used_].second == kZeroWidthNonJoiner &&
          script_ == ViramaScript::kMalayalam))) {
      joiner = codes_[codes_used_];
      if (++codes_used_ == num_codes) {
        if (report_errors_) {
          tprintf("Skipping ending joiner: 0x%x 0x%x\n", output_.back(),
                  joiner.second);
        }
        return true;
      }
      if (codes_[codes_used_].first == CharClass::kVirama) {
        output_.push_back(joiner.second);
      } else {
        if (report_errors_) {
          tprintf("Skipping unnecessary joiner: 0x%x 0x%x 0x%x\n",
                  output_.back(), joiner.second, codes_[codes_used_].second);
        }
        joiner = std::make_pair(CharClass::kOther, 0);
      }
    }
    if (codes_used_ < num_codes &&
        codes_[codes_used_].first == CharClass::kVirama) {
      if (!ConsumeViramaIfValid(joiner, false)) return false;
    } else {
      break;  // No virama, so the run of consonants is over.
    }
  } while (codes_used_ < num_codes &&
           codes_[codes_used_].first == CharClass::kConsonant);
  if (output_used_ < output_.size()) MultiCodePart(1);
  return true;
}

// Helper consumes/copies a tail part of a consonant, comprising optional
// matra/piece, vowel modifier, vedic mark, terminating virama.
bool ValidateIndic::ConsumeConsonantTailIfValid() {
  if (codes_used_ == codes_.size()) return true;
  // No virama: Finish the grapheme.
  // Are multiple matras allowed?
  if (codes_[codes_used_].first == CharClass::kMatra) {
    if (UseMultiCode(1)) return true;
    if (codes_[codes_used_].first == CharClass::kMatraPiece) {
      if (UseMultiCode(1)) return true;
    }
  }
  while (codes_[codes_used_].first == CharClass::kVowelModifier) {
    if (UseMultiCode(1)) return true;
    // Only Malayalam allows only repeated 0xd02.
    if (script_ != ViramaScript::kMalayalam || output_.back() != 0xd02) break;
  }
  while (codes_[codes_used_].first == CharClass::kVedicMark) {
    if (UseMultiCode(1)) return true;
  }
  if (codes_[codes_used_].first == CharClass::kVirama) {
    if (!ConsumeViramaIfValid(IndicPair(CharClass::kOther, 0), true)) {
      return false;
    }
  }
  // What we have consumed so far is a valid consonant cluster.
  if (output_used_ < output_.size()) MultiCodePart(1);

  return true;
}

// Helper consumes/copies a vowel and optional modifiers.
bool ValidateIndic::ConsumeVowelIfValid() {
  if (UseMultiCode(1)) return true;
  while (codes_[codes_used_].first == CharClass::kVowelModifier) {
    if (UseMultiCode(1)) return true;
    // Only Malayalam allows repeated modifiers?
    if (script_ != ViramaScript::kMalayalam) break;
  }
  while (codes_[codes_used_].first == CharClass::kVedicMark) {
    if (UseMultiCode(1)) return true;
  }
  // What we have consumed so far is a valid vowel cluster.
  return true;
}

}  // namespace tesseract
