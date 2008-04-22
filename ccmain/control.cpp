/******************************************************************
 * File:        control.cpp  (Formerly control.c)
 * Description: Module-independent matcher controller.
 * Author:					Ray Smith
 * Created:					Thu Apr 23 11:09:58 BST 1992
 * ReHacked:    Tue Sep 22 08:42:49 BST 1992 Phil Cheatle
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

#include          "mfcpch.h"
#include          "mainblk.h"
#include          <string.h>
#include          <math.h>
#ifdef __UNIX__
#include          <assert.h>
#include          <unistd.h>
#include                    <errno.h>
#endif
#include          <ctype.h>
#include          "ocrclass.h"
#include          "werdit.h"
#include          "drawfx.h"
#include          "tfacep.h"
#include          "tessbox.h"
#include          "tessvars.h"
//#include                                      "fxtop.h"
#include          "pgedit.h"
#include          "reject.h"
#include          "adaptions.h"
#include          "charcut.h"
#include          "fixxht.h"
#include          "fixspace.h"
#include          "genblob.h"
#include          "docqual.h"
#include          "control.h"
#include          "secname.h"
#include          "output.h"
#include          "callcpp.h"
#include          "notdll.h"
#include "tordvars.h"
#include "adaptmatch.h"
#include "globals.h"

#define MIN_FONT_ROW_COUNT  8
#define MAX_XHEIGHT_DIFF  3

#define EXTERN
//extern "C" {
//EXTERN BOOL_VAR(tessedit_small_match,FALSE,"Use small matrix matcher");

//extern FILE*                          matcher_fp;
//extern FILE*                          correct_fp;
//};
BOOL_VAR (tessedit_small_match, FALSE, "Use small matrix matcher");
EXTERN BOOL_VAR (tessedit_print_text, FALSE, "Write text to stdout");
EXTERN BOOL_VAR (tessedit_draw_words, FALSE, "Draw source words");
EXTERN BOOL_VAR (tessedit_draw_outwords, FALSE, "Draw output words");
EXTERN BOOL_VAR (tessedit_training_wiseowl, FALSE, "Call WO to learn blobs");
EXTERN BOOL_VAR (tessedit_training_tess, FALSE, "Call Tess to learn blobs");
EXTERN BOOL_VAR (tessedit_matcher_is_wiseowl, FALSE, "Call WO to classify");
EXTERN BOOL_VAR (tessedit_dump_choices, FALSE, "Dump char choices");
EXTERN BOOL_VAR (tessedit_fix_fuzzy_spaces, TRUE,
"Try to improve fuzzy spaces");
EXTERN BOOL_VAR (tessedit_unrej_any_wd, FALSE,
"Dont bother with word plausibility");
EXTERN BOOL_VAR (tessedit_fix_hyphens, TRUE, "Crunch double hyphens?");

EXTERN BOOL_VAR (tessedit_reject_fullstops, FALSE, "Reject all fullstops");
EXTERN BOOL_VAR (tessedit_reject_suspect_fullstops, FALSE,
"Reject suspect fullstops");
EXTERN BOOL_VAR (tessedit_redo_xheight, TRUE, "Check/Correct x-height");
EXTERN BOOL_VAR (tessedit_cluster_adaption_on, TRUE,
"Do our own adaption - ems only");
EXTERN BOOL_VAR (tessedit_enable_doc_dict, TRUE,
"Add words to the document dictionary");
EXTERN BOOL_VAR (word_occ_first, FALSE, "Do word occ before re-est xht");
EXTERN BOOL_VAR (tessedit_debug_fonts, FALSE, "Output font info per char");
EXTERN BOOL_VAR (tessedit_xht_fiddles_on_done_wds, TRUE,
"Apply xht fix up even if done");
EXTERN BOOL_VAR (tessedit_xht_fiddles_on_no_rej_wds, TRUE,
"Apply xht fix up even in no rejects");
EXTERN INT_VAR (x_ht_check_word_occ, 2, "Check Char Block occupancy");
EXTERN INT_VAR (x_ht_stringency, 1, "How many confirmed a/n to accept?");
EXTERN BOOL_VAR (x_ht_quality_check, TRUE, "Dont allow worse quality");
EXTERN BOOL_VAR (tessedit_debug_block_rejection, FALSE,
"Block and Row stats");
EXTERN INT_VAR (debug_x_ht_level, 0, "Reestimate debug");
EXTERN BOOL_VAR (rej_use_xht, TRUE, "Individual rejection control");
EXTERN BOOL_VAR (debug_acceptable_wds, FALSE, "Dump word pass/fail chk");

EXTERN STRING_VAR (chs_leading_punct, "('`\"", "Leading punctuation");
EXTERN
STRING_VAR (chs_trailing_punct1, ").,;:?!", "1st Trailing punctuation");
EXTERN STRING_VAR (chs_trailing_punct2, ")'`\"",
"2nd Trailing punctuation");

EXTERN double_VAR (quality_rej_pc, 0.08,
"good_quality_doc lte rejection limit");
EXTERN double_VAR (quality_blob_pc, 0.0,
"good_quality_doc gte good blobs limit");
EXTERN double_VAR (quality_outline_pc, 1.0,
"good_quality_doc lte outline error limit");
EXTERN double_VAR (quality_char_pc, 0.95,
"good_quality_doc gte good char limit");
EXTERN INT_VAR (quality_min_initial_alphas_reqd, 2,
"alphas in a good word");

EXTERN BOOL_VAR (tessedit_tess_adapt_to_rejmap, FALSE,
"Use reject map to control Tesseract adaption");
EXTERN INT_VAR (tessedit_tess_adaption_mode, 0x27,
"Adaptation decision algorithm for tess");
EXTERN INT_VAR (tessedit_em_adaption_mode, 0,
"Adaptation decision algorithm for ems matrix matcher");
EXTERN BOOL_VAR (tessedit_cluster_adapt_after_pass1, FALSE,
"Adapt using clusterer after pass 1");
EXTERN BOOL_VAR (tessedit_cluster_adapt_after_pass2, FALSE,
"Adapt using clusterer after pass 1");
EXTERN BOOL_VAR (tessedit_cluster_adapt_after_pass3, FALSE,
"Adapt using clusterer after pass 1");
EXTERN BOOL_VAR (tessedit_cluster_adapt_before_pass1, FALSE,
"Adapt using clusterer before Tess adaping during pass 1");
EXTERN INT_VAR (tessedit_cluster_adaption_mode, 0,
"Adaptation decision algorithm for matrix matcher");
EXTERN BOOL_VAR (tessedit_adaption_debug, FALSE,
"Generate and print debug information for adaption");
EXTERN BOOL_VAR (tessedit_minimal_rej_pass1, FALSE,
"Do minimal rejection on pass 1 output");
EXTERN BOOL_VAR (tessedit_test_adaption, FALSE,
"Test adaption criteria");
EXTERN BOOL_VAR (tessedit_global_adaption, FALSE,
"Adapt to all docs over time");
EXTERN BOOL_VAR (tessedit_matcher_log, FALSE, "Log matcher activity");
EXTERN INT_VAR (tessedit_test_adaption_mode, 3,
"Adaptation decision algorithm for tess");
BOOL_VAR (save_best_choices, FALSE, "Save the results of the recognition step"
" (blob_choices) within the corresponding WERD_CHOICE");

EXTERN BOOL_VAR (test_pt, FALSE, "Test for point");
EXTERN double_VAR (test_pt_x, 99999.99, "xcoord");
EXTERN double_VAR (test_pt_y, 99999.99, "ycoord");

extern int MatcherDebugLevel;
extern int display_ratings;
extern int number_debug;
extern int adjust_debug;
FILE *choice_file = NULL;        //Choice file ptr

CLISTIZEH (PBLOB) CLISTIZE (PBLOB)
/* DEBUGGING */
inT16 blob_count(WERD *w) {
  return w->blob_list ()->length ();
}


/**********************************************************************
 * recog_pseudo_word
 *
 * Make a word from the selected blobs and run Tess on them.
 **********************************************************************/

void recog_pseudo_word(                         //recognize blobs
                       BLOCK_LIST *block_list,  //blocks to check
                       TBOX &selection_box) {
  WERD *word;
  ROW *pseudo_row;               //row of word
  BLOCK *pseudo_block;           //block of word

  word = make_pseudo_word (block_list, selection_box,
    pseudo_block, pseudo_row);
  if (word != NULL) {
    recog_interactive(pseudo_block, pseudo_row, word);
    delete word;
  }
}


/**********************************************************************
 * recog_interactive
 *
 * Recognize a single word in interactive mode.
 **********************************************************************/

BOOL8 recog_interactive(            //recognize blobs
                        BLOCK *,    //block
                        ROW *row,   //row of word
                        WERD *word  //word to recognize
                       ) {
  WERD_RES word_res(word);
  inT16 char_qual;
  inT16 good_char_qual;

  classify_word_pass2(&word_res, row);
  #ifndef SECURE_NAMES
  if (tessedit_debug_quality_metrics) {
    word_char_quality(&word_res, row, &char_qual, &good_char_qual);
    tprintf
      ("\n%d chars;  word_blob_quality: %d;  outline_errs: %d; char_quality: %d; good_char_quality: %d\n",
      word_res.reject_map.length (), word_blob_quality (&word_res, row),
      word_outline_errs (&word_res), char_qual, good_char_qual);
  }
  #endif
  return TRUE;
}


/**********************************************************************
 * recog_all_words()
 *
 * Walk the current block list applying the specified word processor function
 * to all words
 **********************************************************************/

void recog_all_words(                              //process words
                     PAGE_RES *page_res,           //page structure
                     volatile ETEXT_DESC *monitor,  //progress monitor
                     TBOX	*target_word_box,//specifies just to extract a retangle
                     inT16 dopasses //0 - all, 1 just pass 1, 2 passes 2 and higher
					 ) {
                                 //reset page iterator
  static PAGE_RES_IT page_res_it;
  inT16 chars_in_word;
  inT16 rejects_in_word;
  static CHAR_SAMPLES_LIST em_clusters;
  static CHAR_SAMPLE_LIST ems_waiting;
  static CHAR_SAMPLES_LIST char_clusters;
  static CHAR_SAMPLE_LIST chars_waiting;
  inT16 blob_quality = 0;
  inT16 outline_errs = 0;
  static inT16 doc_blob_quality = 0;
  static inT16 doc_outline_errs = 0;
  static inT16 doc_char_quality = 0;
  inT16 all_char_quality;
  inT16 accepted_all_char_quality;
  static inT16 good_char_count = 0;
  static inT16 doc_good_char_quality = 0;
  int i;


  inT32 tess_adapt_mode = 0;
  static inT32 word_count;              //count of words in doc
  inT32 word_index;              //current word
  static int dict_words;

  if (tessedit_minimal_rej_pass1) {
    tessedit_test_adaption.set_value (TRUE);
    tessedit_minimal_rejection.set_value (TRUE);
  }

  if (tessedit_cluster_adapt_before_pass1) {
    tess_adapt_mode = tessedit_tess_adaption_mode;
    tessedit_tess_adaption_mode.set_value (0);
    tessedit_tess_adapt_to_rejmap.set_value (TRUE);
  }


if (dopasses==0 || dopasses==1)
{
	page_res_it.page_res=page_res;
	page_res_it.restart_page();

  /* Pass 1 */
  word_count = 0;
  if (monitor != NULL) {
    monitor->ocr_alive = TRUE;
    while (page_res_it.word () != NULL) {
      word_count++;
      page_res_it.forward ();
    }
    page_res_it.restart_page ();
  }
  else
    word_count = 1;

  word_index = 0;

	em_clusters.clear();
    ems_waiting.clear();
    char_clusters.clear();
    chars_waiting.clear();
    dict_words = 0;
	doc_blob_quality = 0;
	doc_outline_errs = 0;
	doc_char_quality = 0;
	good_char_count = 0;
	doc_good_char_quality = 0;

  while (page_res_it.word () != NULL) {
    set_global_loc_code(LOC_PASS1);
    word_index++;
    if (monitor != NULL) {
      monitor->ocr_alive = TRUE;
      monitor->progress = 30 + 50 * word_index / word_count;
      if ((monitor->end_time != 0 && clock() > monitor->end_time) ||
          (monitor->cancel != NULL && (*monitor->cancel)(monitor->cancel_this,
                                                         dict_words)))
        return;
    }
    classify_word_pass1 (page_res_it.word (),
      page_res_it.row ()->row, FALSE, NULL, NULL);

    if (tessedit_test_adaption && !tessedit_minimal_rejection) {
      if (!word_adaptable (page_res_it.word (),
        tessedit_test_adaption_mode))
        page_res_it.word ()->reject_map.rej_word_tess_failure ();
      //FAKE PERM REJ
      else {
        const STRING* wordstr = &(page_res_it.word ()->best_choice->string ());
        /* Override rejection mechanisms for this word */
        const char* text = wordstr->string ();
        for (i = 0; text[i] != '\0'; i++) {
          if ((text[i] != ' ')
            && page_res_it.word ()->reject_map[i].rejected ())
            page_res_it.word ()->reject_map[i].
              setrej_minimal_rej_accept();
        }
      }
    }

    if ((tessedit_cluster_adapt_after_pass1
      || tessedit_cluster_adapt_after_pass3
      || tessedit_cluster_adapt_before_pass1)
    && tessedit_cluster_adaption_mode != 0) {
      collect_characters_for_adaption (page_res_it.word (),
        &char_clusters, &chars_waiting);
    }
    // Count dict words.
    if (page_res_it.word()->best_choice->permuter() == USER_DAWG_PERM)
      ++dict_words;
    page_res_it.forward ();
  }

  if (tessedit_cluster_adapt_before_pass1)
    tessedit_tess_adaption_mode.set_value (tess_adapt_mode);

  page_res_it.restart_page ();
  while ((tessedit_cluster_adapt_after_pass1
    || tessedit_cluster_adapt_before_pass1)
  && page_res_it.word () != NULL) {
    if (monitor != NULL)
      monitor->ocr_alive = TRUE;
    if (tessedit_cluster_adapt_after_pass1)
      adapt_to_good_samples (page_res_it.word (),
        &char_clusters, &chars_waiting);
    else
      classify_word_pass1 (page_res_it.word (),
        page_res_it.row ()->row,
        TRUE, &char_clusters, &chars_waiting);

    page_res_it.forward ();
  }

  //


 }

if (dopasses==1) return;

  /* Pass 2 */
  page_res_it.restart_page ();
  word_index = 0;
  while (!tessedit_test_adaption && page_res_it.word () != NULL) {
    set_global_loc_code(LOC_PASS2);
    word_index++;
    if (monitor != NULL) {
      monitor->ocr_alive = TRUE;
      monitor->progress = 80 + 10 * word_index / word_count;
      if ((monitor->end_time != 0 && clock() > monitor->end_time) ||
          (monitor->cancel != NULL && (*monitor->cancel)(monitor->cancel_this,
                                                         dict_words)))
        return;
    }
//changed by jetsoft
//specific to its needs to extract one word when need

	if (target_word_box)
	{

		TBOX current_word_box=page_res_it.word ()->word->bounding_box();
		FCOORD center_pt((current_word_box.right()+current_word_box.left())/2,(current_word_box.bottom()+current_word_box.top())/2);
		if (!target_word_box->contains(center_pt))
		{
			page_res_it.forward ();
			continue;
		}

	}
//end jetsoft

    classify_word_pass2 (page_res_it.word (), page_res_it.row ()->row);

    if (tessedit_em_adaption_mode > 0)
      collect_ems_for_adaption (page_res_it.word (),
        &em_clusters, &ems_waiting);

    if (tessedit_cluster_adapt_after_pass2
      && tessedit_cluster_adaption_mode != 0)
      collect_characters_for_adaption (page_res_it.word (),
        &char_clusters, &chars_waiting);
    page_res_it.forward ();
  }

  /* Another pass */
  set_global_loc_code(LOC_FUZZY_SPACE);

  if (!tessedit_test_adaption && tessedit_fix_fuzzy_spaces
    && !tessedit_word_for_word)
    fix_fuzzy_spaces(monitor, word_count, page_res);

  if (!tessedit_test_adaption && tessedit_em_adaption_mode != 0)
                                 // Initially ems only
    print_em_stats(&em_clusters, &ems_waiting);

  /* Pass 3 - used for checking confusion sets */
  page_res_it.restart_page ();
  word_index = 0;
  while (!tessedit_test_adaption && page_res_it.word () != NULL) {
    set_global_loc_code(LOC_MM_ADAPT);
    word_index++;
    if (monitor != NULL) {
      monitor->ocr_alive = TRUE;
      monitor->progress = 95 + 5 * word_index / word_count;
    }
    check_debug_pt (page_res_it.word (), 70);
    /* Use good matches to sort out confusions */


//changed by jetsoft
//specific to its needs to extract one word when need

	if (target_word_box)
	{

		TBOX current_word_box=page_res_it.word ()->word->bounding_box();
		FCOORD center_pt((current_word_box.right()+current_word_box.left())/2,(current_word_box.bottom()+current_word_box.top())/2);
		if (!target_word_box->contains(center_pt))
		{
			page_res_it.forward ();
			continue;
		}

	}
// end jetsoft

    if (tessedit_em_adaption_mode != 0)
      adapt_to_good_ems (page_res_it.word (), &em_clusters, &ems_waiting);

    if (tessedit_cluster_adapt_after_pass2
      && tessedit_cluster_adaption_mode != 0)
      adapt_to_good_samples (page_res_it.word (),
        &char_clusters, &chars_waiting);

    if (tessedit_reject_fullstops
      && strchr (page_res_it.word ()->best_choice->string ().string (),
      '.') != NULL)
      reject_all_fullstops (page_res_it.word ());
    else if (tessedit_reject_suspect_fullstops
      && strchr (page_res_it.word ()->best_choice->string ().
      string (), '.') != NULL)
      reject_suspect_fullstops (page_res_it.word ());

    page_res_it.rej_stat_word ();
    chars_in_word = page_res_it.word ()->reject_map.length ();
    rejects_in_word = page_res_it.word ()->reject_map.reject_count ();

    blob_quality = word_blob_quality (page_res_it.word (),
      page_res_it.row ()->row);
    doc_blob_quality += blob_quality;
    outline_errs = word_outline_errs (page_res_it.word ());
    doc_outline_errs += outline_errs;
    word_char_quality (page_res_it.word (),
      page_res_it.row ()->row,
      &all_char_quality, &accepted_all_char_quality);
    doc_char_quality += all_char_quality;
    uinT8 permuter_type = page_res_it.word ()->best_choice->permuter ();
    if ((permuter_type == SYSTEM_DAWG_PERM) ||
      (permuter_type == FREQ_DAWG_PERM) ||
    (permuter_type == USER_DAWG_PERM)) {
      good_char_count += chars_in_word - rejects_in_word;
      doc_good_char_quality += accepted_all_char_quality;
    }
    check_debug_pt (page_res_it.word (), 80);
    if (tessedit_reject_bad_qual_wds &&
      (blob_quality == 0) && (outline_errs >= chars_in_word))
      page_res_it.word ()->reject_map.rej_word_bad_quality ();
    check_debug_pt (page_res_it.word (), 90);
    page_res_it.forward ();
  }

  page_res_it.restart_page ();
  while (!tessedit_test_adaption
  && tessedit_cluster_adapt_after_pass3 && page_res_it.word () != NULL) {
    if (monitor != NULL)
      monitor->ocr_alive = TRUE;

//changed by jetsoft
//specific to its needs to extract one word when need

	if (target_word_box)
	{

		TBOX current_word_box=page_res_it.word ()->word->bounding_box();
		FCOORD center_pt((current_word_box.right()+current_word_box.left())/2,(current_word_box.bottom()+current_word_box.top())/2);
		if (!target_word_box->contains(center_pt))
		{
			page_res_it.forward ();
			continue;
		}

	}

//end jetsoft
    if (tessedit_cluster_adaption_mode != 0)
      adapt_to_good_samples (page_res_it.word (),
        &char_clusters, &chars_waiting);
    page_res_it.forward ();
  }

  #ifndef SECURE_NAMES
  if (tessedit_debug_quality_metrics) {
    tprintf
      ("QUALITY: num_chs= %d  num_rejs= %d %5.3f blob_qual= %d %5.3f outline_errs= %d %5.3f char_qual= %d %5.3f good_ch_qual= %d %5.3f\n",
      page_res->char_count, page_res->rej_count,
      page_res->rej_count / (float) page_res->char_count, doc_blob_quality,
      doc_blob_quality / (float) page_res->char_count, doc_outline_errs,
      doc_outline_errs / (float) page_res->char_count, doc_char_quality,
      doc_char_quality / (float) page_res->char_count,
      doc_good_char_quality,
      good_char_count >
      0 ? doc_good_char_quality / (float) good_char_count : 0.0);
  }
  #endif
  BOOL8 good_quality_doc =
    (page_res->rej_count / (float) page_res->char_count <= quality_rej_pc)
    &&
    (doc_blob_quality / (float) page_res->char_count >= quality_blob_pc) &&
    (doc_outline_errs / (float) page_res->char_count <= quality_outline_pc) &&
    (doc_char_quality / (float) page_res->char_count >= quality_char_pc);

  /* Do whole document or whole block rejection pass*/

  if (!tessedit_test_adaption) {
    set_global_loc_code(LOC_DOC_BLK_REJ);
    quality_based_rejection(page_res_it, good_quality_doc);
  }
  font_recognition_pass(page_res_it);

  /* Write results pass */
  set_global_loc_code(LOC_WRITE_RESULTS);
  // This is now redundant, but retained commented so show how to obtain
  // bounding boxes and style information.

////changed by jetsoft
//needed for dll to output memory structure
  if ((dopasses == 0 || dopasses == 2) && (monitor || tessedit_write_unlv))
	output_pass(page_res_it, ocr_char_space() > 0, target_word_box);
// end jetsoft

}


/**********************************************************************
 * classify_word_pass1
 *
 * Baseline normalize the word and pass it to Tess.
 **********************************************************************/

void classify_word_pass1(                 //recog one word
                         WERD_RES *word,  //word to do
                         ROW *row,
                         BOOL8 cluster_adapt,
                         CHAR_SAMPLES_LIST *char_clusters,
                         CHAR_SAMPLE_LIST *chars_waiting) {
  WERD *bln_word;                //baseline norm copy
                                 //detailed results
  BLOB_CHOICE_LIST_CLIST local_blob_choices;
  BLOB_CHOICE_LIST_CLIST *blob_choices;
  BOOL8 adapt_ok;
  const char *rejmap;
  inT16 index;
  STRING mapstr = "";
  char *match_string;
  char word_string[1024];

  if (save_best_choices)
    blob_choices = new BLOB_CHOICE_LIST_CLIST();
  else
    blob_choices = &local_blob_choices;

  if (matcher_fp != NULL) {
    fgets (word_string, 1023, correct_fp);
    if ((match_string = strchr (word_string, '\r')) != NULL)
      *match_string = '\0';
    if ((match_string = strchr (word_string, '\n')) != NULL)
      *match_string = '\0';
    if (word_string[0] != '\0') {
      word->word->set_text (word_string);
      word_answer = (char *) word->word->text ();
    }
    else
      word_answer = NULL;
  }

  check_debug_pt (word, 0);
  matcher_pass = 0;
  bln_word = make_bln_copy (word->word, row, word->x_height, &word->denorm);

  word->best_choice = tess_segment_pass1 (bln_word, &word->denorm,
    tess_default_matcher,
    word->raw_choice, blob_choices,
    word->outword);
  /*
     Test for TESS screw up on word. Recog_word has already ensured that the
     choice list, outword blob lists and best_choice string are the same
     length. A TESS screw up is indicated by a blank filled or 0 length string.
   */
  if ((word->best_choice->lengths ().length () == 0) ||
    (strspn (word->best_choice->string ().string (), " ") ==
  word->best_choice->string ().length ())) {
    word->done = FALSE;          //Try again on pass2 - adaption may help
    word->tess_failed = TRUE;
    word->reject_map.initialise (word->best_choice->lengths ().length ());
    word->reject_map.rej_word_tess_failure ();
  }
  else {
    word->tess_failed = FALSE;
    if ((word->best_choice->lengths ().length () !=
      word->outword->blob_list ()->length ()) ||
    (word->best_choice->lengths ().length () != blob_choices->length ())) {
      tprintf
        ("ASSERT FAIL String:\"%s\"; Strlen=%d; #Blobs=%d; #Choices=%d\n",
        word->best_choice->string ().string (),
        word->best_choice->lengths ().length (),
        word->outword->blob_list ()->length (), blob_choices->length ());
    }
    ASSERT_HOST (word->best_choice->lengths ().length () ==
      word->outword->blob_list ()->length ());
    ASSERT_HOST (word->best_choice->lengths ().length () ==
      blob_choices->length ());

    /*
       The adaption step used to be here. It has been moved to after
       make_reject_map so that we know whether the word will be accepted in the
       first pass or not.   This move will PREVENT adaption to words containing
       double quotes because the word will not be identical to what tess thinks
       its best choice is. (See CurrentBestChoiceIs in
       danj/microfeatures/stopper.c which is used by AdaptableWord in
       danj/microfeatures/adaptmatch.c)
     */

    if (word->word->flag (W_REP_CHAR)) {
      fix_rep_char(word);
    }
    else {
      fix_quotes (word->best_choice,
      //turn to double
        word->outword, blob_choices);
      if (tessedit_fix_hyphens)
                                 //turn 2 to 1
        fix_hyphens (word->best_choice, word->outword, blob_choices);
      record_certainty (word->best_choice->certainty (), 1);
      //accounting

      word->tess_accepted = tess_acceptable_word (word->best_choice,
        word->raw_choice);

      word->tess_would_adapt = tess_adaptable_word (word->outword,
        word->best_choice,
        word->raw_choice);
                                 // Also sets word->done flag
      make_reject_map (word, blob_choices, row, 1);

      adapt_ok = word_adaptable (word, tessedit_tess_adaption_mode);

      if (cluster_adapt)
        adapt_to_good_samples(word, char_clusters, chars_waiting);

      if (adapt_ok || tessedit_tess_adapt_to_rejmap) {
        if (!tessedit_tess_adapt_to_rejmap)
          rejmap = NULL;
        else {
          ASSERT_HOST (word->reject_map.length () ==
            word->best_choice->lengths ().length ());

          for (index = 0; index < word->reject_map.length (); index++) {
            if (adapt_ok || word->reject_map[index].accepted ())
              mapstr += '1';
            else
              mapstr += '0';
          }
          rejmap = mapstr.string ();
        }

                                 //adapt to it
        tess_adapter (word->outword, &word->denorm,
                      *word->best_choice,
                      *word->raw_choice, rejmap);
      }

      if (tessedit_enable_doc_dict)
        tess_add_doc_word (word->best_choice);
      set_word_fonts(word, blob_choices);
    }
  }
#if 0
  if (tessedit_print_text) {
    write_cooked_text (bln_word, word->best_choice->string (),
      word->done, FALSE, stdout);
  }
#endif
  delete bln_word;

  // Save best choices in the WERD_CHOICE if needed
  if (blob_choices != &local_blob_choices)
    word->best_choice->set_blob_choices(blob_choices);
  else
    blob_choices->deep_clear();
}


/**********************************************************************
 * classify_word_pass2
 *
 * Control what to do with the word in pass 2
 **********************************************************************/

void classify_word_pass2(  //word to do
                         WERD_RES *word,
                         ROW *row) {
  BOOL8 done_this_pass = FALSE;
  WERD_RES new_x_ht_word (word->word);
  float new_x_ht = 0.0;
  inT16 old_xht_reject_count;
  inT16 new_xht_reject_count;
  inT16 old_xht_accept_count;
  inT16 new_xht_accept_count;
  BOOL8 accept_new_x_ht = FALSE;
  inT16 old_chs_in_wd;
  inT16 new_chs_in_wd;
  inT16 old_word_quality;
  inT16 new_word_quality;
  inT16 dummy;

  set_global_subloc_code(SUBLOC_NORM);
  check_debug_pt (word, 30);
  if (!word->done ||
    tessedit_training_tess ||
  tessedit_training_wiseowl || tessedit_dump_choices) {
    word->caps_height = 0.0;
    if (word->x_height == 0.0f)
      word->x_height = row->x_height();
    if (word->outword != NULL) {
      delete word->outword;      //get rid of junk
      delete word->best_choice;
      delete word->raw_choice;
    }
    match_word_pass2 (word, row, word->x_height);
    done_this_pass = TRUE;
    check_debug_pt (word, 40);
  }

  if (!word->tess_failed && !word->word->flag (W_REP_CHAR)) {
    set_global_subloc_code(SUBLOC_FIX_XHT);
    if ((tessedit_xht_fiddles_on_done_wds || !word->done) &&
      (tessedit_xht_fiddles_on_no_rej_wds ||
    (word->reject_map.reject_count () > 0))) {
      if ((x_ht_check_word_occ >= 2) && word_occ_first)
        check_block_occ(word);

      if (tessedit_redo_xheight)
        re_estimate_x_ht(word, &new_x_ht);

      if (((x_ht_check_word_occ >= 2) && !word_occ_first) ||
        ((x_ht_check_word_occ >= 1) && (new_x_ht > 0)))
        check_block_occ(word);
    }
    if (new_x_ht > 0) {
      old_chs_in_wd = word->reject_map.length ();

      /* Re-estimated x_ht error suggests a rematch is worthwhile. */
      new_x_ht_word.x_height = new_x_ht;
      new_x_ht_word.caps_height = 0.0;
      match_word_pass2 (&new_x_ht_word, row, new_x_ht_word.x_height);
      if (!new_x_ht_word.tess_failed) {
        if ((x_ht_check_word_occ >= 1) && word_occ_first)
          check_block_occ(&new_x_ht_word);

        re_estimate_x_ht(&new_x_ht_word, &new_x_ht);

        if ((x_ht_check_word_occ >= 1) && !word_occ_first)
          check_block_occ(&new_x_ht_word);

        old_xht_reject_count = word->reject_map.reject_count ();
        old_xht_accept_count = old_chs_in_wd - old_xht_reject_count;
        new_xht_reject_count = new_x_ht_word.reject_map.reject_count ();
        new_chs_in_wd = new_x_ht_word.reject_map.length ();
        new_xht_accept_count = new_chs_in_wd - new_xht_reject_count;
        accept_new_x_ht =
          ((new_xht_accept_count > old_xht_accept_count) ||
          ((new_xht_accept_count == old_xht_accept_count) &&
          (new_xht_accept_count > 0))) &&
          (!new_x_ht_word.guessed_x_ht ||
          !new_x_ht_word.guessed_caps_ht);

        if (accept_new_x_ht && x_ht_quality_check) {
          word_char_quality(word, row, &old_word_quality, &dummy);
          word_char_quality(&new_x_ht_word, row, &new_word_quality, &dummy);
          if (old_word_quality > new_word_quality)
            accept_new_x_ht = FALSE;
        }

        if (accept_new_x_ht && (x_ht_stringency > 0)) {
          accept_new_x_ht =
            (count_alphanums (&new_x_ht_word) > x_ht_stringency);
          if (!accept_new_x_ht && rej_use_xht) {
            if (debug_x_ht_level >= 1)
              tprintf
                ("Failed stringency test so reject original word\n");
            word->reject_map.rej_word_xht_fixup ();
          }
        }

        #ifndef SECURE_NAMES
        if (debug_x_ht_level >= 1) {
          tprintf ("New XHT Match:: %s ",
            word->best_choice->string ().string ());
          word->reject_map.print (debug_fp);
          tprintf (" -> %s ",
            new_x_ht_word.best_choice->string ().string ());
          new_x_ht_word.reject_map.print (debug_fp);
          tprintf (" %s->%s %s %s\n",
            word->guessed_x_ht ? "GUESS" : "CERT",
            new_x_ht_word.guessed_x_ht ? "GUESS" : "CERT",
            new_x_ht > 0.1 ? "STILL DOUBT" : "OK",
            accept_new_x_ht ? "ACCEPTED" : "");
        }
        #endif
      }
      if (accept_new_x_ht) {
        /*
           The new x_ht is deemed superior so put the final results in the real word
           and destroy the old results
         */
        delete word->outword;    //get rid of junk
        word->outword = new_x_ht_word.outword;
        word->denorm = new_x_ht_word.denorm;
        delete word->best_choice;
        word->best_choice = new_x_ht_word.best_choice;
        delete word->raw_choice;
        word->raw_choice = new_x_ht_word.raw_choice;
        word->reject_map = new_x_ht_word.reject_map;
        word->done = new_x_ht_word.done;
        done_this_pass = TRUE;
      }
      else {
      /*
         The new x_ht is no better, so destroy the copy word and put any uncertain
         x or cap ht estimate back to default. (I.e. dont blame me if its bad!)
         Conditionally, use any ammended block occ chars.
       */
                                 //get rid of junk
        delete new_x_ht_word.outword;
        delete new_x_ht_word.best_choice;
        delete new_x_ht_word.raw_choice;
      }
                                 //to keep new destructor happy
      new_x_ht_word.outword = NULL;
                                 //to keep new destructor happy
      new_x_ht_word.best_choice = NULL;
                                 //to keep new destructor happy
      new_x_ht_word.raw_choice = NULL;

      if (rej_mostly_reject_mode == 2) {
        reject_mostly_rejects(word);
        tprintf ("Rejecting mostly rejects on %s ",
          word->best_choice->string ().string ());
      }
    }

    set_global_subloc_code(SUBLOC_NORM);

    if (done_this_pass && !word->done && tessedit_save_stats)
      SaveBadWord (word->best_choice->string ().string (),
        word->best_choice->certainty ());
    record_certainty (word->best_choice->certainty (), 2);
    //accounting
  }
#ifndef GRAPHICS_DISABLED
  if (tessedit_draw_outwords) {
    if (fx_win == NULL)
      create_fx_win();
    clear_fx_win();
    word->outword->plot (fx_win);
    TBOX wbox = word->outword->bounding_box();
    fx_win->ZoomToRectangle(wbox.left(), wbox.top(),
                            wbox.right(), wbox.bottom());
    //make_picture_current(fx_win);
    ScrollView::Update();
  }
#endif

  set_global_subloc_code(SUBLOC_NORM);
#if 0
  if (tessedit_print_text) {
    write_cooked_text (word->outword, word->best_choice->string (),
      word->done, done_this_pass, stdout);
  }
#endif
  check_debug_pt (word, 50);
}


/**********************************************************************
 * match_word_pass2
 *
 * Baseline normalize the word and pass it to Tess.
 **********************************************************************/

void match_word_pass2(                 //recog one word
                      WERD_RES *word,  //word to do
                      ROW *row,
                      float x_height) {
  WERD *bln_word;                //baseline norm copy
                                 //detailed results
  BLOB_CHOICE_LIST_CLIST local_blob_choices;
  BLOB_CHOICE_LIST_CLIST *blob_choices;

  if (save_best_choices)
    blob_choices = new BLOB_CHOICE_LIST_CLIST();
  else
    blob_choices = &local_blob_choices;

  set_global_subsubloc_code(SUBSUBLOC_OTHER);
  if (matcher_fp != NULL) {
    word_answer = (char *) word->word->text ();
    if (word_answer != NULL && word_answer[0] == '\0')
      word_answer = NULL;
  }
  matcher_pass = 0;
  bln_word = make_bln_copy (word->word, row, x_height, &word->denorm);
  set_global_subsubloc_code(SUBSUBLOC_TESS);
  if (tessedit_training_tess)
    word->best_choice = correct_segment_pass2 (bln_word,
      &word->denorm,
      tess_default_matcher,
      tess_training_tester,
      word->raw_choice,
      blob_choices, word->outword);
  else if (tessedit_dump_choices)
    word->best_choice = test_segment_pass2 (bln_word,
        &word->denorm,
        tess_default_matcher,
        choice_dump_tester,
        word->raw_choice,
        blob_choices, word->outword);
  //      else if (tessedit_training_wiseowl)
  //              best_choice=correct_segment_pass2( word, &denorm,
  //                                                                                                        tess_default_matcher,wo_learn,
  //                                                                                                        raw_choice,blob_choices,outword);
  //      else if (tessedit_matcher_is_wiseowl)
  //              best_choice=tess_segment_pass2( word, &denorm, wo_classify,
  //                                                                                                raw_choice, blob_choices, outword);
  else {
    word->best_choice = tess_segment_pass2 (bln_word, &word->denorm,
      tess_default_matcher,
      word->raw_choice, blob_choices,
      word->outword);
  }
  set_global_subsubloc_code(SUBSUBLOC_OTHER);
  /*
     Test for TESS screw up on word. Recog_word has already ensured that the
     choice list, outword blob lists and best_choice string are the same
     length. A TESS screw up is indicated by a blank filled or 0 length string.
   */
  if ((word->best_choice->string ().length () == 0) ||
    (strspn (word->best_choice->string ().string (), " ") ==
  word->best_choice->string ().length ())) {
    word->tess_failed = TRUE;
    word->reject_map.initialise (word->best_choice->string ().length ());
    word->reject_map.rej_word_tess_failure ();
    //              tprintf("Empty word produced\n");
  }
  else {
    if ((word->best_choice->lengths ().length () !=
      word->outword->blob_list ()->length ()) ||
    (word->best_choice->lengths ().length () != blob_choices->length ())) {
      tprintf
        ("ASSERT FAIL String:\"%s\"; Strlen=%d; #Blobs=%d; #Choices=%d\n",
        word->best_choice->string ().string (),
        word->best_choice->lengths ().length (),
        word->outword->blob_list ()->length (), blob_choices->length ());
    }
    ASSERT_HOST (word->best_choice->lengths ().length () ==
      word->outword->blob_list ()->length ());
    ASSERT_HOST (word->best_choice->lengths ().length () ==
      blob_choices->length ());

    word->tess_failed = FALSE;
    if (word->word->flag (W_REP_CHAR)) {
      fix_rep_char(word);
    }
    else {
      fix_quotes (word->best_choice,
        word->outword, blob_choices);
      if (tessedit_fix_hyphens)
        fix_hyphens (word->best_choice,
          word->outword, blob_choices);
      /* Dont trust fix_quotes! - though I think I've fixed the bug */
      if ((word->best_choice->lengths ().length () !=
           word->outword->blob_list ()->length ()) ||
          (word->best_choice->lengths ().length () !=
           blob_choices->length ())) {
        #ifndef SECURE_NAMES
        tprintf
          ("POST FIX_QUOTES FAIL String:\"%s\"; Strlen=%d; #Blobs=%d; #Choices=%d\n",
           word->best_choice->string ().string (),
           word->best_choice->lengths ().length (),
           word->outword->blob_list ()->length (),
           blob_choices->length ());
        #endif

      }
      ASSERT_HOST (word->best_choice->lengths ().length () ==
        word->outword->blob_list ()->length ());
      ASSERT_HOST (word->best_choice->lengths ().length () ==
        blob_choices->length ());

      word->tess_accepted = tess_acceptable_word (word->best_choice,
        word->raw_choice);

      make_reject_map (word, blob_choices, row, 2);
    }
  }

  // Save best choices in the WERD_CHOICE if needed
  if (blob_choices != &local_blob_choices)
    word->best_choice->set_blob_choices(blob_choices);
  else
    blob_choices->deep_clear();

  delete bln_word;
  assert (word->raw_choice != NULL);
}


/*************************************************************************
 * fix_rep_char()
 * The word is a repeated char. Find the repeated char character. Make a reject
 * string which rejects any char other than the voted char. Set the word to done
 * to stop rematching it.
 *
 *************************************************************************/
void fix_rep_char(                //Repeated char word
                  WERD_RES *word  //word to do
                 ) {
  struct REP_CH
  {
    char ch[UNICHAR_LEN + 1];
    int count;
  };

  REP_CH *rep_ch;                //array of char counts
  int word_len;
  int rep_ch_count = 0;          //how many unique chs
  const char *word_str;          //the repeated chs
  int i, j;
  int offset;
  int total = 0;
  int max = 0;
  char *maxch = NULL;              //Most common char

  word_str = word->best_choice->string ().string ();
  word_len = word->best_choice->lengths ().length ();;
  rep_ch = (REP_CH *) alloc_mem (word_len * sizeof (REP_CH));
  for (i = 0, offset = 0; i < word_len;
       offset += word->best_choice->lengths()[i++]) {
    for (j = 0; j < rep_ch_count &&
             strncmp(rep_ch[j].ch, word_str + offset,
                     word->best_choice->lengths()[i]) != 0; j++);
    if (j < rep_ch_count)
      rep_ch[j].count++;
    else {
      strncpy(rep_ch[rep_ch_count].ch, word_str + offset,
              word->best_choice->lengths()[i]);
      rep_ch[rep_ch_count].ch[word->best_choice->lengths()[i]] = '\0';
      rep_ch[rep_ch_count].count = 1;
      rep_ch_count++;
    }
  }

  for (j = 0; j < rep_ch_count; j++) {
    total += rep_ch[j].count;
    if ((rep_ch[j].count > max) && (*rep_ch[j].ch != ' ')) {
      max = rep_ch[j].count;
      maxch = rep_ch[j].ch;
    }
  }
  //      tprintf( "REPEATED CHAR %s len=%d total=%d choice=%c\n",
  //                        word_str, word_len, total, maxch );
  free_mem(rep_ch);

  word->reject_map.initialise (word_len);
  for (i = 0, offset = 0; i < word_len;
       offset += word->best_choice->lengths()[i++]) {
    if (strncmp(word_str + offset, maxch,
                word->best_choice->lengths()[i]) != 0)
                                 //rej unrecognised blobs
      word->reject_map[i].setrej_bad_repetition ();
  }
  word->done = TRUE;
}

// TODO(tkielbus) Decide between keeping this behavior here or modifying the
// training data.

// Utility function for fix_quotes
// Return true if the next character in the string (given the UTF8 length in
// bytes) is a quote character.
static int is_simple_quote(const char* signed_str, int length) {
  const unsigned char* str = reinterpret_cast<const unsigned char*>(signed_str);
   //standard 1 byte quotes
  return (length == 1 && (*str == '\'' || *str == '`')) ||
      //utf8 3 bytes curved quotes
      (length == 3 && ((*str == 0xe2 &&
                        *(str + 1) == 0x80 &&
                        *(str + 2) == 0x98) ||
                       (*str == 0xe2 &&
                        *(str + 1) == 0x80 &&
                        *(str + 2) == 0x99)));
}

/**********************************************************************
 * fix_quotes
 *
 * Change pairs of quotes to double quotes.
 **********************************************************************/
void fix_quotes(               //make double quotes
                WERD_CHOICE *choice,  //choice to fix
                WERD *word,    //word to do //char choices
                BLOB_CHOICE_LIST_CLIST *blob_choices) {
  char *str = (char *) choice->string().string();//string ptr
  int i;
  int offset;
                                 //blobs
  PBLOB_IT blob_it = word->blob_list ();
                                 //choices
  BLOB_CHOICE_LIST_C_IT choice_it = blob_choices;
  BLOB_CHOICE_IT it1;            //first choices
  BLOB_CHOICE_IT it2;            //second choices

  for (i = 0, offset = 0; str[offset] != '\0';
       offset += choice->lengths()[i++],
           blob_it.forward (), choice_it.forward ()) {
    if (str[offset + choice->lengths()[i]] != '\0' &&
        is_simple_quote(str + offset, choice->lengths()[i]) &&
        is_simple_quote(str + offset + choice->lengths()[i],
                        choice->lengths()[i + 1]) &&
        unicharset.contains_unichar("\"")) {
      str[offset] = '"';                //turn to double
      strcpy (str + offset + 1,
              str + offset + choice->lengths()[i] +
              choice->lengths()[i + 1]); //shuffle up
      choice->lengths()[i] = 1;
      strcpy ((char*) choice->lengths().string() + i + 1,
              choice->lengths().string() + i + 2);
      merge_blobs (blob_it.data (), blob_it.data_relative (1));
      blob_it.forward ();
      delete blob_it.extract (); //get rid of spare

      it1.set_to_list (choice_it.data ());
      it2.set_to_list (choice_it.data_relative (1));
      if (it1.data ()->certainty () < it2.data ()->certainty ()) {
        choice_it.forward ();
                                 //get rid of spare
        delete choice_it.extract ();
      }
      else {
                                 //get rid of spare
        delete choice_it.extract ();
        choice_it.forward ();
      }
    }
  }
}


/**********************************************************************
 * fix_hyphens
 *
 * Change pairs of hyphens to a single hyphen if the bounding boxes touch
 * Typically a long dash which has been segmented.
 **********************************************************************/
void fix_hyphens(               //crunch double hyphens
                 WERD_CHOICE *choice,  //choice to fix
                 WERD *word,    //word to do //char choices
                 BLOB_CHOICE_LIST_CLIST *blob_choices) {
  char *str = (char *) choice->string().string();//string ptr
  int i;
  int offset;
                                 //blobs
  PBLOB_IT blob_it = word->blob_list ();
                                 //choices
  BLOB_CHOICE_LIST_C_IT choice_it = blob_choices;
  BLOB_CHOICE_IT it1;            //first choices
  BLOB_CHOICE_IT it2;            //second choices

  for (i = 0, offset = 0; str[offset] != '\0';
  offset += choice->lengths()[i++],
           blob_it.forward (), choice_it.forward ()) {
    if ((str[offset] == '-' || str[offset] == '~') &&
      (str[offset + choice->lengths()[i]] == '-' ||
       str[offset + choice->lengths()[i]] == '~') &&
      (blob_it.data ()->bounding_box ().right () >=
    blob_it.data_relative (1)->bounding_box ().left ())) {
      str[offset] = '-';                //turn to single hyphen
      strcpy (str + offset + choice->lengths()[i],
              str + offset + choice->lengths()[i] +
              choice->lengths()[i + 1]); //shuffle up
      strcpy ((char*) choice->lengths().string() + i + 1,
              choice->lengths().string() + i + 2);
      merge_blobs (blob_it.data (), blob_it.data_relative (1));
      blob_it.forward ();
      delete blob_it.extract (); //get rid of spare

      it1.set_to_list (choice_it.data ());
      it2.set_to_list (choice_it.data_relative (1));
      if (it1.data ()->certainty () < it2.data ()->certainty ()) {
        choice_it.forward ();
                                 //get rid of spare
        delete choice_it.extract ();
      }
      else {
                                 //get rid of spare
        delete choice_it.extract ();
        choice_it.forward ();
      }
    }
  }
}


/**********************************************************************
 * merge_blobs
 *
 * Add the outlines from blob2 to blob1. Blob2 is emptied but not deleted.
 **********************************************************************/

void merge_blobs(               //combine 2 blobs
                 PBLOB *blob1,  //dest blob
                 PBLOB *blob2   //source blob
                ) {
  OUTLINE_IT outline_it = blob1->out_list ();
  //iterator

  outline_it.move_to_last ();    //go to end
                                 //do it
  outline_it.add_list_after (blob2->out_list ());
}


/**********************************************************************
 * choice_dump_tester
 *
 * Matcher tester function which generates .chc file entries.
 * Called via test_segment_pass2 for every blob tested by tess in a word.
 * (But only for words for which a correct segmentation could be found.)
 **********************************************************************/

void choice_dump_tester(                           //dump chars in word
                        PBLOB *,                   //blob
                        DENORM *,                  //de-normaliser
                        BOOL8 correct,             //ly segmented
                        char *text,                //correct text
                        inT32 count,               //chars in text
                        BLOB_CHOICE_LIST *ratings  //list of results
                       ) {
  STRING choice_file_name;
  BLOB_CHOICE *blob_choice;
  BLOB_CHOICE_IT it;
  char source_chars[20];
  char correct_char[3];

  if (choice_file == NULL) {
    choice_file_name = imagebasename + ".chc";
    if (!(choice_file = fopen (choice_file_name.string (), "w"))) {
      CANTOPENFILE.error ("choice_dump_tester", EXIT, "%s %d",
        choice_file_name.string (), errno);
    }
  }

  if ((count == 0) || (text == NULL) || (text[0] == '\0')) {
    strcpy (source_chars, "$$");
    strcpy (correct_char, "$$");
  }
  else {
    strncpy(source_chars, text, count);
    source_chars[count] = '\0';
    if (correct) {
      correct_char[0] = text[0];
      correct_char[1] = '\0';
    }
    else {
      strcpy (correct_char, "$$");
    }
  }
  fprintf (choice_file, "%s\t%s", source_chars, correct_char);

  it.set_to_list (ratings);
  for (it.mark_cycle_pt (); !it.cycled_list (); it.forward ()) {
    blob_choice = it.data ();
    fprintf (choice_file, "\t%s\t%f\t%f",
             blob_choice->unichar (),
             blob_choice->rating (), blob_choice->certainty ());
  }
  fprintf (choice_file, "\n");
}


/*************************************************************************
 * make_bln_copy()
 *
 * Generate a baseline normalised copy of the source word. The copy is done so
 * that whatever format the original word is in, a polygonal bln version is
 * generated as output.
 *************************************************************************/

WERD *make_bln_copy(WERD *src_word, ROW *row, float x_height, DENORM *denorm) {
  WERD *result;

  //      if (wordit_linearc && !src_word->flag(W_POLYGON))
  //      {
  //              larc_word = src_word->larc_copy( row->x_height() );
  //              result = larc_word->poly_copy( row->x_height() );
  //              delete larc_word;
  //      }
  // else
  result = src_word->poly_copy (row->x_height ());

  //      if (tessedit_draw_words)
  //      {
  //              if ( la_win == NO_WINDOW )
  //                      create_la_win();
  //              result->plot( la_win );
  //      }
  result->baseline_normalise_x (row, x_height, denorm);
  return result;
}


ACCEPTABLE_WERD_TYPE acceptable_word_string(const char *s,
                                            const char *lengths) {
  int i = 0;
  int offset = 0;
  int leading_punct_count;
  int upper_count = 0;
  int hyphen_pos = -1;
  ACCEPTABLE_WERD_TYPE word_type = AC_UNACCEPTABLE;

  if (strlen (lengths) > 20)
    return word_type;

  /* Single Leading punctuation char*/

  if ((s[offset] != '\0') && (STRING (chs_leading_punct).contains (s[offset])))
    offset += lengths[i++];
  leading_punct_count = i;

  /* Initial cap */
  while ((s[offset] != '\0') &&
         unicharset.get_isupper(s + offset, lengths[i])) {
    offset += lengths[i++];
    upper_count++;
  }
  if (upper_count > 1)
    word_type = AC_UPPER_CASE;
  else {
    /* Lower case word, possibly with an initial cap */
    while ((s[offset] != '\0') &&
           unicharset.get_islower (s + offset, lengths[i])) {
      offset += lengths[i++];
    }
    if (i - leading_punct_count < quality_min_initial_alphas_reqd)
      goto not_a_word;
    /*
    Allow a single hyphen in a lower case word
    - dont trust upper case - I've seen several cases of "H" -> "I-I"
    */
    if (lengths[i] == 1 && s[offset] == '-') {
      hyphen_pos = i;
      offset += lengths[i++];
      if (s[offset] != '\0') {
        while ((s[offset] != '\0') &&
               unicharset.get_islower(s + offset, lengths[i])) {
          offset += lengths[i++];
        }
        if (i < hyphen_pos + 3)
          goto not_a_word;
      }
    }
    else {
      /* Allow "'s" in NON hyphenated lower case words */
      if (lengths[i] == 1 && (s[offset] == '\'') &&
          lengths[i + 1] == 1 && (s[offset + lengths[i]] == 's')) {
        offset += lengths[i++];
        offset += lengths[i++];
      }
    }
    if (upper_count > 0)
      word_type = AC_INITIAL_CAP;
    else
      word_type = AC_LOWER_CASE;
  }

  /* Up to two different, constrained trailing punctuation chars */
  if (lengths[i] == 1 && (s[offset] != '\0') &&
      (STRING (chs_trailing_punct1).contains (s[offset])))
    offset += lengths[i++];
  if (lengths[i] == 1 && (s[offset] != '\0') && i > 0 &&
    (s[offset - lengths[i - 1]] != s[offset]) &&
      (STRING (chs_trailing_punct2).contains (s[offset])))
    offset += lengths[i++];

  if (s[offset] != '\0')
    word_type = AC_UNACCEPTABLE;

  not_a_word:

  if (word_type == AC_UNACCEPTABLE) {
    /* Look for abbreviation string */
    i = 0;
    offset = 0;
    if (s[0] != '\0' && unicharset.get_isupper (s, lengths[0])) {
      word_type = AC_UC_ABBREV;
      while ((s[offset] != '\0') &&
             unicharset.get_isupper(s + offset, lengths[i]) &&
             (lengths[i + 1] == 1 && s[offset + lengths[i]] == '.')) {
        offset += lengths[i++];
        offset += lengths[i++];
      }
    }
    else if (s[0] != '\0' && unicharset.get_islower (s, lengths[0])) {
      word_type = AC_LC_ABBREV;
      while ((s[offset] != '\0') &&
             unicharset.get_islower(s + offset, lengths[i]) &&
             (lengths[i + 1] == 1 && s[offset + lengths[i]] == '.')) {
        offset += lengths[i++];
        offset += lengths[i++];
      }
    }
    if (s[offset] != '\0')
      word_type = AC_UNACCEPTABLE;
  }

  return word_type;
}


/* DEBUGGING ROUTINE */

BOOL8 check_debug_pt(WERD_RES *word, int location) {
  BOOL8 show_map_detail = FALSE;
  inT16 i;

  #ifndef SECURE_NAMES
  if (!test_pt)
    return FALSE;

  tessedit_rejection_debug.set_value (FALSE);
  debug_x_ht_level.set_value (0);
  tessedit_cluster_debug.set_value (FALSE);
  nn_debug.set_value (FALSE);
  nn_reject_debug.set_value (FALSE);

  if (word->word->bounding_box ().contains (FCOORD (test_pt_x, test_pt_y))) {
    if (location < 0)
      return TRUE;               //For breakpoint use
    tessedit_rejection_debug.set_value (TRUE);
    debug_x_ht_level.set_value (20);
    tessedit_cluster_debug.set_value (TRUE);
    nn_debug.set_value (TRUE);
    nn_reject_debug.set_value (TRUE);
    tprintf ("\n\nTESTWD::");
    switch (location) {
      case 0:
        tprintf ("classify_word_pass1 start\n");
        word->word->print (debug_fp);
        break;
      case 10:
        tprintf ("make_reject_map: initial map");
        break;
      case 20:
        tprintf ("make_reject_map: after NN");
        break;
      case 30:
        tprintf ("classify_word_pass2 - START");
        break;
      case 40:
        tprintf ("classify_word_pass2 - Pre Xht");
        break;
      case 50:
        tprintf ("classify_word_pass2 - END");
        show_map_detail = TRUE;
        break;
      case 60:
        tprintf ("fixspace");
        break;
      case 70:
        tprintf ("MM pass START");
        break;
      case 80:
        tprintf ("MM pass END");
        break;
      case 90:
        tprintf ("After Poor quality rejection");
        break;
      case 100:
        tprintf ("unrej_good_quality_words - START");
        break;
      case 110:
        tprintf ("unrej_good_quality_words - END");
        break;
      case 120:
        tprintf ("Write results pass");
        show_map_detail = TRUE;
        break;
    }
    tprintf (" \"%s\" ", word->best_choice->string ().string ());
    word->reject_map.print (debug_fp);
    tprintf ("\n");
    if (show_map_detail) {
      tprintf ("\"%s\"\n", word->best_choice->string ().string ());
      for (i = 0; word->best_choice->string ()[i] != '\0'; i++) {
        tprintf ("**** \"%c\" ****\n", word->best_choice->string ()[i]);
        word->reject_map[i].full_print (debug_fp);
      }
    }

    tprintf ("Tess Accepted: %s\n", word->tess_accepted ? "TRUE" : "FALSE");
    tprintf ("Done flag: %s\n\n", word->done ? "TRUE" : "FALSE");
    return TRUE;
  }
  else
  #endif
    return FALSE;
}


/**********************************************************************
 * set_word_fonts
 *
 * Get the fonts for the word.
 **********************************************************************/

void set_word_fonts(                 //good chars in word
                    WERD_RES *word,  //word to adapt to //detailed results
                    BLOB_CHOICE_LIST_CLIST *blob_choices) {
  inT32 index;                   //char index
  inT32 offset;                  //char offset
  char choice_char[UNICHAR_LEN + 1];    //char from word
  inT8 config;                   //font of char
                                 //character iterator
  BLOB_CHOICE_LIST_C_IT char_it = blob_choices;
  BLOB_CHOICE_IT choice_it;      //choice iterator
  STATS fonts (0, 32);           //font counters
  static inT8 italic_table[32] = {
    1, -1, 1, -1,
    1, -1, 1, -1,
    1, -1, 1, -1,
    1, -1, 1, -1,
    1, -1, 1, -1,
    1, -1, 1, -1,
    1, -1, 1, -1,
    1, -1, 1, -1
  };
  static inT8 bold_table[32] = {
    1, 1, -1, -1,
    1, 1, -1, -1,
    1, 1, -1, -1,
    1, 1, -1, -1,
    1, 1, -1, -1,
    1, 1, -1, -1,
    1, 1, -1, -1,
    1, 1, -1, -1
  };
  static inT8 font_table[32] = {
    2, 2, 2, 2,
    -1, -1, -1, -1,
    0, 0, 0, 0,
    1, 1, 1, 1,
    3, 3, 3, 3,
    4, 4, 4, 4,
    5, 5, 5, 5,
    2, 2, 2, 2
  };

  word->italic = 0;
  word->bold = 0;
  for (char_it.mark_cycle_pt (), index = 0, offset = 0;
  !char_it.cycled_list (); char_it.forward (),
           offset += word->best_choice->lengths()[index++]) {
    strncpy(choice_char, word->best_choice->string ().string() + offset,
            word->best_choice->lengths()[index]);
    choice_char[word->best_choice->lengths()[index]] = '\0';
    choice_it.set_to_list (char_it.data ());
    for (choice_it.mark_cycle_pt (); !choice_it.cycled_list ();
         choice_it.forward ()) {
      if (strcmp(choice_it.data ()->unichar (), choice_char) == 0) {
        config = choice_it.data ()->config ();
        if (tessedit_debug_fonts)
          tprintf ("%s(%d=%d%c%c)",
            choice_char, config, (config & 31) >> 2,
            config & 2 ? 'N' : 'B', config & 1 ? 'N' : 'I');
        if (config != -1) {
          config &= 31;
          word->italic += italic_table[config];
          word->bold += bold_table[config];
          if (font_table[config] != -1)
            fonts.add (font_table[config], 1);
        }
        break;
      }
    }
  }
  find_modal_font (&fonts, &word->font1, &word->font1_count);
  find_modal_font (&fonts, &word->font2, &word->font2_count);
  if (tessedit_debug_fonts)
    tprintf ("\n");
  /*	if (word->font1_count>0)
    {
      for (char_it.mark_cycle_pt(),index=0;
      !char_it.cycled_list();char_it.forward(),index++)
      {
        choice_char=word->best_choice->string()[index];
        choice_it.set_to_list(char_it.data());
        for (choice_it.mark_cycle_pt();!choice_it.cycled_list();choice_it.forward())
        {
          if (choice_it.data()->char_class()==choice_char)
          {
            config=choice_it.data()->config();
            if (config!=-1 && font_table[config&31]==word->font1)
            {
              word->italic+=italic_table[config];
              word->bold+=bold_table[config];
            }
            break;
          }
        }
      }
    }*/
}


/**********************************************************************
 * font_recognition_pass
 *
 * Smooth the fonts for the document.
 **********************************************************************/

void font_recognition_pass(  //good chars in word
                           PAGE_RES_IT &page_res_it) {
  inT32 length;                  //of word
  inT32 count;                   //of a feature
  inT8 doc_font;                 //modal font
  inT8 doc_font_count;           //modal font
  inT32 doc_italic;              //total italics
  inT32 doc_bold;                //total bolds
  ROW_RES *row = NULL;           //current row
  WERD_RES *word;                //current word
  STATS fonts (0, 32);           //font counters
  STATS doc_fonts (0, 32);       //font counters

  doc_italic = 0;
  doc_bold = 0;
  page_res_it.restart_page ();
  while (page_res_it.word () != NULL) {
    if (row != page_res_it.row ()) {
      if (row != NULL) {
        find_modal_font (&fonts, &row->font1, &row->font1_count);
        find_modal_font (&fonts, &row->font2, &row->font2_count);
      }
      row = page_res_it.row ();  //current row
      fonts.clear ();            //clear counters
      row->italic = 0;
      row->bold = 0;
    }
    word = page_res_it.word ();
    row->italic += word->italic;
    row->bold += word->bold;
    fonts.add (word->font1, word->font1_count);
    fonts.add (word->font2, word->font2_count);
    doc_italic += word->italic;
    doc_bold += word->bold;
    doc_fonts.add (word->font1, word->font1_count);
    doc_fonts.add (word->font2, word->font2_count);
    page_res_it.forward ();
  }
  if (row != NULL) {
    find_modal_font (&fonts, &row->font1, &row->font1_count);
    find_modal_font (&fonts, &row->font2, &row->font2_count);
  }
  find_modal_font(&doc_fonts, &doc_font, &doc_font_count);
  /*
    row=NULL;
    page_res_it.restart_page();
    while (page_res_it.word() != NULL)
    {
      if (row!=page_res_it.row())
      {
        row2=row;
        row=page_res_it.row();
        if (row->font1_count<MIN_FONT_ROW_COUNT)
        {
          fonts.clear();
          italic=0;
          bold=0;
          add_in_one_row(row,&fonts,&italic,&bold);
          if (row2!=NULL)
          {
            hdiff=row->row->x_height()-row2->row->x_height();
            if (hdiff<0)
              hdiff=-hdiff;
            if (hdiff<MAX_XHEIGHT_DIFF)
              add_in_one_row(row2,&fonts,&italic,&bold);
          }
          do
            page_res_it.forward();
          while (page_res_it.row()==row);
          row2=page_res_it.row();
          if (row2!=NULL)
          {
            hdiff=row->row->x_height()-row2->row->x_height();
            if (hdiff<0)
              hdiff=-hdiff;
            if (hdiff<MAX_XHEIGHT_DIFF)
              add_in_one_row(row2,&fonts,&italic,&bold);
          }
          row->italic=italic;
          row->bold=bold;
          find_modal_font(&fonts,&row->font1,&row->font1_count);
          find_modal_font(&fonts,&row->font2,&row->font2_count);
        }
        else
          page_res_it.forward();
      }
      else
        page_res_it.forward();
    }*/

  page_res_it.restart_page ();
  while (page_res_it.word () != NULL) {
    row = page_res_it.row ();    //current row
    word = page_res_it.word ();
    length = word->best_choice->string ().length ();

    count = word->italic;
    if (count < 0)
      count = -count;
    if (!(count == length || (length > 3 && count >= length * 3 / 4)))
      word->italic = doc_italic > 0 ? 1 : -1;

    count = word->bold;
    if (count < 0)
      count = -count;
    if (!(count == length || (length > 3 && count >= length * 3 / 4)))
      word->bold = doc_bold > 0 ? 1 : -1;

    count = word->font1_count;
    if (!(count == length || (length > 3 && count >= length * 3 / 4))) {
      word->font1 = doc_font;
      word->font1_count = doc_font_count;
    }

    page_res_it.forward ();
  }
}


/**********************************************************************
 * add_in_one_row
 *
 * Add into the stats for one row.
 **********************************************************************/

void add_in_one_row(               //good chars in word
                    ROW_RES *row,  //current row
                    STATS *fonts,  //font stats
                    inT8 *italic,  //output count
                    inT8 *bold     //output count
                   ) {
  WERD_RES *word;                //current word
  WERD_RES_IT word_it = &row->word_res_list;

  for (word_it.mark_cycle_pt (); !word_it.cycled_list (); word_it.forward ()) {
    word = word_it.data ();
    *italic += word->italic;
    *bold += word->bold;
    if (word->font1_count > 0)
      fonts->add (word->font1, word->font1_count);
    if (word->font2_count > 0)
      fonts->add (word->font2, word->font2_count);

  }
}


/**********************************************************************
 * find_modal_font
 *
 * Find the modal font and remove from the stats.
 **********************************************************************/

void find_modal_font(                  //good chars in word
                     STATS *fonts,     //font stats
                     inT8 *font_out,   //output font
                     inT8 *font_count  //output count
                    ) {
  inT8 font;                     //font index
  inT32 count;                   //pile couat

  if (fonts->get_total () > 0) {
    font = (inT8) fonts->mode ();
    *font_out = font;
    count = fonts->pile_count (font);
    *font_count = count < MAX_INT8 ? count : MAX_INT8;
    fonts->add (font, -*font_count);
  }
  else {
    *font_out = -1;
    *font_count = 0;
  }
}
