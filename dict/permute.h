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

#include "ratngs.h"
#include "params.h"
#include "unicharset.h"

#define MAX_PERM_LENGTH 128

/*----------------------------------------------------------------------
              V a r i a b l e s
----------------------------------------------------------------------*/
extern INT_VAR_H(fragments_debug, 0, "Debug character fragments");
extern INT_VAR_H(segment_debug, 0, "Debug the whole segmentation process");
extern BOOL_VAR_H(permute_debug, 0, "char permutation debug");

extern BOOL_VAR_H(permute_script_word, 0,
                  "Turn on word script consistency permuter");

extern BOOL_VAR_H(permute_fixed_length_dawg, 0,
                  "Turn on fixed-length phrasebook search permuter");

extern BOOL_VAR_H(segment_segcost_rating, 0,
                  "incorporate segmentation cost in word rating?");

extern double_VAR_H(segment_reward_script, 0.95,
                    "Score multipler for script consistency within a word. "
                    "Being a 'reward' factor, it should be <= 1. "
                    "Smaller value implies bigger reward.");

extern BOOL_VAR_H(permute_chartype_word, 0,
         "Turn on character type (property) consistency permuter");
extern double_VAR_H(segment_reward_chartype, 0.97,
           "Score multipler for char type consistency within a word. ");

extern double_VAR_H(segment_reward_ngram_best_choice, 0.99,
                    "Score multipler for ngram permuter's best choice"
                    " (only used in the Han script path).");

extern INT_VAR_H(max_permuter_attempts, 100000,
                 "Maximum number of different character choices to consider"
                 " during permutation. This limit is especially useful when"
                 " user patterns are specified, since overly generic patterns"
                 " can result in dawg search exploring an overly large number"
                 "of options.");

extern int permute_only_top;

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

namespace tesseract {

// This is an awkward solution to allow "compounding" of permuter effects.
// Right now, each permuter generates a WERD_CHOICE with some modified
// rating which is compared to the current best choice, and the winner
// is saved.  Therefore, independent permuter improvements, eg. from script
// consistency, dictionary check, and punctuation promoting, override each
// other and can not be combined.
// We need a trellis and someway to modify the path cost.  Instead, we
// approximate by saving a permutation string, which records the preferred
// char choice [0-9] at each position [0..#chunks], and a cumulative reward
// factor.  Non-conflicting changes can be accumulated and the combined
// result will be returned.
// Default_bias is the initial value for the base multiplier.  In other words,
// it is the multiplier for raw choice rating if nothing is modified.
// This would be 1.0 when used with reward-based permuters in CJK-path,
// but it could be > 1 (eg. segment_penalty_garbage) to be compatible with
// penalty-based permuters in the Latin path.
// Note this class does not handle fragmented characters.  It does so by
// setting the preferred position of fragmented characters to '1' at Init,
// which effectively skips the fragment choice.  However, it can still be
// overridden if collision is allowed.  It is the responsibility of the
// permuters to avoid permuting fragmented characters.
class PermuterState {
 public:
  PermuterState();

  void Init(const BLOB_CHOICE_LIST_VECTOR& char_choices,
            const UNICHARSET &unicharset,
            float default_bias,
            bool debug);

  void AddPreference(int start_pos, char* pos_str, float weight);

  void AddPreference(int char_pos, BLOB_CHOICE* blob_choice, float weight);

  WERD_CHOICE* GetPermutedWord(float *certainties, float *adjust_factor);

  void set_allow_collision(bool flag) { allow_collision_ = flag; }
  void set_adjust_factor(float factor) { adjust_factor_ = factor; }
  void set_debug(bool debug) { debug_ = debug; }
  bool position_marked(int pos) { return perm_state_[pos] != kPosFree; }

 private:
  static const char kPosFree = '.';

  const BLOB_CHOICE_LIST_VECTOR *char_choices_;   // reference pointer only
                            // does not need to be allocated or freed
  char perm_state_[MAX_PERM_LENGTH];   // handles upto MAX_PERM_LENGTH-1 states
                            // stores preferred char choices, '0'..'9', or '.'
  int word_length_;         // the number of char positions in the word
  bool allow_collision_;    // can previously set preference to be overwritten?
  float adjust_factor_;     // multiplying factor for rating adjustment
  bool debug_;              // whether debug statements should be printed
};

}  // namespace tesseract

#endif
