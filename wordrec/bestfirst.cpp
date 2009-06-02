/* -*-C-*-
 ********************************************************************************
 *
 * File:        bestfirst.c  (Formerly bestfirst.c)
 * Description:  Best first search functions
 * Author:       Mark Seaman, OCR Technology
 * Created:      Mon May 14 11:23:29 1990
 * Modified:     Tue Jul 30 16:08:47 1991 (Mark Seaman) marks@hpgrlt
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
 ***************************************************************************/

/*----------------------------------------------------------------------
          I n c l u d e s
---------------------------------------------------------------------*/
#include <assert.h>

#include "bestfirst.h"
#include "heuristic.h"
#include "plotseg.h"
#include "tordvars.h"
#include "debug.h"
#include "pieces.h"
#include "stopper.h"
#include "metrics.h"
#include "states.h"
#include "bitvec.h"
#include "freelist.h"
#include "permute.h"
#include "structures.h"
#include "wordclass.h"

void call_caller();

/*----------------------------------------------------------------------
          V a r i a b l e s
---------------------------------------------------------------------*/
int num_joints;                  /* Number of chunks - 1 */
int num_pushed = 0;
int num_popped = 0;

make_int_var (num_seg_states, 30, make_seg_states,
9, 1, set_seg_states, "Segmentation states");

make_float_var (worst_state, 1, make_worst_state,
9, 9, set_worst_state, "Worst segmentation state");
/**/
/*----------------------------------------------------------------------
          F u n c t i o n s
----------------------------------------------------------------------*/
/**********************************************************************
 * init_bestfirst_vars
 *
 * Create and initialize references to debug variables that control
 * operations in this file.
 **********************************************************************/
void init_bestfirst_vars() {
  make_seg_states();
  make_worst_state();
}


/**********************************************************************
 * best_first_search
 *
 * Find the best segmentation by doing a best first search of the
 * solution space.
 **********************************************************************/
void best_first_search(CHUNKS_RECORD *chunks_record,
                       A_CHOICE *best_choice,
                       A_CHOICE *raw_choice,
                       STATE *state,
                       DANGERR *fixpt,
                       STATE *best_state,
                       inT32 pass) {
  SEARCH_RECORD *the_search;
  inT16 keep_going;
  STATE guided_state;

  num_joints = matrix_dimension (chunks_record->ratings) - 1;
  the_search = new_search (chunks_record, num_joints,
    best_choice, raw_choice, state);

#ifndef GRAPHICS_DISABLED
  save_best_state(chunks_record);
#endif
  start_recording();
  FLOAT32 worst_priority = 2.0f * prioritize_state(chunks_record,
                                                   the_search,
                                                   best_state);
  if (worst_priority < worst_state)
    worst_priority = worst_state;

  guided_state = *state;
  do {
                                 /* Look for answer */
    if (!hash_lookup (the_search->closed_states, the_search->this_state)) {

      if (blob_skip) {
        free_state (the_search->this_state);
        break;
      }

      guided_state = *(the_search->this_state);
      keep_going =
        evaluate_state(chunks_record, the_search, fixpt, best_state, pass);

      hash_add (the_search->closed_states, the_search->this_state);

      if (!keep_going ||
      (the_search->num_states > num_seg_states) || (blob_skip)) {
        free_state (the_search->this_state);
        break;
      }

      expand_node(worst_priority, chunks_record, the_search);
    }

    free_state (the_search->this_state);
    num_popped++;
    the_search->this_state = pop_queue (the_search->open_states);
  }
  while (the_search->this_state);

  state->part1 = the_search->best_state->part1;
  state->part2 = the_search->best_state->part2;
  stop_recording();
  delete_search(the_search);
}


/**********************************************************************
 * chunks_width
 *
 * Return the width of several of the chunks (if they were joined to-
 * gether.
 **********************************************************************/
int chunks_width(WIDTH_RECORD *width_record, int start_chunk, int last_chunk) {
  int result = 0;
  int x;

  for (x = start_chunk * 2; x <= last_chunk * 2; x++)
    result += width_record->widths[x];

  return (result);
}


/**********************************************************************
 * delete_search
 *
 * Terminate the current search and free all the memory involved.
 **********************************************************************/
void delete_search(SEARCH_RECORD *the_search) {
  float closeness;

  closeness = (the_search->num_joints ?
    (hamming_distance(reinterpret_cast<uinT32*>(the_search->first_state),
                      reinterpret_cast<uinT32*>(the_search->best_state), 2) /
      (float) the_search->num_joints) : 0.0f);

  record_search_status (the_search->num_states,
    the_search->before_best, closeness);

  free_state (the_search->first_state);
  free_state (the_search->best_state);

  free_hash_table (the_search->closed_states);
  FreeHeapData (the_search->open_states, (void_dest) free_state);

  memfree(the_search);
}


/**********************************************************************
 * evaluate_chunks
 *
 * A particular word level segmentation has been chosen.  Evaluation
 * this to find the word list that corresponds to it.
 **********************************************************************/
CHOICES_LIST evaluate_chunks(CHUNKS_RECORD *chunks_record,
                             SEARCH_STATE search_state,
                             STATE *this_state,
                             STATE *best_state,
                             inT32 pass) {
  CHOICES_LIST char_choices;
  CHOICES this_choice;
  int i;
  int x = 0;
  int y;

  char_choices = new_choice_list ();
  /* Iterate sub-paths */
  for (i = 1; i <= search_state[0] + 1; i++) {
    if (i > search_state[0])
      y = count_blobs (chunks_record->chunks) - 1;
    else
      y = x + search_state[i];

    if (blob_skip) {
      array_free(char_choices);
      return (NULL);
    }                            /* Process one square */
    /* Classify if needed */
    this_choice = get_piece_rating (chunks_record->ratings,
      chunks_record->chunks,
      chunks_record->splits,
      x, y,
      chunks_record->fx,
      this_state, best_state, pass, i - 1);

    if (this_choice == NIL) {
      array_free(char_choices);
      return (NULL);
    }
    /* Add permuted ratings */
    last_segmentation[i - 1].certainty = best_certainty (this_choice);
    last_segmentation[i - 1].match = best_probability (this_choice);

    last_segmentation[i - 1].width =
      chunks_width (chunks_record->chunk_widths, x, y);
    last_segmentation[i - 1].gap =
      chunks_gap (chunks_record->chunk_widths, y);

    char_choices = array_push (char_choices, this_choice);
    x = y + 1;
  }
  return (char_choices);
}


/**********************************************************************
 * evaluate_state
 *
 * Evaluate the segmentation that is represented by this state in the
 * best first search.  Add this state to the "states_seen" list.
 **********************************************************************/
inT16 evaluate_state(CHUNKS_RECORD *chunks_record,
                     SEARCH_RECORD *the_search,
                     DANGERR *fixpt,
                     STATE *best_state,
                     inT32 pass) {
  CHOICES_LIST char_choices;
  SEARCH_STATE chunk_groups;
  float rating_limit = class_probability (the_search->best_choice);
  inT16 keep_going = TRUE;
  PIECES_STATE widths;

  the_search->num_states++;
  chunk_groups = bin_to_chunks (the_search->this_state,
    the_search->num_joints);
  bin_to_pieces (the_search->this_state, the_search->num_joints, widths);
  LogNewSegmentation(widths);

  rating_limit = class_probability (the_search->best_choice);

  char_choices =
    evaluate_chunks (chunks_record, chunk_groups, the_search->this_state,
    best_state, pass);
  if (char_choices != NULL) {
    permute_characters (char_choices,
      rating_limit,
      the_search->best_choice, the_search->raw_choice);
    if (AcceptableChoice (char_choices, the_search->best_choice,
      the_search->raw_choice, fixpt))
      keep_going = FALSE;
    array_free(char_choices);
  }

#ifndef GRAPHICS_DISABLED
  if (display_segmentations) {
    display_segmentation (chunks_record->chunks, chunk_groups);
    if (display_segmentations > 1)
      window_wait(segm_window);
  }
#endif

  if (rating_limit != class_probability (the_search->best_choice)) {
    the_search->before_best = the_search->num_states;
    the_search->best_state->part1 = the_search->this_state->part1;
    the_search->best_state->part2 = the_search->this_state->part2;
    replace_char_widths(chunks_record, chunk_groups);
  }
  else if (char_choices != NULL)
    fixpt->index = -1;

  memfree(chunk_groups);

  return (keep_going);
}


/**********************************************************************
 * rebuild_current_state
 *
 * Evaluate the segmentation that is represented by this state in the
 * best first search.  Add this state to the "states_seen" list.
 **********************************************************************/
CHOICES_LIST rebuild_current_state(TBLOB *blobs,
                                   SEAMS seam_list,
                                   STATE *state,
                                   CHOICES_LIST old_choices,
                                   int fx) {
  CHOICES_LIST char_choices;
  SEARCH_STATE search_state;
  int i;
  int num_joints = array_count (seam_list);
  int x = 0;
  int blobindex;                 /*current blob */
  TBLOB *p_blob;
  TBLOB *blob;
  TBLOB *next_blob;
  int y;

#ifndef GRAPHICS_DISABLED
    if (display_segmentations) {
      print_state("Rebuiling state", state, num_joints);
    }
#endif
  search_state = bin_to_chunks (state, num_joints);

  char_choices = new_choice_list ();
  /* Iterate sub-paths */
  for (i = 1; i <= search_state[0]; i++) {
    y = x + search_state[i];
    x = y + 1;
    char_choices = array_push (char_choices, NULL);
  }
  char_choices = array_push (char_choices, NULL);

  y = count_blobs (blobs) - 1;
  for (i = search_state[0]; i >= 0; i--) {
    if (x == y) {                /*single fragment */
      array_value (char_choices, i) = array_value (old_choices, x);
                                 /*grab the list */
      array_value (old_choices, x) = NULL;
    }
    else {
      join_pieces(blobs, seam_list, x, y);
      for (blob = blobs, blobindex = 0, p_blob = NULL; blobindex < x;
      blobindex++) {
        p_blob = blob;
        blob = blob->next;
      }
      while (blobindex < y) {
        next_blob = blob->next;
        blob->next = next_blob->next;
        oldblob(next_blob);  /*junk dead blobs */
        blobindex++;
      }
      array_value (char_choices, i) =
        (char *) classify_blob (p_blob, blob, blob->next, NULL, fx,
        "rebuild", Orange, NULL, NULL, 0, 0);
    }

    y = x - 1;
    x = y - search_state[i];
  }

  memfree(search_state);
  free_all_choices(old_choices, x);
  return (char_choices);

}


/**********************************************************************
 * expand_node
 *
 * Create the states that are attached to this one.  Check to see that
 * each one has not already been visited.  If not add it to the priority
 * queue.
 **********************************************************************/
void expand_node(FLOAT32 worst_priority,
                 CHUNKS_RECORD *chunks_record, SEARCH_RECORD *the_search) {
  STATE old_state;
  int x;
  int mask = 1 << (the_search->num_joints - 1 - 32);

  old_state.part1 = the_search->this_state->part1;
  old_state.part2 = the_search->this_state->part2;

  for (x = the_search->num_joints; x > 32; x--) {
    the_search->this_state->part1 = mask ^ old_state.part1;
    if (!hash_lookup (the_search->closed_states, the_search->this_state))
      push_queue (the_search->open_states, the_search->this_state,
                  worst_priority,
                  prioritize_state (chunks_record, the_search, &old_state));
    mask >>= 1;
  }

  if (the_search->num_joints > 32) {
    mask = 1 << 31;
  }
  else {
    mask = 1 << (the_search->num_joints - 1);
  }

  while (x--) {
    the_search->this_state->part2 = mask ^ old_state.part2;
    if (!hash_lookup (the_search->closed_states, the_search->this_state))
      push_queue (the_search->open_states, the_search->this_state,
                  worst_priority,
                  prioritize_state (chunks_record, the_search, &old_state));
    mask >>= 1;
  }
}


/**********************************************************************
 * new_search
 *
 * Create and initialize a new search record.
 **********************************************************************/
SEARCH_RECORD *new_search(CHUNKS_RECORD *chunks_record,
                          int num_joints,
                          A_CHOICE *best_choice,
                          A_CHOICE *raw_choice,
                          STATE *state) {
  SEARCH_RECORD *this_search;

  this_search = (SEARCH_RECORD *) memalloc (sizeof (SEARCH_RECORD));

  this_search->open_states = MakeHeap (num_seg_states * 20);
  this_search->closed_states = new_hash_table ();

  if (state)
    this_search->this_state = new_state (state);
  else
    cprintf ("error: bad initial state in new_search\n");

  this_search->first_state = new_state (this_search->this_state);
  this_search->best_state = new_state (this_search->this_state);

  this_search->best_choice = best_choice;
  this_search->raw_choice = raw_choice;

  this_search->num_joints = num_joints;
  this_search->num_states = 0;
  this_search->before_best = 0;

  return (this_search);
}


/**********************************************************************
 * pop_queue
 *
 * Get this state from the priority queue.  It should be the state that
 * has the greatest urgency to be evaluated.
 **********************************************************************/
STATE *pop_queue(HEAP *queue) {
  HEAPENTRY entry;

  if (GetTopOfHeap (queue, &entry) == OK) {
#ifndef GRAPHICS_DISABLED
    if (display_segmentations) {
      cprintf ("eval state: %8.3f ", entry.Key);
      print_state ("", (STATE *) entry.Data, num_joints);
    }
#endif
    return ((STATE *) entry.Data);
  }
  else {
    return (NULL);
  }
}


/**********************************************************************
 * push_queue
 *
 * Add this state into the priority queue.
 **********************************************************************/
void push_queue(HEAP *queue, STATE *state, FLOAT32 worst_priority,
                FLOAT32 priority) {
  HEAPENTRY entry;

  if (SizeOfHeap (queue) < MaxSizeOfHeap (queue) && priority < worst_priority) {
    entry.Data = (char *) new_state (state);
    num_pushed++;
    entry.Key = priority;
    HeapStore(queue, &entry);
  }
}


/**********************************************************************
 * replace_char_widths
 *
 * Replace the value of the char_width field in the chunks_record with
 * the updated width measurements from the last_segmentation.
 **********************************************************************/
void replace_char_widths(CHUNKS_RECORD *chunks_record, SEARCH_STATE state) {
  WIDTH_RECORD *width_record;
  int num_blobs;
  int i;

  free_widths (chunks_record->char_widths);

  num_blobs = state[0] + 1;
  width_record = (WIDTH_RECORD *) memalloc (sizeof (int) * num_blobs * 2);
  width_record->num_chars = num_blobs;

  for (i = 0; i < num_blobs; i++) {

    width_record->widths[2 * i] = last_segmentation[i].width;

    if (i + 1 < num_blobs)
      width_record->widths[2 * i + 1] = last_segmentation[i].gap;
  }
  chunks_record->char_widths = width_record;
}
