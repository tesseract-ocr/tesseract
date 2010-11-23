/* -*-C-*-
 ********************************************************************************
 *
 * File:        states.h  (Formerly states.h)
 * Description:  Representations of search states
 * Author:       Mark Seaman, OCR Technology
 * Created:      Wed May 16 15:52:40 1990
 * Modified:     Tue May 21 16:26:21 1991 (Mark Seaman) marks@hpgrlt
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
#ifndef STATES_H
#define STATES_H

/*----------------------------------------------------------------------
              I n c l u d e s
----------------------------------------------------------------------*/
#include "host.h"

/*----------------------------------------------------------------------
              T y p e s
----------------------------------------------------------------------*/
#define MAX_NUM_CHUNKS  64       /* Limit on pieces */

typedef struct
{
  uinT32 part1;
  uinT32 part2;
} STATE;

/** State variable for search */
typedef int *SEARCH_STATE;

/** State variable for search */
typedef uinT8 PIECES_STATE[MAX_NUM_CHUNKS + 2];

/*----------------------------------------------------------------------
              F u n c t i o n s
----------------------------------------------------------------------*/
SEARCH_STATE bin_to_chunks(STATE *state, int num_joints);

void bin_to_pieces(STATE *state, int num_joints, PIECES_STATE pieces);

void insert_new_chunk(register STATE *state,
                      register int index,
                      int num_joints);

STATE *new_state(STATE *oldstate);

int ones_in_state(STATE *state, int num_joints);

void print_state(const char *label, STATE *state, int num_joints);

void set_n_ones(STATE *state, int n);

extern void free_state(STATE *);

#endif
