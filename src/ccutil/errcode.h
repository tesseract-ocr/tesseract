/**********************************************************************
 * File:        errcode.h  (Formerly error.h)
 * Description: Header file for generic error handler class
 * Author:      Ray Smith
 *
 * (C) Copyright 1990, Hewlett-Packard Ltd.
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

#ifndef ERRCODE_H
#define ERRCODE_H

#include <tesseract/export.h> // for TESS_API

namespace tesseract {

/*Control parameters for error()*/
enum TessErrorLogCode {
  DBG = -1,     /*log without alert */
  TESSLOG = 0,  /*alert user */
  TESSEXIT = 1, /*exit after error */
  ABORT = 2     /*abort after error */
};

#if !defined(__GNUC__) && !defined(__attribute__)
# define __attribute__(attr) // compiler without support for __attribute__
#endif

class TESS_API ERRCODE { // error handler class
  const char *message;   // error message
public:
  void error(                  // error print function
      const char *caller,      // function location
      TessErrorLogCode action, // action to take
      const char *format, ...  // fprintf format
  ) const __attribute__((format(printf, 4, 5)));
  void error(const char *caller, TessErrorLogCode action) const {
    error(caller, action, nullptr);
  }
  constexpr ERRCODE(const char *string) : message(string) {} // initialize with string
};

constexpr ERRCODE ASSERT_FAILED("Assert failed");

#define DO_NOTHING static_cast<void>(0)

#define ASSERT_HOST(x) \
  (x) ? DO_NOTHING : ASSERT_FAILED.error(#x, ABORT, "in file %s, line %d", __FILE__, __LINE__)

#define ASSERT_HOST_MSG(x, ...)                                                \
  if (!(x)) {                                                                  \
    tprintf(__VA_ARGS__);                                                      \
    ASSERT_FAILED.error(#x, ABORT, "in file %s, line %d", __FILE__, __LINE__); \
  }

} // namespace tesseract

#endif
