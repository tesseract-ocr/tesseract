/**********************************************************************
 * File:        errcode.h  (Formerly error.h)
 * Description: Header file for generic error handler class
 * Author:      Ray Smith
 * Created:     Tue May  1 16:23:36 BST 1990
 *
 * (C) Copyright 1990, Hewlett-Packard Ltd.
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

#ifndef           ERRCODE_H
#define           ERRCODE_H

#include          "host.h"

/*Control parameters for error()*/
enum TessErrorLogCode {
  DBG =     -1,           /*log without alert */
  TESSLOG =  0,            /*alert user */
  TESSEXIT = 1,            /*exit after erro */
  ABORT =    2            /*abort after error */
};

/* Explicit Error Abort codes */
#define NO_ABORT_CODE      0
#define LIST_ABORT      1
#define MEMORY_ABORT    2
#define FILE_ABORT      3

/* Location of code at error codes Reserve 0..2 (status codes 0..23 for UNLV)*/
#define LOC_UNUSED0        0
#define LOC_UNUSED1        1
#define LOC_UNUSED2        2
#define LOC_INIT      3
#define LOC_EDGE_PROG   4
#define LOC_TEXT_ORD_ROWS 5
#define LOC_TEXT_ORD_WORDS  6
#define LOC_PASS1     7
#define LOC_PASS2     8
/* Reserve up to 8..13 for adding subloc 0/3 plus subsubloc 0/1/2 */
#define LOC_FUZZY_SPACE   14
/* Reserve up to 14..20 for adding subloc 0/3 plus subsubloc 0/1/2 */
#define LOC_MM_ADAPT    21
#define LOC_DOC_BLK_REJ   22
#define LOC_WRITE_RESULTS 23
#define LOC_ADAPTIVE    24
/* DONT DEFINE ANY LOCATION > 31 !!! */

/* Sub locatation determines whether pass2 was in normal mode or fix xht mode*/
#define SUBLOC_NORM     0
#define SUBLOC_FIX_XHT    3

/* Sub Sub locatation determines whether match_word_pass2 was in Tess
  matcher, NN matcher or somewhere else */

#define SUBSUBLOC_OTHER   0
#define SUBSUBLOC_TESS    1
#define SUBSUBLOC_NN    2

class TESS_API ERRCODE {           // error handler class
  const char *message;           // error message
 public:
  void error(                  // error print function
    const char *caller,        // function location
    TessErrorLogCode action,   // action to take
    const char *format, ...    // fprintf format
  ) const;
  ERRCODE(const char *string) {
    message = string;
  }                            // initialize with string
};

const ERRCODE ASSERT_FAILED = "Assert failed";

#define ASSERT_HOST(x) if (!(x))                                        \
  {                                                                     \
    ASSERT_FAILED.error(#x, ABORT, "in file %s, line %d",               \
                        __FILE__, __LINE__);                            \
  }

#ifdef _MSC_VER
#define ASSERT_HOST_MSG(x, msg, ...) if (!(x))                            \
  {                                                                     \
    tprintf(msg);                                                       \
    ASSERT_FAILED.error(#x, ABORT, "in file %s, line %d",               \
                        __FILE__, __LINE__);                            \
  }
#else
#define ASSERT_HOST_MSG(x, msg...) if (!(x))                            \
  {                                                                     \
    tprintf(msg);                                                       \
    ASSERT_FAILED.error(#x, ABORT, "in file %s, line %d",               \
                        __FILE__, __LINE__);                            \
  }
#endif

void signal_exit(int signal_code);

void set_global_loc_code(int loc_code);

void set_global_subloc_code(int loc_code);

void set_global_subsubloc_code(int loc_code);
#endif
