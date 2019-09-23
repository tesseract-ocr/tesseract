/**********************************************************************
 * File:        tfacepp.cpp  (Formerly tface++.c)
 * Description: C++ side of the C/C++ Tess/Editor interface.
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

#include <cmath>

#include "blamer.h"
#include "errcode.h"
#include "ratngs.h"
#include "reject.h"
#include "tesseractclass.h"
#include "werd.h"

#define MAX_UNDIVIDED_LENGTH 24



/**********************************************************************
 * recog_word
 *
 * Convert the word to tess form and pass it to the tess segmenter.
 * Convert the output back to editor form.
 **********************************************************************/
namespace tesseract {
void Tesseract::recog_word(WERD_RES *word) {
  if (wordrec_skip_no_truth_words && (word->blamer_bundle == nullptr ||
      word->blamer_bundle->incorrect_result_reason() == IRR_NO_TRUTH)) {
    if (classify_debug_level) tprintf("No truth for word - skipping\n");
    word->tess_failed = true;
    return;
  }
  ASSERT_HOST(!word->chopped_word->blobs.empty());
  recog_word_recursive(word);
  word->SetupBoxWord();
  if (word->best_choice->length() != word->box_word->length()) {
    tprintf("recog_word ASSERT FAIL String:\"%s\"; "
            "Strlen=%d; #Blobs=%d\n",
            word->best_choice->debug_string().c_str(),
            word->best_choice->length(), word->box_word->length());
  }
  ASSERT_HOST(word->best_choice->length() == word->box_word->length());
  // Check that the ratings matrix size matches the sum of all the
  // segmentation states.
  if (!word->StatesAllValid()) {
    tprintf("Not all words have valid states relative to ratings matrix!!");
    word->DebugWordChoices(true, nullptr);
    ASSERT_HOST(word->StatesAllValid());
  }
  if (tessedit_override_permuter) {
    /* Override the permuter type if a straight dictionary check disagrees. */
    uint8_t perm_type = word->best_choice->permuter();
    if ((perm_type != SYSTEM_DAWG_PERM) &&
        (perm_type != FREQ_DAWG_PERM) && (perm_type != USER_DAWG_PERM)) {
      uint8_t real_dict_perm_type = dict_word(*word->best_choice);
      if (((real_dict_perm_type == SYSTEM_DAWG_PERM) ||
           (real_dict_perm_type == FREQ_DAWG_PERM) ||
           (real_dict_perm_type == USER_DAWG_PERM)) &&
          (alpha_count(word->best_choice->unichar_string().c_str(),
                       word->best_choice->unichar_lengths().c_str()) > 0)) {
        word->best_choice->set_permuter(real_dict_perm_type);  // use dict perm
      }
    }
    if (tessedit_rejection_debug &&
        perm_type != word->best_choice->permuter()) {
      tprintf("Permuter Type Flipped from %d to %d\n",
              perm_type, word->best_choice->permuter());
    }
  }
  // Factored out from control.cpp
  ASSERT_HOST((word->best_choice == nullptr) == (word->raw_choice == nullptr));
  if (word->best_choice == nullptr || word->best_choice->length() == 0 ||
      static_cast<int>(strspn(word->best_choice->unichar_string().c_str(),
                              " ")) == word->best_choice->length()) {
    word->tess_failed = true;
    word->reject_map.initialise(word->box_word->length());
    word->reject_map.rej_word_tess_failure();
  } else {
    word->tess_failed = false;
  }
}


/**********************************************************************
 * recog_word_recursive
 *
 * Convert the word to tess form and pass it to the tess segmenter.
 * Convert the output back to editor form.
 **********************************************************************/
void Tesseract::recog_word_recursive(WERD_RES *word) {
  int word_length = word->chopped_word->NumBlobs();  // no of blobs
  if (word_length > MAX_UNDIVIDED_LENGTH) {
    return split_and_recog_word(word);
  }
  cc_recog(word);
  word_length = word->rebuild_word->NumBlobs();  // No of blobs in output.

  // Do sanity checks and minor fixes on best_choice.
  if (word->best_choice->length() > word_length) {
    word->best_choice->make_bad();  // should never happen
    tprintf("recog_word: Discarded long string \"%s\""
            " (%d characters vs %d blobs)\n",
            word->best_choice->unichar_string().c_str(),
            word->best_choice->length(), word_length);
    tprintf("Word is at:");
    word->word->bounding_box().print();
  }
  if (word->best_choice->length() < word_length) {
    UNICHAR_ID space_id = unicharset.unichar_to_id(" ");
    while (word->best_choice->length() < word_length) {
      word->best_choice->append_unichar_id(space_id, 1, 0.0,
                                           word->best_choice->certainty());
    }
  }
}


/**********************************************************************
 * split_and_recog_word
 *
 * Split the word into 2 smaller pieces at the largest gap.
 * Recognize the pieces and stick the results back together.
 **********************************************************************/
void Tesseract::split_and_recog_word(WERD_RES *word) {
  // Find the biggest blob gap in the chopped_word.
  int bestgap = -INT32_MAX;
  int split_index = 0;
  for (int b = 1; b < word->chopped_word->NumBlobs(); ++b) {
    TBOX prev_box = word->chopped_word->blobs[b - 1]->bounding_box();
    TBOX blob_box = word->chopped_word->blobs[b]->bounding_box();
    int gap = blob_box.left() - prev_box.right();
    if (gap > bestgap) {
      bestgap = gap;
      split_index = b;
    }
  }
  ASSERT_HOST(split_index > 0);

  WERD_RES *word2 = nullptr;
  BlamerBundle *orig_bb = nullptr;
  split_word(word, split_index, &word2, &orig_bb);

  // Recognize the first part of the word.
  recog_word_recursive(word);
  // Recognize the second part of the word.
  recog_word_recursive(word2);

  join_words(word, word2, orig_bb);
}


/**********************************************************************
 * split_word
 *
 * Split a given WERD_RES in place into two smaller words for recognition.
 * split_pt is the index of the first blob to go in the second word.
 * The underlying word is left alone, only the TWERD (and subsequent data)
 * are split up.  orig_blamer_bundle is set to the original blamer bundle,
 * and will now be owned by the caller.  New blamer bundles are forged for the
 * two pieces.
 **********************************************************************/
void Tesseract::split_word(WERD_RES *word,
                           int split_pt,
                           WERD_RES **right_piece,
                           BlamerBundle **orig_blamer_bundle) const {
  ASSERT_HOST(split_pt >0 && split_pt < word->chopped_word->NumBlobs());

  // Save a copy of the blamer bundle so we can try to reconstruct it below.
  BlamerBundle *orig_bb =
      word->blamer_bundle ? new BlamerBundle(*word->blamer_bundle) : nullptr;

  auto *word2 = new WERD_RES(*word);

  // blow away the copied chopped_word, as we want to work with
  // the blobs from the input chopped_word so seam_arrays can be merged.
  TWERD *chopped = word->chopped_word;
  auto *chopped2 = new TWERD;
  chopped2->blobs.reserve(chopped->NumBlobs() - split_pt);
  for (int i = split_pt; i < chopped->NumBlobs(); ++i) {
    chopped2->blobs.push_back(chopped->blobs[i]);
  }
  chopped->blobs.truncate(split_pt);
  word->chopped_word = nullptr;
  delete word2->chopped_word;
  word2->chopped_word = nullptr;

  const UNICHARSET &unicharset = *word->uch_set;
  word->ClearResults();
  word2->ClearResults();
  word->chopped_word = chopped;
  word2->chopped_word = chopped2;
  word->SetupBasicsFromChoppedWord(unicharset);
  word2->SetupBasicsFromChoppedWord(unicharset);

  // Try to adjust the blamer bundle.
  if (orig_bb != nullptr) {
    // TODO(rays) Looks like a leak to me.
    // orig_bb should take, rather than copy.
    word->blamer_bundle = new BlamerBundle();
    word2->blamer_bundle = new BlamerBundle();
    orig_bb->SplitBundle(chopped->blobs.back()->bounding_box().right(),
                         word2->chopped_word->blobs[0]->bounding_box().left(),
                         wordrec_debug_blamer,
                         word->blamer_bundle, word2->blamer_bundle);
  }

  *right_piece = word2;
  *orig_blamer_bundle = orig_bb;
}


/**********************************************************************
 * join_words
 *
 * The opposite of split_word():
 *  join word2 (including any recognized data / seam array / etc)
 *  onto the right of word and then delete word2.
 *  Also, if orig_bb is provided, stitch it back into word.
 **********************************************************************/
void Tesseract::join_words(WERD_RES *word,
                           WERD_RES *word2,
                           BlamerBundle *orig_bb) const {
  TBOX prev_box = word->chopped_word->blobs.back()->bounding_box();
  TBOX blob_box = word2->chopped_word->blobs[0]->bounding_box();
  // Tack the word2 outputs onto the end of the word outputs.
  word->chopped_word->blobs += word2->chopped_word->blobs;
  word->rebuild_word->blobs += word2->rebuild_word->blobs;
  word2->chopped_word->blobs.clear();
  word2->rebuild_word->blobs.clear();
  TPOINT split_pt;
  split_pt.x = (prev_box.right() + blob_box.left()) / 2;
  split_pt.y = (prev_box.top() + prev_box.bottom() +
                blob_box.top() + blob_box.bottom()) / 4;
  // Move the word2 seams onto the end of the word1 seam_array.
  // Since the seam list is one element short, an empty seam marking the
  // end of the last blob in the first word is needed first.
  word->seam_array.push_back(new SEAM(0.0f, split_pt));
  word->seam_array += word2->seam_array;
  word2->seam_array.truncate(0);
  // Fix widths and gaps.
  word->blob_widths += word2->blob_widths;
  word->blob_gaps += word2->blob_gaps;
  // Fix the ratings matrix.
  int rat1 = word->ratings->dimension();
  int rat2 = word2->ratings->dimension();
  word->ratings->AttachOnCorner(word2->ratings);
  ASSERT_HOST(word->ratings->dimension() == rat1 + rat2);
  word->best_state += word2->best_state;
  // Append the word choices.
  *word->raw_choice += *word2->raw_choice;

  // How many alt choices from each should we try to get?
  const int kAltsPerPiece = 2;
  // When do we start throwing away extra alt choices?
  const int kTooManyAltChoices = 100;

  // Construct the cartesian product of the best_choices of word(1) and word2.
  WERD_CHOICE_LIST joined_choices;
  WERD_CHOICE_IT jc_it(&joined_choices);
  WERD_CHOICE_IT bc1_it(&word->best_choices);
  WERD_CHOICE_IT bc2_it(&word2->best_choices);
  int num_word1_choices = word->best_choices.length();
  int total_joined_choices = num_word1_choices;
  // Nota Bene: For the main loop here, we operate only on the 2nd and greater
  // word2 choices, and put them in the joined_choices list. The 1st word2
  // choice gets added to the original word1 choices in-place after we have
  // finished with them.
  int bc2_index = 1;
  for (bc2_it.forward(); !bc2_it.at_first(); bc2_it.forward(), ++bc2_index) {
    if (total_joined_choices >= kTooManyAltChoices &&
        bc2_index > kAltsPerPiece)
      break;
    int bc1_index = 0;
    for (bc1_it.move_to_first(); bc1_index < num_word1_choices;
        ++bc1_index, bc1_it.forward()) {
      if (total_joined_choices >= kTooManyAltChoices &&
          bc1_index > kAltsPerPiece)
        break;
      auto *wc = new WERD_CHOICE(*bc1_it.data());
      *wc += *bc2_it.data();
      jc_it.add_after_then_move(wc);
      ++total_joined_choices;
    }
  }
  // Now that we've filled in as many alternates as we want, paste the best
  // choice for word2 onto the original word alt_choices.
  bc1_it.move_to_first();
  bc2_it.move_to_first();
  for (bc1_it.mark_cycle_pt(); !bc1_it.cycled_list(); bc1_it.forward()) {
    *bc1_it.data() += *bc2_it.data();
  }
  bc1_it.move_to_last();
  bc1_it.add_list_after(&joined_choices);

  // Restore the pointer to original blamer bundle and combine blamer
  // information recorded in the splits.
  if (orig_bb != nullptr) {
    orig_bb->JoinBlames(*word->blamer_bundle, *word2->blamer_bundle,
                        wordrec_debug_blamer);
    delete word->blamer_bundle;
    word->blamer_bundle = orig_bb;
  }
  word->SetupBoxWord();
  word->reject_map.initialise(word->box_word->length());
  delete word2;
}


}  // namespace tesseract
