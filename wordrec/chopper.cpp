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
#include "chopper.h"
#include "wordclass.h"
#include "makechop.h"
#include "associate.h"
#include "metrics.h"
#include "tordvars.h"
#include "stopper.h"
#include "callcpp.h"
#include "structures.h"
#include "findseam.h"
#include "render.h"
#include "seam.h"
#include "const.h"
#include "freelist.h"
#include "pieces.h"
#include "permute.h"
//#include "tessvars.h"

#include <math.h>

extern int blob_skip;
INT_VAR (repair_unchopped_blobs, 1, "Fix blobs that aren't chopped");

//?extern int tessedit_dangambigs_chop;
double_VAR (tessedit_certainty_threshold, -2.25, "Good blob limit");

/*----------------------------------------------------------------------
          M a c r o s
----------------------------------------------------------------------*/
/**********************************************************************
 * bounds_inside
 *
 * Check to see if the bounding box of one thing is inside the
 * bounding box of another.
 **********************************************************************/
#define bounds_inside(inner_tl,inner_br,outer_tl,outer_br)  \
((inner_tl.x >= outer_tl.x)	&& \
(inner_tl.y <= outer_tl.y)	&& \
(inner_br.x <= outer_br.x)   && \
(inner_br.y >= outer_br.y))     \

/**********************************************************************
 * set_null_choice
 *
 * Set the fields in this choice to be defaulted bad initial values.
 **********************************************************************/
#define set_null_choice(choice)            \
(class_string     (choice) =  NULL,     \
class_lengths     (choice) =  NULL,     \
class_probability (choice) =  MAX_FLOAT32, \
class_certainty   (choice) = -MAX_FLOAT32) \

/*----------------------------------------------------------------------
          F u n c t i o n s
----------------------------------------------------------------------*/
/**********************************************************************
 * preserve_outline_tree
 *
 * Copy the list of outlines.
 **********************************************************************/
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
  if (srcline->child != NULL)
    preserve_outline_tree (srcline->child);
}


/**********************************************************************
 * restore_outline_tree
 *
 * Copy the list of outlines.
 **********************************************************************/
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
  if (srcline->child != NULL)
    restore_outline_tree (srcline->child);
}


/**********************************************************************
 * attempt_blob_chop
 *
 * Try to split the this blob after this one.  Check to make sure that
 * it was successful.
 **********************************************************************/
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


/**********************************************************************
 * any_shared_split_points
 *
 * Return true if any of the splits share a point with this one.
 **********************************************************************/
int any_shared_split_points(SEAMS seam_list, SEAM *seam) {
  int length;
  int index;

  length = array_count (seam_list);
  for (index = 0; index < length; index++)
    if (shared_split_points ((SEAM *) array_value (seam_list, index), seam))
      return TRUE;
  return FALSE;
}


/**********************************************************************
 * check_blob
 *
 * Return true if blob has a non whole outline.
 **********************************************************************/
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


/**********************************************************************
 * improve_one_blob
 *
 * Start with the current word of blobs and its classification.  Find
 * the worst blobs and try to divide it up to improve the ratings.
 *********************************************************************/
CHOICES_LIST improve_one_blob(TWERD *word,
                              CHOICES_LIST char_choices,
                              int fx,
                              inT32 *blob_number,
                              SEAMS *seam_list,
                              DANGERR *fixpt,
                              STATE *this_state,
                              STATE *correct_state,
                              inT32 pass) {
  TBLOB *pblob;
  TBLOB *blob;
  inT16 x = 0;
  float rating_ceiling = MAX_FLOAT32;
  CHOICES answer;
  SEAM *seam;

  do {
    *blob_number = select_blob_to_split (char_choices, rating_ceiling);
    if (*blob_number == -1)
      return (NULL);

    seam = attempt_blob_chop (word, *blob_number, *seam_list);
    if (seam != NULL)
      break;
    /* Must split null blobs */
    answer = (CHOICES) array_value (char_choices, *blob_number);
    if (answer == NIL)
      return (NULL);             /* Try different blob */
    rating_ceiling = best_probability (answer);
  }
  while (!blob_skip);
  /* Split OK */
  for (blob = word->blobs, pblob = NULL; x < *blob_number; x++) {
    pblob = blob;
    blob = blob->next;
  }

  *seam_list =
    insert_seam (*seam_list, *blob_number, seam, blob, word->blobs);

  free_choices ((CHOICES) array_value (char_choices, *blob_number));

  answer =
    classify_blob (pblob, blob, blob->next, NULL, fx, "improve 1:", Red,
    this_state, correct_state, pass, *blob_number);
  char_choices = array_insert (char_choices, *blob_number, answer);

  answer =
    classify_blob (blob, blob->next, blob->next->next, NULL, fx, "improve 2:",
    Yellow, this_state, correct_state, pass, *blob_number + 1);
  array_value (char_choices, *blob_number + 1) = (char *) answer;

  return (char_choices);
}


/**********************************************************************
 * check_seam_order
 *
 * Make sure that each of the splits in this seam match to outlines
 * in this blob.  If any of the splits could not correspond to this
 * blob then there is a problem (and FALSE should be returned to the
 * caller).
 **********************************************************************/
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


/**********************************************************************
 * chop_word_main
 *
 * Classify the blobs in this word and permute the results.  Find the
 * worst blob in the word and chop it up.  Continue this process until
 * a good answer has been found or all the blobs have been chopped up
 * enough.  Return the word level ratings.
 **********************************************************************/
CHOICES_LIST chop_word_main(register TWERD *word,
                            int fx,
                            A_CHOICE *best_choice,
                            A_CHOICE *raw_choice,
                            BOOL8 tester,
                            BOOL8 trainer) {
  TBLOB *pblob;
  TBLOB *blob;
  CHOICES_LIST char_choices;
  int index;
  int did_chopping;
  float rating_limit = 1000.0;
  STATE state;
  SEAMS seam_list = NULL;
  CHOICES match_result;
  MATRIX ratings = NULL;
  DANGERR fixpt;                 /*dangerous ambig */
  inT32 state_count;             //no of states
  inT32 bit_count;               //no of bits
  static STATE best_state;
  static STATE chop_states[64];  //in between states

  state_count = 0;
  set_null_choice(best_choice);
  set_null_choice(raw_choice);

  char_choices = new_choice_list ();

  did_chopping = 0;
  for (blob = word->blobs, pblob = NULL, index = 0; blob != NULL;
  blob = blob->next, index++) {
    match_result =
      (CHOICES) classify_blob (pblob, blob, blob->next, NULL, fx,
      "chop_word:", Green, &chop_states[0],
      &best_state, matcher_pass, index);
    if (match_result == NULL)
      cprintf("Null classifier output!\n");
    char_choices = array_push (char_choices, match_result);
    pblob = blob;
  }
  bit_count = index - 1;
  permute_characters(char_choices, rating_limit, best_choice, raw_choice);
  set_n_ones (&state, array_count (char_choices) - 1);
  if (matcher_fp != NULL) {
    if (matcher_pass == 0) {
      bits_in_states = bit_count;
      chop_states[state_count] = state;
    }
    state_count++;
  }

  if (!AcceptableChoice (char_choices, best_choice, raw_choice, &fixpt)
    || ((tester || trainer)
  && strcmp (word->correct, class_string (best_choice)))) {
    did_chopping = 1;
    if (first_pass)
      words_chopped1++;
    else
      words_chopped2++;

    seam_list = start_seam_list (word->blobs);

    if (chop_enable)
      improve_by_chopping(word,
                          &char_choices,
                          fx,
                          &state,
                          best_choice,
                          raw_choice,
                          &seam_list,
                          &fixpt,
                          chop_states,
                          &state_count,
                          &best_state,
                          matcher_pass);

    if (chop_debug)
      print_seams ("Final seam list:", seam_list);
    if ((enable_assoc &&
      !AcceptableChoice (char_choices, best_choice, raw_choice, NULL))
      || ((tester || trainer)
    && strcmp (word->correct, class_string (best_choice)))) {
      ratings = word_associator (word->blobs, seam_list, &state, fx,
        best_choice, raw_choice, word->correct,
        /*0, */ &fixpt,
        &best_state, matcher_pass);
    }
    bits_in_states = bit_count + state_count - 1;

  }
  if (ratings != NULL)
    free_matrix(ratings);
  if (did_chopping || tester || trainer)
    char_choices = rebuild_current_state (word->blobs, seam_list, &state,
      char_choices, fx);
  if (seam_list != NULL)
    free_seam_list(seam_list);
  if (matcher_fp != NULL) {
    best_state = state;
  }
  FilterWordChoices();
  return char_choices;
}


/**********************************************************************
 * improve_by_chopping
 *
 * Start with the current word of blobs and its classification.  Find
 * the worst blobs and try to divide them up to improve the ratings.
 * As long as ratings are produced by the new blob splitting.  When
 * all the splitting has been accomplished all the ratings memory is
 * reclaimed.
 **********************************************************************/
void improve_by_chopping(register TWERD *word,
                         CHOICES_LIST *char_choices,
                         int fx,
                         STATE *best_state,
                         A_CHOICE *best_choice,
                         A_CHOICE *raw_choice,
                         SEAMS *seam_list,
                         DANGERR *fixpt,
                         STATE *chop_states,
                         inT32 *state_count,
                         STATE *correct_state,
                         inT32 pass) {
  inT32 blob_number;
  inT32 index;                   //to states
  CHOICES_LIST choices = *char_choices;
  float old_best;
  int fixpt_valid = 1;
  static inT32 old_count;        //from pass1

  do {
                                 /* Improvement loop */
    if (!fixpt_valid)
      fixpt->index = -1;
    old_best = class_probability (best_choice);
    choices = improve_one_blob (word, *char_choices, fx,
      &blob_number, seam_list, fixpt,
      chop_states + *state_count, correct_state,
      pass);
    if (choices != NULL) {
      LogNewSplit(blob_number);
      permute_characters (choices,
        class_probability (best_choice),
        best_choice, raw_choice);
      *char_choices = choices;

      if (old_best > class_probability (best_choice)) {
        set_n_ones (best_state, array_count (*char_choices) - 1);
        fixpt_valid = 1;
      }
      else {
        insert_new_chunk (best_state, blob_number,
          array_count (*char_choices) - 2);
        fixpt_valid = 0;
      }
      if (*state_count > 0) {
        if (pass == 0) {
          for (index = 0; index < *state_count; index++)
            insert_new_chunk (&chop_states[index], blob_number,
              array_count (*char_choices) - 2);
          set_n_ones (&chop_states[index],
            array_count (*char_choices) - 1);
        }
        (*state_count)++;
      }

      if (chop_debug)
        print_state ("best state = ",
          best_state, count_blobs (word->blobs) - 1);
      if (first_pass)
        chops_performed1++;
      else
        chops_performed2++;

    }
  }
  while (choices &&
    !AcceptableChoice (*char_choices, best_choice, raw_choice, fixpt) &&
    !blob_skip && array_count (*char_choices) < MAX_NUM_CHUNKS);
  if (pass == 0)
    old_count = *state_count;
  else {
    if (old_count != *state_count)
      fprintf (matcher_fp,
        "Mis-matched state counts, " INT32FORMAT " pass1, "
        INT32FORMAT " pass2\n", old_count, *state_count);
  }
  if (!fixpt_valid)
    fixpt->index = -1;
}


/**********************************************************************
 * select_blob_to_split
 *
 * These are the results of the last classification.  Find a likely
 * place to apply splits.
 **********************************************************************/
inT16 select_blob_to_split(CHOICES_LIST char_choices, float rating_ceiling) {
  CHOICES this_choice;
  int x;
  float worst = -MAX_FLOAT32;
  int worst_index = -1;

  if (chop_debug) {
    if (rating_ceiling < MAX_FLOAT32)
      cprintf ("rating_ceiling = %8.4f\n", rating_ceiling);
    else
      cprintf ("rating_ceiling = No Limit\n");
  }

  for_each_choice(char_choices, x) {
    this_choice = (CHOICES) array_value (char_choices, x);
    if (this_choice == NIL) {
      return (x);
    }
    else {
      if (best_probability (this_choice) > worst &&
        best_probability (this_choice) < rating_ceiling &&
      best_certainty (this_choice) < tessedit_certainty_threshold) {
        worst_index = x;
        worst = best_probability (this_choice);
      }
    }
  }

  if (chop_debug)
    cprintf ("blob_number = %4d\n", worst_index);

  return (worst_index);
}


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
MATRIX word_associator(TBLOB *blobs,
                       SEAMS seams,
                       STATE *state,
                       int fxid,
                       A_CHOICE *best_choice,
                       A_CHOICE *raw_choice,
                       char *correct,
                       DANGERR *fixpt,
                       STATE *best_state,
                       inT32 pass) {
  CHUNKS_RECORD chunks_record;
  BLOB_WEIGHTS blob_weights;
  int x;
  int num_chunks;
  A_CHOICE *this_choice;

  num_chunks = array_count (seams) + 1;

  chunks_record.chunks = blobs;
  chunks_record.splits = seams;
  chunks_record.ratings = record_piece_ratings (blobs);
  chunks_record.char_widths = blobs_widths (blobs);
  chunks_record.chunk_widths = blobs_widths (blobs);
  chunks_record.fx = fxid;
  /* Save chunk weights */
  for (x = 0; x < num_chunks; x++) {
    this_choice =
      (A_CHOICE *) first_node (matrix_get (chunks_record.ratings, x, x));

	//This is done by Jetsoft. Divide by zero is possible.
	if (class_certainty (this_choice)==0)
		blob_weights[x]=0;
	else
		blob_weights[x] = -(inT16) (10 * class_probability (this_choice) /
				class_certainty (this_choice));

	//

  }
  chunks_record.weights = blob_weights;

  if (chop_debug)
    print_matrix (chunks_record.ratings);
  best_first_search(&chunks_record,
                    best_choice,
                    raw_choice,
                    state,
                    fixpt,
                    best_state,
                    pass);

  free_widths (chunks_record.chunk_widths);
  free_widths (chunks_record.char_widths);
  return chunks_record.ratings;
}
