/**********************************************************************
    * File:        validate_javanese.h
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
 
#ifndef TESSERACT_TRAINING_VALIDATE_JAVANESE_H_
#define TESSERACT_TRAINING_VALIDATE_JAVANESE_H_

#include "validator.h"

namespace tesseract {

// Subclass of Validator that validates and segments Javanese.
class ValidateJavanese : public Validator {
 public:
  ValidateJavanese(ViramaScript script, bool report_errors)
      : Validator(script, report_errors) {}
  ~ValidateJavanese() {}

 protected:
  // Returns whether codes matches the pattern for an Javanese Grapheme.
  // Consumes the next Grapheme in codes_[codes_used_++...] and copies it to
  // parts_ and output_. Returns true if a valid Grapheme was consumed,
  // otherwise does not increment codes_used_.
  bool ConsumeGraphemeIfValid() override;
  // Returns the CharClass corresponding to the given Unicode ch.
  CharClass UnicodeToCharClass(char32 ch) const override;
};

}  // namespace tesseract

#endif  // TESSERACT_TRAINING_VALIDATE_JAVANESE_H_
