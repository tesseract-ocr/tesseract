/**********************************************************************
 * File:        reject.cpp  (Formerly reject.c)
 * Description: Rejection functions used in tessedit
 * Author:		Phil Cheatle
 * Created:		Wed Sep 23 16:50:21 BST 1992
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
#endif

#include "mfcpch.h"

#include          "tessvars.h"
#ifdef __UNIX__
#include          <assert.h>
#include          <errno.h>
#endif
#include          "scanutils.h"
#include          <ctype.h>
#include          <string.h>
//#include                                      "tessbox.h"
#include          "memry.h"
#include          "reject.h"
#include          "tfacep.h"
#include          "mainblk.h"
#include          "charcut.h"
#include          "imgs.h"
#include          "scaleimg.h"
#include          "control.h"
#include          "docqual.h"
#include          "secname.h"
#include          "globals.h"

/* #define SECURE_NAMES done in secnames.h when necessary */

//extern "C" {
#include          "callnet.h"
//}
#include "tesseractclass.h"
#include          "notdll.h"

// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif

CLISTIZEH (STRING) CLISTIZE (STRING)
#define EXTERN
EXTERN
INT_VAR (tessedit_reject_mode, 0, "Rejection algorithm");
EXTERN
INT_VAR (tessedit_ok_mode, 5, "Acceptance decision algorithm");
EXTERN
BOOL_VAR (tessedit_use_nn, FALSE, "");
EXTERN
BOOL_VAR (tessedit_rejection_debug, FALSE, "Adaption debug");
EXTERN
BOOL_VAR (tessedit_rejection_stats, FALSE, "Show NN stats");
EXTERN
BOOL_VAR (tessedit_flip_0O, TRUE, "Contextual 0O O0 flips");
EXTERN
double_VAR (tessedit_lower_flip_hyphen, 1.5,
"Aspect ratio dot/hyphen test");
EXTERN
double_VAR (tessedit_upper_flip_hyphen, 1.8,
"Aspect ratio dot/hyphen test");

EXTERN
BOOL_VAR (rej_trust_doc_dawg, FALSE,
"Use DOC dawg in 11l conf. detector");
EXTERN
BOOL_VAR (rej_1Il_use_dict_word, FALSE, "Use dictword test");
EXTERN
BOOL_VAR (rej_1Il_trust_permuter_type, TRUE, "Dont double check");

EXTERN
BOOL_VAR (one_ell_conflict_default, TRUE, "one_ell_conflict default");
EXTERN
BOOL_VAR (show_char_clipping, FALSE, "Show clip image window?");
EXTERN
BOOL_VAR (nn_debug, FALSE, "NN DEBUGGING?");
EXTERN
BOOL_VAR (nn_reject_debug, FALSE, "NN DEBUG each char?");
EXTERN
BOOL_VAR (nn_lax, FALSE, "Use 2nd rate matches");
EXTERN
BOOL_VAR (nn_double_check_dict, FALSE, "Double check");
EXTERN
BOOL_VAR (nn_conf_double_check_dict, TRUE,
"Double check for confusions");
EXTERN
BOOL_VAR (nn_conf_1Il, TRUE, "NN use 1Il conflicts");
EXTERN
BOOL_VAR (nn_conf_Ss, TRUE, "NN use Ss conflicts");
EXTERN
BOOL_VAR (nn_conf_hyphen, TRUE, "NN hyphen conflicts");
EXTERN
BOOL_VAR (nn_conf_test_good_qual, FALSE, "NN dodgy 1Il cross check");
EXTERN
BOOL_VAR (nn_conf_test_dict, TRUE, "NN dodgy 1Il cross check");
EXTERN
BOOL_VAR (nn_conf_test_sensible, TRUE, "NN dodgy 1Il cross check");
EXTERN
BOOL_VAR (nn_conf_strict_on_dodgy_chs, TRUE,
"Require stronger NN match");
EXTERN
double_VAR (nn_dodgy_char_threshold, 0.99, "min accept score");
EXTERN
INT_VAR (nn_conf_accept_level, 4, "NN accept dodgy 1Il matches? ");
EXTERN
INT_VAR (nn_conf_initial_i_level, 3,
"NN accept initial Ii match level ");

EXTERN
BOOL_VAR (no_unrej_dubious_chars, TRUE, "Dubious chars next to reject?");
EXTERN
BOOL_VAR (no_unrej_no_alphanum_wds, TRUE, "Stop unrej of non A/N wds?");
EXTERN
BOOL_VAR (no_unrej_1Il, FALSE, "Stop unrej of 1Ilchars?");
EXTERN
BOOL_VAR (rej_use_tess_accepted, TRUE, "Individual rejection control");
EXTERN
BOOL_VAR (rej_use_tess_blanks, TRUE, "Individual rejection control");
EXTERN
BOOL_VAR (rej_use_good_perm, TRUE, "Individual rejection control");
EXTERN
BOOL_VAR (rej_use_sensible_wd, FALSE, "Extend permuter check");
EXTERN
BOOL_VAR (rej_alphas_in_number_perm, FALSE, "Extend permuter check");

EXTERN
double_VAR (rej_whole_of_mostly_reject_word_fract, 0.85,
"if >this fract");
EXTERN
INT_VAR (rej_mostly_reject_mode, 1,
"0-never, 1-afterNN, 2-after new xht");
EXTERN
double_VAR (tessed_fullstop_aspect_ratio, 1.2,
"if >this fract then reject");

EXTERN
INT_VAR (net_image_width, 40, "NN input image width");
EXTERN
INT_VAR (net_image_height, 36, "NN input image height");
EXTERN
INT_VAR (net_image_x_height, 22, "NN input image x_height");
EXTERN
INT_VAR (tessedit_image_border, 2, "Rej blbs near image edge limit");

/*
  Net input is assumed to have (net_image_width * net_image_height) input
  units of image pixels, followed by 0, 1, or N units representing the
  baseline position. 0 implies no baseline information. 1 implies a floating
  point value. N implies a "guage" of N units. For any char an initial set
  of these are ON, the remainder OFF to indicate the "level" of the
  baseline.

  HOWEVER!!!  NOTE THAT EACH NEW INPUT LAYER FORMAT EXPECTS TO BE RUN WITH A
  DIFFERENT tessed/netmatch/nmatch.c MODULE. - These are classic C modules
  generated by aspirin with HARD CODED CONSTANTS
*/

EXTERN
INT_VAR (net_bl_nodes, 20, "Number of baseline nodes");

EXTERN
double_VAR (nn_reject_threshold, 0.5, "NN min accept score");
EXTERN
double_VAR (nn_reject_head_and_shoulders, 0.6, "top scores sep factor");

/* NOTE - ctoh doesn't handle "=" properly, hence \075 */
EXTERN
STRING_VAR (ok_single_ch_non_alphanum_wds, "-?\075",
"Allow NN to unrej");
EXTERN
STRING_VAR (ok_repeated_ch_non_alphanum_wds, "-?*\075",
"Allow NN to unrej");
EXTERN
STRING_VAR (conflict_set_I_l_1, "Il1[]", "Il1 conflict set");
EXTERN
STRING_VAR (conflict_set_S_s, "Ss$", "Ss conflict set");
EXTERN
STRING_VAR (conflict_set_hyphen, "-_~", "hyphen conflict set");
EXTERN
STRING_VAR (dubious_chars_left_of_reject, "!'+`()-./\\<>;:^_,~\"",
"Unreliable chars");
EXTERN
STRING_VAR (dubious_chars_right_of_reject, "!'+`()-./\\<>;:^_,~\"",
"Unreliable chars");

EXTERN
INT_VAR (min_sane_x_ht_pixels, 8, "Reject any x-ht lt or eq than this");

/*************************************************************************
 * set_done()
 *
 * Set the done flag based on the word acceptability criteria
 *************************************************************************/

namespace tesseract {
void Tesseract::set_done(  //set done flag
                         WERD_RES *word,
                         inT16 pass) {
  /*
  0: Original heuristic used in Tesseract and Ray's prototype Resaljet
  */
  if (tessedit_ok_mode == 0) {
    /* NOTE - done even if word contains some or all spaces !!! */
    word->done = word->tess_accepted;
  }
  /*
  1: Reject words containing blanks and on pass 1 reject I/l/1 conflicts
  */
  else if (tessedit_ok_mode == 1) {
    word->done = word->tess_accepted &&
      (strchr (word->best_choice->unichar_string().string (), ' ') == NULL);

    if (word->done && (pass == 1) && one_ell_conflict (word, FALSE))
      word->done = FALSE;
  }
  /*
  2: as 1 + only accept dict words or numerics in pass 1
  */
  else if (tessedit_ok_mode == 2) {
    word->done = word->tess_accepted &&
      (strchr (word->best_choice->unichar_string().string (), ' ') == NULL);

    if (word->done && (pass == 1) && one_ell_conflict (word, FALSE))
      word->done = FALSE;

    if (word->done &&
      (pass == 1) &&
      (word->best_choice->permuter () != SYSTEM_DAWG_PERM) &&
      (word->best_choice->permuter () != FREQ_DAWG_PERM) &&
      (word->best_choice->permuter () != USER_DAWG_PERM) &&
    (word->best_choice->permuter () != NUMBER_PERM)) {
      #ifndef SECURE_NAMES
      if (tessedit_rejection_debug)
        tprintf ("\nVETO Tess accepting poor word \"%s\"\n",
          word->best_choice->unichar_string().string ());
      #endif
      word->done = FALSE;
    }
  }
  /*
  3: as 2 + only accept dict words or numerics in pass 2 as well
  */
  else if (tessedit_ok_mode == 3) {
    word->done = word->tess_accepted &&
      (strchr (word->best_choice->unichar_string().string (), ' ') == NULL);

    if (word->done && (pass == 1) && one_ell_conflict (word, FALSE))
      word->done = FALSE;

    if (word->done &&
      (word->best_choice->permuter () != SYSTEM_DAWG_PERM) &&
      (word->best_choice->permuter () != FREQ_DAWG_PERM) &&
      (word->best_choice->permuter () != USER_DAWG_PERM) &&
    (word->best_choice->permuter () != NUMBER_PERM)) {
      #ifndef SECURE_NAMES
      if (tessedit_rejection_debug)
        tprintf ("\nVETO Tess accepting poor word \"%s\"\n",
          word->best_choice->unichar_string().string ());
      #endif
      word->done = FALSE;
    }
  }
  /*
  4: as 2 + reject dict ambigs in pass 1
  */
  else if (tessedit_ok_mode == 4) {
    word->done = word->tess_accepted &&
      (strchr (word->best_choice->unichar_string().string (), ' ') == NULL);

    if (word->done && (pass == 1) && one_ell_conflict (word, FALSE))
      word->done = FALSE;

    if (word->done &&
      (pass == 1) &&
      (((word->best_choice->permuter () != SYSTEM_DAWG_PERM) &&
      (word->best_choice->permuter () != FREQ_DAWG_PERM) &&
      (word->best_choice->permuter () != USER_DAWG_PERM) &&
      (word->best_choice->permuter () != NUMBER_PERM)) ||
    (test_ambig_word (word)))) {
      #ifndef SECURE_NAMES
      if (tessedit_rejection_debug)
        tprintf ("\nVETO Tess accepting poor word \"%s\"\n",
          word->best_choice->unichar_string().string ());
      #endif
      word->done = FALSE;
    }
  }
  /*
  5: as 3 + reject dict ambigs in both passes
  */
  else if (tessedit_ok_mode == 5) {
    word->done = word->tess_accepted &&
      (strchr (word->best_choice->unichar_string().string (), ' ') == NULL);

    if (word->done && (pass == 1) && one_ell_conflict (word, FALSE))
      word->done = FALSE;

    if (word->done &&
      (((word->best_choice->permuter () != SYSTEM_DAWG_PERM) &&
      (word->best_choice->permuter () != FREQ_DAWG_PERM) &&
      (word->best_choice->permuter () != USER_DAWG_PERM) &&
      (word->best_choice->permuter () != NUMBER_PERM)) ||
    (test_ambig_word (word)))) {
      #ifndef SECURE_NAMES
      if (tessedit_rejection_debug)
        tprintf ("\nVETO Tess accepting poor word \"%s\"\n",
          word->best_choice->unichar_string().string ());
      #endif
      word->done = FALSE;
    }
  }

  else {
    tprintf ("BAD tessedit_ok_mode\n");
    err_exit();
  }
}


/*************************************************************************
 * make_reject_map()
 *
 * Sets the done flag to indicate whether the resylt is acceptable.
 *
 * Sets a reject map for the word.
 *************************************************************************/
void Tesseract::make_reject_map(      //make rej map for wd //detailed results
                                WERD_RES *word,
                                BLOB_CHOICE_LIST_CLIST *blob_choices,
                                ROW *row,
                                inT16 pass  //1st or 2nd?
                               ) {
  int i;
  int offset;

  flip_0O(word);
  check_debug_pt (word, -1);     //For trap only
  set_done(word, pass);  //Set acceptance
  word->reject_map.initialise (word->best_choice->unichar_lengths().length ());
  reject_blanks(word);
  /*
  0: Rays original heuristic - the baseline
  */
  if (tessedit_reject_mode == 0) {
    if (!word->done)
      reject_poor_matches(word, blob_choices);
  }
  /*
  5: Reject I/1/l from words where there is no strong contextual confirmation;
    the whole of any unacceptable words (incl PERM rej of dubious 1/I/ls);
    and the whole of any words which are very small
  */
  else if (tessedit_reject_mode == 5) {
    if (bln_x_height / word->denorm.scale () <= min_sane_x_ht_pixels)
      word->reject_map.rej_word_small_xht ();
    else {
      one_ell_conflict(word, TRUE);
      /*
        Originally the code here just used the done flag. Now I have duplicated
        and unpacked the conditions for setting the done flag so that each
        mechanism can be turned on or off independently. This works WITHOUT
        affecting the done flag setting.
      */
      if (rej_use_tess_accepted && !word->tess_accepted)
        word->reject_map.rej_word_not_tess_accepted ();

      if (rej_use_tess_blanks &&
        (strchr (word->best_choice->unichar_string().string (), ' ') != NULL))
        word->reject_map.rej_word_contains_blanks ();

      if (rej_use_good_perm) {
        if (((word->best_choice->permuter () == SYSTEM_DAWG_PERM) ||
          (word->best_choice->permuter () == FREQ_DAWG_PERM) ||
          (word->best_choice->permuter () == USER_DAWG_PERM)) &&
          (!rej_use_sensible_wd ||
          (acceptable_word_string
          (word->best_choice->unichar_string().string (),
           word->best_choice->unichar_lengths().string ()) !=
        AC_UNACCEPTABLE))) {
          //PASSED TEST
        }
        else if (word->best_choice->permuter () == NUMBER_PERM) {
          if (rej_alphas_in_number_perm) {
            for (i = 0, offset = 0;
                 word->best_choice->unichar_string()[offset] != '\0';
                 offset += word->best_choice->unichar_lengths()[i++]) {
              if (word->reject_map[i].accepted () &&
                  unicharset.get_isalpha(
                      word->best_choice->unichar_string().string() + offset,
                      word->best_choice->unichar_lengths()[i]))
                word->reject_map[i].setrej_bad_permuter ();
              //rej alpha
            }
          }
        }
        else {
          word->reject_map.rej_word_bad_permuter ();
        }
      }

      /* Ambig word rejection was here once !!*/

    }
  }
  else {
    tprintf ("BAD tessedit_reject_mode\n");
    err_exit();
  }

  if (tessedit_image_border > -1)
    reject_edge_blobs(word);

  check_debug_pt (word, 10);
  if (tessedit_rejection_debug) {
    tprintf ("Permuter Type = %d\n", word->best_choice->permuter ());
    tprintf ("Certainty: %f     Rating: %f\n",
      word->best_choice->certainty (), word->best_choice->rating ());
    tprintf("Dict word: %d\n", dict_word(*(word->best_choice)));
  }

  /* Un-reject any rejected characters if NN permits */

  if (tessedit_use_nn && (pass == 2) &&
    word->reject_map.recoverable_rejects ())
    nn_recover_rejects(word, row);
  flip_hyphens(word);
  check_debug_pt (word, 20);
}
}  // namespace tesseract


void reject_blanks(WERD_RES *word) {
  inT16 i;
  inT16 offset;

  for (i = 0, offset = 0; word->best_choice->unichar_string()[offset] != '\0';
       offset += word->best_choice->unichar_lengths()[i], i += 1) {
    if (word->best_choice->unichar_string()[offset] == ' ')
                                 //rej unrecognised blobs
      word->reject_map[i].setrej_tess_failure ();
  }
}


void reject_I_1_L(WERD_RES *word) {
  inT16 i;
  inT16 offset;

  for (i = 0, offset = 0; word->best_choice->unichar_string()[offset] != '\0';
       offset += word->best_choice->unichar_lengths()[i], i += 1) {
    if (STRING (conflict_set_I_l_1).
    contains (word->best_choice->unichar_string()[offset])) {
                                 //rej 1Il conflict
      word->reject_map[i].setrej_1Il_conflict ();
    }
  }
}


void reject_poor_matches(  //detailed results
                         WERD_RES *word,
                         BLOB_CHOICE_LIST_CLIST *blob_choices) {
  float threshold;
  inT16 i = 0;
  inT16 offset = 0;
                                 //super iterator
  BLOB_CHOICE_LIST_C_IT list_it = blob_choices;
  BLOB_CHOICE_IT choice_it;      //real iterator

  #ifndef SECURE_NAMES
  if (strlen(word->best_choice->unichar_lengths().string()) !=
      list_it.length()) {
    tprintf
      ("ASSERT FAIL string:\"%s\"; strlen=%d; choices len=%d; blob len=%d\n",
      word->best_choice->unichar_string().string(),
      strlen (word->best_choice->unichar_lengths().string()), list_it.length(),
      word->outword->blob_list()->length());
  }
  #endif
  ASSERT_HOST (strlen (word->best_choice->unichar_lengths().string ()) ==
    list_it.length ());
  ASSERT_HOST (word->outword->blob_list ()->length () == list_it.length ());
  threshold = compute_reject_threshold (blob_choices);

  for (list_it.mark_cycle_pt ();
  !list_it.cycled_list (); list_it.forward (), i++,
           offset += word->best_choice->unichar_lengths()[i]) {
    /* NB - only compares the threshold against the TOP choice char in the
      choices list for a blob !! - the selected one may be below the threshold
    */
    choice_it.set_to_list (list_it.data ());
    if ((word->best_choice->unichar_string()[offset] == ' ') ||
      (choice_it.length () == 0))
                                 //rej unrecognised blobs
      word->reject_map[i].setrej_tess_failure ();
    else if (choice_it.data ()->certainty () < threshold)
                                 //rej poor score blob
      word->reject_map[i].setrej_poor_match ();
  }
}


/**********************************************************************
 * compute_reject_threshold
 *
 * Set a rejection threshold for this word.
 * Initially this is a trivial function which looks for the largest
 * gap in the certainty value.
 **********************************************************************/

float compute_reject_threshold(  //compute threshold //detailed results
                               BLOB_CHOICE_LIST_CLIST *blob_choices) {
  inT16 index;                   //to ratings
  inT16 blob_count;              //no of blobs in word
  inT16 ok_blob_count = 0;       //non TESS rej blobs in word
  float *ratings;                //array of confidences
  float threshold;               //rejection threshold
  float bestgap;                 //biggest gap
  float gapstart;                //bottom of gap
                                 //super iterator
  BLOB_CHOICE_LIST_C_IT list_it = blob_choices;
  BLOB_CHOICE_IT choice_it;      //real iterator

  blob_count = blob_choices->length ();
  ratings = (float *) alloc_mem (blob_count * sizeof (float));
  for (list_it.mark_cycle_pt (), index = 0;
  !list_it.cycled_list (); list_it.forward (), index++) {
    choice_it.set_to_list (list_it.data ());
    if (choice_it.length () > 0) {
      ratings[ok_blob_count] = choice_it.data ()->certainty ();
      //get in an array
      //                 tprintf("Rating[%d]=%c %g %g\n",
      //                         index,choice_it.data()->char_class(),
      //                         choice_it.data()->rating(),choice_it.data()->certainty());
      ok_blob_count++;
    }
  }
  ASSERT_HOST (index == blob_count);
  qsort (ratings, ok_blob_count, sizeof (float), sort_floats);
  //sort them
  bestgap = 0;
  gapstart = ratings[0] - 1;     //all reject if none better
  if (ok_blob_count >= 3) {
    for (index = 0; index < ok_blob_count - 1; index++) {
      if (ratings[index + 1] - ratings[index] > bestgap) {
        bestgap = ratings[index + 1] - ratings[index];
        //find biggest
        gapstart = ratings[index];
      }
    }
  }
  threshold = gapstart + bestgap / 2;
  //      tprintf("First=%g, last=%g, gap=%g, threshold=%g\n",
  //              ratings[0],ratings[index],bestgap,threshold);

  free_mem(ratings);
  return threshold;
}


/*************************************************************************
 * reject_edge_blobs()
 *
 * If the word is perilously close to the edge of the image, reject those blobs
 * in the word which are too close to the edge as they could be clipped.
 *************************************************************************/

void reject_edge_blobs(WERD_RES *word) {
  TBOX word_box = word->word->bounding_box ();
  TBOX blob_box;
  PBLOB_IT blob_it = word->outword->blob_list ();
  //blobs
  int blobindex = 0;
  float centre;

  if ((word_box.left () < tessedit_image_border) ||
    (word_box.bottom () < tessedit_image_border) ||
    (word_box.right () + tessedit_image_border >
    page_image.get_xsize () - 1) ||
  (word_box.top () + tessedit_image_border > page_image.get_ysize () - 1)) {
    ASSERT_HOST (word->reject_map.length () == blob_it.length ());
    for (blobindex = 0, blob_it.mark_cycle_pt ();
    !blob_it.cycled_list (); blobindex++, blob_it.forward ()) {
      blob_box = blob_it.data ()->bounding_box ();
      centre = (blob_box.left () + blob_box.right ()) / 2.0;
      if ((word->denorm.x (blob_box.left ()) < tessedit_image_border) ||
        (word->denorm.y (blob_box.bottom (), centre) <
        tessedit_image_border) ||
        (word->denorm.x (blob_box.right ()) + tessedit_image_border >
        page_image.get_xsize () - 1) ||
        (word->denorm.y (blob_box.top (), centre)
      + tessedit_image_border > page_image.get_ysize () - 1)) {
        word->reject_map[blobindex].setrej_edge_char ();
        //close to edge
      }
    }
  }
}


/**********************************************************************
 * one_ell_conflict()
 *
 * Identify words where there is a potential I/l/1 error.
 * - A bundle of contextual heuristics!
 **********************************************************************/
namespace tesseract {
BOOL8 Tesseract::one_ell_conflict(WERD_RES *word_res, BOOL8 update_map) {
  const char *word;
  const char *lengths;
  inT16 word_len;                //its length
  inT16 first_alphanum_index_;
  inT16 first_alphanum_offset_;
  inT16 i;
  inT16 offset;
  BOOL8 non_conflict_set_char;   //non conf set a/n?
  BOOL8 conflict = FALSE;
  BOOL8 allow_1s;
  ACCEPTABLE_WERD_TYPE word_type;
  BOOL8 dict_perm_type;
  BOOL8 dict_word_ok;
  int dict_word_type;

  word = word_res->best_choice->unichar_string().string ();
  lengths = word_res->best_choice->unichar_lengths().string();
  word_len = strlen (lengths);
  /*
    If there are no occurrences of the conflict set characters then the word
    is OK.
  */
  if (strpbrk (word, conflict_set_I_l_1.string ()) == NULL)
    return FALSE;

  /*
    There is a conflict if there are NO other (confirmed) alphanumerics apart
    from those in the conflict set.
  */

  for (i = 0, offset = 0, non_conflict_set_char = FALSE;
       (i < word_len) && !non_conflict_set_char; offset += lengths[i++])
    non_conflict_set_char =
        (unicharset.get_isalpha(word + offset, lengths[i]) ||
         unicharset.get_isdigit(word + offset, lengths[i])) &&
        !STRING (conflict_set_I_l_1).contains (word[offset]);
  if (!non_conflict_set_char) {
    if (update_map)
      reject_I_1_L(word_res);
    return TRUE;
  }

  /*
    If the word is accepted by a dawg permuter, and the first alpha character
    is "I" or "l", check to see if the alternative is also a dawg word. If it
    is, then there is a potential error otherwise the word is ok.
  */

  dict_perm_type = (word_res->best_choice->permuter () == SYSTEM_DAWG_PERM) ||
    (word_res->best_choice->permuter () == USER_DAWG_PERM) ||
    (rej_trust_doc_dawg &&
    (word_res->best_choice->permuter () == DOC_DAWG_PERM)) ||
    (word_res->best_choice->permuter () == FREQ_DAWG_PERM);
  dict_word_type = dict_word(*(word_res->best_choice));
  dict_word_ok = (dict_word_type > 0) &&
    (rej_trust_doc_dawg || (dict_word_type != DOC_DAWG_PERM));

  if ((rej_1Il_use_dict_word && dict_word_ok) ||
    (rej_1Il_trust_permuter_type && dict_perm_type) ||
  (dict_perm_type && dict_word_ok)) {
    first_alphanum_index_ = first_alphanum_index (word, lengths);
    first_alphanum_offset_ = first_alphanum_offset (word, lengths);
    if (lengths[first_alphanum_index_] == 1 &&
        word[first_alphanum_offset_] == 'I') {
      word_res->best_choice->unichar_string()[first_alphanum_offset_] = 'l';
      if (safe_dict_word(*(word_res->best_choice)) > 0) {
        word_res->best_choice->unichar_string()[first_alphanum_offset_] = 'I';
        if (update_map)
          word_res->reject_map[first_alphanum_index_].
            setrej_1Il_conflict();
        return TRUE;
      }
      else {
        word_res->best_choice->unichar_string()[first_alphanum_offset_] = 'I';
        return FALSE;
      }
    }

    if (lengths[first_alphanum_index_] == 1 &&
        word[first_alphanum_offset_] == 'l') {
      word_res->best_choice->unichar_string()[first_alphanum_offset_] = 'I';
      if (safe_dict_word(*(word_res->best_choice)) > 0) {
        word_res->best_choice->unichar_string()[first_alphanum_offset_] = 'l';
        if (update_map)
          word_res->reject_map[first_alphanum_index_].
            setrej_1Il_conflict();
        return TRUE;
      }
      else {
        word_res->best_choice->unichar_string()[first_alphanum_offset_] = 'l';
        return FALSE;
      }
    }
    return FALSE;
  }

  /*
    NEW 1Il code. The old code relied on permuter types too much. In fact,
    tess will use TOP_CHOICE permute for good things like "palette".
    In this code the string is examined independently to see if it looks like
    a well formed word.
  */

  /*
    REGARDLESS OF PERMUTER, see if flipping a leading I/l generates a
    dictionary word.
  */
  first_alphanum_index_ = first_alphanum_index (word, lengths);
  first_alphanum_offset_ = first_alphanum_offset (word, lengths);
  if (lengths[first_alphanum_index_] == 1 &&
      word[first_alphanum_offset_] == 'l') {
    word_res->best_choice->unichar_string()[first_alphanum_offset_] = 'I';
    if (safe_dict_word(*(word_res->best_choice)) > 0)
      return FALSE;
    else
      word_res->best_choice->unichar_string()[first_alphanum_offset_] = 'l';
  }
  else if (lengths[first_alphanum_index_] == 1 &&
           word[first_alphanum_offset_] == 'I') {
    word_res->best_choice->unichar_string()[first_alphanum_offset_] = 'l';
    if (safe_dict_word(*(word_res->best_choice)) > 0)
      return FALSE;
    else
      word_res->best_choice->unichar_string()[first_alphanum_offset_] = 'I';
  }
  /*
    For strings containing digits:
      If there are no alphas OR the numeric permuter liked the word,
        reject any non 1 conflict chs
      Else reject all conflict chs
  */
  if (word_contains_non_1_digit (word, lengths)) {
    allow_1s = (alpha_count (word, lengths) == 0) ||
      (word_res->best_choice->permuter () == NUMBER_PERM);

    inT16 offset;
    conflict = FALSE;
    for (i = 0, offset = 0; word[offset] != '\0';
         offset += word_res->best_choice->unichar_lengths()[i++]) {
      if ((!allow_1s || (word[offset] != '1')) &&
      STRING (conflict_set_I_l_1).contains (word[offset])) {
        if (update_map)
          word_res->reject_map[i].setrej_1Il_conflict ();
        conflict = TRUE;
      }
    }
    return conflict;
  }
  /*
    For anything else. See if it conforms to an acceptable word type. If so,
    treat accordingly.
  */
  word_type = acceptable_word_string (word, lengths);
  if ((word_type == AC_LOWER_CASE) || (word_type == AC_INITIAL_CAP)) {
    first_alphanum_index_ = first_alphanum_index (word, lengths);
    first_alphanum_offset_ = first_alphanum_offset (word, lengths);
    if (STRING (conflict_set_I_l_1).contains (word[first_alphanum_offset_])) {
      if (update_map)
        word_res->reject_map[first_alphanum_index_].
            setrej_1Il_conflict ();
      return TRUE;
    }
    else
      return FALSE;
  }
  else if (word_type == AC_UPPER_CASE) {
    return FALSE;
  }
  else {
    if (update_map)
      reject_I_1_L(word_res);
    return TRUE;
  }
}


inT16 Tesseract::first_alphanum_index(const char *word,
                                      const char *word_lengths) {
  inT16 i;
  inT16 offset;

  for (i = 0, offset = 0; word[offset] != '\0'; offset += word_lengths[i++]) {
    if (unicharset.get_isalpha(word + offset, word_lengths[i]) ||
        unicharset.get_isdigit(word + offset, word_lengths[i]))
      return i;
  }
  return -1;
}

inT16 Tesseract::first_alphanum_offset(const char *word,
                                       const char *word_lengths) {
  inT16 i;
  inT16 offset;

  for (i = 0, offset = 0; word[offset] != '\0'; offset += word_lengths[i++]) {
    if (unicharset.get_isalpha(word + offset, word_lengths[i]) ||
        unicharset.get_isdigit(word + offset, word_lengths[i]))
      return offset;
  }
  return -1;
}

inT16 Tesseract::alpha_count(const char *word,
                             const char *word_lengths) {
  inT16 i;
  inT16 offset;
  inT16 count = 0;

  for (i = 0, offset = 0; word[offset] != '\0'; offset += word_lengths[i++]) {
    if (unicharset.get_isalpha (word + offset, word_lengths[i]))
      count++;
  }
  return count;
}


BOOL8 Tesseract::word_contains_non_1_digit(const char *word,
                                           const char *word_lengths) {
  inT16 i;
  inT16 offset;

  for (i = 0, offset = 0; word[offset] != '\0'; offset += word_lengths[i++]) {
    if (unicharset.get_isdigit (word + offset, word_lengths[i]) &&
        (word_lengths[i] != 1 || word[offset] != '1'))
      return TRUE;
  }
  return FALSE;
}


BOOL8 Tesseract::test_ambig_word(  //test for ambiguity
                                 WERD_RES *word) {
    BOOL8 ambig = FALSE;

    if ((word->best_choice->permuter () == SYSTEM_DAWG_PERM) ||
      (word->best_choice->permuter () == FREQ_DAWG_PERM) ||
    (word->best_choice->permuter () == USER_DAWG_PERM)) {
      ambig = !getDict().NoDangerousAmbig(
          word->best_choice, NULL, false, NULL, NULL);
  }
  return ambig;
}

/*************************************************************************
 * char_ambiguities()
 *
 * Return a pointer to a string containing the full conflict set of characters
 * which includes the specified character, if there is one. If the specified
 * character is not a member of a conflict set, return NULL.
 * (NOTE that a character is assumed to be a member of only ONE conflict set.)
 *************************************************************************/
const char *Tesseract::char_ambiguities(char c) {
  static STRING_CLIST conflict_sets;
  static BOOL8 read_conflict_sets = FALSE;
  STRING_C_IT cs_it(&conflict_sets);
  const char *cs;
  STRING cs_file_name;
  FILE *cs_file;
  char buff[1024];

  if (!read_conflict_sets) {
    cs_file_name = datadir + "confsets";
    if (!(cs_file = fopen (cs_file_name.string (), "r"))) {
      CANTOPENFILE.error ("char_ambiguities", EXIT, "%s %d",
        cs_file_name.string (), errno);
    }
    while (fscanf (cs_file, "%s", buff) == 1) {
      cs_it.add_after_then_move (new STRING (buff));
    }
    fclose (cs_file);
    read_conflict_sets = TRUE;
    cs_it.move_to_first ();
    if (tessedit_rejection_debug) {
      for (cs_it.mark_cycle_pt ();
      !cs_it.cycled_list (); cs_it.forward ()) {
        tprintf ("\"%s\"\n", cs_it.data ()->string ());
      }
    }
  }

  cs_it.move_to_first ();
  for (cs_it.mark_cycle_pt (); !cs_it.cycled_list (); cs_it.forward ()) {
    cs = cs_it.data ()->string ();
    if (strchr (cs, c) != NULL)
      return cs;
  }
  return NULL;
}

/*************************************************************************
 * nn_recover_rejects()
 * Generate the nn_reject_map - a copy of the current reject map, but dont
 * reject previously rejected chars if the NN matcher agrees with the best
 * choice.
 *************************************************************************/

void Tesseract::nn_recover_rejects(WERD_RES *word, ROW *row) {
                                 //copy for debug
  REJMAP old_map = word->reject_map;
  /*
    NOTE THAT THIS IS RELATIVELY INEFFICIENT AS THE WHOLE OF THE WERD IS
    MATCHED BY THE NN MATCHER. IF COULD EASILY BE RESTRICTED TO JUST THE
    REJECT CHARACTERS  (Though initial use is when words are total rejects
    anyway).
  */

  set_global_subsubloc_code(SUBSUBLOC_NN);
  nn_match_word(word, row);

  if (no_unrej_1Il)
    dont_allow_1Il(word);
  if (no_unrej_dubious_chars)
    dont_allow_dubious_chars(word);

  if (rej_mostly_reject_mode == 1)
    reject_mostly_rejects(word);
  /*
    IF there are no unrejected alphanumerics AND
      The word is not an acceptable single non alphanum char word  AND
      The word is not an acceptable repeated non alphanum char word
    THEN Reject whole word
  */
  if (no_unrej_no_alphanum_wds &&
    (count_alphanums (word) < 1) &&
    !((word->best_choice->unichar_lengths().length () == 1) &&
      STRING(ok_single_ch_non_alphanum_wds).contains(
          word->best_choice->unichar_string()[0]))
    && !repeated_nonalphanum_wd (word, row))

    word->reject_map.rej_word_no_alphanums ();

  #ifndef SECURE_NAMES

  if (nn_debug) {
    tprintf ("\nTess: \"%s\" MAP ",
             word->best_choice->unichar_string().string());
    old_map.print (stdout);
    tprintf ("->");
    word->reject_map.print (stdout);
    tprintf ("\n");
  }
  #endif
  set_global_subsubloc_code(SUBSUBLOC_OTHER);
}

void Tesseract::nn_match_word(  //Match a word
                              WERD_RES *word,
                              ROW *row) {
  PIXROW_LIST *pixrow_list;
  PIXROW_IT pixrow_it;
  IMAGELINE *imlines;            //lines of the image
  TBOX pix_box;                   //box of imlines extent
#ifndef GRAPHICS_DISABLED
  ScrollView* win = NULL;
#endif
  IMAGE clip_image;
  IMAGE scaled_image;
  float baseline_pos;
  inT16 net_image_size;
  inT16 clip_image_size;
  WERD copy_outword;             // copy to denorm
  inT16 i;

  const char *word_string;
  const char *word_string_lengths;
  BOOL8 word_in_dict;            //Tess wd in dict
  BOOL8 checked_dict_word;       //Tess wd definitely in dict
  BOOL8 sensible_word;           //OK char string
  BOOL8 centre;                  //Not at word end       chs
  BOOL8 good_quality_word;
  inT16 char_quality;
  inT16 accepted_char_quality;

  inT16 conf_level;              //0:REJECT
  //1:DODGY ACCEPT
  //2:DICT ACCEPT
  //3:CLEAR ACCEPT
  inT16 first_alphanum_index_;
  inT16 first_alphanum_offset_;

  word_string = word->best_choice->unichar_string().string();
  word_string_lengths = word->best_choice->unichar_lengths().string();
  first_alphanum_index_ = first_alphanum_index (word_string,
                                                word_string_lengths);
  first_alphanum_offset_ = first_alphanum_offset (word_string,
                                                  word_string_lengths);
  word_in_dict = ((word->best_choice->permuter () == SYSTEM_DAWG_PERM) ||
    (word->best_choice->permuter () == FREQ_DAWG_PERM) ||
    (word->best_choice->permuter () == USER_DAWG_PERM));
  checked_dict_word = word_in_dict &&
    (safe_dict_word(*(word->best_choice)) > 0);
  sensible_word = acceptable_word_string (word_string, word_string_lengths) !=
      AC_UNACCEPTABLE;

  word_char_quality(word, row, &char_quality, &accepted_char_quality);
  good_quality_word =
      word->best_choice->unichar_lengths().length () == char_quality;

  #ifndef SECURE_NAMES
  if (nn_reject_debug) {
    tprintf ("Dict: %c   Checked Dict: %c   Sensible: %c   Quality: %c\n",
      word_in_dict ? 'T' : 'F',
      checked_dict_word ? 'T' : 'F',
      sensible_word ? 'T' : 'F', good_quality_word ? 'T' : 'F');
  }
  #endif

  if (word->best_choice->unichar_lengths().length () !=
  word->outword->blob_list ()->length ()) {
    #ifndef SECURE_NAMES
    tprintf ("nn_match_word ASSERT FAIL String:\"%s\";  #Blobs=%d\n",
      word->best_choice->unichar_string().string (),
      word->outword->blob_list ()->length ());
    #endif
    err_exit();
  }

  copy_outword = *(word->outword);
  copy_outword.baseline_denormalise (&word->denorm);
  /*
    For each character, generate and match a new image, containing JUST the
    character we have clipped, centered in the image, on a white background.
    Note that we MUST have a square image so that we can scale it uniformly in
    x and y.  We base the size on x_height as this can be found fairly reliably.
  */
  net_image_size = (net_image_width > net_image_height) ?
    net_image_width : net_image_height;
  clip_image_size = (inT16) floor (0.5 +
    net_image_size * word->x_height /
    net_image_x_height);
  if ((clip_image_size <= 1) || (net_image_size <= 1)) {
    return;
  }

  /*
    Get the image of the word and the pix positions of each char
  */
  char_clip_word(&copy_outword, page_image, pixrow_list, imlines, pix_box);
#ifndef GRAPHICS_DISABLED
  if (show_char_clipping) {
    win = display_clip_image (&copy_outword, page_image,
      pixrow_list, pix_box);
  }
#endif
  pixrow_it.set_to_list (pixrow_list);
  pixrow_it.move_to_first ();
  for (pixrow_it.mark_cycle_pt (), i = 0;
  !pixrow_it.cycled_list (); pixrow_it.forward (), i++) {
    if (pixrow_it.data ()->
      bad_box (page_image.get_xsize (), page_image.get_ysize ()))
      continue;
    clip_image.create (clip_image_size, clip_image_size, 1);
    //make bin imge
    if (!copy_outword.flag (W_INVERSE))
      invert_image(&clip_image);  //white background for black on white
    pixrow_it.data ()->char_clip_image (imlines, pix_box, row,
      clip_image, baseline_pos);
    if (copy_outword.flag (W_INVERSE))
      invert_image(&clip_image);  //invert white on black for scaling &NN
    scaled_image.create (net_image_size, net_image_size, 1);
    scale_image(clip_image, scaled_image);
    baseline_pos *= net_image_size / clip_image_size;
    //scale with im
    centre = !pixrow_it.at_first () && !pixrow_it.at_last ();

    conf_level = nn_match_char (scaled_image, baseline_pos,
      word_in_dict, checked_dict_word,
      sensible_word, centre,
      good_quality_word, word_string[i]);
    if (word->reject_map[i].recoverable ()) {
      if ((i == first_alphanum_index_) &&
          word_string_lengths[first_alphanum_index_] == 1 &&
      ((word_string[first_alphanum_offset_] == 'I') ||
       (word_string[first_alphanum_offset_] == 'i'))) {
        if (conf_level >= nn_conf_initial_i_level)
          word->reject_map[i].setrej_nn_accept ();
        //un-reject char
      }
      else if (conf_level > 0)
                                 //un-reject char
        word->reject_map[i].setrej_nn_accept ();
    }
#ifndef GRAPHICS_DISABLED
    if (show_char_clipping)
      display_images(clip_image, scaled_image);
#endif
   clip_image.destroy();
   scaled_image.destroy();
  }

  delete[]imlines;               // Free array of imlines
  delete pixrow_list;

#ifndef GRAPHICS_DISABLED
  if (show_char_clipping) {
//    destroy_window(win);
//   win->Destroy();
    delete win;
  }
#endif
}
}  // namespace tesseract


/*************************************************************************
 * nn_match_char()
 * Call Neural Net matcher to match a single character, given a scaled,
 * square image
 *************************************************************************/

inT16 nn_match_char(                          //of character
                    IMAGE &scaled_image,
                    float baseline_pos,       //rel to scaled_image
                    BOOL8 dict_word,          //part of dict wd?
                    BOOL8 checked_dict_word,  //part of dict wd?
                    BOOL8 sensible_word,      //part acceptable str?
                    BOOL8 centre,             //not at word ends?
                    BOOL8 good_quality_word,  //initial segmentation
                    char tess_ch              //confirm this?
                   ) {
  inT16 conf_level;              //0..2
  inT32 row;
  inT32 col;
  inT32 y_size = scaled_image.get_ysize ();
  inT32 start_y = y_size - (y_size - net_image_height) / 2 - 1;
  inT32 end_y = start_y - net_image_height + 1;
  IMAGELINE imline;
  float *input_vector;
  float *input_vec_ptr;
  char top;
  float top_score;
  char next;
  float next_score;
  inT16 input_nodes = (net_image_height * net_image_width) + net_bl_nodes;
  inT16 j;

  input_vector = (float *) alloc_mem (input_nodes * sizeof (float));
  input_vec_ptr = input_vector;

  invert_image(&scaled_image);  //cos nns work better
  for (row = start_y; row >= end_y; row--) {
    scaled_image.fast_get_line (0, row, net_image_width, &imline);
    for (col = 0; col < net_image_width; col++)
      *input_vec_ptr++ = imline.pixels[col];
  }
  /*
    The bit map presented to the net may be shorter than the image, so shift
    the coord to be relative to the bitmap portion.
  */
  baseline_pos -= (y_size - net_image_height) / 2.0;
  /*
    Baseline pos is 0 if below bitmap, 1 if above and in proportion otherwise.
    This is represented to the net as a set of bl_nodes, an initial proportion
    of which are set to 1.0, indicating the level of the baseline. The
    remainder are 0.0
  */

  if (baseline_pos < 0)
    baseline_pos = 0;
  else if (baseline_pos >= net_image_height)
    baseline_pos = net_image_height + 1;
  else
    baseline_pos = baseline_pos + 1;
  baseline_pos = baseline_pos / (net_image_height + 1);

  if (net_bl_nodes > 0) {
    baseline_pos *= 1.7;         //Use a wider range
    if (net_bl_nodes > 1) {
      /* Multi-node baseline representation */
      for (j = 0; j < net_bl_nodes; j++) {
        if (baseline_pos > ((float) j / net_bl_nodes))
          *input_vec_ptr++ = 1.0;
        else
          *input_vec_ptr++ = 0.0;
      }
    }
    else {
      /* Single node baseline */
      *input_vec_ptr++ = baseline_pos;
    }
  }

  callnet(input_vector, &top, &top_score, &next, &next_score);
  conf_level = evaluate_net_match (top, top_score, next, next_score,
    tess_ch, dict_word, checked_dict_word,
    sensible_word, centre, good_quality_word);
  #ifndef SECURE_NAMES
  if (nn_reject_debug) {
    tprintf ("top:\"%c\" %4.2f   next:\"%c\" %4.2f  TESS:\"%c\" Conf: %d\n",
      top, top_score, next, next_score, tess_ch, conf_level);
  }
  #endif
  free_mem(input_vector);
  return conf_level;
}


inT16 evaluate_net_match(char top,
                         float top_score,
                         char next,
                         float next_score,
                         char tess_ch,
                         BOOL8 dict_word,
                         BOOL8 checked_dict_word,
                         BOOL8 sensible_word,
                         BOOL8 centre,
                         BOOL8 good_quality_word) {
  inT16 accept_level;            //0 Very clearly matched
  //1 Clearly top
  //2 Top but poor match
  //3 Next & poor top match
  //4 Next but good top match
  //5 No chance
  BOOL8 good_top_choice;
  BOOL8 excellent_top_choice;
  BOOL8 confusion_match = FALSE;
  BOOL8 dodgy_char = !isalnum (tess_ch);

  good_top_choice = (top_score > nn_reject_threshold) &&
    (nn_reject_head_and_shoulders * top_score > next_score);

  excellent_top_choice = good_top_choice &&
    (top_score > nn_dodgy_char_threshold);

  if (top == tess_ch) {
    if (excellent_top_choice)
      accept_level = 0;
    else if (good_top_choice)
      accept_level = 1;          //Top correct and well matched
    else
      accept_level = 2;          //Top correct but poor match
  }
  else if ((nn_conf_1Il &&
    STRING (conflict_set_I_l_1).contains (tess_ch) &&
    STRING (conflict_set_I_l_1).contains (top)) ||
    (nn_conf_hyphen &&
    STRING (conflict_set_hyphen).contains (tess_ch) &&
    STRING (conflict_set_hyphen).contains (top)) ||
    (nn_conf_Ss &&
    STRING (conflict_set_S_s).contains (tess_ch) &&
  STRING (conflict_set_S_s).contains (top))) {
    confusion_match = TRUE;
    if (good_top_choice)
      accept_level = 1;          //Good top confusion
    else
      accept_level = 2;          //Poor top confusion
  }
  else if ((nn_conf_1Il &&
    STRING (conflict_set_I_l_1).contains (tess_ch) &&
    STRING (conflict_set_I_l_1).contains (next)) ||
    (nn_conf_hyphen &&
    STRING (conflict_set_hyphen).contains (tess_ch) &&
    STRING (conflict_set_hyphen).contains (next)) ||
    (nn_conf_Ss &&
    STRING (conflict_set_S_s).contains (tess_ch) &&
  STRING (conflict_set_S_s).contains (next))) {
    confusion_match = TRUE;
    if (!good_top_choice)
      accept_level = 3;          //Next confusion and top match dodgy
    else
      accept_level = 4;          //Next confusion and good top match
  }
  else if (next == tess_ch) {
    if (!good_top_choice)
      accept_level = 3;          //Next match and top match dodgy
    else
      accept_level = 4;          //Next match and good top match
  }
  else
    accept_level = 5;

  /* Could allow some match flexibility here sS$ etc */

  /* Now set confirmation level according to how much we can believe the tess
    char. */

  if ((accept_level == 0) && !confusion_match)
    return 3;

  if ((accept_level <= 1) &&
    (!nn_conf_strict_on_dodgy_chs || !dodgy_char) && !confusion_match)
    return 3;

  if ((accept_level == 2) &&
    !confusion_match && !dodgy_char &&
    good_quality_word &&
    dict_word &&
    (checked_dict_word || !nn_double_check_dict) && sensible_word)
    return 2;

  if (confusion_match &&
    (accept_level <= nn_conf_accept_level) &&
    (good_quality_word ||
    (!nn_conf_test_good_qual &&
    !STRING (conflict_set_I_l_1).contains (tess_ch))) &&
    (dict_word || !nn_conf_test_dict) &&
    (checked_dict_word || !nn_conf_double_check_dict) &&
    (sensible_word || !nn_conf_test_sensible))
    return 1;

  if (!confusion_match &&
    nn_lax &&
    (accept_level == 3) &&
    (good_quality_word || !nn_conf_test_good_qual) &&
    (dict_word || !nn_conf_test_dict) &&
    (sensible_word || !nn_conf_test_sensible))
    return 1;
  else
    return 0;
}


/*************************************************************************
 * dont_allow_dubious_chars()
 * Let Rejects "eat" into adjacent "dubious" chars. I.e those prone to be wrong
 * if adjacent to a reject.
 *************************************************************************/
void dont_allow_dubious_chars(WERD_RES *word) {
  int i = 0;
  int offset = 0;
  int rej_pos;
  int word_len = word->reject_map.length ();

  while (i < word_len) {
    /* Find next reject */

    while ((i < word_len) && (word->reject_map[i].accepted ()))
    {
      offset += word->best_choice->unichar_lengths()[i];
      i++;
    }

    if (i < word_len) {
      rej_pos = i;

      /* Reject dubious chars to the left */
      i--;
      offset -= word->best_choice->unichar_lengths()[i];
      while ((i >= 0) &&
        STRING(dubious_chars_left_of_reject).contains(
            word->best_choice->unichar_string()[offset])) {
        word->reject_map[i--].setrej_dubious ();
        offset -= word->best_choice->unichar_lengths()[i];
      }

      /* Skip adjacent rejects */

      for (i = rej_pos;
        (i < word_len) && (word->reject_map[i].rejected ());
           offset += word->best_choice->unichar_lengths()[i++]);

      /* Reject dubious chars to the right */

      while ((i < word_len) &&
        STRING(dubious_chars_right_of_reject).contains(
            word->best_choice->unichar_string()[offset])) {
        offset += word->best_choice->unichar_lengths()[i];
        word->reject_map[i++].setrej_dubious ();
      }
    }
  }
}


/*************************************************************************
 * dont_allow_1Il()
 * Dont unreject LONE accepted 1Il conflict set chars
 *************************************************************************/
namespace tesseract {
void Tesseract::dont_allow_1Il(WERD_RES *word) {
  int i = 0;
  int offset;
  int word_len = word->reject_map.length ();
  const char *s = word->best_choice->unichar_string().string ();
  const char *lengths = word->best_choice->unichar_lengths().string ();
  BOOL8 accepted_1Il = FALSE;

  for (i = 0, offset = 0; i < word_len;
       offset += word->best_choice->unichar_lengths()[i++]) {
    if (word->reject_map[i].accepted ()) {
      if (STRING (conflict_set_I_l_1).contains (s[offset]))
        accepted_1Il = TRUE;
      else {
        if (unicharset.get_isalpha (s + offset, lengths[i]) ||
            unicharset.get_isdigit (s + offset, lengths[i]))
          return;                // >=1 non 1Il ch accepted
      }
    }
  }
  if (!accepted_1Il)
    return;                      //Nothing to worry about

  for (i = 0, offset = 0; i < word_len;
       offset += word->best_choice->unichar_lengths()[i++]) {
    if (STRING (conflict_set_I_l_1).contains (s[offset]) &&
      word->reject_map[i].accepted ())
      word->reject_map[i].setrej_postNN_1Il ();
  }
}


inT16 Tesseract::count_alphanums(  //how many alphanums
                                 WERD_RES *word_res) {
  int count = 0;
  const WERD_CHOICE *best_choice = word_res->best_choice;
  for (int i = 0; i < word_res->reject_map.length(); ++i) {
    if ((word_res->reject_map[i].accepted()) &&
        (unicharset.get_isalpha(best_choice->unichar_id(i)) ||
         unicharset.get_isdigit(best_choice->unichar_id(i)))) {
      count++;
    }
  }
  return count;
}
}  // namespace tesseract


void reject_mostly_rejects(  //rej all if most rejectd
                           WERD_RES *word) {
  /* Reject the whole of the word if the fraction of rejects exceeds a limit */

  if ((float) word->reject_map.reject_count () / word->reject_map.length () >=
    rej_whole_of_mostly_reject_word_fract)
    word->reject_map.rej_word_mostly_rej ();
}


namespace tesseract {
BOOL8 Tesseract::repeated_nonalphanum_wd(WERD_RES *word, ROW *row) {
  inT16 char_quality;
  inT16 accepted_char_quality;

  if (word->best_choice->unichar_lengths().length () <= 1)
    return FALSE;

  if (!STRING (ok_repeated_ch_non_alphanum_wds).
    contains (word->best_choice->unichar_string()[0]))
    return FALSE;

  if (!repeated_ch_string (word->best_choice->unichar_string().string (),
                           word->best_choice->unichar_lengths().string ()))
    return FALSE;

  word_char_quality(word, row, &char_quality, &accepted_char_quality);

  if ((word->best_choice->unichar_lengths().length () == char_quality) &&
    (char_quality == accepted_char_quality))
    return TRUE;
  else
    return FALSE;
}

BOOL8 Tesseract::repeated_ch_string(const char *rep_ch_str,
                                    const char *lengths) {
  UNICHAR_ID c;

  if ((rep_ch_str == NULL) || (*rep_ch_str == '\0')) {
    return FALSE;
  }

  c = unicharset.unichar_to_id(rep_ch_str, *lengths);
  rep_ch_str += *(lengths++);
  while (*rep_ch_str != '\0' &&
         unicharset.unichar_to_id(rep_ch_str, *lengths) == c) {
    rep_ch_str++;
  }
  if (*rep_ch_str == '\0')
    return TRUE;
  return FALSE;
}


inT16 Tesseract::safe_dict_word(const WERD_CHOICE &word) {
  int dict_word_type = dict_word(word);
  return dict_word_type == DOC_DAWG_PERM ? 0 : dict_word_type;
}


void Tesseract::flip_hyphens(WERD_RES *word_res) {
  WERD_CHOICE *best_choice = word_res->best_choice;
  int i;
  PBLOB_IT outword_it;
  int prev_right = -9999;
  int next_left;
  TBOX out_box;
  float aspect_ratio;

  if (tessedit_lower_flip_hyphen <= 1)
    return;

  outword_it.set_to_list(word_res->outword->blob_list());
  UNICHAR_ID unichar_dash = unicharset.unichar_to_id("-");
  bool modified = false;
  for (i = 0, outword_it.mark_cycle_pt();
       i < best_choice->length() && !outword_it.cycled_list();
       ++i, outword_it.forward()) {
    out_box = outword_it.data()->bounding_box();
    if (outword_it.at_last())
      next_left = 9999;
    else
      next_left = outword_it.data_relative(1)->bounding_box().left();
    // Dont touch small or touching blobs - it is too dangerous.
    if ((out_box.width() > 8 * word_res->denorm.scale()) &&
        (out_box.left() > prev_right) && (out_box.right() < next_left)) {
      aspect_ratio = out_box.width() / (float) out_box.height();
      if (unicharset.eq(best_choice->unichar_id(i), ".")) {
        if (aspect_ratio >= tessedit_upper_flip_hyphen &&
            unicharset.contains_unichar_id(unichar_dash) &&
            unicharset.get_enabled(unichar_dash)) {
          /* Certain HYPHEN */
          best_choice->set_unichar_id(unichar_dash, i);
          modified = true;
          if (word_res->reject_map[i].rejected())
            word_res->reject_map[i].setrej_hyphen_accept();
        }
        if ((aspect_ratio > tessedit_lower_flip_hyphen) &&
          word_res->reject_map[i].accepted())
                                 //Suspected HYPHEN
          word_res->reject_map[i].setrej_hyphen ();
      }
      else if (best_choice->unichar_id(i) == unichar_dash) {
        if ((aspect_ratio >= tessedit_upper_flip_hyphen) &&
          (word_res->reject_map[i].rejected()))
          word_res->reject_map[i].setrej_hyphen_accept();
        //Certain HYPHEN

        if ((aspect_ratio <= tessedit_lower_flip_hyphen) &&
          (word_res->reject_map[i].accepted()))
                                 //Suspected HYPHEN
          word_res->reject_map[i].setrej_hyphen();
      }
    }
    prev_right = out_box.right();
  }
  if (modified) {
    best_choice->populate_unichars(unicharset);
  }
}

void Tesseract::flip_0O(WERD_RES *word_res) {
  WERD_CHOICE *best_choice = word_res->best_choice;
  int i;
  PBLOB_IT outword_it;
  TBOX out_box;

  if (!tessedit_flip_0O)
    return;

  outword_it.set_to_list(word_res->outword->blob_list ());

  for (i = 0, outword_it.mark_cycle_pt ();
       i < best_choice->length() && !outword_it.cycled_list ();
       ++i, outword_it.forward ()) {
    if (unicharset.get_isupper(best_choice->unichar_id(i)) ||
        unicharset.get_isdigit(best_choice->unichar_id(i))) {
      out_box = outword_it.data()->bounding_box ();
      if ((out_box.top() < bln_baseline_offset + bln_x_height) ||
        (out_box.bottom() > bln_baseline_offset + bln_x_height / 4))
        return;                  //Beware words with sub/superscripts
    }
  }
  UNICHAR_ID unichar_0 = unicharset.unichar_to_id("0");
  UNICHAR_ID unichar_O = unicharset.unichar_to_id("O");
  if (unichar_0 == INVALID_UNICHAR_ID || !unicharset.get_enabled(unichar_0) ||
      unichar_O == INVALID_UNICHAR_ID || !unicharset.get_enabled(unichar_O)) {
    return;  // 0 or O are not present/enabled in unicharset
  }
  bool modified = false;
  for (i = 1; i < best_choice->length(); ++i, outword_it.forward ()) {
    if (best_choice->unichar_id(i) == unichar_0 ||
        best_choice->unichar_id(i) == unichar_O) {
      /* A0A */
      if ((i+1) < best_choice->length() &&
          non_O_upper(best_choice->unichar_id(i-1)) &&
          non_O_upper(best_choice->unichar_id(i+1))) {
        best_choice->set_unichar_id(unichar_O, i);
        modified = true;
      }
      /* A00A */
      if (non_O_upper(best_choice->unichar_id(i-1)) &&
          (i+1) < best_choice->length() &&
          (best_choice->unichar_id(i+1) == unichar_0 ||
           best_choice->unichar_id(i+1) == unichar_O) &&
          (i+2) < best_choice->length() &&
          non_O_upper(best_choice->unichar_id(i+2))) {
        best_choice->set_unichar_id(unichar_O, i);
        modified = true;
        i++;
      }
      /* AA0<non digit or end of word> */
      if ((i > 1) &&
          non_O_upper(best_choice->unichar_id(i-2)) &&
          non_O_upper(best_choice->unichar_id(i-1)) &&
          (((i+1) < best_choice->length() &&
            !unicharset.get_isdigit(best_choice->unichar_id(i+1)) &&
            !unicharset.eq(best_choice->unichar_id(i+1), "l") &&
            !unicharset.eq(best_choice->unichar_id(i+1), "I")) ||
           (i == best_choice->length() - 1))) {
        best_choice->set_unichar_id(unichar_O, i);
        modified = true;
      }
      /* 9O9 */
      if (non_0_digit(best_choice->unichar_id(i-1)) &&
          (i+1) < best_choice->length() &&
          non_0_digit(best_choice->unichar_id(i+1))) {
        best_choice->set_unichar_id(unichar_0, i);
        modified = true;
      }
      /* 9OOO */
      if (non_0_digit(best_choice->unichar_id(i-1)) &&
          (i+2) < best_choice->length() &&
          (best_choice->unichar_id(i+1) == unichar_0 ||
           best_choice->unichar_id(i+1) == unichar_O) &&
          (best_choice->unichar_id(i+2) == unichar_0 ||
           best_choice->unichar_id(i+2) == unichar_O)) {
        best_choice->set_unichar_id(unichar_0, i);
        best_choice->set_unichar_id(unichar_0, i+1);
        best_choice->set_unichar_id(unichar_0, i+2);
        modified = true;
        i += 2;
      }
      /* 9OO<non upper> */
      if (non_0_digit(best_choice->unichar_id(i-1)) &&
          (i+2) < best_choice->length() &&
          (best_choice->unichar_id(i+1) == unichar_0 ||
          best_choice->unichar_id(i+1) == unichar_O) &&
          !unicharset.get_isupper(best_choice->unichar_id(i+2))) {
        best_choice->set_unichar_id(unichar_0, i);
        best_choice->set_unichar_id(unichar_0, i+1);
        modified = true;
        i++;
      }
      /* 9O<non upper> */
      if (non_0_digit(best_choice->unichar_id(i-1)) &&
          (i+1) < best_choice->length() &&
          !unicharset.get_isupper(best_choice->unichar_id(i+1))) {
        best_choice->set_unichar_id(unichar_0, i);
      }
      /* 9[.,]OOO.. */
      if ((i > 1) &&
          (unicharset.eq(best_choice->unichar_id(i-1), ".") ||
           unicharset.eq(best_choice->unichar_id(i-1), ",")) &&
          (unicharset.get_isdigit(best_choice->unichar_id(i-2)) ||
           best_choice->unichar_id(i-2) == unichar_O)) {
        if (best_choice->unichar_id(i-2) == unichar_O) {
          best_choice->set_unichar_id(unichar_0, i-2);
          modified = true;
        }
        while (i < best_choice->length() &&
               (best_choice->unichar_id(i) == unichar_O ||
                best_choice->unichar_id(i) == unichar_0)) {
          best_choice->set_unichar_id(unichar_0, i);
          modified = true;
          i++;
        }
        i--;
      }
    }
  }
  if (modified) {
    best_choice->populate_unichars(unicharset);
  }
}

BOOL8 Tesseract::non_O_upper(UNICHAR_ID unichar_id) {
  return (unicharset.get_isupper(unichar_id) &&
          (!unicharset.eq(unichar_id, "O")));
}

BOOL8 Tesseract::non_0_digit(UNICHAR_ID unichar_id) {
  return (unicharset.get_isdigit(unichar_id) &&
          (!unicharset.eq(unichar_id, "0")));
}
}  // namespace tesseract
