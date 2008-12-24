/* -*-C-*-
 ********************************************************************************
 *
 * File:        permdawg.h  (Formerly permdawg.h)
 * Description:
 * Author:       Mark Seaman, SW Productivity
 * Created:      Fri Oct 16 14:37:00 1987
 * Modified:     Mon May 20 16:45:29 1991 (Mark Seaman) marks@hpgrlt
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
#ifndef PERMDAWG_H
#define PERMDAWG_H

/*----------------------------------------------------------------------
              I n c l u d e s
----------------------------------------------------------------------*/
#include "dawg.h"
#include "choices.h"
#include "choicearr.h"

/*---------------------------------------------------------------------
              V a r i a b l e s
----------------------------------------------------------------------*/
extern int dawg_debug;
extern float ok_word;
extern float good_word;
extern float freq_word;

/*----------------------------------------------------------------------
              M a c r o s
----------------------------------------------------------------------*/

/*----------------------------------------------------------------------
            Public Function Prototypes
----------------------------------------------------------------------*/
void adjust_word(A_CHOICE *best_choice, float *certainty_array);

                                 /*previous option */
void append_next_choice(EDGE_ARRAY dawg,
                        NODE_REF node,
                        char permuter,
                        char *word,
                        char unichar_lengths[],
                        int unichar_offsets[],
                        CHOICES_LIST choices,
                        int char_index,
                        A_CHOICE *this_choice,
                        const char *prevchar,
                        float *limit,
                        float rating,
                        float certainty,
                        float *rating_array,
                        float *certainty_array,
                        int word_ending,
                        int last_word,
                        CHOICES *result);

CHOICES dawg_permute(EDGE_ARRAY dawg,
                     NODE_REF node,
                     char permuter,
                     CHOICES_LIST choices,
                     int char_index,
                     float *limit,
                     char *word,
                     char unichar_lengths[],
                     int unichar_offsets[],
                     float rating,
                     float certainty,
                     float *rating_array,
                     float *certainty_array,
                     int last_word);

void dawg_permute_and_select(const char *string,
                             EDGE_ARRAY dawg,
                             char permuter,
                             CHOICES_LIST character_choices,
                             A_CHOICE *best_choice);

void init_permdawg_vars();
void init_permdawg();
void end_permdawg();

int test_freq_words(const char *word);
#endif
