/**********************************************************************
 * File:        boxread.h
 * Description: Read data from a box file.
 * Author:      Ray Smith
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

#ifndef TESSERACT_CCUTIL_BOXREAD_H_
#define TESSERACT_CCUTIL_BOXREAD_H_

#include <cstdio> // for FILE
#include <string> // for std::string
#include <vector> // for std::vector

#include <tesseract/export.h> // for TESS_API

namespace tesseract {

class TBOX;

// Size of buffer used to read a line from a box file.
const int kBoxReadBufSize = 1024;

// Open the boxfile based on the given image filename.
// Returns nullptr if the box file cannot be opened.
TESS_API
FILE *OpenBoxFile(const char *filename);

// Reads all boxes from the given filename.
// Reads a specific target_page number if >= 0, or all pages otherwise.
// Skips blanks if skip_blanks is true.
// The UTF-8 label of the box is put in texts, and the full box definition as
// a string is put in box_texts, with the corresponding page number in pages.
// Each of the output vectors is optional (may be nullptr).
// Returns false if no boxes are found.
bool ReadAllBoxes(int target_page, bool skip_blanks, const char *filename, std::vector<TBOX> *boxes,
                  std::vector<std::string> *texts, std::vector<std::string> *box_texts,
                  std::vector<int> *pages);

// Reads all boxes from the string. Otherwise, as ReadAllBoxes.
// continue_on_failure allows reading to continue even if an invalid box is
// encountered and will return true if it succeeds in reading some boxes.
// It otherwise gives up and returns false on encountering an invalid box.
TESS_API
bool ReadMemBoxes(int target_page, bool skip_blanks, const char *box_data, bool continue_on_failure,
                  std::vector<TBOX> *boxes, std::vector<std::string> *texts,
                  std::vector<std::string> *box_texts, std::vector<int> *pages);

// ReadNextBox factors out the code to interpret a line of a box
// file so that applybox and unicharset_extractor interpret the same way.
// This function returns the next valid box file utf8 string and coords
// and returns true, or false on eof (and closes the file).
// It ignores the utf8 file signature ByteOrderMark (U+FEFF=EF BB BF), checks
// for valid utf-8 and allows space or tab between fields.
// utf8_str is set with the unichar string, and bounding box with the box.
// If there are page numbers in the file, it reads them all.
TESS_API
bool ReadNextBox(int *line_number, FILE *box_file, std::string &utf8_str, TBOX *bounding_box);
// As ReadNextBox above, but get a specific page number. (0-based)
// Use -1 to read any page number. Files without page number all
// read as if they are page 0.
TESS_API
bool ReadNextBox(int target_page, int *line_number, FILE *box_file, std::string &utf8_str,
                 TBOX *bounding_box);

// Parses the given box file string into a page_number, utf8_str, and
// bounding_box. Returns true on a successful parse.
TESS_API
bool ParseBoxFileStr(const char *boxfile_str, int *page_number, std::string &utf8_str,
                     TBOX *bounding_box);

// Creates a box file string from a unichar string, TBOX and page number.
TESS_API
void MakeBoxFileStr(const char *unichar_str, const TBOX &box, int page_num, std::string &box_str);

} // namespace tesseract

#endif // TESSERACT_CCUTIL_BOXREAD_H_
