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

#include "associate.h"
#include "bestfirst.h"
#include "baseline.h"
#include "bitvec.h"
#include "dict.h"
#include "freelist.h"
#include "globals.h"
#include "pageres.h"
#include "permute.h"
#include "pieces.h"
#include "plotseg.h"
#include "ratngs.h"
#include "states.h"
#include "stopper.h"
#include "structures.h"
#include "unicharset.h"
#include "wordclass.h"
#include "wordrec.h"

// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif

void call_caller();

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
                                BLOB_CHOICE_LIST_VECTOR *best_char_choices,
                                WERD_RES *word,
                                STATE *state,
                                DANGERR *fixpt,
                                STATE *best_state) {
  SEARCH_RECORD *the_search;
  inT16 keep_going;
  STATE guided_state;   // not used

  int num_joints = chunks_record->ratings->dimension() - 1;
  the_search = new_search(chunks_record, num_joints, best_char_choices,
                          word->best_choice, word->raw_choice, state);

  // The default state is initialized as the best choice.  In order to apply
  // segmentation adjustment, or any other contextual processing in permute,
  // we give the best choice a poor rating to force the processed raw choice
  // to be promoted to best choice.
  the_search->best_choice->set_rating(WERD_CHOICE::kBadRating);
  evaluate_state(chunks_record, the_search, fixpt);
  if (wordrec_debug_level) {
    tprintf("\n\n\n =========== BestFirstSearch ==============\n");
    word->best_choice->print("**Initial BestChoice**");
  }

  FLOAT32 worst_priority = 2.0f * prioritize_state(chunks_record, the_search);
  if (worst_priority < wordrec_worst_state)
    worst_priority = wordrec_worst_state;
  if (wordrec_debug_level) {
    print_state("BestFirstSearch", best_state, num_joints);
  }

  guided_state = *state;
  do {
                                 /* Look for answer */
    if (!hash_lookup (the_search->closed_states, the_search->this_state)) {
      guided_state = *(the_search->this_state);
      keep_going = evaluate_state(chunks_record, the_search, fixpt);
      hash_add (the_search->closed_states, the_search->this_state);

      if (!keep_going ||
          (the_search->num_states > wordrec_num_seg_states)) {
        if (wordrec_debug_level)
          tprintf("Breaking best_first_search on keep_going %s numstates %d\n",
                  ((keep_going) ? "T" :"F"), the_search->num_states);
        free_state (the_search->this_state);
        break;
      }

      FLOAT32 new_worst_priority = 2.0f * prioritize_state(chunks_record,
                                                           the_search);
      if (new_worst_priority < worst_priority) {
        if (wordrec_debug_level)
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
    if (wordrec_debug_level && !the_search->this_state)
      tprintf("No more states to evalaute after %d evals", num_popped);
  }
  while (the_search->this_state);

  state->part1 = the_search->best_state->part1;
  state->part2 = the_search->best_state->part2;
  if (wordrec_debug_level) {
    tprintf("\n\n\n =========== BestFirstSearch ==============\n");
            // best_choice->debug_string(getDict().getUnicharset()).string());
    word->best_choice->print("**Final BestChoice**");
  }
  // save the best_state stats
  delete_search(the_search);
}

/**
 * delete_search
 *
 * Terminate the current search and free all the memory involved.
 */
void Wordrec::delete_search(SEARCH_RECORD *the_search) {
  float closeness;

  closeness = (the_search->num_joints ?
    (hamming_distance(reinterpret_cast<uinT32*>(the_search->first_state),
                      reinterpret_cast<uinT32*>(the_search->best_state), 2) /
      (float) the_search->num_joints) : 0.0f);

  free_state (the_search->first_state);
  free_state (the_search->best_state);

  free_hash_table(the_search->closed_states);
  FreeHeapData (the_search->open_states, (void_dest) free_state);

  memfree(the_search);
}


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

  // Iterate sub-paths.
  for (i = 1; i <= search_state[0] + 1; i++) {
    if (i > search_state[0])
      y = count_blobs (chunks_record->chunks) - 1;
    else
      y = x + search_state[i];

    // Process one square.

    // Classify if needed.
    blob_choices = get_piece_rating(chunks_record->ratings,
                                    chunks_record->chunks,
                                    chunks_record->splits,
                                    x, y);

    if (blob_choices == NULL) {
      delete char_choices;
      return (NULL);
    }

    // Add permuted ratings.
    blob_choice_it.set_to_list(blob_choices);
    last_segmentation[i - 1].certainty = blob_choice_it.data()->certainty();
    last_segmentation[i - 1].match = blob_choice_it.data()->rating();

    last_segmentation[i - 1].width =
      AssociateUtils::GetChunksWidth(chunks_record->chunk_widths, x, y);
    last_segmentation[i - 1].gap =
      AssociateUtils::GetChunksGap(chunks_record->chunk_widths, y);

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
  bool keep_going = true;
  PIECES_STATE widths;

  the_search->num_states++;
  chunk_groups = bin_to_chunks(the_search->this_state,
                               the_search->num_joints);
  bin_to_pieces (the_search->this_state, the_search->num_joints, widths);
  getDict().LogNewSegmentation(widths);

  char_choices = evaluate_chunks(chunks_record, chunk_groups);
  getDict().SetWordsegRatingAdjustFactor(-1.0f);
  bool updated_best_choice = false;
  if (char_choices != NULL && char_choices->length() > 0) {
    // Compute the segmentation cost and include the cost in word rating.
    // TODO(dsl): We should change the SEARCH_RECORD to store this cost
    // from state evaluation and avoid recomputing it here.
    prioritize_state(chunks_record, the_search);
    getDict().SetWordsegRatingAdjustFactor(the_search->segcost_bias);
    updated_best_choice =
      getDict().permute_characters(*char_choices,
                                   the_search->best_choice,
                                   the_search->raw_choice);
    bool replaced = false;
    if (updated_best_choice) {
      if (getDict().AcceptableChoice(char_choices, the_search->best_choice,
                                     NULL, ASSOCIATOR_CALLER, &replaced)) {
        keep_going = false;
      }
      CopyCharChoices(*char_choices, the_search->best_char_choices);
    }
  }
  getDict().SetWordsegRatingAdjustFactor(-1.0f);

#ifndef GRAPHICS_DISABLED
  if (wordrec_display_segmentations) {
    display_segmentation (chunks_record->chunks, chunk_groups);
    if (wordrec_display_segmentations > 1)
      window_wait(segm_window);
  }
#endif

  if (rating_limit != the_search->best_choice->rating()) {
    ASSERT_HOST(updated_best_choice);
    the_search->before_best = the_search->num_states;
    the_search->best_state->part1 = the_search->this_state->part1;
    the_search->best_state->part2 = the_search->this_state->part2;
    replace_char_widths(chunks_record, chunk_groups);
  } else {
    ASSERT_HOST(!updated_best_choice);
    if (char_choices != NULL) fixpt->clear();
  }

  if (char_choices != NULL) delete char_choices;
  memfree(chunk_groups);

  return (keep_going);
}


/**
 * rebuild_current_state
 *
 * Transfers the given state to the word's output fields: rebuild_word,
 * best_state, box_word, and returns the corresponding blob choices.
 */
BLOB_CHOICE_LIST_VECTOR *Wordrec::rebuild_current_state(
    WERD_RES *word,
    STATE *state,
    BLOB_CHOICE_LIST_VECTOR *old_choices,
    MATRIX *ratings) {
  // Initialize search_state, num_joints, x, y.
  int num_joints = array_count(word->seam_array);
#ifndef GRAPHICS_DISABLED
    if (wordrec_display_segmentations) {
      print_state("Rebuilding state", state, num_joints);
    }
#endif
  // Setup the rebuild_word ready for the output blobs.
  if (word->rebuild_word != NULL)
    delete word->rebuild_word;
  word->rebuild_word = new TWERD;
  // Setup the best_state.
  word->best_state.clear();
  SEARCH_STATE search_state = bin_to_chunks(state, num_joints);
  // See which index is which below for information on x and y.
  int x = 0;
  int y;
  for (int i = 1; i <= search_state[0]; i++) {
    y = x + search_state[i];
    x = y + 1;
  }
  y = count_blobs(word->chopped_word->blobs) - 1;

  // Initialize char_choices, expanded_fragment_lengths:
  // e.g. if fragment_lengths = {1 1 2 3 1},
  // expanded_fragment_lengths_str = {1 1 2 2 3 3 3 1}.
  BLOB_CHOICE_LIST_VECTOR *char_choices = new BLOB_CHOICE_LIST_VECTOR();
  STRING expanded_fragment_lengths_str = "";
  bool state_has_fragments = false;
  const char *fragment_lengths = NULL;

  if (word->best_choice->length() > 0) {
    fragment_lengths = word->best_choice->fragment_lengths();
  }
  if (fragment_lengths) {
    for (int i = 0; i < word->best_choice->length(); ++i) {
      *char_choices += NULL;
      word->best_state.push_back(0);
      if (fragment_lengths[i] > 1) {
        state_has_fragments = true;
      }
      for (int j = 0; j < fragment_lengths[i]; ++j) {
        expanded_fragment_lengths_str += fragment_lengths[i];
      }
    }
  } else {
    for (int i = 0; i <= search_state[0]; ++i) {
      expanded_fragment_lengths_str += (char)1;
      *char_choices += NULL;
      word->best_state.push_back(0);
    }
  }

  // Set up variables for concatenating fragments.
  const char *word_lengths_ptr = NULL;
  const char *word_ptr = NULL;
  if (state_has_fragments) {
    // Make word_lengths_ptr point to the last element in
    // best_choice->unichar_lengths().
    word_lengths_ptr = word->best_choice->unichar_lengths().string();
    word_lengths_ptr += (strlen(word_lengths_ptr)-1);
    // Make word_str point to the beginning of the last
    // unichar in best_choice->unichar_string().
    word_ptr = word->best_choice->unichar_string().string();
    word_ptr += (strlen(word_ptr)-*word_lengths_ptr);
  }
  const char *expanded_fragment_lengths =
    expanded_fragment_lengths_str.string();
  char unichar[UNICHAR_LEN + 1];

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
  int ss_index = search_state[0];
  // Which index is which?
  // char_choices_index refers to the finished product: there is one for each
  // blob/unicharset entry in the final word.
  // ss_index refers to the search_state, and indexes a group (chunk) of blobs
  // that were classified together for the best state.
  // old_choice_index is a copy of ss_index, and accesses the old_choices,
  // which correspond to chunks in the best state. old_choice_index gets
  // set to -1 on a fragment set, as there is no corresponding chunk in
  // the best state.
  // x and y refer to the underlying blobs and are the first and last blob
  // indices in a chunk.
  for (int char_choices_index = char_choices->length() - 1;
       char_choices_index >= 0;
       --char_choices_index) {
    // The start and end of the blob to rebuild.
    int true_x = x;
    int true_y = y;
    // The fake merged fragment choice.
    BLOB_CHOICE* merged_choice = NULL;
    // Test for and combine fragments first.
    int fragment_pieces = expanded_fragment_lengths[ss_index];
    int old_choice_index = ss_index;

    if (fragment_pieces > 1) {
      strncpy(unichar, word_ptr, *word_lengths_ptr);
      unichar[*word_lengths_ptr] = '\0';
      merged_choice = rebuild_fragments(unichar, expanded_fragment_lengths,
                                        old_choice_index, old_choices);
      old_choice_index = -1;
    }
    while (fragment_pieces > 0) {
      true_x = x;
      // Move left to the previous blob.
      y = x - 1;
      x = y - search_state[ss_index--];
      --fragment_pieces;
    }
    word->best_state[char_choices_index] = true_y + 1 - true_x;
    BLOB_CHOICE_LIST *current_choices = join_blobs_and_classify(
        word, true_x, true_y, old_choice_index, ratings, old_choices);
    if (merged_choice != NULL) {
      // Insert merged_blob into current_choices, such that current_choices
      // are still sorted in non-descending order by rating.
      ASSERT_HOST(!current_choices->empty());
      BLOB_CHOICE_IT choice_it(current_choices);
      for (choice_it.mark_cycle_pt(); !choice_it.cycled_list() &&
           merged_choice->rating() > choice_it.data()->rating();
           choice_it.forward());
        choice_it.add_before_stay_put(merged_choice);
    }
    // Get rid of fragments in current_choices.
    BLOB_CHOICE_IT choice_it(current_choices);
    for (choice_it.mark_cycle_pt(); !choice_it.cycled_list();
        choice_it.forward()) {
      if (getDict().getUnicharset().get_fragment(
          choice_it.data()->unichar_id())) {
        delete choice_it.extract();
      }
    }
    char_choices->set(current_choices, char_choices_index);

    // Update word_ptr and word_lengths_ptr.
    if (word_lengths_ptr != NULL && word_ptr != NULL) {
      word_lengths_ptr--;
      word_ptr -= (*word_lengths_ptr);
    }
  }
  old_choices->delete_data_pointers();
  delete old_choices;
  memfree(search_state);

  return char_choices;
}


/**
 * @name expand_node
 *
 * Create the states that are attached to this one.  Check to see that
 * each one has not already been visited.  If not add it to the priority
 * queue.
 */
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
      if (wordrec_debug_level) {
        cprintf ("....checking state: %8.3f ", new_merit);
        print_state ("", the_search->this_state, num_joints);
      }
      if (new_merit < worst_priority) {
        push_queue(the_search->open_states, the_search->this_state,
                   worst_priority, new_merit, wordrec_debug_level > 0);
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
      if (wordrec_debug_level) {
        cprintf ("....checking state: %8.3f ", new_merit);
        print_state ("", the_search->this_state, num_joints);
      }
      if (new_merit < worst_priority) {
        push_queue(the_search->open_states, the_search->this_state,
                   worst_priority, new_merit, wordrec_debug_level > 0);
        nodes_added++;
      }
    }
    mask >>= 1;
  }
}

/**
 * @name new_search
 *
 * Create and initialize a new search record.
 */
SEARCH_RECORD *Wordrec::new_search(CHUNKS_RECORD *chunks_record,
                                   int num_joints,
                                   BLOB_CHOICE_LIST_VECTOR *best_char_choices,
                                   WERD_CHOICE *best_choice,
                                   WERD_CHOICE *raw_choice,
                                   STATE *state) {
  SEARCH_RECORD *this_search;

  this_search = (SEARCH_RECORD *) memalloc (sizeof (SEARCH_RECORD));

  this_search->open_states = MakeHeap (wordrec_num_seg_states * 20);
  this_search->closed_states = new_hash_table();

  if (state)
    this_search->this_state = new_state (state);
  else
    cprintf ("error: bad initial state in new_search\n");

  this_search->first_state = new_state (this_search->this_state);
  this_search->best_state = new_state (this_search->this_state);

  this_search->best_choice = best_choice;
  this_search->raw_choice = raw_choice;
  this_search->best_char_choices = best_char_choices;

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
STATE *Wordrec::pop_queue(HEAP *queue) {
  HEAPENTRY entry;

  if (GetTopOfHeap (queue, &entry) == TESS_HEAP_OK) {
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
void Wordrec::push_queue(HEAP *queue, STATE *state, FLOAT32 worst_priority,
                         FLOAT32 priority, bool debug) {
  HEAPENTRY entry;

  if (priority < worst_priority) {
    if (SizeOfHeap (queue) >= MaxSizeOfHeap(queue)) {
      if (debug) tprintf("Heap is Full\n");
      return;
    }
    if (debug) tprintf("\tpushing %d node  %f\n", num_pushed, priority);
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
void Wordrec::replace_char_widths(CHUNKS_RECORD *chunks_record,
                                  SEARCH_STATE state) {
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

// Creates a fake blob choice from the combination of the given fragments.
// unichar is the class to be made from the combination,
// expanded_fragment_lengths[choice_index] is the number of fragments to use.
// old_choices[choice_index] has the classifier output for each fragment.
// choice index initially indexes the last fragment and should be decremented
// expanded_fragment_lengths[choice_index] times to get the earlier fragments.
// Guarantees to return something non-null, or abort!
BLOB_CHOICE* Wordrec::rebuild_fragments(
    const char* unichar,
    const char* expanded_fragment_lengths,
    int choice_index,
    BLOB_CHOICE_LIST_VECTOR *old_choices) {
  float rating = 0.0f;
  float certainty = 0.0f;
  for (int fragment_pieces = expanded_fragment_lengths[choice_index] - 1;
       fragment_pieces >= 0; --fragment_pieces, --choice_index) {
    // Get a pointer to the classifier results from the old_choices.
    BLOB_CHOICE_LIST *current_choices = old_choices->get(choice_index);
    // Populate fragment with updated values and look for the
    // fragment with the same values in current_choices.
    // Update rating and certainty of the character being composed.
    CHAR_FRAGMENT fragment;
    fragment.set_all(unichar, fragment_pieces,
                     expanded_fragment_lengths[choice_index]);
    BLOB_CHOICE_IT choice_it(current_choices);
    for (choice_it.mark_cycle_pt(); !choice_it.cycled_list();
        choice_it.forward()) {
      const CHAR_FRAGMENT *current_fragment =
          getDict().getUnicharset().get_fragment(
              choice_it.data()->unichar_id());
      if (current_fragment && fragment.equals(current_fragment)) {
        rating += choice_it.data()->rating();
        if (choice_it.data()->certainty() < certainty) {
          certainty = choice_it.data()->certainty();
        }
        break;
      }
    }
    if (choice_it.cycled_list()) {
      print_ratings_list("Failure", current_choices, unicharset);
      tprintf("Failed to find fragment %s at index=%d\n",
              fragment.to_string().string(), choice_index);
    }
    ASSERT_HOST(!choice_it.cycled_list());  // Be sure we found the fragment.
  }
  return new BLOB_CHOICE(getDict().getUnicharset().unichar_to_id(unichar),
                         rating, certainty, -1, -1, 0);
}

// Creates a joined copy of the blobs between x and y (inclusive) and
// inserts as the first blob at word->rebuild_word->blobs.
// Returns a deep copy of the classifier results for the blob.
BLOB_CHOICE_LIST *Wordrec::join_blobs_and_classify(
    WERD_RES* word, int x, int y, int choice_index, MATRIX *ratings,
    BLOB_CHOICE_LIST_VECTOR *old_choices) {
  // Join parts to make the blob if needed.
  if (x != y)
    join_pieces(word->chopped_word->blobs, word->seam_array, x, y);
  TBLOB *blob = word->chopped_word->blobs;
  for (int i = 0; i < x; i++) {
    blob = blob->next;
  }
  // Deep copy this blob into the output word.
  TBLOB* copy_blob = new TBLOB(*blob);
  copy_blob->next = word->rebuild_word->blobs;
  word->rebuild_word->blobs = copy_blob;

  BLOB_CHOICE_LIST *choices = NULL;
  // First check to see if we can look up the classificaiton
  // in old_choices (if there is no need to merge blobs).
  if (choice_index >= 0 && old_choices != NULL) {
    choices = old_choices->get(choice_index);
    old_choices->set(NULL, choice_index);
  }
  // The ratings matrix filled in by the associator will contain the next most
  // up-to-date classification info. Thus we look up the classification there
  // next, and only call classify_blob() if the classification is not found.
  if (choices == NULL && ratings != NULL) {
    choices = ratings->get(x, y);
    if (choices != NOT_CLASSIFIED) {
      ratings->put(x, y, NULL);
    }
  }
  // Get the choices for the blob by classification if necessary.
  if (choices == NULL) {
    choices = classify_blob(blob, "rebuild", Orange);
  }
  // Undo join_pieces to restore the chopped word to its fully chopped state.
  if (x != y)
    break_pieces(blob, word->seam_array, x, y);
  return choices;
}

}  // namespace tesseract
