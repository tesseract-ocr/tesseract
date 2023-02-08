/**********************************************************************
 * File:        boxread.cpp
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

#include "boxread.h"

#include "errcode.h" // for ERRCODE, TESSEXIT
#include "fileerr.h" // for CANTOPENFILE
#include "rect.h"    // for TBOX
#include "tprintf.h" // for tprintf

#include <tesseract/unichar.h> // for UNICHAR
#include "helpers.h"           // for chomp_string

#include <climits> // for INT_MAX
#include <cstring> // for strchr, strcmp
#include <fstream> // for std::ifstream
#include <locale>  // for std::locale::classic
#include <sstream> // for std::stringstream
#include <string>  // for std::string

namespace tesseract {

// Special char code used to identify multi-blob labels.
static const char *kMultiBlobLabelCode = "WordStr";

// Returns the box file name corresponding to the given image_filename.
static std::string BoxFileName(const char *image_filename) {
  std::string box_filename = image_filename;
  size_t length = box_filename.length();
  std::string last = (length > 8) ? box_filename.substr(length - 8) : "";
  if (last == ".bin.png" || last == ".nrm.png" || last == ".raw.png") {
    box_filename.resize(length - 8);
  } else {
    size_t lastdot = box_filename.find_last_of('.');
    if (lastdot < length) {
      box_filename.resize(lastdot);
    }
  }
  box_filename += ".box";
  return box_filename;
}

// Open the boxfile based on the given image filename.
FILE *OpenBoxFile(const char *fname) {
  std::string filename = BoxFileName(fname);
  FILE *box_file = nullptr;
  if (!(box_file = fopen(filename.c_str(), "rb"))) {
    CANTOPENFILE.error("read_next_box", TESSEXIT, "Can't open box file %s", filename.c_str());
    tprintf("Can't open box file %s", filename.c_str());
  }
  return box_file;
}

// Reads all boxes from the given filename.
// Reads a specific target_page number if >= 0, or all pages otherwise.
// Skips blanks if skip_blanks is true.
// The UTF-8 label of the box is put in texts, and the full box definition as
// a string is put in box_texts, with the corresponding page number in pages.
// Each of the output vectors is optional (may be nullptr).
// Returns false if no boxes are found.
bool ReadAllBoxes(int target_page, bool skip_blanks, const char *filename, std::vector<TBOX> *boxes,
                  std::vector<std::string> *texts, std::vector<std::string> *box_texts,
                  std::vector<int> *pages) {
  std::ifstream input(BoxFileName(filename).c_str(), std::ios::in | std::ios::binary);
  if (input.fail()) {
    tprintf("Cannot read box data from '%s'.\n", BoxFileName(filename).c_str());
    tprintf("Does it exists?\n");
    return false;
  }
  std::vector<char> box_data(std::istreambuf_iterator<char>(input), {});
  if (box_data.empty()) {
    tprintf("No box data found in '%s'.\n", BoxFileName(filename).c_str());
    return false;
  }
  // Convert the array of bytes to a string, so it can be used by the parser.
  box_data.push_back('\0');
  return ReadMemBoxes(target_page, skip_blanks, &box_data[0],
                      /*continue_on_failure*/ true, boxes, texts, box_texts, pages);
}

// Reads all boxes from the string. Otherwise, as ReadAllBoxes.
bool ReadMemBoxes(int target_page, bool skip_blanks, const char *box_data, bool continue_on_failure,
                  std::vector<TBOX> *boxes, std::vector<std::string> *texts,
                  std::vector<std::string> *box_texts, std::vector<int> *pages) {
  std::string box_str(box_data);
  std::vector<std::string> lines = split(box_str, '\n');
  if (lines.empty()) {
    return false;
  }
  int num_boxes = 0;
  for (auto &line : lines) {
    int page = 0;
    std::string utf8_str;
    TBOX box;
    if (!ParseBoxFileStr(line.c_str(), &page, utf8_str, &box)) {
      if (continue_on_failure) {
        continue;
      } else {
        return false;
      }
    }
    if (skip_blanks && (utf8_str == " " || utf8_str == "\t")) {
      continue;
    }
    if (target_page >= 0 && page != target_page) {
      continue;
    }
    if (boxes != nullptr) {
      boxes->push_back(box);
    }
    if (texts != nullptr) {
      texts->push_back(utf8_str);
    }
    if (box_texts != nullptr) {
      std::string full_text;
      MakeBoxFileStr(utf8_str.c_str(), box, target_page, full_text);
      box_texts->push_back(full_text);
    }
    if (pages != nullptr) {
      pages->push_back(page);
    }
    ++num_boxes;
  }
  return num_boxes > 0;
}

// TODO(rays) convert all uses of ReadNextBox to use the new ReadAllBoxes.
// Box files are used ONLY DURING TRAINING, but by both processes of
// creating tr files with tesseract, and unicharset_extractor.
// ReadNextBox factors out the code to interpret a line of a box
// file so that applybox and unicharset_extractor interpret the same way.
// This function returns the next valid box file utf8 string and coords
// and returns true, or false on eof (and closes the file).
// It ignores the utf8 file signature ByteOrderMark (U+FEFF=EF BB BF), checks
// for valid utf-8 and allows space or tab between fields.
// utf8_str is set with the unichar string, and bounding box with the box.
// If there are page numbers in the file, it reads them all.
bool ReadNextBox(int *line_number, FILE *box_file, std::string &utf8_str, TBOX *bounding_box) {
  return ReadNextBox(-1, line_number, box_file, utf8_str, bounding_box);
}

// As ReadNextBox above, but get a specific page number. (0-based)
// Use -1 to read any page number. Files without page number all
// read as if they are page 0.
bool ReadNextBox(int target_page, int *line_number, FILE *box_file, std::string &utf8_str,
                 TBOX *bounding_box) {
  int page = 0;
  char buff[kBoxReadBufSize]; // boxfile read buffer
  char *buffptr = buff;

  while (fgets(buff, sizeof(buff) - 1, box_file)) {
    (*line_number)++;

    buffptr = buff;
    const auto *ubuf = reinterpret_cast<const unsigned char *>(buffptr);
    if (ubuf[0] == 0xef && ubuf[1] == 0xbb && ubuf[2] == 0xbf) {
      buffptr += 3; // Skip unicode file designation.
    }
    // Check for blank lines in box file
    if (*buffptr == '\n' || *buffptr == '\0') {
      continue;
    }
    // Skip blank boxes.
    if (*buffptr == ' ' || *buffptr == '\t') {
      continue;
    }
    if (*buffptr != '\0') {
      if (!ParseBoxFileStr(buffptr, &page, utf8_str, bounding_box)) {
        tprintf("Box file format error on line %i; ignored\n", *line_number);
        continue;
      }
      if (target_page >= 0 && target_page != page) {
        continue; // Not on the appropriate page.
      }
      return true; // Successfully read a box.
    }
  }
  fclose(box_file);
  return false; // EOF
}

// Parses the given box file string into a page_number, utf8_str, and
// bounding_box. Returns true on a successful parse.
// The box file is assumed to contain box definitions, one per line, of the
// following format for blob-level boxes:
//   <UTF8 str> <left> <bottom> <right> <top> <page id>
// and for word/line-level boxes:
//   WordStr <left> <bottom> <right> <top> <page id> #<space-delimited word str>
// See applyybox.cpp for more information.
bool ParseBoxFileStr(const char *boxfile_str, int *page_number, std::string &utf8_str,
                     TBOX *bounding_box) {
  *bounding_box = TBOX(); // Initialize it to empty.
  utf8_str = "";
  char uch[kBoxReadBufSize];
  const char *buffptr = boxfile_str;
  // Read the unichar without messing up on Tibetan.
  // According to issue 253 the utf-8 surrogates 85 and A0 are treated
  // as whitespace by sscanf, so it is more reliable to just find
  // ascii space and tab.
  int uch_len = 0;
  // Skip unicode file designation, if present.
  const auto *ubuf = reinterpret_cast<const unsigned char *>(buffptr);
  if (ubuf[0] == 0xef && ubuf[1] == 0xbb && ubuf[2] == 0xbf) {
    buffptr += 3;
  }
  // Allow a single blank as the UTF-8 string. Check for empty string and
  // then blindly eat the first character.
  if (*buffptr == '\0') {
    return false;
  }
  do {
    uch[uch_len++] = *buffptr++;
  } while (*buffptr != '\0' && *buffptr != ' ' && *buffptr != '\t' &&
           uch_len < kBoxReadBufSize - 1);
  uch[uch_len] = '\0';
  if (*buffptr != '\0') {
    ++buffptr;
  }
  int x_min = INT_MAX;
  int y_min = INT_MAX;
  int x_max = INT_MIN;
  int y_max = INT_MIN;
  *page_number = 0;
  std::stringstream stream(buffptr);
  stream.imbue(std::locale::classic());
  stream >> x_min;
  stream >> y_min;
  stream >> x_max;
  stream >> y_max;
  stream >> *page_number;
  if (x_max < x_min || y_max < y_min) {
    tprintf("Bad box coordinates in boxfile string! %s\n", ubuf);
    return false;
  }
  // Test for long space-delimited string label.
  if (strcmp(uch, kMultiBlobLabelCode) == 0 && (buffptr = strchr(buffptr, '#')) != nullptr) {
    strncpy(uch, buffptr + 1, kBoxReadBufSize - 1);
    uch[kBoxReadBufSize - 1] = '\0'; // Prevent buffer overrun.
    chomp_string(uch);
    uch_len = strlen(uch);
  }
  // Validate UTF8 by making unichars with it.
  int used = 0;
  while (used < uch_len) {
    tesseract::UNICHAR ch(uch + used, uch_len - used);
    int new_used = ch.utf8_len();
    if (new_used == 0) {
      tprintf("Bad UTF-8 str %s starts with 0x%02x at col %d\n", uch + used, uch[used], used + 1);
      return false;
    }
    used += new_used;
  }
  utf8_str = uch;
  if (x_min > x_max) {
    std::swap(x_min, x_max);
  }
  if (y_min > y_max) {
    std::swap(y_min, y_max);
  }
  bounding_box->set_to_given_coords(x_min, y_min, x_max, y_max);
  return true; // Successfully read a box.
}

// Creates a box file string from a unichar string, TBOX and page number.
void MakeBoxFileStr(const char *unichar_str, const TBOX &box, int page_num, std::string &box_str) {
  box_str = unichar_str;
  box_str += " " + std::to_string(box.left());
  box_str += " " + std::to_string(box.bottom());
  box_str += " " + std::to_string(box.right());
  box_str += " " + std::to_string(box.top());
  box_str += " " + std::to_string(page_num);
}

} // namespace tesseract
