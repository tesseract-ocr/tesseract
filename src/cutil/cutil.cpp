/* -*-C-*-
 ******************************************************************************
 *
 * File:         cutil.cpp
 * Description:  General utility functions
 * Author:       Mark Seaman, SW Productivity
 * Created:      Fri Oct 16 14:37:00 1987
 * Modified:     Wed Jun  6 16:29:17 1990 (Mark Seaman) marks@hpgrlt
 * Language:     C
 * Package:      N/A
 * Status:       Reusable Software Component
 *
 * (c) Copyright 1987, Hewlett-Packard Company.
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
 ******************************************************************************
 **/

#include "cutil.h"
#include "tprintf.h"
#include "callcpp.h"

#include <cstdlib>

#define RESET_COUNT 2000

/**********************************************************************
 * open_file
 *
 *  Open a file for reading or writing.  If the file name parameter is
 *  nullptr use stdin (or stdout) for the file.  If the file can not be
 *  opened then call the error routine.
 **********************************************************************/
FILE *open_file(const char *filename, const char *mode) {
  FILE *thisfile = nullptr;
  if ((thisfile = fopen (filename, mode)) == nullptr) {
    tprintf ("Could not open file, %s\n", filename);
    exit (1);
  }
  return (thisfile);
}

/// Check whether the file exists
bool exists_file(const char *filename) {
  bool exists = false;
  FILE *f = nullptr;
  if ((f = fopen(filename, "rb")) != nullptr) {
    fclose(f);
    exists = true;
  }
  return exists;
}
