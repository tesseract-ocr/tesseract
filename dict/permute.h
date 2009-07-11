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
#include "choices.h"
#include "ratngs.h"
#include "varable.h"

/*----------------------------------------------------------------------
              T y p e s
----------------------------------------------------------------------*/
#define RATING_PAD      4.0

/*----------------------------------------------------------------------
              V a r i a b l e s
----------------------------------------------------------------------*/
extern INT_VAR_H(fragments_debug, 0, "Debug character fragments");
extern BOOL_VAR_H(segment_debug, 0, "Debug the whole segmentation process");
extern BOOL_VAR_H(permute_debug, 0, "char permutation debug");

extern BOOL_VAR_H(permute_script_word, 0,
                  "Turn on word script consistency permuter");

extern BOOL_VAR_H(segment_segcost_rating, 0,
                  "incorporate segmentation cost in word rating?");

extern double_VAR_H(segment_reward_script, 0.95,
                    "Score multipler for script consistency within a word. "
                    "Being a 'reward' factor, it should be <= 1. "
                    "Smaller value implies bigger reward.");

extern double_VAR_H(segment_penalty_garbage, 1.5,
                    "Score multiplier for poorly cased strings that are not "
                    "in the dictionary and generally look like garbage "
                    "(lower is better).");

extern double_VAR_H(segment_penalty_dict_nonword, 1.25,
                    "Score multiplier for glyph fragment segmentations which "
                    "do not match a dictionary word (lower is better).");

extern int permute_only_top;
extern float wordseg_rating_adjust_factor;

/*----------------------------------------------------------------------
              F u n c t i o n s
----------------------------------------------------------------------*/
void adjust_non_word(const char *word, const char *word_lengths,
                     float rating, float *new_rating, float *adjust_factor);

const char* choose_il1(const char *first_char,   //first choice
                       const char *second_char,  //second choice
                       const char *third_char,   //third choice
                       const char *prev_char,    //prev in word
                       const char *next_char,    //next in word
                       const char *next_next_char);

#endif
