// Copyright 2006 Google Inc.
// All Rights Reserved.
// Author: renn
//
// Contains file io functions (mainly for file parsing), that might not be 
// available, on embedded devices, or that have an incomplete implementation 
// there.
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

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <sys/stat.h>

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

#ifdef EMBEDDED

// Attempts to parse the given file stream s as an integer of the base
// 'base'. Returns the first successfully parsed integer as a uintmax_t, or
// 0, if none was found.
uintmax_t streamtoumax(FILE* s, int base);

// Parse a file stream according to the given format. See the fscanf manpage
// for more information, as this function attempts to mimic its behavior.
// Note that scientific loating-point notation is not supported.
int fscanf(FILE* stream, const char *format, ...);

// Parse a file stream according to the given format. See the fscanf manpage
// for more information, as this function attempts to mimic its behavior.
// Note that scientific loating-point notation is not supported.
int vfscanf(FILE* stream, const char *format, va_list ap);

// Create a file at the specified path. See the creat manpage for more 
// information, as this function attempts to mimic its behavior.
int creat(const char *pathname, mode_t mode);

// Convert the specified C-String to a float. Returns the first parsed float,
// or 0.0 if no floating point value could be found. Note that scientific
// floating-point notation is not supported.
double strtofloat(const char* s);

#endif  // EMBEDDED

#endif  // TESSERACT_CCUTIL_SCANUTILS_H_
