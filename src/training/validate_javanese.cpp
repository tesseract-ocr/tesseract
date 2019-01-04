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
// The Consonant class here includes independent vowels.
// The order of components in an orthographic syllable as expressed in BNF is:
// {C F} C {{R}Y} {V{A}} {Z}
// Translated to the codes used by the CharClass enum:
// [(V|C[N])(H)] (V|C[N]) [[N]N] [M[D]] [v]
// Also see https://r12a.github.io/scripts/javanese/ for detailed notes.
// Validation rules copied from validate_indic.cpp and modified for Javanese.
// Indic - for reference
//  + vowel Grapheme:  V[D](v)*
//  + consonant Grapheme: (C[N](H|HZ|Hz|ZH)?)*C[N](H|Hz)?[M[P]][D](v)*

bool ValidateJavanese::ConsumeGraphemeIfValid() {
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

// Helper consumes/copies a virama and any associated post-virama joiners.
// A linking virama (with either type of pre-virama joiner, post-virama ZWJ, or
// no joiner at all) must be followed by a consonant.
// A non-linking (explicit) virama is indicated by a ZWNJ after it, or a non
// consonant, space, or character from a different script. We clean up the
// representation to make it consistent by adding a ZWNJ if missing from a
// non-linking virama. Returns false with an invalid sequence.
bool ValidateJavanese::ConsumeViramaIfValid(IndicPair joiner, bool post_matra) {
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
          codes_[codes_used_ - 2].second != kCakra &&
          (codes_[codes_used_ + 1].second == kZeroWidthNonJoiner ||
           codes_[codes_used_ + 1].second == kPengkal ||
           codes_[codes_used_ + 1].second == kCakra)) {
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
            output_[output_used_] != kCakra) {
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
        CodeOnlyToOutput();
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
bool ValidateJavanese::ConsumeConsonantHeadIfValid() {
  const int num_codes = codes_.size();
  // Consonant aksara
  do {
    CodeOnlyToOutput();
    // Special Sinhala case of [H Z Yayana/Rayana].
    int index = output_.size() - 3;
    if (output_used_ <= index &&
        (output_.back() == kPengkal || output_.back() == kCakra) &&
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
bool ValidateJavanese::ConsumeConsonantTailIfValid() {
  if (codes_used_ == codes_.size()) return true;
  // No virama: Finish the grapheme.
  // Are multiple matras allowed?
  if (codes_[codes_used_].first == CharClass::kMatra) {
    if (UseMultiCode(1)) return true;
    if (codes_[codes_used_].first == CharClass::kMatraPiece) {
      if (UseMultiCode(1)) return true;
    }
  }
  // Tarung also used for long versions of u and o vowels and vocalic r
  // Taling + Tarung is valid eg. ꦏ + ◌ꦺ + ◌ꦴ
  while (codes_[codes_used_].first == CharClass::kMatraPiece) {
    if (UseMultiCode(1)) return true;
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
bool ValidateJavanese::ConsumeVowelIfValid() {
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
 
 
Validator::CharClass ValidateJavanese::UnicodeToCharClass(char32 ch) const {
  if (ch == kZeroWidthNonJoiner) return CharClass::kZeroWidthNonJoiner;
  if (ch == kZeroWidthJoiner) return CharClass::kZeroWidthJoiner;
  // Offset from the start of the relevant unicode code block aka code page.
  int off = ch - static_cast<char32>(script_);
  // Anything in another code block is other.
  if (off < 0 || off >= kIndicCodePageSize) return CharClass::kOther;
  if (off < 0x4) return CharClass::kVowelModifier;
  if (off <= 0x32) return CharClass::kConsonant; // includes independent vowels
  if (off == 0x33) return CharClass::kNukta; // A9B3 CECAK TELU
  if (off == 0x34) return CharClass::kMatraPiece; // A9B4 TARUNG two part vowels
  if (off <= 0x39) return CharClass::kMatra;
  if (off <= 0x3a) return CharClass::kConsonant; // A9BA TALING - pre base vowel
  if (off <= 0x3d) return CharClass::kMatra;
  if (off <= 0x3f) return CharClass::kNukta; // A9BE-A9BF PENGKAL-CAKRA medial consonants
  if (off == 0x40) return CharClass::kVirama; // A9C0 PANGKON
  return CharClass::kOther;
}

}  // namespace tesseract
