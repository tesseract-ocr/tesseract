/******************************************************************
 * File:        docqual.h  (Formerly docqual.h)
 * Description: Document Quality Metrics
 * Author:		Phil Cheatle
 * Created:		Mon May  9 11:27:28 BST 1994
 *
 * (C) Copyright 1994, Hewlett-Packard Ltd.
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

#ifndef           DOCQUAL_H
#define           DOCQUAL_H

#include          "control.h"
#include          "notdll.h"

enum GARBAGE_LEVEL
{
  G_NEVER_CRUNCH,
  G_OK,
  G_DODGY,
  G_TERRIBLE
};

extern STRING_VAR_H (outlines_odd, "%| ", "Non standard number of outlines");
extern STRING_VAR_H (outlines_2, "ij!?%\":;",
"Non standard number of outlines");
extern BOOL_VAR_H (docqual_excuse_outline_errs, FALSE,
"Allow outline errs in unrejection?");
extern BOOL_VAR_H (tessedit_good_quality_unrej, TRUE,
"Reduce rejection on good docs");
extern BOOL_VAR_H (tessedit_use_reject_spaces, TRUE, "Reject spaces?");
extern double_VAR_H (tessedit_reject_doc_percent, 65.00,
"%rej allowed before rej whole doc");
extern double_VAR_H (tessedit_reject_block_percent, 45.00,
"%rej allowed before rej whole block");
extern double_VAR_H (tessedit_reject_row_percent, 40.00,
"%rej allowed before rej whole row");
extern double_VAR_H (tessedit_whole_wd_rej_row_percent, 70.00,
"%of row rejects in whole word rejects which prevents whole row rejection");
extern BOOL_VAR_H (tessedit_preserve_blk_rej_perfect_wds, TRUE,
"Only rej partially rejected words in block rejection");
extern BOOL_VAR_H (tessedit_preserve_row_rej_perfect_wds, TRUE,
"Only rej partially rejected words in row rejection");
extern BOOL_VAR_H (tessedit_dont_blkrej_good_wds, FALSE,
"Use word segmentation quality metric");
extern BOOL_VAR_H (tessedit_dont_rowrej_good_wds, FALSE,
"Use word segmentation quality metric");
extern INT_VAR_H (tessedit_preserve_min_wd_len, 2,
"Only preserve wds longer than this");
extern BOOL_VAR_H (tessedit_row_rej_good_docs, TRUE,
"Apply row rejection to good docs");
extern double_VAR_H (tessedit_good_doc_still_rowrej_wd, 1.1,
"rej good doc wd if more than this fraction rejected");
extern BOOL_VAR_H (tessedit_reject_bad_qual_wds, TRUE,
"Reject all bad quality wds");
extern BOOL_VAR_H (tessedit_debug_doc_rejection, FALSE, "Page stats");
extern BOOL_VAR_H (tessedit_debug_quality_metrics, FALSE,
"Output data to debug file");
extern BOOL_VAR_H (bland_unrej, FALSE, "unrej potential with no chekcs");
extern double_VAR_H (quality_rowrej_pc, 1.1,
"good_quality_doc gte good char limit");
extern BOOL_VAR_H (unlv_tilde_crunching, TRUE,
"Mark v.bad words for tilde crunch");
extern BOOL_VAR_H (crunch_early_merge_tess_fails, TRUE,
"Before word crunch?");
extern BOOL_VAR_H (crunch_early_convert_bad_unlv_chs, FALSE,
"Take out ~^ early?");
extern double_VAR_H (crunch_terrible_rating, 80.0, "crunch rating lt this");
extern BOOL_VAR_H (crunch_terrible_garbage, TRUE, "As it says");
extern double_VAR_H (crunch_poor_garbage_cert, -9.0,
"crunch garbage cert lt this");
extern double_VAR_H (crunch_poor_garbage_rate, 60,
"crunch garbage rating lt this");
extern double_VAR_H (crunch_pot_poor_rate, 40,
"POTENTIAL crunch rating lt this");
extern double_VAR_H (crunch_pot_poor_cert, -8.0,
"POTENTIAL crunch cert lt this");
extern BOOL_VAR_H (crunch_pot_garbage, TRUE, "POTENTIAL crunch garbage");
extern double_VAR_H (crunch_del_rating, 60,
"POTENTIAL crunch rating lt this");
extern double_VAR_H (crunch_del_cert, -10.0, "POTENTIAL crunch cert lt this");
extern double_VAR_H (crunch_del_min_ht, 0.7, "Del if word ht lt xht x this");
extern double_VAR_H (crunch_del_max_ht, 3.0, "Del if word ht gt xht x this");
extern double_VAR_H (crunch_del_min_width, 3.0,
"Del if word width lt xht x this");
extern double_VAR_H (crunch_del_high_word, 1.5,
"Del if word gt xht x this above bl");
extern double_VAR_H (crunch_del_low_word, 0.5,
"Del if word gt xht x this below bl");
extern double_VAR_H (crunch_small_outlines_size, 0.6,
"Small if lt xht x this");
extern INT_VAR_H (crunch_rating_max, 10, "For adj length in rating per ch");
extern INT_VAR_H (crunch_pot_indicators, 1,
"How many potential indicators needed");
extern BOOL_VAR_H (crunch_leave_ok_strings, TRUE,
"Dont touch sensible strings");
extern BOOL_VAR_H (crunch_accept_ok, TRUE, "Use acceptability in okstring");
extern BOOL_VAR_H (crunch_leave_accept_strings, FALSE,
"Dont pot crunch sensible strings");
extern BOOL_VAR_H (crunch_include_numerals, FALSE, "Fiddle alpha figures");
extern INT_VAR_H (crunch_leave_lc_strings, 4,
"Dont crunch words with long lower case strings");
extern INT_VAR_H (crunch_leave_uc_strings, 4,
"Dont crunch words with long lower case strings");
extern INT_VAR_H (crunch_long_repetitions, 3,
"Crunch words with long repetitions");
extern INT_VAR_H (crunch_debug, 0, "As it says");
inT16 word_blob_quality(  //Blob seg changes
                        WERD_RES *word,
                        ROW *row);
//BOOL8 crude_match_blobs(PBLOB *blob1, PBLOB *blob2);
inT16 word_outline_errs(  //Outline count errs
                        WERD_RES *word);
void word_char_quality(  //Blob seg changes
                       WERD_RES *word,
                       ROW *row,
                       inT16 *match_count,
                       inT16 *accepted_match_count);
//void unrej_good_chs(WERD_RES *word, ROW *row);
void print_boxes(WERD *word);
inT16 count_outline_errs(char c, inT16 outline_count);
void reject_whole_page(PAGE_RES_IT &page_res_it);
BOOL8 terrible_word_crunch(WERD_RES *word, GARBAGE_LEVEL garbage_level);
                                 //word to do
CRUNCH_MODE word_deletable(WERD_RES *word, inT16 &delete_mode);
inT16 failure_count(WERD_RES *word);
BOOL8 noise_outlines(WERD *word);
#endif
