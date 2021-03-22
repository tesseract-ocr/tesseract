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

#include <tesseract/export.h>

#include <climits>
#include <limits>

/* _WIN32 */
#ifdef _WIN32
#  ifndef NOMINMAX
#    define NOMINMAX
#  endif /* NOMINMAX */
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#  include <windows.h>
#  undef min
#  undef max
#endif // _WIN32

#ifndef _WIN32
#  ifndef PATH_MAX
#    define MAX_PATH 4096
#  else
#    define MAX_PATH PATH_MAX
#  endif
#endif

namespace tesseract {

// Return true if x is within tolerance of y
template <class T>
bool NearlyEqual(T x, T y, T tolerance) {
  T diff = x - y;
  return diff <= tolerance && -diff <= tolerance;
}

} // namespace tesseract

#endif // TESSERACT_CCUTIL_HOST_H_
