/**********************************************************************
 * File:        makerow.h  (Formerly makerows.h)
 * Description: Code to arrange blobs into rows of text.
 * Author:		Ray Smith
 * Created:		Mon Sep 21 14:34:48 BST 1992
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

#ifndef           MAKEROW_H
#define           MAKEROW_H

#include          "params.h"
#include          "ocrblock.h"
#include          "blobs.h"
#include          "blobbox.h"
#include          "statistc.h"
#include          "notdll.h"

enum OVERLAP_STATE
{
  ASSIGN,                        //assign it to row
  REJECT,                        //reject it - dual overlap
  NEW_ROW
};

enum ROW_CATEGORY {
  ROW_ASCENDERS_FOUND,
  ROW_DESCENDERS_FOUND,
  ROW_UNKNOWN,
  ROW_INVALID,
};

extern BOOL_VAR_H (textord_show_initial_rows, FALSE,
"Display row accumulation");
extern BOOL_VAR_H (textord_show_parallel_rows, FALSE,
"Display page correlated rows");
extern BOOL_VAR_H (textord_show_expanded_rows, FALSE,
"Display rows after expanding");
extern BOOL_VAR_H (textord_show_final_rows, FALSE,
"Display rows after final fitting");
extern BOOL_VAR_H (textord_show_final_blobs, FALSE,
"Display blob bounds after pre-ass");
extern BOOL_VAR_H (textord_test_landscape, FALSE, "Tests refer to land/port");
extern BOOL_VAR_H (textord_parallel_baselines, TRUE,
"Force parallel baselines");
extern BOOL_VAR_H (textord_straight_baselines, FALSE,
"Force straight baselines");
extern BOOL_VAR_H (textord_quadratic_baselines, FALSE,
"Use quadratic splines");
extern BOOL_VAR_H (textord_old_baselines, TRUE, "Use old baseline algorithm");
extern BOOL_VAR_H (textord_old_xheight, TRUE, "Use old xheight algorithm");
extern BOOL_VAR_H (textord_fix_xheight_bug, TRUE, "Use spline baseline");
extern BOOL_VAR_H (textord_fix_makerow_bug, TRUE,
"Prevent multiple baselines");
extern BOOL_VAR_H (textord_cblob_blockocc, TRUE,
"Use new projection for underlines");
extern BOOL_VAR_H (textord_debug_xheights, FALSE, "Test xheight algorithms");
extern INT_VAR_H (textord_test_x, 0, "coord of test pt");
extern INT_VAR_H (textord_test_y, 0, "coord of test pt");
extern INT_VAR_H (textord_min_blobs_in_row, 4,
"Min blobs before gradient counted");
extern INT_VAR_H (textord_spline_minblobs, 8,
"Min blobs in each spline segment");
extern INT_VAR_H (textord_spline_medianwin, 6,
"Size of window for spline segmentation");
extern INT_VAR_H (textord_min_xheight, 10, "Min credible pixel xheight");
extern double_VAR_H (textord_spline_shift_fraction, 0.02,
"Fraction of line spacing for quad");
extern double_VAR_H (textord_spline_outlier_fraction, 0.1,
"Fraction of line spacing for outlier");
extern double_VAR_H (textord_skew_ile, 0.5, "Ile of gradients for page skew");
extern double_VAR_H (textord_skew_lag, 0.75,
"Lag for skew on row accumulation");
extern double_VAR_H (textord_linespace_iqrlimit, 0.2,
"Max iqr/median for linespace");
extern double_VAR_H (textord_width_limit, 8,
"Max width of blobs to make rows");
extern double_VAR_H (textord_chop_width, 1.5, "Max width before chopping");
extern double_VAR_H (textord_minxh, 0.25,
"fraction of linesize for min xheight");
extern double_VAR_H (textord_min_linesize, 1.25,
"* blob height for initial linesize");
extern double_VAR_H (textord_excess_blobsize, 1.3,
"New row made if blob makes row this big");
extern double_VAR_H (textord_occupancy_threshold, 0.4,
"Fraction of neighbourhood");
extern double_VAR_H (textord_underline_width, 2.0,
"Multiple of line_size for underline");
extern double_VAR_H(textord_min_blob_height_fraction, 0.75,
"Min blob height/top to include blob top into xheight stats");
extern double_VAR_H (textord_xheight_mode_fraction, 0.4,
"Min pile height to make xheight");
extern double_VAR_H (textord_ascheight_mode_fraction, 0.15,
"Min pile height to make ascheight");
extern double_VAR_H (textord_ascx_ratio_min, 1.2, "Min cap/xheight");
extern double_VAR_H (textord_ascx_ratio_max, 1.7, "Max cap/xheight");
extern double_VAR_H (textord_descx_ratio_min, 0.15, "Min desc/xheight");
extern double_VAR_H (textord_descx_ratio_max, 0.6, "Max desc/xheight");
extern double_VAR_H (textord_xheight_error_margin, 0.1, "Accepted variation");
extern INT_VAR_H (textord_lms_line_trials, 12, "Number of linew fits to do");
extern BOOL_VAR_H (textord_new_initial_xheight, TRUE,
"Use test xheight mechanism");

inline void get_min_max_xheight(int block_linesize,
                                int *min_height, int *max_height) {
  *min_height = static_cast<inT32>(floor(block_linesize * textord_minxh));
  if (*min_height < textord_min_xheight) *min_height = textord_min_xheight;
  *max_height = static_cast<inT32>(ceil(block_linesize * 3.0));
}

inline ROW_CATEGORY get_row_category(const TO_ROW *row) {
  if (row->xheight <= 0) return ROW_INVALID;
  return (row->ascrise > 0) ? ROW_ASCENDERS_FOUND :
    (row->descdrop != 0) ? ROW_DESCENDERS_FOUND : ROW_UNKNOWN;
}

inline bool within_error_margin(float test, float num, float margin) {
  return (test >= num * (1 - margin) && test <= num * (1 + margin));
}

void fill_heights(TO_ROW *row, float gradient, int min_height,
                  int max_height, STATS *heights, STATS *floating_heights);

float make_single_row(ICOORD page_tr, TO_BLOCK* block,
                      TO_BLOCK_LIST* blocks);
float make_rows(ICOORD page_tr,              // top right
                TO_BLOCK_LIST *port_blocks);
void make_initial_textrows(ICOORD page_tr,
                           TO_BLOCK *block,  // block to do
                           FCOORD rotation,  // for drawing
                           BOOL8 testing_on);  // correct orientation
void fit_lms_line(TO_ROW *row);
void compute_page_skew(TO_BLOCK_LIST *blocks,  // list of blocks
                       float &page_m,          // average gradient
                       float &page_err);       // average error
void cleanup_rows_making(ICOORD page_tr,     // top right
                         TO_BLOCK *block,    // block to do
                         float gradient,     // gradient to fit
                         FCOORD rotation,    // for drawing
                         inT32 block_edge,   // edge of block
                         BOOL8 testing_on);  // correct orientation
void delete_non_dropout_rows(                   //find lines
                             TO_BLOCK *block,   //block to do
                             float gradient,    //global skew
                             FCOORD rotation,   //deskew vector
                             inT32 block_edge,  //left edge
                             BOOL8 testing_on   //correct orientation
                            );
BOOL8 find_best_dropout_row(                    //find neighbours
                            TO_ROW *row,        //row to test
                            inT32 distance,     //dropout dist
                            float dist_limit,   //threshold distance
                            inT32 line_index,   //index of row
                            TO_ROW_IT *row_it,  //current position
                            BOOL8 testing_on    //correct orientation
                           );
TBOX deskew_block_coords(                  //block box
                        TO_BLOCK *block,  //block to do
                        float gradient    //global skew
                       );
void compute_line_occupation(                    //project blobs
                             TO_BLOCK *block,    //block to do
                             float gradient,     //global skew
                             inT32 min_y,        //min coord in block
                             inT32 max_y,        //in block
                             inT32 *occupation,  //output projection
                             inT32 *deltas       //derivative
                            );
void compute_occupation_threshold(                    //project blobs
                                  inT32 low_window,   //below result point
                                  inT32 high_window,  //above result point
                                  inT32 line_count,   //array sizes
                                  inT32 *occupation,  //input projection
                                  inT32 *thresholds   //output thresholds
                                 );
void compute_dropout_distances(                    //project blobs
                               inT32 *occupation,  //input projection
                               inT32 *thresholds,  //output thresholds
                               inT32 line_count    //array sizes
                              );
void expand_rows(                   //find lines
                 ICOORD page_tr,    //top right
                 TO_BLOCK *block,   //block to do
                 float gradient,    //gradient to fit
                 FCOORD rotation,   //for drawing
                 inT32 block_edge,  //edge of block
                 BOOL8 testing_on   //correct orientation
                );
void adjust_row_limits(                 //tidy limits
                       TO_BLOCK *block  //block to do
                      );
void compute_row_stats(                  //find lines
                       TO_BLOCK *block,  //block to do
                       BOOL8 testing_on  //correct orientation
                      );
float median_block_xheight(                  //find lines
                           TO_BLOCK *block,  //block to do
                           float gradient    //global skew
                          );

int compute_xheight_from_modes(
    STATS *heights, STATS *floating_heights, bool cap_only, int min_height,
    int max_height, float *xheight, float *ascrise);

inT32 compute_row_descdrop(TO_ROW *row,     // row to do
                           float gradient,  // global skew
                           int xheight_blob_count,
                           STATS *heights);
inT32 compute_height_modes(STATS *heights,    // stats to search
                           inT32 min_height,  // bottom of range
                           inT32 max_height,  // top of range
                           inT32 *modes,      // output array
                           inT32 maxmodes);   // size of modes
void correct_row_xheight(TO_ROW *row,    // row to fix
                         float xheight,  // average values
                         float ascrise,
                         float descdrop);
void separate_underlines(TO_BLOCK *block,  // block to do
                         float gradient,   // skew angle
                         FCOORD rotation,  // inverse landscape
                         BOOL8 testing_on);  // correct orientation
void pre_associate_blobs( ICOORD page_tr,   // top right
                         TO_BLOCK *block,  // block to do
                         FCOORD rotation,  // inverse landscape
                         BOOL8 testing_on);  // correct orientation
void fit_parallel_rows(TO_BLOCK *block,   // block to do
                       float gradient,    // gradient to fit
                       FCOORD rotation,   // for drawing
                       inT32 block_edge,  // edge of block
                       BOOL8 testing_on);  // correct orientation
void fit_parallel_lms(float gradient,  // forced gradient
                      TO_ROW *row);      // row to fit
void make_baseline_spline(TO_ROW *row,     // row to fit
                          TO_BLOCK *block);  // block it came from
BOOL8 segment_baseline (         //split baseline
TO_ROW * row,                    //row to fit
TO_BLOCK * block,                //block it came from
inT32 & segments,                //no fo segments
inT32 xstarts[]                  //coords of segments
);
double *linear_spline_baseline ( //split baseline
TO_ROW * row,                    //row to fit
TO_BLOCK * block,                //block it came from
inT32 & segments,                //no fo segments
inT32 xstarts[]                  //coords of segments
);
void assign_blobs_to_rows(                      //find lines
                          TO_BLOCK *block,      //block to do
                          float *gradient,      //block skew
                          int pass,             //identification
                          BOOL8 reject_misses,  //chuck big ones out
                          BOOL8 make_new_rows,  //add rows for unmatched
                          BOOL8 drawing_skew    //draw smoothed skew
                         );
                                 //find best row
OVERLAP_STATE most_overlapping_row(TO_ROW_IT *row_it,  //iterator
                                   TO_ROW *&best_row,  //output row
                                   float top,          //top of blob
                                   float bottom,       //bottom of blob
                                   float rowsize,      //max row size
                                   BOOL8 testing_blob  //test stuff
                                  );
int blob_x_order(                    //sort function
                 const void *item1,  //items to compare
                 const void *item2);
int row_y_order(                    //sort function
                const void *item1,  //items to compare
                const void *item2);
int row_spacing_order(                    //sort function
                      const void *item1,  //items to compare
                      const void *item2);

void mark_repeated_chars(TO_ROW *row);
#endif
