/* -*-C-*-
 ********************************************************************************
 *
 * File:        permdawg.c  (Formerly permdawg.c)
 * Description:  Scale word choices by a dictionary
 * Author:       Mark Seaman, OCR Technology
 * Created:      Fri Oct 16 14:37:00 1987
 * Modified:     Tue Jul  9 15:43:18 1991 (Mark Seaman) marks@hpgrlt
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
/*----------------------------------------------------------------------
              I n c l u d e s
----------------------------------------------------------------------*/

#include "cutil.h"
#include "dawg.h"
#include "freelist.h"
#include "globals.h"
#include "ndminx.h"
#include "permute.h"
#include "stopper.h"
#include "tprintf.h"
#include "params.h"

#include <ctype.h>
#include "dict.h"
#include "image.h"

/*----------------------------------------------------------------------
              F u n c t i o n s
----------------------------------------------------------------------*/
namespace tesseract {

static const float kPermDawgRatingPad = 5.0;

/**
 * @name go_deeper_dawg_fxn
 *
 * If the choice being composed so far could be a dictionary word
 * keep exploring choices.
 *
 * There are two modes for deciding whether to go deeper: regular dawg
 * permuter mode and the special ambigs mode. If *limit is <= 0.0 the
 * function switches to the ambigs mode (this is the case when
 * dawg_permute_and_select() function is called from NoDangerousAmbigs()) and
 * only searches for the first choice that has a rating better than *limit
 * (in this case ratings are fake, since the real ratings can not be < 0).
 * Modification of the hyphen state is turned off in the ambigs mode.
 * When in the regular dawg permuter mode, the function explores all the
 * possible words and chooses the one with the best rating. The letters with
 * ratings that are far worse than the ones seen so far are pruned out.
 */
void Dict::go_deeper_dawg_fxn(
    const char *debug, const BLOB_CHOICE_LIST_VECTOR &char_choices,
    int char_choice_index, const CHAR_FRAGMENT_INFO *prev_char_frag_info,
    bool word_ending, WERD_CHOICE *word, float certainties[], float *limit,
    WERD_CHOICE *best_choice, int *attempts_left, void *void_more_args) {
  DawgArgs *more_args = reinterpret_cast<DawgArgs*>(void_more_args);
  word_ending = (char_choice_index == more_args->end_char_choice_index);
  int word_index = word->length() - 1;

  if (ambigs_mode(*limit)) {
    if (best_choice->rating() < *limit) return;
  } else {
    // Prune bad subwords
    if (more_args->rating_array[word_index] == NO_RATING) {
      more_args->rating_array[word_index] = word->rating();
    } else {
      float permdawg_limit = more_args->rating_array[word_index] *
        more_args->rating_margin + kPermDawgRatingPad;
      if (permdawg_limit < word->rating()) {
        if (permute_debug && dawg_debug_level) {
          tprintf("early pruned word rating=%4.2f,"
                  " permdawg_limit=%4.2f, word=%s\n", word->rating(),
                  permdawg_limit, word->debug_string(getUnicharset()).string());
        }
        return;
      }
    }
  }
  // Deal with hyphens
  if (word_ending && more_args->sought_word_length == kAnyWordLength &&
      has_hyphen_end(*word) && !ambigs_mode(*limit)) {
    // Copy more_args->active_dawgs to clean_active_dawgs removing
    // dawgs of type DAWG_TYPE_PATTERN.
    DawgInfoVector clean_active_dawgs;
    const DawgInfoVector &active_dawgs = *(more_args->active_dawgs);
    for (int i = 0; i < active_dawgs.size(); ++i) {
      if (dawgs_[active_dawgs[i].dawg_index]->type() != DAWG_TYPE_PATTERN) {
        clean_active_dawgs += active_dawgs[i];
      }
    }
    if (clean_active_dawgs.size() > 0) {
      if (permute_debug && dawg_debug_level)
        tprintf("new hyphen choice = %s\n",
                word->debug_string(getUnicharset()).string());
      word->set_permuter(more_args->permuter);
      adjust_word(word, certainties, permute_debug);
      set_hyphen_word(*word, *(more_args->active_dawgs),
                      *(more_args->constraints));
      update_best_choice(*word, best_choice);
    }
  } else {  // Look up char in DAWG
    // TODO(daria): update the rest of the code that specifies alternative
    // letter_is_okay_ functions (e.g. TessCharNgram class) to work with
    // multi-byte unichars and/or unichar ids.

    // If the current unichar is an ngram first try calling
    // letter_is_okay() for each unigram it contains separately.
    UNICHAR_ID orig_uch_id = word->unichar_id(word_index);
    bool checked_unigrams = false;
    if (getUnicharset().get_isngram(orig_uch_id)) {
      if (permute_debug && dawg_debug_level) {
        tprintf("checking unigrams in an ngram %s\n",
                getUnicharset().debug_str(orig_uch_id).string());
      }
      int orig_num_fragments = word->fragment_length(word_index);
      int num_unigrams = 0;
      word->remove_last_unichar_id();
      const char *ngram_str = getUnicharset().id_to_unichar(orig_uch_id);
      const char *ngram_str_end = ngram_str + strlen(ngram_str);
      const char *ngram_ptr = ngram_str;
      bool unigrams_ok = true;
      // Construct DawgArgs that reflect the current state.
      DawgInfoVector unigram_active_dawgs = *(more_args->active_dawgs);
      DawgInfoVector unigram_constraints = *(more_args->constraints);
      DawgInfoVector unigram_updated_active_dawgs;
      DawgInfoVector unigram_updated_constraints;
      DawgArgs unigram_dawg_args(&unigram_active_dawgs,
                                 &unigram_constraints,
                                 &unigram_updated_active_dawgs,
                                 &unigram_updated_constraints, 0.0,
                                 more_args->permuter,
                                 more_args->sought_word_length,
                                 more_args->end_char_choice_index);
      // Check unigrams in the ngram with letter_is_okay().
      while (unigrams_ok && ngram_ptr < ngram_str_end) {
        int step = getUnicharset().step(ngram_ptr);
        UNICHAR_ID uch_id = (step <= 0) ? INVALID_UNICHAR_ID :
            getUnicharset().unichar_to_id(ngram_ptr, step);
        ngram_ptr += step;
        ++num_unigrams;
        word->append_unichar_id(uch_id, 1, 0.0, 0.0);
        unigrams_ok = unigrams_ok && (this->*letter_is_okay_)(
            &unigram_dawg_args,
            word->unichar_id(word_index+num_unigrams-1),
            word_ending && (ngram_ptr == ngram_str_end));
        (*unigram_dawg_args.active_dawgs) =
          *(unigram_dawg_args.updated_active_dawgs);
        (*unigram_dawg_args.constraints) =
          *(unigram_dawg_args.updated_constraints);
        if (permute_debug && dawg_debug_level) {
          tprintf("unigram %s is %s\n",
                  getUnicharset().debug_str(uch_id).string(),
                  unigrams_ok ? "OK" : "not OK");
        }
      }
      // Restore the word and copy the updated dawg state if needed.
      while (num_unigrams-- > 0) word->remove_last_unichar_id();
      word->append_unichar_id_space_allocated(
          orig_uch_id, orig_num_fragments, 0.0, 0.0);
      if (unigrams_ok) {
        checked_unigrams = true;
        more_args->permuter = unigram_dawg_args.permuter;
        *(more_args->updated_active_dawgs) =
          *(unigram_dawg_args.updated_active_dawgs);
        *(more_args->updated_constraints) =
          *(unigram_dawg_args.updated_constraints);
      }
    }

    // Check which dawgs from the dawgs_ vector contain the word
    // up to and including the current unichar.
    if (checked_unigrams || (this->*letter_is_okay_)(
        more_args, word->unichar_id(word_index), word_ending)) {
      // Add a new word choice
      if (word_ending) {
        if (permute_debug && dawg_debug_level) {
          tprintf("found word = %s\n",
                  word->debug_string(getUnicharset()).string());
        }
        WERD_CHOICE *adjusted_word = word;
        WERD_CHOICE hyphen_tail_word;
        if (hyphen_base_size() > 0) {
          hyphen_tail_word = *word;
          remove_hyphen_head(&hyphen_tail_word);
          adjusted_word = &hyphen_tail_word;
        }
        adjusted_word->set_permuter(more_args->permuter);
        if (!ambigs_mode(*limit)) {
          adjust_word(adjusted_word, &certainties[hyphen_base_size()],
                      permute_debug);
        }
        update_best_choice(*adjusted_word, best_choice);
      } else {  // search the next letter
        // Make updated_* point to the next entries in the DawgInfoVector
        // arrays (that were originally created in dawg_permute_and_select)
        ++(more_args->updated_active_dawgs);
        ++(more_args->updated_constraints);
        // Make active_dawgs and constraints point to the updated ones.
        ++(more_args->active_dawgs);
        ++(more_args->constraints);
        permute_choices(debug, char_choices, char_choice_index + 1,
                        prev_char_frag_info, word, certainties, limit,
                        best_choice, attempts_left, more_args);
        // Restore previous state to explore another letter in this position.
        --(more_args->updated_active_dawgs);
        --(more_args->updated_constraints);
        --(more_args->active_dawgs);
        --(more_args->constraints);
      }
    } else {
      if (permute_debug && dawg_debug_level) {
        tprintf("last unichar not OK at index %d in %s\n",
                word_index, word->debug_string(getUnicharset()).string());
      }
    }
  }
}

/**
 * dawg_permute_and_select
 *
 * Recursively explore all the possible character combinations in
 * the given char_choices. Use go_deeper_dawg_fxn() to search all the
 * dawgs in the dawgs_ vector in parallel and discard invalid words.
 *
 * If sought_word_length is not kAnyWordLength, the function only searches
 * for a valid word formed by the given char_choices in one fixed length
 * dawg (that contains words of length sought_word_length) starting at the
 * start_char_choice_index.
 *
 * Allocate and return a WERD_CHOICE with the best valid word found.
 */
WERD_CHOICE *Dict::dawg_permute_and_select(
    const BLOB_CHOICE_LIST_VECTOR &char_choices, float rating_limit,
    int sought_word_length, int start_char_choice_index) {
  WERD_CHOICE *best_choice = new WERD_CHOICE();
  best_choice->make_bad();
  best_choice->set_rating(rating_limit);
  if (char_choices.length() == 0) return best_choice;
  DawgInfoVector *active_dawgs = new DawgInfoVector[char_choices.length() + 1];
  DawgInfoVector *constraints =  new DawgInfoVector[char_choices.length() + 1];
  init_active_dawgs(sought_word_length, &(active_dawgs[0]),
                    ambigs_mode(rating_limit));
  init_constraints(&(constraints[0]));
  int end_char_choice_index = (sought_word_length == kAnyWordLength) ?
    char_choices.length()-1 : start_char_choice_index+sought_word_length-1;
  // Need to skip accumulating word choices if we are only searching a part of
  // the word (e.g. for the phrase search in non-space delimited languages).
  // Also need to skip accumulating choices if char_choices are expanded
  // with ambiguities.
  bool re_enable_choice_accum = ChoiceAccumEnabled();
  if (sought_word_length != kAnyWordLength ||
      ambigs_mode(rating_limit)) DisableChoiceAccum();
  DawgArgs dawg_args(&(active_dawgs[0]), &(constraints[0]),
                     &(active_dawgs[1]), &(constraints[1]),
                     (segment_penalty_dict_case_bad /
                      segment_penalty_dict_case_ok),
                     NO_PERM, sought_word_length, end_char_choice_index);
  WERD_CHOICE word(MAX_WERD_LENGTH);
  copy_hyphen_info(&word);
  // Discard rating and certainty of the hyphen base (if any).
  word.set_rating(0.0);
  word.set_certainty(0.0);
  if (word.length() + char_choices.length() > MAX_WERD_LENGTH) {
    delete[] active_dawgs;
    delete[] constraints;
    return best_choice;  // the word is too long to permute
  }
  float certainties[MAX_WERD_LENGTH];
  this->go_deeper_fxn_ = &tesseract::Dict::go_deeper_dawg_fxn;
  int attempts_left = max_permuter_attempts;
  permute_choices((permute_debug && dawg_debug_level) ?
                  "permute_dawg_debug" : NULL,
                  char_choices, start_char_choice_index, NULL, &word,
                  certainties, &rating_limit, best_choice, &attempts_left,
                  &dawg_args);
  delete[] active_dawgs;
  delete[] constraints;
  if (re_enable_choice_accum) EnableChoiceAccum();
  return best_choice;
}

}  // namespace tesseract
