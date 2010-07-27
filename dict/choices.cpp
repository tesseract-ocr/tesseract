/* -*-C-*-
 ********************************************************************************
 *
 * File:        choices.c  (Formerly choices.c)
 * Description:  Handle the new ratings choices for Wise Owl
 * Author:       Mark Seaman, OCR Technology
 * Created:      Fri Sep 22 14:05:51 1989
 * Modified:     Wed May 22 14:12:34 1991 (Mark Seaman) marks@hpgrlt
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
 *********************************************************************************/
#include "choices.h"
#include "emalloc.h"
#include "globals.h"
#include "host.h"
#include "danerror.h"
#include "structures.h"
#include "tordvars.h"
#include "tprintf.h"
#include "unicharset.h"
#include "dict.h"
#include "image.h"

/*----------------------------------------------------------------------
            Variables
------------------------------------------------------------------------*/
#define CHOICEBLOCK 100          /*  Cells per block */

makestructure (newchoice, oldchoice, printchoice, A_CHOICE,
freechoice, CHOICEBLOCK, "A_CHOICE", choicecount)
/*----------------------------------------------------------------------
              F u n c t i o n s
----------------------------------------------------------------------*/
/**
 * append_char_choice
 *
 * Create a new choice record. Store the string value in a safe place.
 * Add the new choice record to the list.
 *
 * NB - This is only used by matchers, so permuter is always NO_PERM
 * SPC 16/9/92
 */
CHOICES append_char_choice(CHOICES ratings,
                           const char *string,
                           const char *lengths,
                           float rating,
                           float certainty,
                           inT8 config,
                           int script_id) {
  A_CHOICE *this_choice;

  this_choice = new_choice (string, lengths, rating, certainty,
                            config, script_id, NO_PERM, false, NULL);
  ratings = push_last (ratings, (LIST) this_choice);
  return (ratings);
}

/**
 * copy_choices
 *
 * Copy a list of choices.  This means that there will be two copies
 * in memory.
 */
CHOICES copy_choices(CHOICES choices) {
  CHOICES l;
  CHOICES result = NIL;

  iterate_list(l, choices) {
    A_CHOICE *choice = (A_CHOICE *)(first_node(l));
    result = push (result,
      (LIST) new_choice (class_string(choice),
                         class_lengths(choice),
                         class_rating(choice),
                         class_certainty(choice),
                         class_config(choice),
                         class_script_id(choice),
                         class_permuter(choice),
                         class_fragment_mark(choice),
                         class_fragment_lengths(choice)));
  }
  return (reverse_d (result));
}

/**
 * clone_choice
 *
 * Copy the contents of the given values to the corresponding values in
 * a given choice replacing any previous values it might have had.
 */
void clone_choice(A_CHOICE *choice, const char *string,
                  const char *lengths, float rating, float certainty,
                  inT8 permuter, bool fragment_mark,
                  const char *fragment_lengths) {
  if (choice->string) strfree (class_string (choice));
  if (choice->lengths) strfree (class_lengths (choice));
  if (choice->fragment_lengths) strfree(choice->fragment_lengths);

  choice->string = strsave (string);
  choice->lengths = strsave (lengths);
  choice->rating = rating;
  choice->certainty = certainty;
  choice->permuter = permuter;
  choice->fragment_mark = fragment_mark;
  choice->fragment_lengths =
    fragment_lengths ? strsave(fragment_lengths) : NULL;
}

/**
 * clear_choice
 *
 * Set the fields in this choice to be defaulted bad initial values.
 */
void clear_choice(A_CHOICE *choice) {
  choice->string = NULL;
  choice->lengths =  NULL;
  choice->rating =  MAX_FLOAT32;
  choice->certainty = -MAX_FLOAT32;
  choice->fragment_mark = false;
  choice->fragment_lengths = NULL;
}


/**
 * free_choice
 *
 * Free up the memory taken by one choice rating.
 */
void free_choice(void *arg) {  //LIST choice)
  A_CHOICE *this_choice;
  LIST choice = (LIST) arg;

  this_choice = (A_CHOICE *) choice;
  if (this_choice) {
    if (this_choice->string)
      strfree (this_choice->string);
    if (this_choice->lengths)
      strfree (this_choice->lengths);
    if (this_choice->fragment_lengths)
      strfree (this_choice->fragment_lengths);
    oldchoice(this_choice);
  }
}

/**
 * get_best_free_other
 *
 * Returns the best of two choices and frees the other (worse) choice.
 * A choice is better if it has a non-NULL string and has a lower rating
 * than the other choice.
 */
A_CHOICE *get_best_free_other(A_CHOICE *choice_1, A_CHOICE *choice_2) {
  if (!choice_1) return choice_2;
  if (!choice_2) return choice_1;
  if (class_rating (choice_1) < class_rating (choice_2) ||
      class_string (choice_2) == NULL) {
    free_choice(choice_2);
    return choice_1;
  } else {
    free_choice(choice_1);
    return choice_2;
  }
}

/**
 * new_choice
 *
 * Create a new choice record. Store the string value in a safe place.
 */
A_CHOICE *new_choice(const char *string,
                     const char *lengths,
                     float rating,
                     float certainty,
                     inT8 config,
                     int script_id,
                     char permuter,
                     bool fragment_mark,
                     const char *fragment_lengths) {
  A_CHOICE *this_choice;

  this_choice = newchoice();
  this_choice->string = strsave(string);
  this_choice->lengths = strsave(lengths);
  this_choice->rating = rating;
  this_choice->certainty = certainty;
  this_choice->config = config;
  this_choice->permuter = permuter;
  this_choice->script_id = script_id;
  this_choice->fragment_mark = fragment_mark;
  this_choice->fragment_lengths =
    fragment_lengths ? strsave(fragment_lengths) : NULL;

  return (this_choice);
}

A_CHOICE *new_choice(const char *string,
                     const char *lengths,
                     float rating,
                     float certainty,
                     inT8 config,
                     char permuter) {
  return new_choice(string, lengths, rating, certainty,
                    config, -1, permuter, false, NULL);
}


/**
 * print_choices
 *
 * Print the rating for a particular blob or word.
 */
namespace tesseract {
void Dict::print_choices(const char *label,
                         CHOICES choices) {   // List of (A_CHOICE*).
  tprintf("%s\n", label);
  if (choices == NIL)
    tprintf(" No rating ");

  iterate(choices) {
    tprintf("%.2f %.2f", best_rating(choices), best_certainty(choices));
    print_word_string(best_string(choices));
  }
  tprintf("\n");
}

/**
 * print_word_choice
 *
 * Print the string in a human-readable format and ratings for a word.
 */
void Dict::print_word_choice(const char *label, A_CHOICE* choice) {
  tprintf("%s : ", label);
  if (choice == NULL) {
    tprintf("No rating\n");
  } else {
    tprintf("%.2f %.2f", class_rating(choice), class_certainty(choice));
    print_word_string(class_string(choice));
    tprintf("\n");
  }
}

/**
 * print_word_string
 *
 * Print the string in a human-readable format.
 * The output is not newline terminated.
 */
void Dict::print_word_string(const char* str) {
  int step = 1;
  for (int i = 0; str[i] != '\0'; i += step) {
    step = (getUnicharset().get_fragment(str) ?
      strlen(str) : getUnicharset().step(str + i));
    int unichar_id = getUnicharset().unichar_to_id(str + i, step);
    tprintf(" : %s ", getUnicharset().debug_str(unichar_id).string());
  }
}
} // namespace tesseract
