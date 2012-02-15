/**********************************************************************
 * File:        tfacepp.cpp  (Formerly tface++.c)
 * Description: C++ side of the C/C++ Tess/Editor interface.
 * Author:                  Ray Smith
 * Created:                 Thu Apr 23 15:39:23 BST 1992
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

#ifdef _MSC_VER
#pragma warning(disable:4244)  // Conversion warnings
#pragma warning(disable:4305)  // int/float warnings
#pragma warning(disable:4800)  // int/bool warnings
#endif

#include <math.h>

#include "mfcpch.h"
#ifdef __UNIX__
#include          <assert.h>
#endif
#include          "errcode.h"
#include          "ratngs.h"
#include          "reject.h"
#include          "werd.h"
#include          "tfacep.h"
#include          "tfacepp.h"
#include          "tessvars.h"
#include          "globals.h"
#include          "reject.h"
#include          "tesseractclass.h"

#define MAX_UNDIVIDED_LENGTH 24



/**********************************************************************
 * recog_word
 *
 * Convert the word to tess form and pass it to the tess segmenter.
 * Convert the output back to editor form.
 **********************************************************************/
namespace tesseract {
void Tesseract::recog_word(WERD_RES *word,
                           BLOB_CHOICE_LIST_CLIST *blob_choices) {
  ASSERT_HOST(word->chopped_word->blobs != NULL);
  recog_word_recursive(word, blob_choices);
  word->SetupBoxWord();
  if ((word->best_choice->length() != word->box_word->length()) ||
      (word->best_choice->length() != blob_choices->length())) {
    tprintf("recog_word ASSERT FAIL String:\"%s\"; "
            "Strlen=%d; #Blobs=%d; #Choices=%d\n",
            word->best_choice->debug_string().string(),
            word->best_choice->length(), word->box_word->length(),
            blob_choices->length());
  }
  ASSERT_HOST(word->best_choice->length() == word->box_word->length());
  ASSERT_HOST(word->best_choice->length() == blob_choices->length());
  if (tessedit_override_permuter) {
    /* Override the permuter type if a straight dictionary check disagrees. */
    uinT8 perm_type = word->best_choice->permuter();
    if ((perm_type != SYSTEM_DAWG_PERM) &&
        (perm_type != FREQ_DAWG_PERM) && (perm_type != USER_DAWG_PERM)) {
      uinT8 real_dict_perm_type = dict_word(*word->best_choice);
      if (((real_dict_perm_type == SYSTEM_DAWG_PERM) ||
           (real_dict_perm_type == FREQ_DAWG_PERM) ||
           (real_dict_perm_type == USER_DAWG_PERM)) &&
          (alpha_count(word->best_choice->unichar_string().string(),
                       word->best_choice->unichar_lengths().string()) > 0)) {
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
  ASSERT_HOST((word->best_choice == NULL) == (word->raw_choice == NULL));
  if (word->best_choice == NULL || word->best_choice->length() == 0 ||
      strspn(word->best_choice->unichar_string().string(), " ") ==
        word->best_choice->length()) {
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
void Tesseract::recog_word_recursive(WERD_RES *word,
                                     BLOB_CHOICE_LIST_CLIST *blob_choices) {
  int word_length = word->chopped_word->NumBlobs();  // no of blobs
  if (word_length > MAX_UNDIVIDED_LENGTH) {
    return split_and_recog_word(word, blob_choices);
  }
  int initial_blob_choice_len = blob_choices->length();
  BLOB_CHOICE_LIST_VECTOR* tess_ratings = cc_recog(word);

  // Put BLOB_CHOICE_LISTs from tess_ratings into blob_choices.
  BLOB_CHOICE_LIST_C_IT blob_choices_it(blob_choices);
  for (int i = 0; i < tess_ratings->length(); ++i) {
    blob_choices_it.add_to_end(tess_ratings->get(i));
  }
  delete tess_ratings;

  word_length = word->rebuild_word->NumBlobs();  // No of blobs in output.
  // Pad raw_choice with spaces if needed.
  if (word->raw_choice->length() < word_length) {
    UNICHAR_ID space_id = unicharset.unichar_to_id(" ");
    while (word->raw_choice->length() < word_length) {
      word->raw_choice->append_unichar_id(space_id, 1, 0.0,
                                          word->raw_choice->certainty());
    }
  }

  // Do sanity checks and minor fixes on best_choice.
  if (word->best_choice->length() > word_length) {
    word->best_choice->make_bad();  // should never happen
    tprintf("recog_word: Discarded long string \"%s\""
            " (%d characters vs %d blobs)\n",
            word->best_choice->unichar_string().string(),
            word->best_choice->length(), word_length);
    tprintf("Word is at:");
    word->word->bounding_box().print();
  }
  if (blob_choices->length() - initial_blob_choice_len != word_length) {
    word->best_choice->make_bad();  // force rejection
    tprintf("recog_word: Choices list len:%d; blob lists len:%d\n",
            blob_choices->length(), word_length);
    blob_choices_it.set_to_list(blob_choices);  // list of lists
    while (blob_choices->length() - initial_blob_choice_len < word_length) {
      blob_choices_it.add_to_end(new BLOB_CHOICE_LIST());  // add a fake one
      tprintf("recog_word: Added dummy choice list\n");
    }
    while (blob_choices->length() - initial_blob_choice_len > word_length) {
      blob_choices_it.move_to_last(); // should never happen
      delete blob_choices_it.extract();
      tprintf("recog_word: Deleted choice list\n");
    }
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

void Tesseract::split_and_recog_word(WERD_RES *word,
                                     BLOB_CHOICE_LIST_CLIST *blob_choices) {
  // Find the biggest blob gap in the chopped_word.
  int bestgap = -MAX_INT32;
  TPOINT best_split_pt;
  TBLOB* best_end = NULL;
  TBLOB* prev_blob = NULL;
  for (TBLOB* blob = word->chopped_word->blobs; blob != NULL;
       blob = blob->next) {
    if (prev_blob != NULL) {
      TBOX prev_box = prev_blob->bounding_box();
      TBOX blob_box = blob->bounding_box();
      int gap = blob_box.left() - prev_box.right();
      if (gap > bestgap) {
        bestgap = gap;
        best_end = prev_blob;
        best_split_pt.x = (prev_box.right() + blob_box.left()) / 2;
        best_split_pt.y = (prev_box.top() + prev_box.bottom() +
                           blob_box.top() + blob_box.bottom()) / 4;
      }
    }
    prev_blob = blob;
  }
  ASSERT_HOST(best_end != NULL);
  ASSERT_HOST(best_end->next != NULL);

  // Make a copy of the word to put the 2nd half in.
  WERD_RES* word2 = new WERD_RES(*word);
  // Blow away the copied chopped_word, as we want to work with the blobs
  // from the input chopped_word so the seam_arrays can be merged.
  delete word2->chopped_word;
  word2->chopped_word = new TWERD;
  word2->chopped_word->blobs = best_end->next;
  best_end->next = NULL;
  // Make a new seamarray on both words.
  free_seam_list(word->seam_array);
  word->seam_array = start_seam_list(word->chopped_word->blobs);
  word2->seam_array = start_seam_list(word2->chopped_word->blobs);
  BlamerBundle *orig_bb = word->blamer_bundle;
  STRING blamer_debug;
  // Try to adjust truth information.
  if (orig_bb != NULL) {
    // Find truth boxes that correspond to the split in the blobs.
    int b;
    int begin2_truth_index = -1;
    if (orig_bb->incorrect_result_reason != IRR_NO_TRUTH &&
        orig_bb->truth_has_char_boxes) {
      int end1_x = best_end->bounding_box().right();
      int begin2_x = word2->chopped_word->blobs->bounding_box().left();
      blamer_debug = "Looking for truth split at";
      blamer_debug.add_str_int(" end1_x ", end1_x);
      blamer_debug.add_str_int(" begin2_x ", begin2_x);
      blamer_debug += "\nnorm_truth_word boxes:\n";
      if (orig_bb->norm_truth_word.length() > 1) {
        orig_bb->norm_truth_word.BlobBox(0).append_debug(&blamer_debug);
        for (b = 1; b < orig_bb->norm_truth_word.length(); ++b) {
          orig_bb->norm_truth_word.BlobBox(b).append_debug(&blamer_debug);
          if ((abs(end1_x - orig_bb->norm_truth_word.BlobBox(b-1).right()) <
              orig_bb->norm_box_tolerance) &&
              (abs(begin2_x - orig_bb->norm_truth_word.BlobBox(b).left()) <
              orig_bb->norm_box_tolerance)) {
            begin2_truth_index = b;
            blamer_debug += "Split found\n";
            break;
          }
        }
      }
    }
    // Populate truth information in word and word2 with the first and second
    // part of the original truth.
    word->blamer_bundle = new BlamerBundle();
    word2->blamer_bundle = new BlamerBundle();
    if (begin2_truth_index > 0) {
      word->blamer_bundle->truth_has_char_boxes = true;
      word->blamer_bundle->norm_box_tolerance = orig_bb->norm_box_tolerance;
      word2->blamer_bundle->truth_has_char_boxes = true;
      word2->blamer_bundle->norm_box_tolerance = orig_bb->norm_box_tolerance;
      BlamerBundle *curr_bb = word->blamer_bundle;
      for (b = 0; b < orig_bb->norm_truth_word.length(); ++b) {
        if (b == begin2_truth_index) curr_bb = word2->blamer_bundle;
        curr_bb->norm_truth_word.InsertBox(
            b, orig_bb->norm_truth_word.BlobBox(b));
        curr_bb->truth_word.InsertBox(b, orig_bb->truth_word.BlobBox(b));
        curr_bb->truth_text.push_back(orig_bb->truth_text[b]);
      }
    } else if (orig_bb->incorrect_result_reason == IRR_NO_TRUTH) {
      word->blamer_bundle->incorrect_result_reason = IRR_NO_TRUTH;
      word2->blamer_bundle->incorrect_result_reason = IRR_NO_TRUTH;
    } else {
      blamer_debug += "Truth split not found";
      blamer_debug += orig_bb->truth_has_char_boxes ?
          "\n" : " (no truth char boxes)\n";
      word->blamer_bundle->SetBlame(IRR_NO_TRUTH_SPLIT, blamer_debug,
                                    NULL, wordrec_debug_blamer);
      word2->blamer_bundle->SetBlame(IRR_NO_TRUTH_SPLIT, blamer_debug,
                                     NULL, wordrec_debug_blamer);
    }
  }

  // Recognize the first part of the word.
  recog_word_recursive(word, blob_choices);
  // Recognize the second part of the word.
  recog_word_recursive(word2, blob_choices);
  // Tack the word2 outputs onto the end of the word outputs.
  // New blobs might have appeared on the end of word1.
  for (best_end = word->chopped_word->blobs; best_end->next != NULL;
       best_end = best_end->next);
  best_end->next = word2->chopped_word->blobs;
  TBLOB* blob;
  for (blob = word->rebuild_word->blobs; blob->next != NULL; blob = blob->next);
  blob->next = word2->rebuild_word->blobs;
  word2->chopped_word->blobs = NULL;
  word2->rebuild_word->blobs = NULL;
  // Copy the seams onto the end of the word1 seam_array.
  // Since the seam list is one element short, an empty seam marking the
  // end of the last blob in the first word is needed first.
  word->seam_array = add_seam(word->seam_array,
                              new_seam(0.0, best_split_pt, NULL, NULL, NULL));
  for (int i = 0; i < array_count(word2->seam_array); ++i) {
    SEAM* seam = reinterpret_cast<SEAM*>(array_value(word2->seam_array, i));
    array_value(word2->seam_array, i) = NULL;
    word->seam_array = add_seam(word->seam_array, seam);
  }
  word->best_state += word2->best_state;
  // Append the word choices.
  *word->best_choice += *word2->best_choice;
  *word->raw_choice += *word2->raw_choice;

  // How many alt choices from each should we try to get?
  const int kAltsPerPiece = 2;
  // When do we start throwing away extra alt choices?
  const int kTooManyAltChoices = 100;

  if (word->alt_choices.size() > 0 && word2->alt_choices.size() > 0) {
    // Construct the cartesian product of the alt choices of word(1) and word2.
    int num_first_alt_choices = word->alt_choices.size();
    // Nota Bene: For the main loop here, we leave in place word1-only
    // alt_choices in
    //   word->alt_choices[0] .. word_alt_choices[num_first_alt_choices - 1]
    // These will get fused with the best choices for word2 below.
    for (int j = 1; j < word2->alt_choices.size() &&
         (j <= kAltsPerPiece || word->alt_choices.size() < kTooManyAltChoices);
         j++) {
      for (int i = 0; i < num_first_alt_choices &&
           (i <= kAltsPerPiece ||
            word->alt_choices.size() < kTooManyAltChoices);
           i++) {
        WERD_CHOICE *wc = new WERD_CHOICE(*word->alt_choices[i]);
        *wc += *word2->alt_choices[j];
        word->alt_choices.push_back(wc);

        word->alt_states.push_back(GenericVector<int>());
        GenericVector<int> &alt_state = word->alt_states.back();
        alt_state += word->alt_states[i];
        alt_state += word2->alt_states[j];
      }
    }
    // Now that we've filled in as many alternates as we want, paste the best
    // choice for word2 onto the original word alt_choices.
    for (int i = 0; i < num_first_alt_choices; i++) {
      *word->alt_choices[i] += *word2->alt_choices[0];
      word->alt_states[i] += word2->alt_states[0];
    }
  }

  // Restore the pointer to original blamer bundle and combine blamer
  // information recorded in the splits.
  if (orig_bb != NULL) {
    IncorrectResultReason irr = orig_bb->incorrect_result_reason;
    if (irr != IRR_NO_TRUTH_SPLIT) blamer_debug = "";
    if (word->blamer_bundle->incorrect_result_reason != IRR_CORRECT &&
        word->blamer_bundle->incorrect_result_reason != IRR_NO_TRUTH &&
        word->blamer_bundle->incorrect_result_reason != IRR_NO_TRUTH_SPLIT) {
      blamer_debug += "Blame from part 1: ";
      blamer_debug += word->blamer_bundle->debug;
      irr = word->blamer_bundle->incorrect_result_reason;
    }
    if (word2->blamer_bundle->incorrect_result_reason != IRR_CORRECT &&
        word2->blamer_bundle->incorrect_result_reason != IRR_NO_TRUTH &&
        word2->blamer_bundle->incorrect_result_reason != IRR_NO_TRUTH_SPLIT) {
      blamer_debug += "Blame from part 2: ";
      blamer_debug += word2->blamer_bundle->debug;
      if (irr == IRR_CORRECT) {
        irr = word2->blamer_bundle->incorrect_result_reason;
      } else if (irr != word2->blamer_bundle->incorrect_result_reason) {
        irr = IRR_UNKNOWN;
      }
    }
    delete word->blamer_bundle;
    word->blamer_bundle = orig_bb;
    word->blamer_bundle->incorrect_result_reason = irr;
    if (irr != IRR_CORRECT && irr != IRR_NO_TRUTH) {
      word->blamer_bundle->SetBlame(irr, blamer_debug, NULL,
                                    wordrec_debug_blamer);
    }
  }
  delete word2;
}

}  // namespace tesseract
