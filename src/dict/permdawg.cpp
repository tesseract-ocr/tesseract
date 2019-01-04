/* -*-C-*-
 ********************************************************************************
 *
 * File:         permdawg.cpp  (Formerly permdawg.c)
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

#include "dawg.h"
#include "globals.h"
#include "stopper.h"
#include "tprintf.h"
#include "params.h"

#include <algorithm>
#include <cctype>
#include "dict.h"

/*----------------------------------------------------------------------
              F u n c t i o n s
----------------------------------------------------------------------*/
namespace tesseract {

/**
 * @name go_deeper_dawg_fxn
 *
 * If the choice being composed so far could be a dictionary word
 * keep exploring choices.
 */
void Dict::go_deeper_dawg_fxn(
    const char *debug, const BLOB_CHOICE_LIST_VECTOR &char_choices,
    int char_choice_index, const CHAR_FRAGMENT_INFO *prev_char_frag_info,
    bool word_ending, WERD_CHOICE *word, float certainties[], float *limit,
    WERD_CHOICE *best_choice, int *attempts_left, void *void_more_args) {
  DawgArgs *more_args = static_cast<DawgArgs *>(void_more_args);
  word_ending = (char_choice_index == char_choices.size()-1);
  int word_index = word->length() - 1;
  if (best_choice->rating() < *limit) return;
  // Look up char in DAWG

  // If the current unichar is an ngram first try calling
  // letter_is_okay() for each unigram it contains separately.
  UNICHAR_ID orig_uch_id = word->unichar_id(word_index);
  bool checked_unigrams = false;
  if (getUnicharset().get_isngram(orig_uch_id)) {
    if (dawg_debug_level) {
      tprintf("checking unigrams in an ngram %s\n",
              getUnicharset().debug_str(orig_uch_id).string());
    }
    int num_unigrams = 0;
    word->remove_last_unichar_id();
    GenericVector<UNICHAR_ID> encoding;
    const char *ngram_str = getUnicharset().id_to_unichar(orig_uch_id);
    // Since the string came out of the unicharset, failure is impossible.
    ASSERT_HOST(getUnicharset().encode_string(ngram_str, true, &encoding, nullptr,
                                              nullptr));
    bool unigrams_ok = true;
    // Construct DawgArgs that reflect the current state.
    DawgPositionVector unigram_active_dawgs = *(more_args->active_dawgs);
    DawgPositionVector unigram_updated_dawgs;
    DawgArgs unigram_dawg_args(&unigram_active_dawgs,
                               &unigram_updated_dawgs,
                               more_args->permuter);
    // Check unigrams in the ngram with letter_is_okay().
    for (int i = 0; unigrams_ok && i < encoding.size(); ++i) {
      UNICHAR_ID uch_id = encoding[i];
      ASSERT_HOST(uch_id != INVALID_UNICHAR_ID);
      ++num_unigrams;
      word->append_unichar_id(uch_id, 1, 0.0, 0.0);
      unigrams_ok = (this->*letter_is_okay_)(
          &unigram_dawg_args, *word->unicharset(),
          word->unichar_id(word_index+num_unigrams-1),
          word_ending && i == encoding.size() - 1);
      (*unigram_dawg_args.active_dawgs) = *(unigram_dawg_args.updated_dawgs);
      if (dawg_debug_level) {
        tprintf("unigram %s is %s\n",
                getUnicharset().debug_str(uch_id).string(),
                unigrams_ok ? "OK" : "not OK");
      }
    }
    // Restore the word and copy the updated dawg state if needed.
    while (num_unigrams-- > 0) word->remove_last_unichar_id();
    word->append_unichar_id_space_allocated(orig_uch_id, 1, 0.0, 0.0);
    if (unigrams_ok) {
      checked_unigrams = true;
      more_args->permuter = unigram_dawg_args.permuter;
      *(more_args->updated_dawgs) = *(unigram_dawg_args.updated_dawgs);
    }
  }

  // Check which dawgs from the dawgs_ vector contain the word
  // up to and including the current unichar.
  if (checked_unigrams || (this->*letter_is_okay_)(
      more_args, *word->unicharset(), word->unichar_id(word_index),
      word_ending)) {
    // Add a new word choice
    if (word_ending) {
      if (dawg_debug_level) {
        tprintf("found word = %s\n", word->debug_string().string());
      }
      if (strcmp(output_ambig_words_file.string(), "") != 0) {
        if (output_ambig_words_file_ == nullptr) {
          output_ambig_words_file_ =
              fopen(output_ambig_words_file.string(), "wb+");
          if (output_ambig_words_file_ == nullptr) {
            tprintf("Failed to open output_ambig_words_file %s\n",
                    output_ambig_words_file.string());
            exit(1);
          }
          STRING word_str;
          word->string_and_lengths(&word_str, nullptr);
          word_str += " ";
          fprintf(output_ambig_words_file_, "%s", word_str.string());
        }
        STRING word_str;
        word->string_and_lengths(&word_str, nullptr);
        word_str += " ";
        fprintf(output_ambig_words_file_, "%s", word_str.string());
      }
      WERD_CHOICE *adjusted_word = word;
      adjusted_word->set_permuter(more_args->permuter);
      update_best_choice(*adjusted_word, best_choice);
    } else {  // search the next letter
      // Make updated_* point to the next entries in the DawgPositionVector
      // arrays (that were originally created in dawg_permute_and_select)
      ++(more_args->updated_dawgs);
      // Make active_dawgs and constraints point to the updated ones.
      ++(more_args->active_dawgs);
      permute_choices(debug, char_choices, char_choice_index + 1,
                      prev_char_frag_info, word, certainties, limit,
                      best_choice, attempts_left, more_args);
      // Restore previous state to explore another letter in this position.
      --(more_args->updated_dawgs);
      --(more_args->active_dawgs);
    }
  } else {
      if (dawg_debug_level) {
        tprintf("last unichar not OK at index %d in %s\n",
                word_index, word->debug_string().string());
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
 * Allocate and return a WERD_CHOICE with the best valid word found.
 */
WERD_CHOICE *Dict::dawg_permute_and_select(
    const BLOB_CHOICE_LIST_VECTOR &char_choices, float rating_limit) {
  WERD_CHOICE *best_choice = new WERD_CHOICE(&getUnicharset());
  best_choice->make_bad();
  best_choice->set_rating(rating_limit);
  if (char_choices.length() == 0 || char_choices.length() > MAX_WERD_LENGTH)
    return best_choice;
  DawgPositionVector *active_dawgs =
      new DawgPositionVector[char_choices.length() + 1];
  init_active_dawgs(&(active_dawgs[0]), true);
  DawgArgs dawg_args(&(active_dawgs[0]), &(active_dawgs[1]), NO_PERM);
  WERD_CHOICE word(&getUnicharset(), MAX_WERD_LENGTH);

  float certainties[MAX_WERD_LENGTH];
  this->go_deeper_fxn_ = &tesseract::Dict::go_deeper_dawg_fxn;
  int attempts_left = max_permuter_attempts;
  permute_choices((dawg_debug_level) ? "permute_dawg_debug" : nullptr,
      char_choices, 0, nullptr, &word, certainties, &rating_limit, best_choice,
      &attempts_left, &dawg_args);
  delete[] active_dawgs;
  return best_choice;
}

/**
 * permute_choices
 *
 * Call append_choices() for each BLOB_CHOICE in BLOB_CHOICE_LIST
 * with the given char_choice_index in char_choices.
 */
void Dict::permute_choices(
    const char *debug,
    const BLOB_CHOICE_LIST_VECTOR &char_choices,
    int char_choice_index,
    const CHAR_FRAGMENT_INFO *prev_char_frag_info,
    WERD_CHOICE *word,
    float certainties[],
    float *limit,
    WERD_CHOICE *best_choice,
    int *attempts_left,
    void *more_args) {
  if (debug) {
    tprintf("%s permute_choices: char_choice_index=%d"
            " limit=%g rating=%g, certainty=%g word=%s\n",
            debug, char_choice_index, *limit, word->rating(),
            word->certainty(), word->debug_string().string());
  }
  if (char_choice_index < char_choices.length()) {
    BLOB_CHOICE_IT blob_choice_it;
    blob_choice_it.set_to_list(char_choices.get(char_choice_index));
    for (blob_choice_it.mark_cycle_pt(); !blob_choice_it.cycled_list();
         blob_choice_it.forward()) {
      (*attempts_left)--;
      append_choices(debug, char_choices, *(blob_choice_it.data()),
                     char_choice_index, prev_char_frag_info, word,
                     certainties, limit, best_choice, attempts_left, more_args);
      if (*attempts_left <= 0) {
        if (debug) tprintf("permute_choices(): attempts_left is 0\n");
        break;
      }
    }
  }
}

/**
 * append_choices
 *
 * Checks to see whether or not the next choice is worth appending to
 * the word being generated. If so then keeps going deeper into the word.
 *
 * This function assumes that Dict::go_deeper_fxn_ is set.
 */
void Dict::append_choices(
    const char *debug,
    const BLOB_CHOICE_LIST_VECTOR &char_choices,
    const BLOB_CHOICE &blob_choice,
    int char_choice_index,
    const CHAR_FRAGMENT_INFO *prev_char_frag_info,
    WERD_CHOICE *word,
    float certainties[],
    float *limit,
    WERD_CHOICE *best_choice,
    int *attempts_left,
    void *more_args) {
  int word_ending = (char_choice_index == char_choices.length() - 1);

  // Deal with fragments.
  CHAR_FRAGMENT_INFO char_frag_info;
  if (!fragment_state_okay(blob_choice.unichar_id(), blob_choice.rating(),
                           blob_choice.certainty(), prev_char_frag_info, debug,
                           word_ending, &char_frag_info)) {
    return;  // blob_choice must be an invalid fragment
  }
  // Search the next letter if this character is a fragment.
  if (char_frag_info.unichar_id == INVALID_UNICHAR_ID) {
    permute_choices(debug, char_choices, char_choice_index + 1,
                    &char_frag_info, word, certainties, limit,
                    best_choice, attempts_left, more_args);
    return;
  }

  // Add the next unichar.
  float old_rating = word->rating();
  float old_certainty = word->certainty();
  uint8_t old_permuter = word->permuter();
  certainties[word->length()] = char_frag_info.certainty;
  word->append_unichar_id_space_allocated(
      char_frag_info.unichar_id, char_frag_info.num_fragments,
      char_frag_info.rating, char_frag_info.certainty);

  // Explore the next unichar.
  (this->*go_deeper_fxn_)(debug, char_choices, char_choice_index,
                          &char_frag_info, word_ending, word, certainties,
                          limit, best_choice, attempts_left, more_args);

  // Remove the unichar we added to explore other choices in it's place.
  word->remove_last_unichar_id();
  word->set_rating(old_rating);
  word->set_certainty(old_certainty);
  word->set_permuter(old_permuter);
}

/**
 * @name fragment_state
 *
 * Given the current char choice and information about previously seen
 * fragments, determines whether adjacent character fragments are
 * present and whether they can be concatenated.
 *
 * The given prev_char_frag_info contains:
 * - fragment: if not nullptr contains information about immediately
 *   preceding fragmented character choice
 * - num_fragments: number of fragments that have been used so far
 *   to construct a character
 * - certainty: certainty of the current choice or minimum
 *   certainty of all fragments concatenated so far
 * - rating: rating of the current choice or sum of fragment
 *   ratings concatenated so far
 *
 * The output char_frag_info is filled in as follows:
 * - character: is set to be nullptr if the choice is a non-matching
 *   or non-ending fragment piece; is set to unichar of the given choice
 *   if it represents a regular character or a matching ending fragment
 * - fragment,num_fragments,certainty,rating are set as described above
 *
 * @returns false if a non-matching fragment is discovered, true otherwise.
 */
bool Dict::fragment_state_okay(UNICHAR_ID curr_unichar_id,
                               float curr_rating, float curr_certainty,
                               const CHAR_FRAGMENT_INFO *prev_char_frag_info,
                               const char *debug, int word_ending,
                               CHAR_FRAGMENT_INFO *char_frag_info) {
  const CHAR_FRAGMENT *this_fragment =
    getUnicharset().get_fragment(curr_unichar_id);
  const CHAR_FRAGMENT *prev_fragment =
    prev_char_frag_info != nullptr ? prev_char_frag_info->fragment : nullptr;

  // Print debug info for fragments.
  if (debug && (prev_fragment || this_fragment)) {
    tprintf("%s check fragments: choice=%s word_ending=%d\n", debug,
            getUnicharset().debug_str(curr_unichar_id).string(),
            word_ending);
    if (prev_fragment) {
      tprintf("prev_fragment %s\n", prev_fragment->to_string().string());
    }
    if (this_fragment) {
      tprintf("this_fragment %s\n", this_fragment->to_string().string());
    }
  }

  char_frag_info->unichar_id = curr_unichar_id;
  char_frag_info->fragment = this_fragment;
  char_frag_info->rating = curr_rating;
  char_frag_info->certainty = curr_certainty;
  char_frag_info->num_fragments = 1;
  if (prev_fragment && !this_fragment) {
    if (debug) tprintf("Skip choice with incomplete fragment\n");
    return false;
  }
  if (this_fragment) {
    // We are dealing with a fragment.
    char_frag_info->unichar_id = INVALID_UNICHAR_ID;
    if (prev_fragment) {
      if (!this_fragment->is_continuation_of(prev_fragment)) {
        if (debug) tprintf("Non-matching fragment piece\n");
        return false;
      }
      if (this_fragment->is_ending()) {
        char_frag_info->unichar_id =
          getUnicharset().unichar_to_id(this_fragment->get_unichar());
        char_frag_info->fragment = nullptr;
        if (debug) {
          tprintf("Built character %s from fragments\n",
                  getUnicharset().debug_str(
                      char_frag_info->unichar_id).string());
        }
      } else {
        if (debug) tprintf("Record fragment continuation\n");
        char_frag_info->fragment = this_fragment;
      }
      // Update certainty and rating.
      char_frag_info->rating =
        prev_char_frag_info->rating + curr_rating;
      char_frag_info->num_fragments = prev_char_frag_info->num_fragments + 1;
      char_frag_info->certainty =
              std::min(curr_certainty, prev_char_frag_info->certainty);
    } else {
      if (this_fragment->is_beginning()) {
        if (debug) tprintf("Record fragment beginning\n");
      } else {
        if (debug) {
          tprintf("Non-starting fragment piece with no prev_fragment\n");
        }
        return false;
      }
    }
  }
  if (word_ending && char_frag_info->fragment) {
    if (debug) tprintf("Word can not end with a fragment\n");
    return false;
  }
  return true;
}

}  // namespace tesseract
