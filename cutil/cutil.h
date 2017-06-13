/* -*-C-*-
 ********************************************************************************
 *
 * File:         cutil.h
 * Description:  General utility functions
 * Author:       Mark Seaman, SW Productivity
 * Created:      Fri Oct 16 14:37:00 1987
 * Modified:     Wed Dec  5 15:40:26 1990 (Mark Seaman) marks@hpgrlt
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

*/

#ifndef CUTILH
#define CUTILH

/*----------------------------------------------------------------------
                     I n c l u d e s
----------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "host.h"
#include "tprintf.h"

/*----------------------------------------------------------------------
                      T y p e s
----------------------------------------------------------------------*/
#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define CHARS_PER_LINE 500

#if defined(__STDC__) || defined(__cplusplus)
# define _ARGS(s) s
#else
# define _ARGS(s) ()
#endif

//typedef int (*int_proc)               (void);
typedef void (*void_proc) (...);
typedef void *(*void_star_proc) _ARGS ((...));

typedef int (*int_void) (void);
typedef void (*void_void) (void);
typedef int (*int_compare) (void *, void *);
typedef void (*void_dest) (void *);

/*----------------------------------------------------------------------
                     M a c r o s
----------------------------------------------------------------------*/
/**********************************************************************
 * new_line
 *
 * Print a new line character on stdout.
 **********************************************************************/

#define new_line()  \
  tprintf("\n")

/**********************************************************************
 * print_string
 *
 * Print a string on stdout.
 **********************************************************************/

#define print_string(str)  \
  printf ("%s\n", str)

/*----------------------------------------------------------------------
                     F u n c t i o n s
----------------------------------------------------------------------*/
long long_rand(long limit);

FILE *open_file(const char *filename, const char *mode);

bool exists_file(const char *filename);

/* util.c
long long_rand
  _ARGS ((long limit));

FILE *open_file
   _ARGS((char *filename,
    char *mode));

#undef _ARGS
*/
#include "cutil_class.h"
#endif
