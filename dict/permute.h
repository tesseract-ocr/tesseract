/* -*-C-*-
 ********************************************************************************
 *
 * File:        permute.h  (Formerly permute.h)
 * Description:  Permute choices together
 * Author:       Mark Seaman, OCR Technology
 * Created:      Fri Sep 22 14:05:51 1989
 * Modified:     Mon May 20 16:32:04 1991 (Mark Seaman) marks@hpgrlt
 * Language:     C
 * Package:      N/A
 * Status:       Experimental (Do Not Distribute)
 *
 * (c) Copyright 1989, Hewlett-Packard Company.
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
#ifndef PERMUTE_H
#define PERMUTE_H

/*----------------------------------------------------------------------
              I n c l u d e s
----------------------------------------------------------------------*/
#include "choicearr.h"

/*----------------------------------------------------------------------
              T y p e s
----------------------------------------------------------------------*/
#define RATING_PAD      4.0

/*----------------------------------------------------------------------
              V a r i a b l e s
----------------------------------------------------------------------*/
extern int adjust_debug;
extern float garbage;
extern float non_word;
extern int permute_only_top;

/*----------------------------------------------------------------------
              F u n c t i o n s
----------------------------------------------------------------------*/
void add_document_word(A_CHOICE *best_choice);

void adjust_non_word (A_CHOICE * best_choice, float certainties[]);

void init_permute_vars();
void init_permute();
void end_permute();

A_CHOICE *permute_all(CHOICES_LIST char_choices,
                      float rating_limit,
                      A_CHOICE *raw_choice);

void permute_characters(CHOICES_LIST char_choices,
                        float limit,
                        A_CHOICE *best_choice,
                        A_CHOICE *raw_choice);

A_CHOICE *permute_compound_words(CHOICES_LIST character_choices,
                                 float rating_limit);

void permute_subword(CHOICES_LIST character_choices,
                     float rating_limit,
                     int start,
                     int end,
                     char *word,
                     char unichar_lengths[],
                     float *rating,
                     float *certainty);

A_CHOICE *permute_top_choice(CHOICES_LIST character_choices,
                             float rating_limit,
                             A_CHOICE *raw_choice,
                             BOOL8 *any_alpha);

const char* choose_il1(const char *first_char,   //first choice
                       const char *second_char,  //second choice
                       const char *third_char,   //third choice
                       const char *prev_char,    //prev in word
                       const char *next_char,    //next in word
                       const char *next_next_char);

A_CHOICE *permute_words(CHOICES_LIST char_choices, float rating_limit);

int valid_word(const char *string);
#endif
