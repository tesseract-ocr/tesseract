/* -*-C-*-
 ********************************************************************************
 *
 * File:        heuristic.c  (Formerly heuristic.c)
 * Description:
 * Author:       Mark Seaman, OCR Technology
 * Created:      Fri Oct 16 14:37:00 1987
 * Modified:     Wed Jul 10 14:15:08 1991 (Mark Seaman) marks@hpgrlt
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
#include "heuristic.h"
#include "baseline.h"
#include "metrics.h"
#include "freelist.h"
#include <math.h>

/*----------------------------------------------------------------------
              M a c r o s
----------------------------------------------------------------------*/
#define MAX_SQUAT       2.0      /* Width ratio */
#define BAD_RATING   1000.0      /* No valid blob */

/*----------------------------------------------------------------------
              F u n c t i o n s
----------------------------------------------------------------------*/
/**********************************************************************
 * prioritize_state
 *
 * Create a priority for this state.  It represents the urgency of
 * checking this state.
 **********************************************************************/
FLOAT32 prioritize_state(CHUNKS_RECORD *chunks_record,
                         SEARCH_RECORD *the_search,
                         STATE *old_state) {
  FLOAT32 width_pri;
  FLOAT32 match_pri;

  match_pri = rating_priority (chunks_record, the_search->this_state,
    old_state, the_search->num_joints);

  width_pri = width_priority (chunks_record, the_search->this_state,
    the_search->num_joints) * 1000.0;

  record_priorities(the_search, old_state, match_pri, width_pri);

  return (width_pri + match_pri);
}


/**********************************************************************
 * rating_priority
 *
 * Assign a segmentation priority based on the ratings of the blobs
 * (in that segmentation) that have been classified.  The average
 * "goodness" (i.e. rating / weight) for each blob is used to indicate
 * the segmentation priority.
 **********************************************************************/
FLOAT32 rating_priority(CHUNKS_RECORD *chunks_record,
                        STATE *state,
                        STATE *old_state,
                        int num_joints) {
  PIECES_STATE blob_chunks;
  inT16 x;
  inT16 y;
  CHOICES this_choice;
  inT16 first_chunk = 0;
  inT16 last_chunk;
  inT16 ratings = 0;
  inT16 weights = 0;

  bin_to_pieces(state, num_joints, blob_chunks);

  for (x = 0; blob_chunks[x]; x++) {
                                 // Iterate each blob
    last_chunk = first_chunk + blob_chunks[x] - 1;

    this_choice = matrix_get (chunks_record->ratings,
      first_chunk, last_chunk);

    if (this_choice == NIL)
      return (BAD_RATING);

    if (this_choice != NOT_CLASSIFIED) {

      ratings += (inT16) best_probability (this_choice);
      for (y = first_chunk; y <= last_chunk; y++) {
        weights += (inT16) (chunks_record->weights[y]);
      }
    }
    first_chunk += blob_chunks[x];
  }
  if (weights <= 0)
    weights = 1;
  return ((FLOAT32) ratings / weights);
}


/**********************************************************************
 * state_char_widths
 *
 * Return a character width record corresponding to the character
 * width that will be generated in this segmentation state.
 **********************************************************************/
WIDTH_RECORD *state_char_widths(WIDTH_RECORD *chunk_widths,
                                STATE *state,
                                int num_joints,
                                SEARCH_STATE *search_state) {
  WIDTH_RECORD *width_record;
  int num_blobs;
  int x;
  int y;
  int i;
  SEARCH_STATE new_chunks;

  new_chunks = bin_to_chunks (state, num_joints);

  num_blobs = new_chunks[0] + 1;
  width_record = (WIDTH_RECORD *) memalloc (sizeof (int) * num_blobs * 2);
  width_record->num_chars = num_blobs;

  x = 0;
  for (i = 1; i <= new_chunks[0] + 1; i++) {
    if (i > new_chunks[0])
      y = num_joints;
    else
      y = x + new_chunks[i];

    width_record->widths[2 * i - 2] = chunks_width (chunk_widths, x, y);

    if (i <= new_chunks[0])
      width_record->widths[2 * i - 1] = chunks_gap (chunk_widths, y);

    x = y + 1;
  }

  *search_state = new_chunks;
  return (width_record);
}


/**********************************************************************
 * width_priority
 *
 * Return a priority value for this word segmentation based on the
 * character widths present in the new segmentation.
 **********************************************************************/
FLOAT32 width_priority(CHUNKS_RECORD *chunks_record,
                       STATE *state,
                       int num_joints) {
  SEARCH_STATE new_chunks;
  FLOAT32 result = 0.0;
  WIDTH_RECORD *width_record;
  FLOAT32 squat;
  int x;

  width_record = state_char_widths (chunks_record->chunk_widths,
    state, num_joints, &new_chunks);
  for (x = 0; x < width_record->num_chars; x++) {

    squat = width_record->widths[2 * x];
    if (!baseline_enable) {
      squat /= chunks_record->row->lineheight;
    }
    else {
      squat /= BASELINE_SCALE;
    }

    if (squat > MAX_SQUAT)
      result += squat - MAX_SQUAT;

  }

  memfree(new_chunks);
  free_widths(width_record);

  return (result);
}
