/* -*-C-*-
 ********************************************************************************
 *
 * File:        choices.h  (Formerly choices.h)
 * Description:  Handle the new ratings choices for Wise Owl
 * Author:       Mark Seaman, OCR Technology
 * Created:      Fri Sep 22 14:05:51 1989
 * Modified:     Fri Jan  4 12:04:01 1991 (Mark Seaman) marks@hpgrlt
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
 ********************************************************************************
 *
 *                         FUNCTIONS TO CALL
 *                         -----------------
 * append_char_choice - Create a new choice for a character and add it to the list.
 * class_rating       - Return the rating of a given character class.
 * class_string       - Return the string corresponding to a character choice.
 * free_choice        - Free up the memory taken by one choice rating.
 * new_choice         - Create one choice record one set up the fields.
 *
 *********************************************************************************/

#ifndef CHOICES_H
#define CHOICES_H

#include <stdio.h>
#include <string.h>

#include "oldlist.h"
#include "unicharset.h"

/*----------------------------------------------------------------------
                T y p e s
----------------------------------------------------------------------*/
typedef LIST CHOICES;            /* CHOICES */
//typedef float PROBABILITY;       /* PROBABILITY */
//typedef char PERM_TYPE;          /* PERMUTER CODE */

typedef struct choicestruct
{                                /* A_CHOICE */
  float rating;
  float certainty;
  char permuter;
  inT8 config;
  char *string;
  char *lengths;           //<  length of each unichar in the string
  int script_id;
  char *fragment_lengths;  //<  length of fragments for each unichar in string
  /** 
   * if true, indicates that this choice
   * was chosen over a better one that
   * contained a fragment
   */
  bool fragment_mark;

} A_CHOICE;

/*----------------------------------------------------------------------
                M a c r o s
----------------------------------------------------------------------*/
/**
 * best_string
 *
 * Return the string corresponding to the best choice.
 */
#define best_string(choices)  \
(first_node (choices) ? ((A_CHOICE*) (first_node (choices)))->string : NULL)

/**
 * best_lengths
 *
 * Return the lengths corresponding to the best choice.
 */
#define best_lengths(choices)  \
(first_node (choices) ? ((A_CHOICE*) (first_node (choices)))->lengths : NULL)

/**
 * best_rating
 *
 * Return the rating of the best choice.
 */
#define best_rating(choices)  \
(((A_CHOICE*) (first_node (choices)))->rating)

/**
 * best_certainty
 *
 * Return the certainty of the best choice.
 */
#define best_certainty(choices)  \
(((A_CHOICE*) (first_node (choices)))->certainty)

/**
 * class_rating
 *
 * Return the rating of a given character class.
 */
#define class_rating(choice)  \
(((A_CHOICE*) (choice))->rating)

/**
 * class_certainty
 *
 * Return the certainty of a given character class.
 */
#define class_certainty(choice)  \
(((A_CHOICE*) (choice))->certainty)

/**
 * class_string
 *
 * Return the string of a given character class.
 */
#define class_string(choice)  \
(((A_CHOICE*) (choice))->string)

/**
 * class_lengths
 *
 * Return the lengths of a given character class.
 */
#define class_lengths(choice)  \
(((A_CHOICE*) (choice))->lengths)

/**
 * class_permuter
 *
 * Return the permuter of a given character class.
 */
#define class_permuter(choice)  \
(((A_CHOICE*) (choice))->permuter)

/**
 * class_config
 *
 * Return the config of a given character class.
 */
#define class_config(choice)  \
(((A_CHOICE*) (choice))->config)

/**
 * class_script
 *
 * Return the script of a given character class.
 */
#define class_script_id(choice)  \
(((A_CHOICE*) (choice))->script_id)

/**
 * free_choices
 *
 * Free a list of choices.
 */
#define free_choices(c)  \
destroy_nodes ((c), free_choice)

/**
 * print_bold
 *
 * Print a string in bold type by using escape sequences.  This only
 * works for certain output devices.
 */
#define print_bold(string)               \
cprintf ("\033&dB%s\033&d@", string)


/*----------------------------------------------------------------------
              F u n c t i o n s
----------------------------------------------------------------------*/

/// Returns true if fragment_mark is set for the given choice.
inline bool class_fragment_mark(A_CHOICE *choice) {
  return choice->fragment_mark;
}

/// Sets fragment_mark of choice to the given value.
inline void set_class_fragment_mark(A_CHOICE *choice, bool mark) {
  choice->fragment_mark = mark;
}

/// Returns fragment_lengths of the given class.
inline const char *class_fragment_lengths(A_CHOICE *choice) {
  return choice->fragment_lengths;
}

CHOICES append_char_choice(CHOICES ratings,
                           const char *string,
                           const char *lengths,
                           float rating,
                           float certainty,
                           inT8 config,
                           int script_id);

CHOICES copy_choices(CHOICES choices);

/// Copy the given values into corresponding fields of choice.
void clone_choice(A_CHOICE *choice, const char *string,
                  const char *lengths, float rating, float certainty,
                  inT8 permuter, bool fragment_mark,
                  const char *fragment_lengths);

/// Copy the contents of choice_1 into choice_2.
inline void clone_choice(A_CHOICE *choice_2, A_CHOICE *choice_1) {
  clone_choice(choice_2, class_string(choice_1), class_lengths(choice_1),
               class_rating(choice_1), class_certainty(choice_1),
               class_permuter(choice_1), class_fragment_mark(choice_1),
               class_fragment_lengths(choice_1));
}

void clear_choice(A_CHOICE *choice);

void free_choice(void *arg);

A_CHOICE *get_best_free_other(A_CHOICE *choice_1, A_CHOICE *choice_2);

A_CHOICE *new_choice(const char *string,
                     const char *lengths,
                     float rating,
                     float certainty,
                     inT8 config,
                     int script_id,
                     char permuter,
                     bool fragment_mark,
                     const char *fragment_lengths);

A_CHOICE *new_choice(const char *string,
                     const char *lengths,
                     float rating,
                     float certainty,
                     inT8 config,
                     char permuter);

#endif
