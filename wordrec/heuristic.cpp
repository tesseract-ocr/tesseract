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
#include <math.h>

// Note: "heuristic.h" is an empty file and deleted
#include "associate.h"
#include "bestfirst.h"
#include "seam.h"
#include "baseline.h"
#include "freelist.h"
#include "measure.h"
#include "ratngs.h"
#include "wordrec.h"

/*----------------------------------------------------------------------
              M a c r o s
----------------------------------------------------------------------*/
#define BAD_RATING   1000.0      /* No valid blob */


namespace tesseract {

/*----------------------------------------------------------------------
// Some static helpers used only in this file
----------------------------------------------------------------------*/

// Return a character width record corresponding to the character
// width that will be generated in this segmentation state.
// The calling function need to memfree WIDTH_RECORD when finished.
// This is the same as the original function, only cosmetic changes,
// except instead of passing chunks back to be freed, it deallocates
// internally.
WIDTH_RECORD *Wordrec::state_char_widths(WIDTH_RECORD *chunk_widths,
                                         STATE *state,
                                         int num_joints) {
  SEARCH_STATE chunks = bin_to_chunks(state, num_joints);
  int num_chars = chunks[0] + 1;

  // allocate and store (n+1,w0,g0,w1,g1...,wn) in int[2*(n+1)] as a
  // struct { num_chars, widths[2*n+1]; }
  WIDTH_RECORD *char_widths = (WIDTH_RECORD*) memalloc(sizeof(int)*num_chars*2);
  char_widths->num_chars = num_chars;

  int first_blob = 0;
  int last_blob;
  for (int i = 1; i <= num_chars; i++) {
    last_blob = (i > chunks[0]) ? num_joints : first_blob + chunks[i];

    char_widths->widths[2*i-2] =
      AssociateUtils::GetChunksWidth(chunk_widths, first_blob, last_blob);
    if (i <= chunks[0]) {
      char_widths->widths[2*i-1] =
        AssociateUtils::GetChunksGap(chunk_widths, last_blob);
    }

    if (segment_adjust_debug > 3)
      tprintf("width_record[%d]s%d--s%d(%d) %d %d:%d\n",
              i-1, first_blob, last_blob, chunks[i],
              char_widths->widths[2*i-2], char_widths->widths[2*i-1],
              chunk_widths->widths[2*last_blob+1]);
    first_blob = last_blob + 1;
  }

  memfree(chunks);
  return char_widths;
}

// Computes the variance of the char widths normalized to the given height
// TODO(dsl): Do this in a later stage and use char choice info to skip
// punctuations.
FLOAT32 Wordrec::get_width_variance(WIDTH_RECORD *wrec, float norm_height) {
  MEASUREMENT ws;
  new_measurement(ws);
  for (int x = 0; x < wrec->num_chars; x++) {
    FLOAT32 wh_ratio = wrec->widths[2 * x] * 1.0f / norm_height;
    if (x == wrec->num_chars - 1 && wh_ratio > 0.3)
      continue;   // exclude trailing punctuation from stats
    ADD_SAMPLE(ws, wh_ratio);
  }
  if (segment_adjust_debug > 2)
    tprintf("Width Mean=%g Var=%g\n", MEAN(ws), VARIANCE(ws));
  return VARIANCE(ws);
}

// Computes the variance of char positioning (width + spacing) wrt norm_height
FLOAT32 Wordrec::get_gap_variance(WIDTH_RECORD *wrec, float norm_height) {
  MEASUREMENT ws;
  new_measurement(ws);
  for (int x = 0; x < wrec->num_chars - 1; x++) {
    FLOAT32 gap_ratio = (wrec->widths[2 * x] + wrec->widths[ 2*x + 1])
                        * 1.0 / norm_height;
    ADD_SAMPLE(ws, gap_ratio);
  }
  if (segment_adjust_debug > 2)
    tprintf("Gap Mean=%g Var=%g\n", MEAN(ws), VARIANCE(ws));
  return VARIANCE(ws);
}


/*----------------------------------------------------------------------
 Below are the new state prioritization functions, which incorporates
 segmentation cost and width distribution for fixed pitch languages.
 They are included as methods in class Wordrec to obtain more context.
 ----------------------------------------------------------------------*/

/**********************************************************************
 * Returns the cost associated with the segmentation state.
 *
 * The number of states should be the same as the number of seams.
 * If state[i] is 1, then seams[i] is present; otherwise, it is hidden.
 * This function returns the sum of priority for active seams.
 * TODO(dsl): To keep this clean, this function will just return the
 * sum of raw "priority" as seam cost.  The normalization of this score
 * relative to other costs will be done in bestfirst.cpp evaluate_seg().
 **********************************************************************/

FLOAT32 Wordrec::seamcut_priority(SEAMS seams,
                                  STATE *state,
                                  int num_joints) {
  int x;
  unsigned int mask = (num_joints > 32) ? (1 << (num_joints - 1 - 32))
                                        : (1 << (num_joints - 1));
  float seam_cost = 0.0f;
  for (x = num_joints - 1; x >= 0; x--) {
    int i = num_joints - 1 - x;
    uinT32 value = (x < 32) ? state->part2 : state->part1;
    bool state_on = value & mask;
    if (state_on) {
      SEAM* seam = (SEAM *) array_value(seams, i);
      seam_cost += seam->priority;
    }
    if (mask == 1)
      mask = 1 << 31;
    else
      mask >>= 1;
  }
  if (segment_adjust_debug > 2)
    tprintf("seam_cost: %f\n", seam_cost);
  return seam_cost;
}

/**********************************************************************
 * rating_priority
 *
 * Assign a segmentation priority based on the ratings of the blobs
 * (in that segmentation) that have been classified.  The average
 * "goodness" (i.e. rating / weight) for each blob is used to indicate
 * the segmentation priority.  This is the original.
 **********************************************************************/
FLOAT32 Wordrec::rating_priority(CHUNKS_RECORD *chunks_record,
                                 STATE *state,
                                 int num_joints) {
  BLOB_CHOICE_LIST *blob_choices;
  BLOB_CHOICE_IT blob_choice_it;
  inT16 first_chunk = 0;
  inT16 last_chunk;
  inT16 ratings = 0;
  inT16 weights = 0;

  PIECES_STATE blob_chunks;
  bin_to_pieces(state, num_joints, blob_chunks);

  for (int x = 0; blob_chunks[x]; x++) {
    last_chunk = first_chunk + blob_chunks[x];

    blob_choices = chunks_record->ratings->get(first_chunk, last_chunk - 1);
    if (blob_choices != NOT_CLASSIFIED && blob_choices->length() > 0) {
      blob_choice_it.set_to_list(blob_choices);
      ratings += (inT16) blob_choice_it.data()->rating();
      for (int y = first_chunk; y < last_chunk; y++) {
        weights += (inT16) (chunks_record->weights[y]);
      }
    }
    first_chunk = last_chunk;
  }
  if (weights <= 0)
    weights = 1;
  FLOAT32 rating_cost = static_cast<FLOAT32>(ratings) /
                        static_cast<FLOAT32>(weights);
  if (segment_adjust_debug > 2)
    tprintf("rating_cost: r%f / w%f = %f\n", ratings, weights, rating_cost);
  return rating_cost;
}

/**********************************************************************
 * width_priority
 *
 * Return a priority value for this word segmentation based on the
 * character widths present in the new segmentation.
 * For variable-pitch fonts, this should do the same thing as before:
 * ie. penalize only on really wide squatting blobs.
 * For fixed-pitch fonts, this will include a measure of char & gap
 * width consistency.
 * TODO(dsl): generalize this to use a PDF estimate for proportional and
 * fixed pitch mode.
 **********************************************************************/
FLOAT32 Wordrec::width_priority(CHUNKS_RECORD *chunks_record,
                                STATE *state,
                                int num_joints) {
  FLOAT32 penalty = 0.0;
  WIDTH_RECORD *width_rec = state_char_widths(chunks_record->chunk_widths,
                                              state, num_joints);
  // When baseline_enable==True, which is the current default for Tesseract,
  // a fixed value of 128 (BASELINE_SCALE) is always used.
  FLOAT32 normalizing_height = BASELINE_SCALE;
  if (assume_fixed_pitch_char_segment) {
    // For fixed pitch language like CJK, we use the full text height as the
    // normalizing factor so we are not dependent on xheight calculation.
    // In the normalized coord. xheight * scale == BASELINE_SCALE(128),
    // so add proportionally scaled ascender zone to get full text height.
    normalizing_height = denorm_.y_scale() *
        (denorm_.row()->x_height() + denorm_.row()->ascenders());
    if (segment_adjust_debug > 1)
      tprintf("WidthPriority: %f %f normalizing height = %f\n",
              denorm_.row()->x_height(), denorm_.row()->ascenders(),
              normalizing_height);
    // Impose additional segmentation penalties if blob widths or gaps
    // distribution don't fit a fixed-pitch model.
    FLOAT32 width_var = get_width_variance(width_rec, normalizing_height);
    FLOAT32 gap_var = get_gap_variance(width_rec, normalizing_height);
    penalty += width_var;
    penalty += gap_var;
  }

  for (int x = 0; x < width_rec->num_chars; x++) {
    FLOAT32 squat = width_rec->widths[2*x];
    FLOAT32 gap = (x < width_rec->num_chars-1) ? width_rec->widths[2*x+1] : 0;
    squat /= normalizing_height;
    gap /= normalizing_height;
    if (assume_fixed_pitch_char_segment) {
      penalty += AssociateUtils::FixedPitchWidthCost(
          squat, 0.0f, x == 0 || x == width_rec->num_chars -1,
          heuristic_max_char_wh_ratio);
      penalty += AssociateUtils::FixedPitchGapCost(
          gap, x == width_rec->num_chars - 1);
      if (width_rec->num_chars == 1 &&
          squat > AssociateUtils::kMaxFixedPitchCharAspectRatio) {
        penalty += 10;
      }
    } else {
      // Original equation when
      // heuristic_max_char_ratio == AssociateUtils::kMaxSquat
      if (squat > heuristic_max_char_wh_ratio)
        penalty += squat - heuristic_max_char_wh_ratio;
    }
  }

  free_widths(width_rec);
  return (penalty);
}


/**********************************************************************
 * prioritize_state
 *
 * Create a priority for this state.  It represents the urgency of
 * checking this state.  The larger the priority value, the worse the
 * state is (lower priority).  The "value" of this priority should be
 * somewhat consistent with the final word rating.
 * The question is how to normalize the different scores, and adjust
 * the relative importance among them.
 **********************************************************************/
FLOAT32 Wordrec::prioritize_state(CHUNKS_RECORD *chunks_record,
                                  SEARCH_RECORD *the_search) {
  FLOAT32 shape_cost;
  FLOAT32 width_cost;
  FLOAT32 seam_cost;

  shape_cost = rating_priority(chunks_record,
                               the_search->this_state,
                               the_search->num_joints);

  width_cost = width_priority(chunks_record,
                              the_search->this_state,
                              the_search->num_joints);

  // The rating_priority is the same as the original, and the width_priority
  // is the same as before if assume_fixed_pitch_char_segment == FALSE.
  // So this would return the original state priority.
  if (!use_new_state_cost)
    return width_cost * 1000 + shape_cost;

  seam_cost = seamcut_priority(chunks_record->splits,
                               the_search->this_state,
                               the_search->num_joints);

  // TODO(dsl): how do we normalize the scores for these separate evidence?
  // FLOAT32 total_cost = shape_cost + width_cost * 0.01 + seam_cost * 0.001;
  FLOAT32 total_cost = shape_cost * heuristic_weight_rating +
                       width_cost * heuristic_weight_width +
                       seam_cost * heuristic_weight_seamcut;

  // We don't have an adjustment model for variable pitch segmentation cost
  // into word rating
  if (assume_fixed_pitch_char_segment) {
    float seg_bias = 1.0;
    if (width_cost < 1) seg_bias *= 0.85;
    if (width_cost > 3)
      seg_bias *= pow(heuristic_segcost_rating_base, width_cost/3.0);
    if (seam_cost > 10)
      seg_bias *= pow(heuristic_segcost_rating_base, log(seam_cost)/log(10.0));
    if (shape_cost > 5)
      seg_bias *= pow(heuristic_segcost_rating_base, shape_cost/5.0);
    if (segment_adjust_debug) {
      tprintf("SegCost: %g Weight: %g rating: %g  width: %g  seam: %g\n",
               total_cost, seg_bias, shape_cost, width_cost, seam_cost);
    }
    the_search->segcost_bias = seg_bias;
  } else {
    the_search->segcost_bias = 0;
  }

  return total_cost;
}

}  // namespace tesseract
