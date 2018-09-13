/**********************************************************************
 * File:        validator.h
 * Description: Base class for various text validators. Intended mainly for
 *              scripts that use a virama character.
 * Author:      Ray Smith
 * Created:     Tue May 23 2017
 *
 * (C) Copyright 2017, Google Inc.
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

#ifndef TESSERACT_TRAINING_VALIDATOR_H_
#define TESSERACT_TRAINING_VALIDATOR_H_

#include <memory>
#include <vector>
#include "unichar.h"

namespace tesseract {

// Different kinds of grapheme normalization - not just for Indic!
// A grapheme is a syllable unit in Indic and can be several unicodes.
// In other scripts, a grapheme is a base character and accent/diacritic
// combination, as not all accented characters have a single composed form.
enum class GraphemeNormMode {
  // Validation result is a single string, even if input is multi-word.
  kSingleString,
  // Standard unicode graphemes are validated and output as grapheme units.
  kCombined,
  // Graphemes are validated and sub-divided. For virama-using scripts, units
  // that correspond to repeatable glyphs are generated. (Mostly single unicodes
  // but viramas and joiners are paired with the most sensible neighbor.)
  // For non-virama scripts, this means that base/accent pairs are separated,
  // ie the output is individual unicodes.
  kGlyphSplit,
  // The output is always single unicodes, regardless of the script.
  kIndividualUnicodes,
};

// An enum representing the scripts that use a virama character. It is
// guaranteed that the value of any element, (except kNonVirama) can be cast
// to a unicode (char32) value that represents the start of the unicode range
// of the corresponding script.
enum class ViramaScript : char32 {
  kNonVirama = 0,
  kDevanagari = 0x900,
  kBengali = 0x980,
  kGurmukhi = 0xa00,
  kGujarati = 0xa80,
  kOriya = 0xb00,
  kTamil = 0xb80,
  kTelugu = 0xc00,
  kKannada = 0xc80,
  kMalayalam = 0xd00,
  kSinhala = 0xd80,
  kMyanmar = 0x1000,
  kKhmer = 0x1780,
  kJavanese = 0xa980,
};

// Base class offers a validation API and protected methods to allow subclasses
// to easily build the validated/segmented output.
class Validator {
 public:
  // Validates and cleans the src vector of unicodes to the *dest, according to
  // g_mode. In the case of kSingleString, a single vector containing the whole
  // result is added to *dest. With kCombined, multiple vectors are added to
  // *dest with one grapheme in each. With kGlyphSplit, multiple vectors are
  // added to *dest with a smaller unit representing a glyph in each.
  // In case of validation error, returns false and as much as possible of the
  // input, without discarding invalid text.
  static bool ValidateCleanAndSegment(GraphemeNormMode g_mode,
                                      bool report_errors,
                                      const std::vector<char32>& src,
                                      std::vector<std::vector<char32>>* dest);

  // Returns true if the unicode ch is a non-printing zero-width mark of no
  // significance to OCR training or evaluation.
  static bool IsZeroWidthMark(char32 ch) {
    return ch == kZeroWidthSpace || ch == kLeftToRightMark ||
           ch == kRightToLeftMark || ch == kInvalid;
  }
  virtual ~Validator();

  // Some specific but universally useful unicodes.
  static const char32 kZeroWidthSpace;
  static const char32 kZeroWidthNonJoiner;
  static const char32 kZeroWidthJoiner;
  static const char32 kLeftToRightMark;
  static const char32 kRightToLeftMark;
  static const char32 kInvalid;

 protected:
  // These are more or less the character class identifiers in the ISCII
  // standard, section 8.  They have been augmented with the Unicode meta
  // characters Zero Width Joiner and Zero Width Non Joiner, and the
  // Unicode Vedic Marks.
  // The best sources of information on Unicode and Indic scripts are:
  //   http://varamozhi.sourceforge.net/iscii91.pdf
  //   http://www.unicode.org/versions/Unicode9.0.0/ch12.pdf
  //   http://unicode.org/faq/indic.html
  //   http://www.microsoft.com/typography/otfntdev/teluguot/shaping.aspx
  enum class CharClass {
    // NOTE: The values of the enum members are meaningless and arbitrary, ie
    // they are not used for sorting, or any other risky application.
    // The reason they are what they are is they are a single character
    // abbreviation that can be used in a regexp/BNF definition of a grammar,
    // IN A COMMENT, and still not relied upon in the code.
    kConsonant = 'C',
    kVowel = 'V',
    kVirama = 'H',              // (aka Halant)
    kMatra = 'M',               // (aka Dependent Vowel)
    kMatraPiece = 'P',          // unicode provides pieces of Matras.
    kVowelModifier = 'D',       // (candrabindu, anusvara, visarga, other marks)
    kZeroWidthNonJoiner = 'z',  // Unicode Zero Width Non-Joiner U+200C
    kZeroWidthJoiner = 'Z',     // Unicode Zero Width Joiner U+200D
    kVedicMark = 'v',           // Modifiers can come modify any indic syllable.
    kNukta = 'N',               // Occurs only immediately after consonants.
    kRobat = 'R',               // Khmer only.
    kOther = 'O',               // (digits, measures, non-Indic, etc)
    // Additional classes used only by ValidateGrapheme.
    kWhitespace = ' ',
    kCombiner = 'c',  // Combiners other than virama.
  };
  using IndicPair = std::pair<CharClass, char32>;

  Validator(ViramaScript script, bool report_errors)
      : script_(script),
        codes_used_(0),
        output_used_(0),
        report_errors_(report_errors) {}

  // Factory method that understands how to map script to the right subclass.
  static std::unique_ptr<Validator> ScriptValidator(ViramaScript script,
                                                    bool report_errors);

  // Internal version of the public static ValidateCleanAndSegment.
  // Validates and cleans the src vector of unicodes to the *dest, according to
  // its type and the given g_mode.
  // In case of validation error, returns false and returns as much as possible
  // of the input, without discarding invalid text.
  bool ValidateCleanAndSegmentInternal(GraphemeNormMode g_mode,
                                       const std::vector<char32>& src,
                                       std::vector<std::vector<char32>>* dest);
  // Moves the results from parts_ or output_ to dest according to g_mode.
  void MoveResultsToDest(GraphemeNormMode g_mode,
                         std::vector<std::vector<char32>>* dest);

  // Computes and returns the ViramaScript corresponding to the most frequent
  // virama-using script in the input, or kNonVirama if none are present.
  static ViramaScript MostFrequentViramaScript(
      const std::vector<char32>& utf32);
  // Returns true if the given UTF-32 unicode is a "virama" character.
  static bool IsVirama(char32 unicode);
  // Returns true if the given UTF-32 unicode is a vedic accent.
  static bool IsVedicAccent(char32 unicode);
  // Returns true if the script is one that uses subscripts for conjuncts.
  bool IsSubscriptScript() const;

  // Helper function appends the next element of codes_ only to output_,
  // without touching parts_
  // Returns true at the end of codes_.
  bool CodeOnlyToOutput() {
    output_.push_back(codes_[codes_used_].second);
    return ++codes_used_ == codes_.size();
  }

  // Helper function adds a length-element vector to parts_ from the last length
  // elements of output_. If there are more than length unused elements in
  // output_, adds unicodes as single-element vectors to parts_ to catch
  // output_used_ up to output->size() - length before adding the length-element
  // vector.
  void MultiCodePart(int length) {
    while (output_used_ + length < output_.size()) {
      parts_.emplace_back(
          std::initializer_list<char32>{output_[output_used_++]});
    }
    parts_.emplace_back(std::initializer_list<char32>{output_[output_used_]});
    while (++output_used_ < output_.size()) {
      parts_.back().push_back(output_[output_used_]);
    }
  }

  // Helper function appends the next element of codes_ to output_, and then
  // calls MultiCodePart to add the appropriate components to parts_.
  // Returns true at the end of codes_.
  bool UseMultiCode(int length) {
    output_.push_back(codes_[codes_used_].second);
    MultiCodePart(length);
    return ++codes_used_ == codes_.size();
  }

  // Consumes the next Grapheme in codes_[codes_used_++...] and copies it to
  // parts_ and output_. Returns true if a valid Grapheme was consumed,
  // otherwise does not increment codes_used_.
  virtual bool ConsumeGraphemeIfValid() = 0;
  // Sets codes_ to the class codes for the given unicode text.
  void ComputeClassCodes(const std::vector<char32>& text);
  // Returns the CharClass corresponding to the given Unicode ch.
  virtual CharClass UnicodeToCharClass(char32 ch) const = 0;
  // Resets to the initial state.
  void Clear();

  // Number of unicodes in each Indic codepage.
  static const int kIndicCodePageSize = 128;
  // Lowest unicode value of any Indic script. (Devanagari).
  static const char32 kMinIndicUnicode = 0x900;
  // Highest unicode value of any consistent (ISCII-based) Indic script.
  static const char32 kMaxSinhalaUnicode = 0xdff;
  // Highest unicode value of any virama-using script. (Khmer).
  static const char32 kMaxViramaScriptUnicode = 0x17ff;
  // Some special unicodes.
  static const char32 kSinhalaVirama = 0xdca;
  static const char32 kMyanmarVirama = 0x1039;
  static const char32 kKhmerVirama = 0x17d2;
  // Javanese Script - aksarajawa
  static const char32 kJavaneseVirama = 0xa9c0;
  static const char32 kMaxJavaneseUnicode = 0xa9df;

  // Script we are operating on.
  ViramaScript script_;
  // Input unicodes with assigned CharClass is the data to be validated.
  std::vector<IndicPair> codes_;
  // Glyph-like components of the input.
  std::vector<std::vector<char32>> parts_;
  // Copied validated unicodes from codes_ that are OK to output.
  std::vector<char32> output_;
  // The number of elements of codes_ that have been processed so far.
  int codes_used_;
  // The number of elements of output_ that have already been added to parts_.
  int output_used_;
  // Log error messages for reasons why text is invalid.
  bool report_errors_;
};

}  // namespace tesseract

#endif  // TESSERACT_TRAINING_VALIDATOR_H_
