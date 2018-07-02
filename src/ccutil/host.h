/******************************************************************************
 **  Filename:       host.h
 **  Purpose:        This is the system independent typedefs and defines
 **  Author:         MN, JG, MD
 **
 **  (c) Copyright Hewlett-Packard Company, 1988-1996.
 ** Licensed under the Apache License, Version 2.0 (the "License");
 ** you may not use this file except in compliance with the License.
 ** You may obtain a copy of the License at
 ** http://www.apache.org/licenses/LICENSE-2.0
 ** Unless required by applicable law or agreed to in writing, software
 ** distributed under the License is distributed on an "AS IS" BASIS,
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 ** See the License for the specific language governing permissions and
 ** limitations under the License.
 */

#ifndef TESSERACT_CCUTIL_HOST_H_
#define TESSERACT_CCUTIL_HOST_H_

#include <limits>
#include "platform.h"
/* _WIN32 */
#ifdef _WIN32
#include <windows.h>
#undef min
#undef max
#endif

#include <cinttypes>  // PRId32, ...
#include <cstdint>    // int32_t, ...

// definitions of portable data types (numbers and characters)
using BOOL8 = unsigned char;

#if defined(_WIN32)

/* MinGW defines the standard PRI... macros, but MSVS doesn't. */

#if !defined(PRId32)
#define PRId32 "d"
#endif

#if !defined(PRId64)
#define PRId64 "I64d"
#endif

#endif /* _WIN32 */

// Defines
#ifndef TRUE
#define TRUE            1
#endif

#ifndef FALSE
#define FALSE           0
#endif

// Return true if x is within tolerance of y
template<class T> bool NearlyEqual(T x, T y, T tolerance) {
  T diff = x - y;
  return diff <= tolerance && -diff <= tolerance;
}

#endif  // TESSERACT_CCUTIL_HOST_H_
