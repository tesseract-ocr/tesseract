/* -*-C-*-
 ********************************************************************************
 *
 * File:         tordvars.h
 * Description:  Text Ordering Control Variables
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

#include "varable.h"

/*----------------------------------------------------------------------
              V a r i a b l e s
----------------------------------------------------------------------*/
extern FILE *rawfile;                       /* Text before dictionary */
extern FILE *textfile;                      /* Text output file */
extern FILE *correct_fp;                    //correct text
extern FILE *matcher_fp;

extern BOOL_VAR_H(tord_write_output, 0, "Text file output");

extern BOOL_VAR_H(tord_write_raw_output, 0, "Text before context");

extern BOOL_VAR_H(tord_similarity_enable, 0, "Switch for Similarity");

extern double_VAR_H(tord_certainty_threshold, -2.25, "Certainty Value");

extern INT_VAR_H(tord_num_word_choices, 30, "Number of choices");

extern BOOL_VAR_H(tord_blob_skip, 0, "Skip to Next selection");

extern double_VAR_H(tord_overlap_threshold, 0.33, "Overlap Threshold");

extern BOOL_VAR_H(tord_debug_3, 0, "Textord Debug #3");

extern BOOL_VAR_H(tord_debug_5, 0, "Textord Debug #5");

extern BOOL_VAR_H(tord_debug_8, 0, "Textord Debug #8");

extern INT_VAR_H(tord_display_ratings, 0, "Ratings display");

extern BOOL_VAR_H(tord_display_text, 0, "Display Text");

extern BOOL_VAR_H(tord_show_bold, 1, "Show Bold Text");

#endif
