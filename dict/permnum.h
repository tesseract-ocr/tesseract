/* -*-C-*-
 ********************************************************************************
 *
 * File:        permnum.h  (Formerly permnum.h)
 * Description:
 * Author:       Mark Seaman, SW Productivity
 * Created:      Fri Oct 16 14:37:00 1987
 * Modified:     Mon May 20 16:30:03 1991 (Mark Seaman) marks@hpgrlt
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
 *********************************************************************************/
#ifndef PERMNUM_H
#define PERMNUM_H

/*----------------------------------------------------------------------
              I n c l u d e s
----------------------------------------------------------------------*/
#include "choicearr.h"

/*----------------------------------------------------------------------
              V a r i a b l e s
----------------------------------------------------------------------*/
#define GOOD_NUMBER     1.1
#define OK_NUMBER     1.4
extern float ok_number;
extern float good_number;

/*----------------------------------------------------------------------
              F u n c t i o n s
----------------------------------------------------------------------*/
void adjust_number(A_CHOICE *best_choice, float *certainty_array);

void append_number_choices(int state,
                           char *word,
                           char unichar_lengths[],
                           int unichar_offsets[],
                           CHOICES_LIST choices,
                           int char_index,
                           A_CHOICE *this_choice,
                           float *limit,
                           float rating,
                           float certainty,
                           float *certainty_array,
                           CHOICES *result);

void init_permnum();

int number_character_type(char ch, int length, int state);

                                 //current state
int number_state_change(int state, const char *word, const char* lengths);

CHOICES number_permute(int state,
                       CHOICES_LIST choices,
                       int char_index,
                       float *limit,
                       char *word,
                       char unichar_lengths[],
                       int unichar_offsets[],
                       float rating,
                       float certainty,
                       float *certainty_array);

A_CHOICE *number_permute_and_select(CHOICES_LIST char_choices,
                                    float rating_limit);

int pure_number(const char *string, const char *lengths);

int valid_number(const char *string, const char *lengths);
#endif
