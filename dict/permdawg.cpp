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
#include "permdawg.h"
#include "debug.h"
#include "hyphen.h"
#include "permute.h"
#include "tordvars.h"
#include "context.h"
#include "stopper.h"
#include "freelist.h"
#include "globals.h"
#include "tprintf.h"
#include "cutil.h"
#include "dawg.h"
#include "ndminx.h"
#include "varable.h"
#include "conversion.h"

#include <ctype.h>
#include "dict.h"
#include "image.h"

/*----------------------------------------------------------------------
              T y p e s
----------------------------------------------------------------------*/
#define FREQ_WERD     1.0
#define GOOD_WERD     1.1
#define OK_WERD       1.3125
#define MAX_FREQ_EDGES    1500
#define NO_RATING              -1

/*----------------------------------------------------------------------
              V a r i a b l e s
----------------------------------------------------------------------*/
static EDGE_ARRAY frequent_words;
static float rating_margin;
static float rating_pad = 5.0;

BOOL_VAR(segment_dawg_debug, 0, "Debug mode for word segmentation");

double_VAR(segment_penalty_dict_punc_bad, OK_WERD,
           "Default score multiplier for word matches, which may have case or "
           "punctuation issues (lower is better).");

double_VAR(segment_penalty_dict_punc_ok, GOOD_WERD,
           "Score multiplier for word matches that have good case "
           "(lower is better).");

double_VAR(segment_penalty_dict_frequent_word, FREQ_WERD,
           "Score multiplier for word matches which have good case and are "
           "frequent in the given language (lower is better).");

/*----------------------------------------------------------------------
              F u n c t i o n s
----------------------------------------------------------------------*/
/**********************************************************************
 * adjust_word
 *
 * Assign an adjusted value to a string that is a word.	The value
 * that this word choice has is based on case and punctuation rules.
 **********************************************************************/
namespace tesseract {
void Dict::adjust_word(A_CHOICE *best_choice, float *certainty_array) {
  char *this_word;
  int punct_status;
  float adjust_factor;

  if (segment_adjust_debug)
    tprintf ("Word: %s %4.2f ",
      class_string (best_choice), class_rating (best_choice));

  this_word = class_string (best_choice);
  punct_status = punctuation_ok (this_word, class_lengths (best_choice));

  class_rating (best_choice) += RATING_PAD;
  if (case_ok (this_word, class_lengths (best_choice))
      && punct_status != -1) {
    if (punct_status < 1 && word_in_dawg (frequent_words, this_word)) {
      class_rating (best_choice) *= segment_penalty_dict_frequent_word;
      class_permuter (best_choice) = FREQ_DAWG_PERM;
      adjust_factor = segment_penalty_dict_frequent_word;
      if (segment_adjust_debug)
        tprintf(", F, %4.2f ", (double)segment_penalty_dict_frequent_word);
    }
    else {
      class_rating (best_choice) *= segment_penalty_dict_punc_ok;
      adjust_factor = segment_penalty_dict_punc_ok;
      if (segment_adjust_debug)
        tprintf(", %4.2f ", (double)segment_penalty_dict_punc_ok);
    }
  }
  else {
    class_rating (best_choice) *= segment_penalty_dict_punc_bad;
    adjust_factor = segment_penalty_dict_punc_bad;
    if (segment_adjust_debug) {
      if (!case_ok (this_word, class_lengths (best_choice)))
        tprintf(", C");
      if (punctuation_ok (this_word, class_lengths (best_choice)) == -1)
        tprintf(", P");
      tprintf(", %4.2f ", (double)segment_penalty_dict_punc_bad);
    }
  }

  class_rating (best_choice) -= RATING_PAD;

  LogNewWordChoice(best_choice, adjust_factor,
                   certainty_array, getUnicharset());

  if (segment_adjust_debug)
    tprintf(" --> %4.2f\n", class_rating (best_choice));
}


/**********************************************************************
 * append_next_choice
 *
 * Check to see whether or not the next choice is worth appending to
 * the string being generated.  If so then keep going deeper into the
 * word.
 **********************************************************************/
void Dict::append_next_choice(  /*previous option */
                              EDGE_ARRAY dawg,
                              NODE_REF node,
                              char permuter,
                              char *word,
                              char unichar_lengths[],
                              int unichar_offsets[],
                              CHOICES_LIST choices,
                              int char_choice_index,
                              int word_index,
                              A_CHOICE *this_choice,
                              const char *prevchar,
                              float *limit,
                              float rating,
                              float certainty,
                              float *rating_array,
                              float *certainty_array,
                              int word_ending,
                              int last_word,
                              const CHAR_FRAGMENT_INFO *prev_char_frag_info,
                              char fragment_lengths[],
                              CHOICES *result) {
  A_CHOICE *better_choice;
  CHAR_FRAGMENT_INFO char_frag_info;
  const char *ch = NULL;

  /* Deal with fragments */
  if (!fragment_state_okay(
      getUnicharset().unichar_to_id(class_string(this_choice)),
      class_rating(this_choice), class_certainty(this_choice),
      prev_char_frag_info,
      (segment_dawg_debug && (fragments_debug > 1)) ? "dawg_debug" : NULL,
      word_ending, &char_frag_info)) {
    return;  // this_choice must be an invalid fragment
  }
  if (char_frag_info.unichar_id != INVALID_UNICHAR_ID) {
    ch = getUnicharset().id_to_unichar(char_frag_info.unichar_id);
  }
  if (ch == NULL) {   // this character is a fragment
    JOIN_ON(*result,  // so search the next letter
            dawg_permute(dawg, node, permuter, choices,
                         char_choice_index + 1, word_index,
                         limit, word, unichar_lengths, unichar_offsets,
                         rating, certainty, rating_array, certainty_array,
                         last_word, &char_frag_info, fragment_lengths));
    return;
  }

  /* Add new character */
  strcpy(word + unichar_offsets[word_index], ch);

  unichar_lengths[word_index] = strlen(ch);
  unichar_lengths[word_index + 1] = 0;
  fragment_lengths[word_index] = char_frag_info.num_fragments;
  fragment_lengths[word_index + 1] = 0;
  unichar_offsets[word_index + 1] =
    unichar_offsets[word_index] + unichar_lengths[word_index];
  if (word[unichar_offsets[word_index]] == '\0') {
    word[unichar_offsets[word_index]] = ' ';
    word[unichar_offsets[word_index] + 1] = '\0';
    unichar_lengths[word_index] = 1;
    unichar_lengths[word_index + 1] = 0;
    fragment_lengths[word_index] = 1;
    fragment_lengths[word_index + 1] = 0;
    unichar_offsets[word_index + 1] = unichar_offsets[word_index] +
      unichar_lengths[word_index];
  }
  certainty_array[word_index] = char_frag_info.certainty;
  rating += char_frag_info.rating;
  certainty = MIN (char_frag_info.certainty, certainty);

  /* Prune bad subwords */
  if (rating_array[char_choice_index] == NO_RATING) {
    rating_array[char_choice_index] = rating;
  } else {
    if (rating_array[word_index] * rating_margin + rating_pad < rating) {
      if (segment_dawg_debug) {
        tprintf("early pruned word rating=%4.2f, limit=%4.2f", rating, *limit);
        print_word_string(word);
        tprintf("\n");
      }
      return;
    }
  }

  /* Deal with hyphens */
  if (word_ending && last_word && word[unichar_offsets[word_index]] == '-' &&
      word_index > 0) {
    *limit = rating;
    if (segment_dawg_debug)
      tprintf("new hyphen choice = %s\n", word);
    better_choice = new_choice (word, unichar_lengths, rating, certainty,
                                -1, NULL, permuter, false, fragment_lengths);

    adjust_word(better_choice, certainty_array);
    push_on(*result, better_choice);
    set_hyphen_word(word, unichar_lengths, unichar_offsets,
                    rating, node, char_choice_index, fragment_lengths);
  }
  /* Look up char in DAWG */
  else {
    int sub_offset = 0;
    NODE_REF node_saved = node;
    while (sub_offset < unichar_lengths[word_index] &&
           (this->*letter_is_okay_)(dawg, &node,
                                    unichar_offsets[word_index] + sub_offset,
                                    *prevchar, word, word_ending &&
                                    sub_offset ==
                                      unichar_lengths[word_index] - 1)) {
      ++sub_offset;
    }
    if (sub_offset == unichar_lengths[word_index]) {
      /* Add a new word choice */
      if (word_ending) {
        if (segment_dawg_debug == 1)
          tprintf("new choice = %s\n", word);
        *limit = rating;

        better_choice =
          new_choice (hyphen_tail (word), unichar_lengths + hyphen_base_size(),
                      rating, certainty, -1, NULL, permuter, false,
                      fragment_lengths + hyphen_base_size());
        adjust_word (better_choice, &certainty_array[hyphen_base_size()]);
        push_on(*result, better_choice);
      }
      else {
        /* Search the next letter */
        JOIN_ON (*result,
                 dawg_permute (dawg, node, permuter, choices,
                               char_choice_index + 1, word_index + 1, limit,
                               word, unichar_lengths, unichar_offsets, rating,
                               certainty, rating_array, certainty_array,
                               last_word, &char_frag_info, fragment_lengths));
      }
    } else {
      if (segment_dawg_debug == 1) {
        tprintf("letter not OK at char %d, index %d + sub index %d/%d\n",
                word_index, unichar_offsets[word_index],
                sub_offset, unichar_lengths[word_index]);
        tprintf("Word");
        print_word_string(word);
        tprintf("\nRejected tail");
        print_word_string(word + unichar_offsets[word_index]);
        tprintf("\n");
      }
      if (node != 0)
        node = node_saved;
    }
  }
}


/**********************************************************************
 * dawg_permute
 *
 * Permute all the valid words that can be created with this starting
 * point.  The node (in the DAWG) and the word string define a base
 * from which to start adding the remaining character choices.
 **********************************************************************/
CHOICES Dict::dawg_permute(EDGE_ARRAY dawg,
                           NODE_REF node,
                           char permuter,
                           CHOICES_LIST choices,
                           int char_choice_index,
                           int word_index,
                           float *limit,
                           char *word,
                           char unichar_lengths[],
                           int unichar_offsets[],
                           float rating,
                           float certainty,
                           float *rating_array,
                           float *certainty_array,
                           int last_word,
                           const CHAR_FRAGMENT_INFO *prev_char_frag_info,
                           char fragment_lengths[]) {
  CHOICES result = NIL;
  CHOICES c;
  char *prevchar;
  int word_ending = FALSE;

  if (segment_dawg_debug) {
    tprintf("dawg_permute (node=" REFFORMAT ", char_choice_index=%d,"
            " word_index=%d, limit=%f, word=",
            node, char_choice_index, word_index, *limit);
    print_word_string(word);
    tprintf(", rating=%4.2f, certainty=%4.2f)\n", rating, certainty);
  }

  /* Check for EOW */
  if (1 + char_choice_index == array_count (choices) + hyphen_char_choice_size()) {
    word_ending = TRUE;
  }

  if (char_choice_index < array_count (choices) + hyphen_char_choice_size()) {
    prevchar = NULL;
    iterate_list (c, (CHOICES) array_index (choices,
        char_choice_index - hyphen_char_choice_size())) {
      append_next_choice (dawg, node, permuter, word, unichar_lengths,
                          unichar_offsets, choices, char_choice_index,
                          word_index, (A_CHOICE *) first_node (c),
                          prevchar != NULL ? prevchar : "", limit,
                          rating, certainty, rating_array, certainty_array,
                          word_ending, last_word, prev_char_frag_info,
                          fragment_lengths, &result);
      prevchar = best_string (c);
    }
  }

  if (result && (segment_dawg_debug == 1))
    print_choices ("dawg_permute", result);

  return (result);
}

/**********************************************************************
 * dawg_permute_and_select
 *
 * Use a DAWG type data structure to enumerate all the valid strings
 * in some gramar.  Compare each of the choices against the best choice
 * so far.  Update the best choice if needed.
 **********************************************************************/
void Dict::dawg_permute_and_select(const char *string,
                                   EDGE_ARRAY dawg,
                                   char permuter,
                                   CHOICES_LIST character_choices,
                                   A_CHOICE *best_choice,
                                   inT16 system_words) {
  CHOICES result = NIL;
  char word[UNICHAR_LEN * MAX_WERD_LENGTH + 1];
  char unichar_lengths[MAX_WERD_LENGTH + 1];
  int unichar_offsets[MAX_WERD_LENGTH + 1];
  float certainty_array[MAX_WERD_LENGTH + 1];
  float rating_array[MAX_WERD_LENGTH + 1];
  char fragment_lengths[MAX_WERD_LENGTH + 1];  // num fragments from which
                                               // each char was constructed
  float rating;
  int char_choice_index = 0;
  int word_index = 0;
  NODE_REF dawg_node = 0;

                                 /* Pruning margin ratio */
  rating_margin = segment_penalty_dict_punc_bad / segment_penalty_dict_punc_ok;

  word[0] = '\0';
  unichar_lengths[0] = 0;
  fragment_lengths[0] = 0;
  unichar_offsets[0] = 0;
  rating = class_rating (best_choice);

  for (int i = 0; i < MAX_WERD_LENGTH + 1; ++i) {
    rating_array[i] = NO_RATING;
  }

  if (!is_last_word () && hyphen_string) {
    strcpy(word, hyphen_string);
    strcpy(unichar_lengths, hyphen_unichar_lengths);
    strcpy(fragment_lengths, hyphen_fragment_lengths);
    memcpy(unichar_offsets, hyphen_unichar_offsets,
           (hyphen_base_size()) * sizeof (int));
    unichar_offsets[hyphen_base_size()] =
        unichar_offsets[hyphen_base_size() - 1] +
        unichar_lengths[hyphen_base_size() - 1];
    char_choice_index = hyphen_char_choice_size();
    word_index = strlen (hyphen_unichar_lengths);
    if (system_words)
      dawg_node = hyphen_state;
  }

  result = dawg_permute (dawg, dawg_node, permuter, character_choices,
    char_choice_index, word_index, &rating, word, unichar_lengths,
    unichar_offsets, 0.0, 0.0, rating_array, certainty_array,
    is_last_word(), NULL, fragment_lengths);

  if (display_ratings && result) {
    print_choices(string, result);
  }

  while (result != NIL) {
    if (best_rating (result) < class_rating (best_choice)) {
      clone_choice (best_choice, (A_CHOICE *) first_node (result));
    }
    free_choice (first_node (result));
    pop_off(result);
  }
}


/**********************************************************************
 * init_permdawg
 *
 * Initialize the variables needed by this file.
 **********************************************************************/
void Dict::init_permdawg() {
  STRING name;
  name = getImage()->getCCUtil()->language_data_path_prefix;
  name += "freq-dawg";
  frequent_words = read_squished_dawg(name.string());
}

void Dict::end_permdawg() {
  memfree(frequent_words);
  frequent_words = NULL;
}


/**********************************************************************
 * test_freq_words()
 *
 * Tests a word against the frequent word dawg
 **********************************************************************/
int Dict::test_freq_words(const char *word) {
  return (word_in_dawg (frequent_words, word));
}
}  // namespace tesseract
