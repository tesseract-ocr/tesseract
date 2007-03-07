/* -*-C-*-
 ********************************************************************************
 *
 * File:        heuristic.h  (Formerly heuristic.h)
 * Description:
 * Author:       Mark Seaman, SW Productivity
 * Created:      Fri Oct 16 14:37:00 1987
 * Modified:     Tue Jul  9 17:14:44 1991 (Mark Seaman) marks@hpgrlt
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
#ifndef HEURISTIC_H
#define HEURISTIC_H

/*----------------------------------------------------------------------
              I n c l u d e s
----------------------------------------------------------------------*/
#include "associate.h"
#include "bestfirst.h"

/*----------------------------------------------------------------------
              F u n c t i o n s
----------------------------------------------------------------------*/
FLOAT32 prioritize_state(CHUNKS_RECORD *chunks_record,
                         SEARCH_RECORD *the_search,
                         STATE *old_state);

FLOAT32 rating_priority(CHUNKS_RECORD *chunks_record,
                        STATE *state,
                        STATE *old_state,
                        int num_joints);

WIDTH_RECORD *state_char_widths(WIDTH_RECORD *chunk_widths,
                                STATE *state,
                                int num_joints,
                                SEARCH_STATE *search_state);

FLOAT32 width_priority(CHUNKS_RECORD *chunks_record,
                       STATE *state,
                       int num_joints);

/*
#if defined(__STDC__) || defined(__cplusplus) || MAC_OR_DOS
# define	_ARGS(s) s
#else
# define	_ARGS(s) ()
#endif*/

/* heuristic.c
PROBABILITY best_char_rating
  _ARGS((CHUNKS_RECORD *chunks_record,
  int first_chunk,
  int last_chunk,
  char *word));

STATE *first_segmentation
  _ARGS((CHUNKS_RECORD *chunks_record));

FLOAT32 gap_priority
  _ARGS((CHUNKS_RECORD *chunks_record,
  STATE *state,
  int num_joints));

FLOAT32 match_priority
  _ARGS((CHUNKS_RECORD *chunks_record,
  STATE *state,
  STATE *old_state,
  int num_joints));

FLOAT32 frequency_priority
  _ARGS((STATE *state,
  STATE *old_state,
  int num_joints));

STATE *pick_good_segmentation
  _ARGS((CHUNKS_RECORD *chunks_record));

void print_widths
  _ARGS((FILE *file,
  char *string,
  WIDTH_RECORD *width_array));

FLOAT32 prioritize_state
  _ARGS((CHUNKS_RECORD *chunks_record,
  SEARCH_RECORD *the_search,
  STATE *old_state));

FLOAT32 rating_priority
  _ARGS((CHUNKS_RECORD *chunks_record,
  STATE *state,
  STATE *old_state,
  int num_joints));

WIDTH_RECORD *state_char_widths
  _ARGS((WIDTH_RECORD *chunk_widths,
  STATE *state,
  int num_joints,
  SEARCH_STATE *search_state));

FLOAT32 width_priority
  _ARGS((CHUNKS_RECORD *chunks_record,
  STATE *state,
  int num_joints));

#undef _ARGS
*/
#endif
