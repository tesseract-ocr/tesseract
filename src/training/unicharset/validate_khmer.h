#ifndef TESSERACT_TRAINING_VALIDATE_KHMER_H_
#define TESSERACT_TRAINING_VALIDATE_KHMER_H_

#include "validator.h"

namespace tesseract {

// Subclass of Validator that validates and segments Khmer.
class ValidateKhmer : public Validator {
public:
  ValidateKhmer(ViramaScript script, bool report_errors) : Validator(script, report_errors) {}
  ~ValidateKhmer() override = default;

protected:
  // Returns whether codes matches the pattern for an Khmer Grapheme.
  // Consumes the next Grapheme in codes_[codes_used_++...] and copies it to
  // parts_ and output_. Returns true if a valid Grapheme was consumed,
  // otherwise does not increment codes_used_.
  bool ConsumeGraphemeIfValid() override;
  // Returns the CharClass corresponding to the given Unicode ch.
  CharClass UnicodeToCharClass(char32 ch) const override;
};

} // namespace tesseract

#endif // TESSERACT_TRAINING_VALIDATE_KHMER_H_
