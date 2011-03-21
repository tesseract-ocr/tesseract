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

#include "mfcpch.h"

#include <string.h>
#include <math.h>
#ifdef __UNIX__
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#endif
#include <ctype.h>
#include "ocrclass.h"
#include "werdit.h"
#include "drawfx.h"
#include "tfacep.h"
#include "tessbox.h"
#include "tessvars.h"
#include "pgedit.h"
#include "reject.h"
#include "fixspace.h"
#include "docqual.h"
#include "control.h"
#include "secname.h"
#include "output.h"
#include "callcpp.h"
#include "notdll.h"
#include "globals.h"
#include "sorthelper.h"
#include "tesseractclass.h"

// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif

#define MIN_FONT_ROW_COUNT  8
#define MAX_XHEIGHT_DIFF  3

const char* const kBackUpConfigFile = "tempconfigdata.config";
// Multiple of x-height to make a repeated word have spaces in it.
const double kRepcharGapThreshold = 0.5;


/**
 * recog_pseudo_word
 *
 * Make a word from the selected blobs and run Tess on them.
 *
 * @param page_res recognise blobs
 * @param selection_box within this box
 */
namespace tesseract {
void Tesseract::recog_pseudo_word(PAGE_RES* page_res,
                                  TBOX &selection_box) {
  WERD *word;
  ROW *pseudo_row;               // row of word
  BLOCK *pseudo_block;           // block of word

  word = make_pseudo_word(page_res, selection_box,
                          pseudo_block, pseudo_row);
  if (word != NULL) {
    WERD_RES word_res(word);
    recog_interactive(pseudo_block, pseudo_row, &word_res);
    delete word;
  }
}


/**
 * recog_interactive
 *
 * Recognize a single word in interactive mode.
 *
 * @param block block
 * @param row row of word
 * @param word word to recognise
 */
BOOL8 Tesseract::recog_interactive(BLOCK* block, ROW* row, WERD_RES* word_res) {
  inT16 char_qual;
  inT16 good_char_qual;

  classify_word_pass2(word_res, block, row);
  if (tessedit_debug_quality_metrics) {
    word_char_quality(word_res, row, &char_qual, &good_char_qual);
    tprintf
      ("\n%d chars;  word_blob_quality: %d;  outline_errs: %d; char_quality: %d; good_char_quality: %d\n",
      word_res->reject_map.length(), word_blob_quality(word_res, row),
      word_outline_errs(word_res), char_qual, good_char_qual);
  }
  return TRUE;
}

// Helper function to check for a target word and handle it appropriately.
// Inspired by Jetsoft's requirement to process only single words on pass2
// and beyond.
// If word_config is not null:
//   If the word_box and target_word_box overlap, read the word_config file
//   else reset to previous config data.
//   return true.
// else
//   If the word_box and target_word_box overlap or pass <= 1, return true.
// Note that this function uses a fixed temporary file for storing the previous
// configs, so it is neither thread-safe, nor process-safe, but the assumption
// is that it will only be used for one debug window at a time.
bool Tesseract::ProcessTargetWord(const TBOX& word_box,
                                  const TBOX& target_word_box,
                                  const char* word_config,
                                  int pass) {
  if (word_config != NULL) {
    if (word_box.major_overlap(target_word_box)) {
      if (backup_config_file_ == NULL) {
        backup_config_file_ = kBackUpConfigFile;
        FILE* config_fp = fopen(backup_config_file_, "wb");
        ParamUtils::PrintParams(config_fp, params());
        fclose(config_fp);
        ParamUtils::ReadParamsFile(word_config, false, params());
      }
    } else {
      if (backup_config_file_ != NULL) {
        ParamUtils::ReadParamsFile(backup_config_file_, false, params());
        backup_config_file_ = NULL;
      }
    }
  } else if (pass > 1 && !word_box.major_overlap(target_word_box)) {
    return false;
  }
  return true;
}

/**
 * recog_all_words()
 *
 * Walk the page_res, recognizing all the words.
 * If monitor is not null, it is used as a progress monitor/timeout/cancel.
 * If dopasses is 0, all recognition passes are run,
 * 1 just pass 1, 2 passes2 and higher.
 * If target_word_box is not null, special things are done to words that
 * overlap the target_word_box:
 * if word_config is not null, the word config file is read for just the
 * target word(s), otherwise, on pass 2 and beyond ONLY the target words
 * are processed (Jetsoft modification.)
 *
 * @param page_res page structure
 * @param monitor progress monitor
 * @param target_word_box specifies just to extract a rectangle
 * @param dopasses 0 - all, 1 just pass 1, 2 passes 2 and higher
 */

void Tesseract::recog_all_words(PAGE_RES* page_res,
                                ETEXT_DESC* monitor,
                                const TBOX* target_word_box,
                                const char* word_config,
                                int dopasses) {
  // TODO(rays): Normalize the "classify word" interface.  For instance:
  //   (1) word.denorm gets set in word->SetupForRecognition() but does
  //       not get invoked for cube alone.  Maybe it should?
  //   (2) run_cube() checks whether word->best_choice is NULL, and if
  //       so determines that "neither cube nor tess have an answer."
  //       However, if tess gets run at all, the first thing it does is
  //       call word->SetupForRecognition which inserts a poorly scoring
  //       best_answer.  So what is the way that an engine (tess or cube)
  //       says "I don't have an answer": an empty list or a single
  //       poorly scoring best_answer?

                                 // reset page iterator
  // If we only intend to run cube - run it and return.
  if (tessedit_ocr_engine_mode == OEM_CUBE_ONLY) {
    PrepareForCubeOCR();
    mutable_splitter()->Clear();
    run_cube(page_res);
    return;
  }
  // Return if we do not want to run Tesseract.
  if (tessedit_ocr_engine_mode != OEM_TESSERACT_ONLY &&
      tessedit_ocr_engine_mode != OEM_TESSERACT_CUBE_COMBINED) return;

  PAGE_RES_IT page_res_it;
  inT16 chars_in_word;
  inT16 rejects_in_word;
  inT16 blob_quality = 0;
  inT16 outline_errs = 0;
  inT16 all_char_quality;
  inT16 accepted_all_char_quality;
  inT32 word_index;              // current word
  int i;

  if (tessedit_minimal_rej_pass1) {
    tessedit_test_adaption.set_value (TRUE);
    tessedit_minimal_rejection.set_value (TRUE);
  }

  if (dopasses==0 || dopasses==1) {
    page_res_it.page_res=page_res;
    page_res_it.restart_page();

    // ****************** Pass 1 *******************

    // Clear adaptive classifier at the beginning of the page if it is full.
    // This is done only at the beginning of the page to ensure that the
    // classifier is not reset at an arbitraty point while processing the page,
    // which would cripple Passes 2+ if the reset happens towards the end of
    // Pass 1 on a page with very difficul text.
    // TODO(daria): preemptively clear the classifier if it is almost full.
    if (AdaptiveClassifierIsFull()) ResetAdaptiveClassifier();

    stats_.word_count = 0;
    if (monitor != NULL) {
      monitor->ocr_alive = TRUE;
      while (page_res_it.word() != NULL) {
        stats_.word_count++;
        page_res_it.forward();
      }
      page_res_it.restart_page();
    } else {
      stats_.word_count = 1;
    }

    word_index = 0;

    stats_.dict_words = 0;
    stats_.doc_blob_quality = 0;
    stats_.doc_outline_errs = 0;
    stats_.doc_char_quality = 0;
    stats_.good_char_count = 0;
    stats_.doc_good_char_quality = 0;

    while (page_res_it.word() != NULL) {
      set_global_loc_code(LOC_PASS1);
      word_index++;
      if (monitor != NULL) {
        monitor->ocr_alive = TRUE;
        monitor->progress = 30 + 50 * word_index / stats_.word_count;
        if (monitor->deadline_exceeded() ||
            (monitor->cancel != NULL && (*monitor->cancel)(monitor->cancel_this,
                                                           stats_.dict_words)))
          return;
      }
      if (target_word_box &&
          !ProcessTargetWord(page_res_it.word()->word->bounding_box(),
                             *target_word_box, word_config, 1)) {
        page_res_it.forward();
        continue;
      }
      classify_word_pass1(page_res_it.word(), page_res_it.row()->row,
                          page_res_it.block()->block);
      if (page_res_it.word()->word->flag(W_REP_CHAR)) {
        fix_rep_char(&page_res_it);
        page_res_it.forward();
        continue;
      }
      if (tessedit_dump_choices) {
        word_dumper(NULL, page_res_it.row()->row, page_res_it.word());
        tprintf("Pass1: %s [%s]\n",
                page_res_it.word()->best_choice->unichar_string().string(),
                page_res_it.word()->best_choice->
                  debug_string(unicharset).string());
      }

      // tessedit_test_adaption enables testing of the accuracy of the
      // input to the adaptive classifier.
      if (tessedit_test_adaption && !tessedit_minimal_rejection) {
        if (!word_adaptable (page_res_it.word(),
          tessedit_test_adaption_mode)) {
          page_res_it.word()->reject_map.rej_word_tess_failure();
          // FAKE PERM REJ
        } else {
          // Override rejection mechanisms for this word.
          UNICHAR_ID space = unicharset.unichar_to_id(" ");
          for (i = 0; i < page_res_it.word()->best_choice->length(); i++) {
            if ((page_res_it.word()->best_choice->unichar_id(i) != space) &&
                page_res_it.word()->reject_map[i].rejected())
              page_res_it.word()->reject_map[i].setrej_minimal_rej_accept();
          }
        }
      }

      // Count dict words.
      if (page_res_it.word()->best_choice->permuter() == USER_DAWG_PERM)
        ++(stats_.dict_words);
      page_res_it.forward();
    }
  }

  if (dopasses == 1) return;

  // ****************** Pass 2 *******************
  page_res_it.restart_page();
  word_index = 0;
  while (!tessedit_test_adaption && page_res_it.word() != NULL) {
    set_global_loc_code(LOC_PASS2);
    word_index++;
    if (monitor != NULL) {
      monitor->ocr_alive = TRUE;
      monitor->progress = 80 + 10 * word_index / stats_.word_count;
      if (monitor->deadline_exceeded() ||
          (monitor->cancel != NULL && (*monitor->cancel)(monitor->cancel_this,
                                                         stats_.dict_words)))
        return;
    }

    // changed by jetsoft
    // specific to its needs to extract one word when need
    if (target_word_box &&
        !ProcessTargetWord(page_res_it.word()->word->bounding_box(),
                           *target_word_box, word_config, 2)) {
      page_res_it.forward();
      continue;
    }
    // end jetsoft

    classify_word_pass2(page_res_it.word(), page_res_it.block()->block,
                        page_res_it.row()->row);
    if (page_res_it.word()->word->flag(W_REP_CHAR) &&
        !page_res_it.word()->done) {
      fix_rep_char(&page_res_it);
      page_res_it.forward();
      continue;
    }
    if (tessedit_dump_choices) {
      word_dumper(NULL, page_res_it.row()->row, page_res_it.word());
      tprintf("Pass2: %s [%s]\n",
              page_res_it.word()->best_choice->unichar_string().string(),
              page_res_it.word()->best_choice->
                debug_string(unicharset).string());
    }
    page_res_it.forward();
  }

  // ****************** Pass 3 *******************
  // Fix fuzzy spaces.
  set_global_loc_code(LOC_FUZZY_SPACE);

  if (!tessedit_test_adaption && tessedit_fix_fuzzy_spaces
      && !tessedit_word_for_word && !right_to_left())
    fix_fuzzy_spaces(monitor, stats_.word_count, page_res);

  // ****************** Pass 4 *******************
  // Gather statistics on rejects.
  page_res_it.restart_page();
  word_index = 0;
  while (!tessedit_test_adaption && page_res_it.word() != NULL) {
    set_global_loc_code(LOC_MM_ADAPT);
    word_index++;
    if (monitor != NULL) {
      monitor->ocr_alive = TRUE;
      monitor->progress = 95 + 5 * word_index / stats_.word_count;
    }
    check_debug_pt(page_res_it.word(), 70);

    // changed by jetsoft
    // specific to its needs to extract one word when need
    if (target_word_box &&
        !ProcessTargetWord(page_res_it.word()->word->bounding_box(),
                           *target_word_box, word_config, 4)) {
      page_res_it.forward();
      continue;
    }
    // end jetsoft

    page_res_it.rej_stat_word();
    chars_in_word = page_res_it.word()->reject_map.length();
    rejects_in_word = page_res_it.word()->reject_map.reject_count();

    blob_quality = word_blob_quality(page_res_it.word(),
      page_res_it.row()->row);
    stats_.doc_blob_quality += blob_quality;
    outline_errs = word_outline_errs(page_res_it.word());
    stats_.doc_outline_errs += outline_errs;
    word_char_quality(page_res_it.word(),
                      page_res_it.row()->row,
                      &all_char_quality, &accepted_all_char_quality);
    stats_.doc_char_quality += all_char_quality;
    uinT8 permuter_type = page_res_it.word()->best_choice->permuter();
    if ((permuter_type == SYSTEM_DAWG_PERM) ||
        (permuter_type == FREQ_DAWG_PERM) ||
        (permuter_type == USER_DAWG_PERM)) {
      stats_.good_char_count += chars_in_word - rejects_in_word;
      stats_.doc_good_char_quality += accepted_all_char_quality;
    }
    check_debug_pt(page_res_it.word(), 80);
    if (tessedit_reject_bad_qual_wds &&
        (blob_quality == 0) && (outline_errs >= chars_in_word))
      page_res_it.word()->reject_map.rej_word_bad_quality();
    check_debug_pt(page_res_it.word(), 90);
    page_res_it.forward();
  }

  // ****************** Pass 5 *******************
  // If cube is loaded and its combiner is present, run it.
  if (tessedit_ocr_engine_mode == OEM_TESSERACT_CUBE_COMBINED) {
    PrepareForCubeOCR();
    mutable_splitter()->Clear();
    run_cube(page_res);
  }

  if (tessedit_debug_quality_metrics) {
    tprintf
      ("QUALITY: num_chs= %d  num_rejs= %d %5.3f blob_qual= %d %5.3f"
       " outline_errs= %d %5.3f char_qual= %d %5.3f good_ch_qual= %d %5.3f\n",
      page_res->char_count, page_res->rej_count,
      page_res->rej_count / static_cast<float>(page_res->char_count),
      stats_.doc_blob_quality,
      stats_.doc_blob_quality / static_cast<float>(page_res->char_count),
      stats_.doc_outline_errs,
      stats_.doc_outline_errs / static_cast<float>(page_res->char_count),
      stats_.doc_char_quality,
      stats_.doc_char_quality / static_cast<float>(page_res->char_count),
      stats_.doc_good_char_quality,
      (stats_.good_char_count > 0) ?
      (stats_.doc_good_char_quality /
       static_cast<float>(stats_.good_char_count)) : 0.0);
  }
  BOOL8 good_quality_doc =
    ((page_res->rej_count / static_cast<float>(page_res->char_count)) <=
     quality_rej_pc) &&
    (stats_.doc_blob_quality / static_cast<float>(page_res->char_count) >=
     quality_blob_pc) &&
    (stats_.doc_outline_errs / static_cast<float>(page_res->char_count) <=
     quality_outline_pc) &&
    (stats_.doc_char_quality / static_cast<float>(page_res->char_count) >=
     quality_char_pc);

  // ****************** Pass 6 *******************
  // Do whole document or whole block rejection pass
  if (!tessedit_test_adaption) {
    set_global_loc_code(LOC_DOC_BLK_REJ);
    quality_based_rejection(page_res_it, good_quality_doc);
  }

  // ****************** Pass 7 *******************
  font_recognition_pass(page_res_it);

  // Write results pass.
  set_global_loc_code(LOC_WRITE_RESULTS);
  // This is now redundant, but retained commented so show how to obtain
  // bounding boxes and style information.

  // changed by jetsoft
  // needed for dll to output memory structure
  if ((dopasses == 0 || dopasses == 2) && (monitor || tessedit_write_unlv))
    output_pass(page_res_it, target_word_box);
  // end jetsoft
  PageSegMode pageseg_mode = static_cast<PageSegMode>(
      static_cast<int>(tessedit_pageseg_mode));
  textord_.CleanupSingleRowResult(pageseg_mode, page_res);

  if (monitor != NULL) {
    monitor->progress = 100;
  }
}


/**
 * classify_word_pass1
 *
 * Baseline normalize the word and pass it to Tess.
 */

void Tesseract::classify_word_pass1(WERD_RES *word,  // word to do
                                    ROW *row,
                                    BLOCK* block) {
  BLOB_CHOICE_LIST_CLIST *blob_choices = new BLOB_CHOICE_LIST_CLIST();
  BOOL8 adapt_ok;
  const char *rejmap;
  inT16 index;
  STRING mapstr = "";

  check_debug_pt(word, 0);
  if (word->SetupForRecognition(unicharset, classify_bln_numeric_mode,
                                row, block))
    tess_segment_pass1(word, blob_choices);
  if (!word->tess_failed) {
    /*
       The adaption step used to be here. It has been moved to after
       make_reject_map so that we know whether the word will be accepted in the
       first pass or not.   This move will PREVENT adaption to words containing
       double quotes because the word will not be identical to what tess thinks
       its best choice is. (See CurrentBestChoiceIs in
       stopper.cpp which is used by AdaptableWord in
       adaptmatch.cpp)
     */

    if (!word->word->flag(W_REP_CHAR)) {
      // TODO(daria) delete these hacks when replaced by more generic code.
      // Convert '' (double single) to " (single double).
      fix_quotes(word, blob_choices);
      if (tessedit_fix_hyphens)  // turn -- to -
        fix_hyphens(word, blob_choices);

      word->tess_accepted = tess_acceptable_word(word->best_choice,
        word->raw_choice);

      word->tess_would_adapt = word->best_choice && word->raw_choice &&
          AdaptableWord(word->rebuild_word,
                        *word->best_choice,
                        *word->raw_choice);
                                 // Also sets word->done flag
      make_reject_map(word, blob_choices, row, 1);

      adapt_ok = word_adaptable(word, tessedit_tess_adaption_mode);

      if (adapt_ok || tessedit_tess_adapt_to_rejmap) {
        if (!tessedit_tess_adapt_to_rejmap) {
          rejmap = NULL;
        } else {
          ASSERT_HOST(word->reject_map.length() ==
                      word->best_choice->length());

          for (index = 0; index < word->reject_map.length(); index++) {
            if (adapt_ok || word->reject_map[index].accepted())
              mapstr += '1';
            else
              mapstr += '0';
          }
          rejmap = mapstr.string();
        }
        // Send word to adaptive classifier for training.
        word->BestChoiceToCorrectText(unicharset);
        set_word_fonts(word, blob_choices);
        LearnWord(NULL, rejmap, word);
      }

      if (tessedit_enable_doc_dict)
        tess_add_doc_word(word->best_choice);
    }
  }

  // Save best choices in the WERD_CHOICE if needed
  word->best_choice->set_blob_choices(blob_choices);
}

// Helper to switch between the original and new xht word or to discard
// the new xht word, according to accept_new_word.
static void SwitchWordOrDiscard(bool accept_new_word, WERD_RES* word,
                                WERD_RES* new_word) {
  if (accept_new_word) {
    // The new_word is deemed superior so put the final results in the real
    // word and destroy the old results.
    word->denorm = new_word->denorm;
    delete word->chopped_word;
    word->chopped_word = new_word->chopped_word;
    new_word->chopped_word = NULL;
    delete word->rebuild_word;
    word->rebuild_word = new_word->rebuild_word;
    new_word->rebuild_word = NULL;
    delete word->box_word;
    word->box_word = new_word->box_word;
    new_word->box_word = NULL;
    free_seam_list(word->seam_array);
    word->seam_array = new_word->seam_array;
    new_word->seam_array = NULL;
    word->best_state.move(&new_word->best_state);
    word->correct_text.move(&new_word->correct_text);
    delete word->best_choice;
    word->best_choice = new_word->best_choice;
    new_word->best_choice = NULL;
    delete word->raw_choice;
    word->raw_choice = new_word->raw_choice;
    new_word->raw_choice = NULL;
    word->reject_map = new_word->reject_map;
    word->CopySimpleFields(*new_word);
  } else {
    // The new_word is no better, so destroy it and cleanup.
    new_word->ClearResults();
  }
}

// Helper to report the result of the xheight fix.
void Tesseract::ReportXhtFixResult(bool accept_new_word, float new_x_ht,
                                   WERD_RES* word, WERD_RES* new_word) {
  tprintf("New XHT Match:%s = %s ",
          word->best_choice->unichar_string().string(),
          word->best_choice->debug_string(unicharset).string());
  word->reject_map.print(debug_fp);
  tprintf(" -> %s = %s ",
          new_word->best_choice->unichar_string().string(),
          new_word->best_choice->debug_string(unicharset).string());
  new_word->reject_map.print(debug_fp);
  tprintf(" %s->%s %s %s\n",
          word->guessed_x_ht ? "GUESS" : "CERT",
          new_word->guessed_x_ht ? "GUESS" : "CERT",
          new_x_ht > 0.1 ? "STILL DOUBT" : "OK",
          accept_new_word ? "ACCEPTED" : "");
}

// Run the x-height fix-up, based on min/max top/bottom information in
// unicharset.
// Returns true if the word was changed.
// See the comment in fixxht.cpp for a description of the overall process.
bool Tesseract::TrainedXheightFix(WERD_RES *word, BLOCK* block, ROW *row) {
  bool accept_new_x_ht = false;
  int original_misfits = CountMisfitTops(word);
  if (original_misfits == 0)
    return false;
  float new_x_ht = ComputeCompatibleXheight(word);
  if (new_x_ht > 0.0f) {
    WERD_RES new_x_ht_word(word->word);
    new_x_ht_word.x_height = new_x_ht;
    new_x_ht_word.caps_height = 0.0;
    match_word_pass2(&new_x_ht_word, row, block);
    if (!new_x_ht_word.tess_failed) {
      int new_misfits = CountMisfitTops(&new_x_ht_word);
      if (debug_x_ht_level >= 1) {
        tprintf("Old misfits=%d with x-height %f, new=%d with x-height %f\n",
                original_misfits, word->x_height,
                new_misfits, new_x_ht);
        tprintf("Old rating= %f, certainty=%f, new=%f, %f\n",
                word->best_choice->rating(), word->best_choice->certainty(),
                new_x_ht_word.best_choice->rating(),
                new_x_ht_word.best_choice->certainty());
      }
      // The misfits must improve and either the rating or certainty.
      accept_new_x_ht = new_misfits < original_misfits &&
                        (new_x_ht_word.best_choice->certainty() >
                            word->best_choice->certainty() ||
                         new_x_ht_word.best_choice->rating() <
                            word->best_choice->rating());
      if (debug_x_ht_level >= 1) {
        ReportXhtFixResult(accept_new_x_ht, new_x_ht, word, &new_x_ht_word);
      }
    }
    SwitchWordOrDiscard(accept_new_x_ht, word, &new_x_ht_word);
    if (accept_new_x_ht)
      return true;
  }
  return false;
}

/**
 * classify_word_pass2
 *
 * Control what to do with the word in pass 2
 */

void Tesseract::classify_word_pass2(WERD_RES *word, BLOCK* block, ROW *row) {
  bool done_this_pass = false;
  set_global_subloc_code(SUBLOC_NORM);
  check_debug_pt(word, 30);
  if (!word->done || tessedit_training_tess) {
    word->caps_height = 0.0;
    if (word->x_height == 0.0f)
      word->x_height = row->x_height();
    match_word_pass2(word, row, block);
    done_this_pass = TRUE;
    check_debug_pt(word, 40);
  }

  if (!word->tess_failed && !word->word->flag(W_REP_CHAR)) {
    bool accept_new_xht = false;
    if (unicharset.top_bottom_useful() && unicharset.script_has_xheight()) {
      // Use the tops and bottoms since they are available.
      accept_new_xht = TrainedXheightFix(word, block, row);
    }
    if (accept_new_xht)
      done_this_pass = true;
    // Test for small caps. Word capheight must be close to block xheight,
    // and word must contain no lower case letters, and at least one upper case.
    double small_cap_xheight = block->x_height() * kXHeightCapRatio;
    double small_cap_delta = (block->x_height() - small_cap_xheight) / 2.0;
    if (unicharset.script_has_xheight() &&
        small_cap_xheight - small_cap_delta <= word->x_height &&
        word->x_height <= small_cap_xheight + small_cap_delta) {
      // Scan for upper/lower.
      int num_upper = 0;
      int num_lower = 0;
      for (int i = 0; i < word->best_choice->length(); ++i) {
        if (unicharset.get_isupper(word->best_choice->unichar_id(i)))
          ++num_upper;
        else if (unicharset.get_islower(word->best_choice->unichar_id(i)))
          ++num_lower;
      }
      if (num_upper > 0 && num_lower == 0)
        word->small_caps = true;
    }
    word->SetScriptPositions(unicharset);

    set_global_subloc_code(SUBLOC_NORM);
  }
#ifndef GRAPHICS_DISABLED
  if (tessedit_draw_outwords) {
    if (fx_win == NULL)
      create_fx_win();
    clear_fx_win();
    word->rebuild_word->plot(fx_win);
    TBOX wbox = word->rebuild_word->bounding_box();
    fx_win->ZoomToRectangle(wbox.left(), wbox.top(),
                            wbox.right(), wbox.bottom());
    ScrollView::Update();
  }
#endif

  set_global_subloc_code(SUBLOC_NORM);
  check_debug_pt(word, 50);
}


/**
 * match_word_pass2
 *
 * Baseline normalize the word and pass it to Tess.
 */

void Tesseract::match_word_pass2(WERD_RES *word,  //word to do
                                 ROW *row,
                                 BLOCK* block) {
  BLOB_CHOICE_LIST_CLIST *blob_choices = new BLOB_CHOICE_LIST_CLIST();

  if (word->SetupForRecognition(unicharset, classify_bln_numeric_mode,
                                row, block))
    tess_segment_pass2(word, blob_choices);

  if (!word->tess_failed) {
    if (!word->word->flag (W_REP_CHAR)) {
      fix_quotes(word, blob_choices);
      if (tessedit_fix_hyphens)
        fix_hyphens(word, blob_choices);
      /* Dont trust fix_quotes! - though I think I've fixed the bug */
      if (word->best_choice->length() != word->box_word->length() ||
          word->best_choice->length() != blob_choices->length()) {
        tprintf("POST FIX_QUOTES FAIL String:\"%s\"; Strlen=%d;"
                " #Blobs=%d; #Choices=%d\n",
                word->best_choice->debug_string(unicharset).string(),
                word->best_choice->length(),
                word->box_word->length(), blob_choices->length());

      }
      word->tess_accepted = tess_acceptable_word(word->best_choice,
                                                 word->raw_choice);

      make_reject_map (word, blob_choices, row, 2);
    }
  }

  // Save best choices in the WERD_CHOICE if needed
  word->best_choice->set_blob_choices(blob_choices);

  assert (word->raw_choice != NULL);
}

// Helper to find the BLOB_CHOICE in the bc_list that matches the given
// unichar_id, or NULL if there is no match.
static BLOB_CHOICE* FindMatchingChoice(UNICHAR_ID char_id,
                                       BLOB_CHOICE_LIST* bc_list) {
  // Find the corresponding best BLOB_CHOICE.
  BLOB_CHOICE_IT choice_it(bc_list);
  for (choice_it.mark_cycle_pt(); !choice_it.cycled_list();
       choice_it.forward()) {
    BLOB_CHOICE* choice = choice_it.data();
    if (choice->unichar_id() == char_id) {
      return choice;
    }
  }
  return NULL;
}

// Helper to return the best rated BLOB_CHOICE in the whole word that matches
// the given char_id, or NULL if none can be found.
static BLOB_CHOICE* FindBestMatchingChoice(UNICHAR_ID char_id,
                                           WERD_RES* word_res) {
  // Find the corresponding best BLOB_CHOICE from any position in the word_res.
  BLOB_CHOICE* best_choice = NULL;
  BLOB_CHOICE_LIST_C_IT bc_it(word_res->best_choice->blob_choices());
  for (bc_it.mark_cycle_pt(); !bc_it.cycled_list(); bc_it.forward()) {
    BLOB_CHOICE* choice = FindMatchingChoice(char_id, bc_it.data());
    if (choice != NULL) {
      if (best_choice == NULL || choice->rating() < best_choice->rating())
        best_choice = choice;
    }
  }
  return best_choice;
}

// Helper to insert blob_choice in each location in the leader word if there is
// no matching BLOB_CHOICE there already, and correct any incorrect results
// in the best_choice.
static void CorrectRepcharChoices(BLOB_CHOICE* blob_choice,
                                  WERD_RES* word_res) {
  WERD_CHOICE* word = word_res->best_choice;
  BLOB_CHOICE_LIST_C_IT bc_it(word->blob_choices());
  for (bc_it.mark_cycle_pt(); !bc_it.cycled_list(); bc_it.forward()) {
    BLOB_CHOICE* choice = FindMatchingChoice(blob_choice->unichar_id(),
                                             bc_it.data());
    if (choice == NULL) {
      BLOB_CHOICE_IT choice_it(bc_it.data());
      choice_it.add_before_stay_put(new BLOB_CHOICE(*blob_choice));
    }
  }
  // Correct any incorrect results in word.
  for (int i = 0; i < word->length(); ++i) {
    if (word->unichar_id(i) != blob_choice->unichar_id())
      word->set_unichar_id(blob_choice->unichar_id(), i);
  }
}

/**
 * fix_rep_char()
 * The word is a repeated char. (Leader.) Find the repeated char character.
 * Create the appropriate single-word or multi-word sequence according to
 * the size of spaces in between blobs, and correct the classifications
 * where some of the characters disagree with the majority.
 */
void Tesseract::fix_rep_char(PAGE_RES_IT* page_res_it) {
  WERD_RES *word_res = page_res_it->word();
  const WERD_CHOICE &word = *(word_res->best_choice);

  // Find the frequency of each unique character in the word.
  UNICHAR_ID space = unicharset.unichar_to_id(" ");
  SortHelper<UNICHAR_ID> rep_ch(word.length());
  for (int i = 0; i < word.length(); ++i) {
    if (word.unichar_id(i) != space)
      rep_ch.Add(word.unichar_id(i), 1);
  }

  // Find the most frequent result.
  UNICHAR_ID maxch_id = INVALID_UNICHAR_ID; // most common char
  int max_count = rep_ch.MaxCount(&maxch_id);
  // Find the best exemplar of a classifier result for maxch_id.
  BLOB_CHOICE* best_choice = FindBestMatchingChoice(maxch_id, word_res);
  if (best_choice == NULL) {
    tprintf("Failed to find a choice for %s, occurring %d times\n",
            unicharset.debug_str(maxch_id).string(), max_count);
    return;
  }
  word_res->done = TRUE;

  // Measure the mean space.
  int total_gap = 0;
  int gap_count = 0;
  WERD* werd = word_res->word;
  C_BLOB_IT blob_it(werd->cblob_list());
  C_BLOB* prev_blob = blob_it.data();
  for (blob_it.forward(); !blob_it.at_first(); blob_it.forward()) {
    C_BLOB* blob = blob_it.data();
    int gap = blob->bounding_box().left();
    gap -= prev_blob->bounding_box().right();
    total_gap += gap;
    ++gap_count;
    prev_blob = blob;
  }
  if (total_gap > word_res->x_height * gap_count * kRepcharGapThreshold) {
    // Needs spaces between.
    ExplodeRepeatedWord(best_choice, page_res_it);
  } else {
    // Just correct existing classification.
    CorrectRepcharChoices(best_choice, word_res);
    word_res->best_choice->populate_unichars(unicharset);
    word_res->reject_map.initialise(word.length());
  }
}

// Explode the word at the given iterator location into individual words
// of a single given unichar_id defined by best_choice.
// The original word is deleted, and the replacements copy most of their
// fields from the original.
void Tesseract::ExplodeRepeatedWord(BLOB_CHOICE* best_choice,
                                    PAGE_RES_IT* page_res_it) {
  WERD_RES *word_res = page_res_it->word();
  ASSERT_HOST(best_choice != NULL);

  // Make a new word for each blob in the original.
  WERD* werd = word_res->word;
  C_BLOB_IT blob_it(werd->cblob_list());
  for (; !blob_it.empty(); blob_it.forward()) {
    bool first_blob = blob_it.at_first();
    bool last_blob = blob_it.at_last();
    WERD* blob_word = werd->ConstructFromSingleBlob(first_blob, last_blob,
                                                    blob_it.extract());
    WERD_RES* rep_word = page_res_it->InsertCloneWord(*word_res, blob_word);
    // Setup the single char WERD_RES
    rep_word->SetupForRecognition(unicharset, false, page_res_it->row()->row,
                                  page_res_it->block()->block);
    rep_word->CloneChoppedToRebuild();
    BLOB_CHOICE* blob_choice = new BLOB_CHOICE(*best_choice);
    rep_word->FakeClassifyWord(unicharset, 1, &blob_choice);
  }
  page_res_it->DeleteCurrentWord();
}

// TODO(tkielbus) Decide between keeping this behavior here or modifying the
// training data.

// Utility function for fix_quotes
// Return true if the next character in the string (given the UTF8 length in
// bytes) is a quote character.
static int is_simple_quote(const char* signed_str, int length) {
  const unsigned char* str =
    reinterpret_cast<const unsigned char*>(signed_str);
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

// Callback helper for fix_quotes returns a double quote if both
// arguments are quote, otherwise INVALID_UNICHAR_ID.
UNICHAR_ID Tesseract::BothQuotes(UNICHAR_ID id1, UNICHAR_ID id2) {
  const char *ch = unicharset.id_to_unichar(id1);
  const char *next_ch = unicharset.id_to_unichar(id2);
  if (is_simple_quote(ch, strlen(ch)) &&
      is_simple_quote(next_ch, strlen(next_ch)))
    return unicharset.unichar_to_id("\"");
  return INVALID_UNICHAR_ID;
}

/**
 * fix_quotes
 *
 * Change pairs of quotes to double quotes.
 */
void Tesseract::fix_quotes(WERD_RES* word_res,
                           BLOB_CHOICE_LIST_CLIST* blob_choices) {
  if (!unicharset.contains_unichar("\"") ||
      !unicharset.get_enabled(unicharset.unichar_to_id("\"")))
    return;  // Don't create it if it is disallowed.

  word_res->ConditionalBlobMerge(
      unicharset,
      NewPermanentTessCallback(this, &Tesseract::BothQuotes),
      NULL,
      blob_choices);
}

// Callback helper for fix_hyphens returns UNICHAR_ID of - if both
// arguments are hyphen, otherwise INVALID_UNICHAR_ID.
UNICHAR_ID Tesseract::BothHyphens(UNICHAR_ID id1, UNICHAR_ID id2) {
  const char *ch = unicharset.id_to_unichar(id1);
  const char *next_ch = unicharset.id_to_unichar(id2);
  if (strlen(ch) == 1 && strlen(next_ch) == 1 &&
      (*ch == '-' || *ch == '~') && (*next_ch == '-' || *next_ch == '~'))
    return unicharset.unichar_to_id("-");
  return INVALID_UNICHAR_ID;
}

// Callback helper for fix_hyphens returns true if box1 and box2 overlap
// (assuming both on the same textline, are in order and a chopped em dash.)
bool Tesseract::HyphenBoxesOverlap(const TBOX& box1, const TBOX& box2) {
  return box1.right() >= box2.left();
}

/**
 * fix_hyphens
 *
 * Change pairs of hyphens to a single hyphen if the bounding boxes touch
 * Typically a long dash which has been segmented.
 */
void Tesseract::fix_hyphens(WERD_RES *word_res,
                            BLOB_CHOICE_LIST_CLIST *blob_choices) {
  if (!unicharset.contains_unichar("-") ||
      !unicharset.get_enabled(unicharset.unichar_to_id("-")))
    return;  // Don't create it if it is disallowed.

  word_res->ConditionalBlobMerge(
      unicharset,
      NewPermanentTessCallback(this, &Tesseract::BothHyphens),
      NewPermanentTessCallback(this, &Tesseract::HyphenBoxesOverlap),
      blob_choices);
}
}  // namespace tesseract



namespace tesseract {

ACCEPTABLE_WERD_TYPE Tesseract::acceptable_word_string(const char *s,
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

BOOL8 Tesseract::check_debug_pt(WERD_RES *word, int location) {
  BOOL8 show_map_detail = FALSE;
  inT16 i;

  #ifndef SECURE_NAMES
  if (!test_pt)
    return FALSE;

  tessedit_rejection_debug.set_value (FALSE);
  debug_x_ht_level.set_value (0);

  if (word->word->bounding_box ().contains (FCOORD (test_pt_x, test_pt_y))) {
    if (location < 0)
      return TRUE;               // For breakpoint use
    tessedit_rejection_debug.set_value (TRUE);
    debug_x_ht_level.set_value (20);
    tprintf ("\n\nTESTWD::");
    switch (location) {
      case 0:
        tprintf ("classify_word_pass1 start\n");
        word->word->print();
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
    tprintf(" \"%s\" ",
            word->best_choice->unichar_string().string());
    word->reject_map.print (debug_fp);
    tprintf ("\n");
    if (show_map_detail) {
      tprintf ("\"%s\"\n", word->best_choice->unichar_string().string());
      for (i = 0; word->best_choice->unichar_string()[i] != '\0'; i++) {
        tprintf ("**** \"%c\" ****\n", word->best_choice->unichar_string()[i]);
        word->reject_map[i].full_print(debug_fp);
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

/**
 * find_modal_font
 *
 * Find the modal font and remove from the stats.
 */
static void find_modal_font(           //good chars in word
                     STATS *fonts,     //font stats
                     inT16 *font_out,   //output font
                     inT8 *font_count  //output count
                    ) {
  inT16 font;                     //font index
  inT32 count;                   //pile couat

  if (fonts->get_total () > 0) {
    font = (inT16) fonts->mode ();
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

/**
 * set_word_fonts
 *
 * Get the fonts for the word.
 */
void Tesseract::set_word_fonts(WERD_RES *word,
                               BLOB_CHOICE_LIST_CLIST *blob_choices) {
  if (blob_choices == NULL) return;
  // Don't try to set the word fonts for a cube word, as the configs
  // will be meaningless.
  if (word->chopped_word == NULL) return;

  inT32 index;                   // char id index
                                 // character iterator
  BLOB_CHOICE_LIST_C_IT char_it = blob_choices;
  BLOB_CHOICE_IT choice_it;      // choice iterator
  int fontinfo_size = get_fontinfo_table().size();
  int fontset_size = get_fontset_table().size();
  if (fontinfo_size == 0 || fontset_size == 0) return;
  STATS fonts(0, fontinfo_size);  // font counters

  word->italic = 0;
  word->bold = 0;
  if (!word->best_choice_fontinfo_ids.empty()) {
    word->best_choice_fontinfo_ids.clear();
  }
  // Compute the modal font for the word
  for (char_it.mark_cycle_pt(), index = 0;
       !char_it.cycled_list(); ++index, char_it.forward()) {
    UNICHAR_ID word_ch_id = word->best_choice->unichar_id(index);
    if (word_ch_id >= PreTrainedTemplates->NumClasses)
      return;  // This must be a cube word.
    choice_it.set_to_list(char_it.data());
    if (tessedit_debug_fonts) {
      tprintf("Examining fonts in %s\n", word->best_choice->debug_string(
          getDict().getUnicharset()).string());
    }
    for (choice_it.mark_cycle_pt(); !choice_it.cycled_list();
         choice_it.forward()) {
      UNICHAR_ID blob_ch_id = choice_it.data()->unichar_id();
      if (blob_ch_id == word_ch_id) {
        if (tessedit_debug_fonts) {
          tprintf("%s font %s (%d) font2 %s (%d)\n",
                  getDict().getUnicharset().id_to_unichar(blob_ch_id),
                  choice_it.data()->fontinfo_id() < 0 ? "unknown" :
                  fontinfo_table_.get(choice_it.data()->fontinfo_id()).name,
                  choice_it.data()->fontinfo_id(),
                  choice_it.data()->fontinfo_id2() < 0 ? "unknown" :
                  fontinfo_table_.get(choice_it.data()->fontinfo_id2()).name,
                  choice_it.data()->fontinfo_id2());
        }
        // 1st choice font gets 2 pts, 2nd choice 1 pt.
        if (choice_it.data()->fontinfo_id() >= 0) {
          fonts.add(choice_it.data()->fontinfo_id(), 2);
        }
        if (choice_it.data()->fontinfo_id2() >= 0) {
          fonts.add(choice_it.data()->fontinfo_id2(), 1);
        }
        break;
      }
    }
  }
  find_modal_font(&fonts, &word->fontinfo_id, &word->fontinfo_id_count);
  find_modal_font(&fonts, &word->fontinfo_id2, &word->fontinfo_id2_count);
  // All the blobs get the word's best choice font.
  for (int i = 0; i < word->best_choice->length(); ++i) {
    word->best_choice_fontinfo_ids.push_back(word->fontinfo_id);
  }
  if (word->fontinfo_id_count > 0) {
    FontInfo fi = fontinfo_table_.get(word->fontinfo_id);
    if (tessedit_debug_fonts) {
      if (word->fontinfo_id2_count > 0) {
        tprintf("Word modal font=%s, score=%d, 2nd choice %s/%d\n",
                fi.name, word->fontinfo_id_count,
                fontinfo_table_.get(word->fontinfo_id2).name,
                word->fontinfo_id2_count);
      } else {
        tprintf("Word modal font=%s, score=%d. No 2nd choice\n",
                fi.name, word->fontinfo_id_count);
      }
    }
    // 1st choices got 2 pts, so we need to halve the score for the mode.
    word->italic = (fi.is_italic() ? 1 : -1) * (word->fontinfo_id_count + 1) / 2;
    word->bold = (fi.is_bold() ? 1 : -1) * (word->fontinfo_id_count + 1) / 2;
  }
}


/**
 * font_recognition_pass
 *
 * Smooth the fonts for the document.
 */

void Tesseract::font_recognition_pass(  //good chars in word
                                      PAGE_RES_IT &page_res_it) {
  inT32 length;                  //of word
  inT32 count;                   //of a feature
  inT16 doc_font;                 //modal font
  inT8 doc_font_count;           //modal font
  WERD_RES *word;                //current word
  STATS doc_fonts (0, get_fontinfo_table().size() ?
               get_fontinfo_table().size() : 32);           // font counters

  page_res_it.restart_page();
  while (page_res_it.word() != NULL) {
    word = page_res_it.word();
    set_word_fonts(word, word->best_choice->blob_choices());
    if (!save_best_choices) {  // set_blob_choices() does a deep clear
      word->best_choice->set_blob_choices(NULL);
    }
    doc_fonts.add(word->fontinfo_id, word->fontinfo_id_count);
    doc_fonts.add(word->fontinfo_id2, word->fontinfo_id2_count);
    page_res_it.forward();
  }
  find_modal_font(&doc_fonts, &doc_font, &doc_font_count);
  if (doc_font_count == 0)
    return;
  FontInfo fi = fontinfo_table_.get(doc_font);

  page_res_it.restart_page ();
  while (page_res_it.word () != NULL) {
    word = page_res_it.word ();
    length = word->best_choice->length();

    // 1st choices got 2 pts, so we need to halve the score for the mode.
    count = (word->fontinfo_id_count + 1) / 2;
    if (!(count == length || (length > 3 && count >= length * 3 / 4))) {
      word->fontinfo_id = doc_font;
      // Counts only get 1 as it came from the doc.
      word->fontinfo_id_count = 1;
      word->italic = fi.is_italic() ? 1 : -1;
      word->bold = fi.is_bold() ? 1 : -1;
    }
    page_res_it.forward();
  }
}

}  // namespace tesseract
