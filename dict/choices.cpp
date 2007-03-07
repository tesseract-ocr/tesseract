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
#include "callcpp.h"
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
                      float rating,
                      float certainty,
                      INT8 config) {
  A_CHOICE *this_choice;

  this_choice = new_choice (string, rating, certainty, config, NO_PERM);
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
      (LIST) new_choice (class_string (first (l)),
      class_probability (first (l)),
      class_certainty (first (l)),
      class_config (first (l)),
      class_permuter (first (l))));
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
    oldchoice(this_choice); 
  }
}


/**********************************************************************
 * new_choice
 *
 * Create a new choice record. Store the string value in a safe place.
 **********************************************************************/
A_CHOICE *new_choice(const char *string,
                     float rating,
                     float certainty,
                     INT8 config,
                     char permuter) {
  A_CHOICE *this_choice;

  this_choice = newchoice ();
  this_choice->string = strsave (string);
  this_choice->rating = rating;
  this_choice->certainty = certainty;
  this_choice->config = config;
  this_choice->permuter = permuter;
  return (this_choice);
}


/**********************************************************************
 * print_choices
 *
 * Print the probability ratings for a particular blob or word.
 **********************************************************************/
void print_choices(  /* List of (A_CHOICE*) */
                   const char *label,
                   CHOICES rating) {
  int first_one = TRUE;
  char str[CHARS_PER_LINE];
  int len;

  cprintf ("%-20s\n", label);
  if (rating == NIL)
    cprintf (" No rating ");

  iterate(rating) { 

    if (first_one && show_bold) {
      cprintf ("|");
      len = sprintf (str, " %s ", best_string (rating));
      print_bold(str); 
      while (len++ < 8)
        cprintf (" ");
    }
    else {
      cprintf ("| %-7s", best_string (rating));
    }

    cprintf ("%5.2lf ", best_probability (rating));

    cprintf ("%5.2lf", best_certainty (rating));
    first_one = FALSE;
  }
  cprintf ("\n");
}
