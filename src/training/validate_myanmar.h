#ifndef TESSERACT_TRAINING_VALIDATE_MYANMAR_H_
#define TESSERACT_TRAINING_VALIDATE_MYANMAR_H_

#include "validator.h"

namespace tesseract {

// Subclass of Validator that validates and segments Myanmar.
class ValidateMyanmar : public Validator {
 public:
  ValidateMyanmar(ViramaScript script, bool report_errors)
      : Validator(script, report_errors) {}
  ~ValidateMyanmar() {}

 protected:
  // Returns whether codes matches the pattern for a Myanmar Grapheme.
  // Consumes the next Grapheme in codes_[codes_used_++...] and copies it to
  // parts_ and output_. Returns true if a valid Grapheme was consumed,
  // otherwise does not increment codes_used_.
  bool ConsumeGraphemeIfValid() override;
  // Returns the CharClass corresponding to the given Unicode ch.
  Validator::CharClass UnicodeToCharClass(char32 ch) const override;

 private:
  // Helper consumes/copies a virama and any subscript consonant.
  // Returns true if the end of input is reached.
  bool ConsumeSubscriptIfPresent();
  // Helper consumes/copies a series of optional signs.
  // Returns true if the end of input is reached.
  bool ConsumeOptionalSignsIfPresent();
  // Returns true if the unicode is a Myanmar "letter" including consonants
  // and independent vowels. Although table 16-3 distinguishes between some
  // base consonants and vowels, the extensions make no such distinction, so we
  // put them all into a single bucket.
  static bool IsMyanmarLetter(char32 ch);
  // Returns true if ch is a Myanmar digit or other symbol that does not take
  // part in being a syllable.
  static bool IsMyanmarOther(char32 ch);

  // Some special unicodes used only for Myanmar processing.
  static const char32 kMyanmarAsat = 0x103a;
  static const char32 kMyanmarMedialYa = 0x103b;
};

}  // namespace tesseract

#endif  // TESSERACT_TRAINING_VALIDATE_MYANMAR_H_
