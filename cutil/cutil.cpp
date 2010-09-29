/* -*-C-*-
 ********************************************************************************
 *
 * File:         cutil.c
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
 ********************************************************************************
Revision 1.1  2007/02/02 23:39:07  theraysmith
Fixed portability issues

Revision 1.1.1.1  2004/02/20 19:39:06  slumos
Import original HP distribution

 * Revision 1.3  90/03/06  15:39:10  15:39:10  marks (Mark Seaman)
 * Look for correct file of <malloc.h> or <stdlib.h>
 *
 * Revision 1.2  90/01/15  13:02:13  13:02:13  marks (Mark Seaman)
 * Added memory allocator (*allocate) and (*deallocate)
 *
 * Revision 1.1  89/10/09  14:58:29  14:58:29  marks (Mark Seaman)
 * Initial revision
 **/

#include "cutil.h"
#include "tprintf.h"
#include "callcpp.h"

#include <stdlib.h>

#define RESET_COUNT 2000

/**********************************************************************
 * long_rand
 *
 *  Return a long random number whose value is less than limit.  Do this
 *  by calling the standard cheepo random number generator and reseting
 *  it pretty often.
 **********************************************************************/
long long_rand(long limit) {
#if RAND_MAX < 0x1000000
  static long seed;

  long num;
  num = (long) rand () << 16;
  num |= rand () & 0xffff;
  seed ^= num;
  long result = num % limit;
  while (result < 0) {
    result += limit;
  }
  return result;
#else
  return (long)((double)limit * rand()/(RAND_MAX + 1.0));
#endif
}


/**********************************************************************
 * open_file
 *
 *  Open a file for reading or writing.  If the file name parameter is
 *  NULL use stdin (or stdout) for the file.  If the file can not be
 *  opened then call the error routine.
 **********************************************************************/
FILE *open_file(const char *filename, const char *mode) {
  FILE *thisfile = NULL;
  if ((thisfile = fopen (filename, mode)) == NULL) {
    tprintf ("Could not open file, %s\n", filename);
    exit (1);
  }
  return (thisfile);
}

/// Check whether the file exists
bool exists_file(const char *filename) {
  bool exists = false;
  FILE *f = NULL;
  if ((f = fopen(filename, "r")) != NULL) {
    fclose(f);
    exists = true;
  }
  return exists;
}
