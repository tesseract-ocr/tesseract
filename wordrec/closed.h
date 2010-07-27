/* -*-C-*-
 ********************************************************************************
 *
 * File:        closed.h  (Formerly closed.h)
 * Description:  Hash table for closed search states.
 * Author:       Mark Seaman, SW Productivity
 * Created:      Fri Oct 16 14:37:00 1987
 * Modified:     Fri May 25 11:27:11 1990 (Mark Seaman) marks@hpgrlt
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

#ifndef CLOSED_H
#define CLOSED_H

/*----------------------------------------------------------------------
              I n c l u d e s
----------------------------------------------------------------------*/
#include <math.h>
#include "states.h"

/*----------------------------------------------------------------------
              T y p e s
----------------------------------------------------------------------*/
typedef STATE *HASH_TABLE;
#define NO_STATE ~0

/*----------------------------------------------------------------------
              V a r i a b l e s
----------------------------------------------------------------------*/
extern HASH_TABLE global_hash;

/*----------------------------------------------------------------------
              M a c r o s
----------------------------------------------------------------------*/
/**
 * free_hash_table
 *
 * Free the memory taken by a state variable.
 */
#define free_hash_table(table) \
	global_hash = table

/*---------------------------------------------------------------------
              F u n c t i o n s
----------------------------------------------------------------------*/
int hash_add(HASH_TABLE state_table, STATE *state); 

int hash_lookup(HASH_TABLE state_table, STATE *state); 

HASH_TABLE new_hash_table(); 
#endif
