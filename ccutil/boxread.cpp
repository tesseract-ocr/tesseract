/**********************************************************************
 * File:        boxread.cpp
 * Description: Read data from a box file.
 * Author:      Ray Smith
 * Created:     Fri Aug 24 17:47:23 PDT 2007
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

#include "mfcpch.h"
#include <string.h>
#include "boxread.h"
#include "unichar.h"
#include "tprintf.h"

// Box files are used ONLY DURING TRAINING, but by both processes of
// creating tr files with tesseract, and unicharset_extractor.
// read_next_box factors out the code to interpret a line of a box
// file so that applybox and unicharset_extractor interpret the same way.
// This function returns the next valid box file utf8 string and coords
// and returns true, or false on eof (and closes the file).
// It ignores the uft8 file signature, checks for valid utf-8 and allows
// space or tab between fields.
// utf8_str must be at least kBoxReadBufSize in length.
// If there are page numbers in the file, it reads them all.
bool read_next_box(FILE* box_file, char* utf8_str,
                   int* x_min, int* y_min, int* x_max, int* y_max) {
  return read_next_box(-1, box_file, utf8_str,
                       x_min, y_min, x_max, y_max);
}

// As read_next_box above, but get a specific page number. (0-based)
// Use -1 to read any page number. Files without page number all
// read as if they are page 0.
bool read_next_box(int target_page, FILE* box_file, char* utf8_str,
                   int* x_min, int* y_min, int* x_max, int* y_max) {
  static int line = 0;
  int count = 0;
  int page = 0;
  char buff[kBoxReadBufSize];                //boxfile read buffer
  char uch[kBoxReadBufSize];
  char *buffptr = buff;

  while (fgets(buff, sizeof(buff) - 1, box_file)) {
    line++;

    buffptr = buff;
    const unsigned char *ubuf = reinterpret_cast<const unsigned char*>(buffptr);
    if (ubuf[0] == 0xef && ubuf[1] == 0xbb && ubuf[2] == 0xbf)
      buffptr += 3;  // Skip unicode file designation.
    // Check for blank lines in box file
    while (*buffptr == ' ' || *buffptr == '\t')
      buffptr++;
    if (*buffptr != '\0') {
      // Read the unichar without messing up on Tibetan.
      int uch_len = 0;
      while (*buffptr != '\0' && *buffptr != ' ' && *buffptr != '\t')
        uch[uch_len++] = *buffptr++;
      uch[uch_len] = '\0';
      if (*buffptr != '\0') ++buffptr;
      count = sscanf(buffptr, "%d %d %d %d %d",
                     x_min, y_min, x_max, y_max, &page);
      if (count != 5) {
        if (target_page <= 0) {
          // If target_page is negative or zero, allow lines with no page number
          page = 0;
          count = sscanf(buffptr, "%d %d %d %d", x_min, y_min, x_max, y_max);
        } else {
          tprintf("Box file format error on line %i; ignored\n", line);
          continue;
        }
      }
      if (target_page >= 0 && target_page != page)
        continue;  // Not on the appropriate page.
      if (count >= 4) {
        // Validate UTF8 by making unichars with it.
        int used = 0;
        while (used < uch_len) {
          UNICHAR ch(uch + used, uch_len - used);
          int new_used = ch.utf8_len();
          if (new_used == 0) {
            tprintf("Bad UTF-8 str %s starts with 0x%02x at line %d, col %d\n",
                    uch + used, uch[used], line, used + 1);
            count = 0;
            break;
          }
          used += new_used;
        }
        if (uch_len > UNICHAR_LEN) {
          tprintf("utf-8 string too long at line %d\n", line);
          count = 0;
        }
      }
      if ((count < 5 && target_page > 0) || (count < 4 && target_page <= 0)) {
        tprintf("Box file format error on line %i ignored\n", line);
      } else {
        strcpy(utf8_str, uch);
        return true;  // Successfully read a box.
      }
    }
  }
  fclose(box_file);
  line = 0;
  return false;                  //EOF
}
