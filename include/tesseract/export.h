// SPDX-License-Identifier: Apache-2.0
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

#ifndef TESSERACT_PLATFORM_H_
#define TESSERACT_PLATFORM_H_

#ifndef TESS_API
#  if defined(_WIN32) || defined(__CYGWIN__)
#    if defined(TESS_EXPORTS)
#      define TESS_API __declspec(dllexport)
#    elif defined(TESS_IMPORTS)
#      define TESS_API __declspec(dllimport)
#    else
#      define TESS_API
#    endif
#  else
#    if defined(TESS_EXPORTS) || defined(TESS_IMPORTS)
#      define TESS_API __attribute__((visibility("default")))
#    else
#      define TESS_API
#    endif
#  endif
#endif

#endif // TESSERACT_PLATFORM_H_
