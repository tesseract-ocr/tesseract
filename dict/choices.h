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
 * append_choice     - Create a new choice and add it to the list.
 * class_probability - Return the probability of a given character class.
 * class_string      - Return the string corresponding to a character choice.
 * free_choice       - Free up the memory taken by one choice rating.
 * new_choice        - Create one choice record one set up the fields.
 *
 *********************************************************************************/

#ifndef CHOICES_H
#define CHOICES_H

#include <stdio.h>
#include <string.h>

#include "oldlist.h"

/*----------------------------------------------------------------------
                T y p e s
----------------------------------------------------------------------*/
typedef LIST CHOICES;            /* CHOICES */
//typedef float PROBABILITY;                                                            /* PROBABILITY */
//typedef char PERM_TYPE;                                                                       /* PERMUTER CODE */

/* permuter codes used in A_CHOICEs for words */

#define NO_PERM       0
#define TOP_CHOICE_PERM  1
#define LOWER_CASE_PERM  2
#define UPPER_CASE_PERM  3
#define NUMBER_PERM      4
#define SYSTEM_DAWG_PERM 5
#define DOC_DAWG_PERM    6
#define USER_DAWG_PERM   7
#define FREQ_DAWG_PERM   8
#define COMPOUND_PERM    9

typedef struct choicestruct
{                                /* A_CHOICE */
  float rating;
  float certainty;
  char permuter;
  INT8 config;
  char *string;
} A_CHOICE;

/*----------------------------------------------------------------------
                M a c r o s
----------------------------------------------------------------------*/
/**********************************************************************
 * best_string
 *
 * Return the string corresponding to the best choice.
 **********************************************************************/
#define best_string(choices)  \
(first (choices) ? ((A_CHOICE*) (first (choices)))->string : NULL)

/**********************************************************************
 * best_probability
 *
 * Return the probability of the best choice.
 **********************************************************************/
#define best_probability(choices)  \
(((A_CHOICE*) (first (choices)))->rating)

/**********************************************************************
 * best_certainty
 *
 * Return the certainty of the best choice.
 **********************************************************************/
#define best_certainty(choices)  \
(((A_CHOICE*) (first (choices)))->certainty)

/**********************************************************************
 * class_probability
 *
 * Return the probability of a given character class.
 **********************************************************************/
#define class_probability(choice)  \
(((A_CHOICE*) (choice))->rating)

/**********************************************************************
 * class_certainty
 *
 * Return the certainty of a given character class.
 **********************************************************************/
#define class_certainty(choice)  \
(((A_CHOICE*) (choice))->certainty)

/**********************************************************************
 * class_string
 *
 * Return the probability of a given character class.
 **********************************************************************/
#define class_string(choice)  \
(((A_CHOICE*) (choice))->string)

/**********************************************************************
 * class_permuter
 *
 * Return the permuter of a given character class.
 **********************************************************************/
#define class_permuter(choice)  \
(((A_CHOICE*) (choice))->permuter)

/**********************************************************************
 * class_config
 *
 * Return the config of a given character class.
 **********************************************************************/
#define class_config(choice)  \
(((A_CHOICE*) (choice))->config)

/**********************************************************************
 * clone_choice
 *
 * Copy the contents of this choice record onto another replacing any
 * previous value it might of had.
 **********************************************************************/
#define clone_choice(choice_2,choice_1)  \
if (class_string (choice_2)) strfree (class_string (choice_2));    \
class_probability (choice_2) = class_probability (choice_1);       \
class_certainty   (choice_2) = class_certainty   (choice_1);       \
class_permuter    (choice_2) = class_permuter   (choice_1);        \
class_string      (choice_2) = strsave (class_string (choice_1))   \


/**********************************************************************
 * free_choices
 *
 * Free a list of choices.
 **********************************************************************/
#define free_choices(c)  \
destroy_nodes ((c), free_choice)

/**********************************************************************
 * print_bold
 *
 * Print a string in bold type by using escape sequences.  This only
 * works for certain output devices.
 **********************************************************************/
#define print_bold(string)               \
	cprintf ("\033&dB%s\033&d@", string)

/*----------------------------------------------------------------------
              F u n c t i o n s
----------------------------------------------------------------------*/
CHOICES append_choice(CHOICES ratings,
                      const char *string,
                      float rating,
                      float certainty,
                      INT8 config);

CHOICES copy_choices(CHOICES choices); 

void free_choice(void *arg);  //LIST choice);

A_CHOICE *new_choice(const char *string,
                     float rating,
                     float certainty,
                     INT8 config,
                     char permuter);

void print_choices(const char *label, CHOICES rating); 
#endif
