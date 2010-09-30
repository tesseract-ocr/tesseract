/* -*-C-*-
 ********************************************************************************
 *
 * File:        metrics.c  (Formerly metrics.c)
 * Description:
 * Author:       Mark Seaman, OCR Technology
 * Created:      Fri Oct 16 14:37:00 1987
 * Modified:     Tue Jul 30 17:02:07 1991 (Mark Seaman) marks@hpgrlt
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
#include "metrics.h"
#include "bestfirst.h"
#include "associate.h"
#include "tally.h"
#include "plotseg.h"
#include "globals.h"
#include "wordclass.h"
#include "intmatcher.h"
#include "freelist.h"
#include "callcpp.h"
#include "ndminx.h"
#include "wordrec.h"

// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif

/*----------------------------------------------------------------------
              V a r i a b l e s
----------------------------------------------------------------------*/
static int states_timed_out1;    /* Counters */
static int states_timed_out2;
static int words_segmented1;
static int words_segmented2;
static int segmentation_states1;
static int segmentation_states2;
static int save_priorities;

int words_chopped1;
int words_chopped2;
int chops_attempted1;
int chops_performed1;
int chops_attempted2;
int chops_performed2;

int character_count;
int word_count;
int chars_classified;

MEASUREMENT num_pieces;
MEASUREMENT width_measure;

MEASUREMENT width_priority_range;/* Help to normalize */
MEASUREMENT match_priority_range;

TALLY states_before_best;
TALLY best_certainties[2];
TALLY character_widths;          /* Width histogram */

FILE *priority_file_1;           /* Output to cluster */
FILE *priority_file_2;
FILE *priority_file_3;

STATE *known_best_state = NULL;  /* The right answer */

/*----------------------------------------------------------------------
              M a c r o s
----------------------------------------------------------------------*/
#define   CERTAINTY_BUCKET_SIZE -0.5
#define   CERTAINTY_BUCKETS     40

/*----------------------------------------------------------------------
              F u n c t i o n s
----------------------------------------------------------------------*/
/**********************************************************************
 * init_metrics
 *
 * Set up the appropriate variables to record information about the
 * OCR process. Later calls will log the data and save a summary.
 **********************************************************************/
void init_metrics() {
  words_chopped1 = 0;
  words_chopped2 = 0;
  chops_performed1 = 0;
  chops_performed2 = 0;
  chops_attempted1 = 0;
  chops_attempted2 = 0;

  words_segmented1 = 0;
  words_segmented2 = 0;
  states_timed_out1 = 0;
  states_timed_out2 = 0;
  segmentation_states1 = 0;
  segmentation_states2 = 0;

  save_priorities = 0;

  character_count = 0;
  word_count = 0;
  chars_classified = 0;
  permutation_count = 0;

  end_metrics();

  states_before_best = new_tally (MIN (100, wordrec_num_seg_states));

  best_certainties[0] = new_tally (CERTAINTY_BUCKETS);
  best_certainties[1] = new_tally (CERTAINTY_BUCKETS);
  reset_width_tally();
}

void end_metrics() {
  if (states_before_best != NULL) {
    memfree(states_before_best);
    memfree(best_certainties[0]);
    memfree(best_certainties[1]);
    memfree(character_widths);
    states_before_best = NULL;
    best_certainties[0] = NULL;
    best_certainties[1] = NULL;
    character_widths = NULL;
  }
}


/**********************************************************************
 * record_certainty
 *
 * Maintain a record of the best certainty values achieved on each
 * word recognition.
 **********************************************************************/
void record_certainty(float certainty, int pass) {
  int bucket;

  if (certainty / CERTAINTY_BUCKET_SIZE < MAX_INT32)
    bucket = (int) (certainty / CERTAINTY_BUCKET_SIZE);
  else
    bucket = MAX_INT32;

  inc_tally_bucket (best_certainties[pass - 1], bucket);
}


/**********************************************************************
 * record_search_status
 *
 * Record information about each iteration of the search.  This  data
 * is kept in global memory and accumulated over multiple segmenter
 * searches.
 **********************************************************************/
void record_search_status(int num_states, int before_best, float closeness) {
  inc_tally_bucket(states_before_best, before_best);

  if (first_pass) {
    if (num_states == wordrec_num_seg_states + 1)
      states_timed_out1++;
    segmentation_states1 += num_states;
    words_segmented1++;
  }
  else {
    if (num_states == wordrec_num_seg_states + 1)
      states_timed_out2++;
    segmentation_states2 += num_states;
    words_segmented2++;
  }
}


/**********************************************************************
 * save_summary
 *
 * Save the summary information into the file "file.sta".
 **********************************************************************/
namespace tesseract {
void Wordrec::save_summary(inT32 elapsed_time) {
  #ifndef SECURE_NAMES
  STRING outfilename;
  FILE *f;
  int x;
  int total;

  outfilename = imagefile + ".sta";
  f = open_file (outfilename.string(), "w");

  fprintf (f, INT32FORMAT " seconds elapsed\n", elapsed_time);
  fprintf (f, "\n");

  fprintf (f, "%d characters\n", character_count);
  fprintf (f, "%d words\n", word_count);
  fprintf (f, "\n");

  fprintf (f, "%d permutations performed\n", permutation_count);
  fprintf (f, "%d characters classified\n", chars_classified);
  fprintf (f, "%4.0f%% classification overhead\n",
    (float) chars_classified / character_count * 100.0 - 100.0);
  fprintf (f, "\n");

  fprintf (f, "%d words chopped (pass 1) ", words_chopped1);
  fprintf (f, " (%0.0f%%)\n", (float) words_chopped1 / word_count * 100);
  fprintf (f, "%d chops performed\n", chops_performed1);
  fprintf (f, "%d chops attempted\n", chops_attempted1);
  fprintf (f, "\n");

  fprintf (f, "%d words joined (pass 1)", words_segmented1);
  fprintf (f, " (%0.0f%%)\n", (float) words_segmented1 / word_count * 100);
  fprintf (f, "%d segmentation states\n", segmentation_states1);
  fprintf (f, "%d segmentations timed out\n", states_timed_out1);
  fprintf (f, "\n");

  fprintf (f, "%d words chopped (pass 2) ", words_chopped2);
  fprintf (f, " (%0.0f%%)\n", (float) words_chopped2 / word_count * 100);
  fprintf (f, "%d chops performed\n", chops_performed2);
  fprintf (f, "%d chops attempted\n", chops_attempted2);
  fprintf (f, "\n");

  fprintf (f, "%d words joined (pass 2)", words_segmented2);
  fprintf (f, " (%0.0f%%)\n", (float) words_segmented2 / word_count * 100);
  fprintf (f, "%d segmentation states\n", segmentation_states2);
  fprintf (f, "%d segmentations timed out\n", states_timed_out2);
  fprintf (f, "\n");

  total = 0;
  iterate_tally (states_before_best, x)
    total += (tally_entry (states_before_best, x) * x);
  fprintf (f, "segmentations (before best) = %d\n", total);
  if (total != 0.0)
    fprintf (f, "%4.0f%% segmentation overhead\n",
      (float) (segmentation_states1 + segmentation_states2) /
      total * 100.0 - 100.0);
  fprintf (f, "\n");

  print_tally (f, "segmentations (before best)", states_before_best);

  iterate_tally (best_certainties[0], x)
    cprintf ("best certainty of %8.4f = %4d %4d\n",
    x * CERTAINTY_BUCKET_SIZE,
    tally_entry (best_certainties[0], x),
    tally_entry (best_certainties[1], x));

  PrintIntMatcherStats(f);
  dj_statistics(f);
  fclose(f);
  #endif
}
}  // namespace tesseract


/**********************************************************************
 * record_priorities
 *
 * If the record mode is set then record the priorities returned by
 * each of the priority voters.  Save them in a file that is set up for
 * doing clustering.
 **********************************************************************/
void record_priorities(SEARCH_RECORD *the_search,
                       FLOAT32 priority_1,
                       FLOAT32 priority_2) {
  record_samples(priority_1, priority_2);
}


/**********************************************************************
 * record_samples
 *
 * Remember the priority samples to summarize them later.
 **********************************************************************/
void record_samples(FLOAT32 match_pri, FLOAT32 width_pri) {
  ADD_SAMPLE(match_priority_range, match_pri);
  ADD_SAMPLE(width_priority_range, width_pri);
}


/**********************************************************************
 * reset_width_tally
 *
 * Create a tally record and initialize it.
 **********************************************************************/
void reset_width_tally() {
  character_widths = new_tally (20);
  new_measurement(width_measure);
  width_measure.num_samples = 158;
  width_measure.sum_of_samples = 125.0;
  width_measure.sum_of_squares = 118.0;
}


#ifndef GRAPHICS_DISABLED
/**********************************************************************
 * save_best_state
 *
 * Save this state away to be compared later.
 **********************************************************************/
void save_best_state(CHUNKS_RECORD *chunks_record) {
  STATE state;
  SEARCH_STATE chunk_groups;
  int num_joints;

  if (save_priorities) {
    num_joints = chunks_record->ratings->dimension() - 1;

    state.part1 = 0xffffffff;
    state.part2 = 0xffffffff;

    chunk_groups = bin_to_chunks (&state, num_joints);
    display_segmentation (chunks_record->chunks, chunk_groups);
    memfree(chunk_groups);

    cprintf ("Enter the correct segmentation > ");
    fflush(stdout);
    state.part1 = 0;
    scanf ("%x", &state.part2);

    chunk_groups = bin_to_chunks (&state, num_joints);
    display_segmentation (chunks_record->chunks, chunk_groups);
    memfree(chunk_groups);
    window_wait(segm_window);  /* == 'n') */

    if (known_best_state)
      free_state(known_best_state);
    known_best_state = new_state (&state);
  }
}
#endif


/**********************************************************************
 * start_record
 *
 * Set up everything needed to record the priority voters.
 **********************************************************************/
void start_recording() {
  if (save_priorities) {
    priority_file_1 = open_file ("Priorities1", "w");
    priority_file_2 = open_file ("Priorities2", "w");
    priority_file_3 = open_file ("Priorities3", "w");
  }
}


/**********************************************************************
 * stop_recording
 *
 * Put an end to the priority recording mechanism.
 **********************************************************************/
void stop_recording() {
  if (save_priorities) {
    fclose(priority_file_1);
    fclose(priority_file_2);
    fclose(priority_file_3);
  }
}
