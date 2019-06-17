// Copyright 2006 Google Inc.
// All Rights Reserved.
// Author: renn
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef TESSERACT_CCUTIL_SCANUTILS_H_
#define TESSERACT_CCUTIL_SCANUTILS_H_

#include <cstdio>       // for FILE

/**
 * fscanf variant to ensure correct reading regardless of locale.
 *
 * tfscanf parse a file stream according to the given format. See the fscanf
 * manpage for more information, as this function attempts to mimic its
 * behavior.
 *
 * @note Note that scientific floating-point notation is not supported.
 *
 */
int tfscanf(FILE* stream, const char *format, ...);

#endif  // TESSERACT_CCUTIL_SCANUTILS_H_
