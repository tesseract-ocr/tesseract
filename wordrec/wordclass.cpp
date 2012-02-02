/* -*-C-*-
 ********************************************************************************
 *
 * File:        wordclass.c  (Formerly wordclass.c)
 * Description:  Word classifier
 * Author:       Mark Seaman, OCR Technology
 * Created:      Tue Jan 30 14:03:25 1990
 * Modified:     Fri Jul 12 16:03:06 1991 (Mark Seaman) marks@hpgrlt
 * Language:     C
 * Package:      N/A
 * Status:       Experimental (Do Not Distribute)
 *
 * (c) Copyright 1990, Hewlett-Packard Company.
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
 *********************************************************************************/
/*----------------------------------------------------------------------
          I N C L U D E S
----------------------------------------------------------------------*/
#include <stdio.h>
#ifdef __UNIX__
#include <assert.h>
#endif

#include "wordclass.h"
#include "associate.h"
#include "render.h"
#include "matchtab.h"
#include "permute.h"
#include "callcpp.h"
#include <assert.h>
#include "wordrec.h"

// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif

/*----------------------------------------------------------------------
          F u n c t i o n s
----------------------------------------------------------------------*/
namespace tesseract {
/**
 * @name classify_blob
 *
 * Classify the this blob if it is not already recorded in the match
 * table. Attempt to recognize this blob as a character. The recognition
 * rating for this blob will be stored as a part of the blob. This value
 * will also be returned to the caller.
 * @param blob Current blob
 * @param string The string to display in ScrollView
 * @param color The colour to use when displayed with ScrollView
 */
BLOB_CHOICE_LIST *Wordrec::classify_blob(TBLOB *blob, const DENORM& denorm,
                                         const char *string, C_COL color,
                                         BlamerBundle *blamer_bundle) {
  fflush(stdout);
  BLOB_CHOICE_LIST *choices = NULL;
#ifndef GRAPHICS_DISABLED
  if (wordrec_display_all_blobs)
    display_blob(blob, color);
#endif
  choices = blob_match_table.get_match(blob);
  if (choices == NULL) {
    choices = call_matcher(&denorm, blob);
    blob_match_table.put_match(blob, choices);
    // If a blob with the same bounding box as one of the truth character
    // bounding boxes is not classified as the corresponding truth character
    // blame character classifier for incorrect answer.
    if (blamer_bundle != NULL && blamer_bundle->truth_has_char_boxes &&
        blamer_bundle->incorrect_result_reason == IRR_CORRECT) {
      for (int b = 0; b < blamer_bundle->norm_truth_word.length(); ++b) {
        const TBOX &truth_box = blamer_bundle->norm_truth_word.BlobBox(b);
        const TBOX &blob_box = blob->bounding_box();
        // Note that we are more strict on the bounding box boundaries here
        // than in other places (chopper, segmentation search), since we do
        // not have the ability to check the previous and next bounding box.
        if (blob_box.x_almost_equal(truth_box,
                                    blamer_bundle->norm_box_tolerance/2)) {
          BLOB_CHOICE_IT choices_it(choices);
          bool found = false;
          bool incorrect_adapted = false;
          UNICHAR_ID incorrect_adapted_id = INVALID_UNICHAR_ID;
          const char *truth_str = blamer_bundle->truth_text[b].string();
          for (choices_it.mark_cycle_pt(); !choices_it.cycled_list();
              choices_it.forward()) {
            if (strcmp(truth_str, getDict().getUnicharset().get_normed_unichar(
                choices_it.data()->unichar_id())) == 0) {
              found = true;
              break;
            } else if (choices_it.data()->adapted()) {
              incorrect_adapted = true;
              incorrect_adapted_id = choices_it.data()->unichar_id();
            }
          }  // end choices_it for loop
          if (!found) {
            STRING debug = "unichar ";
            debug += truth_str;
            debug += " not found in classification list";
            blamer_bundle->SetBlame(IRR_CLASSIFIER, debug,
                                    NULL, wordrec_debug_blamer);
          } else if (incorrect_adapted) {
            STRING debug = "better rating for adapted ";
            debug += getDict().getUnicharset().id_to_unichar(
                incorrect_adapted_id);
            debug += " than for correct ";
            debug += truth_str;
            blamer_bundle->SetBlame(IRR_ADAPTION, debug,
                                    NULL, wordrec_debug_blamer);
          }
          break;
        }
      }  // end iterating over blamer_bundle->norm_truth_word
    }
  }
#ifndef GRAPHICS_DISABLED
  if (classify_debug_level && string)
    print_ratings_list(string, choices, getDict().getUnicharset());

  if (wordrec_blob_pause)
    window_wait(blob_window);
#endif

  return (choices);
}

// Returns a valid BLOB_CHOICE_LIST representing the given result.
BLOB_CHOICE_LIST *Wordrec::fake_classify_blob(UNICHAR_ID class_id,
                                              float rating, float certainty) {
  BLOB_CHOICE_LIST *ratings = new BLOB_CHOICE_LIST();  // matcher result
  BLOB_CHOICE *choice =
      new BLOB_CHOICE(class_id, rating, certainty, -1, -1, 0, 0, 0, false);
  BLOB_CHOICE_IT temp_it(ratings);
  temp_it.add_after_stay_put(choice);
  return ratings;
}

/**
 * @name update_blob_classifications
 *
 * For each blob in the given word update match_table with the
 * corresponding BLOB_CHOICES_LIST from choices.
 */
void Wordrec::update_blob_classifications(
    TWERD *word, const BLOB_CHOICE_LIST_VECTOR &choices) {
  TBLOB *tblob = word->blobs;
  int index = 0;
  for (; tblob != NULL && index < choices.length();
       tblob = tblob->next, index++) {
    blob_match_table.add_to_match(tblob, choices.get(index));
  }
}

}  // namespace tesseract;
