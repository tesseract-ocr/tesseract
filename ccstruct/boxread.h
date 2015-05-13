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
#include "genericvector.h"
#include "strngs.h"

class STRING;
class TBOX;

// Size of buffer used to read a line from a box file.
const int kBoxReadBufSize = 1024;

// Open the boxfile based on the given image filename.
// Returns NULL if the box file cannot be opened.
FILE* OpenBoxFile(const STRING& fname);

// Reads all boxes from the given filename.
// Reads a specific target_page number if >= 0, or all pages otherwise.
// Skips blanks if skip_blanks is true.
// The UTF-8 label of the box is put in texts, and the full box definition as
// a string is put in box_texts, with the corresponding page number in pages.
// Each of the output vectors is optional (may be NULL).
// Returns false if no boxes are found.
bool ReadAllBoxes(int target_page, bool skip_blanks, const STRING& filename,
                  GenericVector<TBOX>* boxes,
                  GenericVector<STRING>* texts,
                  GenericVector<STRING>* box_texts,
                  GenericVector<int>* pages);

// Reads all boxes from the string. Otherwise, as ReadAllBoxes.
bool ReadMemBoxes(int target_page, bool skip_blanks, const char* box_data,
                  GenericVector<TBOX>* boxes,
                  GenericVector<STRING>* texts,
                  GenericVector<STRING>* box_texts,
                  GenericVector<int>* pages);

// Returns the box file name corresponding to the given image_filename.
STRING BoxFileName(const STRING& image_filename);

// ReadNextBox factors out the code to interpret a line of a box
// file so that applybox and unicharset_extractor interpret the same way.
// This function returns the next valid box file utf8 string and coords
// and returns true, or false on eof (and closes the file).
// It ignores the utf8 file signature ByteOrderMark (U+FEFF=EF BB BF), checks
// for valid utf-8 and allows space or tab between fields.
// utf8_str is set with the unichar string, and bounding box with the box.
// If there are page numbers in the file, it reads them all.
bool ReadNextBox(int *line_number, FILE* box_file,
                 STRING* utf8_str, TBOX* bounding_box);
// As ReadNextBox above, but get a specific page number. (0-based)
// Use -1 to read any page number. Files without page number all
// read as if they are page 0.
bool ReadNextBox(int target_page, int *line_number, FILE* box_file,
                 STRING* utf8_str, TBOX* bounding_box);

// Parses the given box file string into a page_number, utf8_str, and
// bounding_box. Returns true on a successful parse.
bool ParseBoxFileStr(const char* boxfile_str, int* page_number,
                     STRING* utf8_str, TBOX* bounding_box);

// Creates a box file string from a unichar string, TBOX and page number.
void MakeBoxFileStr(const char* unichar_str, const TBOX& box, int page_num,
                    STRING* box_str);

#endif  // TESSERACT_CCUTIL_BOXREAD_H__
