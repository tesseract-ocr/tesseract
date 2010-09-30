/**********************************************************************
 * File:        makerow.cpp  (Formerly makerows.c)
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

#include "mfcpch.h"
#ifdef __UNIX__
#include          <assert.h>
#endif
#include          "stderr.h"
#include          "blobbox.h"
#include          "lmedsq.h"
#include          "statistc.h"
#include          "drawtord.h"
#include          "blkocc.h"
#include          "sortflts.h"
#include          "oldbasel.h"
#include          "tordmain.h"
#include          "underlin.h"
#include          "makerow.h"
#include          "tprintf.h"
#include          "tesseractclass.h"
#include          "tovars.h"

// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif

BOOL_VAR(textord_heavy_nr, FALSE, "Vigorously remove noise");
BOOL_VAR(textord_show_initial_rows, FALSE, "Display row accumulation");
BOOL_VAR(textord_show_parallel_rows, FALSE, "Display page correlated rows");
BOOL_VAR(textord_show_expanded_rows, FALSE, "Display rows after expanding");
BOOL_VAR(textord_show_final_rows, FALSE, "Display rows after final fitting");
BOOL_VAR(textord_show_final_blobs, FALSE, "Display blob bounds after pre-ass");
BOOL_VAR(textord_test_landscape, FALSE, "Tests refer to land/port");
BOOL_VAR(textord_parallel_baselines, TRUE, "Force parallel baselines");
BOOL_VAR(textord_straight_baselines, FALSE, "Force straight baselines");
BOOL_VAR(textord_quadratic_baselines, FALSE, "Use quadratic splines");
BOOL_VAR(textord_old_baselines, TRUE, "Use old baseline algorithm");
BOOL_VAR(textord_old_xheight, FALSE, "Use old xheight algorithm");
BOOL_VAR(textord_fix_xheight_bug, TRUE, "Use spline baseline");
BOOL_VAR(textord_fix_makerow_bug, TRUE, "Prevent multiple baselines");
BOOL_VAR(textord_cblob_blockocc, TRUE, "Use new projection for underlines");
BOOL_VAR(textord_debug_xheights, FALSE, "Test xheight algorithms");
BOOL_VAR(textord_biased_skewcalc, TRUE, "Bias skew estimates with line length");
BOOL_VAR(textord_interpolating_skew, TRUE, "Interpolate across gaps");
INT_VAR(textord_skewsmooth_offset, 2, "For smooth factor");
INT_VAR(textord_skewsmooth_offset2, 1, "For smooth factor");
INT_VAR(textord_test_x, -1, "coord of test pt");
INT_VAR(textord_test_y, -1, "coord of test pt");
INT_VAR(textord_min_blobs_in_row, 4, "Min blobs before gradient counted");
INT_VAR(textord_spline_minblobs, 8, "Min blobs in each spline segment");
INT_VAR(textord_spline_medianwin, 6, "Size of window for spline segmentation");
INT_VAR(textord_max_blob_overlaps, 4,
        "Max number of blobs a big blob can overlap");
INT_VAR(textord_min_xheight, 10, "Min credible pixel xheight");
double_VAR(textord_spline_shift_fraction, 0.02,
           "Fraction of line spacing for quad");
double_VAR(textord_spline_outlier_fraction, 0.1,
           "Fraction of line spacing for outlier");
double_VAR(textord_skew_ile, 0.5, "Ile of gradients for page skew");
double_VAR(textord_skew_lag, 0.01, "Lag for skew on row accumulation");
double_VAR(textord_linespace_iqrlimit, 0.2, "Max iqr/median for linespace");
double_VAR(textord_width_limit, 8, "Max width of blobs to make rows");
double_VAR(textord_chop_width, 1.5, "Max width before chopping");
double_VAR(textord_expansion_factor, 1.0,
           "Factor to expand rows by in expand_rows");
double_VAR(textord_overlap_x, 0.5, "Fraction of linespace for good overlap");
double_VAR(textord_merge_desc, 0.25, "Fraction of linespace for desc drop");
double_VAR(textord_merge_x, 0.5, "Fraction of linespace for x height");
double_VAR(textord_merge_asc, 0.25, "Fraction of linespace for asc height");
double_VAR(textord_minxh, 0.25, "fraction of linesize for min xheight");
double_VAR(textord_min_linesize, 1.25, "* blob height for initial linesize");
double_VAR(textord_excess_blobsize, 1.3,
           "New row made if blob makes row this big");
double_VAR(textord_occupancy_threshold, 0.4, "Fraction of neighbourhood");
double_VAR(textord_underline_width, 2.0, "Multiple of line_size for underline");
double_VAR(textord_min_blob_height_fraction, 0.75,
           "Min blob height/top to include blob top into xheight stats");
double_VAR(textord_xheight_mode_fraction, 0.4,
           "Min pile height to make xheight");
double_VAR(textord_ascheight_mode_fraction, 0.08,
           "Min pile height to make ascheight");
double_VAR(textord_descheight_mode_fraction, 0.08,
           "Min pile height to make descheight");
double_VAR(textord_ascx_ratio_min, 1.3, "Min cap/xheight");
double_VAR(textord_ascx_ratio_max, 1.8, "Max cap/xheight");
double_VAR(textord_descx_ratio_min, 0.25, "Min desc/xheight");
double_VAR(textord_descx_ratio_max, 0.6, "Max desc/xheight");
double_VAR(textord_xheight_error_margin, 0.1, "Accepted variation");

#define MAX_HEIGHT_MODES  12

/**
 * @name make_single_row
 *
 * Arrange the blobs into a single row.
 */
float make_single_row(ICOORD page_tr, TO_BLOCK* block, TO_BLOCK_LIST* blocks,
                      tesseract::Tesseract* tess) {
  BLOBNBOX_IT blob_it = &block->blobs;
  TO_ROW_IT row_it = block->get_rows();

  // Include all the small blobs and large blobs.
  blob_it.add_list_after(&block->small_blobs);
  blob_it.add_list_after(&block->noise_blobs);
  blob_it.add_list_after(&block->large_blobs);
  blob_it.sort(blob_x_order);
  blob_it.move_to_first();
  TO_ROW* row = NULL;
  // Add all the blobs to a single TO_ROW.
  for (blob_it.mark_cycle_pt(); !blob_it.cycled_list(); blob_it.forward()) {
    BLOBNBOX* blob = blob_it.extract();
    int top = blob->bounding_box().top();
    int bottom = blob->bounding_box().bottom();
    if (row == NULL) {
      row = new TO_ROW(blob, top, bottom, block->line_size);
      row_it.add_before_then_move(row);
    } else {
      row->add_blob(blob, top, bottom, block->line_size);
    }
  }
  // Fit an LMS line to the row.
  for (row_it.mark_cycle_pt(); !row_it.cycled_list(); row_it.forward())
    fit_lms_line(row_it.data());
  float gradient;
  float fit_error;
  // Compute the skew based on the fitted line.
  compute_page_skew(blocks, gradient, fit_error);
  FCOORD rotation(1.0f, 0.0f);
  // Associate i dots and other diacriticals with the appropriate blobs.
  pre_associate_blobs(page_tr, block, rotation, false);
  int block_edge = block->block->bounding_box().left();
  fit_parallel_rows(block, gradient, rotation, block_edge, false);
  // Make the curved baselines and setup some key block members.
  make_spline_rows(block, gradient, rotation, block_edge, false, tess);
  return gradient;
}

/**
 * @name make_rows
 *
 * Arrange the blobs into rows.
 */
float make_rows(                             //make rows
                ICOORD page_tr,              //top right
                BLOCK_LIST *blocks,          //block list
                TO_BLOCK_LIST *land_blocks,  //rotated for landscape
                TO_BLOCK_LIST *port_blocks,  //output list
                tesseract::Tesseract* tess
               ) {
  float port_m;                  //global skew
  float port_err;                //global noise
  //   float                                     land_m;                                         //global skew
  //      float                                   land_err;                                       //global noise
  TO_BLOCK_IT block_it;          //iterator

  //don't do landscape for now
  //      block_it.set_to_list(land_blocks);
  //      for (block_it.mark_cycle_pt();!block_it.cycled_list();block_it.forward())
  //              make_initial_textrows(page_tr,block_it.data(),FCOORD(0,-1),
  //                      (BOOL8)textord_test_landscape);
  block_it.set_to_list (port_blocks);
  for (block_it.mark_cycle_pt (); !block_it.cycled_list ();
    block_it.forward ())
  make_initial_textrows (page_tr, block_it.data (), FCOORD (1.0f, 0.0f),
      !(BOOL8) textord_test_landscape);
                                 //compute globally
  compute_page_skew(port_blocks, port_m, port_err);
  //      compute_page_skew(land_blocks,land_m,land_err);  // compute globally
  //      tprintf("Portrait skew gradient=%g, error=%g.\n",
  //              port_m,port_err);
  //      tprintf("Landscape skew gradient=%g, error=%g.\n",
  //              land_m,land_err);
  block_it.set_to_list (port_blocks);
  for (block_it.mark_cycle_pt(); !block_it.cycled_list(); block_it.forward()) {
    cleanup_rows(page_tr, block_it.data(), port_m, FCOORD(1.0f, 0.0f),
                 block_it.data()->block->bounding_box().left(),
                 !(BOOL8)textord_test_landscape, tess);
  }
  block_it.set_to_list (land_blocks);
  //      for (block_it.mark_cycle_pt();!block_it.cycled_list();block_it.forward())
  //      {
  //              cleanup_rows(page_tr,block_it.data(),land_m,FCOORD(0,-1),
  //                      -block_it.data()->block->bounding_box().top(),
  //                      (BOOL8)textord_test_landscape);
  //      }
  return port_m;                 //global skew
}


/**
 * @name make_initial_textrows
 *
 * Arrange the good blobs into rows of text.
 */
void make_initial_textrows(                  //find lines
                           ICOORD page_tr,
                           TO_BLOCK *block,  //block to do
                           FCOORD rotation,  //for drawing
                           BOOL8 testing_on  //correct orientation
                          ) {
  TO_ROW_IT row_it = block->get_rows ();

#ifndef GRAPHICS_DISABLED
  ScrollView::Color colour;                 //of row

  if (textord_show_initial_rows && testing_on) {
    if (to_win == NULL)
      create_to_win(page_tr);
  }
#endif
                                 //guess skew
  assign_blobs_to_rows (block, NULL, 0, TRUE, TRUE, textord_show_initial_rows && testing_on);
  row_it.move_to_first ();
  for (row_it.mark_cycle_pt (); !row_it.cycled_list (); row_it.forward ())
    fit_lms_line (row_it.data ());
#ifndef GRAPHICS_DISABLED
  if (textord_show_initial_rows && testing_on) {
    colour = ScrollView::RED;
    for (row_it.mark_cycle_pt (); !row_it.cycled_list (); row_it.forward ()) {
      plot_to_row (row_it.data (), colour, rotation);
      colour = (ScrollView::Color) (colour + 1);
      if (colour > ScrollView::MAGENTA)
        colour = ScrollView::RED;
    }
  }
#endif
}


/**
 * @name fit_lms_line
 *
 * Fit an LMS line to a row.
 */
void fit_lms_line(             //sort function
                  TO_ROW *row  //row to fit
                 ) {
  float m, c;                    //fitted line
  TBOX box;                       //blob box
  LMS lms (row->blob_list ()->length ());
                                 //blobs
  BLOBNBOX_IT blob_it = row->blob_list ();

  for (blob_it.mark_cycle_pt (); !blob_it.cycled_list (); blob_it.forward ()) {
    box = blob_it.data ()->bounding_box ();
    lms.add (FCOORD ((box.left () + box.right ()) / 2.0, box.bottom ()));
  }
  lms.fit (m, c);
  row->set_line (m, c, lms.error ());
}


/**
 * @name compute_page_skew
 *
 * Compute the skew over a full page by averaging the gradients over
 * all the lines. Get the error of the same row.
 */
void compute_page_skew(                        //get average gradient
                       TO_BLOCK_LIST *blocks,  //list of blocks
                       float &page_m,          //average gradient
                       float &page_err         //average error
                      ) {
  inT32 row_count;               //total rows
  inT32 blob_count;              //total_blobs
  inT32 row_err;                 //integer error
  float *gradients;              //of rows
  float *errors;                 //of rows
  inT32 row_index;               //of total
  TO_ROW *row;                   //current row
  TO_BLOCK_IT block_it = blocks; //iterator
  TO_ROW_IT row_it;

  row_count = 0;
  blob_count = 0;
  for (block_it.mark_cycle_pt (); !block_it.cycled_list ();
       block_it.forward ()) {
    row_count += block_it.data ()->get_rows ()->length ();
    //count up rows
    row_it.set_to_list (block_it.data ()->get_rows ());
    for (row_it.mark_cycle_pt (); !row_it.cycled_list (); row_it.forward ())
      blob_count += row_it.data ()->blob_list ()->length ();
  }
  if (row_count == 0) {
    page_m = 0.0f;
    page_err = 0.0f;
    return;
  }
  gradients = (float *) alloc_mem (blob_count * sizeof (float));
  //get mem
  errors = (float *) alloc_mem (blob_count * sizeof (float));
  if (gradients == NULL || errors == NULL)
    MEMORY_OUT.error ("compute_page_skew", ABORT, NULL);

  row_index = 0;
  for (block_it.mark_cycle_pt (); !block_it.cycled_list ();
       block_it.forward ()) {
    row_it.set_to_list (block_it.data ()->get_rows ());
    for (row_it.mark_cycle_pt (); !row_it.cycled_list (); row_it.forward ()) {
      row = row_it.data ();
      blob_count = row->blob_list ()->length ();
      row_err = (inT32) ceil (row->line_error ());
      if (row_err <= 0)
        row_err = 1;
      if (textord_biased_skewcalc) {
        blob_count /= row_err;
        for (blob_count /= row_err; blob_count > 0; blob_count--) {
          gradients[row_index] = row->line_m ();
          errors[row_index] = row->line_error ();
          row_index++;
        }
      }
      else if (blob_count >= textord_min_blobs_in_row) {
                                 //get gradient
        gradients[row_index] = row->line_m ();
        errors[row_index] = row->line_error ();
        row_index++;
      }
    }
  }
  if (row_index == 0) {
                                 //desperate
    for (block_it.mark_cycle_pt (); !block_it.cycled_list ();
         block_it.forward ()) {
      row_it.set_to_list (block_it.data ()->get_rows ());
      for (row_it.mark_cycle_pt (); !row_it.cycled_list ();
           row_it.forward ()) {
        row = row_it.data ();
        gradients[row_index] = row->line_m ();
        errors[row_index] = row->line_error ();
        row_index++;
      }
    }
  }
  row_count = row_index;
  row_index = choose_nth_item ((inT32) (row_count * textord_skew_ile),
    gradients, row_count);
  page_m = gradients[row_index];
  row_index = choose_nth_item ((inT32) (row_count * textord_skew_ile),
    errors, row_count);
  page_err = errors[row_index];
  free_mem(gradients);
  free_mem(errors);
}

const double kNoiseSize = 0.5;  // Fraction of xheight.
const int kMinSize = 8;  // Min pixels to be xheight.

/**
 * Return true if the dot looks like it is part of the i.
 * Doesn't work for any other diacritical.
 */
static bool dot_of_i(BLOBNBOX* dot, BLOBNBOX* i, TO_ROW* row) {
  const TBOX& ibox = i->bounding_box();
  const TBOX& dotbox = dot->bounding_box();

  // Must overlap horizontally by enough and be high enough.
  int overlap = MIN(dotbox.right(), ibox.right()) -
                MAX(dotbox.left(), ibox.left());
  if (ibox.height() <= 2 * dotbox.height() ||
      (overlap * 2 < ibox.width() && overlap < dotbox.width()))
    return false;

  // If the i is tall and thin then it is good.
  if (ibox.height() > ibox.width() * 2)
    return true;  // The i or ! must be tall and thin.

  // It might still be tall and thin, but it might be joined to something.
  // So search the outline for a piece of large height close to the edges
  // of the dot.
  const double kHeightFraction = 0.6;
  double target_height = MIN(dotbox.bottom(), ibox.top());
  target_height -= row->line_m()*dotbox.left() + row->line_c();
  target_height *= kHeightFraction;
  int left_min = dotbox.left() - dotbox.width();
  int middle = (dotbox.left() + dotbox.right())/2;
  int right_max = dotbox.right() + dotbox.width();
  int left_miny = 0;
  int left_maxy = 0;
  int right_miny = 0;
  int right_maxy = 0;
  bool found_left = false;
  bool found_right = false;
  bool in_left = false;
  bool in_right = false;
  C_BLOB* blob = i->cblob();
  C_OUTLINE_IT o_it = blob->out_list();
  for (o_it.mark_cycle_pt(); !o_it.cycled_list(); o_it.forward()) {
    C_OUTLINE* outline = o_it.data();
    int length = outline->pathlength();
    ICOORD pos = outline->start_pos();
    for (int step = 0; step < length; pos += outline->step(step++)) {
      int x = pos.x();
      int y = pos.y();
      if (x >= left_min && x < middle && !found_left) {
        // We are in the left part so find min and max y.
        if (in_left) {
          if (y > left_maxy) left_maxy = y;
          if (y < left_miny) left_miny = y;
        } else {
          left_maxy = left_miny = y;
          in_left = true;
        }
      } else if (in_left) {
        // We just left the left so look for size.
        if (left_maxy - left_miny > target_height) {
          if (found_right)
            return true;
          found_left = true;
        }
        in_left = false;
      }
      if (x <= right_max && x > middle && !found_right) {
        // We are in the right part so find min and max y.
        if (in_right) {
          if (y > right_maxy) right_maxy = y;
          if (y < right_miny) right_miny = y;
        } else {
          right_maxy = right_miny = y;
          in_right = true;
        }
      } else if (in_right) {
        // We just left the right so look for size.
        if (right_maxy - right_miny > target_height) {
          if (found_left)
            return true;
          found_right = true;
        }
        in_right = false;
      }
    }
  }
  return false;
}

static void vigorous_noise_removal(TO_BLOCK* block) {
  TO_ROW_IT row_it = block->get_rows ();
  for (row_it.mark_cycle_pt (); !row_it.cycled_list (); row_it.forward ()) {
    TO_ROW* row = row_it.data();
    BLOBNBOX_IT b_it = row->blob_list();
    // Estimate the xheight on the row.
    int max_height = 0;
    for (b_it.mark_cycle_pt(); !b_it.cycled_list(); b_it.forward()) {
      BLOBNBOX* blob = b_it.data();
      if (blob->bounding_box().height() > max_height)
        max_height = blob->bounding_box().height();
    }
    STATS hstats(0, max_height + 1);
    for (b_it.mark_cycle_pt(); !b_it.cycled_list(); b_it.forward()) {
      BLOBNBOX* blob = b_it.data();
      int height = blob->bounding_box().height();
      if (height >= kMinSize)
        hstats.add(blob->bounding_box().height(), 1);
    }
    float xheight = hstats.median();
    // Delete small objects.
    BLOBNBOX* prev = NULL;
    for (b_it.mark_cycle_pt(); !b_it.cycled_list(); b_it.forward()) {
      BLOBNBOX* blob = b_it.data();
      const TBOX& box = blob->bounding_box();
      if (box.height() < kNoiseSize * xheight) {
        // Small so delete unless it looks like an i dot.
        if (prev != NULL) {
          if (dot_of_i(blob, prev, row))
            continue;  // Looks OK.
        }
        if (!b_it.at_last()) {
          BLOBNBOX* next = b_it.data_relative(1);
          if (dot_of_i(blob, next, row))
            continue;  // Looks OK.
        }
        // It might be noise so get rid of it.
        if (blob->blob() != NULL)
          delete blob->blob();
        if (blob->cblob() != NULL)
          delete blob->cblob();
        delete b_it.extract();
      } else {
        prev = blob;
      }
    }
  }
}

/**
 * cleanup_rows
 *
 * Remove overlapping rows and fit all the blobs to what's left.
 */
void cleanup_rows(                   //find lines
                  ICOORD page_tr,    //top right
                  TO_BLOCK *block,   //block to do
                  float gradient,    //gradient to fit
                  FCOORD rotation,   //for drawing
                  inT32 block_edge,  //edge of block
                  BOOL8 testing_on,  //correct orientation
                  tesseract::Tesseract* tess
                 ) {
                                 //iterators
  BLOBNBOX_IT blob_it = &block->blobs;
  TO_ROW_IT row_it = block->get_rows ();

#ifndef GRAPHICS_DISABLED
  if (textord_show_parallel_rows && testing_on) {
    if (to_win == NULL)
      create_to_win(page_tr);
  }
#endif
                                 //get row coords
  fit_parallel_rows(block,
                    gradient,
                    rotation,
                    block_edge,
                    textord_show_parallel_rows &&testing_on);
  delete_non_dropout_rows(block,
                          gradient,
                          rotation,
                          block_edge,
                          textord_show_parallel_rows &&testing_on);
  expand_rows(page_tr, block, gradient, rotation, block_edge, testing_on);
  blob_it.set_to_list (&block->blobs);
  row_it.set_to_list (block->get_rows ());
  for (row_it.mark_cycle_pt (); !row_it.cycled_list (); row_it.forward ())
    blob_it.add_list_after (row_it.data ()->blob_list ());
  //give blobs back
  assign_blobs_to_rows (block, &gradient, 1, FALSE, FALSE, FALSE);
  //now new rows must be genuine
  blob_it.set_to_list (&block->blobs);
  blob_it.add_list_after (&block->large_blobs);
  assign_blobs_to_rows (block, &gradient, 2, TRUE, TRUE, FALSE);
  //safe to use big ones now
  blob_it.set_to_list (&block->blobs);
                                 //throw all blobs in
  blob_it.add_list_after (&block->noise_blobs);
  blob_it.add_list_after (&block->small_blobs);
  assign_blobs_to_rows (block, &gradient, 3, FALSE, FALSE, FALSE);
  //no rows for noise
  row_it.set_to_list (block->get_rows ());
  for (row_it.mark_cycle_pt (); !row_it.cycled_list (); row_it.forward ())
    row_it.data ()->blob_list ()->sort (blob_x_order);
  fit_parallel_rows(block, gradient, rotation, block_edge, FALSE);
  if (textord_heavy_nr) {
    vigorous_noise_removal(block);
  }
  separate_underlines(block, gradient, rotation, testing_on);
  pre_associate_blobs(page_tr, block, rotation, testing_on);

#ifndef GRAPHICS_DISABLED
  if (textord_show_final_rows && testing_on) {
    if (to_win == NULL)
      create_to_win(page_tr);
  }
#endif

  fit_parallel_rows(block, gradient, rotation, block_edge, FALSE);
  //              textord_show_final_rows && testing_on);
  make_spline_rows(block,
                   gradient,
                   rotation,
                   block_edge,
                   textord_show_final_rows && testing_on,
                   tess);
  // We only want to call compute_block_xheight() if
  // both textord_old_xheight and textord_old_baselines are false.
  // No need to call compute_block_xheight() if textord_old_baselines
  // is true, since all appropriate xheight computation functions
  // would be called from make_old_baselines().
  // Note: it can not be the case that textord_old_baselines is
  // false, and textord_old_xheight is true.
  if (!textord_old_xheight && !textord_old_baselines)
    compute_block_xheight(block, gradient, tess);
  if (textord_restore_underlines)  // fix underlines
    restore_underlined_blobs(block);
#ifndef GRAPHICS_DISABLED
  if (textord_show_final_rows && testing_on) {
    plot_blob_list (to_win, &block->blobs,
                    ScrollView::MAGENTA, ScrollView::WHITE);
    //show discarded blobs
    plot_blob_list (to_win, &block->underlines,
                    ScrollView::YELLOW, ScrollView::CORAL);
  }
  if (textord_show_final_rows && testing_on && block->blobs.length () > 0)
    tprintf ("%d blobs discarded as noise\n", block->blobs.length ());
  if (textord_show_final_rows && testing_on) {
    draw_meanlines(block, gradient, block_edge, ScrollView::WHITE, rotation);
  }
#endif
}


/**
 * delete_non_dropout_rows
 *
 * Compute the linespacing and offset.
 */
void delete_non_dropout_rows(                   //find lines
                             TO_BLOCK *block,   //block to do
                             float gradient,    //global skew
                             FCOORD rotation,   //deskew vector
                             inT32 block_edge,  //left edge
                             BOOL8 testing_on   //correct orientation
                            ) {
  TBOX block_box;                 //deskewed block
  inT32 *deltas;                 //change in occupation
  inT32 *occupation;             //of pixel coords
  inT32 max_y;                   //in block
  inT32 min_y;
  inT32 line_index;              //of scan line
  inT32 line_count;              //no of scan lines
  inT32 distance;                //to drop-out
  inT32 xleft;                   //of block
  inT32 ybottom;                 //of block
  TO_ROW *row;                   //current row
  TO_ROW_IT row_it = block->get_rows ();
  BLOBNBOX_IT blob_it = &block->blobs;

  if (row_it.length () == 0)
    return;                      //empty block
  block_box = deskew_block_coords (block, gradient);
  xleft = block->block->bounding_box ().left ();
  ybottom = block->block->bounding_box ().bottom ();
  min_y = block_box.bottom () - 1;
  max_y = block_box.top () + 1;
  for (row_it.mark_cycle_pt (); !row_it.cycled_list (); row_it.forward ()) {
    line_index = (inT32) floor (row_it.data ()->intercept ());
    if (line_index <= min_y)
      min_y = line_index - 1;
    if (line_index >= max_y)
      max_y = line_index + 1;
  }
  line_count = max_y - min_y + 1;
  if (line_count <= 0)
    return;                      //empty block
  deltas = (inT32 *) alloc_mem (line_count * sizeof (inT32));
  occupation = (inT32 *) alloc_mem (line_count * sizeof (inT32));
  if (deltas == NULL || occupation == NULL)
    MEMORY_OUT.error ("compute_line_spacing", ABORT, NULL);

  compute_line_occupation(block, gradient, min_y, max_y, occupation, deltas);
  compute_occupation_threshold ((inT32)
    ceil (block->line_spacing *
    (textord_merge_desc +
    textord_merge_asc)),
    (inT32) ceil (block->line_spacing *
    (textord_merge_x +
    textord_merge_asc)),
    max_y - min_y + 1, occupation, deltas);
#ifndef GRAPHICS_DISABLED
  if (testing_on) {
    draw_occupation(xleft, ybottom, min_y, max_y, occupation, deltas);
  }
#endif
  compute_dropout_distances(occupation, deltas, line_count);
  for (row_it.mark_cycle_pt (); !row_it.cycled_list (); row_it.forward ()) {
    row = row_it.data ();
    line_index = (inT32) floor (row->intercept ());
    distance = deltas[line_index - min_y];
    if (find_best_dropout_row (row, distance, block->line_spacing / 2,
    line_index, &row_it, testing_on)) {
#ifndef GRAPHICS_DISABLED
      if (testing_on)
        plot_parallel_row(row, gradient, block_edge,
                          ScrollView::WHITE, rotation);
#endif
      blob_it.add_list_after (row_it.data ()->blob_list ());
      delete row_it.extract ();  //too far away
    }
  }
  for (row_it.mark_cycle_pt (); !row_it.cycled_list (); row_it.forward ()) {
    blob_it.add_list_after (row_it.data ()->blob_list ());
  }

  free_mem(deltas);
  free_mem(occupation);
}


/**
 * @name find_best_dropout_row
 *
 * Delete this row if it has a neighbour with better dropout characteristics.
 * TRUE is returned if the row should be deleted.
 */
BOOL8 find_best_dropout_row(                    //find neighbours
                            TO_ROW *row,        //row to test
                            inT32 distance,     //dropout dist
                            float dist_limit,   //threshold distance
                            inT32 line_index,   //index of row
                            TO_ROW_IT *row_it,  //current position
                            BOOL8 testing_on    //correct orientation
                           ) {
  inT32 next_index;              //of neigbouring row
  inT32 row_offset;              //from current row
  inT32 abs_dist;                //absolute distance
  inT8 row_inc;                  //increment to row_index
  TO_ROW *next_row;              //nextious row

  if (testing_on)
    tprintf ("Row at %g(%g), dropout dist=%d,",
      row->intercept (), row->parallel_c (), distance);
  if (distance < 0) {
    row_inc = 1;
    abs_dist = -distance;
  }
  else {
    row_inc = -1;
    abs_dist = distance;
  }
  if (abs_dist > dist_limit) {
    if (testing_on) {
      tprintf (" too far - deleting\n");
    }
    return TRUE;
  }
  if ((distance < 0 && !row_it->at_last ())
  || (distance >= 0 && !row_it->at_first ())) {
    row_offset = row_inc;
    do {
      next_row = row_it->data_relative (row_offset);
      next_index = (inT32) floor (next_row->intercept ());
      if ((distance < 0
        && next_index < line_index
        && next_index > line_index + distance + distance)
        || (distance >= 0
        && next_index > line_index
      && next_index < line_index + distance + distance)) {
        if (testing_on) {
          tprintf (" nearer neighbour (%d) at %g\n",
            line_index + distance - next_index,
            next_row->intercept ());
        }
        return TRUE;             //other is nearer
      }
      else if (next_index == line_index
      || next_index == line_index + distance + distance) {
        if (row->believability () <= next_row->believability ()) {
          if (testing_on) {
            tprintf (" equal but more believable at %g (%g/%g)\n",
              next_row->intercept (),
              row->believability (),
              next_row->believability ());
          }
          return TRUE;           //other is more believable
        }
      }
      row_offset += row_inc;
    }
    while ((next_index == line_index
      || next_index == line_index + distance + distance)
      && row_offset < row_it->length ());
    if (testing_on)
      tprintf (" keeping\n");
  }
  return FALSE;
}


/**
 * @name deskew_block_coords
 *
 * Compute the bounding box of all the blobs in the block
 * if they were deskewed without actually doing it.
 */
TBOX deskew_block_coords(                  //block box
                        TO_BLOCK *block,  //block to do
                        float gradient    //global skew
                       ) {
  TBOX result;                    //block bounds
  TBOX blob_box;                  //of block
  FCOORD rotation;               //deskew vector
  float length;                  //of gradient vector
  TO_ROW_IT row_it = block->get_rows ();
  TO_ROW *row;                   //current row
  BLOBNBOX *blob;                //current blob
  BLOBNBOX_IT blob_it;           //iterator

  length = sqrt (gradient * gradient + 1);
  rotation = FCOORD (1 / length, -gradient / length);
  for (row_it.mark_cycle_pt (); !row_it.cycled_list (); row_it.forward ()) {
    row = row_it.data ();
    blob_it.set_to_list (row->blob_list ());
    for (blob_it.mark_cycle_pt (); !blob_it.cycled_list ();
    blob_it.forward ()) {
      blob = blob_it.data ();
      blob_box = blob->bounding_box ();
      blob_box.rotate (rotation);//de-skew it
      result += blob_box;
    }
  }
  return result;
}


/**
 * @name compute_line_occupation
 *
 * Compute the pixel projection back on the y axis given the global
 * skew. Also compute the 1st derivative.
 */
void compute_line_occupation(                    //project blobs
                             TO_BLOCK *block,    //block to do
                             float gradient,     //global skew
                             inT32 min_y,        //min coord in block
                             inT32 max_y,        //in block
                             inT32 *occupation,  //output projection
                             inT32 *deltas       //derivative
                            ) {
  inT32 line_count;              //maxy-miny+1
  inT32 line_index;              //of scan line
  int index;                     //array index for daft compilers
  float top, bottom;             //coords of blob
  inT32 width;                   //of blob
  TO_ROW *row;                   //current row
  TO_ROW_IT row_it = block->get_rows ();
  BLOBNBOX *blob;                //current blob
  BLOBNBOX_IT blob_it;           //iterator
  float length;                  //of skew vector
  TBOX blob_box;                  //bounding box
  FCOORD rotation;               //inverse of skew

  line_count = max_y - min_y + 1;
  length = sqrt (gradient * gradient + 1);
  rotation = FCOORD (1 / length, -gradient / length);
  for (line_index = 0; line_index < line_count; line_index++)
    deltas[line_index] = 0;
  for (row_it.mark_cycle_pt (); !row_it.cycled_list (); row_it.forward ()) {
    row = row_it.data ();
    blob_it.set_to_list (row->blob_list ());
    for (blob_it.mark_cycle_pt (); !blob_it.cycled_list ();
    blob_it.forward ()) {
      blob = blob_it.data ();
      blob_box = blob->bounding_box ();
      blob_box.rotate (rotation);//de-skew it
      top = blob_box.top ();
      bottom = blob_box.bottom ();
      width =
        (inT32) floor ((FLOAT32) (blob_box.right () - blob_box.left ()));
      if ((inT32) floor (bottom) < min_y
        || (inT32) floor (bottom) - min_y >= line_count)
        fprintf (stderr,
          "Bad y coord of bottom, " INT32FORMAT "(" INT32FORMAT ","
          INT32FORMAT ")\n", (inT32) floor (bottom), min_y, max_y);
                                 //count transitions
      index = (inT32) floor (bottom) - min_y;
      deltas[index] += width;
      if ((inT32) floor (top) < min_y
        || (inT32) floor (top) - min_y >= line_count)
        fprintf (stderr,
          "Bad y coord of top, " INT32FORMAT "(" INT32FORMAT ","
          INT32FORMAT ")\n", (inT32) floor (top), min_y, max_y);
      index = (inT32) floor (top) - min_y;
      deltas[index] -= width;
    }
  }
  occupation[0] = deltas[0];
  for (line_index = 1; line_index < line_count; line_index++)
    occupation[line_index] = occupation[line_index - 1] + deltas[line_index];
}


/**
 * compute_occupation_threshold
 *
 * Compute thresholds for textline or not for the occupation array.
 */
void compute_occupation_threshold(                    //project blobs
                                  inT32 low_window,   //below result point
                                  inT32 high_window,  //above result point
                                  inT32 line_count,   //array sizes
                                  inT32 *occupation,  //input projection
                                  inT32 *thresholds   //output thresholds
                                 ) {
  inT32 line_index;              //of thresholds line
  inT32 low_index;               //in occupation
  inT32 high_index;              //in occupation
  inT32 sum;                     //current average
  inT32 divisor;                 //to get thresholds
  inT32 min_index;               //of min occ
  inT32 min_occ;                 //min in locality
  inT32 test_index;              //for finding min

  divisor =
    (inT32) ceil ((low_window + high_window) / textord_occupancy_threshold);
  if (low_window + high_window < line_count) {
    for (sum = 0, high_index = 0; high_index < low_window; high_index++)
      sum += occupation[high_index];
    for (low_index = 0; low_index < high_window; low_index++, high_index++)
      sum += occupation[high_index];
    min_occ = occupation[0];
    min_index = 0;
    for (test_index = 1; test_index < high_index; test_index++) {
      if (occupation[test_index] <= min_occ) {
        min_occ = occupation[test_index];
        min_index = test_index;  //find min in region
      }
    }
    for (line_index = 0; line_index < low_window; line_index++)
      thresholds[line_index] = (sum - min_occ) / divisor + min_occ;
    //same out to end
    for (low_index = 0; high_index < line_count; low_index++, high_index++) {
      sum -= occupation[low_index];
      sum += occupation[high_index];
      if (occupation[high_index] <= min_occ) {
                                 //find min in region
        min_occ = occupation[high_index];
        min_index = high_index;
      }
                                 //lost min from region
      if (min_index <= low_index) {
        min_occ = occupation[low_index + 1];
        min_index = low_index + 1;
        for (test_index = low_index + 2; test_index <= high_index;
        test_index++) {
          if (occupation[test_index] <= min_occ) {
            min_occ = occupation[test_index];
                                 //find min in region
            min_index = test_index;
          }
        }
      }
      thresholds[line_index++] = (sum - min_occ) / divisor + min_occ;
    }
  }
  else {
    min_occ = occupation[0];
    min_index = 0;
    for (sum = 0, low_index = 0; low_index < line_count; low_index++) {
      if (occupation[low_index] < min_occ) {
        min_occ = occupation[low_index];
        min_index = low_index;
      }
      sum += occupation[low_index];
    }
    line_index = 0;
  }
  for (; line_index < line_count; line_index++)
    thresholds[line_index] = (sum - min_occ) / divisor + min_occ;
  //same out to end
}


/**
 * @name compute_dropout_distances
 *
 * Compute the distance from each coordinate to the nearest dropout.
 */
void compute_dropout_distances(                    //project blobs
                               inT32 *occupation,  //input projection
                               inT32 *thresholds,  //output thresholds
                               inT32 line_count    //array sizes
                              ) {
  inT32 line_index;              //of thresholds line
  inT32 distance;                //from prev dropout
  inT32 next_dist;               //to next dropout
  inT32 back_index;              //for back filling
  inT32 prev_threshold;          //before overwrite

  distance = -line_count;
  line_index = 0;
  do {
    do {
      distance--;
      prev_threshold = thresholds[line_index];
                                 //distance from prev
      thresholds[line_index] = distance;
      line_index++;
    }
    while (line_index < line_count
      && (occupation[line_index] < thresholds[line_index]
      || occupation[line_index - 1] >= prev_threshold));
    if (line_index < line_count) {
      back_index = line_index - 1;
      next_dist = 1;
      while (next_dist < -distance && back_index >= 0) {
        thresholds[back_index] = next_dist;
        back_index--;
        next_dist++;
        distance++;
      }
      distance = 1;
    }
  }
  while (line_index < line_count);
}


/**
 * @name expand_rows
 *
 * Expand each row to the least of its allowed size and touching its
 * neighbours. If the expansion would entirely swallow a neighbouring row
 * then do so.
 */
void expand_rows(                   //find lines
                 ICOORD page_tr,    //top right
                 TO_BLOCK *block,   //block to do
                 float gradient,    //gradient to fit
                 FCOORD rotation,   //for drawing
                 inT32 block_edge,  //edge of block
                 BOOL8 testing_on   //correct orientation
                ) {
  BOOL8 swallowed_row;           //eaten a neighbour
  float y_max, y_min;            //new row limits
  float y_bottom, y_top;         //allowed limits
  TO_ROW *test_row;              //next row
  TO_ROW *row;                   //current row
                                 //iterators
  BLOBNBOX_IT blob_it = &block->blobs;
  TO_ROW_IT row_it = block->get_rows ();

#ifndef GRAPHICS_DISABLED
  if (textord_show_expanded_rows && testing_on) {
    if (to_win == NULL)
      create_to_win(page_tr);
  }
#endif

  adjust_row_limits(block);  //shift min,max.
  if (textord_new_initial_xheight) {
    if (block->get_rows ()->length () == 0)
      return;
    compute_row_stats(block, textord_show_expanded_rows &&testing_on);
  }
  assign_blobs_to_rows (block, &gradient, 4, TRUE, FALSE, FALSE);
  //get real membership
  if (block->get_rows ()->length () == 0)
    return;
  fit_parallel_rows(block,
                    gradient,
                    rotation,
                    block_edge,
                    textord_show_expanded_rows &&testing_on);
  if (!textord_new_initial_xheight)
    compute_row_stats(block, textord_show_expanded_rows &&testing_on);
  row_it.move_to_last ();
  do {
    row = row_it.data ();
    y_max = row->max_y ();       //get current limits
    y_min = row->min_y ();
    y_bottom = row->intercept () - block->line_size * textord_expansion_factor *
      textord_merge_desc;
    y_top = row->intercept () + block->line_size * textord_expansion_factor *
      (textord_merge_x + textord_merge_asc);
    if (y_min > y_bottom) {      //expansion allowed
      if (textord_show_expanded_rows && testing_on)
        tprintf("Expanding bottom of row at %f from %f to %f\n",
                row->intercept(), y_min, y_bottom);
                                 //expandable
      swallowed_row = TRUE;
      while (swallowed_row && !row_it.at_last ()) {
        swallowed_row = FALSE;
                                 //get next one
        test_row = row_it.data_relative (1);
                                 //overlaps space
        if (test_row->max_y () > y_bottom) {
          if (test_row->min_y () > y_bottom) {
            if (textord_show_expanded_rows && testing_on)
              tprintf("Eating row below at %f\n", test_row->intercept());
            row_it.forward ();
#ifndef GRAPHICS_DISABLED
            if (textord_show_expanded_rows && testing_on)
              plot_parallel_row(test_row,
                                gradient,
                                block_edge,
                                ScrollView::WHITE,
                                rotation);
#endif
            blob_it.set_to_list (row->blob_list ());
            blob_it.add_list_after (test_row->blob_list ());
                                 //swallow complete row
            delete row_it.extract ();
            row_it.backward ();
            swallowed_row = TRUE;
          }
          else if (test_row->max_y () < y_min) {
                                 //shorter limit
            y_bottom = test_row->max_y ();
            if (textord_show_expanded_rows && testing_on)
              tprintf("Truncating limit to %f due to touching row at %f\n",
                      y_bottom, test_row->intercept());
          }
          else {
            y_bottom = y_min;    //can't expand it
            if (textord_show_expanded_rows && testing_on)
              tprintf("Not expanding limit beyond %f due to touching row at %f\n",
                      y_bottom, test_row->intercept());
          }
        }
      }
      y_min = y_bottom;          //expand it
    }
    if (y_max < y_top) {         //expansion allowed
      if (textord_show_expanded_rows && testing_on)
        tprintf("Expanding top of row at %f from %f to %f\n",
                row->intercept(), y_max, y_top);
      swallowed_row = TRUE;
      while (swallowed_row && !row_it.at_first ()) {
        swallowed_row = FALSE;
                                 //get one above
        test_row = row_it.data_relative (-1);
        if (test_row->min_y () < y_top) {
          if (test_row->max_y () < y_top) {
            if (textord_show_expanded_rows && testing_on)
              tprintf("Eating row above at %f\n", test_row->intercept());
            row_it.backward ();
            blob_it.set_to_list (row->blob_list ());
#ifndef GRAPHICS_DISABLED
            if (textord_show_expanded_rows && testing_on)
              plot_parallel_row(test_row,
                                gradient,
                                block_edge,
                                ScrollView::WHITE,
                                rotation);
#endif
            blob_it.add_list_after (test_row->blob_list ());
                                 //swallow complete row
            delete row_it.extract ();
            row_it.forward ();
            swallowed_row = TRUE;
          }
          else if (test_row->min_y () < y_max) {
                                 //shorter limit
            y_top = test_row->min_y ();
            if (textord_show_expanded_rows && testing_on)
              tprintf("Truncating limit to %f due to touching row at %f\n",
                      y_top, test_row->intercept());
          }
          else {
            y_top = y_max;       //can't expand it
            if (textord_show_expanded_rows && testing_on)
              tprintf("Not expanding limit beyond %f due to touching row at %f\n",
                      y_top, test_row->intercept());
          }
        }
      }
      y_max = y_top;
    }
                                 //new limits
    row->set_limits (y_min, y_max);
    row_it.backward ();
  }
  while (!row_it.at_last ());
}


/**
 * adjust_row_limits
 *
 * Change the limits of rows to suit the default fractions.
 */
void adjust_row_limits(                 //tidy limits
                       TO_BLOCK *block  //block to do
                      ) {
  TO_ROW *row;                   //current row
  float size;                    //size of row
  float ymax;                    //top of row
  float ymin;                    //bottom of row
  TO_ROW_IT row_it = block->get_rows ();

  if (textord_show_expanded_rows)
    tprintf("Adjusting row limits for block(%d,%d)\n",
            block->block->bounding_box().left(),
            block->block->bounding_box().top());
  for (row_it.mark_cycle_pt (); !row_it.cycled_list (); row_it.forward ()) {
    row = row_it.data ();
    size = row->max_y () - row->min_y ();
    if (textord_show_expanded_rows)
      tprintf("Row at %f has min %f, max %f, size %f\n",
              row->intercept(), row->min_y(), row->max_y(), size);
    size /= textord_merge_x + textord_merge_asc + textord_merge_desc;
    ymax = size * (textord_merge_x + textord_merge_asc);
    ymin = -size * textord_merge_desc;
    row->set_limits (row->intercept () + ymin, row->intercept () + ymax);
    row->merged = FALSE;
  }
}


/**
 * @name compute_row_stats
 *
 * Compute the linespacing and offset.
 */
void compute_row_stats(                  //find lines
                       TO_BLOCK *block,  //block to do
                       BOOL8 testing_on  //correct orientation
                      ) {
  inT32 row_index;               //of median
  TO_ROW *row;                   //current row
  TO_ROW *prev_row;              //previous row
  float iqr;                     //inter quartile range
  TO_ROW_IT row_it = block->get_rows ();
                                 //number of rows
  inT16 rowcount = row_it.length ();
  TO_ROW **rows;                 //for choose nth

  rows = (TO_ROW **) alloc_mem (rowcount * sizeof (TO_ROW *));
  if (rows == NULL)
    MEMORY_OUT.error ("compute_row_stats", ABORT, NULL);
  rowcount = 0;
  prev_row = NULL;
  row_it.move_to_last ();        //start at bottom
  do {
    row = row_it.data ();
    if (prev_row != NULL) {
      rows[rowcount++] = prev_row;
      prev_row->spacing = row->intercept () - prev_row->intercept ();
      if (testing_on)
        tprintf ("Row at %g yields spacing of %g\n",
          row->intercept (), prev_row->spacing);
    }
    prev_row = row;
    row_it.backward ();
  }
  while (!row_it.at_last ());
  block->key_row = prev_row;
  block->baseline_offset =
    fmod (prev_row->parallel_c (), block->line_spacing);
  if (testing_on)
    tprintf ("Blob based spacing=(%g,%g), offset=%g",
      block->line_size, block->line_spacing, block->baseline_offset);
  if (rowcount > 0) {
    row_index = choose_nth_item (rowcount * 3 / 4, rows, rowcount,
      sizeof (TO_ROW *), row_spacing_order);
    iqr = rows[row_index]->spacing;
    row_index = choose_nth_item (rowcount / 4, rows, rowcount,
      sizeof (TO_ROW *), row_spacing_order);
    iqr -= rows[row_index]->spacing;
    row_index = choose_nth_item (rowcount / 2, rows, rowcount,
      sizeof (TO_ROW *), row_spacing_order);
    block->key_row = rows[row_index];
    if (testing_on)
      tprintf (" row based=%g(%g)", rows[row_index]->spacing, iqr);
    if (rowcount > 2
    && iqr < rows[row_index]->spacing * textord_linespace_iqrlimit) {
      if (!textord_new_initial_xheight) {
        if (rows[row_index]->spacing < block->line_spacing
          && rows[row_index]->spacing > block->line_size)
          //within range
          block->line_size = rows[row_index]->spacing;
        //spacing=size
        else if (rows[row_index]->spacing > block->line_spacing)
          block->line_size = block->line_spacing;
        //too big so use max
      }
      else {
        if (rows[row_index]->spacing < block->line_spacing)
          block->line_size = rows[row_index]->spacing;
        else
          block->line_size = block->line_spacing;
        //too big so use max
      }
      if (block->line_size < textord_min_xheight)
        block->line_size = (float) textord_min_xheight;
      block->line_spacing = rows[row_index]->spacing;
      block->max_blob_size =
        block->line_spacing * textord_excess_blobsize;
    }
    block->baseline_offset = fmod (rows[row_index]->intercept (),
      block->line_spacing);
  }
  if (testing_on)
    tprintf ("\nEstimate line size=%g, spacing=%g, offset=%g\n",
      block->line_size, block->line_spacing, block->baseline_offset);
  free_mem(rows);
}


/**
 * @name compute_block_xheight
 *
 * Compute the xheight of the individual rows, then correlate them
 * and interpret ascenderless lines, correcting xheights.
 *
 * First we compute our best guess of the x-height of each row independently
 * with compute_row_xheight(), which looks for a pair of commonly occurring
 * heights that could be x-height and ascender height. This function also
 * attempts to find descenders of lowercase letters (i.e. not the small
 * descenders that could appear in upper case letters as Q,J).
 *
 * After this computation each row falls into one of the following categories:
 * ROW_ASCENDERS_FOUND: we found xheight and ascender modes, so this must be
 *                      a regular row; we'll use its xheight to compute
 *                      xheight and ascrise estimates for the block
 * ROW_DESCENDERS_FOUND: no ascenders, so we do not have a high confidence in
 *                       the xheight of this row (don't use it for estimating
 *                       block xheight), but this row can't contain all caps
 * ROW_UNKNOWN: a row with no ascenders/descenders, could be all lowercase
 *              (or mostly lowercase for fonts with very few ascenders),
 *              all upper case or small caps
 * ROW_INVALID: no meaningful xheight could be found for this row
 *
 * We then run correct_row_xheight() and use the computed xheight and ascrise
 * averages to correct xheight values of the rows in ROW_DESCENDERS_FOUND,
 * ROW_UNKNOWN and ROW_INVALID categories.
 *
 */
void compute_block_xheight(TO_BLOCK *block, float gradient,
                           tesseract::Tesseract *tess) {
  TO_ROW *row;                          // current row
  float asc_frac_xheight = textord_merge_asc / textord_merge_x;
  float desc_frac_xheight = textord_merge_desc / textord_merge_x;
  inT32 min_height, max_height;         // limits on xheight
  TO_ROW_IT row_it = block->get_rows();
  if (row_it.empty()) return;  // no rows

  // Compute the best guess of xheight of each row individually.
  // Use xheight and ascrise values of the rows where ascenders were found.
  get_min_max_xheight(block->line_size, &min_height, &max_height);
  STATS row_asc_xheights(min_height, max_height + 1);
  STATS row_asc_ascrise(static_cast<int>(min_height * asc_frac_xheight),
                        static_cast<int>(max_height * asc_frac_xheight) + 1);
  int min_desc_height = static_cast<int>(min_height * desc_frac_xheight);
  int max_desc_height = static_cast<int>(max_height * desc_frac_xheight);
  STATS row_asc_descdrop(min_desc_height, max_desc_height + 1);
  STATS row_desc_xheights(min_height, max_height + 1);
  STATS row_desc_descdrop(min_desc_height, max_desc_height + 1);
  STATS row_cap_xheights(min_height, max_height + 1);
  STATS row_cap_floating_xheights(min_height, max_height + 1);
  for (row_it.mark_cycle_pt(); !row_it.cycled_list(); row_it.forward()) {
    row = row_it.data();
    // Compute the xheight of this row if it has not been computed before.
    if (row->xheight <= 0.0) {
      compute_row_xheight(row, gradient, block->line_size, tess);
    }
    ROW_CATEGORY row_category = get_row_category(row);
    if (row_category == ROW_ASCENDERS_FOUND) {
      row_asc_xheights.add(static_cast<inT32>(row->xheight),
                           row->xheight_evidence);
      row_asc_ascrise.add(static_cast<inT32>(row->ascrise),
                          row->xheight_evidence);
      row_asc_descdrop.add(static_cast<inT32>(-row->descdrop),
                           row->xheight_evidence);
    } else if (row_category == ROW_DESCENDERS_FOUND) {
      row_desc_xheights.add(static_cast<inT32>(row->xheight),
                            row->xheight_evidence);
      row_desc_descdrop.add(static_cast<inT32>(-row->descdrop),
                            row->xheight_evidence);
    } else if (row_category == ROW_UNKNOWN) {
      fill_heights(row, gradient, min_height, max_height,
                   &row_cap_xheights, &row_cap_floating_xheights);
    }
  }

  float xheight = 0.0;
  float ascrise = 0.0;
  float descdrop = 0.0;
  // Compute our best guess of xheight of this block.
  if (row_asc_xheights.get_total() > 0) {
    // Determine xheight from rows where ascenders were found.
    xheight = row_asc_xheights.median();
    ascrise = row_asc_ascrise.median();
    descdrop = -row_asc_descdrop.median();
  } else if (row_desc_xheights.get_total() > 0) {
    // Determine xheight from rows where descenders were found.
    xheight = row_desc_xheights.median();
    descdrop = -row_desc_descdrop.median();
  } else if (row_cap_xheights.get_total() > 0) {
    // All the rows in the block were (a/de)scenderless.
    // Try to search for two modes in row_cap_heights that could
    // be the xheight and the capheight (e.g. some of the rows
    // were lowercase, but did not have enough (a/de)scenders.
    // If such two modes can not be found, this block is most
    // likely all caps (or all small caps, in which case the code
    // still works as intended).
    compute_xheight_from_modes(&row_cap_xheights, &row_cap_floating_xheights,
                               min_height, max_height, &(xheight), &(ascrise));
    if (ascrise == 0) {  // assume only caps in the whole block
      xheight = row_cap_xheights.median() * textord_merge_x /
        (textord_merge_x + textord_merge_asc);
    }
  } else {  // default block sizes
    xheight = block->line_size * textord_merge_x;
  }
  // Correct xheight, ascrise and descdrop if necessary.
  bool corrected_xheight = false;
  if (xheight < textord_min_xheight) {
    xheight = static_cast<float>(textord_min_xheight);
    corrected_xheight = true;
  }
  if (corrected_xheight || ascrise <= 0.0) {
    ascrise = xheight * asc_frac_xheight;
  }
  if (corrected_xheight || descdrop >= 0.0) {
    descdrop = -(xheight * desc_frac_xheight);
  }
  block->xheight = xheight;

  if (textord_debug_xheights) {
    tprintf("Block average xheight=%.4f, ascrise=%.4f, descdrop=%.4f\n",
            xheight, ascrise, descdrop);
  }
  // Correct xheight, ascrise, descdrop of rows based on block averages.
  for (row_it.mark_cycle_pt(); !row_it.cycled_list(); row_it.forward()) {
    correct_row_xheight(row_it.data(), xheight, ascrise, descdrop);
  }
}

/**
 * @name compute_row_xheight
 *
 * Estimate the xheight of this row.
 * Compute the ascender rise and descender drop at the same time.
 * Set xheigh_evidence to the number of blobs with the chosen xheight
 * that appear in this row.
 */
void compute_row_xheight(TO_ROW *row,          // row to do
                         float gradient,       // global skew
                         int block_line_size,
                         tesseract::Tesseract *tess) {
  // Find blobs representing repeated characters in rows and mark them.
  // This information is used for computing row xheight and at a later
  // stage when words are formed by make_words.
  if (!row->rep_chars_marked()) {
    mark_repeated_chars(row, block_line_size * textord_merge_x, tess);
  }

  int min_height, max_height;
  get_min_max_xheight(block_line_size, &min_height, &max_height);
  STATS heights(min_height, max_height + 1);
  STATS floating_heights(min_height, max_height + 1);
  fill_heights(row, gradient, min_height, max_height,
               &heights, &floating_heights);
  row->ascrise = 0.0f;
  row->xheight = 0.0f;
  row->xheight_evidence =
    compute_xheight_from_modes(&heights, &floating_heights, min_height,
                               max_height, &(row->xheight), &(row->ascrise));
  row->descdrop = 0.0f;
  if (row->xheight > 0.0) {
    row->descdrop = static_cast<float>(
        compute_row_descdrop(row, gradient, row->xheight_evidence, &heights));
  } else {
    // Since we could not find a meaningful xheight, the results
    // of mark_repeated_chars() should be recomputed at a later stage.
    row->clear_rep_chars_marked();
  }
}

/**
 * @name fill_heights
 *
 * Fill the given heights with heights of the blobs that are legal
 * candidates for estimating xheight.
 */
void fill_heights(TO_ROW *row, float gradient, int min_height,
                  int max_height, STATS *heights, STATS *floating_heights) {
  float xcentre;                 // centre of blob
  float top;                     // top y coord of blob
  float height;                  // height of blob
  BLOBNBOX *blob;                // current blob
  int repeated_set;
  BLOBNBOX_IT blob_it = row->blob_list();
  if (blob_it.empty()) return;  // no blobs in this row
  bool has_rep_chars =
    row->rep_chars_marked() && row->num_repeated_sets() > 0;
  do {
    blob = blob_it.data();
    if (!blob->joined_to_prev()) {
      xcentre = (blob->bounding_box().left() +
                 blob->bounding_box().right()) / 2.0f;
      top = blob->bounding_box().top();
      height = blob->bounding_box().height();
      if (textord_fix_xheight_bug)
        top -= row->baseline.y(xcentre);
      else
        top -= gradient * xcentre + row->parallel_c();
      if (top >= min_height && top <= max_height) {
        heights->add(static_cast<inT32>(floor(top + 0.5)), 1);
        if (height / top < textord_min_blob_height_fraction) {
          floating_heights->add(static_cast<inT32>(floor(top + 0.5)), 1);
        }
      }
    }
    // Skip repeated chars, since they are likely to skew the height stats.
    if (has_rep_chars && blob->repeated_set() != 0) {
      repeated_set = blob->repeated_set();
      blob_it.forward();
      while (!blob_it.at_first() &&
             blob_it.data()->repeated_set() == repeated_set) {
        blob_it.forward();
        if (textord_debug_xheights)
          tprintf("Skipping repeated char when computing xheight\n");
      }
    } else {
      blob_it.forward();
    }
  } while (!blob_it.at_first());
}

/**
 * @name compute_xheight_from_modes
 *
 * Given a STATS object heights, looks for two most frequently occurring
 * heights that look like xheight and xheight + ascrise. If found, sets
 * the values of *xheight and *ascrise accordingly, otherwise sets xheight
 * to any most frequently occurring height and sets *ascrise to 0.
 * Returns the number of times xheight occurred in heights.
 * For each mode that is considered for being an xheight the count of
 * floating blobs (stored in floating_heights) is subtracted from the
 * total count of the blobs of this height. This is done because blobs
 * that sit far above the baseline could represent valid ascenders, but
 * it is highly unlikely that such a character's height will be an xheight
 * (e.g.  -, ', =, ^, `, ", ', etc)
 */
int compute_xheight_from_modes(
    STATS *heights, STATS *floating_heights, int min_height,
    int max_height, float *xheight, float *ascrise) {
  int blob_index = heights->mode();  // find mode
  int blob_count = heights->pile_count(blob_index);  // get count of mode
  if (textord_debug_xheights) {
    tprintf ("min_height=%d, max_height=%d, mode=%d, count=%d, total=%d\n",
      min_height, max_height, blob_index, blob_count, heights->get_total());
    heights->print(NULL, true);
    floating_heights->print(NULL, true);
  }
  if (blob_count == 0) return 0;
  int modes[MAX_HEIGHT_MODES]; // biggest piles
  bool in_best_pile = FALSE;
  int prev_size = -MAX_INT32;
  int best_count = 0;
  int mode_count = compute_height_modes(heights, min_height, max_height,
                                        modes, MAX_HEIGHT_MODES);
  int x;
  if (textord_debug_xheights) {
    tprintf("found %d modes: ", mode_count);
    for (x = 0; x < mode_count; x++) tprintf("%d ", modes[x]);
    tprintf("\n");
  }

  for (x = 0; x < mode_count - 1; x++) {
    if (modes[x] != prev_size + 1)
      in_best_pile = FALSE;    // had empty height
    int modes_x_count = heights->pile_count(modes[x]) -
      floating_heights->pile_count(modes[x]);
    if ((modes_x_count >= blob_count * textord_xheight_mode_fraction) &&
        (in_best_pile || modes_x_count > best_count)) {
      for (int asc = x + 1; asc < mode_count; asc++) {
        float ratio =
          static_cast<float>(modes[asc]) / static_cast<float>(modes[x]);
        if (textord_ascx_ratio_min < ratio &&
            ratio < textord_ascx_ratio_max &&
            (heights->pile_count(modes[asc]) >=
             blob_count * textord_ascheight_mode_fraction)) {
          if (modes_x_count > best_count) {
            in_best_pile = true;
            best_count = modes_x_count;
          }
          if (textord_debug_xheights) {
            tprintf("X=%d, asc=%d, count=%d, ratio=%g\n",
                    modes[x], modes[asc]-modes[x], modes_x_count, ratio);
          }
          prev_size = modes[x];
          *xheight = static_cast<float>(modes[x]);
          *ascrise = static_cast<float>(modes[asc] - modes[x]);
        }
      }
    }
  }
  if (*xheight == 0) {  // single mode
    // Remove counts of the "floating" blobs (the one whose height is too
    // small in relation to it's top end of the bounding box) from heights
    // before computing the single-mode xheight.
    // Restore the counts in heights after the mode is found, since
    // floating blobs might be useful for determining potential ascenders
    // in compute_row_descdrop().
    if (floating_heights->get_total() > 0) {
      for (x = min_height; x < max_height; ++x) {
        heights->add(x, -(floating_heights->pile_count(x)));
      }
      blob_index = heights->mode();  // find the modified mode
      for (x = min_height; x < max_height; ++x) {
        heights->add(x, floating_heights->pile_count(x));
      }
    }
    *xheight = static_cast<float>(blob_index);
    *ascrise = 0.0f;
    best_count = heights->pile_count(blob_index);
    if (textord_debug_xheights)
      tprintf("Single mode xheight set to %g\n", *xheight);
  } else if (textord_debug_xheights) {
    tprintf("Multi-mode xheight set to %g, asc=%g\n", *xheight, *ascrise);
  }
  return best_count;
}

/**
 * @name compute_row_descdrop
 *
 * Estimates the descdrop of this row. This function looks for
 * "significant" descenders of lowercase letters (those that could
 * not just be the small descenders of upper case letters like Q,J).
 * The function also takes into account how many potential ascenders
 * this row might contain. If the number of potential ascenders along
 * with descenders is close to the expected fraction of the total
 * number of blobs in the row, the function returns the descender
 * height, returns 0 otherwise.
 */
inT32 compute_row_descdrop(TO_ROW *row, float gradient,
                           int xheight_blob_count, STATS *asc_heights) {
  // Count how many potential ascenders are in this row.
  int i_min = asc_heights->min_bucket();
  if ((i_min / row->xheight) < textord_ascx_ratio_min) {
    i_min = static_cast<int>(
        floor(row->xheight * textord_ascx_ratio_min + 0.5));
  }
  int i_max = asc_heights->max_bucket();
  if ((i_max / row->xheight) > textord_ascx_ratio_max) {
    i_max = static_cast<int>(floor(row->xheight * textord_ascx_ratio_max));
  }
  int num_potential_asc = 0;
  for (int i = i_min; i <= i_max; ++i) {
    num_potential_asc += asc_heights->pile_count(i);
  }
  inT32 min_height =
    static_cast<inT32>(floor(row->xheight * textord_descx_ratio_min + 0.5));
  inT32 max_height =
    static_cast<inT32>(floor(row->xheight * textord_descx_ratio_max));
  float xcentre;                 // centre of blob
  float height;                  // height of blob
  BLOBNBOX_IT blob_it = row->blob_list ();
  BLOBNBOX *blob;                // current blob
  STATS heights (min_height, max_height + 1);
  for (blob_it.mark_cycle_pt(); !blob_it.cycled_list(); blob_it.forward()) {
    blob = blob_it.data();
    if (!blob->joined_to_prev()) {
      xcentre = (blob->bounding_box().left() +
                 blob->bounding_box().right()) / 2.0f;
      height = (gradient * xcentre + row->parallel_c() -
                blob->bounding_box().bottom());
      if (height >= min_height && height <= max_height)
        heights.add(static_cast<int>(floor(height + 0.5)), 1);
    }
  }
  int blob_index = heights.mode();  // find mode
  int blob_count = heights.pile_count(blob_index);  // get count of mode
  float total_fraction =
    (textord_descheight_mode_fraction + textord_ascheight_mode_fraction);
  if (static_cast<float>(blob_count + num_potential_asc) <
      xheight_blob_count * total_fraction) {
    blob_count = 0;
  }
  int descdrop = blob_count > 0 ? -blob_index : 0;
  if (textord_debug_xheights) {
    tprintf("Descdrop: %d (potential ascenders %d, descenders %d)\n",
            descdrop, num_potential_asc, blob_count);
    heights.print(NULL, true);
  }
  return descdrop;
}


/**
 * @name compute_height_modes
 *
 * Find the top maxmodes values in the input array and put their
 * indices in the output in the order in which they occurred.
 */
inT32 compute_height_modes(                   //find lines
                           STATS *heights,    //stats to search
                           inT32 min_height,  //bottom of range
                           inT32 max_height,  //top of range
                           inT32 *modes,      //output array
                           inT32 maxmodes     //size of modes
                          ) {
  inT32 pile_count;              //no in source pile
  inT32 src_count;               //no of source entries
  inT32 src_index;               //current entry
  inT32 least_count;             //height of smalllest
  inT32 least_index;             //index of least
  inT32 dest_count;              //index in modes

  src_count = max_height + 1 - min_height;
  dest_count = 0;
  least_count = MAX_INT32;
  least_index = -1;
  for (src_index = 0; src_index < src_count; src_index++) {
    pile_count = heights->pile_count (min_height + src_index);
    if (pile_count > 0) {
      if (dest_count < maxmodes) {
        if (pile_count < least_count) {
                                 //find smallest in array
          least_count = pile_count;
          least_index = dest_count;
        }
        modes[dest_count++] = min_height + src_index;
      }
      else if (pile_count >= least_count) {
        while (least_index < maxmodes - 1) {
          modes[least_index] = modes[least_index + 1];
          //shuffle up
          least_index++;
        }
                                 //new one on end
        modes[maxmodes - 1] = min_height + src_index;
        if (pile_count == least_count) {
                                 //new smallest
          least_index = maxmodes - 1;
        }
        else {
          least_count = heights->pile_count (modes[0]);
          least_index = 0;
          for (dest_count = 1; dest_count < maxmodes; dest_count++) {
            pile_count = heights->pile_count (modes[dest_count]);
            if (pile_count < least_count) {
                                 //find smallest
              least_count = pile_count;
              least_index = dest_count;
            }
          }
        }
      }
    }
  }
  return dest_count;
}


/**
 * @name correct_row_xheight
 *
 * Adjust the xheight etc of this row if not within reasonable limits
 * of the average for the block.
 */
void correct_row_xheight(TO_ROW *row, float xheight,
                         float ascrise, float descdrop) {
  ROW_CATEGORY row_category = get_row_category(row);
  if (textord_debug_xheights) {
    tprintf("correcting row xheight: row->xheight %.4f"
            ", row->acrise %.4f row->descdrop %.4f\n",
            row->xheight, row->ascrise, row->descdrop);
  }
  bool normal_xheight =
    within_error_margin(row->xheight, xheight, textord_xheight_error_margin);
  bool cap_xheight =
    within_error_margin(row->xheight, xheight + ascrise,
                        textord_xheight_error_margin);
  // Use the average xheight/ascrise for the following cases:
  // -- the xheight of the row could not be determined at all
  // -- the row has descenders (e.g. "many groups", "ISBN 12345 p.3")
  //    and its xheight is close to either cap height or average xheight
  // -- the row does not have ascenders or descenders, but its xheight
  //    is close to the average block xheight (e.g. row with "www.mmm.com")
  if (row_category == ROW_ASCENDERS_FOUND) {
    if (row->descdrop >= 0.0)  {
      row->descdrop = row->xheight * (descdrop / xheight);
    }
  } else  if (row_category == ROW_INVALID ||
              (row_category == ROW_DESCENDERS_FOUND &&
               (normal_xheight || cap_xheight)) ||
              (row_category == ROW_UNKNOWN && normal_xheight)) {
    if (textord_debug_xheights) tprintf("using average xheight\n");
    row->xheight = xheight;
    row->ascrise = ascrise;
    row->descdrop = descdrop;
  }
  // Assume this is a row with mostly lowercase letters and it's xheight
  // is computed correctly (unfortunately there is no way to distinguish
  // this from the case when descenders are found, but the most common
  // height is capheight).
  else if (row_category == ROW_DESCENDERS_FOUND) {
    if (textord_debug_xheights) tprintf("lowercase, corrected ascrise\n");
    row->ascrise = row->xheight * (ascrise / xheight);
  }
  // Otherwise assume this row is an all-caps or small-caps row
  // and adjust xheight and ascrise of the row.
  else if (row_category == ROW_UNKNOWN) {
    row->all_caps = true;
    if (cap_xheight) { // regular all caps
      if (textord_debug_xheights) tprintf("all caps\n");
      row->xheight = xheight;
      row->ascrise = ascrise;
      row->descdrop = descdrop;
    } else {  // small caps or caps with an odd xheight
      if (textord_debug_xheights) {
        if (row->xheight < xheight + ascrise && row->xheight > xheight) {
          tprintf("small caps\n");
        } else {
          tprintf("all caps with irregular xheight\n");
        }
      }
      row->ascrise = row->xheight * (ascrise / (xheight + ascrise));
      row->xheight -= row->ascrise;
      row->descdrop = row->xheight * (descdrop / xheight);
    }
  }
  if (textord_debug_xheights) {
    tprintf("corrected row->xheight = %.4f, row->acrise = %.4f, row->descdrop"
            " = %.4f\n", row->xheight, row->ascrise, row->descdrop);
  }
}

static int CountOverlaps(const TBOX& box, int min_height,
                         BLOBNBOX_LIST* blobs) {
  int overlaps = 0;
  BLOBNBOX_IT blob_it(blobs);
  for (blob_it.mark_cycle_pt(); !blob_it.cycled_list(); blob_it.forward()) {
    BLOBNBOX* blob = blob_it.data();
    TBOX blob_box = blob->bounding_box();
    if (blob_box.height() >= min_height && box.major_overlap(blob_box)) {
      ++overlaps;
    }
  }
  return overlaps;
}

/**
 * @name separate_underlines
 *
 * Test wide objects for being potential underlines. If they are then
 * put them in a separate list in the block.
 */
void separate_underlines(                  //make rough chars
                         TO_BLOCK *block,  //block to do
                         float gradient,   //skew angle
                         FCOORD rotation,  //inverse landscape
                         BOOL8 testing_on  //correct orientation
                        ) {
  BLOBNBOX *blob;                //current blob
  PBLOB *poly_blob;              //rotated blob
  C_BLOB *rotated_blob;          //rotated blob
  TO_ROW *row;                   //current row
  float length;                  //of g_vec
  TBOX blob_box;
  FCOORD blob_rotation;          //inverse of rotation
  FCOORD g_vec;                  //skew rotation
  BLOBNBOX_IT blob_it;           //iterator
                                 //iterator
  BLOBNBOX_IT under_it = &block->underlines;
  BLOBNBOX_IT large_it = &block->large_blobs;
  TO_ROW_IT row_it = block->get_rows ();
  int min_blob_height = static_cast<int>(textord_min_blob_height_fraction *
                                         block->line_size + 0.5);

                                 //length of vector
  length = sqrt (1 + gradient * gradient);
  g_vec = FCOORD (1 / length, -gradient / length);
  blob_rotation = FCOORD (rotation.x (), -rotation.y ());
  blob_rotation.rotate (g_vec);  //unoding everything
  for (row_it.mark_cycle_pt (); !row_it.cycled_list (); row_it.forward ()) {
    row = row_it.data ();
                                 //get blobs
    blob_it.set_to_list (row->blob_list ());
    for (blob_it.mark_cycle_pt (); !blob_it.cycled_list ();
    blob_it.forward ()) {
      blob = blob_it.data ();
      blob_box = blob->bounding_box ();
      if (blob_box.width () > block->line_size * textord_underline_width) {
        if (textord_cblob_blockocc && blob->cblob () != NULL) {
          rotated_blob = crotate_cblob (blob->cblob (),
            blob_rotation);
          if (test_underline(testing_on && textord_show_final_rows,
                             rotated_blob, static_cast<inT16>(row->intercept()),
                             static_cast<inT16>(block->line_size *
                                                (textord_merge_x +
                                                 textord_merge_asc / 2.0f)))) {
            under_it.add_after_then_move(blob_it.extract());
            if (testing_on && textord_show_final_rows) {
              tprintf("Underlined blob at:");
                rotated_blob->bounding_box().print();
              tprintf("Was:");
                blob_box.print();
            }
          } else if (CountOverlaps(blob->bounding_box(), min_blob_height,
                                   row->blob_list()) >
                     textord_max_blob_overlaps) {
            large_it.add_after_then_move(blob_it.extract());
            if (testing_on && textord_show_final_rows) {
              tprintf("Large blob overlaps %d blobs at:",
                      CountOverlaps(blob_box, min_blob_height,
                                    row->blob_list()));
              blob_box.print();
            }
          }
          delete rotated_blob;
        }
        else {
          if (blob->blob () != NULL) {
            //                                      if (testing_on && textord_show_final_rows)
            //                                              tprintf("Rotating by (%g,%g)\n",
            //                                                      blob_rotation.x(),blob_rotation.y());
            poly_blob = rotate_blob (blob->blob (), blob_rotation);
          }
          else
            poly_blob = rotate_cblob (blob->cblob (),
              block->line_size,
              blob_rotation);
          if (test_underline
            (testing_on
            && textord_show_final_rows, poly_blob,
            row->intercept (),
            block->line_size * (textord_merge_x +
          textord_merge_asc / 2))) {
            if (testing_on && textord_show_final_rows) {
              tprintf ("Underlined blob at (%d,%d)->(%d,%d) ",
                poly_blob->bounding_box ().left (),
                poly_blob->bounding_box ().bottom (),
                poly_blob->bounding_box ().right (),
                poly_blob->bounding_box ().top ());
              tprintf ("(Was (%d,%d)->(%d,%d))\n",
                blob_box.left (), blob_box.bottom (),
                blob_box.right (), blob_box.top ());
            }
            under_it.add_after_then_move (blob_it.extract ());
          }
          delete poly_blob;
        }
      }
    }
  }
}


/**
 * @name pre_associate_blobs
 *
 * Associate overlapping blobs and fake chop wide blobs.
 */
void pre_associate_blobs(                  //make rough chars
                         ICOORD page_tr,   //top right
                         TO_BLOCK *block,  //block to do
                         FCOORD rotation,  //inverse landscape
                         BOOL8 testing_on  //correct orientation
                        ) {
#ifndef GRAPHICS_DISABLED
  ScrollView::Color colour;                 //of boxes
#endif
  BLOBNBOX *blob;                //current blob
  BLOBNBOX *nextblob;            //next in list
  TBOX blob_box;
  FCOORD blob_rotation;          //inverse of rotation
  BLOBNBOX_IT blob_it;           //iterator
  BLOBNBOX_IT start_it;          //iterator
  TO_ROW_IT row_it = block->get_rows ();

#ifndef GRAPHICS_DISABLED
  colour = ScrollView::RED;
#endif

  blob_rotation = FCOORD (rotation.x (), -rotation.y ());
  for (row_it.mark_cycle_pt (); !row_it.cycled_list (); row_it.forward ()) {
                                 //get blobs
    blob_it.set_to_list (row_it.data ()->blob_list ());
    for (blob_it.mark_cycle_pt (); !blob_it.cycled_list ();
    blob_it.forward ()) {
      blob = blob_it.data ();
      blob_box = blob->bounding_box ();
      start_it = blob_it;        //save start point
      //                      if (testing_on && textord_show_final_blobs)
      //                      {
      //                              tprintf("Blob at (%d,%d)->(%d,%d), addr=%x, count=%d\n",
      //                                      blob_box.left(),blob_box.bottom(),
      //                                      blob_box.right(),blob_box.top(),
      //                                      (void*)blob,blob_it.length());
      //                      }
      bool overlap;
      do {
        overlap = false;
        if (!blob_it.at_last ()) {
          nextblob = blob_it.data_relative(1);
          overlap = blob_box.major_x_overlap(nextblob->bounding_box());
          if (overlap) {
            blob->merge(nextblob); // merge new blob
            blob_box = blob->bounding_box(); // get bigger box
            blob_it.forward();
          }
        }
      }
      while (overlap);
      blob->chop (&start_it, &blob_it,
        blob_rotation,
        block->line_size * textord_merge_x *
        textord_chop_width);
      //attempt chop
    }
#ifndef GRAPHICS_DISABLED
    if (testing_on && textord_show_final_blobs) {
      if (to_win == NULL)
        create_to_win(page_tr);
      to_win->Pen(colour);
      for (blob_it.mark_cycle_pt (); !blob_it.cycled_list ();
      blob_it.forward ()) {
        blob = blob_it.data ();
        blob_box = blob->bounding_box ();
        blob_box.rotate (rotation);
        if (!blob->joined_to_prev ()) {
          to_win->Rectangle (blob_box.left (), blob_box.bottom (),
            blob_box.right (), blob_box.top ());
        }
      }
      colour = (ScrollView::Color) (colour + 1);
      if (colour > ScrollView::MAGENTA)
        colour = ScrollView::RED;
    }
#endif
  }
}


/**
 * @name fit_parallel_rows
 *
 * Re-fit the rows in the block to the given gradient.
 */
void fit_parallel_rows(                   //find lines
                       TO_BLOCK *block,   //block to do
                       float gradient,    //gradient to fit
                       FCOORD rotation,   //for drawing
                       inT32 block_edge,  //edge of block
                       BOOL8 testing_on   //correct orientation
                      ) {
#ifndef GRAPHICS_DISABLED
  ScrollView::Color colour;                 //of row
#endif
  TO_ROW_IT row_it = block->get_rows ();

  row_it.move_to_first ();
  for (row_it.mark_cycle_pt (); !row_it.cycled_list (); row_it.forward ()) {
    if (row_it.data ()->blob_list ()->empty ())
      delete row_it.extract ();  //nothing in it
    else
      fit_parallel_lms (gradient, row_it.data ());
  }
#ifndef GRAPHICS_DISABLED
  if (testing_on) {
    colour = ScrollView::RED;
    for (row_it.mark_cycle_pt (); !row_it.cycled_list (); row_it.forward ()) {
      plot_parallel_row (row_it.data (), gradient,
        block_edge, colour, rotation);
      colour = (ScrollView::Color) (colour + 1);
      if (colour > ScrollView::MAGENTA)
        colour = ScrollView::RED;
    }
  }
#endif
  row_it.sort (row_y_order);     //may have gone out of order
}


/**
 * @name fit_parallel_lms
 *
 * Fit an LMS line to a row.
 * Make the fit parallel to the given gradient and set the
 * row accordingly.
 */
void fit_parallel_lms(                 //sort function
                      float gradient,  //forced gradient
                      TO_ROW *row      //row to fit
                     ) {
  float c;                       //fitted line
  int blobcount;                 //no of blobs
  TBOX box;                       //blob box
  LMS lms (row->blob_list ()->length ());
                                 //blobs
  BLOBNBOX_IT blob_it = row->blob_list ();

  blobcount = 0;
  for (blob_it.mark_cycle_pt (); !blob_it.cycled_list (); blob_it.forward ()) {
    if (!blob_it.data ()->joined_to_prev ()) {
      box = blob_it.data ()->bounding_box ();
      lms.
        add (FCOORD ((box.left () + box.right ()) / 2.0, box.bottom ()));
      blobcount++;
    }
  }
  lms.constrained_fit (gradient, c);
  row->set_parallel_line (gradient, c, lms.error ());
  if (textord_straight_baselines && blobcount > lms_line_trials) {
    lms.fit (gradient, c);
  }
                                 //set the other too
  row->set_line (gradient, c, lms.error ());
}


/**
 * @name make_spline_rows
 *
 * Re-fit the rows in the block to the given gradient.
 */
void make_spline_rows(                   //find lines
                      TO_BLOCK *block,   //block to do
                      float gradient,    //gradient to fit
                      FCOORD rotation,   //for drawing
                      inT32 block_edge,  //edge of block
                      BOOL8 testing_on,  //correct orientation
                      tesseract::Tesseract* tess
                      ) {
#ifndef GRAPHICS_DISABLED
  ScrollView::Color colour;       //of row
#endif
  TO_ROW_IT row_it = block->get_rows ();

  row_it.move_to_first ();
  for (row_it.mark_cycle_pt (); !row_it.cycled_list (); row_it.forward ()) {
    if (row_it.data ()->blob_list ()->empty ())
      delete row_it.extract ();  //nothing in it
    else
      make_baseline_spline (row_it.data (), block);
  }
  if (textord_old_baselines) {
#ifndef GRAPHICS_DISABLED
    if (testing_on) {
      colour = ScrollView::RED;
      for (row_it.mark_cycle_pt (); !row_it.cycled_list ();
      row_it.forward ()) {
        row_it.data ()->baseline.plot (to_win, colour);
        colour = (ScrollView::Color) (colour + 1);
        if (colour > ScrollView::MAGENTA)
          colour = ScrollView::RED;
      }
    }
#endif
    make_old_baselines(block, testing_on, gradient, tess);
  }
#ifndef GRAPHICS_DISABLED
  if (testing_on) {
    colour = ScrollView::RED;
    for (row_it.mark_cycle_pt (); !row_it.cycled_list (); row_it.forward ()) {
      row_it.data ()->baseline.plot (to_win, colour);
      colour = (ScrollView::Color) (colour + 1);
      if (colour > ScrollView::MAGENTA)
        colour = ScrollView::RED;
    }
  }
#endif
}


/**
 * @name make_baseline_spline
 *
 * Fit an LMS line to a row.
 * Make the fit parallel to the given gradient and set the
 * row accordingly.
 */
void make_baseline_spline(                 //sort function
                          TO_ROW *row,     //row to fit
                          TO_BLOCK *block  //block it came from
                         ) {
  float b, c;                    //fitted curve
  float middle;                  //x middle of blob
  TBOX box;                       //blob box
  LMS lms (row->blob_list ()->length ());
                                 //blobs
  BLOBNBOX_IT blob_it = row->blob_list ();
  inT32 *xstarts;                //spline boundaries
  double *coeffs;                //quadratic coeffs
  inT32 segments;                //no of segments
  inT32 segment;                 //current segment

  xstarts =
    (inT32 *) alloc_mem ((row->blob_list ()->length () + 1) * sizeof (inT32));
  if (segment_baseline (row, block, segments, xstarts)
  && !textord_straight_baselines && !textord_parallel_baselines) {
    if (textord_quadratic_baselines) {
      coeffs = (double *) alloc_mem (segments * 3 * sizeof (double));
      for (segment = 0; segment < segments; segment++) {
        lms.clear ();
        for (blob_it.mark_cycle_pt (); !blob_it.cycled_list ();
        blob_it.forward ()) {
          if (!blob_it.data ()->joined_to_prev ()) {
            box = blob_it.data ()->bounding_box ();
            middle = (box.left () + box.right ()) / 2.0;
            if (middle >= xstarts[segment]
            && middle < xstarts[segment + 1]) {
              lms.add (FCOORD (middle, box.bottom ()));
            }
          }
        }
        if (textord_quadratic_baselines)
          lms.fit_quadratic (block->line_size *
            textord_spline_outlier_fraction,
            coeffs[segment * 3], b, c);
        else {
          lms.fit (b, c);
          coeffs[segment * 3] = 0;
        }
        coeffs[segment * 3 + 1] = b;
        coeffs[segment * 3 + 2] = c;
      }
    }
    else
      coeffs = linear_spline_baseline (row, block, segments, xstarts);
  }
  else {
    xstarts[1] = xstarts[segments];
    segments = 1;
    coeffs = (double *) alloc_mem (3 * sizeof (double));
    coeffs[0] = 0;
    coeffs[1] = row->line_m ();
    coeffs[2] = row->line_c ();
  }
  row->baseline = QSPLINE (segments, xstarts, coeffs);
  free_mem(coeffs);
  free_mem(xstarts);
}


/**
 * @name segment_baseline
 *
 * Divide the baseline up into segments which require a different
 * quadratic fitted to them.
 * Return TRUE if enough blobs were far enough away to need a quadratic.
 */
BOOL8
segment_baseline (               //split baseline
TO_ROW * row,                    //row to fit
TO_BLOCK * block,                //block it came from
inT32 & segments,                //no fo segments
inT32 xstarts[]                  //coords of segments
) {
  BOOL8 needs_curve;             //needs curved line
  int blobcount;                 //no of blobs
  int blobindex;                 //current blob
  int last_state;                //above, on , below
  int state;                     //of current blob
  float yshift;                  //from baseline
  TBOX box;                       //blob box
  TBOX new_box;                   //new_it box
  float middle;                  //xcentre of blob
                                 //blobs
  BLOBNBOX_IT blob_it = row->blob_list ();
  BLOBNBOX_IT new_it = blob_it;  //front end
  SORTED_FLOATS yshifts;         //shifts from baseline

  needs_curve = FALSE;
  box = box_next_pre_chopped (&blob_it);
  xstarts[0] = box.left ();
  segments = 1;
  blobcount = row->blob_list ()->length ();
  if (textord_oldbl_debug)
    tprintf ("Segmenting baseline of %d blobs at (%d,%d)\n",
      blobcount, box.left (), box.bottom ());
  if (blobcount <= textord_spline_medianwin
  || blobcount < textord_spline_minblobs) {
    blob_it.move_to_last ();
    box = blob_it.data ()->bounding_box ();
    xstarts[1] = box.right ();
    return FALSE;
  }
  last_state = 0;
  new_it.mark_cycle_pt ();
  for (blobindex = 0; blobindex < textord_spline_medianwin; blobindex++) {
    new_box = box_next_pre_chopped (&new_it);
    middle = (new_box.left () + new_box.right ()) / 2.0;
    yshift = new_box.bottom () - row->line_m () * middle - row->line_c ();
                                 //record shift
    yshifts.add (yshift, blobindex);
    if (new_it.cycled_list ()) {
      xstarts[1] = new_box.right ();
      return FALSE;
    }
  }
  for (blobcount = 0; blobcount < textord_spline_medianwin / 2; blobcount++)
    box = box_next_pre_chopped (&blob_it);
  do {
    new_box = box_next_pre_chopped (&new_it);
                                 //get middle one
    yshift = yshifts[textord_spline_medianwin / 2];
    if (yshift > textord_spline_shift_fraction * block->line_size)
      state = 1;
    else if (-yshift > textord_spline_shift_fraction * block->line_size)
      state = -1;
    else
      state = 0;
    if (state != 0)
      needs_curve = TRUE;
    //              tprintf("State=%d, prev=%d, shift=%g\n",
    //                      state,last_state,yshift);
    if (state != last_state && blobcount > textord_spline_minblobs) {
      xstarts[segments++] = box.left ();
      blobcount = 0;
    }
    last_state = state;
    yshifts.remove (blobindex - textord_spline_medianwin);
    box = box_next_pre_chopped (&blob_it);
    middle = (new_box.left () + new_box.right ()) / 2.0;
    yshift = new_box.bottom () - row->line_m () * middle - row->line_c ();
    yshifts.add (yshift, blobindex);
    blobindex++;
    blobcount++;
  }
  while (!new_it.cycled_list ());
  if (blobcount > textord_spline_minblobs || segments == 1) {
    xstarts[segments] = new_box.right ();
  }
  else {
    xstarts[--segments] = new_box.right ();
  }
  if (textord_oldbl_debug)
    tprintf ("Made %d segments on row at (%d,%d)\n",
      segments, box.right (), box.bottom ());
  return needs_curve;
}


/**
 * @name linear_spline_baseline
 *
 * Divide the baseline up into segments which require a different
 * quadratic fitted to them.
 * @return TRUE if enough blobs were far enough away to need a quadratic.
 */
double *
linear_spline_baseline (         //split baseline
TO_ROW * row,                    //row to fit
TO_BLOCK * block,                //block it came from
inT32 & segments,                //no fo segments
inT32 xstarts[]                  //coords of segments
) {
  int blobcount;                 //no of blobs
  int blobindex;                 //current blob
  int index1, index2;            //blob numbers
  int blobs_per_segment;         //blobs in each
  TBOX box;                       //blob box
  TBOX new_box;                   //new_it box
  float middle;                  //xcentre of blob
                                 //blobs
  BLOBNBOX_IT blob_it = row->blob_list ();
  BLOBNBOX_IT new_it = blob_it;  //front end
  float b, c;                    //fitted curve
  LMS lms (row->blob_list ()->length ());
  double *coeffs;                //quadratic coeffs
  inT32 segment;                 //current segment

  box = box_next_pre_chopped (&blob_it);
  xstarts[0] = box.left ();
  blobcount = 1;
  while (!blob_it.at_first ()) {
    blobcount++;
    box = box_next_pre_chopped (&blob_it);
  }
  segments = blobcount / textord_spline_medianwin;
  if (segments < 1)
    segments = 1;
  blobs_per_segment = blobcount / segments;
  coeffs = (double *) alloc_mem (segments * 3 * sizeof (double));
  if (textord_oldbl_debug)
    tprintf
      ("Linear splining baseline of %d blobs at (%d,%d), into %d segments of %d blobs\n",
      blobcount, box.left (), box.bottom (), segments, blobs_per_segment);
  segment = 1;
  for (index2 = 0; index2 < blobs_per_segment / 2; index2++)
    box_next_pre_chopped(&new_it);
  index1 = 0;
  blobindex = index2;
  do {
    blobindex += blobs_per_segment;
    lms.clear ();
    while (index1 < blobindex || (segment == segments && index1 < blobcount)) {
      box = box_next_pre_chopped (&blob_it);
      middle = (box.left () + box.right ()) / 2.0;
      lms.add (FCOORD (middle, box.bottom ()));
      index1++;
      if (index1 == blobindex - blobs_per_segment / 2
      || index1 == blobcount - 1) {
        xstarts[segment] = box.left ();
      }
    }
    lms.fit (b, c);
    coeffs[segment * 3 - 3] = 0;
    coeffs[segment * 3 - 2] = b;
    coeffs[segment * 3 - 1] = c;
    segment++;
    if (segment > segments)
      break;

    blobindex += blobs_per_segment;
    lms.clear ();
    while (index2 < blobindex || (segment == segments && index2 < blobcount)) {
      new_box = box_next_pre_chopped (&new_it);
      middle = (new_box.left () + new_box.right ()) / 2.0;
      lms.add (FCOORD (middle, new_box.bottom ()));
      index2++;
      if (index2 == blobindex - blobs_per_segment / 2
      || index2 == blobcount - 1) {
        xstarts[segment] = new_box.left ();
      }
    }
    lms.fit (b, c);
    coeffs[segment * 3 - 3] = 0;
    coeffs[segment * 3 - 2] = b;
    coeffs[segment * 3 - 1] = c;
    segment++;
  }
  while (segment <= segments);
  return coeffs;
}


/**
 * @name assign_blobs_to_rows
 *
 * Make enough rows to allocate all the given blobs to one.
 * If a block skew is given, use that, else attempt to track it.
 */
void assign_blobs_to_rows(                      //find lines
                          TO_BLOCK *block,      //block to do
                          float *gradient,      //block skew
                          int pass,             //identification
                          BOOL8 reject_misses,  //chuck big ones out
                          BOOL8 make_new_rows,  //add rows for unmatched
                          BOOL8 drawing_skew    //draw smoothed skew
                         ) {
  OVERLAP_STATE overlap_result;  //what to do with it
  float ycoord;                  //current y
  float top, bottom;             //of blob
  float g_length = 1.0f;         //from gradient
  inT16 row_count;               //no of rows
  inT16 left_x;                  //left edge
  inT16 last_x;                  //previous edge
  float block_skew;              //y delta
  float smooth_factor;           //for new coords
  float near_dist;               //dist to nearest row
  ICOORD testpt;                 //testing only
  BLOBNBOX *blob;                //current blob
  TO_ROW *row;                   //current row
  TO_ROW *dest_row;              //row to put blob in
                                 //iterators
  BLOBNBOX_IT blob_it = &block->blobs;
  TO_ROW_IT row_it = block->get_rows ();

  ycoord =
    (block->block->bounding_box ().bottom () +
    block->block->bounding_box ().top ()) / 2.0f;
  if (gradient != NULL)
    g_length = sqrt (1 + *gradient * *gradient);
#ifndef GRAPHICS_DISABLED
  if (drawing_skew)
    to_win->SetCursor(block->block->bounding_box ().left (), ycoord);
#endif
  testpt = ICOORD (textord_test_x, textord_test_y);
  blob_it.sort (blob_x_order);
  smooth_factor = 1.0;
  block_skew = 0.0f;
  row_count = row_it.length ();  //might have rows
  if (!blob_it.empty ()) {
    left_x = blob_it.data ()->bounding_box ().left ();
  }
  else {
    left_x = block->block->bounding_box ().left ();
  }
  last_x = left_x;
  for (blob_it.mark_cycle_pt (); !blob_it.cycled_list (); blob_it.forward ()) {
    blob = blob_it.data ();
    if (gradient != NULL) {
      block_skew = (1 - 1 / g_length) * blob->bounding_box ().bottom ()
        + *gradient / g_length * blob->bounding_box ().left ();
    }
    else if (blob->bounding_box ().left () - last_x > block->line_size / 2
      && last_x - left_x > block->line_size * 2
    && textord_interpolating_skew) {
      //                      tprintf("Interpolating skew from %g",block_skew);
      block_skew *= (float) (blob->bounding_box ().left () - left_x)
        / (last_x - left_x);
      //                      tprintf("to %g\n",block_skew);
    }
    last_x = blob->bounding_box ().left ();
    top = blob->bounding_box ().top () - block_skew;
    bottom = blob->bounding_box ().bottom () - block_skew;
#ifndef GRAPHICS_DISABLED
    if (drawing_skew)
      to_win->DrawTo(blob->bounding_box ().left (), ycoord + block_skew);
#endif
    if (!row_it.empty ()) {
      for (row_it.move_to_first ();
        !row_it.at_last () && row_it.data ()->min_y () > top;
        row_it.forward ());
      row = row_it.data ();
      if (row->min_y () <= top && row->max_y () >= bottom) {
      //any overlap
        dest_row = row;
        overlap_result = most_overlapping_row (&row_it, dest_row,
          top, bottom,
          block->line_size,
          blob->bounding_box ().
          contains (testpt));
        if (overlap_result == NEW_ROW && !reject_misses)
          overlap_result = ASSIGN;
      }
      else {
        overlap_result = NEW_ROW;
        if (!make_new_rows) {
          near_dist = row_it.data_relative (-1)->min_y () - top;
                                 //below bottom
          if (bottom < row->min_y ()) {
            if (row->min_y () - bottom <=
              (block->line_spacing -
            block->line_size) * textord_merge_desc) {
                                 //done it
              overlap_result = ASSIGN;
              dest_row = row;
            }
          }
          else if (near_dist > 0
          && near_dist < bottom - row->max_y ()) {
            row_it.backward ();
            dest_row = row_it.data ();
            if (dest_row->min_y () - bottom <=
              (block->line_spacing -
            block->line_size) * textord_merge_desc) {
                                 //done it
              overlap_result = ASSIGN;
            }
          }
          else {
            if (top - row->max_y () <=
              (block->line_spacing -
              block->line_size) * (textord_overlap_x +
            textord_merge_asc)) {
                                 //done it
              overlap_result = ASSIGN;
              dest_row = row;
            }
          }
        }
      }
      if (overlap_result == ASSIGN)
        dest_row->add_blob (blob_it.extract (), top, bottom,
          block->line_size);
      if (overlap_result == NEW_ROW) {
        if (make_new_rows && top - bottom < block->max_blob_size) {
          dest_row =
            new TO_ROW (blob_it.extract (), top, bottom,
            block->line_size);
          row_count++;
          if (bottom > row_it.data ()->min_y ())
            row_it.add_before_then_move (dest_row);
          //insert in right place
          else
            row_it.add_after_then_move (dest_row);
          smooth_factor =
            1.0 / (row_count * textord_skew_lag +
            textord_skewsmooth_offset);
        }
        else
          overlap_result = REJECT;
      }
    }
    else if (make_new_rows && top - bottom < block->max_blob_size) {
      overlap_result = NEW_ROW;
      dest_row =
        new TO_ROW (blob_it.extract (), top, bottom, block->line_size);
      row_count++;
      row_it.add_after_then_move (dest_row);
      smooth_factor = 1.0 / (row_count * textord_skew_lag +
                             textord_skewsmooth_offset2);
    }
    else
      overlap_result = REJECT;
    if (blob->bounding_box ().contains (testpt)) {
      if (overlap_result != REJECT) {
        tprintf ("Test blob assigned to row at (%g,%g) on pass %d\n",
          dest_row->min_y (), dest_row->max_y (), pass);
      }
      else {
        tprintf ("Test blob assigned to no row on pass %d\n", pass);
      }
    }
    if (overlap_result != REJECT) {
      while (!row_it.at_first ()
        && row_it.data ()->min_y () >
      row_it.data_relative (-1)->min_y ()) {
        row = row_it.extract ();
        row_it.backward ();
        row_it.add_before_then_move (row);
      }
      while (!row_it.at_last ()
        && row_it.data ()->min_y () <
      row_it.data_relative (1)->min_y ()) {
        row = row_it.extract ();
        row_it.forward ();
                                 //keep rows in order
        row_it.add_after_then_move (row);
      }
      block_skew = (1 - smooth_factor) * block_skew
        + smooth_factor * (blob->bounding_box ().bottom () -
        dest_row->initial_min_y ());
    }
  }
  for (row_it.mark_cycle_pt (); !row_it.cycled_list (); row_it.forward ()) {
    if (row_it.data ()->blob_list ()->empty ())
      delete row_it.extract ();  //discard empty rows
  }
}


/**
 * @name most_overlapping_row
 *
 * Return the row which most overlaps the blob.
 */
OVERLAP_STATE most_overlapping_row(                    //find best row
                                   TO_ROW_IT *row_it,  //iterator
                                   TO_ROW *&best_row,  //output row
                                   float top,          //top of blob
                                   float bottom,       //bottom of blob
                                   float rowsize,      //max row size
                                   BOOL8 testing_blob  //test stuff
                                  ) {
  OVERLAP_STATE result;          //result of tests
  float overlap;                 //of blob & row
  float bestover;                //nearest row
  float merge_top, merge_bottom; //size of merged row
  ICOORD testpt;                 //testing only
  TO_ROW *row;                   //current row
  TO_ROW *test_row;              //for multiple overlaps
  BLOBNBOX_IT blob_it;           //for merging rows

  result = ASSIGN;
  row = row_it->data ();
  bestover = top - bottom;
  if (top > row->max_y ())
    bestover -= top - row->max_y ();
  if (bottom < row->min_y ())
                                 //compute overlap
    bestover -= row->min_y () - bottom;
  if (testing_blob) {
    tprintf ("Test blob y=(%g,%g), row=(%f,%f), overlap=%f\n",
      bottom, top, row->min_y (), row->max_y (), bestover);
  }
  test_row = row;
  do {
    if (!row_it->at_last ()) {
      row_it->forward ();
      test_row = row_it->data ();
      if (test_row->min_y () <= top && test_row->max_y () >= bottom) {
        merge_top =
          test_row->max_y () >
          row->max_y ()? test_row->max_y () : row->max_y ();
        merge_bottom =
          test_row->min_y () <
          row->min_y ()? test_row->min_y () : row->min_y ();
        if (merge_top - merge_bottom <= rowsize) {
          if (testing_blob) {
            tprintf ("Merging rows at (%g,%g), (%g,%g)\n",
              row->min_y (), row->max_y (),
              test_row->min_y (), test_row->max_y ());
          }
          test_row->set_limits (merge_bottom, merge_top);
          blob_it.set_to_list (test_row->blob_list ());
          blob_it.add_list_after (row->blob_list ());
          blob_it.sort (blob_x_order);
          row_it->backward ();
          delete row_it->extract ();
          row_it->forward ();
          bestover = -1.0f;      //force replacement
        }
        overlap = top - bottom;
        if (top > test_row->max_y ())
          overlap -= top - test_row->max_y ();
        if (bottom < test_row->min_y ())
          overlap -= test_row->min_y () - bottom;
        if (bestover >= rowsize - 1 && overlap >= rowsize - 1) {
          result = REJECT;
        }
        if (overlap > bestover) {
          bestover = overlap;    //find biggest overlap
          row = test_row;
        }
        if (testing_blob) {
          tprintf
            ("Test blob y=(%g,%g), row=(%f,%f), overlap=%f->%f\n",
            bottom, top, test_row->min_y (), test_row->max_y (),
            overlap, bestover);
        }
      }
    }
  }
  while (!row_it->at_last ()
    && test_row->min_y () <= top && test_row->max_y () >= bottom);
  while (row_it->data () != row)
    row_it->backward ();         //make it point to row
                                 //doesn't overlap much
  if (top - bottom - bestover > rowsize * textord_overlap_x &&
      (!textord_fix_makerow_bug || bestover < rowsize * textord_overlap_x)
    && result == ASSIGN)
    result = NEW_ROW;            //doesn't overlap enough
  best_row = row;
  return result;
}


/**
 * @name blob_x_order
 *
 * Sort function to sort blobs in x from page left.
 */
int blob_x_order(                    //sort function
                 const void *item1,  //items to compare
                 const void *item2) {
                                 //converted ptr
  BLOBNBOX *blob1 = *(BLOBNBOX **) item1;
                                 //converted ptr
  BLOBNBOX *blob2 = *(BLOBNBOX **) item2;

  if (blob1->bounding_box ().left () < blob2->bounding_box ().left ())
    return -1;
  else if (blob1->bounding_box ().left () > blob2->bounding_box ().left ())
    return 1;
  else
    return 0;
}


/**
 * @name row_y_order
 *
 * Sort function to sort rows in y from page top.
 */
int row_y_order(                    //sort function
                const void *item1,  //items to compare
                const void *item2) {
                                 //converted ptr
  TO_ROW *row1 = *(TO_ROW **) item1;
                                 //converted ptr
  TO_ROW *row2 = *(TO_ROW **) item2;

  if (row1->parallel_c () > row2->parallel_c ())
    return -1;
  else if (row1->parallel_c () < row2->parallel_c ())
    return 1;
  else
    return 0;
}


/**
 * @name row_spacing_order
 *
 * Qsort style function to compare 2 TO_ROWS based on their spacing value.
 */
int row_spacing_order(                    //sort function
                      const void *item1,  //items to compare
                      const void *item2) {
                                 //converted ptr
  TO_ROW *row1 = *(TO_ROW **) item1;
                                 //converted ptr
  TO_ROW *row2 = *(TO_ROW **) item2;

  if (row1->spacing < row2->spacing)
    return -1;
  else if (row1->spacing > row2->spacing)
    return 1;
  else
    return 0;
}

/**
 * @name make_repeated_chars
 *
 * Mark textord_repeat_threshold or more adjacent chars which are the
 * same as repeated chars.
 */
void mark_repeated_chars(TO_ROW *row, float block_xheight,
                         tesseract::Tesseract *tess) {
  ROW *real_row = NULL;          //output row
  BLOBNBOX *bblob;               //current blob
  BLOBNBOX *nextblob;            //neighbour to compare
  BLOBNBOX_IT box_it;            //iterator
  BLOBNBOX_IT search_it;         //forward search
  inT32 blobcount;               //no of neighbours
  inT32 matched_blobcount;       //no of matches
  inT32 blobindex;               //in row
  inT32 row_length;              //blobs in row
  inT32 width_change;            //max width change
  inT32 blob_width;              //required blob width
  inT32 space_width;             //required gap width
  inT32 prev_right;              //right edge of last blob
  float rating;                  //match rating
  PBLOB *pblob1;                 //polygonal blob
  PBLOB *pblob2;                 //second blob

  // kern_size and space_size are computed in the same way as in
  // compute_block_pitch().
  float kern_size = ceil(block_xheight * textord_words_default_nonspace);
  float space_size = floor(block_xheight * textord_words_default_minspace);
  int num_repeated_sets = 0;
  box_it.set_to_list(row->blob_list());
  row_length = row->blob_list()->length();
  blobindex = 0;
  if (!box_it.empty()) {
    if (textord_debug_xheights)
      tprintf("Running mark_repeated_chars(), row length %d\n", row_length);
    real_row = new ROW(row, static_cast<inT32>(kern_size),
                       static_cast<inT32>(space_size));
    // Use block_xheight, since xheight of the row (used in the ROW()
    // constructor) might not have been computed yet.
    real_row->set_x_height(block_xheight);
    do {
      bblob = box_it.data();
      blobcount = 1;
      search_it = box_it;
      search_it.forward();
      matched_blobcount = 1;
      width_change = MAX_INT16;
      blob_width = 0;
      space_width = 0;
      prev_right = bblob->bounding_box().right();
      if (bblob->bounding_box().height() * 2 < block_xheight &&
          !bblob->joined_to_prev() &&
          (bblob->blob() != NULL || bblob->cblob() != NULL)) {
        pblob1 = (bblob->cblob() != NULL) ?
          new PBLOB(bblob->cblob(), block_xheight) : bblob->blob();
        rating = 0.0f;
        while (rating < textord_repeat_rating &&
               blobindex + blobcount < row_length &&
               ((nextblob = search_it.data())->blob() != NULL ||
                nextblob->cblob() != NULL) &&
               nextblob->bounding_box().height() * 2 < block_xheight) {
          if (blobcount == 1) {
            space_width = nextblob->bounding_box().left() -
              bblob->bounding_box().right();
            blob_width = bblob->bounding_box().width();
            width_change = blob_width > space_width ? blob_width : space_width;
            width_change =
              static_cast<inT32>(width_change * textord_repch_width_variance);
            if (width_change < 3) width_change = 3;
          }
          if (nextblob->bounding_box().width() > blob_width + width_change ||
              nextblob->bounding_box().width() < blob_width - width_change ||
              nextblob->bounding_box().left() - prev_right >
              space_width + width_change ||
              nextblob->bounding_box().left() - prev_right <
              space_width - width_change) {
            break;  // not good enough
          }
          if (nextblob->blob() != NULL)
            rating = tess->compare_blobs(pblob1, real_row,
                                         nextblob->blob(), real_row);
          else {
              pblob2 = new PBLOB(nextblob->cblob(), block_xheight);
              rating = tess->compare_blobs(pblob1, real_row, pblob2, real_row);
              delete pblob2;
          }
          if (rating < textord_repeat_rating) {
            blobcount++;
            search_it.forward();
            matched_blobcount++;
            while (blobindex + blobcount < row_length &&
                   (search_it.data()->joined_to_prev() ||
                    (search_it.data()->blob() == NULL &&
                     search_it.data()->cblob() == NULL))) {
              search_it.forward();
              blobcount++;     //suck in joined bits
            }
          }
          prev_right = nextblob->bounding_box().right();
        }
        if (bblob->cblob() != NULL) delete pblob1;
      }

      // Record position and length of this run of repeated chars.
      if (matched_blobcount >= textord_repeat_threshold) {
        if (textord_debug_xheights) {
          tprintf("Found %d repeated chars starting at blob index %d\n",
                  blobcount, blobindex);
        }
        blobindex += blobcount;
        num_repeated_sets++;
        while (blobcount-- > 0 && !box_it.at_first()) {
          box_it.data()->set_repeated_set(num_repeated_sets);
          box_it.forward();
        }
      } else {  // just forward box_it to the next blob
        blobindex += blobcount;
        box_it.forward();
      }
    } while (!box_it.at_first());  // until all done

    if (real_row != NULL) delete real_row;
  }
  row->set_num_repeated_sets(num_repeated_sets);
}
