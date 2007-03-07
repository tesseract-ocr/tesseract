/* -*-C-*-
 ********************************************************************************
 *
 * File:        permdawg.c  (Formerly permdawg.c)
 * Description:
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
#include "dawg.h"
#include <ctype.h>

/*----------------------------------------------------------------------
              T y p e s
----------------------------------------------------------------------*/
#define FREQ_WERD     1.0
#define GOOD_WERD     1.1
#define OK_WERD       1.25
#define MAX_FREQ_EDGES    1000
#define NO_RATING              -1

/*----------------------------------------------------------------------
              V a r i a b l e s
----------------------------------------------------------------------*/
static EDGE_ARRAY frequent_words;
static float rating_margin;
static float rating_pad = 5.0;

make_toggle_var (dawg_debug, 0, make_dawg_debug,
8, 10, set_dawg_debug, "DAWG Debug ");

make_float_var (ok_word, OK_WERD, make_ok_word,
8, 17, set_ok_word, "Bad word adjustment");

make_float_var (good_word, GOOD_WERD, make_good_word,
8, 18, set_good_word, "Good word adjustment");

make_float_var (freq_word, FREQ_WERD, make_freq_word,
8, 19, set_freq_word, "Freq word adjustment");

//extern char *demodir;

/*----------------------------------------------------------------------
              F u n c t i o n s
----------------------------------------------------------------------*/
/**********************************************************************
 * adjust_word
 *
 * Assign an adjusted value to a string that is a word.	The value
 * that this word choice has is based on case and punctuation rules.
 **********************************************************************/
void adjust_word(A_CHOICE *best_choice, float *certainty_array) {
  char *this_word;
  int punct_status;
  float adjust_factor;

  if (adjust_debug)
    cprintf ("%s %4.2f ",
      class_string (best_choice), class_probability (best_choice));

  this_word = class_string (best_choice);
  punct_status = punctuation_ok (this_word);

  class_probability (best_choice) += RATING_PAD;
  if (case_ok (this_word) && punct_status != -1) {
    if (punct_status < 1 && word_in_dawg (frequent_words, this_word)) {
      class_probability (best_choice) *= freq_word;
      class_permuter (best_choice) = FREQ_DAWG_PERM;
      adjust_factor = freq_word;
      if (adjust_debug)
        cprintf (", F, %4.2f ", freq_word);
    }
    else {
      class_probability (best_choice) *= good_word;
      adjust_factor = good_word;
      if (adjust_debug)
        cprintf (", %4.2f ", good_word);
    }
  }
  else {
    class_probability (best_choice) *= ok_word;
    adjust_factor = ok_word;
    if (adjust_debug) {
      if (!case_ok (this_word))
        cprintf (", C");
      if (punctuation_ok (this_word) == -1)
        cprintf (", P");
      cprintf (", %4.2f ", ok_word);
    }
  }

  class_probability (best_choice) -= RATING_PAD;

  LogNewWordChoice(best_choice, adjust_factor, certainty_array);

  if (adjust_debug)
    cprintf (" --> %4.2f\n", class_probability (best_choice));
}


/**********************************************************************
 * append_next_choice
 *
 * Check to see whether or not the next choice is worth appending to
 * the string being generated.  If so then keep going deeper into the
 * word.
 **********************************************************************/
void append_next_choice(  /*previous option */
                        EDGE_ARRAY dawg,
                        NODE_REF node,
                        char permuter,
                        char *word,
                        CHOICES_LIST choices,
                        int char_index,
                        A_CHOICE *this_choice,
                        char prevchar,
                        float *limit,
                        float rating,
                        float certainty,
                        float *rating_array,
                        float *certainty_array,
                        int word_ending,
                        int last_word,
                        CHOICES *result) {
  A_CHOICE *better_choice;
  /* Add new character */
  word[char_index] = class_string (this_choice)[0];
  word[char_index + 1] = 0;
  if (word[char_index] == 0)
    word[char_index] = ' ';
  certainty_array[char_index] = class_certainty (this_choice);

  rating += class_probability (this_choice);
  certainty = min (class_certainty (this_choice), certainty);

  if (rating_array[char_index] == NO_RATING) {
                                 /* Prune bad subwords */
    rating_array[char_index] = rating;
  }
  else {
    if (rating_array[char_index] * rating_margin + rating_pad < rating) {
      if (dawg_debug)
        cprintf ("early pruned word (%s, rating=%4.2f, limit=%4.2f)\n",
          word, rating, *limit);
      return;
    }
  }

  /* Deal with hyphens */
  if (word_ending && last_word && word[char_index] == '-' && char_index > 0) {
    *limit = rating;
    if (dawg_debug)
      cprintf ("new hyphen choice = %s\n", word);

    better_choice = new_choice (word, rating, certainty, -1, permuter);
    adjust_word(better_choice, certainty_array);
    push_on(*result, better_choice);
    set_hyphen_word(word, rating, node);
  }
  /* Look up char in DAWG */
  else if (letter_is_okay (dawg, &node, char_index, prevchar,
  word, word_ending)) {
    /* Add a new word choice */
    if (word_ending) {
      if (dawg_debug == 1)
        cprintf ("new choice = %s\n", word);
      *limit = rating;

      better_choice = new_choice (hyphen_tail (word), rating, certainty,
        -1, permuter);
      adjust_word (better_choice, &certainty_array[hyphen_base_size ()]);
      push_on(*result, better_choice);
    }
    else {
                                 /* Search the next letter */
      JOIN_ON (*result,
        dawg_permute (dawg, node, permuter,
        choices, char_index + 1, limit,
        word, rating, certainty,
        rating_array, certainty_array, last_word));
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
CHOICES dawg_permute(EDGE_ARRAY dawg,
                     NODE_REF node,
                     char permuter,
                     CHOICES_LIST choices,
                     int char_index,
                     float *limit,
                     char *word,
                     float rating,
                     float certainty,
                     float *rating_array,
                     float *certainty_array,
                     int last_word) {
  CHOICES result = NIL;
  CHOICES c;
  char *prevchar;
  int word_ending = FALSE;

  if (dawg_debug) {
    cprintf ("dawg_permute (node=%d, char_index=%d, limit=%4.2f, ",
      node, char_index, *limit);
    cprintf ("word=%s, rating=%4.2f, certainty=%4.2f)\n",
      word, rating, certainty);
  }
  /* Check for EOW */
  if (1 + char_index == array_count (choices) + hyphen_base_size ())
    word_ending = TRUE;

  if (char_index < array_count (choices) + hyphen_base_size ()) {
    prevchar = NULL;
    iterate_list (c,
      (CHOICES) array_index (choices,
    char_index - hyphen_base_size ())) {
      append_next_choice (dawg, node, permuter, word, choices, char_index,
        (A_CHOICE *) first (c),
        prevchar != NULL ? *prevchar : '\0', limit,
        rating, certainty, rating_array, certainty_array,
        word_ending, last_word, &result);
      prevchar = best_string (c);
    }
  }

  if (result && (dawg_debug == 1))
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
void dawg_permute_and_select(const char *string,
                             EDGE_ARRAY dawg,
                             char permuter,
                             CHOICES_LIST character_choices,
                             A_CHOICE *best_choice,
                             INT16 system_words) {
  CHOICES result = NIL;
  char word[MAX_WERD_LENGTH + 1];
  float certainty_array[MAX_WERD_LENGTH + 1];
  float rating_array[MAX_WERD_LENGTH + 1];
  float rating;
  int char_index;
  NODE_REF dawg_node = 0;

                                 /* Pruning margin ratio */
  rating_margin = ok_word / good_word;

  word[0] = '\0';
  rating = class_probability (best_choice);

  for (char_index = 0; char_index < MAX_WERD_LENGTH + 1; char_index++)
    rating_array[char_index] = NO_RATING;
  char_index = 0;

  if (!is_last_word () && hyphen_string) {
    strcpy(word, hyphen_string);
    char_index = strlen (hyphen_string);
    if (system_words)
      dawg_node = hyphen_state;
  }
  result = dawg_permute (dawg, dawg_node, permuter, character_choices,
    char_index, &rating, word, 0.0, 0.0,
    rating_array, certainty_array, is_last_word ());

  if (display_ratings && result)
    print_choices(string, result);

  while (result != NIL) {
    if (best_probability (result) < class_probability (best_choice)) {
      clone_choice (best_choice, first (result));
    }
    free_choice (first (result));
    pop_off(result);
  }
}


/**********************************************************************
 * init_permdawg
 *
 * Initialize the variables needed by this file.
 **********************************************************************/
void init_permdawg() {
  char name[1024];
  make_dawg_debug();
  make_ok_word();
  make_good_word();
  make_freq_word();

  frequent_words = (EDGE_ARRAY) memalloc (sizeof (EDGE_RECORD) *
    MAX_FREQ_EDGES);
  strcpy(name, demodir);
  strcat (name, "tessdata/freq-dawg");
  read_squished_dawg(name, frequent_words, MAX_FREQ_EDGES);
}

void end_permdawg() {
  memfree(frequent_words);
  frequent_words = NULL;
}

/**********************************************************************
 * test_freq_words()
 *
 * Tests a word against the frequent word dawg
 **********************************************************************/
int test_freq_words(const char *word) {
  return (word_in_dawg (frequent_words, word));
}
