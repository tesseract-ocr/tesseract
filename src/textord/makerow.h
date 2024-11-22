/**********************************************************************
 * File:        makerow.h  (Formerly makerows.h)
 * Description: Code to arrange blobs into rows of text.
 * Author:      Ray Smith
 *
 * (C) Copyright 1992, Hewlett-Packard Ltd.
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
 **********************************************************************/

#ifndef MAKEROW_H
#define MAKEROW_H

#include "blobbox.h"
#include "blobs.h"
#include "ocrblock.h"
#include "params.h"
#include "statistc.h"

namespace tesseract {

enum OVERLAP_STATE {
  ASSIGN, // assign it to row
  REJECT, // reject it - dual overlap
  NEW_ROW
};

enum ROW_CATEGORY {
  ROW_ASCENDERS_FOUND,
  ROW_DESCENDERS_FOUND,
  ROW_UNKNOWN,
  ROW_INVALID,
};

extern BOOL_VAR_H(textord_heavy_nr);
extern BOOL_VAR_H(textord_show_initial_rows);
extern BOOL_VAR_H(textord_show_parallel_rows);
extern BOOL_VAR_H(textord_show_expanded_rows);
extern BOOL_VAR_H(textord_show_final_rows);
extern BOOL_VAR_H(textord_show_final_blobs);
extern BOOL_VAR_H(textord_test_landscape);
extern BOOL_VAR_H(textord_parallel_baselines);
extern BOOL_VAR_H(textord_straight_baselines);
extern BOOL_VAR_H(textord_old_baselines);
extern BOOL_VAR_H(textord_old_xheight);
extern BOOL_VAR_H(textord_fix_xheight_bug);
extern BOOL_VAR_H(textord_fix_makerow_bug);
extern BOOL_VAR_H(textord_debug_xheights);
extern INT_VAR_H(textord_test_x);
extern INT_VAR_H(textord_test_y);
extern INT_VAR_H(textord_min_blobs_in_row);
extern INT_VAR_H(textord_spline_minblobs);
extern INT_VAR_H(textord_spline_medianwin);
extern INT_VAR_H(textord_min_xheight);
extern double_VAR_H(textord_spline_shift_fraction);
extern double_VAR_H(textord_skew_ile);
extern double_VAR_H(textord_skew_lag);
extern double_VAR_H(textord_linespace_iqrlimit);
extern double_VAR_H(textord_width_limit);
extern double_VAR_H(textord_chop_width);
extern double_VAR_H(textord_minxh);
extern double_VAR_H(textord_min_linesize);
extern double_VAR_H(textord_excess_blobsize);
extern double_VAR_H(textord_occupancy_threshold);
extern double_VAR_H(textord_underline_width);
extern double_VAR_H(textord_min_blob_height_fraction);
extern double_VAR_H(textord_xheight_mode_fraction);
extern double_VAR_H(textord_ascheight_mode_fraction);
extern double_VAR_H(textord_ascx_ratio_min);
extern double_VAR_H(textord_ascx_ratio_max);
extern double_VAR_H(textord_descx_ratio_min);
extern double_VAR_H(textord_descx_ratio_max);
extern double_VAR_H(textord_xheight_error_margin);
extern INT_VAR_H(textord_lms_line_trials);
extern BOOL_VAR_H(textord_new_initial_xheight);
extern BOOL_VAR_H(textord_debug_blob);

inline void get_min_max_xheight(int block_linesize, int *min_height, int *max_height) {
  *min_height = static_cast<int32_t>(floor(block_linesize * textord_minxh));
  if (*min_height < textord_min_xheight) {
    *min_height = textord_min_xheight;
  }
  *max_height = static_cast<int32_t>(ceil(block_linesize * 3.0));
}

inline ROW_CATEGORY get_row_category(const TO_ROW *row) {
  if (row->xheight <= 0) {
    return ROW_INVALID;
  }
  return (row->ascrise > 0) ? ROW_ASCENDERS_FOUND
                            : (row->descdrop != 0) ? ROW_DESCENDERS_FOUND : ROW_UNKNOWN;
}

inline bool within_error_margin(float test, float num, float margin) {
  return (test >= num * (1 - margin) && test <= num * (1 + margin));
}

void fill_heights(TO_ROW *row, float gradient, int min_height, int max_height, STATS *heights,
                  STATS *floating_heights);

float make_single_row(ICOORD page_tr, bool allow_sub_blobs, TO_BLOCK *block, TO_BLOCK_LIST *blocks);
float make_rows(ICOORD page_tr, // top right
                TO_BLOCK_LIST *port_blocks);
void make_initial_textrows(ICOORD page_tr,
                           TO_BLOCK *block,  // block to do
                           FCOORD rotation,  // for drawing
                           bool testing_on); // correct orientation
void fit_lms_line(TO_ROW *row);
void compute_page_skew(TO_BLOCK_LIST *blocks, // list of blocks
                       float &page_m,         // average gradient
                       float &page_err);      // average error
void vigorous_noise_removal(TO_BLOCK *block);
void cleanup_rows_making(ICOORD page_tr,     // top right
                         TO_BLOCK *block,    // block to do
                         float gradient,     // gradient to fit
                         FCOORD rotation,    // for drawing
                         int32_t block_edge, // edge of block
                         bool testing_on);   // correct orientation
void delete_non_dropout_rows(                // find lines
    TO_BLOCK *block,                         // block to do
    float gradient,                          // global skew
    FCOORD rotation,                         // deskew vector
    int32_t block_edge,                      // left edge
    bool testing_on                          // correct orientation
);
bool find_best_dropout_row( // find neighbours
    TO_ROW *row,            // row to test
    int32_t distance,       // dropout dist
    float dist_limit,       // threshold distance
    int32_t line_index,     // index of row
    TO_ROW_IT *row_it,      // current position
    bool testing_on         // correct orientation
);
TBOX deskew_block_coords( // block box
    TO_BLOCK *block,      // block to do
    float gradient        // global skew
);
void compute_line_occupation( // project blobs
    TO_BLOCK *block,          // block to do
    float gradient,           // global skew
    int32_t min_y,            // min coord in block
    int32_t max_y,            // in block
    int32_t *occupation,      // output projection
    int32_t *deltas           // derivative
);
void compute_occupation_threshold( // project blobs
    int32_t low_window,            // below result point
    int32_t high_window,           // above result point
    int32_t line_count,            // array sizes
    int32_t *occupation,           // input projection
    int32_t *thresholds            // output thresholds
);
void compute_dropout_distances( // project blobs
    int32_t *occupation,        // input projection
    int32_t *thresholds,        // output thresholds
    int32_t line_count          // array sizes
);
void expand_rows(       // find lines
    ICOORD page_tr,     // top right
    TO_BLOCK *block,    // block to do
    float gradient,     // gradient to fit
    FCOORD rotation,    // for drawing
    int32_t block_edge, // edge of block
    bool testing_on     // correct orientation
);
void adjust_row_limits( // tidy limits
    TO_BLOCK *block     // block to do
);
void compute_row_stats( // find lines
    TO_BLOCK *block,    // block to do
    bool testing_on     // correct orientation
);
float median_block_xheight( // find lines
    TO_BLOCK *block,        // block to do
    float gradient          // global skew
);

int compute_xheight_from_modes(STATS *heights, STATS *floating_heights, bool cap_only,
                               int min_height, int max_height, float *xheight, float *ascrise);

int32_t compute_row_descdrop(TO_ROW *row,    // row to do
                             float gradient, // global skew
                             int xheight_blob_count, STATS *heights);
int32_t compute_height_modes(STATS *heights,     // stats to search
                             int32_t min_height, // bottom of range
                             int32_t max_height, // top of range
                             int32_t *modes,     // output array
                             int32_t maxmodes);  // size of modes
void correct_row_xheight(TO_ROW *row,            // row to fix
                         float xheight,          // average values
                         float ascrise, float descdrop);
void separate_underlines(TO_BLOCK *block,   // block to do
                         float gradient,    // skew angle
                         FCOORD rotation,   // inverse landscape
                         bool testing_on);  // correct orientation
void pre_associate_blobs(ICOORD page_tr,    // top right
                         TO_BLOCK *block,   // block to do
                         FCOORD rotation,   // inverse landscape
                         bool testing_on);  // correct orientation
void fit_parallel_rows(TO_BLOCK *block,     // block to do
                       float gradient,      // gradient to fit
                       FCOORD rotation,     // for drawing
                       int32_t block_edge,  // edge of block
                       bool testing_on);    // correct orientation
void fit_parallel_lms(float gradient,       // forced gradient
                      TO_ROW *row);         // row to fit
void make_baseline_spline(TO_ROW *row,      // row to fit
                          TO_BLOCK *block); // block it came from
bool segment_baseline(                      // split baseline
    TO_ROW *row,                            // row to fit
    TO_BLOCK *block,                        // block it came from
    int32_t &segments,                      // no fo segments
    int32_t *xstarts                        // coords of segments
);
double *linear_spline_baseline( // split baseline
    TO_ROW *row,                // row to fit
    TO_BLOCK *block,            // block it came from
    int32_t &segments,          // no fo segments
    int32_t xstarts[]           // coords of segments
);
void assign_blobs_to_rows( // find lines
    TO_BLOCK *block,       // block to do
    float *gradient,       // block skew
    int pass,              // identification
    bool reject_misses,    // chuck big ones out
    bool make_new_rows,    // add rows for unmatched
    bool drawing_skew      // draw smoothed skew
);
// find best row
OVERLAP_STATE most_overlapping_row(TO_ROW_IT *row_it, // iterator
                                   TO_ROW *&best_row, // output row
                                   float top,         // top of blob
                                   float bottom,      // bottom of blob
                                   float rowsize,     // max row size
                                   bool testing_blob  // test stuff
);
int blob_x_order(      // sort function
    const BLOBNBOX *item1, // items to compare
    const BLOBNBOX *item2);

void mark_repeated_chars(TO_ROW *row);

} // namespace tesseract

#endif
