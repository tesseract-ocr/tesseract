/**********************************************************************
 * File:        icuerrorcode.h
 * Description: Wrapper class for UErrorCode, with conversion operators for
 *              direct use in ICU C and C++ APIs.
 * Author:      Fredrik Roubert
 * Created:     Thu July 4 2013
 *
 * Features:
 * - The constructor initializes the internal UErrorCode to U_ZERO_ERROR,
 *   removing one common source of errors.
 * - Same use in C APIs taking a UErrorCode* (pointer) and C++ taking
 *   UErrorCode& (reference), via conversion operators.
 * - Automatic checking for success when it goes out of scope. On failure,
 *   the destructor will log an error message and exit.
 *
 * Most of ICU will handle errors gracefully and provide sensible fallbacks.
 * Using IcuErrorCode, it is therefore possible to write very compact code
 * that does sensible things on failure and provides logging for debugging.
 *
 * Example:
 * IcuErrorCode icuerrorcode;
 * return collator.compareUTF8(a, b, icuerrorcode) == UCOL_EQUAL;
 *
 * (C) Copyright 2013, Google Inc.
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
#ifndef TESSERACT_CCUTIL_ICUERRORCODE_H_
#define TESSERACT_CCUTIL_ICUERRORCODE_H_

#include <cstdlib> // for exit
#include "tprintf.h"
#include "unicode/errorcode.h" // From libicu

namespace tesseract {

class IcuErrorCode : public icu::ErrorCode {
public:
  IcuErrorCode() = default;
  ~IcuErrorCode() override;

protected:
  void handleFailure() const override {
    tprintf("ICU ERROR: %s\n", errorName());
    exit(errorCode);
  }

private:
  // Disallow implicit copying of object.
  IcuErrorCode(const IcuErrorCode &) = delete;
  void operator=(const IcuErrorCode &) = delete;
};

} // namespace tesseract
#endif // TESSERACT_CCUTIL_ICUERRORCODE_H_
