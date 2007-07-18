/* -*-C-*-
 ********************************************************************************
 *
 * File:        permnum.c  (Formerly permnum.c)
 * Description:
 * Author:       Mark Seaman, OCR Technology
 * Created:      Fri Oct 16 14:37:00 1987
 * Modified:     Tue Jul  2 14:12:43 1991 (Mark Seaman) marks@hpgrlt
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
#include "const.h"
#include "permnum.h"
#include "debug.h"
#include "permute.h"
#include "dawg.h"
#include "tordvars.h"
#include "stopper.h"
#include "globals.h"

#include <math.h>
#include <ctype.h>

/*----------------------------------------------------------------------
              V a r i a b l e s
----------------------------------------------------------------------*/
#if 0
static const char *allowed_alpha_strs[] = {
  "jan", "feb", "mar", "apr", "may", "jun",
  "jul", "aug", "sep", "oct", "nov", "dec", NULL
};
#endif

#if 0
static const char *allowed_char_strs[] = {
  "adfjmnos", "aceopu", "bcglnrptvy"
};
#endif

const int kNumStates = 7;

static int number_state_table[kNumStates][8] = { {
                                 /* 0. Beginning of string        */
  /*	l	d	o	a	t	1	2	3                    */
    0, 1, 1, -99, -99, 4, -99, -99
  },
  {                              /* 1. After a digit or operator  */
    -99, 1, 1, 3, 2, 4, 3, 3
  },
  {                              /* 2. After trailing punctuation */
    -99, -99, 1, -99, 2, -99, -99, -99
  },
  {                              /* 3. After a alpha character    */
    -99, -99, 3, 3, 2, 3, 3, 3
  },
  {                              /* 4. After 1st  char */
    -99, -1, -1, -99, -2, -99, 5, -99
  },
  {                              /* 5. After 2nd  char */
    -99, -1, -1, -99, -2, -99, -99, 6
  },
  {                              /* 6. After 3rd  char */
    -99, -1, -1, -99, -2, -99, -99, -99
  }
};

// The state is coded with its true state shifted left by kStateShift.
// A repeat count (starting with 0) is stored in the lower bits
// No state is allowed to occur more than kMaxRepeats times.
const int kStateShift = 4;
const int kRepeatMask = (1 << kStateShift) - 1;

const int kMaxRepeats[kNumStates] = {
  3, 10, 3, 3, 3, 3, 3
};

make_float_var (good_number, GOOD_NUMBER, make_good_number,
8, 15, set_good_number, "Good number adjustment");

make_float_var (ok_number, OK_NUMBER, make_ok_number,
8, 16, set_ok_number, "Bad number adjustment");

make_toggle_var (number_debug, 0, make_number_debug,
8, 23, set_number_debug, "Number debug");

make_int_var (number_depth, 3, make_number_depth,
8, 24, set_number_depth, "Number depth");

/*----------------------------------------------------------------------
              M a c r o s
----------------------------------------------------------------------*/
/**********************************************************************
 * isleading
 *
 * Return non-zero if this is a leading type punctuation mark for the
 * numeric grammar.
 **********************************************************************/

#define isleading(ch)     \
((ch == '{'  ) ||       \
	(ch == '['  ) ||       \
	(ch == '('  ) ||       \
	(ch == '#'  ) ||       \
	(ch == '@'  ) ||       \
	(ch == '$'  ))

/**********************************************************************
 * istrailing
 *
 * Return non-zero if this is a leading type punctuation mark for the
 * numeric grammar.
 **********************************************************************/

#define istrailing(ch)    \
((ch == '}'  ) ||       \
	(ch == ']'  ) ||       \
	(ch == ')'  ) ||       \
	(ch == ';'  ) ||       \
	(ch == ':'  ) ||       \
	(ch == ','  ) ||       \
	(ch == '.'  ) ||       \
	(ch == '%'  ))

/**********************************************************************
 * isoperator
 *
 * Return non-zero if this is a leading type punctuation mark for the
 * numeric grammar.
 **********************************************************************/

#define isoperator(ch)    \
((ch == '*'  ) ||       \
	(ch == '+'  ) ||       \
	(ch == '-'  ) ||       \
	(ch == '/'  ) ||       \
	(ch == '.'  ) ||       \
	(ch == ':'  ) ||       \
	(ch == ','  ))

/*----------------------------------------------------------------------
              F u n c t i o n s
----------------------------------------------------------------------*/
/**********************************************************************
 * adjust_number
 *
 * Assign an adjusted value to a string that is a word.  The value
 * that this word choice has is based on case and punctuation rules.
 **********************************************************************/
void adjust_number(A_CHOICE *best_choice, float *certainty_array) {
  float adjust_factor;

  if (adjust_debug)
    cprintf ("Number: %s %4.2f ",
      class_string (best_choice), class_probability (best_choice));

  class_probability (best_choice) += RATING_PAD;
  if (pure_number (class_string (best_choice), class_lengths (best_choice))) {
    class_probability (best_choice) *= good_number;
    adjust_factor = good_number;
    if (adjust_debug)
      cprintf (", %4.2f ", good_number);
  }
  else {
    class_probability (best_choice) *= ok_number;
    adjust_factor = ok_number;
    if (adjust_debug)
      cprintf (", N, %4.2f ", ok_number);
  }

  class_probability (best_choice) -= RATING_PAD;
  LogNewWordChoice(best_choice, adjust_factor, certainty_array);
  if (adjust_debug)
    cprintf (" --> %4.2f\n", class_probability (best_choice));
}


/**********************************************************************
 * append_number_choices
 *
 * Check to see whether or not the next choice is worth appending to
 * the string being generated.  If so then keep going deeper into the
 * word.
 **********************************************************************/
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
                           CHOICES *result) {
  int word_ending = FALSE;
  int x;
  int offset;

  if (char_index == (array_count (choices) - 1))
    word_ending = TRUE;
  strcpy(word + unichar_offsets[char_index], class_string (this_choice));

  unichar_lengths[char_index] = strlen(class_string (this_choice));
  unichar_lengths[char_index + 1] = 0;
  unichar_offsets[char_index + 1] = unichar_offsets[char_index] +
      unichar_lengths[char_index];

  if (word[unichar_offsets[char_index]] == '\0') {
    word[unichar_offsets[char_index]] = ' ';
    word[unichar_offsets[char_index] + 1] = '\0';
    unichar_lengths[char_index] = 1;
    unichar_lengths[char_index + 1] = 0;
    unichar_offsets[char_index + 1] = unichar_offsets[char_index] +
        unichar_lengths[char_index];
  }

  certainty_array[char_index] = class_certainty (this_choice);

  rating += class_probability (this_choice);
  certainty = min (class_certainty (this_choice), certainty);

  if (rating < *limit) {

    state = number_state_change (state, word + unichar_offsets[char_index],
                                 unichar_lengths + char_index);
    if (number_debug)
      cprintf ("%s prob=%4.2f  state=%d\n", word, rating, state);

    if (state != -1) {

      if ((state >> kStateShift) == 3 &&
          char_index + 3 < array_count (choices)) {
        return;
      }

      if (word_ending) {
        for (x = 0, offset = 0; x <= char_index; offset += unichar_lengths[x++]) {
          if (unicharset.get_isdigit (word + offset, unichar_lengths[x])) {
            if (number_debug)
              cprintf ("new choice = %s\n", word);
            push_on (*result, new_choice (word, unichar_lengths, rating, certainty,
              -1, NUMBER_PERM));
            adjust_number ((A_CHOICE *) first_node (*result),
              certainty_array);
            if (best_probability (*result) > *limit) {
              free_choice (first_node (*result));
              pop_off(*result);
            }
            else {
              *limit = best_probability (*result);
              break;
            }
          }
        }
      }
      else {
        JOIN_ON (*result,
          number_permute (state, choices, char_index + 1, limit,
          word, unichar_lengths, unichar_offsets, rating, certainty,
          certainty_array));
      }
    }
  }
  else {
    if (number_debug)
      cprintf ("pruned word (%s, rating=%4.2f, limit=%4.2f)\n",
        word, rating, *limit);
  }
}


/**********************************************************************
 * init_permute
 *
 * Initialize anything that needs to be set up for the permute
 * functions.
 **********************************************************************/
void init_permnum() {
  make_good_number();
  make_ok_number();
  make_number_debug();
  make_number_depth();
}


/**********************************************************************
 * number_character_type
 *
 * Decide which type of a character (with regard to the numeric state
 * table) we are looking at.
 **********************************************************************/
int number_character_type(  //current state
                          const char* ch,
                          int length,
                          int state) {
  if (unicharset.get_isalpha (ch, length)) {
#if 0
    if (state < 4
        && strchr (allowed_char_strs[0], lower_char) != NULL)
      return 5;
    else if (state == 4
        && strchr (allowed_char_strs[1], lower_char) != NULL)
      return 6;
    else if (state == 5
        && strchr (allowed_char_strs[2], lower_char) != NULL)
      return 7;
#endif
    return 3;
  }
  else if (unicharset.get_isdigit (ch, length))
    return (1);
  else if (length == 1 && isoperator (*ch))
    return (2);
  else if (length == 1 && istrailing (*ch))
    return (4);
  else if (length == 1 && isleading (*ch))
    return (0);
  else
    return (-1);
}


/**********************************************************************
 * number_state_change
 *
 * Execute a state transition according to the state table and
 * additional rules.
 **********************************************************************/
int number_state_change(int state,             //current state
                        const char *word,      //current char
                        const char *lengths) { //length of current char
  int char_type;                 //type of char
  int new_state;                 //state to return
  int old_state = state >> kStateShift;
  int repeats = state & kRepeatMask;
#if 0
  int index;
  char copy_word[4];             //tolowered chars
#endif

  char_type = number_character_type (word, *lengths, old_state);
  if (char_type == -1)
    return -1;
  new_state = number_state_table[old_state][char_type];
  if (new_state == old_state) {
    ++repeats;
    if (repeats >= kMaxRepeats[old_state])
      return -1;
  } else {
    repeats = 0;
  }
  if (new_state >= 0)
    return (new_state << kStateShift) | repeats;
  if (new_state == -99)
    return -1;

  //now check to see if the last state-3 chars in the word
  //make an allowable word. For now only 3 letter words
  //are allowed
  if (old_state != 6)
    return -1;                   //only 3 letters now
#if 0
  copy_word[0] = tolower (word[-3]);
  copy_word[1] = tolower (word[-2]);
  copy_word[2] = tolower (word[-1]);
  copy_word[3] = '\0';
  for (index = 0; allowed_alpha_strs[index] != NULL; index++) {
    if (strcmp (copy_word, allowed_alpha_strs[index]) == 0)
      return (-new_state) << kStateShift;
  }
#endif
  return -1;                     //not a good word
}


/**********************************************************************
 * number_permute
 *
 * Permute all the valid string that match the 'grammar' of numbers.
 * The valid syntax for numbers is encoded in a state table.  The
 * permuter uses this state table to enumerate all the string that
 * can be produced using the input choices.
 **********************************************************************/
CHOICES number_permute(int state,
                       CHOICES_LIST choices,
                       int char_index,
                       float *limit,
                       char *word,
                       char unichar_lengths[],
                       int unichar_offsets[],
                       float rating,
                       float certainty,
                       float *certainty_array) {
  CHOICES result = NIL;
  CHOICES c;
  int depth = 0;

  if (number_debug) {
    cprintf ("number_permute (state=%d, char_index=%d, limit=%4.2f, ",
      state, char_index, *limit);
    cprintf ("word=%s, rating=%4.2f, certainty=%4.2f)\n",
      word, rating, certainty);
  }
  if (char_index < array_count (choices)) {
    iterate_list (c, (CHOICES) array_index (choices, char_index)) {
      if (depth++ < number_depth)
        append_number_choices (state, word, unichar_lengths, unichar_offsets,
                               choices, char_index,
                               (A_CHOICE *) first_node (c), limit, rating,
                               certainty, certainty_array, &result);
    }
  }
  if (result && number_debug == 1)
    print_choices ("number_permute:", result);
  return (result);
}


/**********************************************************************
 * number_permute_and_select
 *
 * Permute all the possible valid numbers and adjust their ratings.
 * Save the best rating.
 **********************************************************************/
A_CHOICE *number_permute_and_select(CHOICES_LIST char_choices,
                                    float rating_limit) {
  CHOICES result = NIL;
  char word[UNICHAR_LEN * MAX_WERD_LENGTH + 1];
  char unichar_lengths[MAX_WERD_LENGTH + 1];
  int unichar_offsets[MAX_WERD_LENGTH + 1];
  float certainty_array[MAX_WERD_LENGTH + 1];
  float rating = rating_limit;
  A_CHOICE *best_choice;

  best_choice = new_choice (NULL, NULL, MAXFLOAT, -MAXFLOAT, -1, NO_PERM);

  if (array_count (char_choices) <= MAX_WERD_LENGTH) {
    word[0] = '\0';
    unichar_lengths[0] = 0;
    unichar_offsets[0] = 0;
    result = number_permute (0, char_choices, 0, &rating,
      word, unichar_lengths, unichar_offsets, 0.0, 0.0, certainty_array);

    if (display_ratings && result)
      print_choices ("number_permuter", result);

    while (result != NIL) {
      if (best_probability (result) < class_probability (best_choice)) {
        clone_choice (best_choice, first_node (result));
      }
      free_choice (first_node (result));
      pop_off(result);
    }
  }
  return (best_choice);
}


/**********************************************************************
 * pure_number
 *
 * Check to see if this string is a pure number (one that does not end
 * with alphabetic characters).
 **********************************************************************/
int pure_number(const char *string, const char *lengths) {
  int x;
  int offset;

  x = strlen (lengths) - 1;
  offset = strlen (string) - lengths[x];
  for (;x >= 0; offset -= lengths[--x]) {
    if (unicharset.get_isdigit (string + offset, lengths[x])) {
      return (TRUE);
    }
    else if (unicharset.get_isalpha (string + offset, lengths[x]))
      return (FALSE);
  }
  return (FALSE);
}


/**********************************************************************
 * valid_number
 *
 * Check this string to see if it is a valid number.  Return TRUE if
 * it is.
 **********************************************************************/
int valid_number(const char *string, const char *lengths) {
  int state = 0;
  int char_index;
  int offset;
  int num_chars = strlen (lengths);
  int num_digits = 0;

  for (char_index = 0, offset = 0; char_index < num_chars;
       offset += lengths[char_index++]) {

    state = number_state_change (state, string + offset, lengths + char_index);
    if (state == -1)
      return (FALSE);
    if (unicharset.get_isdigit (string + offset, lengths[char_index]))
      num_digits++;
  }
  return num_digits > num_chars - num_digits;
}
