/* -*-C-*-
 ********************************************************************************
 *
 * File:        tovars.c  (Formerly to-vars.c)
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
#include "debug.h"

/*----------------------------------------------------------------------
              V a r i a b l e s
----------------------------------------------------------------------*/
FILE *rawfile;                   /* Text before dictionary */
FILE *textfile;                  /* Text output file */
FILE *matcher_fp;                //matcher log
FILE *correct_fp;                //correct text

int write_output;                /* Text file output */
int write_raw_output;            /* Text before context */

int similarity_enable = 0;
int similarity_debug = 0;

make_float_var (certainty_threshold, -2.25, make_certainty_threshold,
4, 5, set_certainty_value, "Certainty Value");

make_int_var (num_word_choices, 30, make_num_word_choices,
4, 6, set_num_choices, "Number of choices");

make_toggle_var (blob_skip, 0, make_blob_skip,
4, 7, toggle_skip, "Skip to Next selection");

make_float_var (overlap_threshold, 0.33, make_overlap_threshold,
9, 7, set_overlap, "Overlap Threshold");

make_toggle_var (debug_3, 0, make_debug_3, 6, 3, toggle_debug_3, "Debug #3");

make_toggle_var (debug_5, 0, make_debug_5, 6, 5, toggle_debug_5, "Debug #5");

make_toggle_var (debug_8, 0, make_debug_8, 6, 8, toggle_debug_8, "Debug #8");

make_toggle_var (display_ratings, 0, make_display_ratings,
6, 9, toggle_ratings, "Ratings display");

make_toggle_var (display_text, 0, make_display_text,
6, 10, toggle_text, "Display Text");

make_toggle_var (show_bold, 1, make_show_bold,
6, 17, set_show_bold, "Show Bold Text");

/*----------------------------------------------------------------------
              F u n c t i o n s
----------------------------------------------------------------------*/
/**********************************************************************
 * init_textord_vars
 *
 * Create variables to be used by the texord section of code.
 **********************************************************************/
void init_textord_vars() {
  int_variable (write_output, "write_output", 0);
  int_variable (write_raw_output, "write_raw_output", 0);
  make_certainty_threshold();
  make_num_word_choices();
  make_blob_skip();
  make_overlap_threshold();
  make_show_bold();

  make_debug_3();
  make_debug_5();
  make_debug_8();

  make_display_ratings();
  make_display_text();
}
