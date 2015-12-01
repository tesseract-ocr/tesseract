///////////////////////////////////////////////////////////////////////
// File:        mathfix.h
// Description: Implement missing math functions
// Author:      zdenop
// Created:     Fri Feb 03 06:45:06 CET 2012
//
// (C) Copyright 2012, Google Inc.
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

#ifndef VS2008_INCLUDE_MATHFIX_H_
#define VS2008_INCLUDE_MATHFIXT_H_

#ifndef _MSC_VER
#error "Use this header only with Microsoft Visual C++ compilers!"
#endif

#include <math.h>
#include <float.h>  // for _isnan(), _finite() on VC++

#if _MSC_VER < 1800
#define isnan(x) _isnan(x)
#define isinf(x) (!_finite(x))
#define fmax max //VC++ does not implement all the provisions of C99 Standard
#define round(x) roundf(x)
inline float roundf(float num) { return num > 0 ? floorf(num + 0.5f) : ceilf(num - 0.5f); }
#endif

#endif  // VS2008_INCLUDE_MATHFIXT_H_
