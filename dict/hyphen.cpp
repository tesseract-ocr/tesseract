/* -*-C-*-
 ********************************************************************************
 *
 * File:        hyphen.c  (Formerly hyphen.c)
 * Description:
 * Author:       Mark Seaman, OCR Technology
 * Created:      Fri Oct 16 14:37:00 1987
 * Modified:     Thu Mar 14 11:09:43 1991 (Mark Seaman) marks@hpgrlt
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
#include "hyphen.h"
#include "tordvars.h"
#include "callcpp.h"
#include <math.h>

/*----------------------------------------------------------------------
              V a r i a b l e s
----------------------------------------------------------------------*/
int last_word_on_line = 0;
char *hyphen_string = 0;
char *hyphen_unichar_lengths = 0;
int *hyphen_unichar_offsets = NULL;
float hyphen_rating = MAXFLOAT;
NODE_REF hyphen_state = 0;

/*----------------------------------------------------------------------
              F u n c t i o n s
---------------------------------------------------------------------*/
/**********************************************************************
 * set_hyphen_word
 *
 * If this hyphenated word choice is better than the last one then add
 * it as the new word choice.  This string can be used on the next
 * line to permute the other half of the word.
 **********************************************************************/
void set_hyphen_word(char *word, char *unichar_lengths, int *unichar_offsets,
                     float rating, NODE_REF state) {
  int char_index = strlen (unichar_lengths) - 1;

  if (display_ratings)
    cprintf ("set hyphen word = %s\n", word);

  if (hyphen_rating > rating && char_index > 0) {
    word[unichar_offsets[char_index]] = '\0';
    unichar_lengths[char_index] = 0;

    if (hyphen_string)
    {
      strfree(hyphen_string);
      strfree(hyphen_unichar_lengths);
      Efree(hyphen_unichar_offsets);
    }
    hyphen_string = strsave (word);
    hyphen_unichar_lengths = strsave (unichar_lengths);
    hyphen_unichar_offsets = (int *)
        Emalloc((strlen(unichar_lengths)) * sizeof (int));
    memcpy(hyphen_unichar_offsets, unichar_offsets,
           (strlen(unichar_lengths)) * sizeof (int));

    hyphen_state = state;
    hyphen_rating = rating;

    word[unichar_offsets[char_index]] = '-';
    unichar_lengths[char_index] = 1;
  }
}
