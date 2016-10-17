/******************************************************************
 * File:        fixspace.cpp  (Formerly fixspace.c)
 * Description: Implements a pass over the page res, exploring the alternative
 *              spacing possibilities, trying to use context to improve the
 *              word spacing
* Author:		Phil Cheatle
* Created:		Thu Oct 21 11:38:43 BST 1993
*
* (C) Copyright 1993, Hewlett-Packard Ltd.
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

#include <ctype.h>
#include "reject.h"
#include "statistc.h"
#include "control.h"
#include "fixspace.h"
#include "genblob.h"
#include "tessvars.h"
#include "tessbox.h"
#include "globals.h"
#include "tesseractclass.h"

#define PERFECT_WERDS   999
#define MAXSPACING      128      /*max expected spacing in pix */

namespace tesseract {

/**
 * @name fix_fuzzy_spaces()
 * Walk over the page finding sequences of words joined by fuzzy spaces. Extract
 * them as a sublist, process the sublist to find the optimal arrangement of
 * spaces then replace the sublist in the ROW_RES.
 *
 * @param monitor progress monitor
 * @param word_count count of words in doc
 * @param[out] page_res
 */
void Tesseract::fix_fuzzy_spaces(ETEXT_DESC *monitor,
                                 inT32 word_count,
                                 PAGE_RES *page_res) {
  BLOCK_RES_IT block_res_it;
  ROW_RES_IT row_res_it;
  WERD_RES_IT word_res_it_from;
  WERD_RES_IT word_res_it_to;
  WERD_RES *word_res;
  WERD_RES_LIST fuzzy_space_words;
  inT16 new_length;
  BOOL8 prevent_null_wd_fixsp;   // DON'T process blobless wds
  inT32 word_index;              // current word

  block_res_it.set_to_list(&page_res->block_res_list);
  word_index = 0;
  for (block_res_it.mark_cycle_pt(); !block_res_it.cycled_list();
       block_res_it.forward()) {
    row_res_it.set_to_list(&block_res_it.data()->row_res_list);
    for (row_res_it.mark_cycle_pt(); !row_res_it.cycled_list();
         row_res_it.forward()) {
      word_res_it_from.set_to_list(&row_res_it.data()->word_res_list);
      while (!word_res_it_from.at_last()) {
        word_res = word_res_it_from.data();
        while (!word_res_it_from.at_last() &&
               !(word_res->combination ||
                 word_res_it_from.data_relative(1)->word->flag(W_FUZZY_NON) ||
                 word_res_it_from.data_relative(1)->word->flag(W_FUZZY_SP))) {
          fix_sp_fp_word(word_res_it_from, row_res_it.data()->row,
                         block_res_it.data()->block);
          word_res = word_res_it_from.forward();
          word_index++;
          if (monitor != NULL) {
            monitor->ocr_alive = TRUE;
            monitor->progress = 90 + 5 * word_index / word_count;
            if (monitor->deadline_exceeded() ||
                (monitor->cancel != NULL &&
                 (*monitor->cancel)(monitor->cancel_this, stats_.dict_words)))
            return;
          }
        }

        if (!word_res_it_from.at_last()) {
          word_res_it_to = word_res_it_from;
          prevent_null_wd_fixsp =
            word_res->word->cblob_list()->empty();
          if (check_debug_pt(word_res, 60))
            debug_fix_space_level.set_value(10);
          word_res_it_to.forward();
          word_index++;
          if (monitor != NULL) {
            monitor->ocr_alive = TRUE;
            monitor->progress = 90 + 5 * word_index / word_count;
            if (monitor->deadline_exceeded() ||
                (monitor->cancel != NULL &&
                 (*monitor->cancel)(monitor->cancel_this, stats_.dict_words)))
            return;
          }
          while (!word_res_it_to.at_last () &&
                 (word_res_it_to.data_relative(1)->word->flag(W_FUZZY_NON) ||
                  word_res_it_to.data_relative(1)->word->flag(W_FUZZY_SP))) {
            if (check_debug_pt(word_res, 60))
              debug_fix_space_level.set_value(10);
            if (word_res->word->cblob_list()->empty())
              prevent_null_wd_fixsp = TRUE;
            word_res = word_res_it_to.forward();
          }
          if (check_debug_pt(word_res, 60))
            debug_fix_space_level.set_value(10);
          if (word_res->word->cblob_list()->empty())
            prevent_null_wd_fixsp = TRUE;
          if (prevent_null_wd_fixsp) {
            word_res_it_from = word_res_it_to;
          } else {
            fuzzy_space_words.assign_to_sublist(&word_res_it_from,
                                                &word_res_it_to);
            fix_fuzzy_space_list(fuzzy_space_words,
                                 row_res_it.data()->row,
                                 block_res_it.data()->block);
            new_length = fuzzy_space_words.length();
            word_res_it_from.add_list_before(&fuzzy_space_words);
            for (;
                 !word_res_it_from.at_last() && new_length > 0;
                 new_length--) {
              word_res_it_from.forward();
            }
          }
          if (test_pt)
            debug_fix_space_level.set_value(0);
        }
        fix_sp_fp_word(word_res_it_from, row_res_it.data()->row,
                       block_res_it.data()->block);
        // Last word in row
      }
    }
  }
}

void Tesseract::fix_fuzzy_space_list(WERD_RES_LIST &best_perm,
                                     ROW *row,
                                     BLOCK* block) {
  inT16 best_score;
  WERD_RES_LIST current_perm;
  inT16 current_score;
  BOOL8 improved = FALSE;

  best_score = eval_word_spacing(best_perm);  // default score
  dump_words(best_perm, best_score, 1, improved);

  if (best_score != PERFECT_WERDS)
    initialise_search(best_perm, current_perm);

  while ((best_score != PERFECT_WERDS) && !current_perm.empty()) {
    match_current_words(current_perm, row, block);
    current_score = eval_word_spacing(current_perm);
    dump_words(current_perm, current_score, 2, improved);
    if (current_score > best_score) {
      best_perm.clear();
      best_perm.deep_copy(&current_perm, &WERD_RES::deep_copy);
      best_score = current_score;
      improved = TRUE;
    }
    if (current_score < PERFECT_WERDS)
      transform_to_next_perm(current_perm);
  }
  dump_words(best_perm, best_score, 3, improved);
}

}  // namespace tesseract

void initialise_search(WERD_RES_LIST &src_list, WERD_RES_LIST &new_list) {
  WERD_RES_IT src_it(&src_list);
  WERD_RES_IT new_it(&new_list);
  WERD_RES *src_wd;
  WERD_RES *new_wd;

  for (src_it.mark_cycle_pt(); !src_it.cycled_list(); src_it.forward()) {
    src_wd = src_it.data();
    if (!src_wd->combination) {
      new_wd = WERD_RES::deep_copy(src_wd);
      new_wd->combination = FALSE;
      new_wd->part_of_combo = FALSE;
      new_it.add_after_then_move(new_wd);
    }
  }
}


namespace tesseract {
void Tesseract::match_current_words(WERD_RES_LIST &words, ROW *row,
                                    BLOCK* block) {
  WERD_RES_IT word_it(&words);
  WERD_RES *word;
  // Since we are not using PAGE_RES to iterate over words, we need to update
  // prev_word_best_choice_ before calling classify_word_pass2().
  prev_word_best_choice_ = NULL;
  for (word_it.mark_cycle_pt(); !word_it.cycled_list(); word_it.forward()) {
    word = word_it.data();
    if ((!word->part_of_combo) && (word->box_word == NULL)) {
      WordData word_data(block, row, word);
      SetupWordPassN(2, &word_data);
      classify_word_and_language(2, NULL, &word_data);
    }
    prev_word_best_choice_ = word->best_choice;
  }
}


/**
 * @name eval_word_spacing()
 * The basic measure is the number of characters in contextually confirmed
 * words. (I.e the word is done)
 * If all words are contextually confirmed the evaluation is deemed perfect.
 *
 * Some fiddles are done to handle "1"s as these are VERY frequent causes of
 * fuzzy spaces. The problem with the basic measure is that "561 63" would score
 * the same as "56163", though given our knowledge that the space is fuzzy, and
 * that there is a "1" next to the fuzzy space, we need to ensure that "56163"
 * is preferred.
 *
 * The solution is to NOT COUNT the score of any word which has a digit at one
 * end and a "1Il" as the character the other side of the space.
 *
 * Conversly, any character next to a "1" within a word is counted as a positive
 * score. Thus "561 63" would score 4 (3 chars in a numeric word plus 1 side of
 * the "1" joined).  "56163" would score 7 - all chars in a numeric word + 2
 * sides of a "1" joined.
 *
 * The joined 1 rule is applied to any word REGARDLESS of contextual
 * confirmation.  Thus "PS7a71 3/7a" scores 1 (neither word is contexutally
 * confirmed. The only score is from the joined 1. "PS7a713/7a" scores 2.
 *
 */
inT16 Tesseract::eval_word_spacing(WERD_RES_LIST &word_res_list) {
  WERD_RES_IT word_res_it(&word_res_list);
  inT16 total_score = 0;
  inT16 word_count = 0;
  inT16 done_word_count = 0;
  inT16 word_len;
  inT16 i;
  inT16 offset;
  WERD_RES *word;                 // current word
  inT16 prev_word_score = 0;
  BOOL8 prev_word_done = FALSE;
  BOOL8 prev_char_1 = FALSE;      // prev ch a "1/I/l"?
  BOOL8 prev_char_digit = FALSE;  // prev ch 2..9 or 0
  BOOL8 current_char_1 = FALSE;
  BOOL8 current_word_ok_so_far;
  STRING punct_chars = "!\"`',.:;";
  BOOL8 prev_char_punct = FALSE;
  BOOL8 current_char_punct = FALSE;
  BOOL8 word_done = FALSE;

  do {
    word = word_res_it.data();
    word_done = fixspace_thinks_word_done(word);
    word_count++;
    if (word->tess_failed) {
      total_score += prev_word_score;
      if (prev_word_done)
        done_word_count++;
      prev_word_score = 0;
      prev_char_1 = FALSE;
      prev_char_digit = FALSE;
      prev_word_done = FALSE;
    } else {
      /*
        Can we add the prev word score and potentially count this word?
        Yes IF it didn't end in a 1 when the first char of this word is a digit
          AND it didn't end in a digit when the first char of this word is a 1
      */
      word_len = word->reject_map.length();
      current_word_ok_so_far = FALSE;
      if (!((prev_char_1 && digit_or_numeric_punct(word, 0)) ||
            (prev_char_digit && (
                (word_done &&
                 word->best_choice->unichar_lengths().string()[0] == 1 &&
                 word->best_choice->unichar_string()[0] == '1') ||
                (!word_done && STRING(conflict_set_I_l_1).contains(
                      word->best_choice->unichar_string()[0])))))) {
        total_score += prev_word_score;
        if (prev_word_done)
          done_word_count++;
        current_word_ok_so_far = word_done;
      }

      if (current_word_ok_so_far) {
        prev_word_done = TRUE;
        prev_word_score = word_len;
      } else {
        prev_word_done = FALSE;
        prev_word_score = 0;
      }

      /* Add 1 to total score for every joined 1 regardless of context and
         rejtn */
      for (i = 0, prev_char_1 = FALSE; i < word_len; i++) {
        current_char_1 = word->best_choice->unichar_string()[i] == '1';
        if (prev_char_1 || (current_char_1 && (i > 0)))
          total_score++;
        prev_char_1 = current_char_1;
      }

      /* Add 1 to total score for every joined punctuation regardless of context
        and rejtn */
      if (tessedit_prefer_joined_punct) {
        for (i = 0, offset = 0, prev_char_punct = FALSE; i < word_len;
             offset += word->best_choice->unichar_lengths()[i++]) {
          current_char_punct =
            punct_chars.contains(word->best_choice->unichar_string()[offset]);
          if (prev_char_punct || (current_char_punct && i > 0))
            total_score++;
          prev_char_punct = current_char_punct;
        }
      }
      prev_char_digit = digit_or_numeric_punct(word, word_len - 1);
      for (i = 0, offset = 0; i < word_len - 1;
           offset += word->best_choice->unichar_lengths()[i++]);
      prev_char_1 =
          ((word_done && (word->best_choice->unichar_string()[offset] == '1'))
           || (!word_done && STRING(conflict_set_I_l_1).contains(
                   word->best_choice->unichar_string()[offset])));
    }
    /* Find next word */
    do {
      word_res_it.forward();
    } while (word_res_it.data()->part_of_combo);
  } while (!word_res_it.at_first());
  total_score += prev_word_score;
  if (prev_word_done)
    done_word_count++;
  if (done_word_count == word_count)
    return PERFECT_WERDS;
  else
    return total_score;
}

BOOL8 Tesseract::digit_or_numeric_punct(WERD_RES *word, int char_position) {
  int i;
  int offset;

  for (i = 0, offset = 0; i < char_position;
       offset += word->best_choice->unichar_lengths()[i++]);
  return (
      word->uch_set->get_isdigit(
          word->best_choice->unichar_string().string() + offset,
          word->best_choice->unichar_lengths()[i]) ||
      (word->best_choice->permuter() == NUMBER_PERM &&
       STRING(numeric_punctuation).contains(
           word->best_choice->unichar_string().string()[offset])));
}

}  // namespace tesseract


/**
 * @name transform_to_next_perm()
 * Examines the current word list to find the smallest word gap size. Then walks
 * the word list closing any gaps of this size by either inserted new
 * combination words, or extending existing ones.
 *
 * The routine COULD be limited to stop it building words longer than N blobs.
 *
 * If there are no more gaps then it DELETES the entire list and returns the
 * empty list to cause termination.
 */
void transform_to_next_perm(WERD_RES_LIST &words) {
  WERD_RES_IT word_it(&words);
  WERD_RES_IT prev_word_it(&words);
  WERD_RES *word;
  WERD_RES *prev_word;
  WERD_RES *combo;
  WERD *copy_word;
  inT16 prev_right = -MAX_INT16;
  TBOX box;
  inT16 gap;
  inT16 min_gap = MAX_INT16;

  for (word_it.mark_cycle_pt(); !word_it.cycled_list(); word_it.forward()) {
    word = word_it.data();
    if (!word->part_of_combo) {
      box = word->word->bounding_box();
      if (prev_right > -MAX_INT16) {
        gap = box.left() - prev_right;
        if (gap < min_gap)
          min_gap = gap;
      }
      prev_right = box.right();
    }
  }
  if (min_gap < MAX_INT16) {
    prev_right = -MAX_INT16;        // back to start
    word_it.set_to_list(&words);
    // Note: we can't use cycle_pt due to inserted combos at start of list.
    for (; (prev_right == -MAX_INT16) || !word_it.at_first();
         word_it.forward()) {
      word = word_it.data();
      if (!word->part_of_combo) {
        box = word->word->bounding_box();
        if (prev_right > -MAX_INT16) {
          gap = box.left() - prev_right;
          if (gap <= min_gap) {
            prev_word = prev_word_it.data();
            if (prev_word->combination) {
              combo = prev_word;
            } else {
              /* Make a new combination and insert before
               * the first word being joined. */
              copy_word = new WERD;
              *copy_word = *(prev_word->word);
              // deep copy
              combo = new WERD_RES(copy_word);
              combo->combination = TRUE;
              combo->x_height = prev_word->x_height;
              prev_word->part_of_combo = TRUE;
              prev_word_it.add_before_then_move(combo);
            }
            combo->word->set_flag(W_EOL, word->word->flag(W_EOL));
            if (word->combination) {
              combo->word->join_on(word->word);
              // Move blobs to combo
              // old combo no longer needed
              delete word_it.extract();
            } else {
              // Copy current wd to combo
              combo->copy_on(word);
              word->part_of_combo = TRUE;
            }
            combo->done = FALSE;
            combo->ClearResults();
          } else {
            prev_word_it = word_it;  // catch up
          }
        }
        prev_right = box.right();
      }
    }
  } else {
    words.clear();  // signal termination
  }
}

namespace tesseract {
void Tesseract::dump_words(WERD_RES_LIST &perm, inT16 score,
                           inT16 mode, BOOL8 improved) {
  WERD_RES_IT word_res_it(&perm);

  if (debug_fix_space_level > 0) {
    if (mode == 1) {
      stats_.dump_words_str = "";
      for (word_res_it.mark_cycle_pt(); !word_res_it.cycled_list();
           word_res_it.forward()) {
        if (!word_res_it.data()->part_of_combo) {
          stats_.dump_words_str +=
              word_res_it.data()->best_choice->unichar_string();
          stats_.dump_words_str += ' ';
        }
      }
    }

    if (debug_fix_space_level > 1) {
      switch (mode) {
        case 1:
          tprintf("EXTRACTED (%d): \"", score);
          break;
        case 2:
          tprintf("TESTED (%d): \"", score);
          break;
        case 3:
          tprintf("RETURNED (%d): \"", score);
          break;
      }

      for (word_res_it.mark_cycle_pt(); !word_res_it.cycled_list();
           word_res_it.forward()) {
        if (!word_res_it.data()->part_of_combo) {
          tprintf("%s/%1d ",
                  word_res_it.data()->best_choice->unichar_string().string(),
                  (int)word_res_it.data()->best_choice->permuter());
        }
      }
      tprintf("\"\n");
    } else if (improved) {
      tprintf("FIX SPACING \"%s\" => \"", stats_.dump_words_str.string());
      for (word_res_it.mark_cycle_pt(); !word_res_it.cycled_list();
           word_res_it.forward()) {
        if (!word_res_it.data()->part_of_combo) {
          tprintf("%s/%1d ",
                  word_res_it.data()->best_choice->unichar_string().string(),
                  (int)word_res_it.data()->best_choice->permuter());
        }
      }
      tprintf("\"\n");
    }
  }
}

BOOL8 Tesseract::fixspace_thinks_word_done(WERD_RES *word) {
  if (word->done)
    return TRUE;

  /*
    Use all the standard pass 2 conditions for mode 5 in set_done() in
    reject.c BUT DON'T REJECT IF THE WERD IS AMBIGUOUS - FOR SPACING WE DON'T
    CARE WHETHER WE HAVE of/at on/an etc.
  */
  if (fixsp_done_mode > 0 &&
      (word->tess_accepted ||
       (fixsp_done_mode == 2 && word->reject_map.reject_count() == 0) ||
       fixsp_done_mode == 3) &&
      (strchr(word->best_choice->unichar_string().string(), ' ') == NULL) &&
      ((word->best_choice->permuter() == SYSTEM_DAWG_PERM) ||
       (word->best_choice->permuter() == FREQ_DAWG_PERM) ||
       (word->best_choice->permuter() == USER_DAWG_PERM) ||
       (word->best_choice->permuter() == NUMBER_PERM))) {
    return TRUE;
  } else {
    return FALSE;
  }
}


/**
 * @name fix_sp_fp_word()
 * Test the current word to see if it can be split by deleting noise blobs. If
 * so, do the business.
 * Return with the iterator pointing to the same place if the word is unchanged,
 * or the last of the replacement words.
 */
void Tesseract::fix_sp_fp_word(WERD_RES_IT &word_res_it, ROW *row,
                               BLOCK* block) {
  WERD_RES *word_res;
  WERD_RES_LIST sub_word_list;
  WERD_RES_IT sub_word_list_it(&sub_word_list);
  inT16 blob_index;
  inT16 new_length;
  float junk;

  word_res = word_res_it.data();
  if (word_res->word->flag(W_REP_CHAR) ||
      word_res->combination ||
      word_res->part_of_combo ||
      !word_res->word->flag(W_DONT_CHOP))
    return;

  blob_index = worst_noise_blob(word_res, &junk);
  if (blob_index < 0)
    return;

  if (debug_fix_space_level > 1) {
    tprintf("FP fixspace working on \"%s\"\n",
            word_res->best_choice->unichar_string().string());
  }
  word_res->word->rej_cblob_list()->sort(c_blob_comparator);
  sub_word_list_it.add_after_stay_put(word_res_it.extract());
  fix_noisy_space_list(sub_word_list, row, block);
  new_length = sub_word_list.length();
  word_res_it.add_list_before(&sub_word_list);
  for (; !word_res_it.at_last() && new_length > 1; new_length--) {
    word_res_it.forward();
  }
}

void Tesseract::fix_noisy_space_list(WERD_RES_LIST &best_perm, ROW *row,
                                     BLOCK* block) {
  inT16 best_score;
  WERD_RES_IT best_perm_it(&best_perm);
  WERD_RES_LIST current_perm;
  WERD_RES_IT current_perm_it(&current_perm);
  WERD_RES *old_word_res;
  inT16 current_score;
  BOOL8 improved = FALSE;

  best_score = fp_eval_word_spacing(best_perm);  // default score

  dump_words(best_perm, best_score, 1, improved);

  old_word_res = best_perm_it.data();
  // Even deep_copy doesn't copy the underlying WERD unless its combination
  // flag is true!.
  old_word_res->combination = TRUE;   // Kludge to force deep copy
  current_perm_it.add_to_end(WERD_RES::deep_copy(old_word_res));
  old_word_res->combination = FALSE;  // Undo kludge

  break_noisiest_blob_word(current_perm);

  while (best_score != PERFECT_WERDS && !current_perm.empty()) {
    match_current_words(current_perm, row, block);
    current_score = fp_eval_word_spacing(current_perm);
    dump_words(current_perm, current_score, 2, improved);
    if (current_score > best_score) {
      best_perm.clear();
      best_perm.deep_copy(&current_perm, &WERD_RES::deep_copy);
      best_score = current_score;
      improved = TRUE;
    }
    if (current_score < PERFECT_WERDS) {
      break_noisiest_blob_word(current_perm);
    }
  }
  dump_words(best_perm, best_score, 3, improved);
}


/**
 * break_noisiest_blob_word()
 * Find the word with the blob which looks like the worst noise.
 * Break the word into two, deleting the noise blob.
 */
void Tesseract::break_noisiest_blob_word(WERD_RES_LIST &words) {
  WERD_RES_IT word_it(&words);
  WERD_RES_IT worst_word_it;
  float worst_noise_score = 9999;
  int worst_blob_index = -1;     // Noisiest blob of noisiest wd
  int blob_index;                // of wds noisiest blob
  float noise_score;             // of wds noisiest blob
  WERD_RES *word_res;
  C_BLOB_IT blob_it;
  C_BLOB_IT rej_cblob_it;
  C_BLOB_LIST new_blob_list;
  C_BLOB_IT new_blob_it;
  C_BLOB_IT new_rej_cblob_it;
  WERD *new_word;
  inT16 start_of_noise_blob;
  inT16 i;

  for (word_it.mark_cycle_pt(); !word_it.cycled_list(); word_it.forward()) {
    blob_index = worst_noise_blob(word_it.data(), &noise_score);
    if (blob_index > -1 && worst_noise_score > noise_score) {
      worst_noise_score = noise_score;
      worst_blob_index = blob_index;
      worst_word_it = word_it;
    }
  }
  if (worst_blob_index < 0) {
    words.clear();          // signal termination
    return;
  }

  /* Now split the worst_word_it */

  word_res = worst_word_it.data();

  /* Move blobs before noise blob to a new bloblist */

  new_blob_it.set_to_list(&new_blob_list);
  blob_it.set_to_list(word_res->word->cblob_list());
  for (i = 0; i < worst_blob_index; i++, blob_it.forward()) {
    new_blob_it.add_after_then_move(blob_it.extract());
  }
  start_of_noise_blob = blob_it.data()->bounding_box().left();
  delete blob_it.extract();     // throw out noise blob

  new_word = new WERD(&new_blob_list, word_res->word);
  new_word->set_flag(W_EOL, FALSE);
  word_res->word->set_flag(W_BOL, FALSE);
  word_res->word->set_blanks(1);  // After break

  new_rej_cblob_it.set_to_list(new_word->rej_cblob_list());
  rej_cblob_it.set_to_list(word_res->word->rej_cblob_list());
  for (;
       (!rej_cblob_it.empty() &&
        (rej_cblob_it.data()->bounding_box().left() < start_of_noise_blob));
       rej_cblob_it.forward()) {
    new_rej_cblob_it.add_after_then_move(rej_cblob_it.extract());
  }

  WERD_RES* new_word_res = new WERD_RES(new_word);
  new_word_res->combination = TRUE;
  worst_word_it.add_before_then_move(new_word_res);

  word_res->ClearResults();
}

inT16 Tesseract::worst_noise_blob(WERD_RES *word_res,
                                  float *worst_noise_score) {
  float noise_score[512];
  int i;
  int min_noise_blob;            // 1st contender
  int max_noise_blob;            // last contender
  int non_noise_count;
  int worst_noise_blob;          // Worst blob
  float small_limit = kBlnXHeight * fixsp_small_outlines_size;
  float non_noise_limit = kBlnXHeight * 0.8;

  if (word_res->rebuild_word == NULL)
    return -1;  // Can't handle cube words.

  // Normalised.
  int blob_count = word_res->box_word->length();
  ASSERT_HOST(blob_count <= 512);
  if (blob_count < 5)
    return -1;                   // too short to split

  /* Get the noise scores for all blobs */

  #ifndef SECURE_NAMES
  if (debug_fix_space_level > 5)
    tprintf("FP fixspace Noise metrics for \"%s\": ",
            word_res->best_choice->unichar_string().string());
  #endif

  for (i = 0; i < blob_count && i < word_res->rebuild_word->NumBlobs(); i++) {
    TBLOB* blob = word_res->rebuild_word->blobs[i];
    if (word_res->reject_map[i].accepted())
      noise_score[i] = non_noise_limit;
    else
      noise_score[i] = blob_noise_score(blob);

    if (debug_fix_space_level > 5)
      tprintf("%1.1f ", noise_score[i]);
  }
  if (debug_fix_space_level > 5)
    tprintf("\n");

  /* Now find the worst one which is far enough away from the end of the word */

  non_noise_count = 0;
  for (i = 0; i < blob_count && non_noise_count < fixsp_non_noise_limit; i++) {
    if (noise_score[i] >= non_noise_limit) {
      non_noise_count++;
    }
  }
  if (non_noise_count < fixsp_non_noise_limit)
    return -1;

  min_noise_blob = i;

  non_noise_count = 0;
  for (i = blob_count - 1; i >= 0 && non_noise_count < fixsp_non_noise_limit;
       i--) {
    if (noise_score[i] >= non_noise_limit) {
      non_noise_count++;
    }
  }
  if (non_noise_count < fixsp_non_noise_limit)
    return -1;

  max_noise_blob = i;

  if (min_noise_blob > max_noise_blob)
    return -1;

  *worst_noise_score = small_limit;
  worst_noise_blob = -1;
  for (i = min_noise_blob; i <= max_noise_blob; i++) {
    if (noise_score[i] < *worst_noise_score) {
      worst_noise_blob = i;
      *worst_noise_score = noise_score[i];
    }
  }
  return worst_noise_blob;
}

float Tesseract::blob_noise_score(TBLOB *blob) {
  TBOX box;                       // BB of outline
  inT16 outline_count = 0;
  inT16 max_dimension;
  inT16 largest_outline_dimension = 0;

  for (TESSLINE* ol = blob->outlines; ol != NULL; ol= ol->next) {
    outline_count++;
    box = ol->bounding_box();
    if (box.height() > box.width()) {
      max_dimension = box.height();
    } else {
      max_dimension = box.width();
    }

    if (largest_outline_dimension < max_dimension)
      largest_outline_dimension = max_dimension;
  }

  if (outline_count > 5) {
    // penalise LOTS of blobs
    largest_outline_dimension *= 2;
  }

  box = blob->bounding_box();
  if (box.bottom() > kBlnBaselineOffset * 4 ||
      box.top() < kBlnBaselineOffset / 2) {
    // Lax blob is if high or low
    largest_outline_dimension /= 2;
  }

  return largest_outline_dimension;
}
}  // namespace tesseract

void fixspace_dbg(WERD_RES *word) {
  TBOX box = word->word->bounding_box();
  BOOL8 show_map_detail = FALSE;
  inT16 i;

  box.print();
  tprintf(" \"%s\" ", word->best_choice->unichar_string().string());
  tprintf("Blob count: %d (word); %d/%d (rebuild word)\n",
          word->word->cblob_list()->length(),
          word->rebuild_word->NumBlobs(),
          word->box_word->length());
  word->reject_map.print(debug_fp);
  tprintf("\n");
  if (show_map_detail) {
    tprintf("\"%s\"\n", word->best_choice->unichar_string().string());
    for (i = 0; word->best_choice->unichar_string()[i] != '\0'; i++) {
      tprintf("**** \"%c\" ****\n", word->best_choice->unichar_string()[i]);
      word->reject_map[i].full_print(debug_fp);
    }
  }

  tprintf("Tess Accepted: %s\n", word->tess_accepted ? "TRUE" : "FALSE");
  tprintf("Done flag: %s\n\n", word->done ? "TRUE" : "FALSE");
}


/**
 * fp_eval_word_spacing()
 * Evaluation function for fixed pitch word lists.
 *
 * Basically, count the number of "nice" characters - those which are in tess
 * acceptable words or in dict words and are not rejected.
 * Penalise any potential noise chars
 */
namespace tesseract {
inT16 Tesseract::fp_eval_word_spacing(WERD_RES_LIST &word_res_list) {
  WERD_RES_IT word_it(&word_res_list);
  WERD_RES *word;
  inT16 score = 0;
  inT16 i;
  float small_limit = kBlnXHeight * fixsp_small_outlines_size;

  for (word_it.mark_cycle_pt(); !word_it.cycled_list(); word_it.forward()) {
    word = word_it.data();
    if (word->rebuild_word == NULL)
      continue;  // Can't handle cube words.
    if (word->done ||
        word->tess_accepted ||
        word->best_choice->permuter() == SYSTEM_DAWG_PERM ||
        word->best_choice->permuter() == FREQ_DAWG_PERM ||
        word->best_choice->permuter() == USER_DAWG_PERM ||
        safe_dict_word(word) > 0) {
      int num_blobs = word->rebuild_word->NumBlobs();
      UNICHAR_ID space = word->uch_set->unichar_to_id(" ");
      for (i = 0; i < word->best_choice->length() && i < num_blobs; ++i) {
        TBLOB* blob = word->rebuild_word->blobs[i];
        if (word->best_choice->unichar_id(i) == space ||
            blob_noise_score(blob) < small_limit) {
          score -= 1;  // penalise possibly erroneous non-space
        } else if (word->reject_map[i].accepted()) {
          score++;
        }
      }
    }
  }
  if (score < 0)
    score = 0;
  return score;
}

}  // namespace tesseract
