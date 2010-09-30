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
#include "baseline.h"
#include "bitvec.h"
#include "callback.h"
#include "dict.h"
#include "freelist.h"
#include "globals.h"
#include "heuristic.h"
#include "metrics.h"
#include "permute.h"
#include "pieces.h"
#include "plotseg.h"
#include "ratngs.h"
#include "states.h"
#include "stopper.h"
#include "structures.h"
#include "tordvars.h"
#include "unicharset.h"
#include "wordclass.h"
#include "wordrec.h"

// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif

void call_caller();

/*----------------------------------------------------------------------
          V a r i a b l e s
---------------------------------------------------------------------*/
int num_joints;                  /* Number of chunks - 1 */
int num_pushed = 0;
int num_popped = 0;

INT_VAR(wordrec_num_seg_states, 30, "Segmentation states");

double_VAR(wordrec_worst_state, 1, "Worst segmentation state");

/**/
/*----------------------------------------------------------------------
          F u n c t i o n s
----------------------------------------------------------------------*/

namespace tesseract {
/**
 * @name best_first_search
 *
 * Find the best segmentation by doing a best first search of the
 * solution space.
 */
void Wordrec::best_first_search(CHUNKS_RECORD *chunks_record,
                                WERD_CHOICE *best_choice,
                                WERD_CHOICE *raw_choice,
                                STATE *state,
                                DANGERR *fixpt,
                                STATE *best_state) {
  SEARCH_RECORD *the_search;
  inT16 keep_going;
  STATE guided_state;   // not used

  num_joints = chunks_record->ratings->dimension() - 1;
  the_search = new_search(chunks_record, num_joints,
                          best_choice, raw_choice, state);

  // The default state is initialized as the best choice.  In order to apply
  // segmentation adjustment, or any other contextual processing in permute,
  // we give the best choice a poor rating to force the processed raw choice
  // to be promoted to best choice.
  the_search->best_choice->set_rating(100000.0);
  evaluate_state(chunks_record, the_search, fixpt);
  if (permute_debug) {
    tprintf("\n\n\n =========== BestFirstSearch ==============\n");
    best_choice->print("**Initial BestChoice**");
  }

#ifndef GRAPHICS_DISABLED
  save_best_state(chunks_record);
#endif
  start_recording();
  FLOAT32 worst_priority = 2.0f * prioritize_state(chunks_record, the_search);
  if (worst_priority < wordrec_worst_state)
    worst_priority = wordrec_worst_state;
  if (segment_debug) {
    print_state("BestFirstSearch", best_state, num_joints);
  }

  guided_state = *state;
  do {
                                 /* Look for answer */
    if (!hash_lookup (the_search->closed_states, the_search->this_state)) {

      if (tord_blob_skip) {
        free_state (the_search->this_state);
        break;
      }

      guided_state = *(the_search->this_state);
      keep_going = evaluate_state(chunks_record, the_search, fixpt);
      hash_add (the_search->closed_states, the_search->this_state);

      if (!keep_going ||
          (the_search->num_states > wordrec_num_seg_states) ||
          (tord_blob_skip)) {
        if (segment_debug)
          tprintf("Breaking best_first_search on keep_going %s numstates %d\n",
                  ((keep_going) ? "T" :"F"), the_search->num_states);
        free_state (the_search->this_state);
        break;
      }

      FLOAT32 new_worst_priority = 2.0f * prioritize_state(chunks_record,
                                                           the_search);
      if (new_worst_priority < worst_priority) {
        if (segment_debug)
          tprintf("Lowering WorstPriority %f --> %f\n",
                  worst_priority, new_worst_priority);
        // Tighten the threshold for admitting new paths as better search
        // candidates are found.  After lowering this threshold, we can safely
        // popout everything that is worse than this score also.
        worst_priority = new_worst_priority;
      }
      expand_node(worst_priority, chunks_record, the_search);
    }

    free_state (the_search->this_state);
    num_popped++;
    the_search->this_state = pop_queue (the_search->open_states);
    if (segment_debug && !the_search->this_state)
      tprintf("No more states to evalaute after %d evals", num_popped);
  }
  while (the_search->this_state);

  state->part1 = the_search->best_state->part1;
  state->part2 = the_search->best_state->part2;
  stop_recording();
  if (permute_debug) {
    tprintf("\n\n\n =========== BestFirstSearch ==============\n");
            // best_choice->debug_string(getDict().getUnicharset()).string());
    best_choice->print("**Final BestChoice**");
  }
  // save the best_state stats
  delete_search(the_search);
}
}  // namespace tesseract


/**
 * @name chunks_width
 *
 * Return the width of a chunk which is a composed of several blobs
 * blobs[start_blob..last_blob] inclusively,
 * whose individual widths and gaps are record in width_record in the form
 * width_record->num_char = n
 * width_record->widths[2*n-1] = w0,g0,w1,g1..w(n-1),g(n-1)
 */
int chunks_width(WIDTH_RECORD *width_record, int start_blob, int last_blob) {
  int result = 0;
  for (int x = start_blob * 2; x <= last_blob * 2; x++)
    result += width_record->widths[x];
  return (result);
}

/**
 * @name chunks_gap
 *
 * Return the width of between the specified chunk and next.
 */
int chunks_gap(WIDTH_RECORD *width_record, int last_chunk) {
  return (last_chunk < width_record->num_chars - 1) ?
      width_record->widths[last_chunk * 2 + 1] : 0;
}


/**
 * delete_search
 *
 * Terminate the current search and free all the memory involved.
 */
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


namespace tesseract {
/**
 * evaluate_chunks
 *
 * A particular word level segmentation has been chosen.  Evaluation
 * this to find the word list that corresponds to it.
 */
BLOB_CHOICE_LIST_VECTOR *Wordrec::evaluate_chunks(CHUNKS_RECORD *chunks_record,
                                                  SEARCH_STATE search_state) {
  BLOB_CHOICE_LIST_VECTOR *char_choices = new BLOB_CHOICE_LIST_VECTOR();
  BLOB_CHOICE_LIST *blob_choices;
  BLOB_CHOICE_IT blob_choice_it;
  int i;
  int x = 0;
  int y;

  /* Iterate sub-paths */
  for (i = 1; i <= search_state[0] + 1; i++) {
    if (i > search_state[0])
      y = count_blobs (chunks_record->chunks) - 1;
    else
      y = x + search_state[i];

    if (tord_blob_skip) {
      delete char_choices;
      return (NULL);
    }                            /* Process one square */
    /* Classify if needed */
    blob_choices = get_piece_rating(chunks_record->ratings,
                                    chunks_record->chunks,
                                    chunks_record->splits,
                                    x, y);

    if (blob_choices == NULL) {
      delete char_choices;
      return (NULL);
    }

    /* Add permuted ratings */
    blob_choice_it.set_to_list(blob_choices);
    last_segmentation[i - 1].certainty = blob_choice_it.data()->certainty();
    last_segmentation[i - 1].match = blob_choice_it.data()->rating();

    last_segmentation[i - 1].width =
      chunks_width (chunks_record->chunk_widths, x, y);
    last_segmentation[i - 1].gap =
      chunks_gap (chunks_record->chunk_widths, y);

    *char_choices += blob_choices;
    x = y + 1;
  }
  return (char_choices);
}

/**
 * @name evaluate_state
 *
 * Evaluate the segmentation that is represented by this state in the
 * best first search.  Add this state to the "states_seen" list.
 */
inT16 Wordrec::evaluate_state(CHUNKS_RECORD *chunks_record,
                              SEARCH_RECORD *the_search,
                              DANGERR *fixpt) {
  BLOB_CHOICE_LIST_VECTOR *char_choices;
  SEARCH_STATE chunk_groups;
  float rating_limit = the_search->best_choice->rating();
  inT16 keep_going = TRUE;
  PIECES_STATE widths;

  the_search->num_states++;
  chunk_groups = bin_to_chunks(the_search->this_state,
                               the_search->num_joints);
  bin_to_pieces (the_search->this_state, the_search->num_joints, widths);
  getDict().LogNewSegmentation(widths);

  char_choices = evaluate_chunks(chunks_record, chunk_groups);
  wordseg_rating_adjust_factor = -1.0f;
  if (char_choices != NULL && char_choices->length() > 0) {
    // Compute the segmentation cost and include the cost in word rating.
    // TODO(dsl): We should change the SEARCH_RECORD to store this cost
    // from state evaluation and avoid recomputing it here.
    prioritize_state(chunks_record, the_search);
    wordseg_rating_adjust_factor = the_search->segcost_bias;
    getDict().permute_characters(*char_choices, rating_limit,
                                 the_search->best_choice,
                                 the_search->raw_choice);
    bool replaced = false;
    if (getDict().AcceptableChoice(char_choices, the_search->best_choice,
                                   *(the_search->raw_choice), fixpt,
                                   ASSOCIATOR_CALLER, &replaced)) {
      keep_going = FALSE;
    }
  }
  wordseg_rating_adjust_factor = -1.0f;

#ifndef GRAPHICS_DISABLED
  if (wordrec_display_segmentations) {
    display_segmentation (chunks_record->chunks, chunk_groups);
    if (wordrec_display_segmentations > 1)
      window_wait(segm_window);
  }
#endif

  if (rating_limit != the_search->best_choice->rating()) {
    the_search->before_best = the_search->num_states;
    the_search->best_state->part1 = the_search->this_state->part1;
    the_search->best_state->part2 = the_search->this_state->part2;
    replace_char_widths(chunks_record, chunk_groups);
  }
  else if (char_choices != NULL)
    fixpt->index = -1;

  if (char_choices != NULL) delete char_choices;
  memfree(chunk_groups);

  return (keep_going);
}


/**
 * rebuild_current_state
 *
 * Evaluate the segmentation that is represented by this state in the
 * best first search.  Add this state to the "states_seen" list.
 */
BLOB_CHOICE_LIST_VECTOR *Wordrec::rebuild_current_state(
    TBLOB *blobs,
    SEAMS seam_list,
    STATE *state,
    BLOB_CHOICE_LIST_VECTOR *old_choices,
    int fx,
    bool force_rebuild,
    const WERD_CHOICE &best_choice,
    const MATRIX *ratings) {
  // Initialize search_state, num_joints, x, y.
  int num_joints = array_count(seam_list);
#ifndef GRAPHICS_DISABLED
    if (wordrec_display_segmentations) {
      print_state("Rebuiling state", state, num_joints);
    }
#endif
  SEARCH_STATE search_state = bin_to_chunks(state, num_joints);
  int x = 0;
  int y;
  int i;
  for (i = 1; i <= search_state[0]; i++) {
    y = x + search_state[i];
    x = y + 1;
  }
  y = count_blobs (blobs) - 1;

  // Initialize char_choices, expanded_fragment_lengths:
  // e.g. if fragment_lengths = {1 1 2 3 1},
  // expanded_fragment_lengths_str = {1 1 2 2 3 3 3 1}.
  BLOB_CHOICE_LIST_VECTOR *char_choices = new BLOB_CHOICE_LIST_VECTOR();
  STRING expanded_fragment_lengths_str = "";
  bool state_has_fragments = false;
  const char *fragment_lengths = NULL;

  if (best_choice.length() > 0) {
    fragment_lengths = best_choice.fragment_lengths();
  }
  if (fragment_lengths) {
    for (int i = 0; i < best_choice.length(); ++i) {
      *char_choices += NULL;
      if (fragment_lengths[i] > 1) {
        state_has_fragments = true;
      }
      for (int j = 0; j < fragment_lengths[i]; ++j) {
        expanded_fragment_lengths_str += fragment_lengths[i];
      }
    }
  } else {
    for (i = 0; i <= search_state[0]; ++i) {
      expanded_fragment_lengths_str += (char)1;
      *char_choices += NULL;
    }
  }

  // Finish early if force_rebuld is false and there are no fragments to merge.
  if (!force_rebuild && !state_has_fragments) {
    delete char_choices;
    memfree(search_state);
    return old_choices;
  }

  // Set up variables for concatenating fragments.
  const char *word_lengths_ptr = NULL;
  const char *word_ptr = NULL;
  if (state_has_fragments) {
    // Make word_lengths_ptr point to the last element in
    // best_choice->unichar_lengths().
    word_lengths_ptr = best_choice.unichar_lengths().string();
    word_lengths_ptr += (strlen(word_lengths_ptr)-1);
    // Make word_str point to the beginning of the last
    // unichar in best_choice->unichar_string().
    word_ptr = best_choice.unichar_string().string();
    word_ptr += (strlen(word_ptr)-*word_lengths_ptr);
  }
  const char *expanded_fragment_lengths =
    expanded_fragment_lengths_str.string();
  bool merging_fragment = false;
  int true_y = -1;
  char unichar[UNICHAR_LEN + 1];
  int fragment_pieces = -1;
  float rating = 0.0;
  float certainty = -MAX_FLOAT32;

  // Populate char_choices list such that it corresponds to search_state.
  //
  // If we are rebuilding a state that contains character fragments:
  // -- combine blobs that belong to character fragments
  // -- re-classify the blobs to obtain choices list for the merged blob
  // -- ensure that correct classification appears in the new choices list
  //    NOTE: a choice composed form original fragment choices will be always
  //    added to the new choices list for each character composed from
  //    fragments (even if the choice for the corresponding character appears
  //    in the re-classified choices list of for the newly merged blob).
  BLOB_CHOICE_IT temp_it;
  int char_choices_index = char_choices->length() - 1;
  for (i = search_state[0]; i >= 0; i--) {
    BLOB_CHOICE_LIST *current_choices = join_blobs_and_classify(
        blobs, seam_list, x, y, fx, ratings, old_choices);
    // Combine character fragments.
    if (expanded_fragment_lengths[i] > 1) {
      // Start merging character fragments.
      if (!merging_fragment) {
        merging_fragment = true;
        true_y = y;
        fragment_pieces = expanded_fragment_lengths[i];
        rating = 0.0;
        certainty = -MAX_FLOAT32;
        strncpy(unichar, word_ptr, *word_lengths_ptr);
        unichar[*word_lengths_ptr] = '\0';
      }
      // Take into account the fact that we could have joined pieces
      // since we first recorded the ending point of a fragment (true_y).
      true_y -= y - x;
      // Populate fragment with updated values and look for the
      // fragment with the same values in current_choices.
      // Update rating and certainty of the character being composed.
      fragment_pieces--;
      CHAR_FRAGMENT fragment;
      fragment.set_all(unichar, fragment_pieces,
                       expanded_fragment_lengths[i]);
      temp_it.set_to_list(current_choices);
      for (temp_it.mark_cycle_pt(); !temp_it.cycled_list();
           temp_it.forward()) {
        const CHAR_FRAGMENT *current_fragment =
          getDict().getUnicharset().get_fragment(temp_it.data()->unichar_id());
        if (current_fragment && fragment.equals(current_fragment)) {
          rating += temp_it.data()->rating();
          if (temp_it.data()->certainty() > certainty) {
            certainty = temp_it.data()->certainty();
          }
          break;
        }
      }
      assert(!temp_it.cycled_list());  // make sure we found the fragment
      // Free current_choices for the fragmented character.
      delete current_choices;

      // Finish composing character from fragments.
      if (fragment_pieces == 0) {
        // Populate current_choices with the classification of
        // the blob merged from blobs of each character fragment.
        current_choices = join_blobs_and_classify(blobs, seam_list, x,
                                                  true_y, fx, ratings, NULL);
        BLOB_CHOICE *merged_choice =
          new BLOB_CHOICE(getDict().getUnicharset().unichar_to_id(unichar),
                          rating, certainty, 0, NO_PERM);

        // Insert merged_blob into current_choices, such that current_choices
        // are still sorted in non-descending order by rating.
        ASSERT_HOST(!current_choices->empty());
        temp_it.set_to_list(current_choices);
        for (temp_it.mark_cycle_pt();
             !temp_it.cycled_list() &&
             merged_choice->rating() > temp_it.data()->rating();
             temp_it.forward());
        temp_it.add_before_stay_put(merged_choice);

        // Done merging this fragmented character.
        merging_fragment = false;
      }
    }
    if (!merging_fragment) {
      // Get rid of fragments in current_choices.
      temp_it.set_to_list(current_choices);
      for (temp_it.mark_cycle_pt(); !temp_it.cycled_list();
           temp_it.forward()) {
        if (getDict().getUnicharset().get_fragment(
            temp_it.data()->unichar_id())) {
          delete temp_it.extract();
        }
      }
      char_choices->set(current_choices, char_choices_index);
      char_choices_index--;

      // Update word_ptr and word_lengths_ptr.
      if (word_lengths_ptr != NULL && word_ptr != NULL) {
        word_lengths_ptr--;
        word_ptr -= (*word_lengths_ptr);
      }
    }
    y = x - 1;
    x = y - search_state[i];
  }
  old_choices->delete_data_pointers();
  delete old_choices;
  memfree(search_state);

  return (char_choices);
}
}  // namespace tesseract


/**
 * @name expand_node
 *
 * Create the states that are attached to this one.  Check to see that
 * each one has not already been visited.  If not add it to the priority
 * queue.
 */
namespace tesseract {
void Wordrec::expand_node(FLOAT32 worst_priority,
                          CHUNKS_RECORD *chunks_record,
                          SEARCH_RECORD *the_search) {
  STATE old_state;
  int nodes_added = 0;
  int x;
  uinT32 mask = 1 << (the_search->num_joints - 1 - 32);

  old_state.part1 = the_search->this_state->part1;
  old_state.part2 = the_search->this_state->part2;

  // We need to expand the search more intelligently, or we get stuck
  // with a bad starting segmentation in a long word sequence as in CJK.
  // Expand a child node only if it is within the global bound, and no
  // worse than 2x of its parent.
  // TODO(dsl): There is some redudency here in recomputing the priority,
  // and in filtering of old_merit and worst_priority.
  the_search->this_state->part2 = old_state.part2;
  for (x = the_search->num_joints; x > 32; x--) {
    the_search->this_state->part1 = mask ^ old_state.part1;
    if (!hash_lookup (the_search->closed_states, the_search->this_state)) {
      FLOAT32 new_merit = prioritize_state(chunks_record, the_search);
      if (segment_debug && permute_debug) {
        cprintf ("....checking state: %8.3f ", new_merit);
        print_state ("", the_search->this_state, num_joints);
      }
      if (new_merit < worst_priority) {
        push_queue (the_search->open_states, the_search->this_state,
                    worst_priority, new_merit);
        nodes_added++;
      }
    }
    mask >>= 1;
  }

  if (the_search->num_joints > 32) {
    mask = 1 << 31;
  }
  else {
    mask = 1 << (the_search->num_joints - 1);
  }

  the_search->this_state->part1 = old_state.part1;
  while (x--) {
    the_search->this_state->part2 = mask ^ old_state.part2;
    if (!hash_lookup (the_search->closed_states, the_search->this_state)) {
      FLOAT32 new_merit = prioritize_state(chunks_record, the_search);
      if (segment_debug && permute_debug) {
        cprintf ("....checking state: %8.3f ", new_merit);
        print_state ("", the_search->this_state, num_joints);
      }
      if (new_merit < worst_priority) {
        push_queue(the_search->open_states, the_search->this_state,
                   worst_priority, new_merit);
        nodes_added++;
      }
    }
    mask >>= 1;
  }
}
}  // namespace tesseract


/**
 * @name new_search
 *
 * Create and initialize a new search record.
 */
SEARCH_RECORD *new_search(CHUNKS_RECORD *chunks_record,
                          int num_joints,
                          WERD_CHOICE *best_choice,
                          WERD_CHOICE *raw_choice,
                          STATE *state) {
  SEARCH_RECORD *this_search;

  this_search = (SEARCH_RECORD *) memalloc (sizeof (SEARCH_RECORD));

  this_search->open_states = MakeHeap (wordrec_num_seg_states * 20);
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
  this_search->segcost_bias = 0;

  return (this_search);
}


/**
 * @name pop_queue
 *
 * Get this state from the priority queue.  It should be the state that
 * has the greatest urgency to be evaluated.
 */
STATE *pop_queue(HEAP *queue) {
  HEAPENTRY entry;

  if (GetTopOfHeap (queue, &entry) == OK) {
#ifndef GRAPHICS_DISABLED
    if (wordrec_display_segmentations) {
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


/**
 * @name push_queue
 *
 * Add this state into the priority queue.
 */
void push_queue(HEAP *queue, STATE *state, FLOAT32 worst_priority,
                FLOAT32 priority) {
  HEAPENTRY entry;

  if (priority < worst_priority) {
    if (SizeOfHeap (queue) >= MaxSizeOfHeap(queue)) {
      if (segment_debug) tprintf("Heap is Full\n");
      return;
    }
    if (segment_debug)
      tprintf("\tpushing %d node  %f\n", num_pushed, priority);
    entry.Data = (char *) new_state (state);
    num_pushed++;
    entry.Key = priority;
    HeapStore(queue, &entry);
  }
}


/**
 * @name replace_char_widths
 *
 * Replace the value of the char_width field in the chunks_record with
 * the updated width measurements from the last_segmentation.
 */
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

namespace tesseract {
BLOB_CHOICE_LIST *Wordrec::join_blobs_and_classify(
    TBLOB *blobs, SEAMS seam_list,
    int x, int y, int fx, const MATRIX *ratings,
    BLOB_CHOICE_LIST_VECTOR *old_choices) {
  BLOB_CHOICE_LIST *choices = NULL;
  // First check to see if we can look up the classificaiton
  // in old_choices (if there is no need to merge blobs).
  if (x == y && old_choices != NULL && ratings == NULL) {
    choices = old_choices->get(x);
    old_choices->set(NULL, x);
    return choices;
  }
  // The ratings matrix filled in by the associator will contain the most
  // up-to-date classification info. Thus we look up the classification there
  // first, and only call classify_blob() if the classification is not found.
  if (ratings != NULL) {
    BLOB_CHOICE_LIST *choices_ptr = ratings->get(x, y);
    if (choices_ptr != NOT_CLASSIFIED) {
      choices = new BLOB_CHOICE_LIST();
      choices->deep_copy(choices_ptr, &BLOB_CHOICE::deep_copy);
    }
  }
  if (x != y) {
    join_pieces(blobs, seam_list, x, y);

    int blobindex;  // current blob
    TBLOB *p_blob;
    TBLOB *blob;
    TBLOB *next_blob;
    for (blob = blobs, blobindex = 0, p_blob = NULL;
         blobindex < x; blobindex++) {
      p_blob = blob;
      blob = blob->next;
    }
    while (blobindex < y) {
      next_blob = blob->next;
      blob->next = next_blob->next;
      oldblob(next_blob);  // junk dead blobs
      blobindex++;
    }
    if (choices == NULL) {
      choices = classify_blob(p_blob, blob, blob->next,
                              NULL, "rebuild", Orange);
    }
  }
  return choices;
}
}  // namespace tesseract
