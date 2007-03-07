/* -*-C-*-
 ********************************************************************************
 *
 * File:        tally.c  (Formerly tally.c)
 * Description:
 * Author:       Mark Seaman, OCR Technology
 * Created:      Fri Oct 16 14:37:00 1987
 * Modified:     Mon Apr  8 11:41:32 1991 (Mark Seaman) marks@hpgrlt
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
#include "tally.h"
#include "freelist.h"
#include "callcpp.h"

/*----------------------------------------------------------------------
              F u n c t i o n s
----------------------------------------------------------------------*/
/**********************************************************************
 * new_tally
 *
 * Create a new tally record and initialize it.
 **********************************************************************/
TALLY new_tally(int num_buckets) { 
  TALLY t;
  int x;

  t = (TALLY) memalloc ((num_buckets + 2) * sizeof (int));
  t->count = 0;
  t->num_buckets = num_buckets;

  iterate_tally (t, x) tally_entry (t, x) = 0;

  return (t);
}


/**********************************************************************
 * print_tally
 *
 * Print the results of a given tally.
 **********************************************************************/
void print_tally(FILE *file, const char *string, TALLY t) { 
  int x;

  cprintf ("%d %s buckets\n", t->num_buckets, string);
  cprintf ("%d samples in %s\n", t->count, string);

  iterate_tally (t, x)
    cprintf ("   %s [%d] = %d\n", string, x, tally_entry (t, x));
  cprintf ("\n");
}
