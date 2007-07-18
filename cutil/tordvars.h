/* -*-C-*-
 ********************************************************************************
 *
 * File:        tovars.h  (Formerly to-vars.h)
 * Description:  Do word classification
 * Author:       Mark Seaman, OCR Technology
 * Created:      Wed Oct 25 16:33:01 1989
 * Modified:     Mon Jul  1 14:28:23 1991 (Mark Seaman) marks@hpgrlt
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
#ifndef TOVARS_H
#define TOVARS_H

#include <stdio.h>

/*----------------------------------------------------------------------
              V a r i a b l e s
----------------------------------------------------------------------*/
extern int debug_3;
extern int debug_5;
extern int debug_8;

extern FILE *rawfile;                       /* Text before dictionary */
extern FILE *textfile;                      /* Text output file */
extern FILE *correct_fp;                    //correct text
extern FILE *matcher_fp;

extern int blob_skip;                       /* Skip to next selection */
extern int num_word_choices;                /* How many words to keep */
extern int similarity_enable;               /* Switch for Similarity */
extern int similarity_debug;                /* Level of debug output */
extern int write_raw_output;                /* Text before context */
extern int write_output;                    /* Text file output */
//extern "C" { extern int display_ratings; }  /* Show the ratings */
extern int display_ratings;                 /* Show the ratings */
extern int show_bold;                       /* Use bold text */
extern int display_text;                    /* Show word text */
extern int display_blocks;                  /* Show word as boxes */

extern float overlap_threshold;             /* Overlap Threshold */
extern float certainty_threshold;           /* When to quit looking */

/*----------------------------------------------------------------------
              F u n c t i o n s
----------------------------------------------------------------------*/
void init_textord_vars();
#endif
