///////////////////////////////////////////////////////////////////////
// File:        textord.h
// Description: The Textord class definition gathers text line and word
//              finding functionality.
// Author:      Ray Smith
// Created:     Fri Mar 13 14:29:01 PDT 2009
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

#ifndef TESSERACT_TEXTORD_TEXTORD_H_
#define TESSERACT_TEXTORD_TEXTORD_H_

#include "bbgrid.h"
#include "blobbox.h"
#include "ccstruct.h"
#include "gap_map.h"

#include <tesseract/publictypes.h> // For PageSegMode.

namespace tesseract {

class FCOORD;
class BLOCK_LIST;
class PAGE_RES;
class TO_BLOCK;
class TO_BLOCK_LIST;
class ScrollView;

// A simple class that can be used by BBGrid to hold a word and an expanded
// bounding box that makes it easy to find words to put diacritics.
class WordWithBox {
public:
  WordWithBox() : word_(nullptr) {}
  explicit WordWithBox(WERD *word) : word_(word), bounding_box_(word->bounding_box()) {
    int height = bounding_box_.height();
    bounding_box_.pad(height, height);
  }

  const TBOX &bounding_box() const {
    return bounding_box_;
  }
  // Returns the bounding box of only the good blobs.
  TBOX true_bounding_box() const {
    return word_->true_bounding_box();
  }
  C_BLOB_LIST *RejBlobs() const {
    return word_->rej_cblob_list();
  }
  const WERD *word() const {
    return word_;
  }

private:
  // Borrowed pointer to a real word somewhere that must outlive this class.
  WERD *word_;
  // Cached expanded bounding box of the word, padded all round by its height.
  TBOX bounding_box_;
};

// Make it usable by BBGrid.
CLISTIZEH(WordWithBox)
using WordGrid = BBGrid<WordWithBox, WordWithBox_CLIST, WordWithBox_C_IT>;
using WordSearch = GridSearch<WordWithBox, WordWithBox_CLIST, WordWithBox_C_IT>;

class Textord {
public:
  explicit Textord(CCStruct *ccstruct);
  ~Textord() = default;

  // Make the textlines and words inside each block.
  // binary_pix is mandatory and is the binarized input after line removal.
  // grey_pix is optional, but if present must match the binary_pix in size,
  // and must be a *real* grey image instead of binary_pix * 255.
  // thresholds_pix is expected to be present iff grey_pix is present and
  // can be an integer factor reduction of the grey_pix. It represents the
  // thresholds that were used to create the binary_pix from the grey_pix.
  // diacritic_blobs contain small confusing components that should be added
  // to the appropriate word(s) in case they are really diacritics.
  void TextordPage(PageSegMode pageseg_mode, const FCOORD &reskew, int width, int height,
                   Image binary_pix, Image thresholds_pix, Image grey_pix, bool use_box_bottoms,
                   BLOBNBOX_LIST *diacritic_blobs, BLOCK_LIST *blocks, TO_BLOCK_LIST *to_blocks,
                   float *gradient);

  // If we were supposed to return only a single textline, and there is more
  // than one, clean up and leave only the best.
  void CleanupSingleRowResult(PageSegMode pageseg_mode, PAGE_RES *page_res);

  bool use_cjk_fp_model() const {
    return use_cjk_fp_model_;
  }
  void set_use_cjk_fp_model(bool flag) {
    use_cjk_fp_model_ = flag;
  }

  // tospace.cpp ///////////////////////////////////////////
  void to_spacing(ICOORD page_tr,       // topright of page
                  TO_BLOCK_LIST *blocks // blocks on page
  );
  ROW *make_prop_words(TO_ROW *row,    // row to make
                       FCOORD rotation // for drawing
  );
  ROW *make_blob_words(TO_ROW *row,    // row to make
                       FCOORD rotation // for drawing
  );
  // tordmain.cpp ///////////////////////////////////////////
  void find_components(Image pix, BLOCK_LIST *blocks, TO_BLOCK_LIST *to_blocks);
  void filter_blobs(ICOORD page_tr, TO_BLOCK_LIST *blocks, bool testing_on);

private:
  // For underlying memory management and other utilities.
  CCStruct *ccstruct_;

  // The size of the input image.
  ICOORD page_tr_;

  bool use_cjk_fp_model_;

  // makerow.cpp ///////////////////////////////////////////
  // Make the textlines inside each block.
  void MakeRows(PageSegMode pageseg_mode, const FCOORD &skew, int width, int height,
                TO_BLOCK_LIST *to_blocks);
  // Make the textlines inside a single block.
  void MakeBlockRows(int min_spacing, int max_spacing, const FCOORD &skew, TO_BLOCK *block,
                     ScrollView *win);

public:
  void compute_block_xheight(TO_BLOCK *block, float gradient);
  void compute_row_xheight(TO_ROW *row, // row to do
                           const FCOORD &rotation,
                           float gradient, // global skew
                           int block_line_size);
  void make_spline_rows(TO_BLOCK *block, // block to do
                        float gradient,  // gradient to fit
                        bool testing_on);

private:
  //// oldbasel.cpp ////////////////////////////////////////
  void make_old_baselines(TO_BLOCK *block, // block to do
                          bool testing_on, // correct orientation
                          float gradient);
  void correlate_lines(TO_BLOCK *block, float gradient);
  void correlate_neighbours(TO_BLOCK *block, // block rows are in.
                            TO_ROW **rows,   // rows of block.
                            int rowcount);   // no of rows to do.
  int correlate_with_stats(TO_ROW **rows,    // rows of block.
                           int rowcount,     // no of rows to do.
                           TO_BLOCK *block);
  void find_textlines(TO_BLOCK *block,  // block row is in
                      TO_ROW *row,      // row to do
                      int degree,       // required approximation
                      QSPLINE *spline); // starting spline
  // tospace.cpp ///////////////////////////////////////////
  // DEBUG USE ONLY
  void block_spacing_stats(TO_BLOCK *block, GAPMAP *gapmap, bool &old_text_ord_proportional,
                           // resulting estimate
                           int16_t &block_space_gap_width,
                           // resulting estimate
                           int16_t &block_non_space_gap_width);
  void row_spacing_stats(TO_ROW *row, GAPMAP *gapmap, int16_t block_idx, int16_t row_idx,
                         // estimate for block
                         int16_t block_space_gap_width,
                         // estimate for block
                         int16_t block_non_space_gap_width);
  void old_to_method(TO_ROW *row, STATS *all_gap_stats, STATS *space_gap_stats,
                     STATS *small_gap_stats, int16_t block_space_gap_width,
                     // estimate for block
                     int16_t block_non_space_gap_width);
  bool isolated_row_stats(TO_ROW *row, GAPMAP *gapmap, STATS *all_gap_stats, bool suspected_table,
                          int16_t block_idx, int16_t row_idx);
  int16_t stats_count_under(STATS *stats, int16_t threshold);
  void improve_row_threshold(TO_ROW *row, STATS *all_gap_stats);
  bool make_a_word_break(TO_ROW *row,   // row being made
                         TBOX blob_box, // for next_blob // how many blanks?
                         int16_t prev_gap, TBOX prev_blob_box, int16_t real_current_gap,
                         int16_t within_xht_current_gap, TBOX next_blob_box, int16_t next_gap,
                         uint8_t &blanks, bool &fuzzy_sp, bool &fuzzy_non,
                         bool &prev_gap_was_a_space, bool &break_at_next_gap);
  bool narrow_blob(TO_ROW *row, TBOX blob_box);
  bool wide_blob(TO_ROW *row, TBOX blob_box);
  bool suspected_punct_blob(TO_ROW *row, TBOX box);
  void peek_at_next_gap(TO_ROW *row, BLOBNBOX_IT box_it, TBOX &next_blob_box, int16_t &next_gap,
                        int16_t &next_within_xht_gap);
  void mark_gap(TBOX blob,    // blob following gap
                int16_t rule, // heuristic id
                int16_t prev_gap, int16_t prev_blob_width, int16_t current_gap,
                int16_t next_blob_width, int16_t next_gap);
  float find_mean_blob_spacing(WERD *word);
  bool ignore_big_gap(TO_ROW *row, int32_t row_length, GAPMAP *gapmap, int16_t left, int16_t right);
  // get bounding box
  TBOX reduced_box_next(TO_ROW *row,    // current row
                        BLOBNBOX_IT *it // iterator to blobds
  );
  TBOX reduced_box_for_blob(BLOBNBOX *blob, TO_ROW *row, int16_t *left_above_xht);
  // tordmain.cpp ///////////////////////////////////////////
  float filter_noise_blobs(BLOBNBOX_LIST *src_list, BLOBNBOX_LIST *noise_list,
                           BLOBNBOX_LIST *small_list, BLOBNBOX_LIST *large_list);
  // Fixes the block so it obeys all the rules:
  // Must have at least one ROW.
  // Must have at least one WERD.
  // WERDs contain a fake blob.
  void cleanup_nontext_block(BLOCK *block);
  void cleanup_blocks(bool clean_noise, BLOCK_LIST *blocks);
  bool clean_noise_from_row(ROW *row);
  void clean_noise_from_words(ROW *row);
  // Remove outlines that are a tiny fraction in either width or height
  // of the word height.
  void clean_small_noise_from_words(ROW *row);
  // Groups blocks by rotation, then, for each group, makes a WordGrid and calls
  // TransferDiacriticsToWords to copy the diacritic blobs to the most
  // appropriate words in the group of blocks. Source blobs are not touched.
  void TransferDiacriticsToBlockGroups(BLOBNBOX_LIST *diacritic_blobs, BLOCK_LIST *blocks);
  // Places a copy of blobs that are near a word (after applying rotation to the
  // blob) in the most appropriate word, unless there is doubt, in which case a
  // blob can end up in two words. Source blobs are not touched.
  void TransferDiacriticsToWords(BLOBNBOX_LIST *diacritic_blobs, const FCOORD &rotation,
                                 WordGrid *word_grid);

public:
  // makerow.cpp ///////////////////////////////////////////
  BOOL_VAR_H(textord_single_height_mode);
  // tospace.cpp ///////////////////////////////////////////
  BOOL_VAR_H(tosp_old_to_method);
  BOOL_VAR_H(tosp_old_to_constrain_sp_kn);
  BOOL_VAR_H(tosp_only_use_prop_rows);
  BOOL_VAR_H(tosp_force_wordbreak_on_punct);
  BOOL_VAR_H(tosp_use_pre_chopping);
  BOOL_VAR_H(tosp_old_to_bug_fix);
  BOOL_VAR_H(tosp_block_use_cert_spaces);
  BOOL_VAR_H(tosp_row_use_cert_spaces);
  BOOL_VAR_H(tosp_narrow_blobs_not_cert);
  BOOL_VAR_H(tosp_row_use_cert_spaces1);
  BOOL_VAR_H(tosp_recovery_isolated_row_stats);
  BOOL_VAR_H(tosp_only_small_gaps_for_kern);
  BOOL_VAR_H(tosp_all_flips_fuzzy);
  BOOL_VAR_H(tosp_fuzzy_limit_all);
  BOOL_VAR_H(tosp_stats_use_xht_gaps);
  BOOL_VAR_H(tosp_use_xht_gaps);
  BOOL_VAR_H(tosp_only_use_xht_gaps);
  BOOL_VAR_H(tosp_rule_9_test_punct);
  BOOL_VAR_H(tosp_flip_fuzz_kn_to_sp);
  BOOL_VAR_H(tosp_flip_fuzz_sp_to_kn);
  BOOL_VAR_H(tosp_improve_thresh);
  INT_VAR_H(tosp_debug_level);
  INT_VAR_H(tosp_enough_space_samples_for_median);
  INT_VAR_H(tosp_redo_kern_limit);
  INT_VAR_H(tosp_few_samples);
  INT_VAR_H(tosp_short_row);
  INT_VAR_H(tosp_sanity_method);
  double_VAR_H(tosp_old_sp_kn_th_factor);
  double_VAR_H(tosp_threshold_bias1);
  double_VAR_H(tosp_threshold_bias2);
  double_VAR_H(tosp_narrow_fraction);
  double_VAR_H(tosp_narrow_aspect_ratio);
  double_VAR_H(tosp_wide_fraction);
  double_VAR_H(tosp_wide_aspect_ratio);
  double_VAR_H(tosp_fuzzy_space_factor);
  double_VAR_H(tosp_fuzzy_space_factor1);
  double_VAR_H(tosp_fuzzy_space_factor2);
  double_VAR_H(tosp_gap_factor);
  double_VAR_H(tosp_kern_gap_factor1);
  double_VAR_H(tosp_kern_gap_factor2);
  double_VAR_H(tosp_kern_gap_factor3);
  double_VAR_H(tosp_ignore_big_gaps);
  double_VAR_H(tosp_ignore_very_big_gaps);
  double_VAR_H(tosp_rep_space);
  double_VAR_H(tosp_enough_small_gaps);
  double_VAR_H(tosp_table_kn_sp_ratio);
  double_VAR_H(tosp_table_xht_sp_ratio);
  double_VAR_H(tosp_table_fuzzy_kn_sp_ratio);
  double_VAR_H(tosp_fuzzy_kn_fraction);
  double_VAR_H(tosp_fuzzy_sp_fraction);
  double_VAR_H(tosp_min_sane_kn_sp);
  double_VAR_H(tosp_init_guess_kn_mult);
  double_VAR_H(tosp_init_guess_xht_mult);
  double_VAR_H(tosp_max_sane_kn_thresh);
  double_VAR_H(tosp_flip_caution);
  double_VAR_H(tosp_large_kerning);
  double_VAR_H(tosp_dont_fool_with_small_kerns);
  double_VAR_H(tosp_near_lh_edge);
  double_VAR_H(tosp_silly_kn_sp_gap);
  double_VAR_H(tosp_pass_wide_fuzz_sp_to_context);
  // tordmain.cpp ///////////////////////////////////////////
  BOOL_VAR_H(textord_no_rejects);
  BOOL_VAR_H(textord_show_blobs);
  BOOL_VAR_H(textord_show_boxes);
  INT_VAR_H(textord_max_noise_size);
  INT_VAR_H(textord_baseline_debug);
  double_VAR_H(textord_noise_area_ratio);
  double_VAR_H(textord_initialx_ile);
  double_VAR_H(textord_initialasc_ile);
  INT_VAR_H(textord_noise_sizefraction);
  double_VAR_H(textord_noise_sizelimit);
  INT_VAR_H(textord_noise_translimit);
  double_VAR_H(textord_noise_normratio);
  BOOL_VAR_H(textord_noise_rejwords);
  BOOL_VAR_H(textord_noise_rejrows);
  double_VAR_H(textord_noise_syfract);
  double_VAR_H(textord_noise_sxfract);
  double_VAR_H(textord_noise_hfract);
  INT_VAR_H(textord_noise_sncount);
  double_VAR_H(textord_noise_rowratio);
  BOOL_VAR_H(textord_noise_debug);
  double_VAR_H(textord_blshift_maxshift);
  double_VAR_H(textord_blshift_xfraction);
};

} // namespace tesseract

#endif // TESSERACT_TEXTORD_TEXTORD_H_
