/**********************************************************************
 * File:        boxread.cpp
 * Description: Read data from a box file.
 * Author:		Ray Smith
 * Created:		Fri Aug 24 17:47:23 PDT 2007
 *
 * (C) Copyright 2007, Google Inc.
 ** Licensed under the Apache License, Version 2.0 (the "License");
 ** you may not use this file except in compliance with the License.
 ** You may obtain a copy of the License at
 ** http://www.apache.org/licenses/LICENSE-2.0
 ** Unless required by applicable law or agreed to in writing, software
 ** distributed under the License is distributed on an "AS IS" BASIS,
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 ** See the License for the specific language governing permissions and
 ** limitations under the License.
 *
 **********************************************************************/

#ifndef THIRD_PARTY_TESSERACT_CCUTIL_BOXREAD_H__
#define THIRD_PARTY_TESSERACT_CCUTIL_BOXREAD_H__

#include <stdio.h>

const int kBufSize = 256;
// read_next_box factors out the code to interpret a line of a box
// file so that applybox and unicharset_extractor interpert the same way.
// This function returns the next valid box file utf8 string and coords
// and returns true, or false on eof (and closes the file).
// If ignores the uft8 file signature, checks for valid utf-8 and allows
// space or tab between fields.
// utf8_str must be at least kBufSize in length.
bool read_next_box(FILE* box_file, char* utf8_str,
                   int* x_min, int* y_min, int* x_max, int* y_max);

#endif  // THIRD_PARTY_TESSERACT_CCUTIL_BOXREAD_H__

