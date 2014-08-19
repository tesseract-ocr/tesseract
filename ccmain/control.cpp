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
#include "tessbox.h"
#include "tessvars.h"
#include "pgedit.h"
#include "reject.h"
#include "fixspace.h"
#include "docqual.h"
#include "control.h"
#include "output.h"
#include "callcpp.h"
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
// Min believable x-height for any text when refitting as a fraction of
// original x-height
const double kMinRefitXHeightFraction = 0.5;


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
  PAGE_RES_IT* it = make_pseudo_word(page_res, selection_box);
  if (it != NULL) {
    recog_interactive(it);
    it->DeleteCurrentWord();
    delete it;
  }
}


/**
 * recog_interactive
 *
 * Recognize a single word in interactive mode.
 *
 * @param block block
 * @param row row of word
 * @param word_res word to recognise
 */
BOOL8 Tesseract::recog_interactive(PAGE_RES_IT* pr_it) {
  inT16 char_qual;
  inT16 good_char_qual;

  WordData word_data(*pr_it);
  SetupWordPassN(2, &word_data);
  classify_word_and_language(&Tesseract::classify_word_pass2, pr_it,
                             &word_data);
  if (tessedit_debug_quality_metrics) {
    WERD_RES* word_res = pr_it->word();
    word_char_quality(word_res, pr_it->row()->row, &char_qual, &good_char_qual);
    tprintf("\n%d chars;  word_blob_quality: %d;  outline_errs: %d; "
            "char_quality: %d; good_char_quality: %d\n",
            word_res->reject_map.length(),
            word_blob_quality(word_res, pr_it->row()->row),
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
//
// Since this function is used for debugging (and not to change OCR results)
// set only debug params from the word config file.
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
        ParamUtils::ReadParamsFile(word_config,
                                   SET_PARAM_CONSTRAINT_DEBUG_ONLY,
                                   params());
      }
    } else {
      if (backup_config_file_ != NULL) {
        ParamUtils::ReadParamsFile(backup_config_file_,
                                   SET_PARAM_CONSTRAINT_DEBUG_ONLY,
                                   params());
        backup_config_file_ = NULL;
      }
    }
  } else if (pass > 1 && !word_box.major_overlap(target_word_box)) {
    return false;
  }
  return true;
}

// If tesseract is to be run, sets the words up ready for it.
void Tesseract::SetupAllWordsPassN(int pass_n,
                                   const TBOX* target_word_box,
                                   const char* word_config,
                                   PAGE_RES* page_res,
                                   GenericVector<WordData>* words) {
  // Prepare all the words.
  PAGE_RES_IT page_res_it(page_res);
  for (page_res_it.restart_page(); page_res_it.word() != NULL;
       page_res_it.forward()) {
    if (target_word_box == NULL ||
        ProcessTargetWord(page_res_it.word()->word->bounding_box(),
                          *target_word_box, word_config, 1)) {
      words->push_back(WordData(page_res_it));
    }
  }
  // Setup all the words for recognition with polygonal approximation.
  for (int w = 0; w < words->size(); ++w) {
    SetupWordPassN(pass_n, &(*words)[w]);
    if (w > 0) (*words)[w].prev_word = &(*words)[w - 1];
  }
}

// Sets up the single word ready for whichever engine is to be run.
void Tesseract::SetupWordPassN(int pass_n, WordData* word) {
  if (pass_n == 1 || !word->word->done) {
    if (pass_n == 1) {
      word->word->SetupForRecognition(unicharset, this, BestPix(),
                                      tessedit_ocr_engine_mode, NULL,
                                      classify_bln_numeric_mode,
                                      textord_use_cjk_fp_model,
                                      poly_allow_detailed_fx,
                                      word->row, word->block);
    } else if (pass_n == 2) {
      // TODO(rays) Should we do this on pass1 too?
      word->word->caps_height = 0.0;
      if (word->word->x_height == 0.0f)
        word->word->x_height = word->row->x_height();
    }
    for (int s = 0; s <= sub_langs_.size(); ++s) {
      // The sub_langs_.size() entry is for the master language.
      Tesseract* lang_t = s < sub_langs_.size() ? sub_langs_[s] : this;
      WERD_RES* word_res = new WERD_RES;
      word_res->InitForRetryRecognition(*word->word);
      word->lang_words.push_back(word_res);
      // Cube doesn't get setup for pass2.
      if (pass_n == 1 || lang_t->tessedit_ocr_engine_mode != OEM_CUBE_ONLY) {
        word_res->SetupForRecognition(
              lang_t->unicharset, lang_t, BestPix(),
              lang_t->tessedit_ocr_engine_mode, NULL,
              lang_t->classify_bln_numeric_mode,
              lang_t->textord_use_cjk_fp_model,
              lang_t->poly_allow_detailed_fx, word->row, word->block);
      }
    }
  }
}

// Runs word recognition on all the words.
bool Tesseract::RecogAllWordsPassN(int pass_n, ETEXT_DESC* monitor,
                                   PAGE_RES_IT* pr_it,
                                   GenericVector<WordData>* words) {
  // TODO(rays) Before this loop can be parallelized (it would yield a massive
  // speed-up) all remaining member globals need to be converted to local/heap
  // (eg set_pass1 and set_pass2) and an intermediate adaption pass needs to be
  // added. The results will be significantly different with adaption on, and
  // deterioration will need investigation.
  pr_it->restart_page();
  for (int w = 0; w < words->size(); ++w) {
    WordData* word = &(*words)[w];
    if (w > 0) word->prev_word = &(*words)[w - 1];
    if (monitor != NULL) {
      monitor->ocr_alive = TRUE;
      if (pass_n == 1)
        monitor->progress = 30 + 50 * w / words->size();
      else
        monitor->progress = 80 + 10 * w / words->size();
      if (monitor->deadline_exceeded() ||
          (monitor->cancel != NULL && (*monitor->cancel)(monitor->cancel_this,
                                                         words->size()))) {
        // Timeout. Fake out the rest of the words.
        for (; w < words->size(); ++w) {
          (*words)[w].word->SetupFake(unicharset);
        }
        return false;
      }
    }
    if (word->word->tess_failed) {
      int s;
      for (s = 0; s < word->lang_words.size() &&
           word->lang_words[s]->tess_failed; ++s) {}
      // If all are failed, skip it. Image words are skipped by this test.
      if (s > word->lang_words.size()) continue;
    }
    // Sync pr_it with the wth WordData.
    while (pr_it->word() != NULL && pr_it->word() != word->word)
      pr_it->forward();
    ASSERT_HOST(pr_it->word() != NULL);
    WordRecognizer recognizer = pass_n == 1 ? &Tesseract::classify_word_pass1
                                            : &Tesseract::classify_word_pass2;
    classify_word_and_language(recognizer, pr_it, word);
    if (tessedit_dump_choices) {
      tprintf("Pass%d: %s [%s]\n", pass_n,
              word->word->best_choice->unichar_string().string(),
              word->word->best_choice->debug_string().string());
    }
    pr_it->forward();
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
 * Returns false if we cancelled prematurely.
 *
 * @param page_res page structure
 * @param monitor progress monitor
 * @param word_config word_config file
 * @param target_word_box specifies just to extract a rectangle
 * @param dopasses 0 - all, 1 just pass 1, 2 passes 2 and higher
 */

bool Tesseract::recog_all_words(PAGE_RES* page_res,
                                ETEXT_DESC* monitor,
                                const TBOX* target_word_box,
                                const char* word_config,
                                int dopasses) {
  PAGE_RES_IT page_res_it(page_res);

  if (tessedit_minimal_rej_pass1) {
    tessedit_test_adaption.set_value (TRUE);
    tessedit_minimal_rejection.set_value (TRUE);
  }

  if (dopasses==0 || dopasses==1) {
    page_res_it.restart_page();
    // ****************** Pass 1 *******************

    // Clear adaptive classifier at the beginning of the page if it is full.
    // This is done only at the beginning of the page to ensure that the
    // classifier is not reset at an arbitrary point while processing the page,
    // which would cripple Passes 2+ if the reset happens towards the end of
    // Pass 1 on a page with very difficult text.
    // TODO(daria): preemptively clear the classifier if it is almost full.
    if (AdaptiveClassifierIsFull()) ResetAdaptiveClassifierInternal();
    // Now check the sub-langs as well.
    for (int i = 0; i < sub_langs_.size(); ++i) {
      if (sub_langs_[i]->AdaptiveClassifierIsFull())
        sub_langs_[i]->ResetAdaptiveClassifierInternal();
    }
    // Set up all words ready for recognition, so that if parallelism is on
    // all the input and output classes are ready to run the classifier.
    GenericVector<WordData> words;
    SetupAllWordsPassN(1, target_word_box, word_config, page_res, &words);
    if (tessedit_parallelize) {
      PrerecAllWordsPar(words);
    }

    stats_.word_count = words.size();

    stats_.dict_words = 0;
    stats_.doc_blob_quality = 0;
    stats_.doc_outline_errs = 0;
    stats_.doc_char_quality = 0;
    stats_.good_char_count = 0;
    stats_.doc_good_char_quality = 0;

    most_recently_used_ = this;
    // Run pass 1 word recognition.
    if (!RecogAllWordsPassN(1, monitor, &page_res_it, &words)) return false;
    // Pass 1 post-processing.
    for (page_res_it.restart_page(); page_res_it.word() != NULL;
         page_res_it.forward()) {
      if (page_res_it.word()->word->flag(W_REP_CHAR)) {
        fix_rep_char(&page_res_it);
        continue;
      }

      // Count dict words.
      if (page_res_it.word()->best_choice->permuter() == USER_DAWG_PERM)
        ++(stats_.dict_words);

      // Update misadaption log (we only need to do it on pass 1, since
      // adaption only happens on this pass).
      if (page_res_it.word()->blamer_bundle != NULL &&
          page_res_it.word()->blamer_bundle->misadaption_debug().length() > 0) {
        page_res->misadaption_log.push_back(
            page_res_it.word()->blamer_bundle->misadaption_debug());
      }
    }
  }

  if (dopasses == 1) return true;

  // ****************** Pass 2 *******************
  if (tessedit_tess_adaption_mode != 0x0 && !tessedit_test_adaption &&
      tessedit_ocr_engine_mode != OEM_CUBE_ONLY ) {
    page_res_it.restart_page();
    GenericVector<WordData> words;
    SetupAllWordsPassN(2, target_word_box, word_config, page_res, &words);
    if (tessedit_parallelize) {
      PrerecAllWordsPar(words);
    }
    most_recently_used_ = this;
    // Run pass 2 word recognition.
    if (!RecogAllWordsPassN(2, monitor, &page_res_it, &words)) return false;
  }

  // The next passes can only be run if tesseract has been used, as cube
  // doesn't set all the necessary outputs in WERD_RES.
  if (tessedit_ocr_engine_mode == OEM_TESSERACT_ONLY ||
      tessedit_ocr_engine_mode == OEM_TESSERACT_CUBE_COMBINED) {
    // ****************** Pass 3 *******************
    // Fix fuzzy spaces.
    set_global_loc_code(LOC_FUZZY_SPACE);

    if (!tessedit_test_adaption && tessedit_fix_fuzzy_spaces
        && !tessedit_word_for_word && !right_to_left())
      fix_fuzzy_spaces(monitor, stats_.word_count, page_res);

    // ****************** Pass 4 *******************
    if (tessedit_enable_dict_correction) dictionary_correction_pass(page_res);
    if (tessedit_enable_bigram_correction) bigram_correction_pass(page_res);

    // ****************** Pass 5,6 *******************
    rejection_passes(page_res, monitor, target_word_box, word_config);

    // ****************** Pass 7 *******************
    // Cube combiner.
    // If cube is loaded and its combiner is present, run it.
    if (tessedit_ocr_engine_mode == OEM_TESSERACT_CUBE_COMBINED) {
      run_cube_combiner(page_res);
    }

    // ****************** Pass 8 *******************
    font_recognition_pass(page_res);

    // ****************** Pass 9 *******************
    // Check the correctness of the final results.
    blamer_pass(page_res);
    script_pos_pass(page_res);
  }

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

  // Remove empty words, as these mess up the result iterators.
  for (page_res_it.restart_page(); page_res_it.word() != NULL;
       page_res_it.forward()) {
    WERD_RES* word = page_res_it.word();
    if (word->best_choice == NULL || word->best_choice->length() == 0)
      page_res_it.DeleteCurrentWord();
  }

  if (monitor != NULL) {
    monitor->progress = 100;
  }
  return true;
}

void Tesseract::bigram_correction_pass(PAGE_RES *page_res) {
  PAGE_RES_IT word_it(page_res);

  WERD_RES *w_prev = NULL;
  WERD_RES *w = word_it.word();
  while (1) {
    w_prev = w;
    while (word_it.forward() != NULL &&
           (!word_it.word() || word_it.word()->part_of_combo)) {
      // advance word_it, skipping over parts of combos
    }
    if (!word_it.word()) break;
    w = word_it.word();
    if (!w || !w_prev || w->uch_set != w_prev->uch_set) {
      continue;
    }
    if (w_prev->word->flag(W_REP_CHAR) || w->word->flag(W_REP_CHAR)) {
      if (tessedit_bigram_debug) {
        tprintf("Skipping because one of the words is W_REP_CHAR\n");
      }
      continue;
    }
    // Two words sharing the same language model, excellent!
    GenericVector<WERD_CHOICE *> overrides_word1;
    GenericVector<WERD_CHOICE *> overrides_word2;

    STRING orig_w1_str = w_prev->best_choice->unichar_string();
    STRING orig_w2_str = w->best_choice->unichar_string();
    WERD_CHOICE prev_best(w->uch_set);
    {
      int w1start, w1end;
      w_prev->best_choice->GetNonSuperscriptSpan(&w1start, &w1end);
      prev_best = w_prev->best_choice->shallow_copy(w1start, w1end);
    }
    WERD_CHOICE this_best(w->uch_set);
    {
      int w2start, w2end;
      w->best_choice->GetNonSuperscriptSpan(&w2start, &w2end);
      this_best = w->best_choice->shallow_copy(w2start, w2end);
    }

    if (w->tesseract->getDict().valid_bigram(prev_best, this_best)) {
      if (tessedit_bigram_debug) {
        tprintf("Top choice \"%s %s\" verified by bigram model.\n",
                orig_w1_str.string(), orig_w2_str.string());
      }
      continue;
    }
    if (tessedit_bigram_debug > 2) {
      tprintf("Examining alt choices for \"%s %s\".\n",
              orig_w1_str.string(), orig_w2_str.string());
    }
    if (tessedit_bigram_debug > 1) {
      if (!w_prev->best_choices.singleton()) {
        w_prev->PrintBestChoices();
      }
      if (!w->best_choices.singleton()) {
        w->PrintBestChoices();
      }
    }
    float best_rating = 0.0;
    int best_idx = 0;
    WERD_CHOICE_IT prev_it(&w_prev->best_choices);
    for (prev_it.mark_cycle_pt(); !prev_it.cycled_list(); prev_it.forward()) {
      WERD_CHOICE *p1 = prev_it.data();
      WERD_CHOICE strip1(w->uch_set);
      {
        int p1start, p1end;
        p1->GetNonSuperscriptSpan(&p1start, &p1end);
        strip1 = p1->shallow_copy(p1start, p1end);
      }
      WERD_CHOICE_IT w_it(&w->best_choices);
      for (w_it.mark_cycle_pt(); !w_it.cycled_list(); w_it.forward()) {
        WERD_CHOICE *p2 = w_it.data();
        WERD_CHOICE strip2(w->uch_set);
        {
          int p2start, p2end;
          p2->GetNonSuperscriptSpan(&p2start, &p2end);
          strip2 = p2->shallow_copy(p2start, p2end);
        }
        if (w->tesseract->getDict().valid_bigram(strip1, strip2)) {
          overrides_word1.push_back(p1);
          overrides_word2.push_back(p2);
          if (overrides_word1.size() == 1 ||
              p1->rating() + p2->rating() < best_rating) {
            best_rating = p1->rating() + p2->rating();
            best_idx = overrides_word1.size() - 1;
          }
        }
      }
    }
    if (overrides_word1.size() >= 1) {
      // Excellent, we have some bigram matches.
      if (EqualIgnoringCaseAndTerminalPunct(*w_prev->best_choice,
                                            *overrides_word1[best_idx]) &&
          EqualIgnoringCaseAndTerminalPunct(*w->best_choice,
                                            *overrides_word2[best_idx])) {
        if (tessedit_bigram_debug > 1) {
          tprintf("Top choice \"%s %s\" verified (sans case) by bigram "
                  "model.\n", orig_w1_str.string(), orig_w2_str.string());
        }
        continue;
      }
      STRING new_w1_str = overrides_word1[best_idx]->unichar_string();
      STRING new_w2_str = overrides_word2[best_idx]->unichar_string();
      if (new_w1_str != orig_w1_str) {
        w_prev->ReplaceBestChoice(overrides_word1[best_idx]);
      }
      if (new_w2_str != orig_w2_str) {
        w->ReplaceBestChoice(overrides_word2[best_idx]);
      }
      if (tessedit_bigram_debug > 0) {
        STRING choices_description;
        int num_bigram_choices
            = overrides_word1.size() * overrides_word2.size();
        if (num_bigram_choices == 1) {
          choices_description = "This was the unique bigram choice.";
        } else {
          if (tessedit_bigram_debug > 1) {
            STRING bigrams_list;
            const int kMaxChoicesToPrint = 20;
            for (int i = 0; i < overrides_word1.size() &&
                 i < kMaxChoicesToPrint; i++) {
              if (i > 0) { bigrams_list += ", "; }
              WERD_CHOICE *p1 = overrides_word1[i];
              WERD_CHOICE *p2 = overrides_word2[i];
              bigrams_list += p1->unichar_string() + " " + p2->unichar_string();
              if (i == kMaxChoicesToPrint) {
                bigrams_list += " ...";
              }
            }
            choices_description = "There were many choices: {";
            choices_description += bigrams_list;
            choices_description += "}";
          } else {
            choices_description.add_str_int("There were ", num_bigram_choices);
            choices_description += " compatible bigrams.";
          }
        }
        tprintf("Replaced \"%s %s\" with \"%s %s\" with bigram model. %s\n",
                orig_w1_str.string(), orig_w2_str.string(),
                new_w1_str.string(), new_w2_str.string(),
                choices_description.string());
      }
    }
  }
}

void Tesseract::rejection_passes(PAGE_RES* page_res,
                                 ETEXT_DESC* monitor,
                                 const TBOX* target_word_box,
                                 const char* word_config) {
  PAGE_RES_IT page_res_it(page_res);
  // ****************** Pass 5 *******************
  // Gather statistics on rejects.
  int word_index = 0;
  while (!tessedit_test_adaption && page_res_it.word() != NULL) {
    set_global_loc_code(LOC_MM_ADAPT);
    WERD_RES* word = page_res_it.word();
    word_index++;
    if (monitor != NULL) {
      monitor->ocr_alive = TRUE;
      monitor->progress = 95 + 5 * word_index / stats_.word_count;
    }
    if (word->rebuild_word == NULL) {
      // Word was not processed by tesseract.
      page_res_it.forward();
      continue;
    }
    check_debug_pt(word, 70);

    // changed by jetsoft
    // specific to its needs to extract one word when need
    if (target_word_box &&
        !ProcessTargetWord(word->word->bounding_box(),
                           *target_word_box, word_config, 4)) {
      page_res_it.forward();
      continue;
    }
    // end jetsoft

    page_res_it.rej_stat_word();
    int chars_in_word = word->reject_map.length();
    int rejects_in_word = word->reject_map.reject_count();

    int blob_quality = word_blob_quality(word, page_res_it.row()->row);
    stats_.doc_blob_quality += blob_quality;
    int outline_errs = word_outline_errs(word);
    stats_.doc_outline_errs += outline_errs;
    inT16 all_char_quality;
    inT16 accepted_all_char_quality;
    word_char_quality(word, page_res_it.row()->row,
                      &all_char_quality, &accepted_all_char_quality);
    stats_.doc_char_quality += all_char_quality;
    uinT8 permuter_type = word->best_choice->permuter();
    if ((permuter_type == SYSTEM_DAWG_PERM) ||
        (permuter_type == FREQ_DAWG_PERM) ||
        (permuter_type == USER_DAWG_PERM)) {
      stats_.good_char_count += chars_in_word - rejects_in_word;
      stats_.doc_good_char_quality += accepted_all_char_quality;
    }
    check_debug_pt(word, 80);
    if (tessedit_reject_bad_qual_wds &&
        (blob_quality == 0) && (outline_errs >= chars_in_word))
      word->reject_map.rej_word_bad_quality();
    check_debug_pt(word, 90);
    page_res_it.forward();
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
}

void Tesseract::blamer_pass(PAGE_RES* page_res) {
  if (!wordrec_run_blamer) return;
  PAGE_RES_IT page_res_it(page_res);
  for (page_res_it.restart_page(); page_res_it.word() != NULL;
      page_res_it.forward()) {
    WERD_RES *word = page_res_it.word();
    BlamerBundle::LastChanceBlame(wordrec_debug_blamer, word);
    page_res->blame_reasons[word->blamer_bundle->incorrect_result_reason()]++;
  }
  tprintf("Blame reasons:\n");
  for (int bl = 0; bl < IRR_NUM_REASONS; ++bl) {
    tprintf("%s %d\n", BlamerBundle::IncorrectReasonName(
        static_cast<IncorrectResultReason>(bl)),
        page_res->blame_reasons[bl]);
  }
  if (page_res->misadaption_log.length() > 0) {
    tprintf("Misadaption log:\n");
    for (int i = 0; i < page_res->misadaption_log.length(); ++i) {
      tprintf("%s\n", page_res->misadaption_log[i].string());
    }
  }
}

// Sets script positions and detects smallcaps on all output words.
void Tesseract::script_pos_pass(PAGE_RES* page_res) {
  PAGE_RES_IT page_res_it(page_res);
  for (page_res_it.restart_page(); page_res_it.word() != NULL;
      page_res_it.forward()) {
    WERD_RES* word = page_res_it.word();
     if (word->word->flag(W_REP_CHAR)) {
      page_res_it.forward();
      continue;
    }
    float x_height = page_res_it.block()->block->x_height();
    float word_x_height = word->x_height;
    if (word_x_height < word->best_choice->min_x_height() ||
        word_x_height > word->best_choice->max_x_height()) {
      word_x_height = (word->best_choice->min_x_height() +
          word->best_choice->max_x_height()) / 2.0f;
    }
    // Test for small caps. Word capheight must be close to block xheight,
    // and word must contain no lower case letters, and at least one upper case.
    double small_cap_xheight = x_height * kXHeightCapRatio;
    double small_cap_delta = (x_height - small_cap_xheight) / 2.0;
    if (word->uch_set->script_has_xheight() &&
        small_cap_xheight - small_cap_delta <= word_x_height &&
        word_x_height <= small_cap_xheight + small_cap_delta) {
      // Scan for upper/lower.
      int num_upper = 0;
      int num_lower = 0;
      for (int i = 0; i < word->best_choice->length(); ++i) {
        if (word->uch_set->get_isupper(word->best_choice->unichar_id(i)))
          ++num_upper;
        else if (word->uch_set->get_islower(word->best_choice->unichar_id(i)))
          ++num_lower;
      }
      if (num_upper > 0 && num_lower == 0)
        word->small_caps = true;
    }
    word->SetScriptPositions();
  }
}

// Factored helper considers the indexed word and updates all the pointed
// values.
static void EvaluateWord(const PointerVector<WERD_RES>& words, int index,
                         float* rating, float* certainty, bool* bad,
                         bool* valid_permuter, int* right, int* next_left) {
  *right = -MAX_INT32;
  *next_left = MAX_INT32;
  if (index < words.size()) {
    WERD_CHOICE* choice = words[index]->best_choice;
    if (choice == NULL) {
      *bad = true;
    } else {
      *rating += choice->rating();
      *certainty = MIN(*certainty, choice->certainty());
      if (!Dict::valid_word_permuter(choice->permuter(), false))
        *valid_permuter = false;
    }
    *right = words[index]->word->bounding_box().right();
    if (index + 1 < words.size())
      *next_left = words[index + 1]->word->bounding_box().left();
  } else {
    *valid_permuter = false;
    *bad = true;
  }
}

// Helper chooses the best combination of words, transferring good ones from
// new_words to best_words. To win, a new word must have (better rating and
// certainty) or (better permuter status and rating within rating ratio and
// certainty within certainty margin) than current best.
// All the new_words are consumed (moved to best_words or deleted.)
// The return value is the number of new_words used minus the number of
// best_words that remain in the output.
static int SelectBestWords(double rating_ratio,
                           double certainty_margin,
                           bool debug,
                           PointerVector<WERD_RES>* new_words,
                           PointerVector<WERD_RES>* best_words) {
  // Process the smallest groups of words that have an overlapping word
  // boundary at the end.
  GenericVector<WERD_RES*> out_words;
  // Index into each word vector (best, new).
  int b = 0, n = 0;
  int num_best = 0, num_new = 0;
  while (b < best_words->size() || n < new_words->size()) {
    // Start of the current run in each.
    int start_b = b, start_n = n;
    // Rating of the current run in each.
    float b_rating = 0.0f, n_rating = 0.0f;
    // Certainty of the current run in each.
    float b_certainty = 0.0f, n_certainty = 0.0f;
    // True if any word is missing its best choice.
    bool b_bad = false, n_bad = false;
    // True if all words have a valid permuter.
    bool b_valid_permuter = true, n_valid_permuter = true;

    while (b < best_words->size() || n < new_words->size()) {
      int b_right = -MAX_INT32;
      int next_b_left = MAX_INT32;
      EvaluateWord(*best_words, b, &b_rating, &b_certainty, &b_bad,
                   &b_valid_permuter, &b_right, &next_b_left);
      int n_right = -MAX_INT32;
      int next_n_left = MAX_INT32;
      EvaluateWord(*new_words, n, &n_rating, &n_certainty, &n_bad,
                   &n_valid_permuter, &n_right, &next_n_left);
      if (MAX(b_right, n_right) < MIN(next_b_left, next_n_left)) {
        // The word breaks overlap. [start_b,b] and [start_n, n] match.
        break;
      }
      // Keep searching for the matching word break.
      if ((b_right < n_right && b < best_words->size()) ||
          n == new_words->size())
        ++b;
      else
        ++n;
    }
    bool new_better = false;
    if (!n_bad && (b_bad || (n_certainty > b_certainty &&
                             n_rating < b_rating) ||
                            (!b_valid_permuter && n_valid_permuter &&
                             n_rating < b_rating * rating_ratio &&
                             n_certainty > b_certainty - certainty_margin))) {
      // New is better.
      for (int i = start_n; i <= n; ++i) {
        out_words.push_back((*new_words)[i]);
        (*new_words)[i] = NULL;
        ++num_new;
      }
      new_better = true;
    } else if (!b_bad) {
      // Current best is better.
      for (int i = start_b; i <= b; ++i) {
        out_words.push_back((*best_words)[i]);
        (*best_words)[i] = NULL;
        ++num_best;
      }
    }
    int end_b = b < best_words->size() ? b + 1 : b;
    int end_n = n < new_words->size() ? n + 1 : n;
    if (debug) {
      tprintf("%d new words %s than %d old words: r: %g v %g c: %g v %g"
              " valid dict: %d v %d\n",
              end_n - start_n, new_better ? "better" : "worse",
              end_b - start_b, n_rating, b_rating,
              n_certainty, b_certainty, n_valid_permuter, b_valid_permuter);
    }
    // Move on to the next group.
    b = end_b;
    n = end_n;
  }
  // Transfer from out_words to best_words.
  best_words->clear();
  for (int i = 0; i < out_words.size(); ++i)
    best_words->push_back(out_words[i]);
  return num_new - num_best;
}

// Helper to recognize the word using the given (language-specific) tesseract.
// Returns positive if this recognizer found more new best words than the
// number kept from best_words.
int Tesseract::RetryWithLanguage(const WordData& word_data,
                                 WordRecognizer recognizer,
                                 WERD_RES** in_word,
                                 PointerVector<WERD_RES>* best_words) {
  bool debug = classify_debug_level || cube_debug_level;
  if (debug) {
    tprintf("Trying word using lang %s, oem %d\n",
            lang.string(), static_cast<int>(tessedit_ocr_engine_mode));
  }
  // Run the recognizer on the word.
  PointerVector<WERD_RES> new_words;
  (this->*recognizer)(word_data, in_word, &new_words);
  if (new_words.empty()) {
    // Transfer input word to new_words, as the classifier must have put
    // the result back in the input.
    new_words.push_back(*in_word);
    *in_word = NULL;
  }
  if (debug) {
    for (int i = 0; i < new_words.size(); ++i)
      new_words[i]->DebugTopChoice("Lang result");
  }
  // Initial version is a bit of a hack based on better certainty and rating
  // (to reduce false positives from cube) or a dictionary vs non-dictionary
  // word.
  return SelectBestWords(classify_max_rating_ratio,
                         classify_max_certainty_margin,
                         debug, &new_words, best_words);
}

// Helper returns true if all the words are acceptable.
static bool WordsAcceptable(const PointerVector<WERD_RES>& words) {
  for (int w = 0; w < words.size(); ++w) {
    if (words[w]->tess_failed || !words[w]->tess_accepted) return false;
  }
  return true;
}

// Generic function for classifying a word. Can be used either for pass1 or
// pass2 according to the function passed to recognizer.
// word_data holds the word to be recognized, and its block and row, and
// pr_it points to the word as well, in case we are running LSTM and it wants
// to output multiple words.
// Recognizes in the current language, and if successful that is all.
// If recognition was not successful, tries all available languages until
// it gets a successful result or runs out of languages. Keeps the best result.
void Tesseract::classify_word_and_language(WordRecognizer recognizer,
                                           PAGE_RES_IT* pr_it,
                                           WordData* word_data) {
  // Best result so far.
  PointerVector<WERD_RES> best_words;
  // Points to the best result. May be word or in lang_words.
  WERD_RES* word = word_data->word;
  clock_t start_t = clock();
  if (classify_debug_level || cube_debug_level) {
    tprintf("%s word with lang %s at:",
            word->done ? "Already done" : "Processing",
            most_recently_used_->lang.string());
    word->word->bounding_box().print();
  }
  if (word->done) {
    // If done on pass1, leave it as-is.
    if (!word->tess_failed)
      most_recently_used_ = word->tesseract;
    return;
  }
  int sub = sub_langs_.size();
  if (most_recently_used_ != this) {
    // Get the index of the most_recently_used_.
    for (sub = 0; sub < sub_langs_.size() &&
         most_recently_used_ != sub_langs_[sub]; ++sub) {}
  }
  most_recently_used_->RetryWithLanguage(
      *word_data, recognizer, &word_data->lang_words[sub], &best_words);
  Tesseract* best_lang_tess = most_recently_used_;
  if (!WordsAcceptable(best_words)) {
    // Try all the other languages to see if they are any better.
    if (most_recently_used_ != this &&
        this->RetryWithLanguage(*word_data, recognizer,
                                &word_data->lang_words[sub_langs_.size()],
                                &best_words) > 0) {
      best_lang_tess = this;
    }
    for (int i = 0; !WordsAcceptable(best_words) && i < sub_langs_.size();
         ++i) {
      if (most_recently_used_ != sub_langs_[i] &&
          sub_langs_[i]->RetryWithLanguage(*word_data, recognizer,
                                           &word_data->lang_words[i],
                                           &best_words) > 0) {
        best_lang_tess = sub_langs_[i];
      }
    }
  }
  most_recently_used_ = best_lang_tess;
  if (!best_words.empty()) {
    if (best_words.size() == 1 && !best_words[0]->combination) {
      // Move the best single result to the main word.
      word_data->word->ConsumeWordResults(best_words[0]);
    } else {
      // Words came from LSTM, and must be moved to the PAGE_RES properly.
      word_data->word = best_words.back();
      pr_it->ReplaceCurrentWord(&best_words);
    }
    ASSERT_HOST(word_data->word->box_word != NULL);
  } else {
    tprintf("no best words!!\n");
  }
  clock_t ocr_t = clock();
  if (tessedit_timing_debug) {
    tprintf("%s (ocr took %.2f sec)\n",
            word->best_choice->unichar_string().string(),
            static_cast<double>(ocr_t-start_t)/CLOCKS_PER_SEC);
  }
}

/**
 * classify_word_pass1
 *
 * Baseline normalize the word and pass it to Tess.
 */

void Tesseract::classify_word_pass1(const WordData& word_data,
                                    WERD_RES** in_word,
                                    PointerVector<WERD_RES>* out_words) {
  ROW* row = word_data.row;
  BLOCK* block = word_data.block;
  prev_word_best_choice_ = word_data.prev_word != NULL
      ? word_data.prev_word->word->best_choice : NULL;
  // If we only intend to run cube - run it and return.
  if (tessedit_ocr_engine_mode == OEM_CUBE_ONLY) {
    cube_word_pass1(block, row, *in_word);
    return;
  }
  WERD_RES* word = *in_word;
  match_word_pass_n(1, word, row, block);
  if (!word->tess_failed && !word->word->flag(W_REP_CHAR)) {
    word->tess_would_adapt = AdaptableWord(word);
    bool adapt_ok = word_adaptable(word, tessedit_tess_adaption_mode);

    if (adapt_ok) {
      // Send word to adaptive classifier for training.
      word->BestChoiceToCorrectText();
      LearnWord(NULL, word);
      // Mark misadaptions if running blamer.
      if (word->blamer_bundle != NULL) {
        word->blamer_bundle->SetMisAdaptionDebug(word->best_choice,
                                                 wordrec_debug_blamer);
      }
    }

    if (tessedit_enable_doc_dict && !word->IsAmbiguous())
      tess_add_doc_word(word->best_choice);
  }
}

// Helper to report the result of the xheight fix.
void Tesseract::ReportXhtFixResult(bool accept_new_word, float new_x_ht,
                                   WERD_RES* word, WERD_RES* new_word) {
  tprintf("New XHT Match:%s = %s ",
          word->best_choice->unichar_string().string(),
          word->best_choice->debug_string().string());
  word->reject_map.print(debug_fp);
  tprintf(" -> %s = %s ",
          new_word->best_choice->unichar_string().string(),
          new_word->best_choice->debug_string().string());
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
  if (new_x_ht >= kMinRefitXHeightFraction * word->x_height) {
    WERD_RES new_x_ht_word(word->word);
    if (word->blamer_bundle != NULL) {
      new_x_ht_word.blamer_bundle = new BlamerBundle();
      new_x_ht_word.blamer_bundle->CopyTruth(*(word->blamer_bundle));
    }
    new_x_ht_word.x_height = new_x_ht;
    new_x_ht_word.caps_height = 0.0;
    new_x_ht_word.SetupForRecognition(
          unicharset, this, BestPix(), tessedit_ocr_engine_mode, NULL,
          classify_bln_numeric_mode, textord_use_cjk_fp_model,
          poly_allow_detailed_fx, row, block);
    match_word_pass_n(2, &new_x_ht_word, row, block);
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
    if (accept_new_x_ht) {
      word->ConsumeWordResults(&new_x_ht_word);
      return true;
    }
  }
  return false;
}

/**
 * classify_word_pass2
 *
 * Control what to do with the word in pass 2
 */

void Tesseract::classify_word_pass2(const WordData& word_data,
                                    WERD_RES** in_word,
                                    PointerVector<WERD_RES>* out_words) {
  // Return if we do not want to run Tesseract.
  if (tessedit_ocr_engine_mode != OEM_TESSERACT_ONLY &&
      tessedit_ocr_engine_mode != OEM_TESSERACT_CUBE_COMBINED &&
      word_data.word->best_choice != NULL)
    return;
  ROW* row = word_data.row;
  BLOCK* block = word_data.block;
  WERD_RES* word = *in_word;
  prev_word_best_choice_ = word_data.prev_word != NULL
      ? word_data.prev_word->word->best_choice : NULL;

  set_global_subloc_code(SUBLOC_NORM);
  check_debug_pt(word, 30);
  if (!word->done) {
    word->caps_height = 0.0;
    if (word->x_height == 0.0f)
      word->x_height = row->x_height();
    match_word_pass_n(2, word, row, block);
    check_debug_pt(word, 40);
  }

  SubAndSuperscriptFix(word);

  if (!word->tess_failed && !word->word->flag(W_REP_CHAR)) {
    if (unicharset.top_bottom_useful() && unicharset.script_has_xheight() &&
        block->classify_rotation().y() == 0.0f) {
      // Use the tops and bottoms since they are available.
      TrainedXheightFix(word, block, row);
    }

    set_global_subloc_code(SUBLOC_NORM);
  }
#ifndef GRAPHICS_DISABLED
  if (tessedit_display_outwords) {
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

void Tesseract::match_word_pass_n(int pass_n, WERD_RES *word,
                                  ROW *row, BLOCK* block) {
  if (word->tess_failed) return;
  tess_segment_pass_n(pass_n, word);

  if (!word->tess_failed) {
    if (!word->word->flag (W_REP_CHAR)) {
       word->fix_quotes();
      if (tessedit_fix_hyphens)
        word->fix_hyphens();
      /* Dont trust fix_quotes! - though I think I've fixed the bug */
      if (word->best_choice->length() != word->box_word->length()) {
        tprintf("POST FIX_QUOTES FAIL String:\"%s\"; Strlen=%d;"
                " #Blobs=%d\n",
                word->best_choice->debug_string().string(),
                word->best_choice->length(),
                word->box_word->length());

      }
      word->tess_accepted = tess_acceptable_word(word);

      // Also sets word->done flag
      make_reject_map(word, row, pass_n);
    }
  }
  set_word_fonts(word);

  ASSERT_HOST(word->raw_choice != NULL);
}

// Helper to return the best rated BLOB_CHOICE in the whole word that matches
// the given char_id, or NULL if none can be found.
static BLOB_CHOICE* FindBestMatchingChoice(UNICHAR_ID char_id,
                                           WERD_RES* word_res) {
  // Find the corresponding best BLOB_CHOICE from any position in the word_res.
  BLOB_CHOICE* best_choice = NULL;
  for (int i = 0; i < word_res->best_choice->length(); ++i) {
    BLOB_CHOICE* choice = FindMatchingChoice(char_id,
                                             word_res->GetBlobChoices(i));
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
  for (int i = 0; i < word_res->best_choice->length(); ++i) {
    BLOB_CHOICE* choice = FindMatchingChoice(blob_choice->unichar_id(),
                                             word_res->GetBlobChoices(i));
    if (choice == NULL) {
      BLOB_CHOICE_IT choice_it(word_res->GetBlobChoices(i));
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
  SortHelper<UNICHAR_ID> rep_ch(word.length());
  for (int i = 0; i < word.length(); ++i) {
    rep_ch.Add(word.unichar_id(i), 1);
  }

  // Find the most frequent result.
  UNICHAR_ID maxch_id = INVALID_UNICHAR_ID; // most common char
  int max_count = rep_ch.MaxCount(&maxch_id);
  // Find the best exemplar of a classifier result for maxch_id.
  BLOB_CHOICE* best_choice = FindBestMatchingChoice(maxch_id, word_res);
  if (best_choice == NULL) {
    tprintf("Failed to find a choice for %s, occurring %d times\n",
            word_res->uch_set->debug_str(maxch_id).string(), max_count);
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
  // Just correct existing classification.
  CorrectRepcharChoices(best_choice, word_res);
  word_res->reject_map.initialise(word.length());
}

ACCEPTABLE_WERD_TYPE Tesseract::acceptable_word_string(
    const UNICHARSET& char_set, const char *s, const char *lengths) {
  int i = 0;
  int offset = 0;
  int leading_punct_count;
  int upper_count = 0;
  int hyphen_pos = -1;
  ACCEPTABLE_WERD_TYPE word_type = AC_UNACCEPTABLE;

  if (strlen (lengths) > 20)
    return word_type;

  /* Single Leading punctuation char*/

  if (s[offset] != '\0' && STRING(chs_leading_punct).contains(s[offset]))
    offset += lengths[i++];
  leading_punct_count = i;

  /* Initial cap */
  while (s[offset] != '\0' && char_set.get_isupper(s + offset, lengths[i])) {
    offset += lengths[i++];
    upper_count++;
  }
  if (upper_count > 1) {
    word_type = AC_UPPER_CASE;
  } else {
    /* Lower case word, possibly with an initial cap */
    while (s[offset] != '\0' && char_set.get_islower(s + offset, lengths[i])) {
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
               char_set.get_islower(s + offset, lengths[i])) {
          offset += lengths[i++];
        }
        if (i < hyphen_pos + 3)
          goto not_a_word;
      }
    } else {
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
  if (lengths[i] == 1 && s[offset] != '\0' &&
      STRING(chs_trailing_punct1).contains(s[offset]))
    offset += lengths[i++];
  if (lengths[i] == 1 && s[offset] != '\0' && i > 0 &&
      s[offset - lengths[i - 1]] != s[offset] &&
      STRING(chs_trailing_punct2).contains (s[offset]))
    offset += lengths[i++];

  if (s[offset] != '\0')
    word_type = AC_UNACCEPTABLE;

  not_a_word:

  if (word_type == AC_UNACCEPTABLE) {
    /* Look for abbreviation string */
    i = 0;
    offset = 0;
    if (s[0] != '\0' && char_set.get_isupper(s, lengths[0])) {
      word_type = AC_UC_ABBREV;
      while (s[offset] != '\0' &&
             char_set.get_isupper(s + offset, lengths[i]) &&
             lengths[i + 1] == 1 && s[offset + lengths[i]] == '.') {
        offset += lengths[i++];
        offset += lengths[i++];
      }
    }
    else if (s[0] != '\0' && char_set.get_islower(s, lengths[0])) {
      word_type = AC_LC_ABBREV;
      while (s[offset] != '\0' &&
             char_set.get_islower(s + offset, lengths[i]) &&
             lengths[i + 1] == 1 && s[offset + lengths[i]] == '.') {
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
    if (word->best_choice != NULL) {
      tprintf(" \"%s\" ", word->best_choice->unichar_string().string());
      word->reject_map.print(debug_fp);
      tprintf("\n");
      if (show_map_detail) {
        tprintf("\"%s\"\n", word->best_choice->unichar_string().string());
        for (i = 0; word->best_choice->unichar_string()[i] != '\0'; i++) {
          tprintf("**** \"%c\" ****\n", word->best_choice->unichar_string()[i]);
          word->reject_map[i].full_print(debug_fp);
        }
      }
    } else {
      tprintf("null best choice\n");
    }
    tprintf ("Tess Accepted: %s\n", word->tess_accepted ? "TRUE" : "FALSE");
    tprintf ("Done flag: %s\n\n", word->done ? "TRUE" : "FALSE");
    return TRUE;
  } else {
    return FALSE;
  }
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
void Tesseract::set_word_fonts(WERD_RES *word) {
  // Don't try to set the word fonts for a cube word, as the configs
  // will be meaningless.
  if (word->chopped_word == NULL) return;
  ASSERT_HOST(word->best_choice != NULL);

  inT32 index;                   // char id index
                                 // character iterator
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
  for (index = 0; index < word->best_choice->length(); ++index) {
    UNICHAR_ID word_ch_id = word->best_choice->unichar_id(index);
    choice_it.set_to_list(word->GetBlobChoices(index));
    if (tessedit_debug_fonts) {
      tprintf("Examining fonts in %s\n",
              word->best_choice->debug_string().string());
    }
    for (choice_it.mark_cycle_pt(); !choice_it.cycled_list();
         choice_it.forward()) {
      UNICHAR_ID blob_ch_id = choice_it.data()->unichar_id();
      if (blob_ch_id == word_ch_id) {
        if (tessedit_debug_fonts) {
          tprintf("%s font %s (%d) font2 %s (%d)\n",
                  word->uch_set->id_to_unichar(blob_ch_id),
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
  inT16 font_id1, font_id2;
  find_modal_font(&fonts, &font_id1, &word->fontinfo_id_count);
  find_modal_font(&fonts, &font_id2, &word->fontinfo_id2_count);
  word->fontinfo = font_id1 >= 0 ? &fontinfo_table_.get(font_id1) : NULL;
  word->fontinfo2 = font_id2 >= 0 ? &fontinfo_table_.get(font_id2) : NULL;
  // All the blobs get the word's best choice font.
  for (int i = 0; i < word->best_choice->length(); ++i) {
    word->best_choice_fontinfo_ids.push_back(font_id1);
  }
  if (word->fontinfo_id_count > 0) {
    FontInfo fi = fontinfo_table_.get(font_id1);
    if (tessedit_debug_fonts) {
      if (word->fontinfo_id2_count > 0) {
        tprintf("Word modal font=%s, score=%d, 2nd choice %s/%d\n",
                fi.name, word->fontinfo_id_count,
                fontinfo_table_.get(font_id2).name,
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

void Tesseract::font_recognition_pass(PAGE_RES* page_res) {
  PAGE_RES_IT page_res_it(page_res);
  WERD_RES *word;                // current word
  STATS doc_fonts(0, font_table_size_);           // font counters

  // Gather font id statistics.
  for (page_res_it.restart_page(); page_res_it.word() != NULL;
       page_res_it.forward()) {
    word = page_res_it.word();
    if (word->fontinfo != NULL) {
      doc_fonts.add(word->fontinfo->universal_id, word->fontinfo_id_count);
    }
    if (word->fontinfo2 != NULL) {
      doc_fonts.add(word->fontinfo2->universal_id, word->fontinfo_id2_count);
    }
  }
  inT16 doc_font;                 // modal font
  inT8 doc_font_count;           // modal font
  find_modal_font(&doc_fonts, &doc_font, &doc_font_count);
  if (doc_font_count == 0)
    return;
  // Get the modal font pointer.
  const FontInfo* modal_font = NULL;
  for (page_res_it.restart_page(); page_res_it.word() != NULL;
       page_res_it.forward()) {
    word = page_res_it.word();
    if (word->fontinfo != NULL && word->fontinfo->universal_id == doc_font) {
      modal_font = word->fontinfo;
      break;
    }
    if (word->fontinfo2 != NULL && word->fontinfo2->universal_id == doc_font) {
      modal_font = word->fontinfo2;
      break;
    }
  }
  ASSERT_HOST(modal_font != NULL);

  // Assign modal font to weak words.
  for (page_res_it.restart_page(); page_res_it.word() != NULL;
       page_res_it.forward()) {
    word = page_res_it.word();
    int length = word->best_choice->length();

    // 1st choices got 2 pts, so we need to halve the score for the mode.
    int count = (word->fontinfo_id_count + 1) / 2;
    if (!(count == length || (length > 3 && count >= length * 3 / 4))) {
      word->fontinfo = modal_font;
      // Counts only get 1 as it came from the doc.
      word->fontinfo_id_count = 1;
      word->italic = modal_font->is_italic() ? 1 : -1;
      word->bold = modal_font->is_bold() ? 1 : -1;
    }
  }
}

// If a word has multiple alternates check if the best choice is in the
// dictionary. If not, replace it with an alternate that exists in the
// dictionary.
void Tesseract::dictionary_correction_pass(PAGE_RES *page_res) {
  PAGE_RES_IT word_it(page_res);
  for (WERD_RES* word = word_it.word(); word != NULL;
       word = word_it.forward()) {
    if (word->best_choices.singleton())
      continue;  // There are no alternates.

    WERD_CHOICE* best = word->best_choice;
    if (word->tesseract->getDict().valid_word(*best) != 0)
      continue;  // The best choice is in the dictionary.

    WERD_CHOICE_IT choice_it(&word->best_choices);
    for (choice_it.mark_cycle_pt(); !choice_it.cycled_list();
         choice_it.forward()) {
      WERD_CHOICE* alternate = choice_it.data();
      if (word->tesseract->getDict().valid_word(*alternate)) {
        // The alternate choice is in the dictionary.
        if (tessedit_bigram_debug) {
          tprintf("Dictionary correction replaces best choice '%s' with '%s'\n",
                  best->unichar_string().string(),
                  alternate->unichar_string().string());
        }
        // Replace the 'best' choice with a better choice.
        word->ReplaceBestChoice(alternate);
        break;
      }
    }
  }
}

}  // namespace tesseract
