/* -*-C-*-
 ********************************************************************************
 *
 * File:        context.h  (Formerly context.h)
 * Description:  Context checking functions
 * Author:       Mark Seaman, OCR Technology
 * Created:      Thu Feb 15 11:18:24 1990
 * Modified:     Tue Jul  9 17:00:38 1991 (Mark Seaman) marks@hpgrlt
 * Language:     C
 * Package:      N/A
 * Status:       Experimental (Do Not Distribute)
 *
 * (c) Copyright 1990, Hewlett-Packard Company.
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
 */

#ifndef CONTEXT_H
#define CONTEXT_H

#include "choices.h"
#include "ratngs.h"

/*----------------------------------------------------------------------
              F u n c t i o n s
----------------------------------------------------------------------*/
namespace tesseract {

class Context {
 public:
  static int case_ok(const WERD_CHOICE &word, const UNICHARSET &unicharset);
};
}  // namespace tesseract

void close_choices();

void write_choice_line();

typedef double (*PROBABILITY_IN_CONTEXT_FUNCTION)(const char* context,
                                                  int context_bytes,
                                                  const char* character,
                                                  int character_bytes);

extern PROBABILITY_IN_CONTEXT_FUNCTION probability_in_context;

extern double def_probability_in_context(const char* context,
                                         int context_bytes,
                                         const char* character,
                                         int character_bytes);

/*
#if defined(__STDC__) || defined(__cplusplus)
# define _ARGS(s) s
#else
# define _ARGS(s) ()
#endif*/

/* context.c
void close_choices
  _ARGS((void));

void fix_quotes
  _ARGS((char *str));

int punctuation_ok
  _ARGS((char *word));

int case_ok
  _ARGS((char *word));

void write_choice_line
  _ARGS((void));

#undef _ARGS
*/
#endif
