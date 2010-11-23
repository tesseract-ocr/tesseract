/* -*-C-*-
 ********************************************************************************
 *
 * File:        closed.c  (Formerly closed.c)
 * Description:  Hash table for closed search states.
 * Author:       Mark Seaman, OCR Technology
 * Created:      Fri Oct 16 14:37:00 1987
 * Modified:     Fri May 25 11:31:16 1990 (Mark Seaman) marks@hpgrlt
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
#include "freelist.h"
#include "closed.h"
#include "cutil.h"
#include "callcpp.h"
#ifdef __UNIX__
#include <assert.h>
#endif

/*----------------------------------------------------------------------
              V a r i a b l e s
----------------------------------------------------------------------*/
#define TABLE_SIZE 2000

/*----------------------------------------------------------------------
              F u n c t i o n s
----------------------------------------------------------------------*/
/**
 * @name hash_add
 *
 * Look in the hash table for a particular value. If it is not there
 * then add it.
 */
int hash_add(HASH_TABLE state_table, STATE *state) {
  int x;
  int i = 0;
  int table_limit = TABLE_SIZE;

  x = state->part2 % table_limit;
  while (i < table_limit) {
    assert (0 <= x && x < table_limit);
    /* Found it */
    if ((state_table[x].part2 == state->part2) &&
    (state_table[x].part1 == state->part1)) {
      return (FALSE);
    }
    /* Not in table */
    else if (state_table[x].part1 == NO_STATE) {
      state_table[x].part2 = state->part2;
      state_table[x].part1 = state->part1;
      return (TRUE);
    }
    i++;
    if (++x >= table_limit)
      x = 0;
  }
  cprintf("warning: hash table is full");

  abort();
  return 0;
}


/**
 * @name hash_lookup
 *
 * Look in the hash table for a particular value. If the value is there
 * then return TRUE, FALSE otherwise.
 */
int hash_lookup(HASH_TABLE state_table, STATE *state) {
  int x;
  int i = 0;
  int table_limit = TABLE_SIZE;

  x = state->part2 % table_limit;
  while (i < table_limit) {
    assert (0 <= x && x < table_limit);
    /* Found it */
    if ((state_table[x].part2 == state->part2) &&
    (state_table[x].part1 == state->part1)) {
      return (TRUE);
    }
    /* Not in table */
    else if (state_table[x].part1 == NO_STATE) {
      return (FALSE);
    }

    i++;
    if (++x >= table_limit)
      x = 0;
  }
  cprintf ("warning: fell off end of hash table  (%x) %x\n",
    state->part2, state->part2 % table_limit);
  abort();
  return 0;
}


/**
 * @name new_hash_table
 *
 * Create and initialize a hash table.
 */
HASH_TABLE new_hash_table() {
  HASH_TABLE ht;
  int x;

  ht = (HASH_TABLE) memalloc (TABLE_SIZE * sizeof (STATE));
  for (x = 0; x < TABLE_SIZE; x++) {
    ht[x].part1 = NO_STATE;
    ht[x].part2 = NO_STATE;
  }
  return (ht);
}
