/* -*-C-*-
 ********************************************************************************
 *
 * File:         helpers.h
 * Description:  General utility functions
 * Author:       Daria Antonova
 * Created:      Wed Apr 8 14:37:00 2009
 * Language:     C
 * Package:      N/A
 * Status:       Reusable Software Component
 *
 * (c) Copyright 2009, Google Inc.
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
 ********************************************************************************/

#ifndef TESSERACT_CCUTIL_HELPERS_H_
#define TESSERACT_CCUTIL_HELPERS_H_

// Remove newline (if any) at the end of the string.
inline void chomp_string(char *string) {
  int last_index = strlen(string) - 1;
  if (string[last_index] == '\n') {
    string[last_index] = '\0';
  }
}

// Advance the current pointer of the file if it points to a newline character.
inline void SkipNewline(FILE *file) {
  if (fgetc(file) != '\n') fseek(file, -1, SEEK_CUR);
}

#endif
