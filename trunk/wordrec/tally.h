/* -*-C-*-
 ********************************************************************************
 *
 * File:        tally.h  (Formerly tally.h)
 * Description:
 * Author:       Mark Seaman, SW Productivity
 * Created:      Fri Oct 16 14:37:00 1987
 * Modified:     Wed Apr 10 10:45:41 1991 (Mark Seaman) marks@hpgrlt
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
#ifndef TALLY_H
#define TALLY_H

/*----------------------------------------------------------------------
              I n c l u d e s
----------------------------------------------------------------------*/
#include <stdio.h>

/*----------------------------------------------------------------------
              T y p e s
----------------------------------------------------------------------*/
typedef struct _TALLY_
{
  int count;
  int num_buckets;
  int buckets[1];
} *TALLY;

/*----------------------------------------------------------------------
              M a c r o s
----------------------------------------------------------------------*/
/**********************************************************************
 * inc_tally_bucket
 *
 * Increment the bucket count for the chosen bucket.
 **********************************************************************/

#define inc_tally_bucket(t,i) \
(t->count++,                \
	((i < t->num_buckets) ?    \
	(t->buckets[i]++) :       \
	(t->buckets[t->num_buckets-1]++)))

/**********************************************************************
 * iterate_tally
 *
 * Iterate through all the buckets in a tally record.
 **********************************************************************/

#define iterate_tally(t,i)  \
for (i=0; i<t->num_buckets; i++)

/**********************************************************************
 * tally_entry
 *
 * Access one of the buckets of a tally record without bounds checking.
 **********************************************************************/

#define tally_entry(t,i)            \
(t->buckets[i])

/**********************************************************************
 * tally_value
 *
 * Access one of the buckets of a tally record with bounds checking.
 **********************************************************************

#define tally_value(t,i)            \
((i>=0 && i<t->num_buckets) ?     \
  (tally_entry (t,i))   :          \
  (cprintf ("error: tried to access non-existant bucket %d\n", i)))
*/

/*----------------------------------------------------------------------
              F u n c t i o n s
----------------------------------------------------------------------*/
TALLY new_tally(int num_buckets); 

void print_tally(FILE *file, const char *string, TALLY t); 
#endif
