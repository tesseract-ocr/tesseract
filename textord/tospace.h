/**********************************************************************
 * to_spacing
 *
 * Compute fuzzy word spacing thresholds for each row.
 **********************************************************************/

#ifndef           TOSPACE_H
#define           TOSPACE_H

#include          "blobbox.h"
#include          "gap_map.h"
#include          "statistc.h"
#include          "notdll.h"

extern BOOL_VAR_H(tosp_old_to_method, FALSE, "Space stats use prechopping?");
extern BOOL_VAR_H(tosp_only_use_prop_rows, TRUE,
"Block stats to use fixed pitch rows?");
extern BOOL_VAR_H(tosp_use_pre_chopping, FALSE,
"Space stats use prechopping?");
extern BOOL_VAR_H(tosp_old_to_bug_fix, FALSE,
"Fix suspected bug in old code");
extern BOOL_VAR_H(tosp_block_use_cert_spaces, TRUE,
"Only stat OBVIOUS spaces");
extern BOOL_VAR_H(tosp_row_use_cert_spaces, TRUE,
"Only stat OBVIOUS spaces");
extern BOOL_VAR_H(tosp_narrow_blobs_not_cert, TRUE,
"Only stat OBVIOUS spaces");
extern BOOL_VAR_H(tosp_row_use_cert_spaces1, TRUE,
"Only stat OBVIOUS spaces");
extern BOOL_VAR_H(tosp_recovery_isolated_row_stats, TRUE,
"Use row alone when inadequate cert spaces");
extern BOOL_VAR_H(tosp_force_wordbreak_on_punct, FALSE,
"Force word breaks on punct to break long lines in non-space delimited langs");
extern BOOL_VAR_H(tosp_only_small_gaps_for_kern, FALSE, "Better guess");
extern BOOL_VAR_H(tosp_all_flips_fuzzy, FALSE, "Pass ANY flip to context?");
extern BOOL_VAR_H(tosp_fuzzy_limit_all, TRUE,
"Dont restrict kn->sp fuzzy limit to tables");
extern BOOL_VAR_H(tosp_stats_use_xht_gaps, TRUE,
"Use within xht gap for wd breaks");
extern BOOL_VAR_H(tosp_use_xht_gaps, TRUE,
"Use within xht gap for wd breaks");
extern BOOL_VAR_H(tosp_only_use_xht_gaps, FALSE,
"Only use within xht gap for wd breaks");
extern BOOL_VAR_H(tosp_rule_9_test_punct, FALSE,
"Dont chng kn to space next to punct");
extern BOOL_VAR_H(tosp_flip_fuzz_kn_to_sp, TRUE, "Default flip");
extern BOOL_VAR_H(tosp_flip_fuzz_sp_to_kn, TRUE, "Default flip");
extern BOOL_VAR_H(tosp_improve_thresh, FALSE,
"Enable improvement heuristic");
extern INT_VAR_H(tosp_debug_level, 0, "Debug data");
extern INT_VAR_H(tosp_enough_space_samples_for_median, 3,
"or should we use mean");
extern INT_VAR_H(tosp_redo_kern_limit, 10,
"No.samples reqd to reestimate for row");
extern INT_VAR_H(tosp_few_samples, 40,
"No.gaps reqd with 1 large gap to treat as a table");
extern INT_VAR_H(tosp_short_row, 20,
"No.gaps reqd with few cert spaces to use certs");
extern INT_VAR_H(tosp_sanity_method, 1, "How to avoid being silly");
extern double_VAR_H(tosp_threshold_bias1, 0,
"how far between kern and space?");
extern double_VAR_H(tosp_threshold_bias2, 0,
"how far between kern and space?");
extern double_VAR_H(tosp_narrow_fraction, 0.3,
"Fract of xheight for narrow");
extern double_VAR_H(tosp_narrow_aspect_ratio, 0.48,
"narrow if w/h less than this");
extern double_VAR_H(tosp_wide_fraction, 0.52, "Fract of xheight for wide");
extern double_VAR_H(tosp_wide_aspect_ratio, 0.0,
"wide if w/h less than this");
extern double_VAR_H(tosp_fuzzy_space_factor, 0.6,
"Fract of xheight for fuzz sp");
extern double_VAR_H(tosp_fuzzy_space_factor1, 0.5,
"Fract of xheight for fuzz sp");
extern double_VAR_H(tosp_fuzzy_space_factor2, 0.72,
"Fract of xheight for fuzz sp");
extern double_VAR_H(tosp_gap_factor, 0.83, "gap ratio to flip sp->kern");
extern double_VAR_H(tosp_kern_gap_factor1, 2.0,
"gap ratio to flip kern->sp");
extern double_VAR_H(tosp_kern_gap_factor2, 1.3,
"gap ratio to flip kern->sp");
extern double_VAR_H(tosp_kern_gap_factor3, 2.5,
"gap ratio to flip kern->sp");
extern double_VAR_H(tosp_ignore_big_gaps, -1, "xht multiplier");
extern double_VAR_H(tosp_ignore_very_big_gaps, 3.5, "xht multiplier");
extern double_VAR_H(tosp_rep_space, 1.6, "rep gap multiplier for space");
extern double_VAR_H(tosp_enough_small_gaps, 0.65,
"Fract of kerns reqd for isolated row stats");
extern double_VAR_H(tosp_table_kn_sp_ratio, 2.25,
"Min difference of kn & sp in table");
extern double_VAR_H(tosp_table_xht_sp_ratio, 0.33,
"Expect spaces bigger than this");
extern double_VAR_H(tosp_table_fuzzy_kn_sp_ratio, 3.0,
"Fuzzy if less than this");
extern double_VAR_H(tosp_fuzzy_kn_fraction, 0.5, "New fuzzy kn alg");
extern double_VAR_H(tosp_fuzzy_sp_fraction, 0.5, "New fuzzy sp alg");
extern double_VAR_H(tosp_min_sane_kn_sp, 1.5,
"Dont trust spaces less than this time kn");
extern double_VAR_H(tosp_init_guess_kn_mult, 2.2,
"Thresh guess - mult kn by this");
extern double_VAR_H(tosp_init_guess_xht_mult, 0.28,
"Thresh guess - mult xht by this");
extern double_VAR_H(tosp_max_sane_kn_thresh, 5.0,
"Multiplier on kn to limit thresh");
extern double_VAR_H(tosp_flip_caution, 0.0,
"Dont autoflip kn to sp when large separation");
extern double_VAR_H(tosp_large_kerning, 0.19,
"Limit use of xht gap with large kns");
extern double_VAR_H(tosp_dont_fool_with_small_kerns, -1,
"Limit use of xht gap with odd small kns");
extern double_VAR_H(tosp_near_lh_edge, 0,
"Dont reduce box if the top left is non blank");
extern double_VAR_H(tosp_silly_kn_sp_gap, 0.2,
"Dont let sp minus kn get too small");
extern double_VAR_H(tosp_pass_wide_fuzz_sp_to_context, 0.75,
"How wide fuzzies need context");

void to_spacing(                       //set spacing
                ICOORD page_tr,        //topright of page
                TO_BLOCK_LIST *blocks  //blocks on page
               );
                                 //DEBUG USE ONLY
void block_spacing_stats(TO_BLOCK *block,
                         GAPMAP *gapmap,
                         BOOL8 &old_text_ord_proportional,
                         inT16 &block_space_gap_width,     //resulting estimate
                         inT16 &block_non_space_gap_width  //resulting estimate
                        );
                                 //estimate for block
void row_spacing_stats(TO_ROW *row,
                       GAPMAP *gapmap,
                       inT16 block_idx,
                       inT16 row_idx,
                       inT16 block_space_gap_width,
                       inT16 block_non_space_gap_width  //estimate for block
                      );
                                 //estimate for block
void old_to_method(TO_ROW *row,
                   STATS *all_gap_stats,
                   STATS *space_gap_stats,
                   STATS *small_gap_stats,
                   inT16 block_space_gap_width,
                   inT16 block_non_space_gap_width  //estimate for block
                  );
BOOL8 isolated_row_stats(TO_ROW *row,
                         GAPMAP *gapmap,
                         STATS *all_gap_stats,
                         BOOL8 suspected_table,
                         inT16 block_idx,
                         inT16 row_idx);
inT16 stats_count_under(STATS *stats, inT16 threshold);
void improve_row_threshold(TO_ROW *row, STATS *all_gap_stats);
ROW *make_prop_words(                 // find lines
                     TO_ROW *row,     // row to make
                     FCOORD rotation  // for drawing
                    );
ROW *make_blob_words(                 // find lines
                     TO_ROW *row,     // row to make
                     FCOORD rotation  // for drawing
                    );
BOOL8 make_a_word_break(               // decide on word break
                        TO_ROW *row,   // row being made
                        TBOX blob_box, // for next_blob // how many blanks?
                        inT16 prev_gap,
                        TBOX prev_blob_box,
                        inT16 real_current_gap,
                        inT16 within_xht_current_gap,
                        TBOX next_blob_box,
                        inT16 next_gap,
                        uinT8 &blanks,
                        BOOL8 &fuzzy_sp,
                        BOOL8 &fuzzy_non);
BOOL8 narrow_blob(TO_ROW *row, TBOX blob_box);
BOOL8 wide_blob(TO_ROW *row, TBOX blob_box);
BOOL8 suspected_punct_blob(TO_ROW *row, TBOX box);
                                 //A COPY FOR PEEKING
void peek_at_next_gap(TO_ROW *row,
                      BLOBNBOX_IT box_it,
                      TBOX &next_blob_box,
                      inT16 &next_gap,
                      inT16 &next_within_xht_gap);
void mark_gap(             //Debug stuff
              TBOX blob,    //blob following gap
              inT16 rule,  // heuristic id
              inT16 prev_gap,
              inT16 prev_blob_width,
              inT16 current_gap,
              inT16 next_blob_width,
              inT16 next_gap);
float find_mean_blob_spacing(WERD *word);
BOOL8 ignore_big_gap(TO_ROW *row,
                     inT32 row_length,
                     GAPMAP *gapmap,
                     inT16 left,
                     inT16 right);
TBOX reduced_box_next(                 //get bounding box
                     TO_ROW *row,     //current row
                     BLOBNBOX_IT *it  //iterator to blobds
                    );
TBOX reduced_box_for_blob(BLOBNBOX *blob, TO_ROW *row, inT16 *left_above_xht);
#endif
