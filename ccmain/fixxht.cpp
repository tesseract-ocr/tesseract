/**********************************************************************
 * File:        fixxht.cpp  (Formerly fixxht.c)
 * Description: Improve x_ht and look out for case inconsistencies
 * Author:		Phil Cheatle
 * Created:		Thu Aug  5 14:11:08 BST 1993
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
#endif

#include "mfcpch.h"
#include          <string.h>
#include          <ctype.h>
#include          "varable.h"
#include          "tessvars.h"
#include          "control.h"
#include          "reject.h"
#include          "fixxht.h"
#include          "secname.h"
#include          "tesseractclass.h"

#define EXTERN

EXTERN double_VAR (x_ht_fraction_of_caps_ht, 0.7,
"Fract of cps ht est of xht");
EXTERN double_VAR (x_ht_variation, 0.35,
"Err band as fract of caps/xht dist");
EXTERN double_VAR (x_ht_sub_variation, 0.5,
"Err band as fract of caps/xht dist");
EXTERN BOOL_VAR (rej_trial_ambigs, TRUE,
"reject x-ht ambigs when under trial");
EXTERN BOOL_VAR (x_ht_conservative_ambigs, FALSE,
"Dont rely on ambigs + maxht");
EXTERN BOOL_VAR (x_ht_check_est, TRUE, "Cross check estimates");
EXTERN BOOL_VAR (x_ht_case_flip, FALSE, "Flip or reject suspect case");
EXTERN BOOL_VAR (x_ht_include_dodgy_blobs, TRUE,
"Include blobs with possible noise?");
EXTERN BOOL_VAR (x_ht_limit_flip_trials, TRUE,
"Dont do trial flips when ambigs are close to xht?");
EXTERN BOOL_VAR (rej_use_check_block_occ, TRUE,
"Analyse rejection behaviour");

EXTERN STRING_VAR (chs_non_ambig_caps_ht,
"!#$%&()/12346789?ABDEFGHIKLNQRT[]\\bdfhkl",
"Reliable ascenders");
EXTERN STRING_VAR (chs_x_ht, "acegmnopqrsuvwxyz", "X height chars");
EXTERN STRING_VAR (chs_non_ambig_x_ht, "aenqr", "reliable X height chars");
EXTERN STRING_VAR (chs_ambig_caps_x, "cCmMoO05sSuUvVwWxXzZ",
"X ht or caps ht chars");
EXTERN STRING_VAR (chs_bl_ambig_caps_x, "pPyY", " Caps or descender ambigs");

/* The following arent used in this module but are used in applybox.c */
EXTERN STRING_VAR (chs_caps_ht,
"!#$%&()/0123456789?ABCDEFGHIJKLMNOPQRSTUVWXYZ[]\\bdfhkl{|}",
"Ascender chars");
EXTERN STRING_VAR (chs_desc, "gjpqy", "Descender chars");
EXTERN STRING_VAR (chs_non_ambig_bl,
"!#$%&01246789?ABCDEFGHIKLMNORSTUVWXYZabcdehiklmnorstuvwxz",
"Reliable baseline chars");
EXTERN STRING_VAR (chs_odd_top, "ijt", "Chars with funny ascender region");
EXTERN STRING_VAR (chs_odd_bot, "()35JQ[]\\/{}|", "Chars with funny base");

/* The following arent used but are defined for completeness */
EXTERN STRING_VAR (chs_bl,
"!#$%&()/01246789?ABCDEFGHIJKLMNOPRSTUVWXYZ[]\\abcdefhiklmnorstuvwxz{}",
"Baseline chars");
EXTERN STRING_VAR (chs_non_ambig_desc, "gq", "Reliable descender chars");

/**
 * re_estimate_x_ht()
 *
 * Walk the blobs in the word together with the text string and reject map.
 * NOTE: All evaluation is done on the baseline normalised word. This is so that
 * the TBOX class can be used (integer). The reasons for this are:
 *   a) We must use the outword - ie the Tess result
 *   b) The outword is always converted to integer representation as that is how
 *      Tess works
 *   c) We would like to use the TBOX class, cos its there - this is integer
 *      precision.
 *   d) If we de-normed the outword we would get rounding errors and would find
 *      that integers are too imprecise (x-height around 15 pixels instead of a
 *      scale of 128 in bln form.
 *   CONVINCED?
 *
 * A) Try to re-estimatate x-ht and caps ht from confirmed pts in word.
 *
 * @verbatim
     FOR each non reject blob
        IF char is baseline posn ambiguous
 			Remove ambiguity by comparing its posn with respect to baseline.
 		IF char is a confirmed x-ht char
 			Add x-ht posn to confirmed_x_ht pts for word
     IF char is a confirmed caps-ht char
 			Add blob_ht to caps ht pts for word
 
     IF Std Dev of caps hts < 2  (AND # samples > 0)
 		Use mean as caps ht estimate (Dont use median as we can expect a
 			fair variation between the heights of the NON_AMBIG_CAPS_HT_CHS)
     IF Std Dev of caps hts >= 2  (AND # samples > 0)
 			Suspect small caps font.
 			Look for 2 clusters,	each with Std Dev < 2.
 			IF 2 clusters found
 			Pick the smaller median as the caps ht estimate of the smallcaps.
 
     IF failed to estimate a caps ht
        Use the median caps ht if there is one,
 		ELSE use the caps ht estimate of the previous word. NO!!!
 
 
     IF there are confirmed x-height chars
 			Estimate confirmed x-height as the median value
     ELSE IF there is a confirmed caps ht
 			Estimate confirmed x-height as a fraction of confirmed caps ht value
 		ELSE
 			Use the value for the previous word or the row value if this is the
 			first word in the block. NO!!!
   @endverbatim
 *
 * B) Add in case ambiguous blobs based on confirmed x-ht/caps ht, changing case
 *    as necessary. Reestimate caps ht and x-ht as in A, using the extended
 *    clusters.
 *
 * C) If word contains rejects, and x-ht estimate significantly differs from
 *    original estimate, return TRUE so that the word can be rematched
 */

void re_estimate_x_ht(                     //improve for 1 word
                      WERD_RES *word_res,  //word to do
                      float *trial_x_ht    //new match value
                     ) {
  PBLOB_IT blob_it;
  inT16 blob_ht_above_baseline;

  const char *word_str;
  inT16 i;
  inT16 offset;

  STATS all_blobs_ht (0, 300);   //every blob in word
  STATS x_ht (0, 300);           //confirmed pts in wd
  STATS caps_ht (0, 300);        //confirmed pts in wd
  STATS case_ambig (0, 300);     //lower case ambigs

  inT16 rej_blobs_count = 0;
  inT16 rej_blobs_max_height = 0;
  inT32 rej_blobs_max_area = 0;
  float x_ht_ok_variation;
  float max_blob_ht;
  float marginally_above_x_ht;

  TBOX blob_box;                  //blob bounding box
  float est_x_ht = 0.0;          //word estimate
  float est_caps_ht = 0.0;       //word estimate
                                 //based on hard data?
  BOOL8 est_caps_ht_certain = FALSE;
  BOOL8 est_x_ht_certain = FALSE;//based on hard data?
  BOOL8 trial = FALSE;           //Sepeculative values?
  BOOL8 no_comment = FALSE;      //No change in xht
  float ambig_lc_x_est;
  float ambig_uc_caps_est;
  inT16 x_ht_ambigs = 0;
  inT16 caps_ht_ambigs = 0;

  /* Calculate default variation of blob x_ht from bln x_ht for bln word */
  x_ht_ok_variation =
    (bln_x_height / x_ht_fraction_of_caps_ht - bln_x_height) * x_ht_variation;

  word_str = word_res->best_choice->unichar_string().string();
  /*
    Cycle blobs, allocating to one of the stats sets when possible.
  */
  blob_it.set_to_list (word_res->outword->blob_list ());
  for (blob_it.mark_cycle_pt (), i = 0, offset = 0;
  !blob_it.cycled_list (); blob_it.forward (),
           offset += word_res->best_choice->unichar_lengths()[i++]) {
    if (!dodgy_blob (blob_it.data ())) {
      blob_box = blob_it.data ()->bounding_box ();
      blob_ht_above_baseline = blob_box.top () - bln_baseline_offset;
      all_blobs_ht.add (blob_ht_above_baseline, 1);

      if (word_res->reject_map[i].rejected ()) {
        rej_blobs_count++;
        if (blob_box.height () > rej_blobs_max_height)
          rej_blobs_max_height = blob_box.height ();
        if (blob_box.area () > rej_blobs_max_area)
          rej_blobs_max_area = blob_box.area ();
      }
      else {
        if (STRING (chs_non_ambig_x_ht).contains (word_str[offset]))
          x_ht.add (blob_ht_above_baseline, 1);

        if (STRING (chs_non_ambig_caps_ht).contains (word_str[offset]))
          caps_ht.add (blob_ht_above_baseline, 1);

        if (STRING (chs_ambig_caps_x).contains (word_str[offset])) {
          case_ambig.add (blob_ht_above_baseline, 1);
          if (STRING (chs_x_ht).contains (word_str[offset]))
            x_ht_ambigs++;
          else
            caps_ht_ambigs++;
        }

        if (STRING (chs_bl_ambig_caps_x).contains (word_str[offset])) {
          if (STRING (chs_x_ht).contains (word_str[offset])) {
            /* confirm x_height provided > 15% total height below baseline */
            if ((bln_baseline_offset - blob_box.bottom ()) /
              (float) blob_box.height () > 0.15)
              x_ht.add (blob_ht_above_baseline, 1);
          }
          else {
            /* confirm caps_height provided < 5% total height below baseline */
            if ((bln_baseline_offset - blob_box.bottom ()) /
              (float) blob_box.height () < 0.05)
              caps_ht.add (blob_ht_above_baseline, 1);
          }
        }
      }
    }
  }
  est_caps_ht = estimate_from_stats (caps_ht);
  est_x_ht = estimate_from_stats (x_ht);
  est_ambigs(word_res, case_ambig, &ambig_lc_x_est, &ambig_uc_caps_est);
  max_blob_ht = all_blobs_ht.ile (0.9999);

  #ifndef SECURE_NAMES
  if (debug_x_ht_level >= 20) {
    tprintf ("Mode20:A: %s ", word_str);
    word_res->reject_map.print (debug_fp);
    tprintf (" XHT:%f CAP:%f MAX:%f AMBIG X:%f CAP:%f\n",
      est_x_ht, est_caps_ht, max_blob_ht,
      ambig_lc_x_est, ambig_uc_caps_est);
  }
  #endif
  if (!x_ht_conservative_ambigs &&
    (ambig_lc_x_est > 0) &&
    (ambig_lc_x_est == ambig_uc_caps_est) &&
  (max_blob_ht > ambig_lc_x_est + x_ht_ok_variation)) {
                                 //may be zero but believe xht
    ambig_uc_caps_est = est_caps_ht;
    #ifndef SECURE_NAMES
    if (debug_x_ht_level >= 20)
      tprintf ("Mode20:B: Fiddle ambig_uc_caps_est to %f\n",
        ambig_lc_x_est);
    #endif
  }

  /* Now make some estimates */

  if ((est_x_ht > 0) || (est_caps_ht > 0) ||
      ((ambig_lc_x_est > 0) && (ambig_lc_x_est != ambig_uc_caps_est))) {
    /* There is some sensible data to go on so make the most of it. */
    if (debug_x_ht_level >= 20)
      tprintf ("Mode20:C: Sensible Data\n", ambig_lc_x_est);
    if (est_x_ht > 0) {
      est_x_ht_certain = TRUE;
      if (est_caps_ht == 0) {
        if ((ambig_uc_caps_est > ambig_lc_x_est) &&
            (ambig_uc_caps_est > est_x_ht + x_ht_ok_variation))
          est_caps_ht = ambig_uc_caps_est;
        else
          est_caps_ht = est_x_ht / x_ht_fraction_of_caps_ht;
      }
      if (case_ambig.get_total () > 0)
        improve_estimate(word_res, est_x_ht, est_caps_ht, x_ht, caps_ht);
      est_caps_ht_certain = caps_ht.get_total () > 0;
      #ifndef SECURE_NAMES
      if (debug_x_ht_level >= 20)
        tprintf ("Mode20:D: Est from xht XHT:%f CAP:%f\n",
          est_x_ht, est_caps_ht);
      #endif
    }
    else if (est_caps_ht > 0) {
      est_caps_ht_certain = TRUE;
      if ((ambig_lc_x_est > 0) &&
        (ambig_lc_x_est < est_caps_ht - x_ht_ok_variation))
        est_x_ht = ambig_lc_x_est;
      else
        est_x_ht = est_caps_ht * x_ht_fraction_of_caps_ht;
      if (ambig_lc_x_est + ambig_uc_caps_est > 0)
        improve_estimate(word_res, est_x_ht, est_caps_ht, x_ht, caps_ht);
      est_x_ht_certain = x_ht.get_total () > 0;
      #ifndef SECURE_NAMES
      if (debug_x_ht_level >= 20)
        tprintf ("Mode20:E: Est from caps XHT:%f CAP:%f\n",
          est_x_ht, est_caps_ht);
      #endif
    }
    else {
      /* Do something based on case ambig chars alone - we have guessed that the
        ambigs are lower case. */
      est_x_ht = ambig_lc_x_est;
      est_x_ht_certain = TRUE;
      if (ambig_uc_caps_est > ambig_lc_x_est) {
        est_caps_ht = ambig_uc_caps_est;
        est_caps_ht_certain = TRUE;
      }
      else
        est_caps_ht = est_x_ht / x_ht_fraction_of_caps_ht;

      #ifndef SECURE_NAMES
      if (debug_x_ht_level >= 20)
        tprintf ("Mode20:F: Est from ambigs XHT:%f CAP:%f\n",
          est_x_ht, est_caps_ht);
      #endif
    }
    /* Check for sane interpretation of evidence:
      Try shifting caps ht if min certain caps ht is not significantly greater
      than the estimated x ht or the max certain x ht is not significantly less
      than the estimated caps ht. */
    if (x_ht_check_est) {
      if ((caps_ht.get_total () > 0) &&
      (est_x_ht + x_ht_ok_variation >= caps_ht.ile (0.0001))) {
        trial = TRUE;
        est_caps_ht = est_x_ht;
        est_x_ht = x_ht_fraction_of_caps_ht * est_caps_ht;

        #ifndef SECURE_NAMES
        if (debug_x_ht_level >= 20)
          tprintf ("Mode20:G: Trial XHT:%f CAP:%f\n",
            est_x_ht, est_caps_ht);
        #endif
      }
      else if ((x_ht.get_total () > 0) &&
      (est_caps_ht - x_ht_ok_variation <= x_ht.ile (0.9999))) {
        trial = TRUE;
        est_x_ht = est_caps_ht;
        est_caps_ht = est_x_ht / x_ht_fraction_of_caps_ht;
        #ifndef SECURE_NAMES
        if (debug_x_ht_level >= 20)
          tprintf ("Mode20:H: Trial XHT:%f CAP:%f\n",
            est_x_ht, est_caps_ht);
        #endif
      }
    }
  }

  else {
    /* There is no sensible data so we're in the dark. */

    marginally_above_x_ht = bln_x_height +
      x_ht_ok_variation * x_ht_sub_variation;
    /*
      If there are no rejects, or the only rejects have a narrow height, or have
      a small area compared to a normal char, then estimate the x-height as the
      original one. (I.e dont fiddle about if the only rejects look like
      punctuation) - we use max height as mean or median will be too low if
      there are only two blobs - Eg "F."
    */

    if (debug_x_ht_level >= 20)
      tprintf ("Mode20:I: In the dark\n");

    if ((rej_blobs_count == 0) ||
      (rej_blobs_max_height < 0.3 * max_blob_ht) ||
    (rej_blobs_max_area < 0.3 * max_blob_ht * max_blob_ht)) {
      no_comment = TRUE;
      if (debug_x_ht_level >= 20)
        tprintf ("Mode20:J: No comment due to no rejects\n");
    }
    else if (x_ht_limit_flip_trials &&
             ((max_blob_ht < marginally_above_x_ht) ||
             ((ambig_lc_x_est > 0) &&
             (ambig_lc_x_est == ambig_uc_caps_est) &&
             (ambig_lc_x_est < marginally_above_x_ht)))) {
      no_comment = TRUE;
      if (debug_x_ht_level >= 20)
        tprintf ("Mode20:K: No comment as close to xht %f < %f\n",
          ambig_lc_x_est, marginally_above_x_ht);
    }
    else if (x_ht_conservative_ambigs && (ambig_uc_caps_est > 0)) {
      trial = TRUE;
      est_caps_ht = ambig_lc_x_est;
      est_x_ht = x_ht_fraction_of_caps_ht * est_caps_ht;

      #ifndef SECURE_NAMES
      if (debug_x_ht_level >= 20)
        tprintf ("Mode20:L: Trial XHT:%f CAP:%f\n",
          est_x_ht, est_caps_ht);
      #endif
    }
    /*
      If the top of the word is nowhere near where we expect ascenders to be
      (less than half the x_ht -> caps_ht distance) - suspect an all caps word
      at the x-ht. Estimate x-ht accordingly - but only as a TRIAL!
      NOTE we do NOT check location of baseline. Commas can descend as much as
      real descenders so we would need to do something to make sure that any
      disqualifying descenders were not at the end.
    */
    else {
      if (max_blob_ht <
          (bln_x_height + bln_x_height / x_ht_fraction_of_caps_ht) / 2.0) {
        trial = TRUE;
        est_x_ht = x_ht_fraction_of_caps_ht * max_blob_ht;
        est_caps_ht = max_blob_ht;

        #ifndef SECURE_NAMES
        if (debug_x_ht_level >= 20)
          tprintf ("Mode20:M: Trial XHT:%f CAP:%f\n",
            est_x_ht, est_caps_ht);
        #endif
      }
      else {
        no_comment = TRUE;
        if (debug_x_ht_level >= 20)
          tprintf ("Mode20:N: No comment as nothing else matched\n");
      }
    }
  }

  /* Sanity check - reject word if fails */

  if (!no_comment &&
      ((est_x_ht > 2 * bln_x_height) ||
       (est_x_ht / word_res->denorm.scale () <= min_sane_x_ht_pixels) ||
       (est_caps_ht <= est_x_ht) || (est_caps_ht >= 2.5 * est_x_ht))) {
    no_comment = TRUE;
    if (!trial && rej_use_xht) {
      if (debug_x_ht_level >= 2) {
        tprintf ("Sanity check rejecting %s ", word_str);
        word_res->reject_map.print (debug_fp);
        tprintf ("\n");
      }
      word_res->reject_map.rej_word_xht_fixup ();

    }
    if (debug_x_ht_level >= 20)
      tprintf ("Mode20:O: No comment as nothing else matched\n");
  }

  if (no_comment || trial) {
    word_res->x_height = bln_x_height / word_res->denorm.scale ();
    word_res->guessed_x_ht = TRUE;
    word_res->caps_height = (bln_x_height / x_ht_fraction_of_caps_ht) /
      word_res->denorm.scale ();
    word_res->guessed_caps_ht = TRUE;
    /*
    Reject ambigs in the current word if we are uncertain and:
        there are rejects OR
        there is only one char which is an ambig OR
        there is conflict between the case of the ambigs even though there is
        no height separation Eg "Ms" recognised from "MS"
    */
    if (rej_trial_ambigs &&
      ((word_res->reject_map.reject_count () > 0) ||
      (word_res->reject_map.length () == 1) ||
    ((x_ht_ambigs > 0) && (caps_ht_ambigs > 0)))) {
      #ifndef SECURE_NAMES
      if (debug_x_ht_level >= 2) {
        tprintf ("TRIAL Rej Ambigs %s ", word_str);
        word_res->reject_map.print (debug_fp);
      }
      #endif
      reject_ambigs(word_res);
      if (debug_x_ht_level >= 2) {
        tprintf (" ");
        word_res->reject_map.print (debug_fp);
        tprintf ("\n");
      }
    }
  }
  else {
    word_res->x_height = est_x_ht / word_res->denorm.scale ();
    word_res->guessed_x_ht = !est_x_ht_certain;
    word_res->caps_height = est_caps_ht / word_res->denorm.scale ();
    word_res->guessed_caps_ht = !est_caps_ht_certain;
  }

  if (!no_comment && (fabs (est_x_ht - bln_x_height) > x_ht_ok_variation))
    *trial_x_ht = est_x_ht / word_res->denorm.scale ();
  else
    *trial_x_ht = 0.0;

  #ifndef SECURE_NAMES
  if (((*trial_x_ht > 0) && (debug_x_ht_level >= 3)) ||
      (debug_x_ht_level >= 5)) {
    tprintf ("%s ", word_str);
    word_res->reject_map.print (debug_fp);
    tprintf
      (" X:%0.2f Cps:%0.2f Mxht:%0.2f RJ MxHt:%d MxAr:%d Rematch:%c\n",
      est_x_ht, est_caps_ht, max_blob_ht, rej_blobs_max_height,
      rej_blobs_max_area, *trial_x_ht > 0 ? '*' : ' ');
  }
  #endif

}


namespace tesseract {
/**
 * check_block_occ()
 * Checks word for coarse block occupancy, rejecting more chars and flipping
 * case of case ambiguous chars as required.
 */
void Tesseract::check_block_occ(WERD_RES *word_res) {
  PBLOB_IT blob_it;
  STRING new_string;
  STRING new_string_lengths(word_res->best_choice->unichar_lengths());
  REJMAP new_map = word_res->reject_map;
  WERD_CHOICE *new_choice;

  const char *word_str = word_res->best_choice->unichar_string().string();
  inT16 i;
  inT16 offset;
  inT16 reject_count = 0;
  char confirmed_char[UNICHAR_LEN + 1];
  char temp_char[UNICHAR_LEN + 1];
  float x_ht;
  float caps_ht;

  new_string_lengths[0] = 0;

  if (word_res->x_height > 0)
    x_ht = word_res->x_height * word_res->denorm.scale ();
  else
    x_ht = bln_x_height;

  if (word_res->caps_height > 0)
    caps_ht = word_res->caps_height * word_res->denorm.scale ();
  else
    caps_ht = x_ht / x_ht_fraction_of_caps_ht;

  blob_it.set_to_list (word_res->outword->blob_list ());

  for (blob_it.mark_cycle_pt (), i = 0, offset = 0;
  !blob_it.cycled_list (); blob_it.forward (),
           offset += word_res->best_choice->unichar_lengths()[i++]) {
    strncpy(temp_char, word_str + offset,
            word_res->best_choice->unichar_lengths()[i]); //default copy
    temp_char[word_res->best_choice->unichar_lengths()[i]] = '\0';
    if (word_res->reject_map[i].accepted ()) {
      check_blob_occ (temp_char,
                      blob_it.data ()->bounding_box ().
                      top () - bln_baseline_offset, x_ht,
                      caps_ht, confirmed_char);

      if (strcmp(confirmed_char, "") == 0) {
        if (rej_use_check_block_occ) {
          new_map[i].setrej_xht_fixup ();
          reject_count++;
        }
      }
      else
        strcpy(temp_char, confirmed_char);
    }
    new_string += temp_char;
    new_string_lengths[i] = strlen(temp_char);
    new_string_lengths[i + 1] = 0;

  }
  if ((reject_count > 0) || (new_string != word_str)) {
    if (debug_x_ht_level >= 2) {
      tprintf ("Shape Verification: %s ", word_str);
      word_res->reject_map.print (debug_fp);
      tprintf (" -> %s ", new_string.string ());
      new_map.print (debug_fp);
      tprintf ("\n");
    }
    new_choice = new WERD_CHOICE(new_string.string(),
                                 new_string_lengths.string(),
                                 word_res->best_choice->rating(),
                                 word_res->best_choice->certainty(),
                                 word_res->best_choice->permuter(),
                                 unicharset);
    new_choice->populate_unichars(unicharset);
    delete word_res->best_choice;
    word_res->best_choice = new_choice;
    word_res->reject_map = new_map;
  }
}
}  // namespace tesseract

/**
 * check_blob_occ()
 *
 * Checks blob for position relative to position above baseline
 * @return 0 for reject, or (possibly case shifted) confirmed char
 */

void check_blob_occ(char* proposed_char,
                    inT16 blob_ht_above_baseline,
                    float x_ht,
                    float caps_ht,
                    char* confirmed_char) {
  BOOL8 blob_definite_x_ht;
  BOOL8 blob_definite_caps_ht;
  float acceptable_variation;

  acceptable_variation = (caps_ht - x_ht) * x_ht_variation;
  /* ??? REJECT if expected descender and nothing significantly below BL */

  /* ??? REJECT if expected ascender and nothing significantly above x-ht */

  /*
    IF AMBIG_CAPS_X_CHS
      IF blob is definitely an ascender ( > xht + xht err )AND
        char is an x-ht char
      THEN
        flip case
      IF blob is defintiely an x-ht ( <= xht + xht err ) AND
        char is an ascender char
      THEN
        flip case
  */
  blob_definite_x_ht = blob_ht_above_baseline <= x_ht + acceptable_variation;
  blob_definite_caps_ht = blob_ht_above_baseline >=
    caps_ht - acceptable_variation;

  if (STRING (chs_ambig_caps_x).contains (*proposed_char)) {
    if ((!blob_definite_x_ht && !blob_definite_caps_ht) ||
        ((strcmp(proposed_char, "0") == 0) && !blob_definite_caps_ht) ||
        ((strcmp(proposed_char, "o") == 0) && !blob_definite_x_ht)) {
      strcpy(confirmed_char, "");
      return;
    }

    else if (blob_definite_caps_ht &&
    STRING (chs_x_ht).contains (*proposed_char)) {
      if (x_ht_case_flip) {
                                 //flip to upper case
        proposed_char[0] = (char) toupper (*proposed_char);
        return;
      } else {
        strcpy(confirmed_char, "");
        return;
      }
    }

    else if (blob_definite_x_ht &&
    !STRING (chs_x_ht).contains (*proposed_char)) {
      if (x_ht_case_flip) {
                                 //flip to lower case
        proposed_char[0] = (char) tolower (*proposed_char);
      } else {
        strcpy(confirmed_char, "");
        return;
      }
    }
  }
  else
  if ((STRING (chs_non_ambig_x_ht).contains (*proposed_char)
    && !blob_definite_x_ht)
    || (STRING (chs_non_ambig_caps_ht).contains (*proposed_char)
        && !blob_definite_caps_ht)) {
    strcpy(confirmed_char, "");
    return;
  }
  strcpy(confirmed_char, proposed_char);
  return;
}


float estimate_from_stats(STATS &stats) {
  if (stats.get_total () <= 0)
    return 0.0;
  else if (stats.get_total () >= 3)
    return stats.ile (0.5);      //median
  else
    return stats.mean ();
}


void improve_estimate(WERD_RES *word_res,
                      float &est_x_ht,
                      float &est_caps_ht,
                      STATS &x_ht,
                      STATS &caps_ht) {
  PBLOB_IT blob_it;
  inT16 blob_ht_above_baseline;

  const char *word_str;
  inT16 i;
  inT16 offset;
  TBOX blob_box;                  //blob bounding box
  char confirmed_char[UNICHAR_LEN + 1];
  char temp_char[UNICHAR_LEN + 1];
  float new_val;

  /* IMPROVE estimates here - if good estimates, and case ambig chars,
    rescan blobs to fix case ambig blobs, re-estimate hts  ??? maybe always do
    it after deciding x-height
  */

  blob_it.set_to_list (word_res->outword->blob_list ());
  word_str = word_res->best_choice->unichar_string().string();
  for (blob_it.mark_cycle_pt (), i = 0, offset = 0;
       !blob_it.cycled_list (); blob_it.forward (),
           offset += word_res->best_choice->unichar_lengths()[i++]) {
    if ((STRING (chs_ambig_caps_x).contains (word_str[offset])) &&
        (!dodgy_blob (blob_it.data ()))) {
      blob_box = blob_it.data ()->bounding_box ();
      blob_ht_above_baseline = blob_box.top () - bln_baseline_offset;
      strncpy(temp_char, word_str + offset,
              word_res->best_choice->unichar_lengths()[i]);
      temp_char[word_res->best_choice->unichar_lengths()[i]] = '\0';
      check_blob_occ (temp_char,
                      blob_ht_above_baseline,
                      est_x_ht, est_caps_ht, confirmed_char);
      if (strcmp(confirmed_char, "") != 0) {
        if (STRING (chs_x_ht).contains (*confirmed_char))
          x_ht.add (blob_ht_above_baseline, 1);
        else
          caps_ht.add (blob_ht_above_baseline, 1);
      }
    }
  }
  new_val = estimate_from_stats (x_ht);
  if (new_val > 0)
    est_x_ht = new_val;
  new_val = estimate_from_stats (caps_ht);
  if (new_val > 0)
    est_caps_ht = new_val;
}


void reject_ambigs(  //rej any accepted xht ambig chars
                   WERD_RES *word) {
  const char *word_str;
  int i = 0;

  word_str = word->best_choice->unichar_string().string();
  while (*word_str != '\0') {
    if (STRING (chs_ambig_caps_x).contains (*word_str))
      word->reject_map[i].setrej_xht_fixup ();
    word_str += word->best_choice->unichar_lengths()[i++];
  }
}


void est_ambigs(                          //xht ambig ht stats
                WERD_RES *word_res,
                STATS &stats,
                float *ambig_lc_x_est,    //xht est
                float *ambig_uc_caps_est  //caps est
               ) {
  float x_ht_ok_variation;
  STATS short_ambigs (0, 300);
  STATS tall_ambigs (0, 300);
  PBLOB_IT blob_it;
  TBOX blob_box;                  //blob bounding box
  inT16 blob_ht_above_baseline;

  const char *word_str;
  inT16 i;
  inT16 offset;
  float min;                     //min ambig ch ht
  float max;                     //max ambig ch ht
  float short_limit;             // for lower case
  float tall_limit;              // for upper case

  x_ht_ok_variation =
    (bln_x_height / x_ht_fraction_of_caps_ht - bln_x_height) * x_ht_variation;

  if (stats.get_total () == 0) {
    *ambig_lc_x_est = 0;
    *ambig_uc_caps_est = 0;
  }
  else {
    min = stats.ile (0.0);
    max = stats.ile (0.99999);
    if ((max - min) < x_ht_ok_variation) {
      *ambig_lc_x_est = *ambig_uc_caps_est = stats.mean ();
      //close enough
    }
    else {
    /* Try reclustering into lower and upper case chars */
      short_limit = min + (max - min) * x_ht_variation;
      tall_limit = max - (max - min) * x_ht_variation;
      word_str = word_res->best_choice->unichar_string().string();
      blob_it.set_to_list (word_res->outword->blob_list ());
      for (blob_it.mark_cycle_pt (), i = 0, offset = 0;
      !blob_it.cycled_list (); blob_it.forward (),
               offset += word_res->best_choice->unichar_lengths()[i++]) {
        if (word_res->reject_map[i].accepted () &&
          STRING (chs_ambig_caps_x).contains (word_str[offset]) &&
        (!dodgy_blob (blob_it.data ()))) {
          blob_box = blob_it.data ()->bounding_box ();
          blob_ht_above_baseline =
            blob_box.top () - bln_baseline_offset;
          if (blob_ht_above_baseline <= short_limit)
            short_ambigs.add (blob_ht_above_baseline, 1);
          else if (blob_ht_above_baseline >= tall_limit)
            tall_ambigs.add (blob_ht_above_baseline, 1);
        }
      }
      *ambig_lc_x_est = short_ambigs.mean ();
      *ambig_uc_caps_est = tall_ambigs.mean ();
      /* Cop out if we havent got sensible clusters. */
      if (*ambig_uc_caps_est - *ambig_lc_x_est <= x_ht_ok_variation)
        *ambig_lc_x_est = *ambig_uc_caps_est = stats.mean ();
      //close enough
    }
  }
}


/**
 * dodgy_blob()
 * Returns true if the blob has more than one outline, one above the other.
 * These are dodgy as the top blob could be noise, causing the bounding box xht
 * to be misleading
 */

BOOL8 dodgy_blob(PBLOB *blob) {
  OUTLINE_IT outline_it = blob->out_list ();
  inT16 highest_bottom = -MAX_INT16;
  inT16 lowest_top = MAX_INT16;
  TBOX outline_box;

  if (x_ht_include_dodgy_blobs)
    return FALSE;                //no blob is ever dodgy
  for (outline_it.mark_cycle_pt ();
  !outline_it.cycled_list (); outline_it.forward ()) {
    outline_box = outline_it.data ()->bounding_box ();
    if (lowest_top > outline_box.top ())
      lowest_top = outline_box.top ();
    if (highest_bottom < outline_box.bottom ())
      highest_bottom = outline_box.bottom ();
  }
  return highest_bottom >= lowest_top;
}
