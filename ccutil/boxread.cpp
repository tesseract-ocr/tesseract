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

#include "mfcpch.h"
#include <string.h>
#include "boxread.h"
#include "unichar.h"
#include "tprintf.h"

bool read_next_box(FILE* box_file, char* utf8_str,
                   int* x_min, int* y_min, int* x_max, int* y_max) {
  return read_next_box(-1, box_file, utf8_str,
                       x_min, y_min, x_max, y_max);
}

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
    /* Check for blank lines in box file */
    while (*buffptr == ' ' || *buffptr == '\t')
      buffptr++;
    if (*buffptr != '\0') {
      count = sscanf(buffptr, "%s %d %d %d %d %d",
                     uch, x_min, y_min, x_max, y_max, &page);
      if (count != 6) {
        page = 0;
        count = sscanf(buffptr, "%s %d %d %d %d",
                       uch, x_min, y_min, x_max, y_max);
      }
      if (target_page >= 0 && target_page != page)
        continue;  // Not on the appropriate page.
      if (count == 5) {
        // Validate UTF8 by making unichars with it.
        int used = 0;
        int uch_len = strlen(uch);
        while (used < uch_len) {
          UNICHAR ch(uch + used, uch_len - used);
          int new_used = ch.utf8_len();
          if (new_used == 0) {
            tprintf("Bad utf-8 char starting with 0x%x at line %d, col %d, \n",
                    uch[used], used + 1, line);
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
      if (count < 5) {
        tprintf("Box file format error on line %i ignored\n", line);
      } else {
        strcpy(utf8_str, uch);
        return true;             //read a box ok
      }
    }
  }
  fclose(box_file);
  line = 0;
  return false;                  //EOF
}
