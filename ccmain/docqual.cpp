/******************************************************************
 * File:        docqual.cpp  (Formerly docqual.c)
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

#ifdef _MSC_VER
#pragma warning(disable:4244)  // Conversion warnings
#endif

#include "mfcpch.h"
#include          <ctype.h>
#include          "docqual.h"
#include          "tstruct.h"
#include          "tfacep.h"
#include          "reject.h"
#include          "tessvars.h"
#include          "genblob.h"
#include          "secname.h"
#include          "globals.h"
#include          "tesseractclass.h"

#define EXTERN

EXTERN STRING_VAR (outlines_odd, "%| ", "Non standard number of outlines");
EXTERN STRING_VAR (outlines_2, "ij!?%\":;",
"Non standard number of outlines");
EXTERN BOOL_VAR (docqual_excuse_outline_errs, FALSE,
"Allow outline errs in unrejection?");
EXTERN BOOL_VAR (tessedit_good_quality_unrej, TRUE,
"Reduce rejection on good docs");
EXTERN BOOL_VAR (tessedit_use_reject_spaces, TRUE, "Reject spaces?");
EXTERN double_VAR (tessedit_reject_doc_percent, 65.00,
"%rej allowed before rej whole doc");
EXTERN double_VAR (tessedit_reject_block_percent, 45.00,
"%rej allowed before rej whole block");
EXTERN double_VAR (tessedit_reject_row_percent, 40.00,
"%rej allowed before rej whole row");
EXTERN double_VAR (tessedit_whole_wd_rej_row_percent, 70.00,
"%of row rejects in whole word rejects which prevents whole row rejection");
EXTERN BOOL_VAR (tessedit_preserve_blk_rej_perfect_wds, TRUE,
"Only rej partially rejected words in block rejection");
EXTERN BOOL_VAR (tessedit_preserve_row_rej_perfect_wds, TRUE,
"Only rej partially rejected words in row rejection");
EXTERN BOOL_VAR (tessedit_dont_blkrej_good_wds, FALSE,
"Use word segmentation quality metric");
EXTERN BOOL_VAR (tessedit_dont_rowrej_good_wds, FALSE,
"Use word segmentation quality metric");
EXTERN INT_VAR (tessedit_preserve_min_wd_len, 2,
"Only preserve wds longer than this");
EXTERN BOOL_VAR (tessedit_row_rej_good_docs, TRUE,
"Apply row rejection to good docs");
EXTERN double_VAR (tessedit_good_doc_still_rowrej_wd, 1.1,
"rej good doc wd if more than this fraction rejected");
EXTERN BOOL_VAR (tessedit_reject_bad_qual_wds, TRUE,
"Reject all bad quality wds");
EXTERN BOOL_VAR (tessedit_debug_doc_rejection, FALSE, "Page stats");
EXTERN BOOL_VAR (tessedit_debug_quality_metrics, FALSE,
"Output data to debug file");
EXTERN BOOL_VAR (bland_unrej, FALSE, "unrej potential with no checks");
EXTERN double_VAR (quality_rowrej_pc, 1.1,
"good_quality_doc gte good char limit");

EXTERN BOOL_VAR (unlv_tilde_crunching, TRUE,
"Mark v.bad words for tilde crunch");
EXTERN BOOL_VAR (crunch_early_merge_tess_fails, TRUE, "Before word crunch?");
EXTERN BOOL_EVAR (crunch_early_convert_bad_unlv_chs, FALSE,
"Take out ~^ early?");

EXTERN double_VAR (crunch_terrible_rating, 80.0, "crunch rating lt this");
EXTERN BOOL_VAR (crunch_terrible_garbage, TRUE, "As it says");
EXTERN double_VAR (crunch_poor_garbage_cert, -9.0,
"crunch garbage cert lt this");
EXTERN double_VAR (crunch_poor_garbage_rate, 60,
"crunch garbage rating lt this");

EXTERN double_VAR (crunch_pot_poor_rate, 40,
"POTENTIAL crunch rating lt this");
EXTERN double_VAR (crunch_pot_poor_cert, -8.0,
"POTENTIAL crunch cert lt this");
EXTERN BOOL_VAR (crunch_pot_garbage, TRUE, "POTENTIAL crunch garbage");

EXTERN double_VAR (crunch_del_rating, 60, "POTENTIAL crunch rating lt this");
EXTERN double_VAR (crunch_del_cert, -10.0, "POTENTIAL crunch cert lt this");
EXTERN double_VAR (crunch_del_min_ht, 0.7, "Del if word ht lt xht x this");
EXTERN double_VAR (crunch_del_max_ht, 3.0, "Del if word ht gt xht x this");
EXTERN double_VAR (crunch_del_min_width, 3.0,
"Del if word width lt xht x this");
EXTERN double_VAR (crunch_del_high_word, 1.5,
"Del if word gt xht x this above bl");
EXTERN double_VAR (crunch_del_low_word, 0.5,
"Del if word gt xht x this below bl");
EXTERN double_VAR (crunch_small_outlines_size, 0.6, "Small if lt xht x this");

EXTERN INT_VAR (crunch_rating_max, 10, "For adj length in rating per ch");
EXTERN INT_VAR (crunch_pot_indicators, 1,
"How many potential indicators needed");

EXTERN BOOL_VAR (crunch_leave_ok_strings, TRUE,
"Dont touch sensible strings");
EXTERN BOOL_VAR (crunch_accept_ok, TRUE, "Use acceptability in okstring");
EXTERN BOOL_VAR (crunch_leave_accept_strings, FALSE,
"Dont pot crunch sensible strings");
EXTERN BOOL_VAR (crunch_include_numerals, FALSE, "Fiddle alpha figures");
EXTERN INT_VAR (crunch_leave_lc_strings, 4,
"Dont crunch words with long lower case strings");
EXTERN INT_VAR (crunch_leave_uc_strings, 4,
"Dont crunch words with long lower case strings");
EXTERN INT_VAR (crunch_long_repetitions, 3,
"Crunch words with long repetitions");

EXTERN INT_VAR (crunch_debug, 0, "As it says");

static BOOL8 crude_match_blobs(PBLOB *blob1, PBLOB *blob2);
static void unrej_good_chs(WERD_RES *word, ROW *row);

/*************************************************************************
 * word_blob_quality()
 * How many blobs in the outword are identical to those of the inword?
 * ASSUME blobs in both initial word and outword are in ascending order of
 * left hand blob edge.
 *************************************************************************/
inT16 word_blob_quality(  //Blob seg changes
                        WERD_RES *word,
                        ROW *row) {
  WERD *bln_word;                //BL norm init word
  TWERD *tessword;               //tess format
  WERD *init_word;               //BL norm init word
  PBLOB_IT outword_it;
  PBLOB_IT initial_it;
  inT16 i;
  inT16 init_blobs_left;
  inT16 match_count = 0;
  BOOL8 matched;
  TBOX out_box;
  PBLOB *test_blob;
  DENORM denorm;
  float bln_xht;

  if (word->word->gblob_list ()->empty ())
    return 0;
                                 //xht used for blnorm
  bln_xht = bln_x_height / word->denorm.scale ();
  bln_word = make_bln_copy(word->word, row, NULL, bln_xht, &denorm);
  /*
    NOTE: Need to convert to tess format and back again to ensure that the
    same float -> int rounding of coords is done to source wd as out wd before
    comparison
  */
  tessword = make_tess_word(bln_word, NULL);  // Convert word.
  init_word = make_ed_word(tessword, bln_word);
  delete bln_word;
  delete_word(tessword);
  if (init_word == NULL) {
    // Conversion failed.
    return 0;
  }

  initial_it.set_to_list(init_word->blob_list());
  init_blobs_left = initial_it.length();
  outword_it.set_to_list(word->outword->blob_list());

  for (outword_it.mark_cycle_pt();
       !outword_it.cycled_list(); outword_it.forward()) {
    out_box = outword_it.data()->bounding_box();

    // Skip any initial blobs LEFT of current outword blob.
    while (!initial_it.at_last() &&
           (initial_it.data()->bounding_box().left() < out_box.left())) {
      initial_it.forward();
      init_blobs_left--;
    }

    /* See if current outword blob matches any initial blob with the same left
      coord. (Normally only one but possibly more - in unknown order) */

    i = 0;
    matched = FALSE;
    do {
      test_blob = initial_it.data_relative (i++);
      matched = crude_match_blobs (test_blob, outword_it.data ());
      if (matched)
        match_count++;
    }
    while (!matched &&
           (init_blobs_left - i > 0) &&
           (i < 129) &&
           !initial_it.at_last() &&
           test_blob->bounding_box().left() == out_box.left());
  }
  delete init_word;
  return match_count;
}


/*************************************************************************
 * crude_match_blobs()
 * Check bounding boxes are the same and the number of outlines are the same.
 *************************************************************************/
static BOOL8 crude_match_blobs(PBLOB *blob1, PBLOB *blob2) {
  TBOX box1 = blob1->bounding_box();
  TBOX box2 = blob2->bounding_box();

  if (box1.contains(box2) &&
      box2.contains(box1) &&
      (blob1->out_list()->length() == blob1->out_list()->length()))
    return TRUE;
  else
    return FALSE;
}


inT16 word_outline_errs(WERD_RES *word) {
  PBLOB_IT outword_it;
  inT16 i = 0;
  inT16 err_count = 0;

  outword_it.set_to_list(word->outword->blob_list());

  for (outword_it.mark_cycle_pt();
       !outword_it.cycled_list(); outword_it.forward()) {
    err_count += count_outline_errs(word->best_choice->unichar_string()[i],
                                    outword_it.data()->out_list()->length());
    i++;
  }
  return err_count;
}


/*************************************************************************
 * word_char_quality()
 * Combination of blob quality and outline quality - how many good chars are
 * there? - I.e chars which pass the blob AND outline tests.
 *************************************************************************/
void word_char_quality(WERD_RES *word,
                       ROW *row,
                       inT16 *match_count,
                       inT16 *accepted_match_count) {
  WERD *bln_word;                // BL norm init word
  TWERD *tessword;               // tess format
  WERD *init_word;               // BL norm init word
  PBLOB_IT outword_it;
  PBLOB_IT initial_it;
  inT16 i;
  inT16 init_blobs_left;
  BOOL8 matched;
  TBOX out_box;
  PBLOB *test_blob;
  DENORM denorm;
  float bln_xht;
  inT16 j = 0;

  *match_count = 0;
  *accepted_match_count = 0;
  if (word->word->gblob_list ()->empty ())
    return;

                                 // xht used for blnorm
  bln_xht = bln_x_height / word->denorm.scale();
  bln_word = make_bln_copy(word->word, row, NULL, bln_xht, &denorm);
  /*
    NOTE: Need to convert to tess format and back again to ensure that the
    same float -> int rounding of coords is done to source wd as out wd before
    comparison
  */
  tessword = make_tess_word(bln_word, NULL);  // Convert word.
  init_word = make_ed_word(tessword, bln_word);
  delete bln_word;
  delete_word(tessword);
  if (init_word == NULL)
    return;

  initial_it.set_to_list(init_word->blob_list());
  init_blobs_left = initial_it.length();
  outword_it.set_to_list(word->outword->blob_list());

  for (outword_it.mark_cycle_pt();
  !outword_it.cycled_list(); outword_it.forward()) {
    out_box = outword_it.data()->bounding_box();

    /* Skip any initial blobs LEFT of current outword blob */
    while (!initial_it.at_last() &&
           (initial_it.data()->bounding_box().left() < out_box.left())) {
           initial_it.forward();
      init_blobs_left--;
    }

    /* See if current outword blob matches any initial blob with the same left
      coord. (Normally only one but possibly more - in unknown order) */

    i = 0;
    matched = FALSE;
    do {
      test_blob = initial_it.data_relative(i++);
      matched = crude_match_blobs(test_blob, outword_it.data());
      if (matched &&
        (count_outline_errs (word->best_choice->unichar_string()[j],
        outword_it.data ()->out_list ()->length ())
      == 0)) {
        (*match_count)++;
        if (word->reject_map[j].accepted ())
          (*accepted_match_count)++;
      }
    }
    while (!matched &&
           (init_blobs_left - i > 0) &&
           (i < 129) &&
           !initial_it.at_last() &&
           test_blob->bounding_box().left() == out_box.left());
    j++;
  }
  delete init_word;
}


/*************************************************************************
 * unrej_good_chs()
 * Unreject POTENTIAL rejects if the blob passes the blob and outline checks
 *************************************************************************/
static void unrej_good_chs(WERD_RES *word, ROW *row) {
  WERD *bln_word;                // BL norm init word
  TWERD *tessword;               // tess format
  WERD *init_word;               // BL norm init word
  PBLOB_IT outword_it;
  PBLOB_IT initial_it;
  inT16 i;
  inT16 init_blobs_left;
  BOOL8 matched;
  TBOX out_box;
  PBLOB *test_blob;
  DENORM denorm;
  float bln_xht;
  inT16 j = 0;

  if (word->word->gblob_list ()->empty ())
    return;

                                 // xht used for blnorm
  bln_xht = bln_x_height / word->denorm.scale ();
  bln_word = make_bln_copy(word->word, row, NULL, bln_xht, &denorm);
  /*
    NOTE: Need to convert to tess format and back again to ensure that the
    same float -> int rounding of coords is done to source wd as out wd before
    comparison
  */
  tessword = make_tess_word(bln_word, NULL);  // Convert word
  init_word = make_ed_word(tessword, bln_word);
  delete bln_word;
  delete_word(tessword);
  if (init_word == NULL)
    return;

  initial_it.set_to_list (init_word->blob_list ());
  init_blobs_left = initial_it.length ();
  outword_it.set_to_list (word->outword->blob_list ());

  for (outword_it.mark_cycle_pt ();
  !outword_it.cycled_list (); outword_it.forward ()) {
    out_box = outword_it.data ()->bounding_box ();

    /* Skip any initial blobs LEFT of current outword blob */
    while (!initial_it.at_last () &&
    (initial_it.data ()->bounding_box ().left () < out_box.left ())) {
      initial_it.forward ();
      init_blobs_left--;
    }

    /* See if current outword blob matches any initial blob with the same left
      coord. (Normally only one but possibly more - in unknown order) */

    i = 0;
    matched = FALSE;
    do {
      test_blob = initial_it.data_relative (i++);
      matched = crude_match_blobs (test_blob, outword_it.data ());
      if (matched &&
        (word->reject_map[j].accept_if_good_quality ()) &&
        (docqual_excuse_outline_errs ||
        (count_outline_errs (word->best_choice->unichar_string()[j],
        outword_it.data ()->out_list ()->
        length ()) == 0)))
        word->reject_map[j].setrej_quality_accept ();
    }
    while (!matched &&
      (init_blobs_left - i > 0) &&
      (i < 129) &&
      !initial_it.at_last () &&
      test_blob->bounding_box ().left () == out_box.left ());
    j++;
  }
  delete init_word;
}


void print_boxes(WERD *word) {
  PBLOB_IT it;
  TBOX box;

  it.set_to_list (word->blob_list ());
  for (it.mark_cycle_pt (); !it.cycled_list (); it.forward ()) {
    box = it.data ()->bounding_box ();
    box.print ();
  }
}


inT16 count_outline_errs(char c, inT16 outline_count) {
  int expected_outline_count;

  if (STRING (outlines_odd).contains (c))
    return 0;                    //Dont use this char
  else if (STRING (outlines_2).contains (c))
    expected_outline_count = 2;
  else
    expected_outline_count = 1;
  return abs (outline_count - expected_outline_count);
}


namespace tesseract {
void Tesseract::quality_based_rejection(PAGE_RES_IT &page_res_it,
                                        BOOL8 good_quality_doc) {
  if ((tessedit_good_quality_unrej && good_quality_doc))
    unrej_good_quality_words(page_res_it);
  doc_and_block_rejection(page_res_it, good_quality_doc);

  page_res_it.restart_page ();
  while (page_res_it.word () != NULL) {
    insert_rej_cblobs(page_res_it.word());
    page_res_it.forward();
  }

  if (unlv_tilde_crunching) {
    tilde_crunch(page_res_it);
    tilde_delete(page_res_it);
  }
}


/*************************************************************************
 * unrej_good_quality_words()
 * Accept potential rejects in words which pass the following checks:
 *    - Contains a potential reject
 *    - Word looks like a sensible alpha word.
 *    - Word segmentation is the same as the original image
 *		- All characters have the expected number of outlines
 * NOTE - the rejection counts are recalculated after unrejection
 *      - CANT do it in a single pass without a bit of fiddling
 *		- keep it simple but inefficient
 *************************************************************************/
void Tesseract::unrej_good_quality_words(  //unreject potential
                                         PAGE_RES_IT &page_res_it) {
  WERD_RES *word;
  ROW_RES *current_row;
  BLOCK_RES *current_block;
  int i;

  page_res_it.restart_page ();
  while (page_res_it.word () != NULL) {
    check_debug_pt (page_res_it.word (), 100);
    if (bland_unrej) {
      word = page_res_it.word ();
      for (i = 0; i < word->reject_map.length (); i++) {
        if (word->reject_map[i].accept_if_good_quality ())
          word->reject_map[i].setrej_quality_accept ();
      }
      page_res_it.forward ();
    }
    else if ((page_res_it.row ()->char_count > 0) &&
      ((page_res_it.row ()->rej_count /
      (float) page_res_it.row ()->char_count) <=
    quality_rowrej_pc)) {
      word = page_res_it.word ();
      if (word->reject_map.quality_recoverable_rejects () &&
        (tessedit_unrej_any_wd ||
        acceptable_word_string (word->best_choice->unichar_string().string(),
                                word->best_choice->unichar_lengths().string())
      != AC_UNACCEPTABLE)) {
        unrej_good_chs (word, page_res_it.row ()->row);
      }
      page_res_it.forward ();
    }
    else {
      /* Skip to end of dodgy row */
      current_row = page_res_it.row ();
      while ((page_res_it.word () != NULL) &&
        (page_res_it.row () == current_row))
        page_res_it.forward ();
    }
    check_debug_pt (page_res_it.word (), 110);
  }
  page_res_it.restart_page ();
  page_res_it.page_res->char_count = 0;
  page_res_it.page_res->rej_count = 0;
  current_block = NULL;
  current_row = NULL;
  while (page_res_it.word () != NULL) {
    if (current_block != page_res_it.block ()) {
      current_block = page_res_it.block ();
      current_block->char_count = 0;
      current_block->rej_count = 0;
    }
    if (current_row != page_res_it.row ()) {
      current_row = page_res_it.row ();
      current_row->char_count = 0;
      current_row->rej_count = 0;
      current_row->whole_word_rej_count = 0;
    }
    page_res_it.rej_stat_word ();
    page_res_it.forward ();
  }
}


/*************************************************************************
 * doc_and_block_rejection()
 *
 * If the page has too many rejects - reject all of it.
 * If any block has too many rejects - reject all words in the block
 *************************************************************************/

void Tesseract::doc_and_block_rejection(  //reject big chunks
                                        PAGE_RES_IT &page_res_it,
                                        BOOL8 good_quality_doc) {
  inT16 block_no = 0;
  inT16 row_no = 0;
  BLOCK_RES *current_block;
  ROW_RES *current_row;

  BOOL8 rej_word;
  BOOL8 prev_word_rejected;
  inT16 char_quality;
  inT16 accepted_char_quality;

  if ((page_res_it.page_res->rej_count * 100.0 /
  page_res_it.page_res->char_count) > tessedit_reject_doc_percent) {
    reject_whole_page(page_res_it);
    #ifndef SECURE_NAMES
    if (tessedit_debug_doc_rejection) {
      tprintf ("REJECT ALL #chars: %d #Rejects: %d; \n",
        page_res_it.page_res->char_count,
        page_res_it.page_res->rej_count);
    }
    #endif
  }
  else {
    #ifndef SECURE_NAMES
    if (tessedit_debug_doc_rejection)
      tprintf ("NO PAGE REJECTION #chars: %d  # Rejects: %d; \n",
        page_res_it.page_res->char_count,
        page_res_it.page_res->rej_count);
    #endif

    /* Walk blocks testing for block rejection */

    page_res_it.restart_page ();
    while (page_res_it.word () != NULL) {
      current_block = page_res_it.block();
      block_no = current_block->block->index();
      if ((page_res_it.block ()->char_count > 0) &&
        ((page_res_it.block ()->rej_count * 100.0 /
        page_res_it.block ()->char_count) >
      tessedit_reject_block_percent)) {
        #ifndef SECURE_NAMES
        if (tessedit_debug_block_rejection)
          tprintf ("REJECTING BLOCK %d  #chars: %d;  #Rejects: %d\n",
            block_no,
            page_res_it.block ()->char_count,
            page_res_it.block ()->rej_count);
        #endif
        prev_word_rejected = FALSE;
        while ((page_res_it.word () != NULL) &&
        (page_res_it.block () == current_block)) {
          if (tessedit_preserve_blk_rej_perfect_wds) {
            rej_word =
              (page_res_it.word ()->reject_map.reject_count () > 0)
              || (page_res_it.word ()->reject_map.length () <
              tessedit_preserve_min_wd_len);
            if (rej_word && tessedit_dont_blkrej_good_wds
              && !(page_res_it.word ()->reject_map.length () <
              tessedit_preserve_min_wd_len)
              &&
              (acceptable_word_string
               (page_res_it.word()->best_choice->unichar_string().string(),
               page_res_it.word ()->best_choice->unichar_lengths().string()) !=
               AC_UNACCEPTABLE)) {
              word_char_quality (page_res_it.word (),
                page_res_it.row ()->row,
                &char_quality,
                &accepted_char_quality);
              rej_word = char_quality !=
                page_res_it.word ()->reject_map.length ();
            }
          }
          else
            rej_word = TRUE;
          if (rej_word) {
            /*
              Reject spacing if both current and prev words are rejected.
              NOTE - this is NOT restricted to FUZZY spaces. - When tried this
              generated more space errors.
            */
            if (tessedit_use_reject_spaces &&
              prev_word_rejected &&
              (page_res_it.prev_row () == page_res_it.row ()) &&
              (page_res_it.word ()->word->space () == 1))
              page_res_it.word ()->reject_spaces = TRUE;
            page_res_it.word ()->reject_map.rej_word_block_rej ();
          }
          prev_word_rejected = rej_word;
          page_res_it.forward ();
        }
      }
      else {
        #ifndef SECURE_NAMES
        if (tessedit_debug_block_rejection)
          tprintf
            ("NOT REJECTING BLOCK %d #chars: %d  # Rejects: %d; \n",
            block_no, page_res_it.block ()->char_count,
            page_res_it.block ()->rej_count);
        #endif

        /* Walk rows in block testing for row rejection */
        row_no = 0;
        while ((page_res_it.word () != NULL) &&
        (page_res_it.block () == current_block)) {
          current_row = page_res_it.row ();
          row_no++;
          /* Reject whole row if:
            fraction of chars on row which are rejected exceed a limit AND
            fraction rejects which occur in WHOLE WERD rejects is LESS THAN a
            limit
          */
          if ((page_res_it.row ()->char_count > 0) &&
            ((page_res_it.row ()->rej_count * 100.0 /
            page_res_it.row ()->char_count) >
            tessedit_reject_row_percent) &&
            ((page_res_it.row ()->whole_word_rej_count * 100.0 /
            page_res_it.row ()->rej_count) <
          tessedit_whole_wd_rej_row_percent)) {
            #ifndef SECURE_NAMES
            if (tessedit_debug_block_rejection)
              tprintf
                ("REJECTING ROW %d  #chars: %d;  #Rejects: %d\n",
                row_no, page_res_it.row ()->char_count,
                page_res_it.row ()->rej_count);
            #endif
            prev_word_rejected = FALSE;
            while ((page_res_it.word () != NULL) &&
            (page_res_it.row () == current_row)) {
              /* Preserve words on good docs unless they are mostly rejected*/
              if (!tessedit_row_rej_good_docs && good_quality_doc) {
                rej_word =
                  page_res_it.word ()->reject_map.
                  reject_count () /
                  (float) page_res_it.word ()->reject_map.
                  length () > tessedit_good_doc_still_rowrej_wd;
              }

              /* Preserve perfect words anyway */
              else if (tessedit_preserve_row_rej_perfect_wds) {
                rej_word =
                  (page_res_it.word ()->reject_map.
                  reject_count () > 0)
                  || (page_res_it.word ()->reject_map.
                  length () < tessedit_preserve_min_wd_len);
                if (rej_word && tessedit_dont_rowrej_good_wds
                  && !(page_res_it.word ()->reject_map.
                  length () <
                  tessedit_preserve_min_wd_len)
                  &&
                  (acceptable_word_string
                   (page_res_it.word ()->best_choice->
                    unichar_string().string(),
                    page_res_it.word ()->best_choice->
                    unichar_lengths().string()) != AC_UNACCEPTABLE)) {
                  word_char_quality (page_res_it.word (),
                    page_res_it.row ()->row,
                    &char_quality,
                    &accepted_char_quality);
                  rej_word = char_quality !=
                    page_res_it.word ()->reject_map.length ();
                }
              }
              else
                rej_word = TRUE;
              if (rej_word) {
                /*
                  Reject spacing if both current and prev words are rejected.
                  NOTE - this is NOT restricted to FUZZY spaces. - When tried
                  this generated more space errors.
                */
                if (tessedit_use_reject_spaces &&
                  prev_word_rejected &&
                  (page_res_it.prev_row () ==
                  page_res_it.row ())
                  && (page_res_it.word ()->word->space () ==
                  1))
                  page_res_it.word ()->reject_spaces = TRUE;
                page_res_it.word ()->reject_map.
                  rej_word_row_rej();
              }
              prev_word_rejected = rej_word;
              page_res_it.forward ();
            }
          }
          else {
            #ifndef SECURE_NAMES
            if (tessedit_debug_block_rejection)
              tprintf
                ("NOT REJECTING ROW %d #chars: %d  # Rejects: %d; \n",
                row_no, page_res_it.row ()->char_count,
                page_res_it.row ()->rej_count);
            #endif
            while ((page_res_it.word () != NULL) &&
              (page_res_it.row () == current_row))
              page_res_it.forward ();
          }
        }
      }
    }
  }
}
}  // namespace tesseract


/*************************************************************************
 * reject_whole_page()
 * Dont believe any of it - set the reject map to 00..00 in all words
 *
 *************************************************************************/

void reject_whole_page(PAGE_RES_IT &page_res_it) {
  page_res_it.restart_page ();
  while (page_res_it.word () != NULL) {
    page_res_it.word ()->reject_map.rej_word_doc_rej ();
    page_res_it.forward ();
  }
                                 //whole page is rejected
  page_res_it.page_res->rejected = TRUE;
}

namespace tesseract {
void Tesseract::tilde_crunch(PAGE_RES_IT &page_res_it) {
  WERD_RES *word;
  GARBAGE_LEVEL garbage_level;
  PAGE_RES_IT copy_it;
  BOOL8 prev_potential_marked = FALSE;
  BOOL8 found_terrible_word = FALSE;
  BOOL8 ok_dict_word;

  page_res_it.restart_page ();
  while (page_res_it.word () != NULL) {
    word = page_res_it.word ();

    if (crunch_early_convert_bad_unlv_chs)
      convert_bad_unlv_chs(word);

    if (crunch_early_merge_tess_fails)
      merge_tess_fails(word);

    if (word->reject_map.accept_count () != 0) {
      found_terrible_word = FALSE;
                                 //Forget earlier potential crunches
      prev_potential_marked = FALSE;
    }
    else {
      ok_dict_word = safe_dict_word(*(word->best_choice));
      garbage_level = garbage_word (word, ok_dict_word);

      if ((garbage_level != G_NEVER_CRUNCH) &&
      (terrible_word_crunch (word, garbage_level))) {
        if (crunch_debug > 0) {
          tprintf ("T CRUNCHING: \"%s\"\n",
            word->best_choice->unichar_string().string());
        }
        word->unlv_crunch_mode = CR_KEEP_SPACE;
        if (prev_potential_marked) {
          while (copy_it.word () != word) {
            if (crunch_debug > 0) {
              tprintf ("P1 CRUNCHING: \"%s\"\n",
                copy_it.word()->best_choice->unichar_string().string());
            }
            copy_it.word ()->unlv_crunch_mode = CR_KEEP_SPACE;
            copy_it.forward ();
          }
          prev_potential_marked = FALSE;
        }
        found_terrible_word = TRUE;
      }
      else if ((garbage_level != G_NEVER_CRUNCH) &&
        (potential_word_crunch (word,
      garbage_level, ok_dict_word))) {
        if (found_terrible_word) {
          if (crunch_debug > 0) {
            tprintf ("P2 CRUNCHING: \"%s\"\n",
              word->best_choice->unichar_string().string());
          }
          word->unlv_crunch_mode = CR_KEEP_SPACE;
        }
        else if (!prev_potential_marked) {
          copy_it = page_res_it;
          prev_potential_marked = TRUE;
          if (crunch_debug > 1) {
            tprintf ("P3 CRUNCHING: \"%s\"\n",
              word->best_choice->unichar_string().string());
          }
        }
      }
      else {
        found_terrible_word = FALSE;
                                 //Forget earlier potential crunches
        prev_potential_marked = FALSE;
        if (crunch_debug > 2) {
          tprintf ("NO CRUNCH: \"%s\"\n",
            word->best_choice->unichar_string().string());
        }
      }
    }
    page_res_it.forward ();
  }
}
}  // namespace tesseract


BOOL8 terrible_word_crunch(WERD_RES *word, GARBAGE_LEVEL garbage_level) {
  float rating_per_ch;
  int adjusted_len;
  int crunch_mode = 0;

  if ((word->best_choice->unichar_string().length () == 0) ||
    (strspn (word->best_choice->unichar_string().string(), " ") ==
    word->best_choice->unichar_string().length ()))
    crunch_mode = 1;
  else {
    adjusted_len = word->reject_map.length ();
    if (adjusted_len > crunch_rating_max)
      adjusted_len = crunch_rating_max;
    rating_per_ch = word->best_choice->rating () / adjusted_len;

    if (rating_per_ch > crunch_terrible_rating)
      crunch_mode = 2;
    else if (crunch_terrible_garbage && (garbage_level == G_TERRIBLE))
      crunch_mode = 3;
    else if ((word->best_choice->certainty () < crunch_poor_garbage_cert) &&
      (garbage_level != G_OK))
      crunch_mode = 4;
    else if ((rating_per_ch > crunch_poor_garbage_rate) &&
      (garbage_level != G_OK))
      crunch_mode = 5;
  }
  if (crunch_mode > 0) {
    if (crunch_debug > 2) {
      tprintf ("Terrible_word_crunch (%d) on \"%s\"\n",
        crunch_mode, word->best_choice->unichar_string().string());
    }
    return TRUE;
  }
  else
    return FALSE;
}

namespace tesseract {
BOOL8 Tesseract::potential_word_crunch(WERD_RES *word,
                                       GARBAGE_LEVEL garbage_level,
                                       BOOL8 ok_dict_word) {
  float rating_per_ch;
  int adjusted_len;
  const char *str = word->best_choice->unichar_string().string();
  const char *lengths = word->best_choice->unichar_lengths().string();
  BOOL8 word_crunchable;
  int poor_indicator_count = 0;

  word_crunchable =
    !crunch_leave_accept_strings ||
    (word->reject_map.length () < 3) ||
    ((acceptable_word_string (str, lengths) == AC_UNACCEPTABLE) &&
     !ok_dict_word);

  adjusted_len = word->reject_map.length ();
  if (adjusted_len > 10)
    adjusted_len = 10;
  rating_per_ch = word->best_choice->rating () / adjusted_len;

  if (rating_per_ch > crunch_pot_poor_rate) {
    if (crunch_debug > 2) {
      tprintf ("Potential poor rating on \"%s\"\n",
        word->best_choice->unichar_string().string());
    }
    poor_indicator_count++;
  }

  if (word_crunchable &&
  (word->best_choice->certainty () < crunch_pot_poor_cert)) {
    if (crunch_debug > 2) {
      tprintf ("Potential poor cert on \"%s\"\n",
        word->best_choice->unichar_string().string());
    }
    poor_indicator_count++;
  }

  if (garbage_level != G_OK) {
    if (crunch_debug > 2) {
      tprintf ("Potential garbage on \"%s\"\n",
        word->best_choice->unichar_string().string());
    }
    poor_indicator_count++;
  }
  return (poor_indicator_count >= crunch_pot_indicators);
}
}  // namespace tesseract


namespace tesseract {
void Tesseract::tilde_delete(PAGE_RES_IT &page_res_it) {
  WERD_RES *word;
  PAGE_RES_IT copy_it;
  BOOL8 deleting_from_bol = FALSE;
  BOOL8 marked_delete_point = FALSE;
  inT16 debug_delete_mode;
  CRUNCH_MODE delete_mode;
  inT16 x_debug_delete_mode;
  CRUNCH_MODE x_delete_mode;

  page_res_it.restart_page ();
  while (page_res_it.word () != NULL) {
    word = page_res_it.word ();

    delete_mode = word_deletable (word, debug_delete_mode);
    if (delete_mode != CR_NONE) {
      if (word->word->flag (W_BOL) || deleting_from_bol) {
        if (crunch_debug > 0) {
          tprintf ("BOL CRUNCH DELETING(%d): \"%s\"\n",
            debug_delete_mode,
            word->best_choice->unichar_string().string());
        }
        word->unlv_crunch_mode = delete_mode;
        deleting_from_bol = TRUE;
      }
      else if (word->word->flag (W_EOL)) {
        if (marked_delete_point) {
          while (copy_it.word () != word) {
            x_delete_mode = word_deletable (copy_it.word (),
              x_debug_delete_mode);
            if (crunch_debug > 0) {
              tprintf ("EOL CRUNCH DELETING(%d): \"%s\"\n",
                x_debug_delete_mode,
                copy_it.word()->best_choice->unichar_string().string());
            }
            copy_it.word ()->unlv_crunch_mode = x_delete_mode;
            copy_it.forward ();
          }
        }
        if (crunch_debug > 0) {
          tprintf ("EOL CRUNCH DELETING(%d): \"%s\"\n",
            debug_delete_mode,
            word->best_choice->unichar_string().string());
        }
        word->unlv_crunch_mode = delete_mode;
        deleting_from_bol = FALSE;
        marked_delete_point = FALSE;
      }
      else {
        if (!marked_delete_point) {
          copy_it = page_res_it;
          marked_delete_point = TRUE;
        }
      }
    }
    else {
      deleting_from_bol = FALSE;
                                 //Forget earlier potential crunches
      marked_delete_point = FALSE;
    }
    /*
      The following step has been left till now as the tess fails are used to
      determine if the word is deletable.
    */
    if (!crunch_early_merge_tess_fails)
      merge_tess_fails(word);
    page_res_it.forward ();
  }
}


void Tesseract::convert_bad_unlv_chs(WERD_RES *word_res) {
  int i;
  UNICHAR_ID unichar_dash = unicharset.unichar_to_id("-");
  UNICHAR_ID unichar_space = unicharset.unichar_to_id(" ");
  UNICHAR_ID unichar_tilde = unicharset.unichar_to_id("~");
  UNICHAR_ID unichar_pow = unicharset.unichar_to_id("^");
  bool modified = false;
  for (i = 0; i < word_res->reject_map.length(); ++i) {
    if (word_res->best_choice->unichar_id(i) == unichar_tilde) {
      word_res->best_choice->set_unichar_id(unichar_dash, i);
      modified = true;
      if (word_res->reject_map[i].accepted ())
        word_res->reject_map[i].setrej_unlv_rej ();
    }
    if (word_res->best_choice->unichar_id(i) == unichar_pow) {
      word_res->best_choice->set_unichar_id(unichar_space, i);
      modified = true;
      if (word_res->reject_map[i].accepted ())
        word_res->reject_map[i].setrej_unlv_rej ();
    }
  }
  if (modified) {
    word_res->best_choice->populate_unichars(unicharset);
  }
}

// Change pairs of tess failures to a single one
void Tesseract::merge_tess_fails(WERD_RES *word_res) {
  PBLOB_IT blob_it;              //blobs
  int len = word_res->best_choice->length();
  bool modified = false;

  ASSERT_HOST (word_res->reject_map.length () == len);
  ASSERT_HOST (word_res->outword->blob_list ()->length () == len);

  UNICHAR_ID unichar_space = unicharset.unichar_to_id(" ");
  blob_it = word_res->outword->blob_list ();
  int i = 0;
  while (i < word_res->best_choice->length()-1) {
    if ((word_res->best_choice->unichar_id(i) == unichar_space) &&
        (word_res->best_choice->unichar_id(i+1) == unichar_space)) {
      modified = true;
      word_res->best_choice->remove_unichar_id(i);
      word_res->reject_map.remove_pos (i);
      merge_blobs (blob_it.data_relative (1), blob_it.data ());
      delete blob_it.extract (); //get rid of spare
    } else {
      i++;
    }
    blob_it.forward ();
  }
  len = word_res->best_choice->length();
  ASSERT_HOST (word_res->reject_map.length () == len);
  ASSERT_HOST (word_res->outword->blob_list ()->length () == len);
  if (modified) {
    word_res->best_choice->populate_unichars(unicharset);
  }
}

GARBAGE_LEVEL Tesseract::garbage_word(WERD_RES *word, BOOL8 ok_dict_word) {
  enum STATES
  {
    JUNK,
    FIRST_UPPER,
    FIRST_LOWER,
    FIRST_NUM,
    SUBSEQUENT_UPPER,
    SUBSEQUENT_LOWER,
    SUBSEQUENT_NUM
  };
  const char *str = word->best_choice->unichar_string().string();
  const char *lengths = word->best_choice->unichar_lengths().string();
  STATES state = JUNK;
  int len = 0;
  int isolated_digits = 0;
  int isolated_alphas = 0;
  int bad_char_count = 0;
  int tess_rejs = 0;
  int dodgy_chars = 0;
  int ok_chars;
  UNICHAR_ID last_char = -1;
  int alpha_repetition_count = 0;
  int longest_alpha_repetition_count = 0;
  int longest_lower_run_len = 0;
  int lower_string_count = 0;
  int longest_upper_run_len = 0;
  int upper_string_count = 0;
  int total_alpha_count = 0;
  int total_digit_count = 0;

  for (; *str != '\0'; str += *(lengths++)) {
    len++;
    if (unicharset.get_isupper (str, *lengths)) {
      total_alpha_count++;
      switch (state) {
        case SUBSEQUENT_UPPER:
        case FIRST_UPPER:
          state = SUBSEQUENT_UPPER;
          upper_string_count++;
          if (longest_upper_run_len < upper_string_count)
            longest_upper_run_len = upper_string_count;
          if (last_char == unicharset.unichar_to_id(str, *lengths)) {
            alpha_repetition_count++;
            if (longest_alpha_repetition_count < alpha_repetition_count) {
              longest_alpha_repetition_count = alpha_repetition_count;
            }
          }
          else {
            last_char = unicharset.unichar_to_id(str, *lengths);
            alpha_repetition_count = 1;
          }
          break;
        case FIRST_NUM:
          isolated_digits++;
        default:
          state = FIRST_UPPER;
          last_char = unicharset.unichar_to_id(str, *lengths);
          alpha_repetition_count = 1;
          upper_string_count = 1;
          break;
      }
    }
    else if (unicharset.get_islower (str, *lengths)) {
      total_alpha_count++;
      switch (state) {
        case SUBSEQUENT_LOWER:
        case FIRST_LOWER:
          state = SUBSEQUENT_LOWER;
          lower_string_count++;
          if (longest_lower_run_len < lower_string_count)
            longest_lower_run_len = lower_string_count;
          if (last_char == unicharset.unichar_to_id(str, *lengths)) {
            alpha_repetition_count++;
            if (longest_alpha_repetition_count < alpha_repetition_count) {
              longest_alpha_repetition_count = alpha_repetition_count;
            }
          }
          else {
            last_char = unicharset.unichar_to_id(str, *lengths);
            alpha_repetition_count = 1;
          }
          break;
        case FIRST_NUM:
          isolated_digits++;
        default:
          state = FIRST_LOWER;
          last_char = unicharset.unichar_to_id(str, *lengths);
          alpha_repetition_count = 1;
          lower_string_count = 1;
          break;
      }
    }
    else if (unicharset.get_isdigit (str, *lengths)) {
      total_digit_count++;
      switch (state) {
        case FIRST_NUM:
          state = SUBSEQUENT_NUM;
        case SUBSEQUENT_NUM:
          break;
        case FIRST_UPPER:
        case FIRST_LOWER:
          isolated_alphas++;
        default:
          state = FIRST_NUM;
          break;
      }
    }
    else {
      if (*lengths == 1 && *str == ' ')
        tess_rejs++;
      else
        bad_char_count++;
      switch (state) {
        case FIRST_NUM:
          isolated_digits++;
          break;
        case FIRST_UPPER:
        case FIRST_LOWER:
          isolated_alphas++;
        default:
          break;
      }
      state = JUNK;
    }
  }

  switch (state) {
    case FIRST_NUM:
      isolated_digits++;
      break;
    case FIRST_UPPER:
    case FIRST_LOWER:
      isolated_alphas++;
    default:
      break;
  }

  if (crunch_include_numerals) {
    total_alpha_count += total_digit_count - isolated_digits;
  }

  if (crunch_leave_ok_strings &&
    (len >= 4) &&
    (2 * (total_alpha_count - isolated_alphas) > len) &&
  (longest_alpha_repetition_count < crunch_long_repetitions)) {
    if ((crunch_accept_ok &&
      (acceptable_word_string (str, lengths) != AC_UNACCEPTABLE)) ||
      (longest_lower_run_len > crunch_leave_lc_strings) ||
      (longest_upper_run_len > crunch_leave_uc_strings))
      return G_NEVER_CRUNCH;
  }
  if ((word->reject_map.length () > 1) &&
    (strpbrk (str, " ") == NULL) &&
    ((word->best_choice->permuter () == SYSTEM_DAWG_PERM) ||
    (word->best_choice->permuter () == FREQ_DAWG_PERM) ||
    (word->best_choice->permuter () == USER_DAWG_PERM) ||
    (word->best_choice->permuter () == NUMBER_PERM) ||
    (acceptable_word_string (str, lengths) != AC_UNACCEPTABLE) || ok_dict_word))
    return G_OK;

  ok_chars = len - bad_char_count - isolated_digits -
    isolated_alphas - tess_rejs;

  if (crunch_debug > 3) {
    tprintf ("garbage_word: \"%s\"\n",
      word->best_choice->unichar_string().string());
    tprintf ("LEN: %d  bad: %d  iso_N: %d  iso_A: %d  rej: %d\n",
      len,
      bad_char_count, isolated_digits, isolated_alphas, tess_rejs);
  }
  if ((bad_char_count == 0) &&
    (tess_rejs == 0) &&
    ((len > isolated_digits + isolated_alphas) || (len <= 2)))
    return G_OK;

  if ((tess_rejs > ok_chars) ||
    ((tess_rejs > 0) && ((bad_char_count + tess_rejs) * 2 > len)))
    return G_TERRIBLE;

  if (len > 4) {
    dodgy_chars = 2 * tess_rejs + bad_char_count +
      isolated_digits + isolated_alphas;
    if ((dodgy_chars > 5) || ((dodgy_chars / (float) len) > 0.5))
      return G_DODGY;
    else
      return G_OK;
  }
  else {
    dodgy_chars = 2 * tess_rejs + bad_char_count;
    if (((len == 4) && (dodgy_chars > 2)) ||
      ((len == 3) && (dodgy_chars > 2)) || (dodgy_chars >= len))
      return G_DODGY;
    else
      return G_OK;
  }
}
}  // namespace tesseract


/*************************************************************************
 * word_deletable()
 *     DELETE WERDS AT ENDS OF ROWS IF
 *        Word is crunched &&
 *        ( string length = 0                                          OR
 *          > 50% of chars are "|" (before merging)                    OR
 *          certainty < -10                                            OR
 *          rating /char > 60                                          OR
 *          TOP of word is more than 0.5 xht BELOW baseline            OR
 *          BOTTOM of word is more than 0.5 xht ABOVE xht              OR
 *          length of word < 3xht                                      OR
 *          height of word < 0.7 xht                                   OR
 *          height of word > 3.0 xht                                   OR
 *          >75% of the outline BBs have longest dimension < 0.5xht
 *************************************************************************/

CRUNCH_MODE word_deletable(WERD_RES *word, inT16 &delete_mode) {
  int word_len = word->reject_map.length ();
  float rating_per_ch;
  TBOX box;                       //BB of word

  if (word->unlv_crunch_mode == CR_NONE) {
    delete_mode = 0;
    return CR_NONE;
  }

  if (word_len == 0) {
    delete_mode = 1;
    return CR_DELETE;
  }

  box = word->outword->bounding_box ();
  if (box.height () < crunch_del_min_ht * bln_x_height) {
    delete_mode = 4;
    return CR_DELETE;
  }

  if (noise_outlines (word->outword)) {
    delete_mode = 5;
    return CR_DELETE;
  }

  if ((failure_count (word) * 1.5) > word_len) {
    delete_mode = 2;
    return CR_LOOSE_SPACE;
  }

  if (word->best_choice->certainty () < crunch_del_cert) {
    delete_mode = 7;
    return CR_LOOSE_SPACE;
  }

  rating_per_ch = word->best_choice->rating () / word_len;

  if (rating_per_ch > crunch_del_rating) {
    delete_mode = 8;
    return CR_LOOSE_SPACE;
  }

  if (box.top () < bln_baseline_offset - crunch_del_low_word * bln_x_height) {
    delete_mode = 9;
    return CR_LOOSE_SPACE;
  }

  if (box.bottom () >
  bln_baseline_offset + crunch_del_high_word * bln_x_height) {
    delete_mode = 10;
    return CR_LOOSE_SPACE;
  }

  if (box.height () > crunch_del_max_ht * bln_x_height) {
    delete_mode = 11;
    return CR_LOOSE_SPACE;
  }

  if (box.width () < crunch_del_min_width * bln_x_height) {
    delete_mode = 3;
    return CR_LOOSE_SPACE;
  }

  delete_mode = 0;
  return CR_NONE;
}

inT16 failure_count(WERD_RES *word) {
  const char *str = word->best_choice->unichar_string().string();
  int tess_rejs = 0;

  for (; *str != '\0'; str++) {
    if (*str == ' ')
      tess_rejs++;
  }
  return tess_rejs;
}


BOOL8 noise_outlines(WERD *word) {
  PBLOB_IT blob_it;
  OUTLINE_IT outline_it;
  TBOX box;                       //BB of outline
  inT16 outline_count = 0;
  inT16 small_outline_count = 0;
  inT16 max_dimension;
  float small_limit = bln_x_height * crunch_small_outlines_size;

  blob_it.set_to_list (word->blob_list ());
  for (blob_it.mark_cycle_pt (); !blob_it.cycled_list (); blob_it.forward ()) {
    outline_it.set_to_list (blob_it.data ()->out_list ());
    for (outline_it.mark_cycle_pt ();
    !outline_it.cycled_list (); outline_it.forward ()) {
      outline_count++;
      box = outline_it.data ()->bounding_box ();
      if (box.height () > box.width ())
        max_dimension = box.height ();
      else
        max_dimension = box.width ();
      if (max_dimension < small_limit)
        small_outline_count++;
    }
  }
  return (small_outline_count >= outline_count);
}


/*************************************************************************
 * insert_rej_cblobs()
 * Put rejected word blobs back into the outword.
 * NOTE!!! AFTER THIS THE CHOICES LIST WILL NOT HAVE THE CORRECT NUMBER
 * OF ELEMENTS.
 *************************************************************************/
namespace tesseract {
void Tesseract::insert_rej_cblobs(WERD_RES *word) {
  PBLOB_IT blob_it;              //blob iterator
  PBLOB_IT rej_blob_it;
  const STRING *word_str;
  const STRING *word_lengths;
  int old_len;
  int rej_len;
  char new_str[512 * UNICHAR_LEN];
  char new_lengths[512];
  REJMAP new_map;
  int i = 0;                     //new_str index
  int j = 0;                     //old_str index
  int i_offset = 0;              //new_str offset
  int j_offset = 0;              //old_str offset
  int new_len;

  gblob_sort_list (word->outword->rej_blob_list (), TRUE);
  rej_blob_it.set_to_list (word->outword->rej_blob_list ());
  if (rej_blob_it.empty ())
    return;
  rej_len = rej_blob_it.length ();
  blob_it.set_to_list (word->outword->blob_list ());
  word_str = &(word->best_choice->unichar_string());
  word_lengths = &(word->best_choice->unichar_lengths());
  old_len = word->best_choice->length();
  ASSERT_HOST (word->reject_map.length () == old_len);
  ASSERT_HOST (blob_it.length () == old_len);
  if ((old_len + rej_len) > 511)
    return;                      //Word is garbage anyway prevent abort
  new_map.initialise (old_len + rej_len);

  while (!rej_blob_it.empty ()) {
    if ((j >= old_len) ||
      (rej_blob_it.data ()->bounding_box ().left () <=
    blob_it.data ()->bounding_box ().left ())) {
      /* Insert reject blob */
      if (j >= old_len)
        blob_it.add_to_end (rej_blob_it.extract ());
      else
        blob_it.add_before_stay_put (rej_blob_it.extract ());
      if (!rej_blob_it.empty ())
        rej_blob_it.forward ();
      new_str[i_offset] = ' ';
      new_lengths[i] = 1;
      new_map[i].setrej_rej_cblob ();
      i_offset += new_lengths[i++];
    }
    else {
      strncpy(new_str + i_offset, &(*word_str)[j_offset],
              (*word_lengths)[j]);
      new_lengths[i] = (*word_lengths)[j];
      new_map[i] = word->reject_map[j];
      i_offset += new_lengths[i++];
      j_offset += (*word_lengths)[j++];
      blob_it.forward ();
    }
  }
  /* Add any extra normal blobs to strings */
  while (j < word_lengths->length ()) {
    strncpy(new_str + i_offset, &(*word_str)[j_offset],
            (*word_lengths)[j]);
    new_lengths[i] = (*word_lengths)[j];
    new_map[i] = word->reject_map[j];
    i_offset += new_lengths[i++];
    j_offset += (*word_lengths)[j++];
  }
  new_str[i_offset] = '\0';
  new_lengths[i] = 0;
  /*
    tprintf(
          "\nOld len %d; New len %d; New str \"%s\"; New map \"%s\"\n",
          old_len, i, new_str, new_map );
  */
  ASSERT_HOST (i == blob_it.length ());
  ASSERT_HOST (i == old_len + rej_len);
  word->reject_map = new_map;

  // Update word->best_choice if needed.
  if (strcmp(new_str, word->best_choice->unichar_string().string()) != 0 ||
      strcmp(new_lengths, word->best_choice->unichar_lengths().string()) != 0) {
    WERD_CHOICE *new_choice =
      new WERD_CHOICE(new_str, new_lengths,
                      word->best_choice->rating(),
                      word->best_choice->certainty(),
                      word->best_choice->permuter(),
                      getDict().getUnicharset());
   new_choice->populate_unichars(getDict().getUnicharset());
   delete word->best_choice;
   word->best_choice = new_choice;
  }
  new_len = word->best_choice->length();
  ASSERT_HOST (word->reject_map.length () == new_len);
  ASSERT_HOST (word->outword->blob_list ()->length () == new_len);

}
}  // namespace tesseract
