///////////////////////////////////////////////////////////////////////
// File:        textord.cpp
// Description: The top-level text line and word finding functionality.
// Author:      Ray Smith
// Created:     Fri Mar 13 14:43:01 PDT 2009
//
// (C) Copyright 2009, Google Inc.
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
///////////////////////////////////////////////////////////////////////

// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif

#include "baselinedetect.h"
#include "drawtord.h"
#include "textord.h"
#include "makerow.h"
#include "pageres.h"
#include "tordmain.h"
#include "wordseg.h"

namespace tesseract {

Textord::Textord(CCStruct* ccstruct)
    : ccstruct_(ccstruct), use_cjk_fp_model_(false),
      // makerow.cpp ///////////////////////////////////////////
      BOOL_MEMBER(textord_single_height_mode, false,
                  "Script has no xheight, so use a single mode",
                  ccstruct_->params()),
      // tospace.cpp ///////////////////////////////////////////
      BOOL_MEMBER(tosp_old_to_method, false, "Space stats use prechopping?",
                  ccstruct_->params()),
      BOOL_MEMBER(tosp_old_to_constrain_sp_kn, false,
                  "Constrain relative values of inter and intra-word gaps for "
                  "old_to_method.",
                  ccstruct_->params()),
      BOOL_MEMBER(tosp_only_use_prop_rows, true,
                  "Block stats to use fixed pitch rows?",
                  ccstruct_->params()),
      BOOL_MEMBER(tosp_force_wordbreak_on_punct, false,
                  "Force word breaks on punct to break long lines in non-space "
                  "delimited langs",
                  ccstruct_->params()),
      BOOL_MEMBER(tosp_use_pre_chopping, false,
                  "Space stats use prechopping?",
                  ccstruct_->params()),
      BOOL_MEMBER(tosp_old_to_bug_fix, false, "Fix suspected bug in old code",
                  ccstruct_->params()),
      BOOL_MEMBER(tosp_block_use_cert_spaces, true,
                  "Only stat OBVIOUS spaces",
                  ccstruct_->params()),
      BOOL_MEMBER(tosp_row_use_cert_spaces, true, "Only stat OBVIOUS spaces",
                  ccstruct_->params()),
      BOOL_MEMBER(tosp_narrow_blobs_not_cert, true,
            "Only stat OBVIOUS spaces",
                  ccstruct_->params()),
      BOOL_MEMBER(tosp_row_use_cert_spaces1, true, "Only stat OBVIOUS spaces",
                  ccstruct_->params()),
      BOOL_MEMBER(tosp_recovery_isolated_row_stats, true,
                  "Use row alone when inadequate cert spaces",
                  ccstruct_->params()),
      BOOL_MEMBER(tosp_only_small_gaps_for_kern, false, "Better guess",
                  ccstruct_->params()),
      BOOL_MEMBER(tosp_all_flips_fuzzy, false, "Pass ANY flip to context?",
                  ccstruct_->params()),
      BOOL_MEMBER(tosp_fuzzy_limit_all, true,
                  "Dont restrict kn->sp fuzzy limit to tables",
                  ccstruct_->params()),
      BOOL_MEMBER(tosp_stats_use_xht_gaps, true,
                  "Use within xht gap for wd breaks",
                  ccstruct_->params()),
      BOOL_MEMBER(tosp_use_xht_gaps, true, "Use within xht gap for wd breaks",
                  ccstruct_->params()),
      BOOL_MEMBER(tosp_only_use_xht_gaps, false,
                  "Only use within xht gap for wd breaks",
                  ccstruct_->params()),
      BOOL_MEMBER(tosp_rule_9_test_punct, false,
                  "Dont chng kn to space next to punct",
                  ccstruct_->params()),
      BOOL_MEMBER(tosp_flip_fuzz_kn_to_sp, true, "Default flip",
                  ccstruct_->params()),
      BOOL_MEMBER(tosp_flip_fuzz_sp_to_kn, true, "Default flip",
                  ccstruct_->params()),
      BOOL_MEMBER(tosp_improve_thresh, false, "Enable improvement heuristic",
                  ccstruct_->params()),
      INT_MEMBER(tosp_debug_level, 0, "Debug data",
                 ccstruct_->params()),
      INT_MEMBER(tosp_enough_space_samples_for_median, 3,
           "or should we use mean",
                 ccstruct_->params()),
      INT_MEMBER(tosp_redo_kern_limit, 10,
                 "No.samples reqd to reestimate for row",
                 ccstruct_->params()),
      INT_MEMBER(tosp_few_samples, 40,
                 "No.gaps reqd with 1 large gap to treat as a table",
                 ccstruct_->params()),
      INT_MEMBER(tosp_short_row, 20,
                 "No.gaps reqd with few cert spaces to use certs",
                 ccstruct_->params()),
      INT_MEMBER(tosp_sanity_method, 1, "How to avoid being silly",
                 ccstruct_->params()),
      double_MEMBER(tosp_old_sp_kn_th_factor, 2.0,
                    "Factor for defining space threshold in terms of space and "
                    "kern sizes",
                    ccstruct_->params()),
      double_MEMBER(tosp_threshold_bias1, 0,
                    "how far between kern and space?",
                    ccstruct_->params()),
      double_MEMBER(tosp_threshold_bias2, 0,
                    "how far between kern and space?",
                    ccstruct_->params()),
      double_MEMBER(tosp_narrow_fraction, 0.3, "Fract of xheight for narrow",
                    ccstruct_->params()),
      double_MEMBER(tosp_narrow_aspect_ratio, 0.48,
                    "narrow if w/h less than this",
                    ccstruct_->params()),
      double_MEMBER(tosp_wide_fraction, 0.52, "Fract of xheight for wide",
                    ccstruct_->params()),
      double_MEMBER(tosp_wide_aspect_ratio, 0.0, "wide if w/h less than this",
                    ccstruct_->params()),
      double_MEMBER(tosp_fuzzy_space_factor, 0.6,
                    "Fract of xheight for fuzz sp",
                    ccstruct_->params()),
      double_MEMBER(tosp_fuzzy_space_factor1, 0.5,
                    "Fract of xheight for fuzz sp",
                    ccstruct_->params()),
      double_MEMBER(tosp_fuzzy_space_factor2, 0.72,
                    "Fract of xheight for fuzz sp",
                    ccstruct_->params()),
      double_MEMBER(tosp_gap_factor, 0.83, "gap ratio to flip sp->kern",
                    ccstruct_->params()),
      double_MEMBER(tosp_kern_gap_factor1, 2.0, "gap ratio to flip kern->sp",
                    ccstruct_->params()),
      double_MEMBER(tosp_kern_gap_factor2, 1.3, "gap ratio to flip kern->sp",
                    ccstruct_->params()),
      double_MEMBER(tosp_kern_gap_factor3, 2.5, "gap ratio to flip kern->sp",
                    ccstruct_->params()),
      double_MEMBER(tosp_ignore_big_gaps, -1, "xht multiplier",
                    ccstruct_->params()),
      double_MEMBER(tosp_ignore_very_big_gaps, 3.5, "xht multiplier",
                    ccstruct_->params()),
      double_MEMBER(tosp_rep_space, 1.6, "rep gap multiplier for space",
                    ccstruct_->params()),
      double_MEMBER(tosp_enough_small_gaps, 0.65,
                    "Fract of kerns reqd for isolated row stats",
                    ccstruct_->params()),
      double_MEMBER(tosp_table_kn_sp_ratio, 2.25,
                    "Min difference of kn & sp in table",
                    ccstruct_->params()),
      double_MEMBER(tosp_table_xht_sp_ratio, 0.33,
                    "Expect spaces bigger than this",
                    ccstruct_->params()),
      double_MEMBER(tosp_table_fuzzy_kn_sp_ratio, 3.0,
                    "Fuzzy if less than this",
                    ccstruct_->params()),
      double_MEMBER(tosp_fuzzy_kn_fraction, 0.5, "New fuzzy kn alg",
                    ccstruct_->params()),
      double_MEMBER(tosp_fuzzy_sp_fraction, 0.5, "New fuzzy sp alg",
                    ccstruct_->params()),
      double_MEMBER(tosp_min_sane_kn_sp, 1.5,
                    "Dont trust spaces less than this time kn",
                    ccstruct_->params()),
      double_MEMBER(tosp_init_guess_kn_mult, 2.2,
                    "Thresh guess - mult kn by this",
                    ccstruct_->params()),
      double_MEMBER(tosp_init_guess_xht_mult, 0.28,
                    "Thresh guess - mult xht by this",
                    ccstruct_->params()),
      double_MEMBER(tosp_max_sane_kn_thresh, 5.0,
                    "Multiplier on kn to limit thresh",
                    ccstruct_->params()),
      double_MEMBER(tosp_flip_caution, 0.0,
                    "Dont autoflip kn to sp when large separation",
                    ccstruct_->params()),
      double_MEMBER(tosp_large_kerning, 0.19,
                    "Limit use of xht gap with large kns",
                    ccstruct_->params()),
      double_MEMBER(tosp_dont_fool_with_small_kerns, -1,
                    "Limit use of xht gap with odd small kns",
                    ccstruct_->params()),
      double_MEMBER(tosp_near_lh_edge, 0,
                    "Dont reduce box if the top left is non blank",
                    ccstruct_->params()),
      double_MEMBER(tosp_silly_kn_sp_gap, 0.2,
                    "Dont let sp minus kn get too small",
                    ccstruct_->params()),
      double_MEMBER(tosp_pass_wide_fuzz_sp_to_context, 0.75,
                    "How wide fuzzies need context",
                    ccstruct_->params()),
      // tordmain.cpp ///////////////////////////////////////////
      BOOL_MEMBER(textord_no_rejects, false, "Don't remove noise blobs",
                  ccstruct_->params()),
      BOOL_MEMBER(textord_show_blobs, false, "Display unsorted blobs",
                  ccstruct_->params()),
      BOOL_MEMBER(textord_show_boxes, false, "Display unsorted blobs",
                  ccstruct_->params()),
      INT_MEMBER(textord_max_noise_size, 7, "Pixel size of noise",
                  ccstruct_->params()),
      INT_MEMBER(textord_baseline_debug, 0, "Baseline debug level",
                  ccstruct_->params()),
      double_MEMBER(textord_blob_size_bigile, 95, "Percentile for large blobs",
                    ccstruct_->params()),
      double_MEMBER(textord_noise_area_ratio, 0.7,
                    "Fraction of bounding box for noise",
                    ccstruct_->params()),
      double_MEMBER(textord_blob_size_smallile, 20,
                    "Percentile for small blobs",
                    ccstruct_->params()),
      double_MEMBER(textord_initialx_ile, 0.75,
                    "Ile of sizes for xheight guess",
                    ccstruct_->params()),
      double_MEMBER(textord_initialasc_ile, 0.90,
                    "Ile of sizes for xheight guess",
                    ccstruct_->params()),
      INT_MEMBER(textord_noise_sizefraction, 10,
                 "Fraction of size for maxima",
                 ccstruct_->params()),
      double_MEMBER(textord_noise_sizelimit, 0.5,
                    "Fraction of x for big t count",
                    ccstruct_->params()),
      INT_MEMBER(textord_noise_translimit, 16, "Transitions for normal blob",
                 ccstruct_->params()),
      double_MEMBER(textord_noise_normratio, 2.0,
                    "Dot to norm ratio for deletion",
                    ccstruct_->params()),
      BOOL_MEMBER(textord_noise_rejwords, true, "Reject noise-like words",
                  ccstruct_->params()),
      BOOL_MEMBER(textord_noise_rejrows, true, "Reject noise-like rows",
                  ccstruct_->params()),
      double_MEMBER(textord_noise_syfract, 0.2,
                    "xh fract height error for norm blobs",
                    ccstruct_->params()),
      double_MEMBER(textord_noise_sxfract, 0.4,
                    "xh fract width error for norm blobs",
                    ccstruct_->params()),
      double_MEMBER(textord_noise_hfract, 1.0/64,
                    "Height fraction to discard outlines as speckle noise",
                    ccstruct_->params()),
      INT_MEMBER(textord_noise_sncount, 1, "super norm blobs to save row",
                 ccstruct_->params()),
      double_MEMBER(textord_noise_rowratio, 6.0,
                    "Dot to norm ratio for deletion",
                    ccstruct_->params()),
      BOOL_MEMBER(textord_noise_debug, false, "Debug row garbage detector",
                  ccstruct_->params()),
      double_MEMBER(textord_blshift_maxshift, 0.00, "Max baseline shift",
                    ccstruct_->params()),
      double_MEMBER(textord_blshift_xfraction, 9.99,
                    "Min size of baseline shift",
                    ccstruct_->params()) {
}

Textord::~Textord() {
}

// Make the textlines and words inside each block.
void Textord::TextordPage(PageSegMode pageseg_mode, const FCOORD& reskew,
                          int width, int height, Pix* binary_pix,
                          Pix* thresholds_pix, Pix* grey_pix,
                          bool use_box_bottoms,
                          BLOCK_LIST* blocks, TO_BLOCK_LIST* to_blocks) {
  page_tr_.set_x(width);
  page_tr_.set_y(height);
  if (to_blocks->empty()) {
    // AutoPageSeg was not used, so we need to find_components first.
    find_components(binary_pix, blocks, to_blocks);
    TO_BLOCK_IT it(to_blocks);
    for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
      TO_BLOCK* to_block = it.data();
      // Compute the edge offsets whether or not there is a grey_pix.
      // We have by-passed auto page seg, so we have to run it here.
      // By page segmentation mode there is no non-text to avoid running on.
      to_block->ComputeEdgeOffsets(thresholds_pix, grey_pix);
    }
  } else if (!PSM_SPARSE(pageseg_mode)) {
    // AutoPageSeg does not need to find_components as it did that already.
    // Filter_blobs sets up the TO_BLOCKs the same as find_components does.
    filter_blobs(page_tr_, to_blocks, true);
  }

  ASSERT_HOST(!to_blocks->empty());
  if (pageseg_mode == PSM_SINGLE_BLOCK_VERT_TEXT) {
    const FCOORD anticlockwise90(0.0f, 1.0f);
    const FCOORD clockwise90(0.0f, -1.0f);
    TO_BLOCK_IT it(to_blocks);
    for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
      TO_BLOCK* to_block = it.data();
      BLOCK* block = to_block->block;
      // Create a fake poly_block in block from its bounding box.
      block->set_poly_block(new POLY_BLOCK(block->bounding_box(),
                                           PT_VERTICAL_TEXT));
      // Rotate the to_block along with its contained block and blobnbox lists.
      to_block->rotate(anticlockwise90);
      // Set the block's rotation values to obey the convention followed in
      // layout analysis for vertical text.
      block->set_re_rotation(clockwise90);
      block->set_classify_rotation(clockwise90);
    }
  }

  TO_BLOCK_IT to_block_it(to_blocks);
  TO_BLOCK* to_block = to_block_it.data();
  // Make the rows in the block.
  float gradient;
  // Do it the old fashioned way.
  if (PSM_LINE_FIND_ENABLED(pageseg_mode)) {
    gradient = make_rows(page_tr_, to_blocks);
  } else if (!PSM_SPARSE(pageseg_mode)) {
    // RAW_LINE, SINGLE_LINE, SINGLE_WORD and SINGLE_CHAR all need a single row.
    gradient = make_single_row(page_tr_, pageseg_mode != PSM_RAW_LINE,
                               to_block, to_blocks);
  }
  BaselineDetect baseline_detector(textord_baseline_debug,
                                   reskew, to_blocks);
  baseline_detector.ComputeStraightBaselines(use_box_bottoms);
  baseline_detector.ComputeBaselineSplinesAndXheights(page_tr_, true,
                                                      textord_heavy_nr,
                                                      textord_show_final_rows,
                                                      this);
  // Now make the words in the lines.
  if (PSM_WORD_FIND_ENABLED(pageseg_mode)) {
    // SINGLE_LINE uses the old word maker on the single line.
    make_words(this, page_tr_, gradient, blocks, to_blocks);
  } else {
    // SINGLE_WORD and SINGLE_CHAR cram all the blobs into a
    // single word, and in SINGLE_CHAR mode, all the outlines
    // go in a single blob.
    TO_BLOCK* to_block = to_block_it.data();
    make_single_word(pageseg_mode == PSM_SINGLE_CHAR,
                     to_block->get_rows(), to_block->block->row_list());
  }
  cleanup_blocks(PSM_WORD_FIND_ENABLED(pageseg_mode), blocks);
  // Remove empties.

  // Compute the margins for each row in the block, to be used later for
  // paragraph detection.
  BLOCK_IT b_it(blocks);
  for (b_it.mark_cycle_pt(); !b_it.cycled_list(); b_it.forward()) {
    b_it.data()->compute_row_margins();
  }
#ifndef GRAPHICS_DISABLED
  close_to_win();
#endif
}

// If we were supposed to return only a single textline, and there is more
// than one, clean up and leave only the best.
void Textord::CleanupSingleRowResult(PageSegMode pageseg_mode,
                                     PAGE_RES* page_res) {
  if (PSM_LINE_FIND_ENABLED(pageseg_mode) || PSM_SPARSE(pageseg_mode))
    return;  // No cleanup required.
  PAGE_RES_IT it(page_res);
  // Find the best row, being the greatest mean word conf.
  float row_total_conf = 0.0f;
  int row_word_count = 0;
  ROW_RES* best_row = NULL;
  float best_conf = 0.0f;
  for (it.restart_page(); it.word() != NULL; it.forward()) {
    WERD_RES* word = it.word();
    row_total_conf += word->best_choice->certainty();
    ++row_word_count;
    if (it.next_row() != it.row()) {
      row_total_conf /= row_word_count;
      if (best_row == NULL || best_conf < row_total_conf) {
        best_row = it.row();
        best_conf = row_total_conf;
      }
      row_total_conf = 0.0f;
      row_word_count = 0;
    }
  }
  // Now eliminate any word not in the best row.
  for (it.restart_page(); it.word() != NULL; it.forward()) {
    if (it.row() != best_row)
      it.DeleteCurrentWord();
  }
}

}  // namespace tesseract.
