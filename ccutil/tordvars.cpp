/* -*-C-*-
 ********************************************************************************
 *
 * File:         tordvars.cpp
 * Description:  Text Ordering Control Variables
 * Author:       Mark Seaman, OCR Technology
 * Created:      Wed Jan 17 12:47:29 1990
 * Modified:     Tue Jul 30 16:22:40 1991 (Mark Seaman) marks@hpgrlt
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
/*----------------------------------------------------------------------
              I n c l u d e s
----------------------------------------------------------------------*/
#include "mfcpch.h"

#include  <stdio.h>

#include "varable.h"

/*----------------------------------------------------------------------
              V a r i a b l e s
----------------------------------------------------------------------*/
FILE *rawfile;                   /* Text before dictionary */
FILE *textfile;                  /* Text output file */
FILE *matcher_fp;                //matcher log
FILE *correct_fp;                //correct text

BOOL_VAR(tord_write_output, 0, "Text file output");

BOOL_VAR(tord_write_raw_output, 0, "Text before context");

BOOL_VAR(tord_similarity_enable, 0, "Switch for Similarity");

double_VAR(tord_certainty_threshold, -2.25, "Certainty Value");

INT_VAR(tord_num_word_choices, 30, "Number of choices");

BOOL_VAR(tord_blob_skip, 0, "Skip to Next selection");

double_VAR(tord_overlap_threshold, 0.33, "Overlap Threshold");

BOOL_VAR(tord_debug_3, 0, "Textord Debug #3");

BOOL_VAR(tord_debug_5, 0, "Textord Debug #5");

BOOL_VAR(tord_debug_8, 0, "Textord Debug #8");

INT_VAR(tord_display_ratings, 0, "Ratings display");

BOOL_VAR(tord_display_text, 0, "Display Text");

BOOL_VAR(tord_show_bold, 1, "Show Bold Text");
