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

#ifndef TESSERACT_CCUTIL_BOXREAD_H__
#define TESSERACT_CCUTIL_BOXREAD_H__

#include <stdio.h>

// Size of buffer used to read a line from a box file.
const int kBoxReadBufSize = 256;

// read_next_box factors out the code to interpret a line of a box
// file so that applybox and unicharset_extractor interpret the same way.
// This function returns the next valid box file utf8 string and coords
// and returns true, or false on eof (and closes the file).
// If ignores the uft8 file signature, checks for valid utf-8 and allows
// space or tab between fields.
// utf8_str must be at least kBoxReadBufSize in length.
// If there are page numbers in the file, it reads them all.
bool read_next_box(FILE* box_file, char* utf8_str,
                   int* x_min, int* y_min, int* x_max, int* y_max);
// As read_next_box above, but get a specific page number. (0-based)
// Use -1 to read any page number. Files without page number all
// read as if they are page 0.
bool read_next_box(int page, FILE* box_file, char* utf8_str,
                   int* x_min, int* y_min, int* x_max, int* y_max);

#endif  // TESSERACT_CCUTIL_BOXREAD_H__
