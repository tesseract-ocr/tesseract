/* -*-C-*-
 ********************************************************************************
 *
 * File:        chopper.c  (Formerly chopper.c)
 * Description:
 * Author:       Mark Seaman, OCR Technology
 * Created:      Fri Oct 16 14:37:00 1987
 * Modified:     Tue Jul 30 16:18:52 1991 (Mark Seaman) marks@hpgrlt
 * Language:     C
 * Package:      N/A
 * Status:       Reusable Software Component
 *
 * (c) Copyright 1987, Hewlett-Packard Company.
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
 **************************************************************************/

/*----------------------------------------------------------------------
          I n c l u d e s
----------------------------------------------------------------------*/

#include <math.h>

#include "chopper.h"

#include "assert.h"
#include "associate.h"
#include "callcpp.h"
#include "const.h"
#include "findseam.h"
#include "freelist.h"
#include "globals.h"
#include "makechop.h"
#include "render.h"
#include "pageres.h"
#include "permute.h"
#include "seam.h"
#include "stopper.h"
#include "structures.h"
#include "unicharset.h"
#include "wordclass.h"
#include "wordrec.h"

// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif

/*----------------------------------------------------------------------
          F u n c t i o n s
----------------------------------------------------------------------*/
/**
 * @name preserve_outline_tree
 *
 * Copy the list of outlines.
 */
void preserve_outline(EDGEPT *start) {
  EDGEPT *srcpt;

  if (start == NULL)
    return;
  srcpt = start;
  do {
    srcpt->flags[1] = 1;
    srcpt = srcpt->next;
  }
  while (srcpt != start);
  srcpt->flags[1] = 2;
}


/**************************************************************************/
void preserve_outline_tree(TESSLINE *srcline) {
  TESSLINE *outline;

  for (outline = srcline; outline != NULL; outline = outline->next) {
    preserve_outline (outline->loop);
  }
}


/**
 * @name restore_outline_tree
 *
 * Copy the list of outlines.
 */
EDGEPT *restore_outline(EDGEPT *start) {
  EDGEPT *srcpt;
  EDGEPT *real_start;
  EDGEPT *deadpt;

  if (start == NULL)
    return NULL;
  srcpt = start;
  do {
    if (srcpt->flags[1] == 2)
      break;
    srcpt = srcpt->next;
  }
  while (srcpt != start);
  real_start = srcpt;
  do {
    if (srcpt->flags[1] == 0) {
      deadpt = srcpt;
      srcpt = srcpt->next;
      srcpt->prev = deadpt->prev;
      deadpt->prev->next = srcpt;
      deadpt->prev->vec.x = srcpt->pos.x - deadpt->prev->pos.x;
      deadpt->prev->vec.y = srcpt->pos.y - deadpt->prev->pos.y;
      delete deadpt;
    }
    else
      srcpt = srcpt->next;
  }
  while (srcpt != real_start);
  return real_start;
}


/******************************************************************************/
void restore_outline_tree(TESSLINE *srcline) {
  TESSLINE *outline;

  for (outline = srcline; outline != NULL; outline = outline->next) {
    outline->loop = restore_outline (outline->loop);
    outline->start = outline->loop->pos;
  }
}


/**
 * @name attempt_blob_chop
 *
 * Try to split the this blob after this one.  Check to make sure that
 * it was successful.
 */
namespace tesseract {
SEAM *Wordrec::attempt_blob_chop(TWERD *word, TBLOB *blob, inT32 blob_number,
                                 bool italic_blob, SEAMS seam_list) {
  TBLOB *next_blob = blob->next;
  TBLOB *other_blob;
  SEAM *seam;

  if (repair_unchopped_blobs)
    preserve_outline_tree (blob->outlines);
  other_blob = new TBLOB;       /* Make new blob */
  other_blob->next = blob->next;
  other_blob->outlines = NULL;
  blob->next = other_blob;

  seam = NULL;
  if (prioritize_division) {
    TPOINT location;
    if (divisible_blob(blob, italic_blob, &location)) {
      seam = new_seam(0.0f, location, NULL, NULL, NULL);
    }
  }
  if (seam == NULL)
    seam = pick_good_seam(blob);
  if (seam == NULL && word->latin_script) {
    // If the blob can simply be divided into outlines, then do that.
    TPOINT location;
    if (divisible_blob(blob, italic_blob, &location)) {
      seam = new_seam(0.0f, location, NULL, NULL, NULL);
    }
  }
  if (chop_debug) {
    if (seam != NULL) {
      print_seam ("Good seam picked=", seam);
    }
    else
      cprintf ("\n** no seam picked *** \n");
  }
  if (seam) {
    apply_seam(blob, other_blob, italic_blob, seam);
  }

  if ((seam == NULL) ||
    (blob->outlines == NULL) ||
    (other_blob->outlines == NULL) ||
    total_containment (blob, other_blob) ||
    check_blob (other_blob) ||
    !(check_seam_order (blob, seam) &&
    check_seam_order (other_blob, seam)) ||
    any_shared_split_points (seam_list, seam) ||
    !test_insert_seam(seam_list, blob_number, blob, word->blobs)) {

    blob->next = next_blob;
    if (seam) {
      undo_seam(blob, other_blob, seam);
      delete_seam(seam);
#ifndef GRAPHICS_DISABLED
      if (chop_debug) {
        if (chop_debug >2)
          display_blob(blob, Red);
        cprintf ("\n** seam being removed ** \n");
      }
#endif
    } else {
      delete other_blob;
    }

    if (repair_unchopped_blobs)
      restore_outline_tree (blob->outlines);
    return (NULL);
  }
  return (seam);
}


SEAM *Wordrec::chop_numbered_blob(TWERD *word, inT32 blob_number,
                                  bool italic_blob, SEAMS seam_list) {
  TBLOB *blob;
  inT16 x;

  blob = word->blobs;
  for (x = 0; x < blob_number; x++)
    blob = blob->next;

  return attempt_blob_chop(word, blob, blob_number,
                           italic_blob, seam_list);
}


SEAM *Wordrec::chop_overlapping_blob(const GenericVector<TBOX>& boxes,
                                     WERD_RES *word_res, inT32 *blob_number,
                                     bool italic_blob, SEAMS seam_list) {
  TWERD *word = word_res->chopped_word;
  TBLOB *blob;

  *blob_number = 0;
  blob = word->blobs;
  while (blob != NULL) {
    TPOINT topleft, botright;
    topleft.x = blob->bounding_box().left();
    topleft.y = blob->bounding_box().top();
    botright.x = blob->bounding_box().right();
    botright.y = blob->bounding_box().bottom();

    TPOINT original_topleft, original_botright;
    word_res->denorm.DenormTransform(topleft, &original_topleft);
    word_res->denorm.DenormTransform(botright, &original_botright);

    TBOX original_box = TBOX(original_topleft.x, original_botright.y,
                             original_botright.x, original_topleft.y);

    bool almost_equal_box = false;
    int num_overlap = 0;
    for (int i = 0; i < boxes.size(); i++) {
      if (original_box.overlap_fraction(boxes[i]) > 0.125)
        num_overlap++;
      if (original_box.almost_equal(boxes[i], 3))
        almost_equal_box = true;
    }

    TPOINT location;
    if (divisible_blob(blob, italic_blob, &location) ||
        (!almost_equal_box && num_overlap > 1)) {
      SEAM *seam = attempt_blob_chop(word, blob, *blob_number,
                                     italic_blob, seam_list);
      if (seam != NULL)
        return seam;
    }

    *blob_number = *blob_number + 1;
    blob = blob->next;
  }

  *blob_number = -1;
  return NULL;
}

}  // namespace tesseract


/**
 * @name any_shared_split_points
 *
 * Return true if any of the splits share a point with this one.
 */
int any_shared_split_points(SEAMS seam_list, SEAM *seam) {
  int length;
  int index;

  length = array_count (seam_list);
  for (index = 0; index < length; index++)
    if (shared_split_points ((SEAM *) array_value (seam_list, index), seam))
      return TRUE;
  return FALSE;
}


/**
 * @name check_blob
 *
 * @return true if blob has a non whole outline.
 */
int check_blob(TBLOB *blob) {
  TESSLINE *outline;
  EDGEPT *edgept;

  for (outline = blob->outlines; outline != NULL; outline = outline->next) {
    edgept = outline->loop;
    do {
      if (edgept == NULL)
        break;
      edgept = edgept->next;
    }
    while (edgept != outline->loop);
    if (edgept == NULL)
      return 1;
  }
  return 0;
}


namespace tesseract {
/**
 * @name improve_one_blob
 *
 * Start with the current word of blobs and its classification.  Find
 * the worst blobs and try to divide it up to improve the ratings.
 */
bool Wordrec::improve_one_blob(WERD_RES *word_res,
                               BLOB_CHOICE_LIST_VECTOR *char_choices,
                               inT32 *blob_number,
                               SEAMS *seam_list,
                               DANGERR *fixpt,
                               bool split_next_to_fragment,
                               BlamerBundle *blamer_bundle) {
  TWERD* word = word_res->chopped_word;
  TBLOB *blob;
  inT16 x = 0;
  float rating_ceiling = MAX_FLOAT32;
  BLOB_CHOICE_LIST *answer;
  BLOB_CHOICE_IT answer_it;
  SEAM *seam;

  do {
    *blob_number = select_blob_to_split_from_fixpt(fixpt);
    bool split_point_from_dict = (*blob_number != -1);
    if (split_point_from_dict) {
      fixpt->clear();
    } else {
      *blob_number = select_blob_to_split(*char_choices, rating_ceiling,
                                          split_next_to_fragment);
    }
    if (chop_debug)
      cprintf("blob_number = %d\n", *blob_number);
    if (*blob_number == -1)
      return false;

    // TODO(rays) it may eventually help to allow italic_blob to be true,
    seam = chop_numbered_blob(word, *blob_number, false, *seam_list);
    if (seam != NULL)
      break;
    /* Must split null blobs */
    answer = char_choices->get(*blob_number);
    if (answer == NULL)
      return false;
    answer_it.set_to_list(answer);
    if (!split_point_from_dict) {
      // We chopped the worst rated blob, try something else next time.
      rating_ceiling = answer_it.data()->rating();
    }
  } while (true);
  /* Split OK */
  for (blob = word->blobs; x < *blob_number; x++) {
    blob = blob->next;
  }

  *seam_list =
    insert_seam (*seam_list, *blob_number, seam, blob, word->blobs);

  delete char_choices->get(*blob_number);

  answer = classify_blob(blob, word_res->denorm, "improve 1:", Red,
                         blamer_bundle);
  char_choices->insert(answer, *blob_number);

  answer = classify_blob(blob->next, word_res->denorm, "improve 2:", Yellow,
                         blamer_bundle);
  char_choices->set(answer, *blob_number + 1);

  return true;
}

/**
 * @name modify_blob_choice
 *
 * Takes a blob and its chop index, converts that chop index to a
 * unichar_id, and stores the chop index in place of the blob's
 * original unichar_id.
 */
void Wordrec::modify_blob_choice(BLOB_CHOICE_LIST *answer,
                        int chop_index) {
  char chop_index_string[2];
  if (chop_index <= 9) {
    snprintf(chop_index_string, sizeof(chop_index_string), "%d", chop_index);
  } else {
    chop_index_string[0] = static_cast<char>('A' - 10 + chop_index);
    chop_index_string[1] = '\0';
  }
  UNICHAR_ID unichar_id = unicharset.unichar_to_id(chop_index_string);
  if (unichar_id == INVALID_UNICHAR_ID) {
    // If the word is very long, we might exhaust the possibilities.
    unichar_id = 1;
  }
  BLOB_CHOICE_IT answer_it(answer);
  BLOB_CHOICE *modified_blob =
      new BLOB_CHOICE(unichar_id,
                      answer_it.data()->rating(),
                      answer_it.data()->certainty(),
                      answer_it.data()->fontinfo_id(),
                      answer_it.data()->fontinfo_id2(),
                      answer_it.data()->script_id(),
                      answer_it.data()->min_xheight(),
                      answer_it.data()->max_xheight(),
                      answer_it.data()->adapted());
  answer->clear();
  answer_it.set_to_list(answer);
  answer_it.add_after_then_move(modified_blob);
}


/**
 * @name chop_one_blob
 *
 * Start with the current one-blob word and its classification.  Find
 * the worst blobs and try to divide it up to improve the ratings.
 * Used for testing chopper.
 */
bool Wordrec::chop_one_blob(TWERD *word,
                            BLOB_CHOICE_LIST_VECTOR *char_choices,
                            inT32 *blob_number,
                            SEAMS *seam_list,
                            int *right_chop_index) {
  TBLOB *blob;
  inT16 x = 0;
  float rating_ceiling = MAX_FLOAT32;
  BLOB_CHOICE_LIST *answer;
  BLOB_CHOICE_IT answer_it;
  SEAM *seam;
  UNICHAR_ID unichar_id = 0;
  int left_chop_index = 0;

  do {
    *blob_number = select_blob_to_split(*char_choices, rating_ceiling, false);
    if (chop_debug)
      cprintf("blob_number = %d\n", *blob_number);
    if (*blob_number == -1)
      return false;
    seam = chop_numbered_blob(word, *blob_number, true, *seam_list);
    if (seam != NULL)
      break;
    /* Must split null blobs */
    answer = char_choices->get(*blob_number);
    if (answer == NULL)
      return false;
    answer_it.set_to_list(answer);
    rating_ceiling = answer_it.data()->rating();  // try a different blob
  } while (true);
  /* Split OK */
  for (blob = word->blobs; x < *blob_number; x++) {
    blob = blob->next;
  }
  if (chop_debug) {
    tprintf("Chop made blob1:");
    blob->bounding_box().print();
    tprintf("and blob2:");
    blob->next->bounding_box().print();
  }
  *seam_list = insert_seam(*seam_list, *blob_number, seam, blob, word->blobs);

  answer = char_choices->get(*blob_number);
  answer_it.set_to_list(answer);
  unichar_id = answer_it.data()->unichar_id();
  float rating = answer_it.data()->rating() / exp(1.0);
  left_chop_index = atoi(unicharset.id_to_unichar(unichar_id));

  delete char_choices->get(*blob_number);
  // combine confidence w/ serial #
  answer = fake_classify_blob(0, rating, -rating);
  modify_blob_choice(answer, left_chop_index);
  char_choices->insert(answer, *blob_number);

  answer = fake_classify_blob(0, rating - 0.125f, -rating);
  modify_blob_choice(answer, ++*right_chop_index);
  char_choices->set(answer, *blob_number + 1);
  return true;
}


bool Wordrec::chop_one_blob2(const GenericVector<TBOX>& boxes,
                             WERD_RES *word_res,
                             SEAMS *seam_list) {
  inT32 blob_number;
  inT16 x = 0;
  TBLOB *blob;
  SEAM *seam;

  seam = chop_overlapping_blob(boxes, word_res, &blob_number,
                               true, *seam_list);
  if (seam == NULL)
    return false;

  /* Split OK */
  for (blob = word_res->chopped_word->blobs; x < blob_number; x++) {
    blob = blob->next;
  }
  if (chop_debug) {
    tprintf("Chop made blob1:");
    blob->bounding_box().print();
    tprintf("and blob2:");
    blob->next->bounding_box().print();
  }
  *seam_list = insert_seam(*seam_list, blob_number, seam, blob,
                           word_res->chopped_word->blobs);
  return true;
}
}  // namespace tesseract

/**
 * @name check_seam_order
 *
 * Make sure that each of the splits in this seam match to outlines
 * in this blob.  If any of the splits could not correspond to this
 * blob then there is a problem (and FALSE should be returned to the
 * caller).
 */
inT16 check_seam_order(TBLOB *blob, SEAM *seam) {
  TESSLINE *outline;
  TESSLINE *last_outline;
  inT8 found_em[3];

  if (seam->split1 == NULL || seam->split1 == NULL || blob == NULL)
    return (TRUE);

  found_em[0] = found_em[1] = found_em[2] = FALSE;

  for (outline = blob->outlines; outline; outline = outline->next) {
    if (!found_em[0] &&
      ((seam->split1 == NULL) ||
    is_split_outline (outline, seam->split1))) {
      found_em[0] = TRUE;
    }
    if (!found_em[1] &&
      ((seam->split2 == NULL) ||
    is_split_outline (outline, seam->split2))) {
      found_em[1] = TRUE;
    }
    if (!found_em[2] &&
      ((seam->split3 == NULL) ||
    is_split_outline (outline, seam->split3))) {
      found_em[2] = TRUE;
    }
    last_outline = outline;
  }

  if (!found_em[0] || !found_em[1] || !found_em[2])
    return (FALSE);
  else
    return (TRUE);
}

namespace tesseract {
/**
 * @name chop_word_main
 *
 * Classify the blobs in this word and permute the results.  Find the
 * worst blob in the word and chop it up.  Continue this process until
 * a good answer has been found or all the blobs have been chopped up
 * enough.  Return the word level ratings.
 */
BLOB_CHOICE_LIST_VECTOR *Wordrec::chop_word_main(WERD_RES *word) {
  TBLOB *blob;
  int index;
  int did_chopping;
  STATE state;
  BLOB_CHOICE_LIST *match_result;
  MATRIX *ratings = NULL;
  DANGERR fixpt;                 /*dangerous ambig */
  inT32 bit_count;               //no of bits

  BLOB_CHOICE_LIST_VECTOR *char_choices = new BLOB_CHOICE_LIST_VECTOR();
  BLOB_CHOICE_LIST_VECTOR *best_char_choices = new BLOB_CHOICE_LIST_VECTOR();

  did_chopping = 0;
  for (blob = word->chopped_word->blobs, index = 0;
       blob != NULL; blob = blob->next, index++) {
    match_result = classify_blob(blob, word->denorm, "chop_word:", Green,
                                 word->blamer_bundle);
    if (match_result == NULL)
      cprintf("Null classifier output!\n");
    *char_choices += match_result;
  }
  bit_count = index - 1;
  set_n_ones(&state, char_choices->length() - 1);
  bool acceptable = false;
  bool replaced = false;
  bool best_choice_updated =
    getDict().permute_characters(*char_choices, word->best_choice,
                                 word->raw_choice);
  if (best_choice_updated &&
      getDict().AcceptableChoice(char_choices, word->best_choice, &fixpt,
                                 CHOPPER_CALLER, &replaced)) {
    acceptable = true;
  }
  if (replaced)
    update_blob_classifications(word->chopped_word, *char_choices);
  CopyCharChoices(*char_choices, best_char_choices);
  if (!acceptable) {  // do more work to find a better choice
    did_chopping = 1;

    bool best_choice_acceptable = false;
    if (chop_enable)
      improve_by_chopping(word,
                          char_choices,
                          &state,
                          best_char_choices,
                          &fixpt,
                          &best_choice_acceptable);
    if (chop_debug)
      print_seams ("Final seam list:", word->seam_array);

    if (word->blamer_bundle != NULL &&
        !ChoiceIsCorrect(*word->uch_set, word->best_choice,
                         word->blamer_bundle->truth_text)) {
      set_chopper_blame(word);
    }

    // The force_word_assoc is almost redundant to enable_assoc.  However,
    // it is not conditioned on the dict behavior.  For CJK, we need to force
    // the associator to be invoked.  When we figure out the exact behavior
    // of dict on CJK, we can remove the flag if it turns out to be redundant.
    if ((wordrec_enable_assoc && !best_choice_acceptable) || force_word_assoc) {
      ratings = word_associator(false, word, &state, best_char_choices,
                                &fixpt, &state);
    }
  }
  best_char_choices = rebuild_current_state(word, &state, best_char_choices,
                                            ratings);

  // If after running only the chopper best_choice is incorrect and no blame
  // has been yet set, blame the classifier if best_choice is classifier's
  // top choice and is a dictionary word (i.e. language model could not have
  // helped). Otherwise blame the tradeoff between the classifier and
  // the old language model (permuters).
  if (word->blamer_bundle != NULL &&
      word->blamer_bundle->incorrect_result_reason == IRR_CORRECT &&
      ratings == NULL &&  // only the chopper was run
      !ChoiceIsCorrect(*word->uch_set, word->best_choice,
                       word->blamer_bundle->truth_text)) {
    if (word->best_choice != NULL &&
        Dict::valid_word_permuter(word->best_choice->permuter(), false)) {
      // Find out whether best choice is a top choice.
      word->blamer_bundle->best_choice_is_dict_and_top_choice = true;
      for (int i = 0; i < word->best_choice->length(); ++i) {
        BLOB_CHOICE_IT blob_choice_it(best_char_choices->get(i));
        ASSERT_HOST(!blob_choice_it.empty());
        BLOB_CHOICE *first_choice = NULL;
        for (blob_choice_it.mark_cycle_pt(); !blob_choice_it.cycled_list();
             blob_choice_it.forward()) {  // find first non-fragment choice
          if (!(getDict().getUnicharset().get_fragment(
                blob_choice_it.data()->unichar_id()))) {
            first_choice = blob_choice_it.data();
            break;
          }
        }
        ASSERT_HOST(first_choice != NULL);
        if (first_choice->unichar_id() != word->best_choice->unichar_id(i)) {
          word->blamer_bundle->best_choice_is_dict_and_top_choice = false;
          break;
        }
      }
    }
    STRING debug;
    if (word->blamer_bundle->best_choice_is_dict_and_top_choice) {
      debug = "Best choice is: incorrect, top choice, dictionary word";
      debug += " with permuter ";
      debug += word->best_choice->permuter_name();
    } else {
      debug = "Classifier/Old LM tradeoff is to blame";
    }
    word->blamer_bundle->SetBlame(
        word->blamer_bundle->best_choice_is_dict_and_top_choice ?
            IRR_CLASSIFIER : IRR_CLASS_OLD_LM_TRADEOFF,
        debug, word->best_choice, wordrec_debug_blamer);
  }

  if (word->blamer_bundle != NULL && this->fill_lattice_ != NULL) {
    if (ratings == NULL) {
      ratings = word_associator(true, word, NULL, NULL, NULL, NULL);
    }
    CallFillLattice(*ratings, getDict().getBestChoices(),
                    *word->uch_set, word->blamer_bundle);
  }
  if (ratings != NULL) {
    if (wordrec_debug_level > 0) {
      tprintf("Final Ratings Matrix:\n");
      ratings->print(getDict().getUnicharset());
    }
    ratings->delete_matrix_pointers();
    delete ratings;
  }
  getDict().FilterWordChoices();
  // TODO(antonova, eger): check that FilterWordChoices() does not filter
  // out anything useful for word bigram or phrase search.
  // TODO(antonova, eger): when implementing word bigram and phrase search
  // we will need to think carefully about how to replace a word with its
  // alternative choice.
  // In particular it might be required to save the segmentation state
  // associated with the word, so that best_char_choices could be updated
  // by rebuild_current_state() correctly.
  if (save_alt_choices) SaveAltChoices(getDict().getBestChoices(), word);
  char_choices->delete_data_pointers();
  delete char_choices;

  return best_char_choices;
}



/**
 * @name improve_by_chopping
 *
 * Start with the current word of blobs and its classification.  Find
 * the worst blobs and try to divide them up to improve the ratings.
 * As long as ratings are produced by the new blob splitting.  When
 * all the splitting has been accomplished all the ratings memory is
 * reclaimed.
 */
void Wordrec::improve_by_chopping(WERD_RES *word,
                                  BLOB_CHOICE_LIST_VECTOR *char_choices,
                                  STATE *best_state,
                                  BLOB_CHOICE_LIST_VECTOR *best_char_choices,
                                  DANGERR *fixpt,
                                  bool *best_choice_acceptable) {
  inT32 blob_number;
  float old_best;
  bool updated_best_choice = false;

  while (1) {  // improvement loop
    old_best = word->best_choice->rating();
    if (improve_one_blob(word, char_choices,
                         &blob_number, &word->seam_array,
                         fixpt, (fragments_guide_chopper &&
                                 word->best_choice->fragment_mark()),
                                 word->blamer_bundle)) {
      getDict().LogNewSplit(blob_number);
      updated_best_choice =
        getDict().permute_characters(*char_choices, word->best_choice,
                                     word->raw_choice);

      if (old_best > word->best_choice->rating()) {
        set_n_ones(best_state, char_choices->length() - 1);
      } else {
        insert_new_chunk(best_state, blob_number, char_choices->length() - 2);
        fixpt->clear();
      }

      if (chop_debug)
        print_state("best state = ",
          best_state, count_blobs(word->chopped_word->blobs) - 1);
    } else {
      break;
    }

    // Check if we should break from the loop.
    bool done = false;
    bool replaced = false;
    if ((updated_best_choice &&
         (*best_choice_acceptable =
          getDict().AcceptableChoice(char_choices, word->best_choice,
                                     fixpt, CHOPPER_CALLER, &replaced))) ||
        char_choices->length() >= MAX_NUM_CHUNKS) {
      done = true;
    }
    if (replaced) update_blob_classifications(word->chopped_word,
                                              *char_choices);
    if (updated_best_choice) CopyCharChoices(*char_choices, best_char_choices);
    if (done) break;
  }
}


/**********************************************************************
 * select_blob_to_split
 *
 * These are the results of the last classification.  Find a likely
 * place to apply splits.  If none, return -1.
 **********************************************************************/
inT16 Wordrec::select_blob_to_split(const BLOB_CHOICE_LIST_VECTOR &char_choices,
                                    float rating_ceiling,
                                    bool split_next_to_fragment) {
  BLOB_CHOICE_IT blob_choice_it;
  BLOB_CHOICE *blob_choice;
  BLOB_CHOICE_IT temp_it;
  int x;
  float worst = -MAX_FLOAT32;
  int worst_index = -1;
  float worst_near_fragment = -MAX_FLOAT32;
  int worst_index_near_fragment = -1;
  const CHAR_FRAGMENT **fragments = NULL;

  if (chop_debug) {
    if (rating_ceiling < MAX_FLOAT32)
      cprintf("rating_ceiling = %8.4f\n", rating_ceiling);
    else
      cprintf("rating_ceiling = No Limit\n");
  }

  if (split_next_to_fragment && char_choices.length() > 0) {
    fragments = new const CHAR_FRAGMENT *[char_choices.length()];
    if (char_choices.get(0) != NULL) {
      temp_it.set_to_list(char_choices.get(0));
      fragments[0] = getDict().getUnicharset().get_fragment(
          temp_it.data()->unichar_id());
    } else {
      fragments[0] = NULL;
    }
  }

  for (x = 0; x < char_choices.length(); ++x) {
    if (char_choices.get(x) == NULL) {
      if (fragments != NULL) {
        delete[] fragments;
      }
      return x;
    } else {
      blob_choice_it.set_to_list(char_choices.get(x));
      blob_choice = blob_choice_it.data();
      // Populate fragments for the following position.
      if (split_next_to_fragment && x+1 < char_choices.length()) {
        if (char_choices.get(x+1) != NULL) {
          temp_it.set_to_list(char_choices.get(x+1));
          fragments[x+1] = getDict().getUnicharset().get_fragment(
              temp_it.data()->unichar_id());
        } else {
          fragments[x+1] = NULL;
        }
      }
      if (blob_choice->rating() < rating_ceiling &&
          blob_choice->certainty() < tessedit_certainty_threshold) {
        // Update worst and worst_index.
        if (blob_choice->rating() > worst) {
          worst_index = x;
          worst = blob_choice->rating();
        }
        if (split_next_to_fragment) {
          // Update worst_near_fragment and worst_index_near_fragment.
          bool expand_following_fragment =
            (x + 1 < char_choices.length() &&
             fragments[x+1] != NULL && !fragments[x+1]->is_beginning());
          bool expand_preceding_fragment =
            (x > 0 && fragments[x-1] != NULL && !fragments[x-1]->is_ending());
          if ((expand_following_fragment || expand_preceding_fragment) &&
              blob_choice->rating() > worst_near_fragment) {
            worst_index_near_fragment = x;
            worst_near_fragment = blob_choice->rating();
            if (chop_debug) {
              cprintf("worst_index_near_fragment=%d"
                      " expand_following_fragment=%d"
                      " expand_preceding_fragment=%d\n",
                      worst_index_near_fragment,
                      expand_following_fragment,
                      expand_preceding_fragment);
            }
          }
        }
      }
    }
  }
  if (fragments != NULL) {
    delete[] fragments;
  }
  // TODO(daria): maybe a threshold of badness for
  // worst_near_fragment would be useful.
  return worst_index_near_fragment != -1 ?
    worst_index_near_fragment : worst_index;
}

/**********************************************************************
 * select_blob_to_split_from_fixpt
 *
 * Given the fix point from a dictionary search, if there is a single
 * dangerous blob that maps to multiple characters, return that blob
 * index as a place we need to split.  If none, return -1.
 **********************************************************************/
inT16 Wordrec::select_blob_to_split_from_fixpt(DANGERR *fixpt) {
  if (!fixpt)
    return -1;
  for (int i = 0; i < fixpt->size(); i++) {
    if ((*fixpt)[i].begin == (*fixpt)[i].end &&
        (*fixpt)[i].dangerous &&
        (*fixpt)[i].correct_is_ngram) {
      return (*fixpt)[i].begin;
    }
  }
  return -1;
}

/**********************************************************************
 * set_chopper_blame
 *
 * Check whether chops were made at all the character bounding box boundaries
 * in word->truth_word. If not - blame the chopper for an incorrect answer.
 **********************************************************************/
void Wordrec::set_chopper_blame(WERD_RES *word) {
  BlamerBundle *blamer_bundle = word->blamer_bundle;
  assert(blamer_bundle != NULL);
  if (blamer_bundle->NoTruth() || !(blamer_bundle->truth_has_char_boxes) ||
      word->chopped_word->blobs == NULL) {
    return;
  }
  STRING debug;
  bool missing_chop = false;
  TBLOB * curr_blob = word->chopped_word->blobs;
  int b = 0;
  inT16 truth_x;
  while (b < blamer_bundle->truth_word.length() && curr_blob != NULL) {
    truth_x = blamer_bundle->norm_truth_word.BlobBox(b).right();
    if (curr_blob->bounding_box().right() <
        (truth_x - blamer_bundle->norm_box_tolerance)) {
      curr_blob = curr_blob->next;
      continue;  // encountered an extra chop, keep looking
    } else if (curr_blob->bounding_box().right() >
                (truth_x + blamer_bundle->norm_box_tolerance)) {
      missing_chop = true;
      break;
    } else {
      curr_blob = curr_blob->next;
      ++b;
    }
  }
  if (missing_chop || b < blamer_bundle->norm_truth_word.length()) {
    STRING debug;
    char debug_buffer[256];
    if (missing_chop) {
      sprintf(debug_buffer, "Detected missing chop (tolerance=%d) at ",
              blamer_bundle->norm_box_tolerance);
      debug += debug_buffer;
      curr_blob->bounding_box().append_debug(&debug);
      debug.add_str_int("\nNo chop for truth at x=", truth_x);
    } else {
      debug.add_str_int("Missing chops for last ",
                        blamer_bundle->norm_truth_word.length()-b);
      debug += " truth box(es)";
    }
    debug += "\nMaximally chopped word boxes:\n";
    for (curr_blob = word->chopped_word->blobs; curr_blob != NULL;
        curr_blob = curr_blob->next) {
      const TBOX &tbox = curr_blob->bounding_box();
      sprintf(debug_buffer, "(%d,%d)->(%d,%d)\n",
              tbox.left(), tbox.bottom(), tbox.right(), tbox.top());
      debug += debug_buffer;
    }
    debug += "Truth  bounding  boxes:\n";
    for (b = 0; b < blamer_bundle->norm_truth_word.length(); ++b) {
      const TBOX &tbox = blamer_bundle->norm_truth_word.BlobBox(b);
      sprintf(debug_buffer, "(%d,%d)->(%d,%d)\n",
              tbox.left(), tbox.bottom(), tbox.right(), tbox.top());
      debug += debug_buffer;
    }
    blamer_bundle->SetBlame(IRR_CHOPPER, debug, word->best_choice,
                            wordrec_debug_blamer);
  }
}

/**********************************************************************
 * word_associator
 *
 * Reassociate and classify the blobs in a word.  Continue this process
 * until a good answer is found or all the possibilities have been tried.
 **********************************************************************/
MATRIX *Wordrec::word_associator(bool only_create_ratings_matrix,
                                 WERD_RES *word,
                                 STATE *state,
                                 BLOB_CHOICE_LIST_VECTOR *best_char_choices,
                                 DANGERR *fixpt,
                                 STATE *best_state) {
  CHUNKS_RECORD chunks_record;
  BLOB_WEIGHTS blob_weights;
  int x;
  int num_chunks;
  BLOB_CHOICE_IT blob_choice_it;

  num_chunks = array_count(word->seam_array) + 1;

  TBLOB* blobs = word->chopped_word->blobs;
  chunks_record.ratings = record_piece_ratings(blobs);
  chunks_record.chunks = blobs;
  chunks_record.word_res = word;
  chunks_record.splits = word->seam_array;
  chunks_record.chunk_widths = blobs_widths(blobs);
  chunks_record.char_widths = blobs_widths(blobs);
  /* Save chunk weights */
  for (x = 0; x < num_chunks; x++) {
    BLOB_CHOICE_LIST* choices = get_piece_rating(chunks_record.ratings, blobs,
                                                 chunks_record.word_res->denorm,
                                                 word->seam_array, x, x,
                                                 word->blamer_bundle);
    blob_choice_it.set_to_list(choices);
    //This is done by Jetsoft. Divide by zero is possible.
    if (blob_choice_it.data()->certainty() == 0) {
      blob_weights[x]=0;
    } else {
      blob_weights[x] =
        -(inT16) (10 * blob_choice_it.data()->rating() /
                  blob_choice_it.data()->certainty());
    }
  }
  chunks_record.weights = blob_weights;

  if (chop_debug)
    chunks_record.ratings->print(getDict().getUnicharset());

  if (!only_create_ratings_matrix) {
    if (enable_new_segsearch) {
      SegSearch(&chunks_record, word->best_choice,
                best_char_choices, word->raw_choice,
                state, word->blamer_bundle);
    } else {
      best_first_search(&chunks_record, best_char_choices, word,
                        state, fixpt, best_state);
    }
  }

  free_widths(chunks_record.chunk_widths);
  free_widths(chunks_record.char_widths);
  return chunks_record.ratings;
}
}  // namespace tesseract


/**********************************************************************
 * total_containment
 *
 * Check to see if one of these outlines is totally contained within
 * the bounding box of the other.
 **********************************************************************/
inT16 total_containment(TBLOB *blob1, TBLOB *blob2) {
  TBOX box1 = blob1->bounding_box();
  TBOX box2 = blob2->bounding_box();
  return box1.contains(box2) || box2.contains(box1);
}
