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
#include "structures.h"
#include "tordvars.h"
#include "tprintf.h"
#include "globals.h"
#include "danerror.h"
#include "host.h"

/*----------------------------------------------------------------------
            Variables
------------------------------------------------------------------------*/
#define CHOICEBLOCK 100          /*  Cells per block */

makestructure (newchoice, oldchoice, printchoice, A_CHOICE,
freechoice, CHOICEBLOCK, "A_CHOICE", choicecount)
/*----------------------------------------------------------------------
              F u n c t i o n s
----------------------------------------------------------------------*/
/**********************************************************************
 * append_choice
 *
 * Create a new choice record. Store the string value in a safe place.
 * Add the new choice record to the list.
 *
 * NB - This is only used by matchers, so permuter is always NO_PERM
 * SPC 16/9/92
 **********************************************************************/
CHOICES append_choice(CHOICES ratings,
                      const char *string,
                      const char *lengths,
                      float rating,
                      float certainty,
                      inT8 config) {
  A_CHOICE *this_choice;

  this_choice = new_choice (string, lengths, rating, certainty, config, NO_PERM);
  ratings = push_last (ratings, (LIST) this_choice);
  return (ratings);
}

CHOICES append_choice(CHOICES ratings,
                      const char *string,
                      const char *lengths,
                      float rating,
                      float certainty,
                      inT8 config,
                      const char* script) {
  A_CHOICE *this_choice;

  this_choice = new_choice (string, lengths, rating, certainty, config,
                            script, NO_PERM);
  ratings = push_last (ratings, (LIST) this_choice);
  return (ratings);
}


/**********************************************************************
 * copy_choices
 *
 * Copy a list of choices.  This means that there will be two copies
 * in memory.
 **********************************************************************/
CHOICES copy_choices(CHOICES choices) {
  CHOICES l;
  CHOICES result = NIL;

  iterate_list(l, choices) {
    result = push (result,
      (LIST) new_choice (class_string (first_node (l)),
                         class_lengths (first_node (l)),
                         class_probability (first_node (l)),
                         class_certainty (first_node (l)),
                         class_config (first_node (l)),
                         class_script (first_node (l)),
                         class_permuter (first_node (l))));
  }
  return (reverse_d (result));
}


/**********************************************************************
 * free_choice
 *
 * Free up the memory taken by one choice rating.
 **********************************************************************/
void free_choice(void *arg) {  //LIST choice)
  A_CHOICE *this_choice;
  LIST choice = (LIST) arg;

  this_choice = (A_CHOICE *) choice;
  if (this_choice) {
    if (this_choice->string)
      strfree (this_choice->string);
    if (this_choice->lengths)
      strfree (this_choice->lengths);
    oldchoice(this_choice);
  }
}


/**********************************************************************
 * new_choice
 *
 * Create a new choice record. Store the string value in a safe place.
 **********************************************************************/
A_CHOICE *new_choice(const char *string,
                     const char *lengths,
                     float rating,
                     float certainty,
                     inT8 config,
                     char permuter) {
  return new_choice(string, lengths, rating, certainty,
                    config, "dummy", permuter);
}

A_CHOICE *new_choice(const char *string,
                     const char *lengths,
                     float rating,
                     float certainty,
                     inT8 config,
                     const char* script,
                     char permuter) {
  A_CHOICE *this_choice;

  this_choice = newchoice ();
  this_choice->string = strsave (string);
  this_choice->lengths = strsave (lengths);
  this_choice->rating = rating;
  this_choice->certainty = certainty;
  this_choice->config = config;
  this_choice->permuter = permuter;
  this_choice->script = script;
  return (this_choice);
}


/**********************************************************************
 * print_choices
 *
 * Print the probability ratings for a particular blob or word.
 **********************************************************************/
void print_choices(const char *label,
                   CHOICES rating) {   // List of (A_CHOICE*).
  tprintf("%s\n", label);
  if (rating == NIL)
    tprintf(" No rating ");

  iterate(rating) {
    tprintf("%.2f %.2f", best_probability(rating), best_certainty(rating));
    print_word_string(best_string(rating));
  }
  tprintf("\n");
}

/**********************************************************************
 * print_word_choice
 *
 * Print the string in a human-readable format and ratings for a word.
 **********************************************************************/
void print_word_choice(const char *label, A_CHOICE* choice) {
  tprintf("%s : ", label);
  if (choice == NULL) {
    tprintf("No rating\n");
  } else {
    tprintf("%.2f %.2f", class_probability(choice), class_certainty(choice));
    print_word_string(class_string(choice));
    tprintf("\n");
  }
}

/**********************************************************************
 * print_word_string
 *
 * Print the string in a human-readable format.
 * The output is not newline terminated.
 **********************************************************************/
void print_word_string(const char* str) {
  int step = 1;
  for (int i = 0; str[i] != '\0'; i += step) {
    step = unicharset.step(str + i);
    int unichar_id = unicharset.unichar_to_id(str + i, step);
    STRING ch_str = unicharset.debug_str(unichar_id);
    tprintf(" : %s ", ch_str.string());
  }
}

