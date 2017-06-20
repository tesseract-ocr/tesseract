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

#ifndef   __HOST__
#define   __HOST__

#include "platform.h"
/* _WIN32 */
#ifdef _WIN32
#include <windows.h>
#include <winbase.h>             // winbase.h contains windows.h
#endif

#include <cstdint>      // int32_t, ...

// definitions of portable data types (numbers and characters)
typedef SIGNED char inT8;
typedef unsigned char uinT8;
typedef short inT16;
typedef unsigned short uinT16;
typedef int inT32;
typedef unsigned int uinT32;
#if (_MSC_VER >= 1200)            //%%% vkr for VC 6.0
typedef INT64 inT64;
typedef UINT64 uinT64;
#else
typedef long long int inT64;
typedef unsigned long long int uinT64;
#endif                           //%%% vkr for VC 6.0
typedef float FLOAT32;
typedef double FLOAT64;
typedef unsigned char BOOL8;

#define INT32FORMAT "%d"
#define INT64FORMAT "%lld"

#define MAX_INT8  0x7f
#define MAX_INT16 0x7fff
#define MAX_INT32 0x7fffffff
#define MAX_UINT8 0xff
#define MAX_UINT16  0xffff
#define MAX_UINT32  0xffffffff
#define MAX_FLOAT32 ((float)3.40282347e+38)

#define MIN_INT8  0x80
#define MIN_INT16 0x8000
#define MIN_INT32 static_cast<int>(0x80000000)
#define MIN_UINT8 0x00
#define MIN_UINT16  0x0000
#define MIN_UINT32  0x00000000
#define MIN_FLOAT32 ((float)1.17549435e-38)

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

#endif
