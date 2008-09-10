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
#include "ndminx.h"
#include "dict.h"
#include "image.h"
#include "ccutil.h"
#include "conversion.h"

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

double_VAR(segment_penalty_number_good, GOOD_NUMBER,
           "Score multiplier for good-looking numbers "
           "(lower is better).");

double_VAR(segment_penalty_number_ok, OK_NUMBER,
           "Score multiplier for ok-looking numbers "
           "(lower is better).");

BOOL_VAR(number_debug, 0, "Segmentation number debug mode");

INT_VAR(segment_digits_max, 3,
        "Maximum length of a number we will try to segment.");

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
namespace tesseract {
void Dict::adjust_number(A_CHOICE *best_choice, float *certainty_array) {
  float adjust_factor;

  if (segment_adjust_debug)
    cprintf ("Number: %s %4.2f ",
      class_string (best_choice), class_rating (best_choice));

  class_rating (best_choice) += RATING_PAD;
  if (pure_number (class_string (best_choice), class_lengths (best_choice))) {
    class_rating (best_choice) *= segment_penalty_number_good;
    adjust_factor = segment_penalty_number_good;
    if (segment_adjust_debug)
      cprintf (", %4.2f ", (double)segment_penalty_number_good);
  }
  else {
    class_rating (best_choice) *= segment_penalty_number_ok;
    adjust_factor = segment_penalty_number_ok;
    if (segment_adjust_debug)
      cprintf (", N, %4.2f ", (double)segment_penalty_number_ok);
  }

  class_rating (best_choice) -= RATING_PAD;
  LogNewWordChoice(best_choice, adjust_factor,
                   certainty_array, getUnicharset());
  if (segment_adjust_debug)
    cprintf (" --> %4.2f\n", class_rating (best_choice));
}


/**********************************************************************
 * append_number_choices
 *
 * Check to see whether or not the next choice is worth appending to
 * the string being generated.  If so then keep going deeper into the
 * word.
 **********************************************************************/
void Dict::append_number_choices(int state,
                                 char *word,
                                 char unichar_lengths[],
                                 int unichar_offsets[],
                                 CHOICES_LIST choices,
                                 int char_choice_index,
                                 int word_index,
                                 A_CHOICE *this_choice,
                                 float *limit,
                                 float rating,
                                 float certainty,
                                 float *certainty_array,
                                 const CHAR_FRAGMENT_INFO *prev_char_frag_info,
                                 char fragment_lengths[],
                                 CHOICES *result) {
  int x;
  int offset;
  CHAR_FRAGMENT_INFO char_frag_info;
  int word_ending =
    (char_choice_index == (array_count(choices) - 1)) ? TRUE : FALSE;
  const char *ch = NULL;

  /* Deal lwith fragments */
  if (!fragment_state_okay(
      getUnicharset().unichar_to_id(class_string(this_choice)),
      class_rating(this_choice), class_certainty(this_choice),
      prev_char_frag_info,
      (number_debug && (fragments_debug > 1)) ? "number_debug" : NULL,
      word_ending, &char_frag_info)) {
    return;  // this_choice must be an invalid fragment
  }
  if (char_frag_info.unichar_id != INVALID_UNICHAR_ID) {
    ch = getUnicharset().id_to_unichar(char_frag_info.unichar_id);
  }
  if (ch == NULL) {   // this character is a fragment
    JOIN_ON(*result,  // so search the next letter
            number_permute(state, choices, char_choice_index + 1,
                           word_index, limit, word, unichar_lengths,
                           unichar_offsets, rating, certainty, certainty_array,
                           &char_frag_info, fragment_lengths));
    return;
  }

  /* Add new character */
  strcpy(word + unichar_offsets[word_index], ch);

  unichar_lengths[word_index] = strlen(ch);
  unichar_lengths[word_index + 1] = 0;
  fragment_lengths[word_index] = char_frag_info.num_fragments;
  fragment_lengths[word_index + 1] = 0;
  unichar_offsets[word_index + 1] = unichar_offsets[word_index] +
      unichar_lengths[word_index];

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

  if (rating < *limit) {
    state = number_state_change (state, word + unichar_offsets[word_index],
                                 unichar_lengths + word_index);
    if (number_debug) {
      cprintf ("%s rating=%4.2f  state=%d\n", word, rating, state);
    }

    if (state != -1) {

      if ((state >> kStateShift) == 3 &&
          word_index + 3 < array_count (choices)) {
        return;
      }

      if (word_ending) {
        for (x = 0, offset = 0; x <= word_index; offset += unichar_lengths[x++]) {
          if (getUnicharset().get_isdigit (
              word + offset, unichar_lengths[x])) {
            if (number_debug) cprintf ("new choice = %s\n", word);
            push_on (*result, new_choice (word, unichar_lengths, rating,
                                          certainty, -1, NULL, NUMBER_PERM,
                                          false, fragment_lengths));

            adjust_number ((A_CHOICE *) first_node (*result), certainty_array);
            if (best_rating (*result) > *limit) {
              free_choice (first_node (*result));
              pop_off(*result);
            }
            else {
              *limit = best_rating (*result);
              break;
            }
          }
        }
      }
      else {
        JOIN_ON (*result,
          number_permute (state, choices, char_choice_index + 1,
                          word_index + 1, limit, word, unichar_lengths,
                          unichar_offsets, rating, certainty,
                          certainty_array, &char_frag_info, fragment_lengths));
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
 * number_character_type
 *
 * Decide which type of a character (with regard to the numeric state
 * table) we are looking at.
 **********************************************************************/
int Dict::number_character_type(  //current state
                                  const char* ch,
                                  int length,
                                  int state) {
  if (getUnicharset().get_isalpha (ch, length)) {
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
  else if (getUnicharset().get_isdigit (ch, length))
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
int Dict::number_state_change(int state,             //current state
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
CHOICES Dict::number_permute(int state,
                             CHOICES_LIST choices,
                             int char_choice_index,
                             int word_index,
                             float *limit,
                             char *word,
                             char unichar_lengths[],
                             int unichar_offsets[],
                             float rating,
                             float certainty,
                             float *certainty_array,
                             const CHAR_FRAGMENT_INFO *prev_char_frag_info,
                             char fragment_lengths[]) {
  CHOICES result = NIL;
  CHOICES c;
  int depth = 0;

  if (number_debug) {
    cprintf ("number_permute (state=%d, char_choice_index=%d,"
             " word_index=%d, limit=%4.2f, ", state, char_choice_index,
             word_index, *limit);
    cprintf ("word=%s, rating=%4.2f, certainty=%4.2f)\n",
      word, rating, certainty);
  }
  if (char_choice_index < array_count (choices)) {
    iterate_list (c, (CHOICES) array_index (choices, char_choice_index)) {
      if (depth++ < segment_digits_max)
        append_number_choices (state, word, unichar_lengths, unichar_offsets,
                               choices, char_choice_index, word_index,
                               (A_CHOICE *) first_node (c), limit, rating,
                               certainty, certainty_array, prev_char_frag_info,
                               fragment_lengths, &result);
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
A_CHOICE *Dict::number_permute_and_select(CHOICES_LIST char_choices,
                                          float rating_limit) {
  CHOICES result = NIL;
  char word[UNICHAR_LEN * MAX_WERD_LENGTH + 1];
  char unichar_lengths[MAX_WERD_LENGTH + 1];
  int unichar_offsets[MAX_WERD_LENGTH + 1];
  float certainty_array[MAX_WERD_LENGTH + 1];
  char fragment_lengths[MAX_WERD_LENGTH + 1];  // num fragments from which
                                               // each char was constructed
  float rating = rating_limit;
  A_CHOICE *best_choice;

  best_choice = new_choice (NULL, NULL, MAXFLOAT, -MAXFLOAT, -1, NO_PERM);

  if (array_count (char_choices) <= MAX_WERD_LENGTH) {
    word[0] = '\0';
    unichar_lengths[0] = 0;
    fragment_lengths[0] = 0;
    unichar_offsets[0] = 0;
    result = number_permute (0, char_choices, 0, 0, &rating, word,
                             unichar_lengths, unichar_offsets,
                             0.0, 0.0, certainty_array, NULL, fragment_lengths);
    if (display_ratings && result)
      print_choices ("number_permuter", result);

    while (result != NIL) {
      if (best_rating (result) < class_rating (best_choice)) {
        clone_choice (best_choice, (A_CHOICE *) first_node (result));
      }
      free_choice (first_node (result));
      pop_off(result);
    }
  }
  return (best_choice);
}
}  // namespace tesseract


/**********************************************************************
 * pure_number
 *
 * Check to see if this string is a pure number (one that does not end
 * with alphabetic characters).
 **********************************************************************/
namespace tesseract {
int Dict::pure_number(const char *string, const char *lengths) {
  int x;
  int offset;

  x = strlen (lengths) - 1;
  offset = strlen (string) - lengths[x];
  for (;x >= 0; offset -= lengths[--x]) {
    if (getUnicharset().get_isdigit (string + offset, lengths[x])) {
      return (TRUE);
    }
    else if (getUnicharset().get_isalpha (string + offset, lengths[x]))
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
int Dict::valid_number(const char *string, const char *lengths) {
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
    if (getUnicharset().get_isdigit (string + offset, lengths[char_index]))
      num_digits++;
  }
  return num_digits > num_chars - num_digits;
}
}  // namespace tesseract
