/* -*-C-*-
 ********************************************************************************
 *
 * File:        associate.c  (Formerly associate.c)
 * Description:  Associate the outlines and classify them
 * Author:       Mark Seaman, OCR Technology
 * Created:      Tue Jan 30 14:03:25 1990
 * Modified:     Mon Jul 22 10:48:01 1991 (Mark Seaman) marks@hpgrlt
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
 ********************************************************************************
 */

/*----------------------------------------------------------------------
              I n c l u d e s
----------------------------------------------------------------------*/
#include <stdio.h>
#ifdef __UNIX__
#include <assert.h>
#endif

#include "associate.h"
#include "callcpp.h"

extern TBLOB *newblob();

/*----------------------------------------------------------------------
              V a r i a b l e s
----------------------------------------------------------------------*/

BOOL_VAR(wordrec_enable_assoc, 1, "Associator Enable");
BOOL_VAR(force_word_assoc, FALSE,
         "always force associator to run, independent of what enable_assoc is."
         "This is used for CJK where component grouping is necessary.");

EVALUATION_ARRAY last_segmentation;

/*----------------------------------------------------------------------
              F u n c t i o n s
----------------------------------------------------------------------*/

/**********************************************************************
 * print_eval_record
 **********************************************************************/
void print_eval_record(const char *label, EVALUATION_RECORD *rec) {
  if (rec) {
    cprintf ("%s: r%.2f c%.2f width=%d gap=%d\n", label,
             rec->match, rec->certainty, rec->width, rec->gap);
  }
}
