/* -*-C-*-
 ********************************************************************************
 *
 * File:        metrics.h  (Formerly metrics.h)
 * Description:
 * Author:       Mark Seaman, SW Productivity
 * Created:      Fri Oct 16 14:37:00 1987
 * Modified:     Tue Jul 30 17:02:48 1991 (Mark Seaman) marks@hpgrlt
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
#ifndef METRICS_H
#define METRICS_H

/*----------------------------------------------------------------------
              I n c l u d e s
----------------------------------------------------------------------*/
#include "measure.h"
#include "bestfirst.h"
#include "states.h"

/*----------------------------------------------------------------------
              V a r i a b l e s
----------------------------------------------------------------------*/
extern int words_chopped1;
extern int words_chopped2;
extern int chops_attempted1;
extern int chops_performed1;
extern int chops_attempted2;
extern int chops_performed2;
extern int permutation_count;

extern int character_count;
extern int word_count;
extern int chars_classified;

extern MEASUREMENT width_measure;
extern MEASUREMENT width_priority_range;
extern MEASUREMENT match_priority_range;

/*----------------------------------------------------------------------
              F u n c t i o n s
----------------------------------------------------------------------*/
void init_metrics();
void end_metrics();

void record_certainty(float certainty, int pass);

void record_search_status(int num_states, int before_best, float closeness);

void record_priorities(SEARCH_RECORD *the_search,
                       FLOAT32 priority_1,
                       FLOAT32 priority_2);

void record_samples(FLOAT32 match_pri, FLOAT32 width_pri);

void reset_width_tally();

void save_best_state(CHUNKS_RECORD *chunks_record);

void start_recording();

void stop_recording();

/*
#if defined(__STDC__) || defined(__cplusplus) || MAC_OR_DOS
# define	_ARGS(s) s
#else
# define	_ARGS(s) ()
#endif*/

/* metrics.c
void init_metrics
  _ARGS((void));

void record_certainty
  _ARGS((float certainty,
  int pass));

void record_search_status
  _ARGS((int num_states,
  int before_best,
  float closeness));

void save_summary
  _ARGS((inT32 elapsed_time));

void record_priorities
  _ARGS((SEARCH_RECORD *the_search,
  STATE *old_state,
  FLOAT32 priority_1,
  FLOAT32 priority_2));

void record_samples
  _ARGS((FLOAT32 match_pri,
  FLOAT32 width_pri));

void reset_width_tally
  _ARGS((void));

void save_best_state
  _ARGS((CHUNKS_RECORD *chunks_record));

void start_recording
  _ARGS((void));

void stop_recording
  _ARGS((void));

#undef _ARGS
*/
#endif
