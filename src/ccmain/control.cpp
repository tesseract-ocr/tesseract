/******************************************************************
 * File:        control.cpp  (Formerly control.c)
 * Description: Module-independent matcher controller.
 * Author:      Ray Smith
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

// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#  include "config_auto.h"
#endif

#include <cctype>
#include <cmath>
#include <cstdint> // for int16_t, int32_t
#include <cstdio>  // for fclose, fopen, FILE
#include <ctime>   // for clock
#include "control.h"
#ifndef DISABLED_LEGACY_ENGINE
#  include "docqual.h"
#  include "drawfx.h"
#  include "fixspace.h"
#endif
#include <tesseract/ocrclass.h>
#include "lstmrecognizer.h"
#include "output.h"
#include "pageres.h" // for WERD_RES, PAGE_RES_IT, PAGE_RES, BLO...
#ifndef DISABLED_LEGACY_ENGINE
#  include "reject.h"
#endif
#include "sorthelper.h"
#include "tesseractclass.h"
#include "tesserrstream.h"  // for tesserr
#include "tessvars.h"
#include "werdit.h"

const char *const kBackUpConfigFile = "tempconfigdata.config";
#ifndef DISABLED_LEGACY_ENGINE
// Min believable x-height for any text when refitting as a fraction of
// original x-height
const double kMinRefitXHeightFraction = 0.5;
#endif // ! DISABLED_LEGACY_ENGINE

namespace tesseract {

/**
 * Make a word from the selected blobs and run Tess on them.
 *
 * @param page_res recognise blobs
 * @param selection_box within this box
 */

void Tesseract::recog_pseudo_word(PAGE_RES *page_res, TBOX &selection_box) {
  PAGE_RES_IT *it = make_pseudo_word(page_res, selection_box);
  if (it != nullptr) {
    recog_interactive(it);
    it->DeleteCurrentWord();
    delete it;
  }
}

/**
 * Recognize a single word in interactive mode.
 *
 * @param pr_it the page results iterator
 */
bool Tesseract::recog_interactive(PAGE_RES_IT *pr_it) {
  WordData word_data(*pr_it);
  SetupWordPassN(2, &word_data);
  // LSTM doesn't run on pass2, but we want to run pass2 for tesseract.
  if (lstm_recognizer_ == nullptr) {
#ifndef DISABLED_LEGACY_ENGINE
    classify_word_and_language(2, pr_it, &word_data);
#endif // ndef DISABLED_LEGACY_ENGINE
  } else {
    classify_word_and_language(1, pr_it, &word_data);
  }
#ifndef DISABLED_LEGACY_ENGINE
  if (tessedit_debug_quality_metrics) {
    int16_t char_qual;
    int16_t good_char_qual;
    WERD_RES *word_res = pr_it->word();
    word_char_quality(word_res, &char_qual, &good_char_qual);
    tprintf(
        "\n%d chars;  word_blob_quality: %d;  outline_errs: %d; "
        "char_quality: %d; good_char_quality: %d\n",
        word_res->reject_map.length(), word_blob_quality(word_res), word_outline_errs(word_res),
        char_qual, good_char_qual);
  }
#endif // ndef DISABLED_LEGACY_ENGINE
  return true;
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
bool Tesseract::ProcessTargetWord(const TBOX &word_box, const TBOX &target_word_box,
                                  const char *word_config, int pass) {
  if (word_config != nullptr) {
    if (word_box.major_overlap(target_word_box)) {
      if (backup_config_file_ == nullptr) {
        backup_config_file_ = kBackUpConfigFile;
        FILE *config_fp = fopen(backup_config_file_, "wb");
        if (config_fp == nullptr) {
          tprintf("Error, failed to open file \"%s\"\n", backup_config_file_);
        } else {
          ParamUtils::PrintParams(config_fp, params(), false);
          fclose(config_fp);
        }
        ParamUtils::ReadParamsFile(word_config, SET_PARAM_CONSTRAINT_DEBUG_ONLY, params());
      }
    } else {
      if (backup_config_file_ != nullptr) {
        ParamUtils::ReadParamsFile(backup_config_file_, SET_PARAM_CONSTRAINT_DEBUG_ONLY, params());
        backup_config_file_ = nullptr;
      }
    }
  } else if (pass > 1 && !word_box.major_overlap(target_word_box)) {
    return false;
  }
  return true;
}

/** If tesseract is to be run, sets the words up ready for it. */
void Tesseract::SetupAllWordsPassN(int pass_n, const TBOX *target_word_box, const char *word_config,
                                   PAGE_RES *page_res, std::vector<WordData> *words) {
  // Prepare all the words.
  PAGE_RES_IT page_res_it(page_res);
  for (page_res_it.restart_page(); page_res_it.word() != nullptr; page_res_it.forward()) {
    if (target_word_box == nullptr || ProcessTargetWord(page_res_it.word()->word->bounding_box(),
                                                        *target_word_box, word_config, 1)) {
      words->push_back(WordData(page_res_it));
    }
  }
  // Setup all the words for recognition with polygonal approximation.
  for (unsigned w = 0; w < words->size(); ++w) {
    SetupWordPassN(pass_n, &(*words)[w]);
    if (w > 0) {
      (*words)[w].prev_word = &(*words)[w - 1];
    }
  }
}

// Sets up the single word ready for whichever engine is to be run.
void Tesseract::SetupWordPassN(int pass_n, WordData *word) {
  if (pass_n == 1 || !word->word->done) {
    if (pass_n == 1) {
      word->word->SetupForRecognition(unicharset, this, BestPix(), tessedit_ocr_engine_mode,
                                      nullptr, classify_bln_numeric_mode, textord_use_cjk_fp_model,
                                      poly_allow_detailed_fx, word->row, word->block);
    } else if (pass_n == 2) {
      // TODO(rays) Should we do this on pass1 too?
      word->word->caps_height = 0.0;
      if (word->word->x_height == 0.0f) {
        word->word->x_height = word->row->x_height();
      }
    }
    word->lang_words.truncate(0);
    for (unsigned s = 0; s <= sub_langs_.size(); ++s) {
      // The sub_langs_.size() entry is for the master language.
      Tesseract *lang_t = s < sub_langs_.size() ? sub_langs_[s] : this;
      auto *word_res = new WERD_RES;
      word_res->InitForRetryRecognition(*word->word);
      word->lang_words.push_back(word_res);
      // LSTM doesn't get setup for pass2.
      if (pass_n == 1 || lang_t->tessedit_ocr_engine_mode != OEM_LSTM_ONLY) {
        word_res->SetupForRecognition(
            lang_t->unicharset, lang_t, BestPix(), lang_t->tessedit_ocr_engine_mode, nullptr,
            lang_t->classify_bln_numeric_mode, lang_t->textord_use_cjk_fp_model,
            lang_t->poly_allow_detailed_fx, word->row, word->block);
      }
    }
  }
}

// Runs word recognition on all the words.
bool Tesseract::RecogAllWordsPassN(int pass_n, ETEXT_DESC *monitor, PAGE_RES_IT *pr_it,
                                   std::vector<WordData> *words) {
  // TODO(rays) Before this loop can be parallelized (it would yield a massive
  // speed-up) all remaining member globals need to be converted to local/heap
  // (eg set_pass1 and set_pass2) and an intermediate adaption pass needs to be
  // added. The results will be significantly different with adaption on, and
  // deterioration will need investigation.
  pr_it->restart_page();
  for (unsigned w = 0; w < words->size(); ++w) {
    WordData *word = &(*words)[w];
    if (w > 0) {
      word->prev_word = &(*words)[w - 1];
    }
    if (monitor != nullptr) {
      monitor->ocr_alive = true;
      if (pass_n == 1) {
        monitor->progress = 70 * w / words->size();
      } else {
        monitor->progress = 70 + 30 * w / words->size();
      }
      if (monitor->progress_callback2 != nullptr) {
        TBOX box = pr_it->word()->word->bounding_box();
        (*monitor->progress_callback2)(monitor, box.left(), box.right(), box.top(), box.bottom());
      }
      if (monitor->deadline_exceeded() ||
          (monitor->cancel != nullptr && (*monitor->cancel)(monitor->cancel_this, words->size()))) {
        // Timeout. Fake out the rest of the words.
        for (; w < words->size(); ++w) {
          (*words)[w].word->SetupFake(unicharset);
        }
        return false;
      }
    }
    if (word->word->tess_failed) {
      unsigned s;
      for (s = 0; s < word->lang_words.size() && word->lang_words[s]->tess_failed; ++s) {
      }
      // If all are failed, skip it. Image words are skipped by this test.
      if (s > word->lang_words.size()) {
        continue;
      }
    }
    // Sync pr_it with the WordData.
    while (pr_it->word() != nullptr && pr_it->word() != word->word) {
      pr_it->forward();
    }
    ASSERT_HOST(pr_it->word() != nullptr);
    bool make_next_word_fuzzy = false;
#ifndef DISABLED_LEGACY_ENGINE
    if (!AnyLSTMLang() && ReassignDiacritics(pass_n, pr_it, &make_next_word_fuzzy)) {
      // Needs to be setup again to see the new outlines in the chopped_word.
      SetupWordPassN(pass_n, word);
    }
#endif // ndef DISABLED_LEGACY_ENGINE

    classify_word_and_language(pass_n, pr_it, word);
    if (tessedit_dump_choices || debug_noise_removal) {
      tprintf("Pass%d: %s [%s]\n", pass_n, word->word->best_choice->unichar_string().c_str(),
              word->word->best_choice->debug_string().c_str());
    }
    pr_it->forward();
    if (make_next_word_fuzzy && pr_it->word() != nullptr) {
      pr_it->MakeCurrentWordFuzzy();
    }
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

bool Tesseract::recog_all_words(PAGE_RES *page_res, ETEXT_DESC *monitor,
                                const TBOX *target_word_box, const char *word_config,
                                int dopasses) {
  PAGE_RES_IT page_res_it(page_res);

  if (tessedit_minimal_rej_pass1) {
    tessedit_test_adaption.set_value(true);
    tessedit_minimal_rejection.set_value(true);
  }

  if (dopasses == 0 || dopasses == 1) {
    page_res_it.restart_page();
    // ****************** Pass 1 *******************

#ifndef DISABLED_LEGACY_ENGINE
    // If the adaptive classifier is full switch to one we prepared earlier,
    // ie on the previous page. If the current adaptive classifier is non-empty,
    // prepare a backup starting at this page, in case it fills up. Do all this
    // independently for each language.
    if (AdaptiveClassifierIsFull()) {
      SwitchAdaptiveClassifier();
    } else if (!AdaptiveClassifierIsEmpty()) {
      StartBackupAdaptiveClassifier();
    }
    // Now check the sub-langs as well.
    for (auto &lang : sub_langs_) {
      if (lang->AdaptiveClassifierIsFull()) {
        lang->SwitchAdaptiveClassifier();
      } else if (!lang->AdaptiveClassifierIsEmpty()) {
        lang->StartBackupAdaptiveClassifier();
      }
    }

#endif // ndef DISABLED_LEGACY_ENGINE

    // Set up all words ready for recognition, so that if parallelism is on
    // all the input and output classes are ready to run the classifier.
    std::vector<WordData> words;
    SetupAllWordsPassN(1, target_word_box, word_config, page_res, &words);
#ifndef DISABLED_LEGACY_ENGINE
    if (tessedit_parallelize) {
      PrerecAllWordsPar(words);
    }
#endif // ndef DISABLED_LEGACY_ENGINE

    stats_.word_count = words.size();

    stats_.dict_words = 0;
    stats_.doc_blob_quality = 0;
    stats_.doc_outline_errs = 0;
    stats_.doc_char_quality = 0;
    stats_.good_char_count = 0;
    stats_.doc_good_char_quality = 0;

    most_recently_used_ = this;
    // Run pass 1 word recognition.
    if (!RecogAllWordsPassN(1, monitor, &page_res_it, &words)) {
      return false;
    }
    // Pass 1 post-processing.
    for (page_res_it.restart_page(); page_res_it.word() != nullptr; page_res_it.forward()) {
      if (page_res_it.word()->word->flag(W_REP_CHAR)) {
        fix_rep_char(&page_res_it);
        continue;
      }

      // Count dict words.
      if (page_res_it.word()->best_choice->permuter() == USER_DAWG_PERM) {
        ++(stats_.dict_words);
      }

      // Update misadaption log (we only need to do it on pass 1, since
      // adaption only happens on this pass).
      if (page_res_it.word()->blamer_bundle != nullptr &&
          page_res_it.word()->blamer_bundle->misadaption_debug().length() > 0) {
        page_res->misadaption_log.push_back(page_res_it.word()->blamer_bundle->misadaption_debug());
      }
    }
  }

  if (dopasses == 1) {
    return true;
  }

#ifndef DISABLED_LEGACY_ENGINE

  // ****************** Pass 2 *******************
  if (tessedit_tess_adaption_mode != 0x0 && !tessedit_test_adaption && AnyTessLang()) {
    page_res_it.restart_page();
    std::vector<WordData> words;
    SetupAllWordsPassN(2, target_word_box, word_config, page_res, &words);
    if (tessedit_parallelize) {
      PrerecAllWordsPar(words);
    }
    most_recently_used_ = this;
    // Run pass 2 word recognition.
    if (!RecogAllWordsPassN(2, monitor, &page_res_it, &words)) {
      return false;
    }
  }

  // The next passes are only required for Tess-only.
  if (AnyTessLang() && !AnyLSTMLang()) {
    // ****************** Pass 3 *******************
    // Fix fuzzy spaces.

    if (!tessedit_test_adaption && tessedit_fix_fuzzy_spaces && !tessedit_word_for_word &&
        !right_to_left()) {
      fix_fuzzy_spaces(monitor, stats_.word_count, page_res);
    }

    // ****************** Pass 4 *******************
    if (tessedit_enable_dict_correction) {
      dictionary_correction_pass(page_res);
    }
    if (tessedit_enable_bigram_correction) {
      bigram_correction_pass(page_res);
    }

    // ****************** Pass 5,6 *******************
    rejection_passes(page_res, monitor, target_word_box, word_config);

    // ****************** Pass 8 *******************
    font_recognition_pass(page_res);

    // ****************** Pass 9 *******************
    // Check the correctness of the final results.
    blamer_pass(page_res);
    script_pos_pass(page_res);
  }

#endif // ndef DISABLED_LEGACY_ENGINE

  // Write results pass.
  // This is now redundant, but retained commented so show how to obtain
  // bounding boxes and style information.

#ifndef DISABLED_LEGACY_ENGINE
  // changed by jetsoft
  // needed for dll to output memory structure
  if ((dopasses == 0 || dopasses == 2) && (monitor || tessedit_write_unlv)) {
    output_pass(page_res_it, target_word_box);
  }
// end jetsoft
#endif // ndef DISABLED_LEGACY_ENGINE

  const auto pageseg_mode = static_cast<PageSegMode>(static_cast<int>(tessedit_pageseg_mode));
  textord_.CleanupSingleRowResult(pageseg_mode, page_res);

  // Remove empty words, as these mess up the result iterators.
  for (page_res_it.restart_page(); page_res_it.word() != nullptr; page_res_it.forward()) {
    const WERD_RES *word = page_res_it.word();
    const POLY_BLOCK *pb = page_res_it.block()->block != nullptr
                               ? page_res_it.block()->block->pdblk.poly_block()
                               : nullptr;
    if (word->best_choice == nullptr || word->best_choice->empty() ||
        (word->best_choice->IsAllSpaces() && (pb == nullptr || pb->IsText()))) {
      page_res_it.DeleteCurrentWord();
    }
  }

  if (monitor != nullptr) {
    monitor->progress = 100;
  }
  return true;
}

#ifndef DISABLED_LEGACY_ENGINE

void Tesseract::bigram_correction_pass(PAGE_RES *page_res) {
  PAGE_RES_IT word_it(page_res);

  WERD_RES *w_prev = nullptr;
  WERD_RES *w = word_it.word();
  while (true) {
    w_prev = w;
    while (word_it.forward() != nullptr && (!word_it.word() || word_it.word()->part_of_combo)) {
      // advance word_it, skipping over parts of combos
    }
    if (!word_it.word()) {
      break;
    }
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
    std::vector<WERD_CHOICE *> overrides_word1;
    std::vector<WERD_CHOICE *> overrides_word2;

    const auto &orig_w1_str = w_prev->best_choice->unichar_string();
    const auto &orig_w2_str = w->best_choice->unichar_string();
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
        tprintf("Top choice \"%s %s\" verified by bigram model.\n", orig_w1_str.c_str(),
                orig_w2_str.c_str());
      }
      continue;
    }
    if (tessedit_bigram_debug > 2) {
      tprintf("Examining alt choices for \"%s %s\".\n", orig_w1_str.c_str(), orig_w2_str.c_str());
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
          if (overrides_word1.size() == 1 || p1->rating() + p2->rating() < best_rating) {
            best_rating = p1->rating() + p2->rating();
            best_idx = overrides_word1.size() - 1;
          }
        }
      }
    }
    if (!overrides_word1.empty()) {
      // Excellent, we have some bigram matches.
      if (EqualIgnoringCaseAndTerminalPunct(*w_prev->best_choice, *overrides_word1[best_idx]) &&
          EqualIgnoringCaseAndTerminalPunct(*w->best_choice, *overrides_word2[best_idx])) {
        if (tessedit_bigram_debug > 1) {
          tprintf(
              "Top choice \"%s %s\" verified (sans case) by bigram "
              "model.\n",
              orig_w1_str.c_str(), orig_w2_str.c_str());
        }
        continue;
      }
      const auto &new_w1_str = overrides_word1[best_idx]->unichar_string();
      const auto &new_w2_str = overrides_word2[best_idx]->unichar_string();
      if (new_w1_str != orig_w1_str) {
        w_prev->ReplaceBestChoice(overrides_word1[best_idx]);
      }
      if (new_w2_str != orig_w2_str) {
        w->ReplaceBestChoice(overrides_word2[best_idx]);
      }
      if (tessedit_bigram_debug > 0) {
        std::string choices_description;
        int num_bigram_choices = overrides_word1.size() * overrides_word2.size();
        if (num_bigram_choices == 1) {
          choices_description = "This was the unique bigram choice.";
        } else {
          if (tessedit_bigram_debug > 1) {
            std::string bigrams_list;
            const int kMaxChoicesToPrint = 20;
            for (unsigned i = 0; i < overrides_word1.size() && i < kMaxChoicesToPrint; i++) {
              if (i > 0) {
                bigrams_list += ", ";
              }
              WERD_CHOICE *p1 = overrides_word1[i];
              WERD_CHOICE *p2 = overrides_word2[i];
              bigrams_list += p1->unichar_string() + " " + p2->unichar_string();
            }
            choices_description = "There were many choices: {";
            choices_description += bigrams_list;
            choices_description += "}";
          } else {
            choices_description += "There were " + std::to_string(num_bigram_choices);
            choices_description += " compatible bigrams.";
          }
        }
        tprintf("Replaced \"%s %s\" with \"%s %s\" with bigram model. %s\n", orig_w1_str.c_str(),
                orig_w2_str.c_str(), new_w1_str.c_str(), new_w2_str.c_str(),
                choices_description.c_str());
      }
    }
  }
}

void Tesseract::rejection_passes(PAGE_RES *page_res, ETEXT_DESC *monitor,
                                 const TBOX *target_word_box, const char *word_config) {
  PAGE_RES_IT page_res_it(page_res);
  // ****************** Pass 5 *******************
  // Gather statistics on rejects.
  int word_index = 0;
  while (!tessedit_test_adaption && page_res_it.word() != nullptr) {
    WERD_RES *word = page_res_it.word();
    word_index++;
    if (monitor != nullptr) {
      monitor->ocr_alive = true;
      monitor->progress = 95 + 5 * word_index / stats_.word_count;
    }
    if (word->rebuild_word == nullptr) {
      // Word was not processed by tesseract.
      page_res_it.forward();
      continue;
    }
    check_debug_pt(word, 70);

    // changed by jetsoft
    // specific to its needs to extract one word when need
    if (target_word_box &&
        !ProcessTargetWord(word->word->bounding_box(), *target_word_box, word_config, 4)) {
      page_res_it.forward();
      continue;
    }
    // end jetsoft

    page_res_it.rej_stat_word();
    const int chars_in_word = word->reject_map.length();
    const int rejects_in_word = word->reject_map.reject_count();

    const int blob_quality = word_blob_quality(word);
    stats_.doc_blob_quality += blob_quality;
    const int outline_errs = word_outline_errs(word);
    stats_.doc_outline_errs += outline_errs;
    int16_t all_char_quality;
    int16_t accepted_all_char_quality;
    word_char_quality(word, &all_char_quality, &accepted_all_char_quality);
    stats_.doc_char_quality += all_char_quality;
    const uint8_t permuter_type = word->best_choice->permuter();
    if ((permuter_type == SYSTEM_DAWG_PERM) || (permuter_type == FREQ_DAWG_PERM) ||
        (permuter_type == USER_DAWG_PERM)) {
      stats_.good_char_count += chars_in_word - rejects_in_word;
      stats_.doc_good_char_quality += accepted_all_char_quality;
    }
    check_debug_pt(word, 80);
    if (tessedit_reject_bad_qual_wds && (blob_quality == 0) && (outline_errs >= chars_in_word)) {
      word->reject_map.rej_word_bad_quality();
    }
    check_debug_pt(word, 90);
    page_res_it.forward();
  }

  if (tessedit_debug_quality_metrics) {
    tprintf(
        "QUALITY: num_chs= %d  num_rejs= %d %5.3f blob_qual= %d %5.3f"
        " outline_errs= %d %5.3f char_qual= %d %5.3f good_ch_qual= %d %5.3f\n",
        page_res->char_count, page_res->rej_count,
        page_res->rej_count / static_cast<float>(page_res->char_count), stats_.doc_blob_quality,
        stats_.doc_blob_quality / static_cast<float>(page_res->char_count), stats_.doc_outline_errs,
        stats_.doc_outline_errs / static_cast<float>(page_res->char_count), stats_.doc_char_quality,
        stats_.doc_char_quality / static_cast<float>(page_res->char_count),
        stats_.doc_good_char_quality,
        (stats_.good_char_count > 0)
            ? (stats_.doc_good_char_quality / static_cast<float>(stats_.good_char_count))
            : 0.0);
  }
  bool good_quality_doc =
      ((page_res->rej_count / static_cast<float>(page_res->char_count)) <= quality_rej_pc) &&
      (stats_.doc_blob_quality / static_cast<float>(page_res->char_count) >= quality_blob_pc) &&
      (stats_.doc_outline_errs / static_cast<float>(page_res->char_count) <= quality_outline_pc) &&
      (stats_.doc_char_quality / static_cast<float>(page_res->char_count) >= quality_char_pc);

  // ****************** Pass 6 *******************
  // Do whole document or whole block rejection pass
  if (!tessedit_test_adaption) {
    quality_based_rejection(page_res_it, good_quality_doc);
  }
}

#endif // ndef DISABLED_LEGACY_ENGINE

void Tesseract::blamer_pass(PAGE_RES *page_res) {
  if (!wordrec_run_blamer) {
    return;
  }
  PAGE_RES_IT page_res_it(page_res);
  for (page_res_it.restart_page(); page_res_it.word() != nullptr; page_res_it.forward()) {
    WERD_RES *word = page_res_it.word();
    BlamerBundle::LastChanceBlame(wordrec_debug_blamer, word);
    page_res->blame_reasons[word->blamer_bundle->incorrect_result_reason()]++;
  }
  tprintf("Blame reasons:\n");
  for (int bl = 0; bl < IRR_NUM_REASONS; ++bl) {
    tprintf("%s %d\n", BlamerBundle::IncorrectReasonName(static_cast<IncorrectResultReason>(bl)),
            page_res->blame_reasons[bl]);
  }
  if (page_res->misadaption_log.size() > 0) {
    tprintf("Misadaption log:\n");
    for (auto &log : page_res->misadaption_log) {
      tprintf("%s\n", log.c_str());
    }
  }
}

// Sets script positions and detects smallcaps on all output words.
void Tesseract::script_pos_pass(PAGE_RES *page_res) {
  PAGE_RES_IT page_res_it(page_res);
  for (page_res_it.restart_page(); page_res_it.word() != nullptr; page_res_it.forward()) {
    WERD_RES *word = page_res_it.word();
    if (word->word->flag(W_REP_CHAR)) {
      page_res_it.forward();
      continue;
    }
    const float x_height = page_res_it.block()->block->x_height();
    float word_x_height = word->x_height;
    if (word_x_height < word->best_choice->min_x_height() ||
        word_x_height > word->best_choice->max_x_height()) {
      word_x_height =
          (word->best_choice->min_x_height() + word->best_choice->max_x_height()) / 2.0f;
    }
    // Test for small caps. Word capheight must be close to block xheight,
    // and word must contain no lower case letters, and at least one upper case.
    const double small_cap_xheight = x_height * kXHeightCapRatio;
    const double small_cap_delta = (x_height - small_cap_xheight) / 2.0;
    if (word->uch_set->script_has_xheight() &&
        small_cap_xheight - small_cap_delta <= word_x_height &&
        word_x_height <= small_cap_xheight + small_cap_delta) {
      // Scan for upper/lower.
      int num_upper = 0;
      int num_lower = 0;
      for (unsigned i = 0; i < word->best_choice->length(); ++i) {
        if (word->uch_set->get_isupper(word->best_choice->unichar_id(i))) {
          ++num_upper;
        } else if (word->uch_set->get_islower(word->best_choice->unichar_id(i))) {
          ++num_lower;
        }
      }
      if (num_upper > 0 && num_lower == 0) {
        word->small_caps = true;
      }
    }
    word->SetScriptPositions();
  }
}

// Helper finds the gap between the index word and the next.
static void WordGap(const PointerVector<WERD_RES> &words, unsigned index, int *right, int *next_left) {
  *right = -INT32_MAX;
  *next_left = INT32_MAX;
  if (index < words.size()) {
    *right = words[index]->word->bounding_box().right();
    if (index + 1 < words.size()) {
      *next_left = words[index + 1]->word->bounding_box().left();
    }
  }
}

// Factored helper computes the rating, certainty, badness and validity of
// the permuter of the words in [first_index, end_index).
static void EvaluateWordSpan(const PointerVector<WERD_RES> &words, unsigned first_index, unsigned end_index,
                             float *rating, float *certainty, bool *bad, bool *valid_permuter) {
  if (end_index <= first_index) {
    *bad = true;
    *valid_permuter = false;
  }
  for (unsigned index = first_index; index < end_index && index < words.size(); ++index) {
    WERD_CHOICE *choice = words[index]->best_choice;
    if (choice == nullptr) {
      *bad = true;
    } else {
      *rating += choice->rating();
      *certainty = std::min(*certainty, choice->certainty());
      if (!Dict::valid_word_permuter(choice->permuter(), false)) {
        *valid_permuter = false;
      }
    }
  }
}

// Helper chooses the best combination of words, transferring good ones from
// new_words to best_words. To win, a new word must have (better rating and
// certainty) or (better permuter status and rating within rating ratio and
// certainty within certainty margin) than current best.
// All the new_words are consumed (moved to best_words or deleted.)
// The return value is the number of new_words used minus the number of
// best_words that remain in the output.
static int SelectBestWords(double rating_ratio, double certainty_margin, bool debug,
                           PointerVector<WERD_RES> *new_words,
                           PointerVector<WERD_RES> *best_words) {
  // Process the smallest groups of words that have an overlapping word
  // boundary at the end.
  std::vector<WERD_RES *> out_words;
  // Index into each word vector (best, new).
  unsigned b = 0, n = 0;
  int num_best = 0, num_new = 0;
  while (b < best_words->size() || n < new_words->size()) {
    // Start of the current run in each.
    auto start_b = b, start_n = n;
    while (b < best_words->size() || n < new_words->size()) {
      int b_right = -INT32_MAX;
      int next_b_left = INT32_MAX;
      WordGap(*best_words, b, &b_right, &next_b_left);
      int n_right = -INT32_MAX;
      int next_n_left = INT32_MAX;
      WordGap(*new_words, n, &n_right, &next_n_left);
      if (std::max(b_right, n_right) < std::min(next_b_left, next_n_left)) {
        // The word breaks overlap. [start_b,b] and [start_n, n] match.
        break;
      }
      // Keep searching for the matching word break.
      if ((b_right < n_right && b < best_words->size()) || n == new_words->size()) {
        ++b;
      } else {
        ++n;
      }
    }
    // Rating of the current run in each.
    float b_rating = 0.0f, n_rating = 0.0f;
    // Certainty of the current run in each.
    float b_certainty = 0.0f, n_certainty = 0.0f;
    // True if any word is missing its best choice.
    bool b_bad = false, n_bad = false;
    // True if all words have a valid permuter.
    bool b_valid_permuter = true, n_valid_permuter = true;
    const int end_b = b < best_words->size() ? b + 1 : b;
    const int end_n = n < new_words->size() ? n + 1 : n;
    EvaluateWordSpan(*best_words, start_b, end_b, &b_rating, &b_certainty, &b_bad,
                     &b_valid_permuter);
    EvaluateWordSpan(*new_words, start_n, end_n, &n_rating, &n_certainty, &n_bad,
                     &n_valid_permuter);
    bool new_better = false;
    if (!n_bad && (b_bad || (n_certainty > b_certainty && n_rating < b_rating) ||
                   (!b_valid_permuter && n_valid_permuter && n_rating < b_rating * rating_ratio &&
                    n_certainty > b_certainty - certainty_margin))) {
      // New is better.
      for (int i = start_n; i < end_n; ++i) {
        out_words.push_back((*new_words)[i]);
        (*new_words)[i] = nullptr;
        ++num_new;
      }
      new_better = true;
    } else if (!b_bad) {
      // Current best is better.
      for (int i = start_b; i < end_b; ++i) {
        out_words.push_back((*best_words)[i]);
        (*best_words)[i] = nullptr;
        ++num_best;
      }
    }
    if (debug) {
      tprintf(
          "%d new words %s than %d old words: r: %g v %g c: %g v %g"
          " valid dict: %d v %d\n",
          end_n - start_n, new_better ? "better" : "worse", end_b - start_b, n_rating, b_rating,
          n_certainty, b_certainty, n_valid_permuter, b_valid_permuter);
    }
    // Move on to the next group.
    b = end_b;
    n = end_n;
  }
  // Transfer from out_words to best_words.
  best_words->clear();
  for (auto &out_word : out_words) {
    best_words->push_back(out_word);
  }
  return num_new - num_best;
}

// Helper to recognize the word using the given (language-specific) tesseract.
// Returns positive if this recognizer found more new best words than the
// number kept from best_words.
int Tesseract::RetryWithLanguage(const WordData &word_data, WordRecognizer recognizer, bool debug,
                                 WERD_RES **in_word, PointerVector<WERD_RES> *best_words) {
  if (debug) {
    tprintf("Trying word using lang %s, oem %d\n", lang.c_str(),
            static_cast<int>(tessedit_ocr_engine_mode));
  }
  // Run the recognizer on the word.
  PointerVector<WERD_RES> new_words;
  (this->*recognizer)(word_data, in_word, &new_words);
  if (new_words.empty()) {
    // Transfer input word to new_words, as the classifier must have put
    // the result back in the input.
    new_words.push_back(*in_word);
    *in_word = nullptr;
  }
  if (debug) {
    for (unsigned i = 0; i < new_words.size(); ++i) {
      new_words[i]->DebugTopChoice("Lang result");
    }
  }
  // Initial version is a bit of a hack based on better certainty and rating
  // or a dictionary vs non-dictionary word.
  return SelectBestWords(classify_max_rating_ratio, classify_max_certainty_margin, debug,
                         &new_words, best_words);
}

// Helper returns true if all the words are acceptable.
static bool WordsAcceptable(const PointerVector<WERD_RES> &words) {
  for (unsigned w = 0; w < words.size(); ++w) {
    if (words[w]->tess_failed || !words[w]->tess_accepted) {
      return false;
    }
  }
  return true;
}

#ifndef DISABLED_LEGACY_ENGINE

// Moves good-looking "noise"/diacritics from the reject list to the main
// blob list on the current word. Returns true if anything was done, and
// sets make_next_word_fuzzy if blob(s) were added to the end of the word.
bool Tesseract::ReassignDiacritics(int pass, PAGE_RES_IT *pr_it, bool *make_next_word_fuzzy) {
  *make_next_word_fuzzy = false;
  WERD *real_word = pr_it->word()->word;
  if (real_word->rej_cblob_list()->empty() || real_word->cblob_list()->empty() ||
      real_word->rej_cblob_list()->length() > noise_maxperword) {
    return false;
  }
  real_word->rej_cblob_list()->sort(&C_BLOB::SortByXMiddle);
  // Get the noise outlines into a vector with matching bool map.
  std::vector<C_OUTLINE *> outlines;
  real_word->GetNoiseOutlines(&outlines);
  std::vector<bool> word_wanted;
  std::vector<bool> overlapped_any_blob;
  std::vector<C_BLOB *> target_blobs;
  AssignDiacriticsToOverlappingBlobs(outlines, pass, real_word, pr_it, &word_wanted,
                                     &overlapped_any_blob, &target_blobs);
  // Filter the outlines that overlapped any blob and put them into the word
  // now. This simplifies the remaining task and also makes it more accurate
  // as it has more completed blobs to work on.
  std::vector<bool> wanted;
  std::vector<C_BLOB *> wanted_blobs;
  std::vector<C_OUTLINE *> wanted_outlines;
  int num_overlapped = 0;
  int num_overlapped_used = 0;
  for (unsigned i = 0; i < overlapped_any_blob.size(); ++i) {
    if (overlapped_any_blob[i]) {
      ++num_overlapped;
      if (word_wanted[i]) {
        ++num_overlapped_used;
      }
      wanted.push_back(word_wanted[i]);
      wanted_blobs.push_back(target_blobs[i]);
      wanted_outlines.push_back(outlines[i]);
      outlines[i] = nullptr;
    }
  }
  real_word->AddSelectedOutlines(wanted, wanted_blobs, wanted_outlines, nullptr);
  AssignDiacriticsToNewBlobs(outlines, pass, real_word, pr_it, &word_wanted, &target_blobs);
  // TODO: check code.
  int non_overlapped = 0;
  int non_overlapped_used = 0;
  for (unsigned i = 0; i < word_wanted.size(); ++i) {
    if (word_wanted[i]) {
      ++non_overlapped_used;
    }
    if (outlines[i] != nullptr) {
      ++non_overlapped_used;
    }
  }
  if (debug_noise_removal) {
    tprintf("Used %d/%d overlapped %d/%d non-overlapped diacritics on word:", num_overlapped_used,
            num_overlapped, non_overlapped_used, non_overlapped);
    real_word->bounding_box().print();
  }
  // Now we have decided which outlines we want, put them into the real_word.
  if (real_word->AddSelectedOutlines(word_wanted, target_blobs, outlines, make_next_word_fuzzy)) {
    pr_it->MakeCurrentWordFuzzy();
  }
  // TODO(rays) Parts of combos have a deep copy of the real word, and need
  // to have their noise outlines moved/assigned in the same way!!
  return num_overlapped_used != 0 || non_overlapped_used != 0;
}

// Attempts to put noise/diacritic outlines into the blobs that they overlap.
// Input: a set of noisy outlines that probably belong to the real_word.
// Output: word_wanted indicates which outlines are to be assigned to a blob,
//   target_blobs indicates which to assign to, and overlapped_any_blob is
//   true for all outlines that overlapped a blob.
void Tesseract::AssignDiacriticsToOverlappingBlobs(const std::vector<C_OUTLINE *> &outlines,
                                                   int pass, WERD *real_word, PAGE_RES_IT *pr_it,
                                                   std::vector<bool> *word_wanted,
                                                   std::vector<bool> *overlapped_any_blob,
                                                   std::vector<C_BLOB *> *target_blobs) {
  std::vector<bool> blob_wanted;
  word_wanted->clear();
  word_wanted->resize(outlines.size());
  overlapped_any_blob->clear();
  overlapped_any_blob->resize(outlines.size());
  target_blobs->clear();
  target_blobs->resize(outlines.size());
  // For each real blob, find the outlines that seriously overlap it.
  // A single blob could be several merged characters, so there can be quite
  // a few outlines overlapping, and the full engine needs to be used to chop
  // and join to get a sensible result.
  C_BLOB_IT blob_it(real_word->cblob_list());
  for (blob_it.mark_cycle_pt(); !blob_it.cycled_list(); blob_it.forward()) {
    C_BLOB *blob = blob_it.data();
    const TBOX blob_box = blob->bounding_box();
    blob_wanted.clear();
    blob_wanted.resize(outlines.size());
    int num_blob_outlines = 0;
    for (unsigned i = 0; i < outlines.size(); ++i) {
      if (blob_box.major_x_overlap(outlines[i]->bounding_box()) && !(*word_wanted)[i]) {
        blob_wanted[i] = true;
        (*overlapped_any_blob)[i] = true;
        ++num_blob_outlines;
      }
    }
    if (debug_noise_removal) {
      tprintf("%d noise outlines overlap blob at:", num_blob_outlines);
      blob_box.print();
    }
    // If any outlines overlap the blob, and not too many, classify the blob
    // (using the full engine, languages and all), and choose the maximal
    // combination of outlines that doesn't hurt the end-result classification
    // by too much. Mark them as wanted.
    if (0 < num_blob_outlines && num_blob_outlines < noise_maxperblob) {
      if (SelectGoodDiacriticOutlines(pass, noise_cert_basechar, pr_it, blob, outlines,
                                      num_blob_outlines, &blob_wanted)) {
        for (unsigned i = 0; i < blob_wanted.size(); ++i) {
          if (blob_wanted[i]) {
            // Claim the outline and record where it is going.
            (*word_wanted)[i] = true;
            (*target_blobs)[i] = blob;
          }
        }
      }
    }
  }
}

// Attempts to assign non-overlapping outlines to their nearest blobs or
// make new blobs out of them.
void Tesseract::AssignDiacriticsToNewBlobs(const std::vector<C_OUTLINE *> &outlines, int pass,
                                           WERD *real_word, PAGE_RES_IT *pr_it,
                                           std::vector<bool> *word_wanted,
                                           std::vector<C_BLOB *> *target_blobs) {
  std::vector<bool> blob_wanted;
  word_wanted->clear();
  word_wanted->resize(outlines.size());
  target_blobs->clear();
  target_blobs->resize(outlines.size());
  // Check for outlines that need to be turned into stand-alone blobs.
  for (unsigned i = 0; i < outlines.size(); ++i) {
    if (outlines[i] == nullptr) {
      continue;
    }
    // Get a set of adjacent outlines that don't overlap any existing blob.
    blob_wanted.clear();
    blob_wanted.resize(outlines.size());
    int num_blob_outlines = 0;
    TBOX total_ol_box(outlines[i]->bounding_box());
    while (i < outlines.size() && outlines[i] != nullptr) {
      blob_wanted[i] = true;
      total_ol_box += outlines[i]->bounding_box();
      ++i;
      ++num_blob_outlines;
    }
    // Find the insertion point.
    C_BLOB_IT blob_it(real_word->cblob_list());
    while (!blob_it.at_last() &&
           blob_it.data_relative(1)->bounding_box().left() <= total_ol_box.left()) {
      blob_it.forward();
    }
    // Choose which combination of them we actually want and where to put
    // them.
    if (debug_noise_removal) {
      tprintf("Num blobless outlines = %d\n", num_blob_outlines);
    }
    C_BLOB *left_blob = blob_it.data();
    TBOX left_box = left_blob->bounding_box();
    C_BLOB *right_blob = blob_it.at_last() ? nullptr : blob_it.data_relative(1);
    if ((left_box.x_overlap(total_ol_box) || right_blob == nullptr ||
         !right_blob->bounding_box().x_overlap(total_ol_box)) &&
        SelectGoodDiacriticOutlines(pass, noise_cert_disjoint, pr_it, left_blob, outlines,
                                    num_blob_outlines, &blob_wanted)) {
      if (debug_noise_removal) {
        tprintf("Added to left blob\n");
      }
      for (unsigned j = 0; j < blob_wanted.size(); ++j) {
        if (blob_wanted[j]) {
          (*word_wanted)[j] = true;
          (*target_blobs)[j] = left_blob;
        }
      }
    } else if (right_blob != nullptr &&
               (!left_box.x_overlap(total_ol_box) ||
                right_blob->bounding_box().x_overlap(total_ol_box)) &&
               SelectGoodDiacriticOutlines(pass, noise_cert_disjoint, pr_it, right_blob, outlines,
                                           num_blob_outlines, &blob_wanted)) {
      if (debug_noise_removal) {
        tprintf("Added to right blob\n");
      }
      for (unsigned j = 0; j < blob_wanted.size(); ++j) {
        if (blob_wanted[j]) {
          (*word_wanted)[j] = true;
          (*target_blobs)[j] = right_blob;
        }
      }
    } else if (SelectGoodDiacriticOutlines(pass, noise_cert_punc, pr_it, nullptr, outlines,
                                           num_blob_outlines, &blob_wanted)) {
      if (debug_noise_removal) {
        tprintf("Fitted between blobs\n");
      }
      for (unsigned j = 0; j < blob_wanted.size(); ++j) {
        if (blob_wanted[j]) {
          (*word_wanted)[j] = true;
          (*target_blobs)[j] = nullptr;
        }
      }
    }
  }
}

// Starting with ok_outlines set to indicate which outlines overlap the blob,
// chooses the optimal set (approximately) and returns true if any outlines
// are desired, in which case ok_outlines indicates which ones.
bool Tesseract::SelectGoodDiacriticOutlines(int pass, float certainty_threshold, PAGE_RES_IT *pr_it,
                                            C_BLOB *blob,
                                            const std::vector<C_OUTLINE *> &outlines,
                                            int num_outlines, std::vector<bool> *ok_outlines) {
  float target_cert = certainty_threshold;
  if (blob != nullptr) {
    std::string best_str;
    float target_c2;
    target_cert = ClassifyBlobAsWord(pass, pr_it, blob, best_str, &target_c2);
    if (debug_noise_removal) {
      tprintf("No Noise blob classified as %s=%g(%g) at:", best_str.c_str(), target_cert,
              target_c2);
      blob->bounding_box().print();
    }
    target_cert -= (target_cert - certainty_threshold) * noise_cert_factor;
  }
  std::vector<bool> test_outlines = *ok_outlines;
  // Start with all the outlines in.
  std::string all_str;
  std::vector<bool> best_outlines = *ok_outlines;
  float best_cert = ClassifyBlobPlusOutlines(test_outlines, outlines, pass, pr_it, blob, all_str);
  if (debug_noise_removal) {
    TBOX ol_box;
    for (unsigned i = 0; i < test_outlines.size(); ++i) {
      if (test_outlines[i]) {
        ol_box += outlines[i]->bounding_box();
      }
    }
    tprintf("All Noise blob classified as %s=%g, delta=%g at:", all_str.c_str(), best_cert,
            best_cert - target_cert);
    ol_box.print();
  }
  // Iteratively zero out the bit that improves the certainty the most, until
  // we get past the threshold, have zero bits, or fail to improve.
  int best_index = 0; // To zero out.
  while (num_outlines > 1 && best_index >= 0 &&
         (blob == nullptr || best_cert < target_cert || blob != nullptr)) {
    // Find the best bit to zero out.
    best_index = -1;
    for (unsigned i = 0; i < outlines.size(); ++i) {
      if (test_outlines[i]) {
        test_outlines[i] = false;
        std::string str;
        float cert = ClassifyBlobPlusOutlines(test_outlines, outlines, pass, pr_it, blob, str);
        if (debug_noise_removal) {
          TBOX ol_box;
          for (unsigned j = 0; j < outlines.size(); ++j) {
            if (test_outlines[j]) {
              ol_box += outlines[j]->bounding_box();
            }
            tprintf("%c", test_outlines[j] ? 'T' : 'F');
          }
          tprintf(" blob classified as %s=%g, delta=%g) at:", str.c_str(), cert,
                  cert - target_cert);
          ol_box.print();
        }
        if (cert > best_cert) {
          best_cert = cert;
          best_index = i;
          best_outlines = test_outlines;
        }
        test_outlines[i] = true;
      }
    }
    if (best_index >= 0) {
      test_outlines[best_index] = false;
      --num_outlines;
    }
  }
  if (best_cert >= target_cert) {
    // Save the best combination.
    *ok_outlines = best_outlines;
    if (debug_noise_removal) {
      tprintf("%s noise combination ", blob ? "Adding" : "New");
      for (auto &&best_outline : best_outlines) {
        tprintf("%c", best_outline ? 'T' : 'F');
      }
      tprintf(" yields certainty %g, beating target of %g\n", best_cert, target_cert);
    }
    return true;
  }

  return false;
}

// Classifies the given blob plus the outlines flagged by ok_outlines, undoes
// the inclusion of the outlines, and returns the certainty of the raw choice.
float Tesseract::ClassifyBlobPlusOutlines(const std::vector<bool> &ok_outlines,
                                          const std::vector<C_OUTLINE *> &outlines, int pass_n,
                                          PAGE_RES_IT *pr_it, C_BLOB *blob, std::string &best_str) {
  C_OUTLINE_IT ol_it;
  C_OUTLINE *first_to_keep = nullptr;
  C_BLOB *local_blob = nullptr;
  if (blob != nullptr) {
    // Add the required outlines to the blob.
    ol_it.set_to_list(blob->out_list());
    first_to_keep = ol_it.data();
  }
  for (unsigned i = 0; i < ok_outlines.size(); ++i) {
    if (ok_outlines[i]) {
      // This outline is to be added.
      if (blob == nullptr) {
        local_blob = new C_BLOB(outlines[i]);
        blob = local_blob;
        ol_it.set_to_list(blob->out_list());
      } else {
        ol_it.add_before_stay_put(outlines[i]);
      }
    }
  }
  float c2;
  float cert = ClassifyBlobAsWord(pass_n, pr_it, blob, best_str, &c2);
  ol_it.move_to_first();
  if (first_to_keep == nullptr) {
    // We created blob. Empty its outlines and delete it.
    for (; !ol_it.empty(); ol_it.forward()) {
      ol_it.extract();
    }
    delete local_blob;
    cert = -c2;
  } else {
    // Remove the outlines that we put in.
    for (; ol_it.data() != first_to_keep; ol_it.forward()) {
      ol_it.extract();
    }
  }
  return cert;
}

// Classifies the given blob (part of word_data->word->word) as an individual
// word, using languages, chopper etc, returning only the certainty of the
// best raw choice, and undoing all the work done to fake out the word.
float Tesseract::ClassifyBlobAsWord(int pass_n, PAGE_RES_IT *pr_it, C_BLOB *blob, std::string &best_str,
                                    float *c2) {
  WERD *real_word = pr_it->word()->word;
  WERD *word = real_word->ConstructFromSingleBlob(real_word->flag(W_BOL), real_word->flag(W_EOL),
                                                  C_BLOB::deep_copy(blob));
  WERD_RES *word_res = pr_it->InsertSimpleCloneWord(*pr_it->word(), word);
  // Get a new iterator that points to the new word.
  PAGE_RES_IT it(pr_it->page_res);
  while (it.word() != word_res && it.word() != nullptr) {
    it.forward();
  }
  ASSERT_HOST(it.word() == word_res);
  WordData wd(it);
  // Force full initialization.
  SetupWordPassN(1, &wd);
  classify_word_and_language(pass_n, &it, &wd);
  if (debug_noise_removal) {
    if (wd.word->raw_choice != nullptr) {
      tprintf("word xheight=%g, row=%g, range=[%g,%g]\n", word_res->x_height, wd.row->x_height(),
              wd.word->raw_choice->min_x_height(), wd.word->raw_choice->max_x_height());
    } else {
      tprintf("Got word with null raw choice xheight=%g, row=%g\n", word_res->x_height,
              wd.row->x_height());
    }
  }
  float cert = 0.0f;
  if (wd.word->raw_choice != nullptr) { // This probably shouldn't happen, but...
    cert = wd.word->raw_choice->certainty();
    float rat = wd.word->raw_choice->rating();
    *c2 = rat > 0.0f ? cert * cert / rat : 0.0f;
    best_str = wd.word->raw_choice->unichar_string();
  } else {
    *c2 = 0.0f;
    best_str.clear();
  }
  it.DeleteCurrentWord();
  pr_it->ResetWordIterator();
  return cert;
}

#endif // ndef DISABLED_LEGACY_ENGINE

// Generic function for classifying a word. Can be used either for pass1 or
// pass2 according to the function passed to recognizer.
// word_data holds the word to be recognized, and its block and row, and
// pr_it points to the word as well, in case we are running LSTM and it wants
// to output multiple words.
// Recognizes in the current language, and if successful that is all.
// If recognition was not successful, tries all available languages until
// it gets a successful result or runs out of languages. Keeps the best result.
void Tesseract::classify_word_and_language(int pass_n, PAGE_RES_IT *pr_it, WordData *word_data) {
#ifdef DISABLED_LEGACY_ENGINE
  WordRecognizer recognizer = &Tesseract::classify_word_pass1;
#else
  WordRecognizer recognizer =
      pass_n == 1 ? &Tesseract::classify_word_pass1 : &Tesseract::classify_word_pass2;
#endif // def DISABLED_LEGACY_ENGINE

  // Best result so far.
  PointerVector<WERD_RES> best_words;
  // Points to the best result. May be word or in lang_words.
  const WERD_RES *word = word_data->word;
  clock_t total_time = 0;
  const bool timing_debug = tessedit_timing_debug;
  if (timing_debug) {
    total_time = clock();
  }
  const bool debug = classify_debug_level > 0 || multilang_debug_level > 0;
  if (debug) {
    tprintf("%s word with lang %s at:", word->done ? "Already done" : "Processing",
            most_recently_used_->lang.c_str());
    word->word->bounding_box().print();
  }
  if (word->done) {
    // If done on pass1, leave it as-is.
    if (!word->tess_failed) {
      most_recently_used_ = word->tesseract;
    }
    return;
  }
  auto sub = sub_langs_.size();
  if (most_recently_used_ != this) {
    // Get the index of the most_recently_used_.
    for (sub = 0; sub < sub_langs_.size() && most_recently_used_ != sub_langs_[sub]; ++sub) {
    }
  }
  most_recently_used_->RetryWithLanguage(*word_data, recognizer, debug, &word_data->lang_words[sub],
                                         &best_words);
  Tesseract *best_lang_tess = most_recently_used_;
  if (!WordsAcceptable(best_words)) {
    // Try all the other languages to see if they are any better.
    if (most_recently_used_ != this &&
        this->RetryWithLanguage(*word_data, recognizer, debug,
                                &word_data->lang_words[sub_langs_.size()], &best_words) > 0) {
      best_lang_tess = this;
    }
    for (unsigned i = 0; !WordsAcceptable(best_words) && i < sub_langs_.size(); ++i) {
      if (most_recently_used_ != sub_langs_[i] &&
          sub_langs_[i]->RetryWithLanguage(*word_data, recognizer, debug, &word_data->lang_words[i],
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
    ASSERT_HOST(word_data->word->box_word != nullptr);
  } else {
    tprintf("no best words!!\n");
  }
  if (timing_debug) {
    total_time = clock() - total_time;
    tesserr << word_data->word->best_choice->unichar_string()
            << " (ocr took " << 1000 * total_time / CLOCKS_PER_SEC << " ms)\n";
  }
}

/**
 * classify_word_pass1
 *
 * Baseline normalize the word and pass it to Tess.
 */

void Tesseract::classify_word_pass1(const WordData &word_data, WERD_RES **in_word,
                                    PointerVector<WERD_RES> *out_words) {
  ROW *row = word_data.row;
  BLOCK *block = word_data.block;
  prev_word_best_choice_ =
      word_data.prev_word != nullptr ? word_data.prev_word->word->best_choice : nullptr;
#ifdef DISABLED_LEGACY_ENGINE
  if (tessedit_ocr_engine_mode == OEM_LSTM_ONLY) {
#else
  if (tessedit_ocr_engine_mode == OEM_LSTM_ONLY ||
      tessedit_ocr_engine_mode == OEM_TESSERACT_LSTM_COMBINED) {
#endif // def DISABLED_LEGACY_ENGINE
    if (!(*in_word)->odd_size || tessedit_ocr_engine_mode == OEM_LSTM_ONLY) {
      LSTMRecognizeWord(*block, row, *in_word, out_words);
      if (!out_words->empty()) {
        return; // Successful lstm recognition.
      }
    }
    if (tessedit_ocr_engine_mode == OEM_LSTM_ONLY) {
      // No fallback allowed, so use a fake.
      (*in_word)->SetupFake(lstm_recognizer_->GetUnicharset());
      return;
    }

#ifndef DISABLED_LEGACY_ENGINE
    // Fall back to tesseract for failed words or odd words.
    (*in_word)->SetupForRecognition(unicharset, this, BestPix(), OEM_TESSERACT_ONLY, nullptr,
                                    classify_bln_numeric_mode, textord_use_cjk_fp_model,
                                    poly_allow_detailed_fx, row, block);
#endif // ndef DISABLED_LEGACY_ENGINE
  }

#ifndef DISABLED_LEGACY_ENGINE
  WERD_RES *word = *in_word;
  match_word_pass_n(1, word, row, block);
  if (!word->tess_failed && !word->word->flag(W_REP_CHAR)) {
    word->tess_would_adapt = AdaptableWord(word);
    bool adapt_ok = word_adaptable(word, tessedit_tess_adaption_mode);

    if (adapt_ok) {
      // Send word to adaptive classifier for training.
      word->BestChoiceToCorrectText();
      LearnWord(nullptr, word);
      // Mark misadaptions if running blamer.
      if (word->blamer_bundle != nullptr) {
        word->blamer_bundle->SetMisAdaptionDebug(word->best_choice, wordrec_debug_blamer);
      }
    }

    if (tessedit_enable_doc_dict && !word->IsAmbiguous()) {
      tess_add_doc_word(word->best_choice);
    }
  }
#endif // ndef DISABLED_LEGACY_ENGINE
}

// Helper to report the result of the xheight fix.
void Tesseract::ReportXhtFixResult(bool accept_new_word, float new_x_ht, WERD_RES *word,
                                   WERD_RES *new_word) {
  tprintf("New XHT Match:%s = %s ", word->best_choice->unichar_string().c_str(),
          word->best_choice->debug_string().c_str());
  word->reject_map.print(debug_fp);
  tprintf(" -> %s = %s ", new_word->best_choice->unichar_string().c_str(),
          new_word->best_choice->debug_string().c_str());
  new_word->reject_map.print(debug_fp);
  tprintf(" %s->%s %s %s\n", word->guessed_x_ht ? "GUESS" : "CERT",
          new_word->guessed_x_ht ? "GUESS" : "CERT", new_x_ht > 0.1 ? "STILL DOUBT" : "OK",
          accept_new_word ? "ACCEPTED" : "");
}

#ifndef DISABLED_LEGACY_ENGINE

// Run the x-height fix-up, based on min/max top/bottom information in
// unicharset.
// Returns true if the word was changed.
// See the comment in fixxht.cpp for a description of the overall process.
bool Tesseract::TrainedXheightFix(WERD_RES *word, BLOCK *block, ROW *row) {
  int original_misfits = CountMisfitTops(word);
  if (original_misfits == 0) {
    return false;
  }
  float baseline_shift = 0.0f;
  float new_x_ht = ComputeCompatibleXheight(word, &baseline_shift);
  if (baseline_shift != 0.0f) {
    // Try the shift on its own first.
    if (!TestNewNormalization(original_misfits, baseline_shift, word->x_height, word, block, row)) {
      return false;
    }
    original_misfits = CountMisfitTops(word);
    if (original_misfits > 0) {
      float new_baseline_shift;
      // Now recompute the new x_height.
      new_x_ht = ComputeCompatibleXheight(word, &new_baseline_shift);
      if (new_x_ht >= kMinRefitXHeightFraction * word->x_height) {
        // No test of return value here, as we are definitely making a change
        // to the word by shifting the baseline.
        TestNewNormalization(original_misfits, baseline_shift, new_x_ht, word, block, row);
      }
    }
    return true;
  } else if (new_x_ht >= kMinRefitXHeightFraction * word->x_height) {
    return TestNewNormalization(original_misfits, 0.0f, new_x_ht, word, block, row);
  } else {
    return false;
  }
}

// Runs recognition with the test baseline shift and x-height and returns true
// if there was an improvement in recognition result.
bool Tesseract::TestNewNormalization(int original_misfits, float baseline_shift, float new_x_ht,
                                     WERD_RES *word, BLOCK *block, ROW *row) {
  bool accept_new_x_ht = false;
  WERD_RES new_x_ht_word(word->word);
  if (word->blamer_bundle != nullptr) {
    new_x_ht_word.blamer_bundle = new BlamerBundle();
    new_x_ht_word.blamer_bundle->CopyTruth(*(word->blamer_bundle));
  }
  new_x_ht_word.x_height = new_x_ht;
  new_x_ht_word.baseline_shift = baseline_shift;
  new_x_ht_word.caps_height = 0.0;
  new_x_ht_word.SetupForRecognition(unicharset, this, BestPix(), tessedit_ocr_engine_mode, nullptr,
                                    classify_bln_numeric_mode, textord_use_cjk_fp_model,
                                    poly_allow_detailed_fx, row, block);
  match_word_pass_n(2, &new_x_ht_word, row, block);
  if (!new_x_ht_word.tess_failed) {
    int new_misfits = CountMisfitTops(&new_x_ht_word);
    if (debug_x_ht_level >= 1) {
      tprintf("Old misfits=%d with x-height %f, new=%d with x-height %f\n", original_misfits,
              word->x_height, new_misfits, new_x_ht);
      tprintf("Old rating= %f, certainty=%f, new=%f, %f\n", word->best_choice->rating(),
              word->best_choice->certainty(), new_x_ht_word.best_choice->rating(),
              new_x_ht_word.best_choice->certainty());
    }
    // The misfits must improve and either the rating or certainty.
    accept_new_x_ht = new_misfits < original_misfits &&
                      (new_x_ht_word.best_choice->certainty() > word->best_choice->certainty() ||
                       new_x_ht_word.best_choice->rating() < word->best_choice->rating());
    if (debug_x_ht_level >= 1) {
      ReportXhtFixResult(accept_new_x_ht, new_x_ht, word, &new_x_ht_word);
    }
  }
  if (accept_new_x_ht) {
    word->ConsumeWordResults(&new_x_ht_word);
    return true;
  }
  return false;
}

#endif // ndef DISABLED_LEGACY_ENGINE

/**
 * classify_word_pass2
 *
 * Control what to do with the word in pass 2
 */

void Tesseract::classify_word_pass2(const WordData &word_data, WERD_RES **in_word,
                                    PointerVector<WERD_RES> *out_words) {
  // Return if we do not want to run Tesseract.
  if (tessedit_ocr_engine_mode == OEM_LSTM_ONLY) {
    return;
  }
#ifndef DISABLED_LEGACY_ENGINE
  ROW *row = word_data.row;
  BLOCK *block = word_data.block;
  WERD_RES *word = *in_word;
  prev_word_best_choice_ =
      word_data.prev_word != nullptr ? word_data.prev_word->word->best_choice : nullptr;

  check_debug_pt(word, 30);
  if (!word->done) {
    word->caps_height = 0.0;
    if (word->x_height == 0.0f) {
      word->x_height = row->x_height();
    }
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
  }
#  ifndef GRAPHICS_DISABLED
  if (tessedit_display_outwords) {
    if (fx_win == nullptr) {
      create_fx_win();
    }
    clear_fx_win();
    word->rebuild_word->plot(fx_win);
    TBOX wbox = word->rebuild_word->bounding_box();
    fx_win->ZoomToRectangle(wbox.left(), wbox.top(), wbox.right(), wbox.bottom());
    ScrollView::Update();
  }
#  endif
  check_debug_pt(word, 50);
#endif // ndef DISABLED_LEGACY_ENGINE
}

#ifndef DISABLED_LEGACY_ENGINE
/**
 * match_word_pass2
 *
 * Baseline normalize the word and pass it to Tess.
 */
void Tesseract::match_word_pass_n(int pass_n, WERD_RES *word, ROW *row, BLOCK *block) {
  if (word->tess_failed) {
    return;
  }
  tess_segment_pass_n(pass_n, word);

  if (!word->tess_failed) {
    if (!word->word->flag(W_REP_CHAR)) {
      word->fix_quotes();
      if (tessedit_fix_hyphens) {
        word->fix_hyphens();
      }
      /* Don't trust fix_quotes! - though I think I've fixed the bug */
      if (static_cast<unsigned>(word->best_choice->length()) != word->box_word->length()) {
        tprintf(
            "POST FIX_QUOTES FAIL String:\"%s\"; Strlen=%d;"
            " #Blobs=%u\n",
            word->best_choice->debug_string().c_str(), word->best_choice->length(),
            word->box_word->length());
      }
      word->tess_accepted = tess_acceptable_word(word);

      // Also sets word->done flag
      make_reject_map(word, row, pass_n);
    }
  }
  set_word_fonts(word);

  ASSERT_HOST(word->raw_choice != nullptr);
}
#endif // ndef DISABLED_LEGACY_ENGINE

// Helper to return the best rated BLOB_CHOICE in the whole word that matches
// the given char_id, or nullptr if none can be found.
static BLOB_CHOICE *FindBestMatchingChoice(UNICHAR_ID char_id, WERD_RES *word_res) {
  // Find the corresponding best BLOB_CHOICE from any position in the word_res.
  BLOB_CHOICE *best_choice = nullptr;
  for (unsigned i = 0; i < word_res->best_choice->length(); ++i) {
    BLOB_CHOICE *choice = FindMatchingChoice(char_id, word_res->GetBlobChoices(i));
    if (choice != nullptr) {
      if (best_choice == nullptr || choice->rating() < best_choice->rating()) {
        best_choice = choice;
      }
    }
  }
  return best_choice;
}

// Helper to insert blob_choice in each location in the leader word if there is
// no matching BLOB_CHOICE there already, and correct any incorrect results
// in the best_choice.
static void CorrectRepcharChoices(BLOB_CHOICE *blob_choice, WERD_RES *word_res) {
  WERD_CHOICE *word = word_res->best_choice;
  for (unsigned i = 0; i < word_res->best_choice->length(); ++i) {
    BLOB_CHOICE *choice =
        FindMatchingChoice(blob_choice->unichar_id(), word_res->GetBlobChoices(i));
    if (choice == nullptr) {
      BLOB_CHOICE_IT choice_it(word_res->GetBlobChoices(i));
      choice_it.add_before_stay_put(new BLOB_CHOICE(*blob_choice));
    }
  }
  // Correct any incorrect results in word.
  for (unsigned i = 0; i < word->length(); ++i) {
    if (word->unichar_id(i) != blob_choice->unichar_id()) {
      word->set_unichar_id(blob_choice->unichar_id(), i);
    }
  }
}

/**
 * fix_rep_char()
 * The word is a repeated char. (Leader.) Find the repeated char character.
 * Create the appropriate single-word or multi-word sequence according to
 * the size of spaces in between blobs, and correct the classifications
 * where some of the characters disagree with the majority.
 */
void Tesseract::fix_rep_char(PAGE_RES_IT *page_res_it) {
  WERD_RES *word_res = page_res_it->word();
  const WERD_CHOICE &word = *(word_res->best_choice);

  // Find the frequency of each unique character in the word.
  SortHelper<UNICHAR_ID> rep_ch(word.length());
  for (unsigned i = 0; i < word.length(); ++i) {
    rep_ch.Add(word.unichar_id(i), 1);
  }

  // Find the most frequent result.
  UNICHAR_ID maxch_id = INVALID_UNICHAR_ID; // most common char
  int max_count = rep_ch.MaxCount(&maxch_id);
  // Find the best exemplar of a classifier result for maxch_id.
  BLOB_CHOICE *best_choice = FindBestMatchingChoice(maxch_id, word_res);
  if (best_choice == nullptr) {
    tprintf("Failed to find a choice for %s, occurring %d times\n",
            word_res->uch_set->debug_str(maxch_id).c_str(), max_count);
    return;
  }
  word_res->done = true;

  // Just correct existing classification.
  CorrectRepcharChoices(best_choice, word_res);
  word_res->reject_map.initialise(word.length());
}

ACCEPTABLE_WERD_TYPE Tesseract::acceptable_word_string(const UNICHARSET &char_set, const char *s,
                                                       const char *lengths) {
  int i = 0;
  int offset = 0;
  int leading_punct_count;
  int upper_count = 0;
  int hyphen_pos = -1;
  ACCEPTABLE_WERD_TYPE word_type = AC_UNACCEPTABLE;

  if (strlen(lengths) > 20) {
    return word_type;
  }

  /* Single Leading punctuation char*/

  if (s[offset] != '\0' && chs_leading_punct.contains(s[offset])) {
    offset += lengths[i++];
  }
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
    if (i - leading_punct_count < quality_min_initial_alphas_reqd) {
      goto not_a_word;
    }
    /*
Allow a single hyphen in a lower case word
- don't trust upper case - I've seen several cases of "H" -> "I-I"
*/
    if (lengths[i] == 1 && s[offset] == '-') {
      hyphen_pos = i;
      offset += lengths[i++];
      if (s[offset] != '\0') {
        while ((s[offset] != '\0') && char_set.get_islower(s + offset, lengths[i])) {
          offset += lengths[i++];
        }
        if (i < hyphen_pos + 3) {
          goto not_a_word;
        }
      }
    } else {
      /* Allow "'s" in NON hyphenated lower case words */
      if (lengths[i] == 1 && (s[offset] == '\'') && lengths[i + 1] == 1 &&
          (s[offset + lengths[i]] == 's')) {
        offset += lengths[i++];
        offset += lengths[i++];
      }
    }
    if (upper_count > 0) {
      word_type = AC_INITIAL_CAP;
    } else {
      word_type = AC_LOWER_CASE;
    }
  }

  /* Up to two different, constrained trailing punctuation chars */
  if (lengths[i] == 1 && s[offset] != '\0' && chs_trailing_punct1.contains(s[offset])) {
    offset += lengths[i++];
  }
  if (lengths[i] == 1 && s[offset] != '\0' && i > 0 && s[offset - lengths[i - 1]] != s[offset] &&
      chs_trailing_punct2.contains(s[offset])) {
    offset += lengths[i++];
  }

  if (s[offset] != '\0') {
    word_type = AC_UNACCEPTABLE;
  }

not_a_word:

  if (word_type == AC_UNACCEPTABLE) {
    /* Look for abbreviation string */
    i = 0;
    offset = 0;
    if (s[0] != '\0' && char_set.get_isupper(s, lengths[0])) {
      word_type = AC_UC_ABBREV;
      while (s[offset] != '\0' && char_set.get_isupper(s + offset, lengths[i]) &&
             lengths[i + 1] == 1 && s[offset + lengths[i]] == '.') {
        offset += lengths[i++];
        offset += lengths[i++];
      }
    } else if (s[0] != '\0' && char_set.get_islower(s, lengths[0])) {
      word_type = AC_LC_ABBREV;
      while (s[offset] != '\0' && char_set.get_islower(s + offset, lengths[i]) &&
             lengths[i + 1] == 1 && s[offset + lengths[i]] == '.') {
        offset += lengths[i++];
        offset += lengths[i++];
      }
    }
    if (s[offset] != '\0') {
      word_type = AC_UNACCEPTABLE;
    }
  }

  return word_type;
}

bool Tesseract::check_debug_pt(WERD_RES *word, int location) {
  if (!test_pt) {
    return false;
  }

  tessedit_rejection_debug.set_value(false);
  debug_x_ht_level.set_value(0);

  if (word->word->bounding_box().contains(FCOORD(test_pt_x, test_pt_y))) {
    if (location < 0) {
      return true; // For breakpoint use
    }
    bool show_map_detail = false;
    tessedit_rejection_debug.set_value(true);
    debug_x_ht_level.set_value(2);
    tprintf("\n\nTESTWD::");
    switch (location) {
      case 0:
        tprintf("classify_word_pass1 start\n");
        word->word->print();
        break;
      case 10:
        tprintf("make_reject_map: initial map");
        break;
      case 20:
        tprintf("make_reject_map: after NN");
        break;
      case 30:
        tprintf("classify_word_pass2 - START");
        break;
      case 40:
        tprintf("classify_word_pass2 - Pre Xht");
        break;
      case 50:
        tprintf("classify_word_pass2 - END");
        show_map_detail = true;
        break;
      case 60:
        tprintf("fixspace");
        break;
      case 70:
        tprintf("MM pass START");
        break;
      case 80:
        tprintf("MM pass END");
        break;
      case 90:
        tprintf("After Poor quality rejection");
        break;
      case 100:
        tprintf("unrej_good_quality_words - START");
        break;
      case 110:
        tprintf("unrej_good_quality_words - END");
        break;
      case 120:
        tprintf("Write results pass");
        show_map_detail = true;
        break;
    }
    if (word->best_choice != nullptr) {
      tprintf(" \"%s\" ", word->best_choice->unichar_string().c_str());
      word->reject_map.print(debug_fp);
      tprintf("\n");
      if (show_map_detail) {
        tprintf("\"%s\"\n", word->best_choice->unichar_string().c_str());
        for (unsigned i = 0; word->best_choice->unichar_string()[i] != '\0'; i++) {
          tprintf("**** \"%c\" ****\n", word->best_choice->unichar_string()[i]);
          word->reject_map[i].full_print(debug_fp);
        }
      }
    } else {
      tprintf("null best choice\n");
    }
    tprintf("Tess Accepted: %s\n", word->tess_accepted ? "TRUE" : "FALSE");
    tprintf("Done flag: %s\n\n", word->done ? "TRUE" : "FALSE");
    return true;
  } else {
    return false;
  }
}

/**
 * find_modal_font
 *
 * Find the modal font and remove from the stats.
 */
#ifndef DISABLED_LEGACY_ENGINE
static void find_modal_font( // good chars in word
    STATS *fonts,            // font stats
    int16_t *font_out,       // output font
    int8_t *font_count       // output count
) {
  if (fonts->get_total() > 0) {
    // font index
    int16_t font = static_cast<int16_t>(fonts->mode());
    *font_out = font;
    // pile count
    int32_t count = fonts->pile_count(font);
    *font_count = count < INT8_MAX ? count : INT8_MAX;
    fonts->add(font, -*font_count);
  } else {
    *font_out = -1;
    *font_count = 0;
  }
}
#endif // ! DISABLED_LEGACY_ENGINE

/**
 * set_word_fonts
 *
 * Get the fonts for the word.
 */
void Tesseract::set_word_fonts(WERD_RES *word) {
  // Don't try to set the word fonts for an lstm word, as the configs
  // will be meaningless.
  if (word->chopped_word == nullptr) {
    return;
  }
  ASSERT_HOST(word->best_choice != nullptr);

#ifndef DISABLED_LEGACY_ENGINE
  const int fontinfo_size = fontinfo_table_.size();
  if (fontinfo_size == 0) {
    return;
  }
  if (tessedit_font_id > 0) {
    if (tessedit_font_id >= fontinfo_size) {
      tprintf("Error, invalid font ID provided: must be below %d.\n"
              "Falling back to font auto-detection.\n", fontinfo_size);
    } else {
      word->fontinfo = &fontinfo_table_.at(tessedit_font_id);
      word->fontinfo2 = nullptr;
      word->fontinfo_id_count = INT8_MAX;
      word->fontinfo_id2_count = 0;
      return;
    }
  }
  std::vector<int> font_total_score(fontinfo_size);

  // Compute the font scores for the word
  if (tessedit_debug_fonts) {
    tprintf("Examining fonts in %s\n", word->best_choice->debug_string().c_str());
  }
  for (unsigned b = 0; b < word->best_choice->length(); ++b) {
    const BLOB_CHOICE *choice = word->GetBlobChoice(b);
    if (choice == nullptr) {
      continue;
    }
    auto &fonts = choice->fonts();
    for (auto &f : fonts) {
      const int fontinfo_id = f.fontinfo_id;
      if (0 <= fontinfo_id && fontinfo_id < fontinfo_size) {
        font_total_score[fontinfo_id] += f.score;
      }
    }
  }
  // Find the top and 2nd choice for the word.
  int score1 = 0, score2 = 0;
  int16_t font_id1 = -1, font_id2 = -1;
  for (int f = 0; f < fontinfo_size; ++f) {
    if (tessedit_debug_fonts && font_total_score[f] > 0) {
      tprintf("Font %s, total score = %d\n", fontinfo_table_.at(f).name, font_total_score[f]);
    }
    if (font_total_score[f] > score1) {
      score2 = score1;
      font_id2 = font_id1;
      score1 = font_total_score[f];
      font_id1 = f;
    } else if (font_total_score[f] > score2) {
      score2 = font_total_score[f];
      font_id2 = f;
    }
  }
  word->fontinfo = font_id1 >= 0 ? &fontinfo_table_.at(font_id1) : nullptr;
  word->fontinfo2 = font_id2 >= 0 ? &fontinfo_table_.at(font_id2) : nullptr;
  // Each score has a limit of UINT16_MAX, so divide by that to get the number
  // of "votes" for that font, ie number of perfect scores.
  word->fontinfo_id_count = ClipToRange<int>(score1 / UINT16_MAX, 1, INT8_MAX);
  word->fontinfo_id2_count = ClipToRange<int>(score2 / UINT16_MAX, 0, INT8_MAX);
  if (score1 > 0) {
    const FontInfo fi = fontinfo_table_.at(font_id1);
    if (tessedit_debug_fonts) {
      if (word->fontinfo_id2_count > 0 && font_id2 >= 0) {
        tprintf("Word modal font=%s, score=%d, 2nd choice %s/%d\n", fi.name,
                word->fontinfo_id_count, fontinfo_table_.at(font_id2).name,
                word->fontinfo_id2_count);
      } else {
        tprintf("Word modal font=%s, score=%d. No 2nd choice\n", fi.name, word->fontinfo_id_count);
      }
    }
  }
#endif // ndef DISABLED_LEGACY_ENGINE
}

#ifndef DISABLED_LEGACY_ENGINE
/**
 * font_recognition_pass
 *
 * Smooth the fonts for the document.
 */
void Tesseract::font_recognition_pass(PAGE_RES *page_res) {
  PAGE_RES_IT page_res_it(page_res);
  WERD_RES *word;                       // current word
  STATS doc_fonts(0, font_table_size_ - 1); // font counters

  // Gather font id statistics.
  for (page_res_it.restart_page(); page_res_it.word() != nullptr; page_res_it.forward()) {
    word = page_res_it.word();
    if (word->fontinfo != nullptr) {
      doc_fonts.add(word->fontinfo->universal_id, word->fontinfo_id_count);
    }
    if (word->fontinfo2 != nullptr) {
      doc_fonts.add(word->fontinfo2->universal_id, word->fontinfo_id2_count);
    }
  }
  int16_t doc_font;      // modal font
  int8_t doc_font_count; // modal font
  find_modal_font(&doc_fonts, &doc_font, &doc_font_count);
  if (doc_font_count == 0) {
    return;
  }
  // Get the modal font pointer.
  const FontInfo *modal_font = nullptr;
  for (page_res_it.restart_page(); page_res_it.word() != nullptr; page_res_it.forward()) {
    word = page_res_it.word();
    if (word->fontinfo != nullptr && word->fontinfo->universal_id == doc_font) {
      modal_font = word->fontinfo;
      break;
    }
    if (word->fontinfo2 != nullptr && word->fontinfo2->universal_id == doc_font) {
      modal_font = word->fontinfo2;
      break;
    }
  }
  ASSERT_HOST(modal_font != nullptr);

  // Assign modal font to weak words.
  for (page_res_it.restart_page(); page_res_it.word() != nullptr; page_res_it.forward()) {
    word = page_res_it.word();
    const int length = word->best_choice->length();

    const int count = word->fontinfo_id_count;
    if (!(count == length || (length > 3 && count >= length * 3 / 4))) {
      word->fontinfo = modal_font;
      // Counts only get 1 as it came from the doc.
      word->fontinfo_id_count = 1;
    }
  }
}
#endif // ndef DISABLED_LEGACY_ENGINE

// If a word has multiple alternates check if the best choice is in the
// dictionary. If not, replace it with an alternate that exists in the
// dictionary.
void Tesseract::dictionary_correction_pass(PAGE_RES *page_res) {
  PAGE_RES_IT word_it(page_res);
  for (WERD_RES *word = word_it.word(); word != nullptr; word = word_it.forward()) {
    if (word->best_choices.singleton()) {
      continue; // There are no alternates.
    }

    const WERD_CHOICE *best = word->best_choice;
    if (word->tesseract->getDict().valid_word(*best) != 0) {
      continue; // The best choice is in the dictionary.
    }

    WERD_CHOICE_IT choice_it(&word->best_choices);
    for (choice_it.mark_cycle_pt(); !choice_it.cycled_list(); choice_it.forward()) {
      WERD_CHOICE *alternate = choice_it.data();
      if (word->tesseract->getDict().valid_word(*alternate)) {
        // The alternate choice is in the dictionary.
        if (tessedit_bigram_debug) {
          tprintf("Dictionary correction replaces best choice '%s' with '%s'\n",
                  best->unichar_string().c_str(), alternate->unichar_string().c_str());
        }
        // Replace the 'best' choice with a better choice.
        word->ReplaceBestChoice(alternate);
        break;
      }
    }
  }
}

} // namespace tesseract
