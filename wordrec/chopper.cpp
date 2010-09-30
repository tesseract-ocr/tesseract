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
#include "choices.h"
#include "const.h"
#include "findseam.h"
#include "freelist.h"
#include "globals.h"
#include "makechop.h"
#include "metrics.h"
#include "render.h"
#include "permute.h"
#include "pieces.h"
#include "seam.h"
#include "stopper.h"
#include "structures.h"
#include "tordvars.h"
#include "unicharset.h"
#include "wordclass.h"
#include "wordrec.h"

// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif

INT_VAR (repair_unchopped_blobs, 1, "Fix blobs that aren't chopped");

//?extern int tessedit_dangambigs_chop;
double_VAR(tessedit_certainty_threshold, -2.25, "Good blob limit");

BOOL_VAR(fragments_guide_chopper, FALSE,
         "Use information from fragments to guide chopping process");

/*----------------------------------------------------------------------
          M a c r o s
----------------------------------------------------------------------*/
/**
 * @name bounds_inside
 *
 * Check to see if the bounding box of one thing is inside the
 * bounding box of another.
 */
#define bounds_inside(inner_tl,inner_br,outer_tl,outer_br)  \
((inner_tl.x >= outer_tl.x)	&& \
(inner_tl.y <= outer_tl.y)	&& \
(inner_br.x <= outer_br.x)   && \
(inner_br.y >= outer_br.y))     \

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
  if (srcline != NULL && srcline->child != NULL)
    preserve_outline_tree (srcline->child);
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
      oldedgept(deadpt);
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
  if (srcline != NULL && srcline->child != NULL)
    restore_outline_tree (srcline->child);
}


/**
 * @name attempt_blob_chop
 *
 * Try to split the this blob after this one.  Check to make sure that
 * it was successful.
 */
SEAM *attempt_blob_chop(TWERD *word, inT32 blob_number, SEAMS seam_list) {
  TBLOB *blob;
  TBLOB *other_blob;
  SEAM *seam;
  TBLOB *last_blob;
  TBLOB *next_blob;
  inT16 x;

  if (first_pass)
    chops_attempted1++;
  else
    chops_attempted2++;

  last_blob = NULL;
  blob = word->blobs;
  for (x = 0; x < blob_number; x++) {
    last_blob = blob;
    blob = blob->next;
  }
  next_blob = blob->next;

  if (repair_unchopped_blobs)
    preserve_outline_tree (blob->outlines);
  other_blob = newblob ();       /* Make new blob */
  other_blob->next = blob->next;
  other_blob->outlines = NULL;
  blob->next = other_blob;

  seam = pick_good_seam (blob);
  if (chop_debug) {
    if (seam != NULL) {
      print_seam ("Good seam picked=", seam);
    }
    else
      cprintf ("\n** no seam picked *** \n");
  }
  if (seam) {
    apply_seam(blob, other_blob, seam);
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
    }
    else {
      oldblob(other_blob);
    }

    if (repair_unchopped_blobs)
      restore_outline_tree (blob->outlines);
    return (NULL);
  }
  return (seam);
}


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
bool Wordrec::improve_one_blob(TWERD *word,
                               BLOB_CHOICE_LIST_VECTOR *char_choices,
                               int fx,
                               inT32 *blob_number,
                               SEAMS *seam_list,
                               DANGERR *fixpt,
                               bool split_next_to_fragment) {
  TBLOB *pblob;
  TBLOB *blob;
  inT16 x = 0;
  float rating_ceiling = MAX_FLOAT32;
  BLOB_CHOICE_LIST *answer;
  BLOB_CHOICE_IT answer_it;
  SEAM *seam;

  do {
    *blob_number = select_blob_to_split(*char_choices, rating_ceiling,
                                        split_next_to_fragment);
    if (chop_debug)
      cprintf("blob_number = %d\n", *blob_number);
    if (*blob_number == -1)
      return false;

    seam = attempt_blob_chop (word, *blob_number, *seam_list);
    if (seam != NULL)
      break;
    /* Must split null blobs */
    answer = char_choices->get(*blob_number);
    if (answer == NULL)
      return false;
    answer_it.set_to_list(answer);
    rating_ceiling = answer_it.data()->rating();  // try a different blob
  } while (!tord_blob_skip);
  /* Split OK */
  for (blob = word->blobs, pblob = NULL; x < *blob_number; x++) {
    pblob = blob;
    blob = blob->next;
  }

  *seam_list =
    insert_seam (*seam_list, *blob_number, seam, blob, word->blobs);

  delete char_choices->get(*blob_number);

  answer = classify_blob(pblob, blob, blob->next, NULL, "improve 1:", Red);
  char_choices->insert(answer, *blob_number);

  answer = classify_blob(blob, blob->next, blob->next->next, NULL,
                         "improve 2:", Yellow);
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
  ASSERT_HOST(unichar_id!=INVALID_UNICHAR_ID);
  BLOB_CHOICE_IT answer_it(answer);
  BLOB_CHOICE *modified_blob = new BLOB_CHOICE(unichar_id,
                                             answer_it.data()->rating(),
                                             answer_it.data()->certainty(),
                                             answer_it.data()->config(),
                                             answer_it.data()->script_id());
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
  TBLOB *pblob;
  TBLOB *blob;
  inT16 x = 0;
  float rating_ceiling = MAX_FLOAT32;
  BLOB_CHOICE_LIST *answer;
  BLOB_CHOICE_IT answer_it;
  SEAM *seam;
  UNICHAR_ID unichar_id = 0;
  int left_chop_index = 0;

  do {
    *blob_number = select_blob_to_split(*char_choices, rating_ceiling,
                                        false);
    if (chop_debug)
      cprintf("blob_number = %d\n", *blob_number);
    if (*blob_number == -1)
      return false;
    seam = attempt_blob_chop(word, *blob_number, *seam_list);
    if (seam != NULL)
      break;
    /* Must split null blobs */
    answer = char_choices->get(*blob_number);
    if (answer == NULL)
      return false;
    answer_it.set_to_list(answer);
    rating_ceiling = answer_it.data()->rating();  // try a different blob
  } while (!tord_blob_skip);
  /* Split OK */
  for (blob = word->blobs, pblob = NULL; x < *blob_number; x++) {
    pblob = blob;
    blob = blob->next;
  }
  *seam_list =
    insert_seam(*seam_list, *blob_number, seam, blob, word->blobs);

  answer = char_choices->get(*blob_number);
  answer_it.set_to_list(answer);
  unichar_id = answer_it.data()->unichar_id();
  left_chop_index = atoi(unicharset.id_to_unichar(unichar_id));

  delete char_choices->get(*blob_number);
  // combine confidence w/ serial #
  answer = classify_blob(pblob, blob, blob->next, NULL, "improve 1:", Red);
  modify_blob_choice(answer, left_chop_index);
  char_choices->insert(answer, *blob_number);

  answer = classify_blob(blob, blob->next, blob->next->next, NULL,
                         "improve 2:", Yellow);
  modify_blob_choice(answer, ++*right_chop_index);
  char_choices->set(answer, *blob_number + 1);
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
BLOB_CHOICE_LIST_VECTOR *Wordrec::chop_word_main(register TWERD *word,
                                                 int fx,
                                                 WERD_CHOICE *best_choice,
                                                 WERD_CHOICE *raw_choice,
                                                 BOOL8 tester,
                                                 BOOL8 trainer) {
  TBLOB *pblob;
  TBLOB *blob;
  int index;
  int did_chopping;
  float rating_limit = 1000.0;
  STATE state;
  SEAMS seam_list = start_seam_list(word->blobs);
  BLOB_CHOICE_LIST *match_result;
  MATRIX *ratings = NULL;
  DANGERR fixpt;                 /*dangerous ambig */
  inT32 state_count;             //no of states
  inT32 bit_count;               //no of bits
  static STATE best_state;
  static STATE chop_states[64];  //in between states

  state_count = 0;
  best_choice->make_bad();
  raw_choice->make_bad();

  BLOB_CHOICE_LIST_VECTOR *char_choices = new BLOB_CHOICE_LIST_VECTOR();

  did_chopping = 0;
  for (blob = word->blobs, pblob = NULL, index = 0;
       blob != NULL; blob = blob->next, index++) {
    match_result = classify_blob(pblob, blob, blob->next, NULL,
                                 "chop_word:", Green);
    if (match_result == NULL)
      cprintf("Null classifier output!\n");
    *char_choices += match_result;
    pblob = blob;
  }
  bit_count = index - 1;
  getDict().permute_characters(*char_choices, rating_limit,
                               best_choice, raw_choice);
  set_n_ones(&state, char_choices->length() - 1);
  if (matcher_fp != NULL) {
    bits_in_states = bit_count;
    chop_states[state_count] = state;
    state_count++;
  }
  bool replaced = false;
  if (!getDict().AcceptableChoice(char_choices, best_choice, *raw_choice,
                                  &fixpt, CHOPPER_CALLER, &replaced) ||
      ((tester || trainer) &&
       strcmp(word->correct, best_choice->unichar_string().string()))) {
    if (replaced) update_blob_classifications(word, *char_choices);
    did_chopping = 1;
    if (first_pass)
      words_chopped1++;
    else
      words_chopped2++;

    if (chop_enable)
      improve_by_chopping(word,
                          char_choices,
                          fx,
                          &state,
                          best_choice,
                          raw_choice,
                          &seam_list,
                          &fixpt,
                          chop_states,
                          &state_count);
    if (chop_debug)
      print_seams ("Final seam list:", seam_list);

    // The force_word_assoc is almost redundant to enable_assoc.  However,
    // it is not conditioned on the dict behavior.  For CJK, we need to force
    // the associator to be invoked.  When we figure out the exact behavior
    // of dict on CJK, we can remove the flag if it turns out to be redundant.
    if ((wordrec_enable_assoc &&
         !getDict().AcceptableChoice(char_choices, best_choice, *raw_choice,
                                     NULL, CHOPPER_CALLER, &replaced)) ||
        force_word_assoc ||
        ((tester || trainer) &&
         strcmp(word->correct, best_choice->unichar_string().string()))) {
      ratings = word_associator (word->blobs, seam_list, &state, fx,
        best_choice, raw_choice, word->correct,
        /*0, */ &fixpt, &best_state);
    }
    bits_in_states = bit_count + state_count - 1;
  }
  if (replaced) update_blob_classifications(word, *char_choices);

  char_choices =
    rebuild_current_state(word->blobs, seam_list, &state, char_choices, fx,
                          (did_chopping || tester || trainer), *best_choice,
                          ratings);

  if (ratings != NULL) {
    ratings->delete_matrix_pointers();
    delete ratings;
  }
  if (seam_list != NULL)
    free_seam_list(seam_list);
  if (matcher_fp != NULL) {
    best_state = state;
  }
  getDict().FilterWordChoices();
  return char_choices;
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
void Wordrec::improve_by_chopping(register TWERD *word,
                                  BLOB_CHOICE_LIST_VECTOR *char_choices,
                                  int fx,
                                  STATE *best_state,
                                  WERD_CHOICE *best_choice,
                                  WERD_CHOICE *raw_choice,
                                  SEAMS *seam_list,
                                  DANGERR *fixpt,
                                  STATE *chop_states,
                                  inT32 *state_count) {
  inT32 blob_number;
  inT32 index;                   //to states
  float old_best;
  int fixpt_valid = 1;
  static inT32 old_count;        //from pass1
  bool replaced = false;

  do {  // improvement loop
    if (replaced) update_blob_classifications(word, *char_choices);
    if (!fixpt_valid)
      fixpt->index = -1;
    old_best = best_choice->rating();
    if (improve_one_blob(word, char_choices, fx, &blob_number, seam_list,
                         fixpt, (fragments_guide_chopper &&
                                 best_choice->fragment_mark()))) {
      getDict().LogNewSplit(blob_number);
      getDict().permute_characters(*char_choices, best_choice->rating(),
                                   best_choice, raw_choice);

      if (old_best > best_choice->rating()) {
        set_n_ones(best_state, char_choices->length() - 1);
        fixpt_valid = 1;
      }
      else {
        insert_new_chunk(best_state, blob_number, char_choices->length() - 2);
        fixpt_valid = 0;
      }
      if (*state_count > 0) {
        for (index = 0; index < *state_count; index++) {
          insert_new_chunk(&chop_states[index], blob_number,
                           char_choices->length() - 2);
        }
        set_n_ones(&chop_states[index], char_choices->length() - 1);
        (*state_count)++;
      }

      if (chop_debug)
        print_state ("best state = ",
          best_state, count_blobs (word->blobs) - 1);
      if (first_pass)
        chops_performed1++;
      else
        chops_performed2++;
    } else {
      break;
    }
  } while (!getDict().AcceptableChoice(char_choices, best_choice, *raw_choice,
                                       fixpt, CHOPPER_CALLER, &replaced) &&
           !tord_blob_skip && char_choices->length() < MAX_NUM_CHUNKS);
  if (replaced) update_blob_classifications(word, *char_choices);
  old_count = *state_count;
  if (!fixpt_valid)
    fixpt->index = -1;
}


/**********************************************************************
 * select_blob_to_split
 *
 * These are the results of the last classification.  Find a likely
 * place to apply splits.
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
}  // namespace tesseract


/**********************************************************************
 * start_seam_list
 *
 * Initialize a list of seams that match the original number of blobs
 * present in the starting segmentation.  Each of the seams created
 * by this routine have location information only.
 **********************************************************************/
SEAMS start_seam_list(TBLOB *blobs) {
  TBLOB *blob;
  SEAMS seam_list;
  TPOINT topleft;
  TPOINT botright;
  int location;
  /* Seam slot per char */
  seam_list = new_seam_list ();

  for (blob = blobs; blob->next != NULL; blob = blob->next) {

    blob_bounding_box(blob, &topleft, &botright);
    location = botright.x;
    blob_bounding_box (blob->next, &topleft, &botright);
    location += topleft.x;
    location /= 2;

    seam_list = add_seam (seam_list,
      new_seam (0.0, location, NULL, NULL, NULL));
  }

  return (seam_list);
}


/**********************************************************************
 * total_containment
 *
 * Check to see if one of these outlines is totally contained within
 * the bounding box of the other.
 **********************************************************************/
inT16 total_containment(TBLOB *blob1, TBLOB *blob2) {
  TPOINT topleft1;
  TPOINT botright1;
  TPOINT topleft2;
  TPOINT botright2;

  blob_bounding_box(blob1, &topleft1, &botright1);
  blob_bounding_box(blob2, &topleft2, &botright2);

  return (bounds_inside (topleft1, botright1, topleft2, botright2) ||
    bounds_inside (topleft2, botright2, topleft1, botright1));
}


/**********************************************************************
 * word_associator
 *
 * Reassociate and classify the blobs in a word.  Continue this process
 * until a good answer is found or all the possibilities have been tried.
 **********************************************************************/
namespace tesseract {
MATRIX *Wordrec::word_associator(TBLOB *blobs,
                                 SEAMS seams,
                                 STATE *state,
                                 int fxid,
                                 WERD_CHOICE *best_choice,
                                 WERD_CHOICE *raw_choice,
                                 char *correct,
                                 DANGERR *fixpt,
                                 STATE *best_state) {
  CHUNKS_RECORD chunks_record;
  BLOB_WEIGHTS blob_weights;
  int x;
  int num_chunks;
  BLOB_CHOICE_IT blob_choice_it;

  num_chunks = array_count (seams) + 1;

  chunks_record.chunks = blobs;
  chunks_record.splits = seams;
  chunks_record.ratings = record_piece_ratings (blobs);
  chunks_record.char_widths = blobs_widths (blobs);
  chunks_record.chunk_widths = blobs_widths (blobs);
  chunks_record.fx = fxid;
  /* Save chunk weights */
  for (x = 0; x < num_chunks; x++) {
    BLOB_CHOICE_LIST* choices = get_piece_rating(chunks_record.ratings,
                                                 blobs, seams, x, x);
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

  best_first_search(&chunks_record,
                    best_choice,
                    raw_choice,
                    state,
                    fixpt,
                    best_state);

  free_widths (chunks_record.chunk_widths);
  free_widths (chunks_record.char_widths);
  return chunks_record.ratings;
}
}  // namespace tesseract


