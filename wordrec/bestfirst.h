/* -*-C-*-
 ********************************************************************************
 *
 * File:        bestfirst.h  (Formerly bestfirst.h)
 * Description:  Best first search functions
 * Author:       Mark Seaman, OCR Technology
 * Created:      Mon May 14 11:23:29 1990
 * Modified:     Mon Apr 29 14:21:57 1991 (Mark Seaman) marks@hpgrlt
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
 *******************************************************************************/

#ifndef BESTFIRST_H
#define BESTFIRST_H

/*----------------------------------------------------------------------
              I n c l u d e s
----------------------------------------------------------------------*/
#include "oldheap.h"
#include "closed.h"
#include "choicearr.h"
#include "associate.h"
#include "choices.h"
#include "states.h"
#include "stopper.h"
#include "blobs.h"
#include "tessclas.h"
#include "seam.h"

/*----------------------------------------------------------------------
              T y p e s
----------------------------------------------------------------------*/
typedef struct
{
  HEAP *open_states;
  HASH_TABLE closed_states;
  STATE *this_state;
  STATE *first_state;
  STATE *best_state;
  int num_joints;
  long num_states;
  long before_best;
  A_CHOICE *best_choice;
  A_CHOICE *raw_choice;
} SEARCH_RECORD;

/*----------------------------------------------------------------------
              V a r i a b l e s
---------------------------------------------------------------------*/
extern int num_seg_states;
extern int num_popped;

/*----------------------------------------------------------------------
              M a c r o s
---------------------------------------------------------------------*/
/**********************************************************************
 * chunks_gap
 *
 * Return the width of several of the chunks (if they were joined to-
 * gether.
 **********************************************************************/
#define chunks_gap(chunk_widths,last_chunk)                    \
((last_chunk < (chunk_widths)->num_chars - 1) ?  \
	((chunk_widths)->widths[last_chunk * 2 + 1]) :  \
	(0))

/*----------------------------------------------------------------------
              F u n c t i o n s
----------------------------------------------------------------------*/
void init_bestfirst_vars();

void best_first_search(CHUNKS_RECORD *chunks_record,
                       A_CHOICE *best_choice,
                       A_CHOICE *raw_choice,
                       STATE *state,
                       DANGERR *fixpt,
                       STATE *best_state,
                       inT32 pass);

int chunks_width(WIDTH_RECORD *width_record, int start_chunk, int last_chunk);

void delete_search(SEARCH_RECORD *the_search);

CHOICES_LIST evaluate_chunks(CHUNKS_RECORD *chunks_record,
                             SEARCH_STATE search_state,
                             STATE *this_state,
                             STATE *best_state,
                             inT32 pass);

inT16 evaluate_state(CHUNKS_RECORD *chunks_record,
                     SEARCH_RECORD *the_search,
                     DANGERR *fixpt,
                     STATE *best_state,
                     inT32 pass);

CHOICES_LIST rebuild_current_state(TBLOB *blobs,
                                   SEAMS seam_list,
                                   STATE *state,
                                   CHOICES_LIST old_choices,
                                   int fx);

void expand_node(FLOAT32 worst_priority,
                 CHUNKS_RECORD *chunks_record,
                 SEARCH_RECORD *the_search);

SEARCH_RECORD *new_search(CHUNKS_RECORD *chunks_record,
                          int num_joints,
                          A_CHOICE *best_choice,
                          A_CHOICE *raw_choice,
                          STATE *state);

STATE *pop_queue(HEAP *queue);

void push_queue(HEAP *queue, STATE *state,
                FLOAT32 worst_priority, FLOAT32 priority);

void replace_char_widths(CHUNKS_RECORD *chunks_record, SEARCH_STATE state);

/*
#if defined(__STDC__) || defined(__cplusplus)
# define	_ARGS(s) s
#else
# define	_ARGS(s) ()
#endif*/

/* bestfirst.c
void init_bestfirst_vars
  _ARGS((void));

void best_first_search
  _ARGS((CHUNKS_RECORD *chunks_record,
  A_CHOICE *best_choice,
  A_CHOICE *raw_choice,
  STATE *state,
  STATE*					best_state,
  inT32					pass));

CHOICES_LIST rebuild_current_state();

void write_segmentation
  _ARGS((char *correct,
    CHUNKS_RECORD *chunks_record,
  SEARCH_RECORD  *the_search));

int chunks_width
  _ARGS((WIDTH_RECORD *width_record,
  int start_chunk,
  int last_chunk));

void delete_search
  _ARGS((SEARCH_RECORD *the_search));

CHOICES_LIST evaluate_chunks
  _ARGS((CHUNKS_RECORD *chunks_record,
  SEARCH_STATE search_state,
  STATE*					this_state,
  STATE*					best_state,
  inT32					pass));

inT16 evaluate_state
  _ARGS((CHUNKS_RECORD *chunks_record,
  SEARCH_RECORD *the_search,
  STATE*					best_state,
  inT32					pass));

void expand_node
  _ARGS((CHUNKS_RECORD *chunks_record,
  SEARCH_RECORD *the_search));

SEARCH_RECORD *new_search
  _ARGS((CHUNKS_RECORD *chunks_record,
  int num_joints,
  A_CHOICE *best_choice,
  A_CHOICE *raw_choice,
  STATE *state));

STATE *pop_queue
  _ARGS((HEAP *queue));

void push_queue
  _ARGS((HEAP *queue,
  STATE *state,
  FLOAT32 priority));

void replace_char_widths
  _ARGS((CHUNKS_RECORD *chunks_record,
  SEARCH_STATE state));
#undef _ARGS
*/
#endif
