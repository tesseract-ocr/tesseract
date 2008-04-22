/* -*-C-*-
 ********************************************************************************
 *
 * File:        context.c  (Formerly context.c)
 * Description:  Context checking functions
 * Author:       Mark Seaman, OCR Technology
 * Created:      Thu Feb 15 11:18:24 1990
 * Modified:     Tue Jul  9 17:38:16 1991 (Mark Seaman) marks@hpgrlt
 * Language:     C
 * Package:      N/A
 * Status:       Experimental (Do Not Distribute)
 *
 * (c) Copyright 1990, Hewlett-Packard Company.
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
#include "context.h"
#include "tordvars.h"
#include "callcpp.h"
#include "globals.h"

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <math.h>

// Initialize probability_in_context to point to a default implementation (a
// main program can override this).
PROBABILITY_IN_CONTEXT_FUNCTION probability_in_context = &def_probability_in_context;

double def_probability_in_context(const char* context,
                                  int context_bytes,
                                  const char* character,
                                  int character_bytes) {
  (void) context;
  (void) context_bytes;
  (void) character;
  (void) character_bytes;
  return 0.0;
}

/*----------------------------------------------------------------------
              V a r i a b l e s
----------------------------------------------------------------------*/
static FILE *choice_file = NULL; /* File to save choices */

/*----------------------------------------------------------------------
              F u n c t i o n s
----------------------------------------------------------------------*/
/**********************************************************************
 * close_choices
 *
 * Close the choices file.
 **********************************************************************/
void close_choices() {
  if (choice_file)
    fclose(choice_file);
}


/**********************************************************************
 * fix_quotes
 *
 * Fix up two single quote to make them two double quotes.
 **********************************************************************/
void fix_quotes(char *str) {
  int i;
  for (i = 0; i < strlen (str); i++) {

    if (((str[i] == '\'') || (str[i] == '`')) &&
    ((str[i + 1] == '\'') || (str[i + 1] == '`'))) {
      str[i] = '\"';
      strcpy (str + i + 1, str + i + 2);
    }
  }
}


/**********************************************************************
 * punctuation_ok
 *
 * Check a string to see if it matches a set of punctuation rules.
 **********************************************************************/
int punctuation_ok(const char *word, const char *lengths) {
  int punctuation_types[5];
  int trailing = 0;
  int num_puncts = 0;
  register int x;
  int offset;
  UNICHAR_ID ch_id;

  for (x = 0; x < 5; x++)
    punctuation_types[x] = 0;

  // check for un-supported symbols
  for (x = 0, offset = 0; x < strlen (lengths); offset += lengths[x++]) {
    // a un-supported symbol
    if (!unicharset.contains_unichar (word + offset, lengths[x])) {
      return -1;
    }
  }

  for (x = 0, offset = 0; x < strlen (lengths); offset += lengths[x++]) {
    if (unicharset.get_isalpha (word + offset, lengths[x])) {
      if (trailing &&
        !(unicharset.get_isalpha (word + offset - lengths[x - 1], lengths[x - 1])
#if 0
          ||
        (word[x - 1] == '\'' &&
        (word[x] == 's' || word[x] == 'd' || word[x] == 'l')) ||
        (word[x - 1] == '-')
#endif
          ))
        return (-1);
      trailing = 1;
    }
    else {
      ch_id = unicharset.unichar_to_id(word + offset, lengths[x]);

      if (unicharset.eq(ch_id, ".") && trailing) {
        if (punctuation_types[0])
          return (-1);
        (punctuation_types[0])++;
      }

      else if (((unicharset.eq(ch_id, "{")) ||
                (unicharset.eq(ch_id, "[")) ||
                (unicharset.eq(ch_id, "("))) && !trailing) {
        if (punctuation_types[1])
          return (-1);
        (punctuation_types[1])++;
      }

      else if (((unicharset.eq(ch_id, "}")) ||
                (unicharset.eq(ch_id, "]")) ||
                (unicharset.eq(ch_id, ")"))) && trailing) {
        if (punctuation_types[2])
          return (-1);
        (punctuation_types[2])++;
      }

      else if (((unicharset.eq(ch_id, ":")) ||
                (unicharset.eq(ch_id, ";")) ||
                (unicharset.eq(ch_id, "!")) ||
                (unicharset.eq(ch_id, "-")) ||
                (unicharset.eq(ch_id, ",")) ||
                (unicharset.eq(ch_id, "?"))) && trailing) {
        if (punctuation_types[3])
          return (-1);
        (punctuation_types[3])++;
        if (unicharset.eq(ch_id, "-"))
          punctuation_types[3] = 0;
      }

      else if (x < strlen(lengths) - 1 &&
               ((unicharset.eq(ch_id, "`")) ||
               (unicharset.eq(ch_id, "\"")) ||
               (unicharset.eq(ch_id, "\'")))) {
        UNICHAR_ID ch_id2 = unicharset.unichar_to_id(word + offset + lengths[x],
                                                     lengths[x + 1]);
        if ((unicharset.eq(ch_id2, "`")) ||
            (unicharset.eq(ch_id2, "\'"))) {
          offset += lengths[x++];
        }
        (punctuation_types[4])++;
        if (punctuation_types[4] > 2)
          return (-1);
      }

      else if (!unicharset.get_isdigit (ch_id))
        return (-1);
    }
  }

  for (x = 0; x < 5; x++) {
    if (punctuation_types[x])
      num_puncts++;
  }

  return (num_puncts);
}


/**********************************************************************
 * case_ok
 *
 * Check a string to see if it matches a set of lexical rules.
 **********************************************************************/
int case_ok(const char *word, const char *lengths) {
  static int case_state_table[6][4] = { {
                                 /*  0. Begining of word         */
    /*    P   U   L   D                                     */
    /* -1. Error on case            */
      0, 1, 5, 4
    },
    {                            /*  1. After initial capital    */
      0, 3, 2, 4
    },
    {                            /*  2. After lower case         */
      0, -1, 2, -1
    },
    {                            /*  3. After upper case         */
      0, 3, -1, 4
    },
    {                            /*  4. After a digit            */
      0, -1, -1, 4
    },
    {                            /*  5. After initial lower case */
      5, -1, 2, -1
    },
  };

  register int last_state = 0;
  register int state = 0;
  register int x;
  int offset;
  UNICHAR_ID ch_id;

  for (x = 0, offset = 0; x < strlen (lengths); offset += lengths[x++]) {

    ch_id = unicharset.unichar_to_id(word + offset, lengths[x]);
    if (unicharset.get_isupper(ch_id))
      state = case_state_table[state][1];
    else if (unicharset.get_isalpha(ch_id))
      state = case_state_table[state][2];
    else if (unicharset.get_isdigit(ch_id))
      state = case_state_table[state][3];
    else
      state = case_state_table[state][0];

    if (debug_3)
      cprintf ("Case state = %d, char = %s\n", state,
               unicharset.id_to_unichar(ch_id));
    if (state == -1) {
                                 /* Handle ACCRONYMs */
#if 0
      if (word[x] == 's' &&
        !isalpha (word[x + 1]) && !isdigit (word[x + 1]))
        state = last_state;
      else
#endif
        return (FALSE);
    }

    last_state = state;
  }
  return state != 5;             /*single lower is bad */
}


/**********************************************************************
 * write_choice_line
 *
 * Write a blank line to the choices file.  This will indicate that
 * there is a new word that is following.
 **********************************************************************/
void write_choice_line() {
  if (choice_file) {
    fprintf (choice_file, "\n");
    fflush(choice_file);
  }
}
