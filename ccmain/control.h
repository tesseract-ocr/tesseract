/**********************************************************************
 * File:        control.h  (Formerly control.h)
 * Description: Module-independent matcher controller.
 * Author:		Ray Smith
 * Created:		Thu Apr 23 11:09:58 BST 1992
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

/**
 * @file control.h
 * Module-independent matcher controller.
 */

#ifndef           CONTROL_H
#define           CONTROL_H

#include          "varable.h"
#include          "ocrblock.h"
//#include                                      "epapdest.h"
#include          "ratngs.h"
#include          "statistc.h"
//#include                                      "epapconv.h"
#include          "ocrshell.h"
#include          "pageres.h"
//TODO (wanke) why does the app. path have to be so weird here?
#include          "charsample.h"
#include          "notdll.h"

enum ACCEPTABLE_WERD_TYPE
{
  AC_UNACCEPTABLE,               ///< Unacceptable word
  AC_LOWER_CASE,                 ///< ALL lower case
  AC_UPPER_CASE,                 ///< ALL upper case
  AC_INITIAL_CAP,                ///< ALL but initial lc
  AC_LC_ABBREV,                  ///< a.b.c.
  AC_UC_ABBREV                   ///< A.B.C.
};

typedef BOOL8 (*BLOB_REJECTOR) (PBLOB *, BLOB_CHOICE_IT *, void *);

extern INT_VAR_H (tessedit_single_match, FALSE, "Top choice only from CP");
//extern BOOL_VAR_H(tessedit_small_match,FALSE,"Use small matrix matcher");
extern BOOL_VAR_H (tessedit_print_text, FALSE, "Write text to stdout");
extern BOOL_VAR_H (tessedit_draw_words, FALSE, "Draw source words");
extern BOOL_VAR_H (tessedit_draw_outwords, FALSE, "Draw output words");
extern BOOL_VAR_H (tessedit_training_wiseowl, FALSE,
"Call WO to learn blobs");
extern BOOL_VAR_H (tessedit_training_tess, FALSE, "Call Tess to learn blobs");
extern BOOL_VAR_H (tessedit_matcher_is_wiseowl, FALSE, "Call WO to classify");
extern BOOL_VAR_H (tessedit_dump_choices, FALSE, "Dump char choices");
extern BOOL_VAR_H (tessedit_fix_fuzzy_spaces, TRUE,
"Try to improve fuzzy spaces");
extern BOOL_VAR_H (tessedit_unrej_any_wd, FALSE,
"Dont bother with word plausibility");
extern BOOL_VAR_H (tessedit_fix_hyphens, TRUE, "Crunch double hyphens?");
extern BOOL_VAR_H (tessedit_reject_fullstops, FALSE, "Reject all fullstops");
extern BOOL_VAR_H (tessedit_reject_suspect_fullstops, FALSE,
"Reject suspect fullstops");
extern BOOL_VAR_H (tessedit_redo_xheight, TRUE, "Check/Correct x-height");
extern BOOL_VAR_H (tessedit_cluster_adaption_on, TRUE,
"Do our own adaption - ems only");
extern BOOL_VAR_H (tessedit_enable_doc_dict, TRUE,
"Add words to the document dictionary");
extern BOOL_VAR_H (word_occ_first, FALSE, "Do word occ before re-est xht");
extern BOOL_VAR_H (tessedit_xht_fiddles_on_done_wds, TRUE,
"Apply xht fix up even if done");
extern BOOL_VAR_H (tessedit_xht_fiddles_on_no_rej_wds, TRUE,
"Apply xht fix up even in no rejects");
extern INT_VAR_H (x_ht_check_word_occ, 2, "Check Char Block occupancy");
extern INT_VAR_H (x_ht_stringency, 1, "How many confirmed a/n to accept?");
extern BOOL_VAR_H (x_ht_quality_check, TRUE, "Dont allow worse quality");
extern BOOL_VAR_H (tessedit_debug_block_rejection, FALSE,
"Block and Row stats");
extern INT_VAR_H (debug_x_ht_level, 0, "Reestimate debug");
extern BOOL_VAR_H (rej_use_xht, TRUE, "Individual rejection control");
extern BOOL_VAR_H (debug_acceptable_wds, FALSE, "Dump word pass/fail chk");
extern STRING_VAR_H (chs_leading_punct, "('`\"", "Leading punctuation");
extern
STRING_VAR_H (chs_trailing_punct1, ").,;:?!", "1st Trailing punctuation");
extern STRING_VAR_H (chs_trailing_punct2, ")'`\"",
"2nd Trailing punctuation");
extern double_VAR_H (quality_rej_pc, 0.08,
"good_quality_doc lte rejection limit");
extern double_VAR_H (quality_blob_pc, 0.0,
"good_quality_doc gte good blobs limit");
extern double_VAR_H (quality_outline_pc, 1.0,
"good_quality_doc lte outline error limit");
extern double_VAR_H (quality_char_pc, 0.95,
"good_quality_doc gte good char limit");
extern INT_VAR_H (quality_min_initial_alphas_reqd, 2,
"alphas in a good word");
extern BOOL_VAR_H (tessedit_tess_adapt_to_rejmap, FALSE,
"Use reject map to control Tesseract adaption");
extern INT_VAR_H (tessedit_tess_adaption_mode, 3,
"Adaptation decision algorithm for tess");
extern INT_VAR_H (tessedit_em_adaption_mode, 62,
"Adaptation decision algorithm for ems matrix matcher");
extern BOOL_VAR_H (tessedit_cluster_adapt_after_pass1, FALSE,
"Adapt using clusterer after pass 1");
extern BOOL_VAR_H (tessedit_cluster_adapt_after_pass2, FALSE,
"Adapt using clusterer after pass 1");
extern BOOL_VAR_H (tessedit_cluster_adapt_after_pass3, FALSE,
"Adapt using clusterer after pass 1");
extern BOOL_VAR_H (tessedit_cluster_adapt_before_pass1, FALSE,
"Adapt using clusterer before Tess adaping during pass 1");
extern INT_VAR_H (tessedit_cluster_adaption_mode, 0,
"Adaptation decision algorithm for matrix matcher");
extern BOOL_VAR_H (tessedit_adaption_debug, FALSE,
"Generate and print debug information for adaption");
extern BOOL_VAR_H (tessedit_minimal_rej_pass1, FALSE,
"Do minimal rejection on pass 1 output");
extern BOOL_VAR_H (tessedit_test_adaption, FALSE,
"Test adaption criteria");
extern BOOL_VAR_H (tessedit_global_adaption, FALSE,
"Adapt to all docs over time");
extern BOOL_VAR_H (tessedit_matcher_log, FALSE, "Log matcher activity");
extern INT_VAR_H (tessedit_test_adaption_mode, 3,
"Adaptation decision algorithm for tess");
extern BOOL_VAR_H (test_pt, FALSE, "Test for point");
extern double_VAR_H (test_pt_x, 99999.99, "xcoord");
extern double_VAR_H (test_pt_y, 99999.99, "ycoord");
extern BOOL_VAR_H(save_best_choices, FALSE,
                  "Save the results of the recognition step"
                  " (blob_choices) within the corresponding WERD_CHOICE");

/*
void classify_word_pass1(                 //recog one word
                         WERD_RES *word,  //word to do
                         ROW *row,
                         BOOL8 cluster_adapt,
                         CHAR_SAMPLES_LIST *char_clusters,
                         CHAR_SAMPLE_LIST *chars_waiting);
*/
                                 //word to do
void classify_word_pass2(WERD_RES *word, ROW *row);
/**
 * recognize one word
 * @param word word to do
 */
void match_word_pass2(
                      WERD_RES *word,
                      ROW *row,
                      float x_height);
/**
 * crunch double hyphens
 * @param choice string to fix
 * @param word word to do
 * @param blob_choices char choices
 */
void fix_hyphens(
                 WERD_CHOICE *choice,
                 WERD *word,
                 BLOB_CHOICE_LIST_CLIST *blob_choices);

/**
 * combine 2 blobs
 * @param blob1 dest blob
 * @param blob2 source blob
 */
void merge_blobs(
                 PBLOB *blob1,
                 PBLOB *blob2
                );
/** dump chars in word */
void choice_dump_tester(
                        PBLOB *,                   ///< blob
                        DENORM *,                  ///< de-normaliser
                        BOOL8 correct,             ///< ly segmented
                        char *text,                ///< correct text
                        inT32 count,               ///< chars in text
                        BLOB_CHOICE_LIST *ratings  ///< list of results
                       );
WERD *make_bln_copy(WERD *src_word, ROW *row, BLOCK* block,
                    float x_height, DENORM *denorm);
BOOL8 check_debug_pt(WERD_RES *word, int location);
/** good chars in word */
void add_in_one_row(
                    ROW_RES *row,  ///< current row
                    STATS *fonts,  ///< font stats
                    inT8 *italic,  ///< output count
                    inT8 *bold     ///< output count
                   );
/** good chars in word */
void find_modal_font(
                     STATS *fonts,     ///< font stats
                     inT8 *font_out,   ///< output font
                     inT8 *font_count  ///< output count
                    );
#endif
