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

#include <cinttypes>    // PRId32, ...
#include <cstdint>      // int32_t, ...

// definitions of portable data types (numbers and characters)
typedef int8_t inT8;
typedef uint8_t uinT8;
typedef int16_t inT16;
typedef uint16_t uinT16;
typedef int32_t inT32;
typedef uint32_t uinT32;
typedef int64_t inT64;
typedef uint64_t uinT64;
typedef float FLOAT32;
typedef double FLOAT64;
typedef unsigned char BOOL8;

#if defined(_WIN32)

/* MinGW defines the standard PRI... macros, but MSVS doesn't. */

#if !defined(PRId32)
# define PRId32 "d"
#endif

#if !defined(PRId64)
# define PRId64 "I64d"
#endif

#endif /* _WIN32 */

#define MAX_INT8  0x7f
#define MAX_INT16 0x7fff
#define MAX_INT32 0x7fffffff
#define MAX_UINT8 0xff
#define MAX_UINT16  0xffff
#define MAX_UINT32  0xffffffff
#define MAX_FLOAT32 std::numeric_limits<float>::max()

#define MIN_INT8 static_cast<inT8>(0x80)
#define MIN_INT16 static_cast<inT16>(0x8000)
#define MIN_INT32 static_cast<inT32>(0x80000000)
#define MIN_UINT8 0x00
#define MIN_UINT16  0x0000
#define MIN_UINT32  0x00000000
// Minimum positive value ie 1e-37ish.
#define MIN_FLOAT32 std::numeric_limits<float>::min()

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
