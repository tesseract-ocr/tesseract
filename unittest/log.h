///////////////////////////////////////////////////////////////////////
// File:        log.h
// Description: Include for custom log message for unittest for tesseract.
//              based on
//              https://stackoverflow.com/questions/16491675/how-to-send-custom-message-in-google-c-testing-framework
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
///////////////////////////////////////////////////////////////////////

#ifndef TESSERACT_UNITTEST_LOG_H_
#define TESSERACT_UNITTEST_LOG_H_

// This is a minimal implementation of the TensorFlow logging API
// which is sufficient for the Tesseract unit tests.

// See tensorflow/core/platform/default/logging.h for the original code.

#include <iostream>

enum LogLevel { INFO, WARNING, ERROR, FATAL };

// Avoid conflict with logging.h from TensorFlow.
#undef LOG

static inline std::ostream &LOG(enum LogLevel level) {
  switch (level) {
    case INFO:
      std::cout << "[INFO]  ";
      break;
    case WARNING:
      std::cout << "[WARN]  ";
      break;
    case ERROR:
      std::cout << "[ERROR] ";
      break;
    case FATAL:
      std::cout << "[FATAL] ";
      break;
  }
  return std::cout;
}

// Avoid conflict with logging.h from TensorFlow.
#undef QCHECK

// https://github.com/google/ion/blob/master/ion/base/logging.h
static inline std::ostream &QCHECK(bool condition) {
  if (condition) {
    static std::ostream null_stream(nullptr);
    return null_stream;
  }
  return std::cout;
}

#endif // TESSERACT_UNITTEST_LOG_H_
