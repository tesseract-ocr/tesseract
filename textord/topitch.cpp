/**********************************************************************
 * File:        topitch.cpp  (Formerly to_pitch.c)
 * Description: Code to determine fixed pitchness and the pitch if fixed.
 * Author:		Ray Smith
 * Created:		Tue Aug 24 16:57:29 BST 1993
 *
 * (C) Copyright 1993, Hewlett-Packard Ltd.
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

#ifdef __UNIX__
#include          <assert.h>
#endif
#include          "stderr.h"
#include          "blobbox.h"
#include          "statistc.h"
#include          "drawtord.h"
#include          "makerow.h"
#include          "pitsync1.h"
#include          "pithsync.h"
#include          "tovars.h"
#include          "wordseg.h"
#include          "topitch.h"
#include          "helpers.h"

// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif

#define EXTERN

EXTERN BOOL_VAR (textord_all_prop, FALSE, "All doc is proportial text");
EXTERN BOOL_VAR (textord_debug_pitch_test, FALSE,
"Debug on fixed pitch test");
EXTERN BOOL_VAR (textord_disable_pitch_test, FALSE,
"Turn off dp fixed pitch algorithm");
EXTERN BOOL_VAR (textord_fast_pitch_test, FALSE,
"Do even faster pitch algorithm");
EXTERN BOOL_VAR (textord_debug_pitch_metric, FALSE,
"Write full metric stuff");
EXTERN BOOL_VAR (textord_show_row_cuts, FALSE, "Draw row-level cuts");
EXTERN BOOL_VAR (textord_show_page_cuts, FALSE, "Draw page-level cuts");
EXTERN BOOL_VAR (textord_pitch_cheat, FALSE,
"Use correct answer for fixed/prop");
EXTERN BOOL_VAR (textord_blockndoc_fixed, FALSE,
"Attempt whole doc/block fixed pitch");
EXTERN double_VAR (textord_projection_scale, 0.200, "Ding rate for mid-cuts");
EXTERN double_VAR (textord_balance_factor, 1.0,
"Ding rate for unbalanced char cells");

#define FIXED_WIDTH_MULTIPLE  5
#define BLOCK_STATS_CLUSTERS  10
#define MAX_ALLOWED_PITCH 100    //max pixel pitch.

/**********************************************************************
 * compute_fixed_pitch
 *
 * Decide whether each row is fixed pitch individually.
 * Correlate definite and uncertain results to obtain an individual
 * result for each row in the TO_ROW class.
 **********************************************************************/

void compute_fixed_pitch(ICOORD page_tr,              // top right
                         TO_BLOCK_LIST *port_blocks,  // input list
                         float gradient,              // page skew
                         FCOORD rotation,             // for drawing
                         BOOL8 testing_on) {          // correct orientation
  TO_BLOCK_IT block_it;          //iterator
  TO_BLOCK *block;               //current block;
  TO_ROW_IT row_it;              //row iterator
  TO_ROW *row;                   //current row
  int block_index;               //block number
  int row_index;                 //row number

#ifndef GRAPHICS_DISABLED
  if (textord_show_initial_words && testing_on) {
    if (to_win == NULL)
      create_to_win(page_tr);
  }
#endif

  block_it.set_to_list (port_blocks);
  block_index = 1;
  for (block_it.mark_cycle_pt (); !block_it.cycled_list ();
  block_it.forward ()) {
    block = block_it.data ();
    compute_block_pitch(block, rotation, block_index, testing_on);
    block_index++;
  }

  if (!try_doc_fixed (page_tr, port_blocks, gradient)) {
    block_index = 1;
    for (block_it.mark_cycle_pt (); !block_it.cycled_list ();
    block_it.forward ()) {
      block = block_it.data ();
      if (!try_block_fixed (block, block_index))
        try_rows_fixed(block, block_index, testing_on);
      block_index++;
    }
  }

  block_index = 1;
  for (block_it.mark_cycle_pt(); !block_it.cycled_list();
       block_it.forward()) {
    block = block_it.data ();
    POLY_BLOCK* pb = block->block->poly_block();
    if (pb != NULL && !pb->IsText()) continue;  // Non-text doesn't exist!
    row_it.set_to_list (block->get_rows ());
    row_index = 1;
    for (row_it.mark_cycle_pt (); !row_it.cycled_list (); row_it.forward ()) {
      row = row_it.data ();
      fix_row_pitch(row, block, port_blocks, row_index, block_index);
      row_index++;
    }
    block_index++;
  }
#ifndef GRAPHICS_DISABLED
  if (textord_show_initial_words && testing_on) {
    ScrollView::Update();
  }
#endif
}


/**********************************************************************
 * fix_row_pitch
 *
 * Get a pitch_decision for this row by voting among similar rows in the
 * block, then similar rows over all the page, or any other rows at all.
 **********************************************************************/

void fix_row_pitch(TO_ROW *bad_row,        // row to fix
                   TO_BLOCK *bad_block,    // block of bad_row
                   TO_BLOCK_LIST *blocks,  // blocks to scan
                   inT32 row_target,       // number of row
                   inT32 block_target) {   // number of block
  inT16 mid_cuts;
  int block_votes;               //votes in block
  int like_votes;                //votes over page
  int other_votes;               //votes of unlike blocks
  int block_index;               //number of block
  int row_index;                 //number of row
  int maxwidth;                  //max pitch
  TO_BLOCK_IT block_it = blocks; //block iterator
  TO_ROW_IT row_it;
  TO_BLOCK *block;               //current block
  TO_ROW *row;                   //current row
  float sp_sd;                   //space deviation
  STATS block_stats;             //pitches in block
  STATS like_stats;              //pitches in page

  block_votes = like_votes = other_votes = 0;
  maxwidth = (inT32) ceil (bad_row->xheight * textord_words_maxspace);
  if (bad_row->pitch_decision != PITCH_DEF_FIXED
  && bad_row->pitch_decision != PITCH_DEF_PROP) {
    block_stats.set_range (0, maxwidth);
    like_stats.set_range (0, maxwidth);
    block_index = 1;
    for (block_it.mark_cycle_pt(); !block_it.cycled_list();
         block_it.forward()) {
      block = block_it.data();
      POLY_BLOCK* pb = block->block->poly_block();
      if (pb != NULL && !pb->IsText()) continue;  // Non text doesn't exist!
      row_index = 1;
      row_it.set_to_list (block->get_rows ());
      for (row_it.mark_cycle_pt (); !row_it.cycled_list ();
      row_it.forward ()) {
        row = row_it.data ();
        if ((bad_row->all_caps
          && row->xheight + row->ascrise
          <
          (bad_row->xheight + bad_row->ascrise) * (1 +
          textord_pitch_rowsimilarity)
          && row->xheight + row->ascrise >
          (bad_row->xheight + bad_row->ascrise) * (1 -
          textord_pitch_rowsimilarity))
          || (!bad_row->all_caps
          && row->xheight <
          bad_row->xheight * (1 + textord_pitch_rowsimilarity)
          && row->xheight >
        bad_row->xheight * (1 - textord_pitch_rowsimilarity))) {
          if (block_index == block_target) {
            if (row->pitch_decision == PITCH_DEF_FIXED) {
              block_votes += textord_words_veto_power;
              block_stats.add ((inT32) row->fixed_pitch,
                textord_words_veto_power);
            }
            else if (row->pitch_decision == PITCH_MAYBE_FIXED
            || row->pitch_decision == PITCH_CORR_FIXED) {
              block_votes++;
              block_stats.add ((inT32) row->fixed_pitch, 1);
            }
            else if (row->pitch_decision == PITCH_DEF_PROP)
              block_votes -= textord_words_veto_power;
            else if (row->pitch_decision == PITCH_MAYBE_PROP
              || row->pitch_decision == PITCH_CORR_PROP)
              block_votes--;
          }
          else {
            if (row->pitch_decision == PITCH_DEF_FIXED) {
              like_votes += textord_words_veto_power;
              like_stats.add ((inT32) row->fixed_pitch,
                textord_words_veto_power);
            }
            else if (row->pitch_decision == PITCH_MAYBE_FIXED
            || row->pitch_decision == PITCH_CORR_FIXED) {
              like_votes++;
              like_stats.add ((inT32) row->fixed_pitch, 1);
            }
            else if (row->pitch_decision == PITCH_DEF_PROP)
              like_votes -= textord_words_veto_power;
            else if (row->pitch_decision == PITCH_MAYBE_PROP
              || row->pitch_decision == PITCH_CORR_PROP)
              like_votes--;
          }
        }
        else {
          if (row->pitch_decision == PITCH_DEF_FIXED)
            other_votes += textord_words_veto_power;
          else if (row->pitch_decision == PITCH_MAYBE_FIXED
            || row->pitch_decision == PITCH_CORR_FIXED)
            other_votes++;
          else if (row->pitch_decision == PITCH_DEF_PROP)
            other_votes -= textord_words_veto_power;
          else if (row->pitch_decision == PITCH_MAYBE_PROP
            || row->pitch_decision == PITCH_CORR_PROP)
            other_votes--;
        }
        row_index++;
      }
      block_index++;
    }
    if (block_votes > textord_words_veto_power) {
      bad_row->fixed_pitch = block_stats.ile (0.5);
      bad_row->pitch_decision = PITCH_CORR_FIXED;
    }
    else if (block_votes <= textord_words_veto_power && like_votes > 0) {
      bad_row->fixed_pitch = like_stats.ile (0.5);
      bad_row->pitch_decision = PITCH_CORR_FIXED;
    }
    else {
      bad_row->pitch_decision = PITCH_CORR_PROP;
      if (block_votes == 0 && like_votes == 0 && other_votes > 0
        && (textord_debug_pitch_test || textord_debug_pitch_metric))
        tprintf
          ("Warning:row %d of block %d set prop with no like rows against trend\n",
          row_target, block_target);
    }
  }
  if (textord_debug_pitch_metric) {
    tprintf(":b_votes=%d:l_votes=%d:o_votes=%d",
            block_votes, like_votes, other_votes);
    tprintf("x=%g:asc=%g\n", bad_row->xheight, bad_row->ascrise);
  }
  if (bad_row->pitch_decision == PITCH_CORR_FIXED) {
    if (bad_row->fixed_pitch < textord_min_xheight) {
      if (block_votes > 0)
        bad_row->fixed_pitch = block_stats.ile (0.5);
      else if (block_votes == 0 && like_votes > 0)
        bad_row->fixed_pitch = like_stats.ile (0.5);
      else {
        tprintf
          ("Warning:guessing pitch as xheight on row %d, block %d\n",
          row_target, block_target);
        bad_row->fixed_pitch = bad_row->xheight;
      }
    }
    if (bad_row->fixed_pitch < textord_min_xheight)
      bad_row->fixed_pitch = (float) textord_min_xheight;
    bad_row->kern_size = bad_row->fixed_pitch / 4;
    bad_row->min_space = (inT32) (bad_row->fixed_pitch * 0.6);
    bad_row->max_nonspace = (inT32) (bad_row->fixed_pitch * 0.4);
    bad_row->space_threshold =
      (bad_row->min_space + bad_row->max_nonspace) / 2;
    bad_row->space_size = bad_row->fixed_pitch;
    if (bad_row->char_cells.empty ())
      tune_row_pitch (bad_row, &bad_row->projection,
        bad_row->projection_left, bad_row->projection_right,
        (bad_row->fixed_pitch +
        bad_row->max_nonspace * 3) / 4, bad_row->fixed_pitch,
        sp_sd, mid_cuts, &bad_row->char_cells, FALSE);
  }
  else if (bad_row->pitch_decision == PITCH_CORR_PROP
  || bad_row->pitch_decision == PITCH_DEF_PROP) {
    bad_row->fixed_pitch = 0.0f;
    bad_row->char_cells.clear ();
  }
}


/**********************************************************************
 * compute_block_pitch
 *
 * Decide whether each block is fixed pitch individually.
 **********************************************************************/

void compute_block_pitch(TO_BLOCK *block,     // input list
                         FCOORD rotation,     // for drawing
                         inT32 block_index,   // block number
                         BOOL8 testing_on) {  // correct orientation
   TBOX block_box;                 //bounding box

  block_box = block->block->bounding_box ();
  if (testing_on && textord_debug_pitch_test) {
    tprintf ("Block %d at (%d,%d)->(%d,%d)\n",
      block_index,
      block_box.left (), block_box.bottom (),
      block_box.right (), block_box.top ());
  }
  block->min_space = (inT32) floor (block->xheight
    * textord_words_default_minspace);
  block->max_nonspace = (inT32) ceil (block->xheight
    * textord_words_default_nonspace);
  block->fixed_pitch = 0.0f;
  block->space_size = (float) block->min_space;
  block->kern_size = (float) block->max_nonspace;
  block->pr_nonsp = block->xheight * words_default_prop_nonspace;
  block->pr_space = block->pr_nonsp * textord_spacesize_ratioprop;
  if (!block->get_rows ()->empty ()) {
    ASSERT_HOST (block->xheight > 0);
    find_repeated_chars(block, textord_show_initial_words && testing_on);
#ifndef GRAPHICS_DISABLED
    if (textord_show_initial_words && testing_on)
      //overlap_picture_ops(TRUE);
      ScrollView::Update();
#endif
    compute_rows_pitch(block,
                       block_index,
                       textord_debug_pitch_test &&testing_on);
  }
}


/**********************************************************************
 * compute_rows_pitch
 *
 * Decide whether each row is fixed pitch individually.
 **********************************************************************/

BOOL8 compute_rows_pitch(                    //find line stats
                         TO_BLOCK *block,    //block to do
                         inT32 block_index,  //block number
                         BOOL8 testing_on    //correct orientation
                        ) {
  inT32 maxwidth;                //of spaces
  TO_ROW *row;                   //current row
  inT32 row_index;               //row number.
  float lower, upper;            //cluster thresholds
  TO_ROW_IT row_it = block->get_rows ();

  row_index = 1;
  for (row_it.mark_cycle_pt (); !row_it.cycled_list (); row_it.forward ()) {
    row = row_it.data ();
    ASSERT_HOST (row->xheight > 0);
    row->compute_vertical_projection ();
    maxwidth = (inT32) ceil (row->xheight * textord_words_maxspace);
    if (row_pitch_stats (row, maxwidth, testing_on)
      && find_row_pitch (row, maxwidth,
      textord_dotmatrix_gap + 1, block, block_index,
    row_index, testing_on)) {
      if (row->fixed_pitch == 0) {
        lower = row->pr_nonsp;
        upper = row->pr_space;
        row->space_size = upper;
        row->kern_size = lower;
      }
    }
    else {
      row->fixed_pitch = 0.0f;   //insufficient data
      row->pitch_decision = PITCH_DUNNO;
    }
    row_index++;
  }
  return FALSE;
}


/**********************************************************************
 * try_doc_fixed
 *
 * Attempt to call the entire document fixed pitch.
 **********************************************************************/

BOOL8 try_doc_fixed(                             //determine pitch
                    ICOORD page_tr,              //top right
                    TO_BLOCK_LIST *port_blocks,  //input list
                    float gradient               //page skew
                   ) {
  inT16 master_x;                //uniform shifts
  inT16 pitch;                   //median pitch.
  int x;                         //profile coord
  int prop_blocks;               //correct counts
  int fixed_blocks;
  int total_row_count;           //total in page
                                 //iterator
  TO_BLOCK_IT block_it = port_blocks;
  TO_BLOCK *block;               //current block;
  TO_ROW_IT row_it;              //row iterator
  TO_ROW *row;                   //current row
  inT16 projection_left;         //edges
  inT16 projection_right;
  inT16 row_left;                //edges of row
  inT16 row_right;
  ICOORDELT_LIST *master_cells;  //cells for page
  float master_y;                //uniform shifts
  float shift_factor;            //page skew correction
  float row_shift;               //shift for row
  float final_pitch;             //output pitch
  float row_y;                   //baseline
  STATS projection;              //entire page
  STATS pitches (0, MAX_ALLOWED_PITCH);
  //for median
  float sp_sd;                   //space sd
  inT16 mid_cuts;                //no of cheap cuts
  float pitch_sd;                //sync rating

  if (block_it.empty ()
    //      || block_it.data()==block_it.data_relative(1)
    || !textord_blockndoc_fixed)
    return FALSE;
  shift_factor = gradient / (gradient * gradient + 1);
  row_it.set_to_list (block_it.data ()->get_rows ());
  master_x = row_it.data ()->projection_left;
  master_y = row_it.data ()->baseline.y (master_x);
  projection_left = MAX_INT16;
  projection_right = -MAX_INT16;
  prop_blocks = 0;
  fixed_blocks = 0;
  total_row_count = 0;

  for (block_it.mark_cycle_pt (); !block_it.cycled_list ();
  block_it.forward ()) {
    block = block_it.data ();
    row_it.set_to_list (block->get_rows ());
    for (row_it.mark_cycle_pt (); !row_it.cycled_list (); row_it.forward ()) {
      row = row_it.data ();
      total_row_count++;
      if (row->fixed_pitch > 0)
        pitches.add ((inT32) (row->fixed_pitch), 1);
      //find median
      row_y = row->baseline.y (master_x);
      row_left =
        (inT16) (row->projection_left -
        shift_factor * (master_y - row_y));
      row_right =
        (inT16) (row->projection_right -
        shift_factor * (master_y - row_y));
      if (row_left < projection_left)
        projection_left = row_left;
      if (row_right > projection_right)
        projection_right = row_right;
    }
  }
  if (pitches.get_total () == 0)
    return FALSE;
  projection.set_range (projection_left, projection_right);

  for (block_it.mark_cycle_pt (); !block_it.cycled_list ();
  block_it.forward ()) {
    block = block_it.data ();
    row_it.set_to_list (block->get_rows ());
    for (row_it.mark_cycle_pt (); !row_it.cycled_list (); row_it.forward ()) {
      row = row_it.data ();
      row_y = row->baseline.y (master_x);
      row_left =
        (inT16) (row->projection_left -
        shift_factor * (master_y - row_y));
      for (x = row->projection_left; x < row->projection_right;
      x++, row_left++) {
        projection.add (row_left, row->projection.pile_count (x));
      }
    }
  }

  row_it.set_to_list (block_it.data ()->get_rows ());
  row = row_it.data ();
#ifndef GRAPHICS_DISABLED
  if (textord_show_page_cuts && to_win != NULL)
    projection.plot (to_win, projection_left,
      row->intercept (), 1.0f, -1.0f, ScrollView::CORAL);
#endif
  final_pitch = pitches.ile (0.5);
  pitch = (inT16) final_pitch;
  pitch_sd =
    tune_row_pitch (row, &projection, projection_left, projection_right,
    pitch * 0.75, final_pitch, sp_sd, mid_cuts,
    &row->char_cells, FALSE);

  if (textord_debug_pitch_metric)
    tprintf
      ("try_doc:props=%d:fixed=%d:pitch=%d:final_pitch=%g:pitch_sd=%g:sp_sd=%g:sd/trc=%g:sd/p=%g:sd/trc/p=%g\n",
      prop_blocks, fixed_blocks, pitch, final_pitch, pitch_sd, sp_sd,
      pitch_sd / total_row_count, pitch_sd / pitch,
      pitch_sd / total_row_count / pitch);

#ifndef GRAPHICS_DISABLED
  if (textord_show_page_cuts && to_win != NULL) {
    master_cells = &row->char_cells;
    for (block_it.mark_cycle_pt (); !block_it.cycled_list ();
    block_it.forward ()) {
      block = block_it.data ();
      row_it.set_to_list (block->get_rows ());
      for (row_it.mark_cycle_pt (); !row_it.cycled_list ();
      row_it.forward ()) {
        row = row_it.data ();
        row_y = row->baseline.y (master_x);
        row_shift = shift_factor * (master_y - row_y);
        plot_row_cells(to_win, ScrollView::GOLDENROD, row, row_shift, master_cells);
      }
    }
  }
#endif
  row->char_cells.clear ();
  return FALSE;
}


/**********************************************************************
 * try_block_fixed
 *
 * Try to call the entire block fixed.
 **********************************************************************/

BOOL8 try_block_fixed(                   //find line stats
                      TO_BLOCK *block,   //block to do
                      inT32 block_index  //block number
                     ) {
  return FALSE;
}


/**********************************************************************
 * try_rows_fixed
 *
 * Decide whether each row is fixed pitch individually.
 **********************************************************************/

BOOL8 try_rows_fixed(                    //find line stats
                     TO_BLOCK *block,    //block to do
                     inT32 block_index,  //block number
                     BOOL8 testing_on    //correct orientation
                    ) {
  TO_ROW *row;                   //current row
  inT32 row_index;               //row number.
  inT32 def_fixed = 0;           //counters
  inT32 def_prop = 0;
  inT32 maybe_fixed = 0;
  inT32 maybe_prop = 0;
  inT32 dunno = 0;
  inT32 corr_fixed = 0;
  inT32 corr_prop = 0;
  float lower, upper;            //cluster thresholds
  TO_ROW_IT row_it = block->get_rows ();

  row_index = 1;
  for (row_it.mark_cycle_pt (); !row_it.cycled_list (); row_it.forward ()) {
    row = row_it.data ();
    ASSERT_HOST (row->xheight > 0);
    if (row->fixed_pitch > 0 &&
        fixed_pitch_row(row, block->block, block_index)) {
      if (row->fixed_pitch == 0) {
        lower = row->pr_nonsp;
        upper = row->pr_space;
        row->space_size = upper;
        row->kern_size = lower;
      }
    }
    row_index++;
  }
  count_block_votes(block,
                    def_fixed,
                    def_prop,
                    maybe_fixed,
                    maybe_prop,
                    corr_fixed,
                    corr_prop,
                    dunno);
  if (testing_on
    && (textord_debug_pitch_test
  || textord_blocksall_prop || textord_blocksall_fixed)) {
    tprintf ("Initially:");
    print_block_counts(block, block_index);
  }
  if (def_fixed > def_prop * textord_words_veto_power)
    block->pitch_decision = PITCH_DEF_FIXED;
  else if (def_prop > def_fixed * textord_words_veto_power)
    block->pitch_decision = PITCH_DEF_PROP;
  else if (def_fixed > 0 || def_prop > 0)
    block->pitch_decision = PITCH_DUNNO;
  else if (maybe_fixed > maybe_prop * textord_words_veto_power)
    block->pitch_decision = PITCH_MAYBE_FIXED;
  else if (maybe_prop > maybe_fixed * textord_words_veto_power)
    block->pitch_decision = PITCH_MAYBE_PROP;
  else
    block->pitch_decision = PITCH_DUNNO;
  return FALSE;
}


/**********************************************************************
 * print_block_counts
 *
 * Count up how many rows have what decision and print the results.
 **********************************************************************/

void print_block_counts(                   //find line stats
                        TO_BLOCK *block,   //block to do
                        inT32 block_index  //block number
                       ) {
  inT32 def_fixed = 0;           //counters
  inT32 def_prop = 0;
  inT32 maybe_fixed = 0;
  inT32 maybe_prop = 0;
  inT32 dunno = 0;
  inT32 corr_fixed = 0;
  inT32 corr_prop = 0;

  count_block_votes(block,
                    def_fixed,
                    def_prop,
                    maybe_fixed,
                    maybe_prop,
                    corr_fixed,
                    corr_prop,
                    dunno);
  tprintf ("Block %d has (%d,%d,%d)",
    block_index, def_fixed, maybe_fixed, corr_fixed);
  if (textord_blocksall_prop && (def_fixed || maybe_fixed || corr_fixed))
    tprintf (" (Wrongly)");
  tprintf (" fixed, (%d,%d,%d)", def_prop, maybe_prop, corr_prop);
  if (textord_blocksall_fixed && (def_prop || maybe_prop || corr_prop))
    tprintf (" (Wrongly)");
  tprintf (" prop, %d dunno\n", dunno);
}


/**********************************************************************
 * count_block_votes
 *
 * Count the number of rows in the block with each kind of pitch_decision.
 **********************************************************************/

void count_block_votes(                   //find line stats
                       TO_BLOCK *block,   //block to do
                       inT32 &def_fixed,  //add to counts
                       inT32 &def_prop,
                       inT32 &maybe_fixed,
                       inT32 &maybe_prop,
                       inT32 &corr_fixed,
                       inT32 &corr_prop,
                       inT32 &dunno) {
  TO_ROW *row;                   //current row
  TO_ROW_IT row_it = block->get_rows ();

  for (row_it.mark_cycle_pt (); !row_it.cycled_list (); row_it.forward ()) {
    row = row_it.data ();
    switch (row->pitch_decision) {
      case PITCH_DUNNO:
        dunno++;
        break;
      case PITCH_DEF_PROP:
        def_prop++;
        break;
      case PITCH_MAYBE_PROP:
        maybe_prop++;
        break;
      case PITCH_DEF_FIXED:
        def_fixed++;
        break;
      case PITCH_MAYBE_FIXED:
        maybe_fixed++;
        break;
      case PITCH_CORR_PROP:
        corr_prop++;
        break;
      case PITCH_CORR_FIXED:
        corr_fixed++;
        break;
    }
  }
}


/**********************************************************************
 * row_pitch_stats
 *
 * Decide whether each row is fixed pitch individually.
 **********************************************************************/

BOOL8 row_pitch_stats(                  //find line stats
                      TO_ROW *row,      //current row
                      inT32 maxwidth,   //of spaces
                      BOOL8 testing_on  //correct orientation
                     ) {
  BLOBNBOX *blob;                //current blob
  int gap_index;                 //current gap
  inT32 prev_x;                  //end of prev blob
  inT32 cluster_count;           //no of clusters
  inT32 prev_count;              //of clusters
  inT32 smooth_factor;           //for smoothing stats
  TBOX blob_box;                  //bounding box
  float lower, upper;            //cluster thresholds
                                 //gap sizes
  float gaps[BLOCK_STATS_CLUSTERS];
                                 //blobs
  BLOBNBOX_IT blob_it = row->blob_list ();
  STATS gap_stats (0, maxwidth);
  STATS cluster_stats[BLOCK_STATS_CLUSTERS + 1];
  //clusters

  smooth_factor =
    (inT32) (row->xheight * textord_wordstats_smooth_factor + 1.5);
  if (!blob_it.empty ()) {
    prev_x = blob_it.data ()->bounding_box ().right ();
    blob_it.forward ();
    while (!blob_it.at_first ()) {
      blob = blob_it.data ();
      if (!blob->joined_to_prev ()) {
        blob_box = blob->bounding_box ();
        if (blob_box.left () - prev_x < maxwidth)
          gap_stats.add (blob_box.left () - prev_x, 1);
        prev_x = blob_box.right ();
      }
      blob_it.forward ();
    }
  }
  if (gap_stats.get_total () == 0) {
    return FALSE;
  }
  cluster_count = 0;
  lower = row->xheight * words_initial_lower;
  upper = row->xheight * words_initial_upper;
  gap_stats.smooth (smooth_factor);
  do {
    prev_count = cluster_count;
    cluster_count = gap_stats.cluster (lower, upper,
      textord_spacesize_ratioprop,
      BLOCK_STATS_CLUSTERS, cluster_stats);
  }
  while (cluster_count > prev_count && cluster_count < BLOCK_STATS_CLUSTERS);
  if (cluster_count < 1) {
    return FALSE;
  }
  for (gap_index = 0; gap_index < cluster_count; gap_index++)
    gaps[gap_index] = cluster_stats[gap_index + 1].ile (0.5);
  //get medians
  if (testing_on) {
    tprintf ("cluster_count=%d:", cluster_count);
    for (gap_index = 0; gap_index < cluster_count; gap_index++)
      tprintf (" %g(%d)", gaps[gap_index],
        cluster_stats[gap_index + 1].get_total ());
    tprintf ("\n");
  }
  qsort (gaps, cluster_count, sizeof (float), sort_floats);

  //Try to find proportional non-space and space for row.
  lower = row->xheight * words_default_prop_nonspace;
  upper = row->xheight * textord_words_min_minspace;
  for (gap_index = 0; gap_index < cluster_count
    && gaps[gap_index] < lower; gap_index++);
  if (gap_index == 0) {
    if (testing_on)
      tprintf ("No clusters below nonspace threshold!!\n");
    if (cluster_count > 1) {
      row->pr_nonsp = gaps[0];
      row->pr_space = gaps[1];
    }
    else {
      row->pr_nonsp = lower;
      row->pr_space = gaps[0];
    }
  }
  else {
    row->pr_nonsp = gaps[gap_index - 1];
    while (gap_index < cluster_count && gaps[gap_index] < upper)
      gap_index++;
    if (gap_index == cluster_count) {
      if (testing_on)
        tprintf ("No clusters above nonspace threshold!!\n");
      row->pr_space = lower * textord_spacesize_ratioprop;
    }
    else
      row->pr_space = gaps[gap_index];
  }

  //Now try to find the fixed pitch space and non-space.
  upper = row->xheight * words_default_fixed_space;
  for (gap_index = 0; gap_index < cluster_count
    && gaps[gap_index] < upper; gap_index++);
  if (gap_index == 0) {
    if (testing_on)
      tprintf ("No clusters below space threshold!!\n");
    row->fp_nonsp = upper;
    row->fp_space = gaps[0];
  }
  else {
    row->fp_nonsp = gaps[gap_index - 1];
    if (gap_index == cluster_count) {
      if (testing_on)
        tprintf ("No clusters above space threshold!!\n");
      row->fp_space = row->xheight;
    }
    else
      row->fp_space = gaps[gap_index];
  }
  if (testing_on) {
    tprintf
      ("Initial estimates:pr_nonsp=%g, pr_space=%g, fp_nonsp=%g, fp_space=%g\n",
      row->pr_nonsp, row->pr_space, row->fp_nonsp, row->fp_space);
  }
  return TRUE;                   //computed some stats
}


/**********************************************************************
 * find_row_pitch
 *
 * Check to see if this row could be fixed pitch using the given spacings.
 * Blobs with gaps smaller than the lower threshold are assumed to be one.
 * The larger threshold is the word gap threshold.
 **********************************************************************/

BOOL8 find_row_pitch(                    //find lines
                     TO_ROW *row,        //row to do
                     inT32 maxwidth,     //max permitted space
                     inT32 dm_gap,       //ignorable gaps
                     TO_BLOCK *block,    //block of row
                     inT32 block_index,  //block_number
                     inT32 row_index,    //number of row
                     BOOL8 testing_on    //correct orientation
                    ) {
  BOOL8 used_dm_model;           //looks lik dot matrix
  float min_space;               //estimate threshold
  float non_space;               //gap size
  float gap_iqr;                 //interquartile range
  float pitch_iqr;
  float dm_gap_iqr;              //interquartile range
  float dm_pitch_iqr;
  float dm_pitch;                //pitch with dm on
  float pitch;                   //revised estimate
  float initial_pitch;           //guess at pitch
  STATS gap_stats (0, maxwidth);
                                 //centre-centre
  STATS pitch_stats (0, maxwidth);

  row->fixed_pitch = 0.0f;
  initial_pitch = row->fp_space;
  if (initial_pitch > row->xheight * (1 + words_default_fixed_limit))
    initial_pitch = row->xheight;//keep pitch decent
  non_space = row->fp_nonsp;
  if (non_space > initial_pitch)
    non_space = initial_pitch;
  min_space = (initial_pitch + non_space) / 2;

  if (!count_pitch_stats (row, &gap_stats, &pitch_stats,
  initial_pitch, min_space, TRUE, FALSE, dm_gap)) {
    dm_gap_iqr = 0.0001;
    dm_pitch_iqr = maxwidth * 2.0f;
    dm_pitch = initial_pitch;
  }
  else {
    dm_gap_iqr = gap_stats.ile (0.75) - gap_stats.ile (0.25);
    dm_pitch_iqr = pitch_stats.ile (0.75) - pitch_stats.ile (0.25);
    dm_pitch = pitch_stats.ile (0.5);
  }
  gap_stats.clear ();
  pitch_stats.clear ();
  if (!count_pitch_stats (row, &gap_stats, &pitch_stats,
  initial_pitch, min_space, TRUE, FALSE, 0)) {
    gap_iqr = 0.0001;
    pitch_iqr = maxwidth * 3.0f;
  }
  else {
    gap_iqr = gap_stats.ile (0.75) - gap_stats.ile (0.25);
    pitch_iqr = pitch_stats.ile (0.75) - pitch_stats.ile (0.25);
    if (testing_on)
      tprintf
        ("First fp iteration:initial_pitch=%g, gap_iqr=%g, pitch_iqr=%g, pitch=%g\n",
        initial_pitch, gap_iqr, pitch_iqr, pitch_stats.ile (0.5));
    initial_pitch = pitch_stats.ile (0.5);
    if (min_space > initial_pitch
      && count_pitch_stats (row, &gap_stats, &pitch_stats,
    initial_pitch, initial_pitch, TRUE, FALSE, 0)) {
      min_space = initial_pitch;
      gap_iqr = gap_stats.ile (0.75) - gap_stats.ile (0.25);
      pitch_iqr = pitch_stats.ile (0.75) - pitch_stats.ile (0.25);
      if (testing_on)
        tprintf
          ("Revised fp iteration:initial_pitch=%g, gap_iqr=%g, pitch_iqr=%g, pitch=%g\n",
          initial_pitch, gap_iqr, pitch_iqr, pitch_stats.ile (0.5));
      initial_pitch = pitch_stats.ile (0.5);
    }
  }
  if (textord_debug_pitch_metric)
    tprintf("Blk=%d:Row=%d:%c:p_iqr=%g:g_iqr=%g:dm_p_iqr=%g:dm_g_iqr=%g:%c:",
            block_index, row_index, 'X',
            pitch_iqr, gap_iqr, dm_pitch_iqr, dm_gap_iqr,
            pitch_iqr > maxwidth && dm_pitch_iqr > maxwidth ? 'D' :
              (pitch_iqr * dm_gap_iqr <= dm_pitch_iqr * gap_iqr ? 'S' : 'M'));
  if (pitch_iqr > maxwidth && dm_pitch_iqr > maxwidth) {
    row->pitch_decision = PITCH_DUNNO;
    if (textord_debug_pitch_metric)
      tprintf ("\n");
    return FALSE;                //insufficient data
  }
  if (pitch_iqr * dm_gap_iqr <= dm_pitch_iqr * gap_iqr) {
    if (testing_on)
      tprintf
        ("Choosing non dm version:pitch_iqr=%g, gap_iqr=%g, dm_pitch_iqr=%g, dm_gap_iqr=%g\n",
        pitch_iqr, gap_iqr, dm_pitch_iqr, dm_gap_iqr);
    gap_iqr = gap_stats.ile (0.75) - gap_stats.ile (0.25);
    pitch_iqr = pitch_stats.ile (0.75) - pitch_stats.ile (0.25);
    pitch = pitch_stats.ile (0.5);
    used_dm_model = FALSE;
  }
  else {
    if (testing_on)
      tprintf
        ("Choosing dm version:pitch_iqr=%g, gap_iqr=%g, dm_pitch_iqr=%g, dm_gap_iqr=%g\n",
        pitch_iqr, gap_iqr, dm_pitch_iqr, dm_gap_iqr);
    gap_iqr = dm_gap_iqr;
    pitch_iqr = dm_pitch_iqr;
    pitch = dm_pitch;
    used_dm_model = TRUE;
  }
  if (textord_debug_pitch_metric) {
    tprintf ("rev_p_iqr=%g:rev_g_iqr=%g:pitch=%g:",
      pitch_iqr, gap_iqr, pitch);
    tprintf ("p_iqr/g=%g:p_iqr/x=%g:iqr_res=%c:",
      pitch_iqr / gap_iqr, pitch_iqr / block->xheight,
      pitch_iqr < gap_iqr * textord_fpiqr_ratio
      && pitch_iqr < block->xheight * textord_max_pitch_iqr
      && pitch < block->xheight * textord_words_default_maxspace
      ? 'F' : 'P');
  }
  if (pitch_iqr < gap_iqr * textord_fpiqr_ratio
    && pitch_iqr < block->xheight * textord_max_pitch_iqr
    && pitch < block->xheight * textord_words_default_maxspace)
    row->pitch_decision = PITCH_MAYBE_FIXED;
  else
    row->pitch_decision = PITCH_MAYBE_PROP;
  row->fixed_pitch = pitch;
  row->kern_size = gap_stats.ile (0.5);
  row->min_space = (inT32) (row->fixed_pitch + non_space) / 2;
  if (row->min_space > row->fixed_pitch)
    row->min_space = (inT32) row->fixed_pitch;
  row->max_nonspace = row->min_space;
  row->space_size = row->fixed_pitch;
  row->space_threshold = (row->max_nonspace + row->min_space) / 2;
  row->used_dm_model = used_dm_model;
  return TRUE;
}


/**********************************************************************
 * fixed_pitch_row
 *
 * Check to see if this row could be fixed pitch using the given spacings.
 * Blobs with gaps smaller than the lower threshold are assumed to be one.
 * The larger threshold is the word gap threshold.
 **********************************************************************/

BOOL8 fixed_pitch_row(TO_ROW *row,       // row to do
                      BLOCK* block,
                      inT32 block_index  // block_number
                     ) {
  const char *res_string;        // pitch result
  inT16 mid_cuts;                // no of cheap cuts
  float non_space;               // gap size
  float pitch_sd;                // error on pitch
  float sp_sd = 0.0f;            // space sd

  non_space = row->fp_nonsp;
  if (non_space > row->fixed_pitch)
    non_space = row->fixed_pitch;
  POLY_BLOCK* pb = block != NULL ? block->poly_block() : NULL;
  if (textord_all_prop || (pb != NULL && !pb->IsText())) {
    // Set the decision to definitely proportional.
    pitch_sd = textord_words_def_prop * row->fixed_pitch;
    row->pitch_decision = PITCH_DEF_PROP;
  } else {
    pitch_sd = tune_row_pitch (row, &row->projection, row->projection_left,
                               row->projection_right,
                               (row->fixed_pitch + non_space * 3) / 4,
                               row->fixed_pitch, sp_sd, mid_cuts,
                               &row->char_cells,
                               block_index == textord_debug_block);
    if (pitch_sd < textord_words_pitchsd_threshold * row->fixed_pitch
      && ((pitsync_linear_version & 3) < 3
      || ((pitsync_linear_version & 3) >= 3 && (row->used_dm_model
      || sp_sd > 20
    || (pitch_sd == 0 && sp_sd > 10))))) {
      if (pitch_sd < textord_words_def_fixed * row->fixed_pitch
        && !row->all_caps
        && ((pitsync_linear_version & 3) < 3 || sp_sd > 20))
        row->pitch_decision = PITCH_DEF_FIXED;
      else
        row->pitch_decision = PITCH_MAYBE_FIXED;
    }
    else if ((pitsync_linear_version & 3) < 3
      || sp_sd > 20
      || mid_cuts > 0
      || pitch_sd >= textord_words_pitchsd_threshold * row->fixed_pitch) {
      if (pitch_sd < textord_words_def_prop * row->fixed_pitch)
        row->pitch_decision = PITCH_MAYBE_PROP;
      else
        row->pitch_decision = PITCH_DEF_PROP;
    }
    else
      row->pitch_decision = PITCH_DUNNO;
  }

  if (textord_debug_pitch_metric) {
    res_string = "??";
    switch (row->pitch_decision) {
      case PITCH_DEF_PROP:
        res_string = "DP";
        break;
      case PITCH_MAYBE_PROP:
        res_string = "MP";
        break;
      case PITCH_DEF_FIXED:
        res_string = "DF";
        break;
      case PITCH_MAYBE_FIXED:
        res_string = "MF";
        break;
      default:
        res_string = "??";
    }
    tprintf (":sd/p=%g:occ=%g:init_res=%s\n",
      pitch_sd / row->fixed_pitch, sp_sd, res_string);
  }
  return TRUE;
}


/**********************************************************************
 * count_pitch_stats
 *
 * Count up the gap and pitch stats on the block to see if it is fixed pitch.
 * Blobs with gaps smaller than the lower threshold are assumed to be one.
 * The larger threshold is the word gap threshold.
 * The return value indicates whether there were any decent values to use.
 **********************************************************************/

BOOL8 count_pitch_stats(                       //find lines
                        TO_ROW *row,           //row to do
                        STATS *gap_stats,      //blob gaps
                        STATS *pitch_stats,    //centre-centre stats
                        float initial_pitch,   //guess at pitch
                        float min_space,       //estimate space size
                        BOOL8 ignore_outsize,  //discard big objects
                        BOOL8 split_outsize,   //split big objects
                        inT32 dm_gap           //ignorable gaps
                       ) {
  BOOL8 prev_valid;              //not word broken
  BLOBNBOX *blob;                //current blob
                                 //blobs
  BLOBNBOX_IT blob_it = row->blob_list ();
  inT32 prev_right;              //end of prev blob
  inT32 prev_centre;             //centre of previous blob
  inT32 x_centre;                //centre of this blob
  inT32 blob_width;              //width of blob
  inT32 width_units;             //no of widths in blob
  float width;                   //blob width
  TBOX blob_box;                  //bounding box
  TBOX joined_box;                //of super blob

  gap_stats->clear ();
  pitch_stats->clear ();
  if (blob_it.empty ())
    return FALSE;
  prev_valid = FALSE;
  prev_centre = 0;
  prev_right = 0;                //stop complier warning
  joined_box = blob_it.data ()->bounding_box ();
  do {
    blob_it.forward ();
    blob = blob_it.data ();
    if (!blob->joined_to_prev ()) {
      blob_box = blob->bounding_box ();
      if ((blob_box.left () - joined_box.right () < dm_gap
        && !blob_it.at_first ())
        || blob->cblob() == NULL)
        joined_box += blob_box;  //merge blobs
      else {
        blob_width = joined_box.width ();
        if (split_outsize) {
          width_units =
            (inT32) floor ((float) blob_width / initial_pitch + 0.5);
          if (width_units < 1)
            width_units = 1;
          width_units--;
        }
        else if (ignore_outsize) {
          width = (float) blob_width / initial_pitch;
          width_units = width < 1 + words_default_fixed_limit
            && width > 1 - words_default_fixed_limit ? 0 : -1;
        }
        else
          width_units = 0;       //everything in
        x_centre = (inT32) (joined_box.left ()
          + (blob_width -
          width_units * initial_pitch) / 2);
        if (prev_valid && width_units >= 0) {
          //                                              if (width_units>0)
          //                                              {
          //                                                      tprintf("wu=%d, width=%d, xc=%d, adding %d\n",
          //                                                              width_units,blob_width,x_centre,x_centre-prev_centre);
          //                                              }
          gap_stats->add (joined_box.left () - prev_right, 1);
          pitch_stats->add (x_centre - prev_centre, 1);
        }
        prev_centre = (inT32) (x_centre + width_units * initial_pitch);
        prev_right = joined_box.right ();
        prev_valid = blob_box.left () - joined_box.right () < min_space;
        prev_valid = prev_valid && width_units >= 0;
        joined_box = blob_box;
      }
    }
  }
  while (!blob_it.at_first ());
  return gap_stats->get_total () >= 3;
}


/**********************************************************************
 * tune_row_pitch
 *
 * Use a dp algorithm to fit the character cells and return the sd of
 * the cell size over the row.
 **********************************************************************/

float tune_row_pitch(                             //find fp cells
                     TO_ROW *row,                 //row to do
                     STATS *projection,           //vertical projection
                     inT16 projection_left,       //edge of projection
                     inT16 projection_right,      //edge of projection
                     float space_size,            //size of blank
                     float &initial_pitch,        //guess at pitch
                     float &best_sp_sd,           //space sd
                     inT16 &best_mid_cuts,        //no of cheap cuts
                     ICOORDELT_LIST *best_cells,  //row cells
                     BOOL8 testing_on             //inidividual words
                    ) {
  int pitch_delta;               //offset pitch
  inT16 mid_cuts;                //cheap cuts
  float pitch_sd;                //current sd
  float best_sd;                 //best result
  float best_pitch;              //pitch for best result
  float initial_sd;              //starting error
  float sp_sd;                   //space sd
  ICOORDELT_LIST test_cells;     //row cells
  ICOORDELT_IT best_it;          //start of best list

  if (textord_fast_pitch_test)
    return tune_row_pitch2 (row, projection, projection_left,
      projection_right, space_size, initial_pitch,
      best_sp_sd,
    //space sd
      best_mid_cuts, best_cells, testing_on);
  if (textord_disable_pitch_test) {
    best_sp_sd = initial_pitch;
    return initial_pitch;
  }
  initial_sd =
    compute_pitch_sd(row,
                     projection,
                     projection_left,
                     projection_right,
                     space_size,
                     initial_pitch,
                     best_sp_sd,
                     best_mid_cuts,
                     best_cells,
                     testing_on);
  best_sd = initial_sd;
  best_pitch = initial_pitch;
  if (testing_on)
    tprintf ("tune_row_pitch:start pitch=%g, sd=%g\n", best_pitch, best_sd);
  for (pitch_delta = 1; pitch_delta <= textord_pitch_range; pitch_delta++) {
    pitch_sd =
      compute_pitch_sd (row, projection, projection_left, projection_right,
      space_size, initial_pitch + pitch_delta, sp_sd,
      mid_cuts, &test_cells, testing_on);
    if (testing_on)
      tprintf ("testing pitch at %g, sd=%g\n", initial_pitch + pitch_delta,
        pitch_sd);
    if (pitch_sd < best_sd) {
      best_sd = pitch_sd;
      best_mid_cuts = mid_cuts;
      best_sp_sd = sp_sd;
      best_pitch = initial_pitch + pitch_delta;
      best_cells->clear ();
      best_it.set_to_list (best_cells);
      best_it.add_list_after (&test_cells);
    }
    else
      test_cells.clear ();
    if (pitch_sd > initial_sd)
      break;                     //getting worse
  }
  for (pitch_delta = 1; pitch_delta <= textord_pitch_range; pitch_delta++) {
    pitch_sd =
      compute_pitch_sd (row, projection, projection_left, projection_right,
      space_size, initial_pitch - pitch_delta, sp_sd,
      mid_cuts, &test_cells, testing_on);
    if (testing_on)
      tprintf ("testing pitch at %g, sd=%g\n", initial_pitch - pitch_delta,
        pitch_sd);
    if (pitch_sd < best_sd) {
      best_sd = pitch_sd;
      best_mid_cuts = mid_cuts;
      best_sp_sd = sp_sd;
      best_pitch = initial_pitch - pitch_delta;
      best_cells->clear ();
      best_it.set_to_list (best_cells);
      best_it.add_list_after (&test_cells);
    }
    else
      test_cells.clear ();
    if (pitch_sd > initial_sd)
      break;
  }
  initial_pitch = best_pitch;

  if (textord_debug_pitch_metric)
    print_pitch_sd(row,
                   projection,
                   projection_left,
                   projection_right,
                   space_size,
                   best_pitch);

  return best_sd;
}


/**********************************************************************
 * tune_row_pitch
 *
 * Use a dp algorithm to fit the character cells and return the sd of
 * the cell size over the row.
 **********************************************************************/

float tune_row_pitch2(                             //find fp cells
                      TO_ROW *row,                 //row to do
                      STATS *projection,           //vertical projection
                      inT16 projection_left,       //edge of projection
                      inT16 projection_right,      //edge of projection
                      float space_size,            //size of blank
                      float &initial_pitch,        //guess at pitch
                      float &best_sp_sd,           //space sd
                      inT16 &best_mid_cuts,        //no of cheap cuts
                      ICOORDELT_LIST *best_cells,  //row cells
                      BOOL8 testing_on             //inidividual words
                     ) {
  int pitch_delta;               //offset pitch
  inT16 pixel;                   //pixel coord
  inT16 best_pixel;              //pixel coord
  inT16 best_delta;              //best pitch
  inT16 best_pitch;              //best pitch
  inT16 start;                   //of good range
  inT16 end;                     //of good range
  inT32 best_count;              //lowest sum
  float best_sd;                 //best result
  STATS *sum_proj;               //summed projection

  best_sp_sd = initial_pitch;

  if (textord_disable_pitch_test) {
    return initial_pitch;
  }
  sum_proj = new STATS[textord_pitch_range * 2 + 1];
  if (sum_proj == NULL)
    return initial_pitch;
  best_pitch = (inT32) initial_pitch;

  for (pitch_delta = -textord_pitch_range; pitch_delta <= textord_pitch_range;
    pitch_delta++)
  sum_proj[textord_pitch_range + pitch_delta].set_range (0,
      best_pitch +
      pitch_delta + 1);
  for (pixel = projection_left; pixel <= projection_right; pixel++) {
    for (pitch_delta = -textord_pitch_range;
      pitch_delta <= textord_pitch_range; pitch_delta++)
    sum_proj[textord_pitch_range +
        pitch_delta].add ((pixel - projection_left) % (best_pitch +
        pitch_delta),
        projection->pile_count (pixel));
  }
  best_count = sum_proj[textord_pitch_range].pile_count (0);
  best_delta = 0;
  best_pixel = 0;
  for (pitch_delta = -textord_pitch_range; pitch_delta <= textord_pitch_range;
  pitch_delta++) {
    for (pixel = 0; pixel < best_pitch + pitch_delta; pixel++) {
      if (sum_proj[textord_pitch_range + pitch_delta].pile_count (pixel)
      < best_count) {
        best_count =
          sum_proj[textord_pitch_range +
          pitch_delta].pile_count (pixel);
        best_delta = pitch_delta;
        best_pixel = pixel;
      }
    }
  }
  if (testing_on)
    tprintf ("tune_row_pitch:start pitch=%g, best_delta=%d, count=%d\n",
      initial_pitch, best_delta, best_count);
  best_pitch += best_delta;
  initial_pitch = best_pitch;
  best_count++;
  best_count += best_count;
  for (start = best_pixel - 2; start > best_pixel - best_pitch
    && sum_proj[textord_pitch_range +
    best_delta].pile_count (start % best_pitch) <= best_count;
    start--);
  for (end = best_pixel + 2;
    end < best_pixel + best_pitch
    && sum_proj[textord_pitch_range +
    best_delta].pile_count (end % best_pitch) <= best_count;
    end++);

  best_sd =
    compute_pitch_sd(row,
                     projection,
                     projection_left,
                     projection_right,
                     space_size,
                     initial_pitch,
                     best_sp_sd,
                     best_mid_cuts,
                     best_cells,
                     testing_on,
                     start,
                     end);
  if (testing_on)
    tprintf ("tune_row_pitch:output pitch=%g, sd=%g\n", initial_pitch,
      best_sd);

  if (textord_debug_pitch_metric)
    print_pitch_sd(row,
                   projection,
                   projection_left,
                   projection_right,
                   space_size,
                   initial_pitch);

  delete[]sum_proj;

  return best_sd;
}


/**********************************************************************
 * compute_pitch_sd
 *
 * Use a dp algorithm to fit the character cells and return the sd of
 * the cell size over the row.
 **********************************************************************/

float compute_pitch_sd(                            //find fp cells
                       TO_ROW *row,                //row to do
                       STATS *projection,          //vertical projection
                       inT16 projection_left,      //edge
                       inT16 projection_right,     //edge
                       float space_size,           //size of blank
                       float initial_pitch,        //guess at pitch
                       float &sp_sd,               //space sd
                       inT16 &mid_cuts,            //no of free cuts
                       ICOORDELT_LIST *row_cells,  //list of chop pts
                       BOOL8 testing_on,           //inidividual words
                       inT16 start,                //start of good range
                       inT16 end                   //end of good range
                      ) {
  inT16 occupation;              //no of cells in word.
                                 //blobs
  BLOBNBOX_IT blob_it = row->blob_list ();
  BLOBNBOX_IT start_it;          //start of word
  BLOBNBOX_IT plot_it;           //for plotting
  inT16 blob_count;              //no of blobs
  TBOX blob_box;                  //bounding box
  TBOX prev_box;                  //of super blob
  inT32 prev_right;              //of word sync
  int scale_factor;              //on scores for big words
  inT32 sp_count;                //spaces
  FPSEGPT_LIST seg_list;         //char cells
  FPSEGPT_IT seg_it;             //iterator
  inT16 segpos;                  //position of segment
  inT16 cellpos;                 //previous cell boundary
                                 //iterator
  ICOORDELT_IT cell_it = row_cells;
  ICOORDELT *cell;               //new cell
  double sqsum;                  //sum of squares
  double spsum;                  //of spaces
  double sp_var;                 //space error
  double word_sync;              //result for word
  inT32 total_count;             //total blobs

  if ((pitsync_linear_version & 3) > 1) {
    word_sync = compute_pitch_sd2 (row, projection, projection_left,
      projection_right, initial_pitch,
      occupation, mid_cuts, row_cells,
      testing_on, start, end);
    sp_sd = occupation;
    return word_sync;
  }
  mid_cuts = 0;
  cellpos = 0;
  total_count = 0;
  sqsum = 0;
  sp_count = 0;
  spsum = 0;
  prev_right = -1;
  if (blob_it.empty ())
    return space_size * 10;
#ifndef GRAPHICS_DISABLED
  if (testing_on && to_win > 0) {
    blob_box = blob_it.data ()->bounding_box ();
    projection->plot (to_win, projection_left,
      row->intercept (), 1.0f, -1.0f, ScrollView::CORAL);
  }
#endif
  start_it = blob_it;
  blob_count = 0;
  blob_box = box_next (&blob_it);//first blob
  blob_it.mark_cycle_pt ();
  do {
    for (; blob_count > 0; blob_count--)
      box_next(&start_it);
    do {
      prev_box = blob_box;
      blob_count++;
      blob_box = box_next (&blob_it);
    }
    while (!blob_it.cycled_list ()
      && blob_box.left () - prev_box.right () < space_size);
    plot_it = start_it;
    if (pitsync_linear_version & 3)
      word_sync =
        check_pitch_sync2 (&start_it, blob_count, (inT16) initial_pitch, 2,
        projection, projection_left, projection_right,
        row->xheight * textord_projection_scale,
        occupation, &seg_list, start, end);
    else
      word_sync =
        check_pitch_sync (&start_it, blob_count, (inT16) initial_pitch, 2,
        projection, &seg_list);
    if (testing_on) {
      tprintf ("Word ending at (%d,%d), len=%d, sync rating=%g, ",
        prev_box.right (), prev_box.top (),
        seg_list.length () - 1, word_sync);
      seg_it.set_to_list (&seg_list);
      for (seg_it.mark_cycle_pt (); !seg_it.cycled_list ();
      seg_it.forward ()) {
        if (seg_it.data ()->faked)
          tprintf ("(F)");
        tprintf ("%d, ", seg_it.data ()->position ());
        //                              tprintf("C=%g, s=%g, sq=%g\n",
        //                                      seg_it.data()->cost_function(),
        //                                      seg_it.data()->sum(),
        //                                      seg_it.data()->squares());
      }
      tprintf ("\n");
    }
#ifndef GRAPHICS_DISABLED
    if (textord_show_fixed_cuts && blob_count > 0 && to_win > 0)
      plot_fp_cells2(to_win, ScrollView::GOLDENROD, row, &seg_list);
#endif
    seg_it.set_to_list (&seg_list);
    if (prev_right >= 0) {
      sp_var = seg_it.data ()->position () - prev_right;
      sp_var -= floor (sp_var / initial_pitch + 0.5) * initial_pitch;
      sp_var *= sp_var;
      spsum += sp_var;
      sp_count++;
    }
    for (seg_it.mark_cycle_pt (); !seg_it.cycled_list (); seg_it.forward ()) {
      segpos = seg_it.data ()->position ();
      if (cell_it.empty () || segpos > cellpos + initial_pitch / 2) {
                                 //big gap
        while (!cell_it.empty () && segpos > cellpos + initial_pitch * 3 / 2) {
          cell = new ICOORDELT (cellpos + (inT16) initial_pitch, 0);
          cell_it.add_after_then_move (cell);
          cellpos += (inT16) initial_pitch;
        }
                                 //make new one
        cell = new ICOORDELT (segpos, 0);
        cell_it.add_after_then_move (cell);
        cellpos = segpos;
      }
      else if (segpos > cellpos - initial_pitch / 2) {
        cell = cell_it.data ();
                                 //average positions
        cell->set_x ((cellpos + segpos) / 2);
        cellpos = cell->x ();
      }
    }
    seg_it.move_to_last ();
    prev_right = seg_it.data ()->position ();
    if (textord_pitch_scalebigwords) {
      scale_factor = (seg_list.length () - 2) / 2;
      if (scale_factor < 1)
        scale_factor = 1;
    }
    else
      scale_factor = 1;
    sqsum += word_sync * scale_factor;
    total_count += (seg_list.length () - 1) * scale_factor;
    seg_list.clear ();
  }
  while (!blob_it.cycled_list ());
  sp_sd = sp_count > 0 ? sqrt (spsum / sp_count) : 0;
  return total_count > 0 ? sqrt (sqsum / total_count) : space_size * 10;
}


/**********************************************************************
 * compute_pitch_sd2
 *
 * Use a dp algorithm to fit the character cells and return the sd of
 * the cell size over the row.
 **********************************************************************/

float compute_pitch_sd2(                            //find fp cells
                        TO_ROW *row,                //row to do
                        STATS *projection,          //vertical projection
                        inT16 projection_left,      //edge
                        inT16 projection_right,     //edge
                        float initial_pitch,        //guess at pitch
                        inT16 &occupation,          //no of occupied cells
                        inT16 &mid_cuts,            //no of free cuts
                        ICOORDELT_LIST *row_cells,  //list of chop pts
                        BOOL8 testing_on,           //inidividual words
                        inT16 start,                //start of good range
                        inT16 end                   //end of good range
                       ) {
                                 //blobs
  BLOBNBOX_IT blob_it = row->blob_list ();
  BLOBNBOX_IT plot_it;
  inT16 blob_count;              //no of blobs
  TBOX blob_box;                  //bounding box
  FPSEGPT_LIST seg_list;         //char cells
  FPSEGPT_IT seg_it;             //iterator
  inT16 segpos;                  //position of segment
                                 //iterator
  ICOORDELT_IT cell_it = row_cells;
  ICOORDELT *cell;               //new cell
  double word_sync;              //result for word

  mid_cuts = 0;
  if (blob_it.empty ()) {
    occupation = 0;
    return initial_pitch * 10;
  }
#ifndef GRAPHICS_DISABLED
  if (testing_on && to_win > 0) {
    projection->plot (to_win, projection_left,
      row->intercept (), 1.0f, -1.0f, ScrollView::CORAL);
  }
#endif
  blob_count = 0;
  blob_it.mark_cycle_pt ();
  do {
                                 //first blob
    blob_box = box_next (&blob_it);
    blob_count++;
  }
  while (!blob_it.cycled_list ());
  plot_it = blob_it;
  word_sync = check_pitch_sync2 (&blob_it, blob_count, (inT16) initial_pitch,
    2, projection, projection_left,
    projection_right,
    row->xheight * textord_projection_scale,
    occupation, &seg_list, start, end);
  if (testing_on) {
    tprintf ("Row ending at (%d,%d), len=%d, sync rating=%g, ",
      blob_box.right (), blob_box.top (),
      seg_list.length () - 1, word_sync);
    seg_it.set_to_list (&seg_list);
    for (seg_it.mark_cycle_pt (); !seg_it.cycled_list (); seg_it.forward ()) {
      if (seg_it.data ()->faked)
        tprintf ("(F)");
      tprintf ("%d, ", seg_it.data ()->position ());
      //                              tprintf("C=%g, s=%g, sq=%g\n",
      //                                      seg_it.data()->cost_function(),
      //                                      seg_it.data()->sum(),
      //                                      seg_it.data()->squares());
    }
    tprintf ("\n");
  }
#ifndef GRAPHICS_DISABLED
  if (textord_show_fixed_cuts && blob_count > 0 && to_win > 0)
    plot_fp_cells2(to_win, ScrollView::GOLDENROD, row, &seg_list);
#endif
  seg_it.set_to_list (&seg_list);
  for (seg_it.mark_cycle_pt (); !seg_it.cycled_list (); seg_it.forward ()) {
    segpos = seg_it.data ()->position ();
                                 //make new one
    cell = new ICOORDELT (segpos, 0);
    cell_it.add_after_then_move (cell);
    if (seg_it.at_last ())
      mid_cuts = seg_it.data ()->cheap_cuts ();
  }
  seg_list.clear ();
  return occupation > 0 ? sqrt (word_sync / occupation) : initial_pitch * 10;
}


/**********************************************************************
 * print_pitch_sd
 *
 * Use a dp algorithm to fit the character cells and return the sd of
 * the cell size over the row.
 **********************************************************************/

void print_pitch_sd(                        //find fp cells
                    TO_ROW *row,            //row to do
                    STATS *projection,      //vertical projection
                    inT16 projection_left,  //edges //size of blank
                    inT16 projection_right,
                    float space_size,
                    float initial_pitch     //guess at pitch
                   ) {
  const char *res2;              //pitch result
  inT16 occupation;              //used cells
  float sp_sd;                   //space sd
                                 //blobs
  BLOBNBOX_IT blob_it = row->blob_list ();
  BLOBNBOX_IT start_it;          //start of word
  BLOBNBOX_IT row_start;         //start of row
  inT16 blob_count;              //no of blobs
  inT16 total_blob_count;        //total blobs in line
  TBOX blob_box;                  //bounding box
  TBOX prev_box;                  //of super blob
  inT32 prev_right;              //of word sync
  int scale_factor;              //on scores for big words
  inT32 sp_count;                //spaces
  FPSEGPT_LIST seg_list;         //char cells
  FPSEGPT_IT seg_it;             //iterator
  double sqsum;                  //sum of squares
  double spsum;                  //of spaces
  double sp_var;                 //space error
  double word_sync;              //result for word
  double total_count;            //total cuts

  if (blob_it.empty ())
    return;
  row_start = blob_it;
  total_blob_count = 0;

  total_count = 0;
  sqsum = 0;
  sp_count = 0;
  spsum = 0;
  prev_right = -1;
  blob_it = row_start;
  start_it = blob_it;
  blob_count = 0;
  blob_box = box_next (&blob_it);//first blob
  blob_it.mark_cycle_pt ();
  do {
    for (; blob_count > 0; blob_count--)
      box_next(&start_it);
    do {
      prev_box = blob_box;
      blob_count++;
      blob_box = box_next (&blob_it);
    }
    while (!blob_it.cycled_list ()
      && blob_box.left () - prev_box.right () < space_size);
    word_sync =
      check_pitch_sync2 (&start_it, blob_count, (inT16) initial_pitch, 2,
      projection, projection_left, projection_right,
      row->xheight * textord_projection_scale,
      occupation, &seg_list, 0, 0);
    total_blob_count += blob_count;
    seg_it.set_to_list (&seg_list);
    if (prev_right >= 0) {
      sp_var = seg_it.data ()->position () - prev_right;
      sp_var -= floor (sp_var / initial_pitch + 0.5) * initial_pitch;
      sp_var *= sp_var;
      spsum += sp_var;
      sp_count++;
    }
    seg_it.move_to_last ();
    prev_right = seg_it.data ()->position ();
    if (textord_pitch_scalebigwords) {
      scale_factor = (seg_list.length () - 2) / 2;
      if (scale_factor < 1)
        scale_factor = 1;
    }
    else
      scale_factor = 1;
    sqsum += word_sync * scale_factor;
    total_count += (seg_list.length () - 1) * scale_factor;
    seg_list.clear ();
  }
  while (!blob_it.cycled_list ());
  sp_sd = sp_count > 0 ? sqrt (spsum / sp_count) : 0;
  word_sync = total_count > 0 ? sqrt (sqsum / total_count) : space_size * 10;
  tprintf ("new_sd=%g:sd/p=%g:new_sp_sd=%g:res=%c:",
    word_sync, word_sync / initial_pitch, sp_sd,
    word_sync < textord_words_pitchsd_threshold * initial_pitch
    ? 'F' : 'P');

  start_it = row_start;
  blob_it = row_start;
  word_sync =
    check_pitch_sync2 (&blob_it, total_blob_count, (inT16) initial_pitch, 2,
    projection, projection_left, projection_right,
    row->xheight * textord_projection_scale, occupation,
    &seg_list, 0, 0);
  if (occupation > 1)
    word_sync /= occupation;
  word_sync = sqrt (word_sync);

#ifndef GRAPHICS_DISABLED
  if (textord_show_row_cuts && to_win != NULL)
    plot_fp_cells2(to_win, ScrollView::CORAL, row, &seg_list);
#endif
  seg_list.clear ();
  if (word_sync < textord_words_pitchsd_threshold * initial_pitch) {
    if (word_sync < textord_words_def_fixed * initial_pitch
      && !row->all_caps)
      res2 = "DF";
    else
      res2 = "MF";
  }
  else
    res2 = word_sync < textord_words_def_prop * initial_pitch ? "MP" : "DP";
  tprintf
    ("row_sd=%g:sd/p=%g:res=%c:N=%d:res2=%s,init pitch=%g, row_pitch=%g, all_caps=%d\n",
    word_sync, word_sync / initial_pitch,
    word_sync < textord_words_pitchsd_threshold * initial_pitch ? 'F' : 'P',
    occupation, res2, initial_pitch, row->fixed_pitch, row->all_caps);
}

/**********************************************************************
 * find_repeated_chars
 *
 * Extract marked leader blobs and put them
 * into words in advance of fixed pitch checking and word generation.
 **********************************************************************/
void find_repeated_chars(TO_BLOCK *block,       // Block to search.
                         BOOL8 testing_on) {    // Debug mode.
  POLY_BLOCK* pb = block->block->poly_block();
  if (pb != NULL && !pb->IsText())
    return;  // Don't find repeated chars in non-text blocks.

  TO_ROW *row;
  BLOBNBOX_IT box_it;
  BLOBNBOX_IT search_it;         // forward search
  WERD_IT word_it;               // new words
  WERD *word;                    // new word
  TBOX word_box;                 // for plotting
  int blobcount, repeated_set;

  TO_ROW_IT row_it = block->get_rows();
  if (row_it.empty()) return;  // empty block
  for (row_it.mark_cycle_pt(); !row_it.cycled_list(); row_it.forward()) {
    row = row_it.data();
    box_it.set_to_list(row->blob_list());
    if (box_it.empty())  continue; // no blobs in this row
    if (!row->rep_chars_marked()) {
      mark_repeated_chars(row);
    }
    if (row->num_repeated_sets() == 0) continue;  // nothing to do for this row
    word_it.set_to_list(&row->rep_words);
    do {
      if (box_it.data()->repeated_set() != 0 &&
          !box_it.data()->joined_to_prev()) {
        blobcount = 1;
        repeated_set = box_it.data()->repeated_set();
        search_it = box_it;
        search_it.forward();
        while (!search_it.at_first() &&
               search_it.data()->repeated_set() == repeated_set) {
          blobcount++;
          search_it.forward();
        }
        // After the call to make_real_word() all the blobs from this
        // repeated set will be removed from the blob list. box_it will be
        // set to point to the blob after the end of the extracted sequence.
        word = make_real_word(&box_it, blobcount, box_it.at_first(), 1);
        if (!box_it.empty() && box_it.data()->joined_to_prev()) {
          tprintf("Bad box joined to prev at");
          box_it.data()->bounding_box().print();
          tprintf("After repeated word:");
          word->bounding_box().print();
        }
        ASSERT_HOST(box_it.empty() || !box_it.data()->joined_to_prev());
        word->set_flag(W_REP_CHAR, true);
        word->set_flag(W_DONT_CHOP, true);
        word_it.add_after_then_move(word);
      } else {
        box_it.forward();
      }
    } while (!box_it.at_first());
  }
}


/**********************************************************************
 * plot_fp_word
 *
 * Plot a block of words as if fixed pitch.
 **********************************************************************/

#ifndef GRAPHICS_DISABLED
void plot_fp_word(                  //draw block of words
                  TO_BLOCK *block,  //block to draw
                  float pitch,      //pitch to draw with
                  float nonspace    //for space threshold
                 ) {
  TO_ROW *row;                   //current row
  TO_ROW_IT row_it = block->get_rows ();

  for (row_it.mark_cycle_pt (); !row_it.cycled_list (); row_it.forward ()) {
    row = row_it.data ();
    row->min_space = (inT32) ((pitch + nonspace) / 2);
    row->max_nonspace = row->min_space;
    row->space_threshold = row->min_space;
    plot_word_decisions (to_win, (inT16) pitch, row);
  }
}
#endif
