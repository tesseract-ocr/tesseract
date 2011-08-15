///////////////////////////////////////////////////////////////////////
// File:        platform.h
// Description: Place holder
// Author:
// Created:
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

#ifndef TESSERACT_CCUTIL_PLATFORM_H__
#define TESSERACT_CCUTIL_PLATFORM_H__

#define DLLSYM
#ifdef __MSW32__
#ifdef __GNUC__
#define ultoa _ultoa
typedef struct _BLOB {
  unsigned int cbSize;
  char *pBlobData;
} BLOB, *LPBLOB;
#endif  /* __GNUC__ */
#define SIGNED
#define snprintf _snprintf
#if (_MSC_VER <= 1400)
#define vsnprintf _vsnprintf
#endif /* __MSW32__ */
#else
#define __UNIX__
#include <limits.h>
#ifndef PATH_MAX
#define MAX_PATH 4096
#else
#define MAX_PATH PATH_MAX
#endif
#define SIGNED signed
#endif

#endif  // TESSERACT_CCUTIL_PLATFORM_H__
