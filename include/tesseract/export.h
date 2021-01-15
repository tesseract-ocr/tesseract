///////////////////////////////////////////////////////////////////////
// File:        export.h
// Description: Place holder
//
// (C) Copyright 2006, Google Inc.
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
///////////////////////////////////////////////////////////////////////

#ifndef TESSERACT_PLATFORM_H_
#define TESSERACT_PLATFORM_H_

#if !defined(__GNUC__)
# define __attribute__(attr) /* compiler without support for __attribute__ */
#endif

#ifndef TESS_API
# if defined(_WIN32) || defined(__CYGWIN__)
#  if defined(TESS_EXPORTS)
#    define TESS_API __declspec(dllexport)
#  elif defined(TESS_IMPORTS)
#    define TESS_API __declspec(dllimport)
#  else
#    define TESS_API
#  endif
# else
#  if defined(TESS_EXPORTS) || defined(TESS_IMPORTS)
#    define TESS_API __attribute__((visibility("default")))
#  else
#    define TESS_API
#  endif
# endif
#endif

#if !defined(__GNUC__)
# define __attribute__(attr) /* compiler without support for __attribute__ */
#endif

/* GCC can do type checking of printf strings */
#ifdef __printflike
#  define TS_PRINTFLIKE(F, V) __printflike(F, V)
#else
#  if defined(__GNUC__) && \
      (__GNUC__ > 2 || __GNUC__ == 2 && __GNUC_MINOR__ >= 7)
#    define TS_PRINTFLIKE(F, V) __attribute__((format(gnu_printf, F, V)))
#  else
#    define TS_PRINTFLIKE(F, V)
#  endif
#endif
/* https://stackoverflow.com/questions/2354784/attribute-formatprintf-1-2-for-msvc/6849629#6849629 */
#undef FZ_FORMAT_STRING
#if _MSC_VER >= 1400
#  include <sal.h>
#  if _MSC_VER > 1400
#    define TS_FORMAT_STRING(p) _Printf_format_string_ p
#  else
#    define TS_FORMAT_STRING(p) __format_string p
#  endif
#else
#  define TS_FORMAT_STRING(p) p
#endif /* _MSC_VER */


#endif  // TESSERACT_PLATFORM_H_
