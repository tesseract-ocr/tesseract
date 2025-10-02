/**********************************************************************
 * File:        errcode.cpp  (Formerly error.c)
 * Description: Generic error handler function
 * Author:      Ray Smith
 *
 * (C) Copyright 1989, Hewlett-Packard Ltd.
 ** Licensed under the Apache License, Version 2.0 (the "License");
 ** you may not use this file except in compliance with the License.
 ** You may obtain a copy of the License at
 ** http://www.apache.org/licenses/LICENSE-2.0
 ** Unless required by applicable law or agreed to in writing, software
 ** distributed under the License is distributed on an "AS IS" BASIS,
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 ** See the License for the specific language governing permissions and
 ** limitations under the License.
 *
 **********************************************************************/

#include "errcode.h"

#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream> // for std::cerr
#include <sstream>  // for std::stringstream

namespace tesseract {

constexpr ERRCODE BADERRACTION("Illegal error action");
#define MAX_MSG 1024

/**********************************************************************
 * error
 *
 * Print an error message and continue, exit or abort according to action.
 * Makes use of error messages and numbers in a common place.
 *
 **********************************************************************/
void ERRCODE::error(         // handle error
    const char *caller,      // name of caller
    TessErrorLogCode action, // action to take
    const char *format, ...  // special message
    ) const {
  va_list args; // variable args
  std::stringstream msg;

  if (caller != nullptr) {
    // name of caller
    msg << caller << ':';
  }
  // actual message
  msg << "Error:" << message;
  if (format != nullptr) {
    char str[MAX_MSG];
    va_start(args, format); // variable list
    // print remainder
    std::vsnprintf(str, sizeof(str), format, args);
    // ensure termination
    str[sizeof(str) - 1] = '\0';
    va_end(args);
    msg << ':' << str;
  }

  std::cerr << msg.str() << '\n';

  switch (action) {
    case DBG:
    case TESSLOG:
      return; // report only
    case TESSEXIT:
    case ABORT:
      // This used to trigger a segfault or abort();
      // However, at least for library use, only exceptions should be acceptable.
      // Even in the standalone application case, exceptions are better,
      // because the default handler will print the message along with the stack trace.
      throw std::runtime_error(msg.str());
    default:
      BADERRACTION.error("error", ABORT);
  }
}

} // namespace tesseract
