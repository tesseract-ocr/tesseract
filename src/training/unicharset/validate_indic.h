#ifndef TESSERACT_TRAINING_VALIDATE_INDIC_H_
#define TESSERACT_TRAINING_VALIDATE_INDIC_H_

#include "validator.h"

namespace tesseract {

// Subclass of Validator that validates and segments Indic scripts in the
// unicode range 0x900-0xdff (Devanagari-Sinhala).
class ValidateIndic : public Validator {
public:
  ValidateIndic(ViramaScript script, bool report_errors) : Validator(script, report_errors) {}
  ~ValidateIndic() override = default;

protected:
  // Returns whether codes matches the pattern for an Indic Grapheme.
  // Consumes the next Grapheme in codes_[codes_used_++...] and copies it to
  // parts_ and output_. Returns true if a valid Grapheme was consumed,
  // otherwise does not increment codes_used_.
  bool ConsumeGraphemeIfValid() override;
  // Returns the CharClass corresponding to the given Unicode ch.
  Validator::CharClass UnicodeToCharClass(char32 ch) const override;

private:
  // Helper consumes/copies a virama and any associated post-virama joiners.
  bool ConsumeViramaIfValid(IndicPair joiner, bool post_matra);
  // Helper consumes/copies a series of consonants separated by viramas while
  // valid, but not any vowel or other modifiers.
  bool ConsumeConsonantHeadIfValid();
  // Helper consumes/copies a tail part of a consonant, comprising optional
  // matra/piece, vowel modifier, vedic mark, terminating virama.
  bool ConsumeConsonantTailIfValid();
  // Helper consumes/copies a vowel and optional modifiers.
  bool ConsumeVowelIfValid();

  // Some special unicodes used only for Indic processing.
  static const char32 kYayana = 0xdba; // Sinhala Ya
  static const char32 kRayana = 0xdbb; // Sinhala Ra
};

} // namespace tesseract

#endif // TESSERACT_TRAINING_VALIDATE_INDIC_H_
