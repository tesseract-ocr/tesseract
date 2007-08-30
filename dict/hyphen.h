/* -*-C-*-
 ********************************************************************************
 *
 * File:        hyphen.h  (Formerly hyphen.h)
 * Description:
 * Author:       Mark Seaman, SW Productivity
 * Created:      Fri Oct 16 14:37:00 1987
 * Modified:     Mon Jan 14 17:52:50 1991 (Mark Seaman) marks@hpgrlt
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
#ifndef HYPHEN_H
#define HYPHEN_H

/*----------------------------------------------------------------------
              I n c l u d e s
----------------------------------------------------------------------*/
#include "choices.h"
#include "emalloc.h"
#include "dawg.h"

/*----------------------------------------------------------------------
              V a r i a b l e s
----------------------------------------------------------------------*/
extern int last_word_on_line;
extern char *hyphen_string;
extern char *hyphen_unichar_lengths;
extern int *hyphen_unichar_offsets;
extern float hyphen_rating;
extern NODE_REF hyphen_state;

/*----------------------------------------------------------------------
              M a c r o s
----------------------------------------------------------------------*/
/**********************************************************************
 * set_last_word
 *
 * Set the flag that indicated that this is the last word on a line.
 **********************************************************************/

#define set_last_word()  \
last_word_on_line = TRUE

/**********************************************************************
 * reset_hyphen_word
 *
 * Erase the hyphenation word that my have been stored at this location.
 **********************************************************************/

#define reset_hyphen_word()                      \
if (last_word_on_line == FALSE) {              \
	if (hyphen_string) strfree (hyphen_string); \
	if (hyphen_unichar_lengths) strfree (hyphen_unichar_lengths); \
	if (hyphen_unichar_offsets) Efree (hyphen_unichar_offsets); \
	hyphen_string = NULL;                       \
	hyphen_unichar_lengths = NULL;                       \
	hyphen_unichar_offsets = NULL;                       \
	hyphen_rating = MAX_FLOAT32;                   \
	hyphen_state = 0;                           \
}                                              \


/**********************************************************************
 * reset_last_word
 *
 * Reset the flag that indicated that this is the last word on a line.
 **********************************************************************/

#define reset_last_word()  \
last_word_on_line = FALSE

/**********************************************************************
 * is_last_word
 *
 * Test the flag that indicated that this is the last word on a line.
 **********************************************************************/

#define is_last_word()  \
(last_word_on_line)

/**********************************************************************
 * hyphen_base_size
 *
 * Size of the base word (the part on the line before) of a hyphenated
 * coumpound word.
 **********************************************************************/

#define hyphen_base_size()                 \
((! is_last_word () && hyphen_string) ?  \
	(strlen (hyphen_unichar_lengths))             :  \
	(0))                                    \


/**********************************************************************
 * hyphen_tail
 *
 * Return the a pointer to the part of the word that was not on the
 * previous line.  This routine is used for words that were split
 * between lines and hyphenated.
 **********************************************************************/

#define hyphen_tail(word)        \
(&word[hyphen_base_size() > 0 ? \
      (hyphen_unichar_offsets[hyphen_base_size() - 1] + \
       hyphen_unichar_lengths[hyphen_base_size() - 1]) : 0]) \

/*----------------------------------------------------------------------
          Public Function Prototypes
----------------------------------------------------------------------*/
void set_hyphen_word(char *word, char *unichar_lengths, int *unichar_offsets,
                     float rating, NODE_REF state);
#endif
