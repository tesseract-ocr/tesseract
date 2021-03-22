#ifndef TESSERACT_TRAINING_VALIDATE_GRAPHEME_H_
#define TESSERACT_TRAINING_VALIDATE_GRAPHEME_H_

#include "validator.h"

namespace tesseract {

// Subclass of Validator that validates and segments generic unicode into
// grapheme clusters, including Latin with diacritics.
class ValidateGrapheme : public Validator {
public:
  ValidateGrapheme(ViramaScript script, bool report_errors) : Validator(script, report_errors) {}
  ~ValidateGrapheme() override = default;

protected:
  // Consumes the next Grapheme in codes_[codes_used_++...] and copies it to
  // parts_ and output_. Returns true if a valid Grapheme was consumed,
  // otherwise does not increment codes_used_.
  bool ConsumeGraphemeIfValid() override;
  // Returns the CharClass corresponding to the given Unicode ch.
  CharClass UnicodeToCharClass(char32 ch) const override;

private:
  // Helper returns true if the sequence prev_ch,ch is invalid.
  bool IsBadlyFormed(char32 prev_ch, char32 ch);
  // Helper returns true if the sequence prev_ch,ch is an invalid Indic vowel.
  static bool IsBadlyFormedIndicVowel(char32 prev_ch, char32 ch);
  // Helper returns true if the sequence prev_ch,ch is invalid Thai.
  static bool IsBadlyFormedThai(char32 prev_ch, char32 ch);
};

} // namespace tesseract

#endif // TESSERACT_TRAINING_VALIDATE_GRAPHEME_H_
