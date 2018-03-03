///////////////////////////////////////////////////////////////////////
// File:        log.h
// Description: Include for custom log message for unittest for tesseract.
//               based on //https://stackoverflow.com/questions/16491675/how-to-send-custom-message-in-google-c-testing-framework
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

#include <iostream>

  static class LOG { public: LOG() {}
    std::ostream&  info() {
        std::cout << "[ LOG MSG  ] ";
        return std::cout;
    }
} log;

#endif  // TESSERACT_UNITTEST_LOG_H_
