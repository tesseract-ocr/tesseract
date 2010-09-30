/**********************************************************************
 * File:        adaptions.cpp  (Formerly adaptions.c)
 * Description: Functions used to adapt to blobs already confidently
 *					identified
 * Author:		Chris Newton
 * Created:		Thu Oct  7 10:17:28 BST 1993
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

#ifdef __UNIX__
#include          <assert.h>
#endif
#include          <ctype.h>
#include          <string.h>
#include          "tessbox.h"
#include          "tessvars.h"
#include          "memry.h"
#include          "mainblk.h"
#include          "charcut.h"
#include          "imgs.h"
#include          "scaleimg.h"
#include          "reject.h"
#include          "control.h"
#include          "adaptions.h"
#include          "stopper.h"
#include          "charsample.h"
#include          "matmatch.h"
#include          "secname.h"
#include          "tesseractclass.h"

// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif

inT32 demo_word = 0;

#define WINDOWNAMESIZE    13     /*max size of name */

#define EXTERN

EXTERN BOOL_VAR (tessedit_reject_ems, FALSE, "Reject all m's");
EXTERN BOOL_VAR (tessedit_reject_suspect_ems, FALSE, "Reject suspect m's");

EXTERN double_VAR (tessedit_cluster_t1, 0.20,
"t1 threshold for clustering samples");
EXTERN double_VAR (tessedit_cluster_t2, 0.40,
"t2 threshold for clustering samples");
EXTERN double_VAR (tessedit_cluster_t3, 0.12,
"Extra threshold for clustering samples, only keep a new sample if best score greater than this value");
EXTERN double_VAR (tessedit_cluster_accept_fraction, 0.80,
"Largest fraction of characters in cluster for it to be used for adaption");
EXTERN INT_VAR (tessedit_cluster_min_size, 3,
"Smallest number of samples in a cluster for it to be used for adaption");
EXTERN BOOL_VAR (tessedit_cluster_debug, FALSE,
"Generate and print debug information for adaption by clustering");
EXTERN BOOL_VAR (tessedit_use_best_sample, FALSE,
"Use best sample from cluster when adapting");
EXTERN BOOL_VAR (tessedit_test_cluster_input, FALSE,
"Set reject map to enable cluster input to be measured");

EXTERN BOOL_VAR (tessedit_matrix_match, TRUE, "Use matrix matcher");
EXTERN BOOL_VAR (tessedit_mm_use_non_adaption_set, FALSE,
"Don't try to adapt to characters on this list");
EXTERN STRING_VAR (tessedit_non_adaption_set, ",.;:'~@*",
"Characters to be avoided when adapting");
EXTERN BOOL_VAR (tessedit_mm_adapt_using_prototypes, TRUE,
"Use prototypes when adapting");
EXTERN BOOL_VAR (tessedit_mm_use_prototypes, TRUE,
"Use prototypes as clusters are built");
EXTERN BOOL_VAR (tessedit_mm_use_rejmap, FALSE,
"Adapt to characters using reject map");
EXTERN BOOL_VAR (tessedit_mm_all_rejects, FALSE,
"Adapt to all characters using, matrix matcher");
EXTERN BOOL_VAR (tessedit_mm_only_match_same_char, FALSE,
"Only match samples against clusters for the same character");
EXTERN BOOL_VAR (tessedit_process_rns, FALSE, "Handle m - rn ambigs");

EXTERN BOOL_VAR (tessedit_demo_adaption, FALSE,
"Display cut images and matrix match for demo purposes");
EXTERN INT_VAR (tessedit_demo_word1, 62,
"Word number of first word to display");
EXTERN INT_VAR (tessedit_demo_word2, 64,
"Word number of second word to display");
EXTERN STRING_VAR (tessedit_demo_file, "academe",
"Name of document containing demo words");
EXTERN BOOL_VAR(tessedit_adapt_to_char_fragments, TRUE,
                "Adapt to words that contain "
                " a character composed form fragments");

namespace tesseract {
BOOL8 Tesseract::word_adaptable(  //should we adapt?
                                WERD_RES *word,
                                uinT16 mode) {
  if (tessedit_adaption_debug) {
    tprintf("Running word_adaptable() for %s rating %.4f certainty %.4f\n",
          word->best_choice == NULL ? "" :
          word->best_choice->unichar_string().string(),
          word->best_choice->rating(), word->best_choice->certainty());
  }

  BOOL8 status = FALSE;
  BITS16 flags(mode);

  enum MODES
  {
    ADAPTABLE_WERD,
    ACCEPTABLE_WERD,
    CHECK_DAWGS,
    CHECK_SPACES,
    CHECK_ONE_ELL_CONFLICT,
    CHECK_AMBIG_WERD
  };

  /*
  0: NO adaption
  */
  if (mode == 0) {
    if (tessedit_adaption_debug) tprintf("adaption disabled\n");
    return FALSE;
  }

  if (flags.bit (ADAPTABLE_WERD)) {
    status |= word->tess_would_adapt;  // result of Classify::AdaptableWord()
    if (tessedit_adaption_debug && !status) {
      tprintf("tess_would_adapt bit is false\n");
    }
  }

  if (flags.bit (ACCEPTABLE_WERD)) {
    status |= word->tess_accepted;
    if (tessedit_adaption_debug && !status) {
      tprintf("tess_accepted bit is false\n");
    }
  }

  if (!status) {                  // If not set then
    return FALSE;                // ignore other checks
  }

  if (flags.bit (CHECK_DAWGS) &&
    (word->best_choice->permuter () != SYSTEM_DAWG_PERM) &&
    (word->best_choice->permuter () != FREQ_DAWG_PERM) &&
    (word->best_choice->permuter () != USER_DAWG_PERM) &&
    (word->best_choice->permuter () != NUMBER_PERM)) {
    if (tessedit_adaption_debug) tprintf("word not in dawgs\n");
    return FALSE;
  }

  if (flags.bit (CHECK_ONE_ELL_CONFLICT) && one_ell_conflict (word, FALSE)) {
    if (tessedit_adaption_debug) tprintf("word has ell conflict\n");
    return FALSE;
  }

  if (flags.bit (CHECK_SPACES) &&
    (strchr(word->best_choice->unichar_string().string(), ' ') != NULL)) {
    if (tessedit_adaption_debug) tprintf("word contains spaces\n");
    return FALSE;
  }

//  if (flags.bit (CHECK_AMBIG_WERD) && test_ambig_word (word))
  if (flags.bit (CHECK_AMBIG_WERD) &&
      !getDict().NoDangerousAmbig(word->best_choice, NULL, false, NULL, NULL)) {
    if (tessedit_adaption_debug) tprintf("word is ambiguous\n");
    return FALSE;
  }

  // Do not adapt to words that are composed from fragments if
  // tessedit_adapt_to_char_fragments is false.
  if (!tessedit_adapt_to_char_fragments) {
    const char *fragment_lengths = word->best_choice->fragment_lengths();
    if (fragment_lengths != NULL && *fragment_lengths != '\0') {
      for (int i = 0; i < word->best_choice->length(); ++i) {
        if (fragment_lengths[i] > 1) {
          if (tessedit_adaption_debug) tprintf("won't adapt to fragments\n");
          return false;  // found a character composed from fragments
        }
      }
    }
  }

  if (tessedit_adaption_debug) {
    tprintf("returning status %d\n", status);
  }
  return status;

}


void Tesseract::collect_ems_for_adaption(WERD_RES *word,
                                         CHAR_SAMPLES_LIST *char_clusters,
                                         CHAR_SAMPLE_LIST *chars_waiting) {
  PBLOB_LIST *blobs = word->outword->blob_list ();
  PBLOB_IT blob_it(blobs);
  inT16 i;
  CHAR_SAMPLE *sample;
  PIXROW_LIST *pixrow_list;
  PIXROW_IT pixrow_it;
  IMAGELINE *imlines;            // lines of the image
  TBOX pix_box;                   // box of imlines
  // extent
  WERD copy_outword;             // copy to denorm
  PBLOB_IT copy_blob_it;
  OUTLINE_IT copy_outline_it;
  inT32 resolution = page_image.get_res ();

  if (tessedit_reject_ems || tessedit_reject_suspect_ems)
    return;                      // Do nothing

  if (word->word->bounding_box ().height () > resolution / 3)
    return;

  if (tessedit_demo_adaption)
                                 // Make sure not set
    tessedit_display_mm.set_value (FALSE);

  if (word_adaptable (word, tessedit_em_adaption_mode)
    && word->reject_map.reject_count () == 0
    && (strchr (word->best_choice->unichar_string().string (), 'm') != NULL
    || (tessedit_process_rns
    && strstr (word->best_choice->unichar_string().string (),
  "rn") != NULL))) {
    if (tessedit_process_rns
    && strstr (word->best_choice->unichar_string().string (), "rn") != NULL) {
      copy_outword = *(word->outword);
      copy_blob_it.set_to_list (copy_outword.blob_list ());
      i = 0;
      while (word->best_choice->unichar_string()[i] != '\0') {
        if (word->best_choice->unichar_string()[i] == 'r'
        && word->best_choice->unichar_string()[i + 1] == 'n') {
          copy_outline_it.set_to_list (copy_blob_it.data ()->
            out_list ());
          copy_outline_it.add_list_after (copy_blob_it.
            data_relative (1)->
            out_list ());
          copy_blob_it.forward ();
          delete (copy_blob_it.extract ());
          i++;
        }
        copy_blob_it.forward ();
        i++;
      }
    }
    else
      copy_outword = *(word->outword);

    copy_outword.baseline_denormalise (&word->denorm);
    char_clip_word(&copy_outword, page_image, pixrow_list, imlines, pix_box);
    pixrow_it.set_to_list (pixrow_list);
    pixrow_it.move_to_first ();

    blob_it.move_to_first ();
    for (i = 0;
      word->best_choice->unichar_string()[i] != '\0';
    i++, pixrow_it.forward (), blob_it.forward ()) {

      if (word->best_choice->unichar_string()[i] == 'm'
        || (word->best_choice->unichar_string()[i] == 'r'
      && word->best_choice->unichar_string()[i + 1] == 'n')) {
        #ifndef SECURE_NAMES
        if (tessedit_cluster_debug)
          tprintf ("Sample %c for adaption found in %s, index %d\n",
            word->best_choice->unichar_string()[i],
            word->best_choice->unichar_string().string (), i);
        #endif
        if (tessedit_matrix_match) {
          sample = clip_sample (pixrow_it.data (),
            imlines,
            pix_box,
            copy_outword.flag (W_INVERSE),
            word->best_choice->unichar_string()[i]);

          if (sample == NULL) {  //Clip failed
            #ifndef SECURE_NAMES
            tprintf ("Unable to clip sample from %s, index %d\n",
              word->best_choice->unichar_string().string (), i);
            #endif
            if (word->best_choice->unichar_string()[i] == 'r')
              i++;

            continue;
          }
        }
        else
          sample = new CHAR_SAMPLE (blob_it.data (),
            &word->denorm,
            word->best_choice->unichar_string()[i]);

        cluster_sample(sample, char_clusters, chars_waiting);

        if (word->best_choice->unichar_string()[i] == 'r')
          i++;                   // Skip next character
      }
    }
    delete[]imlines;             // Free array of imlines
    delete pixrow_list;
  }
}


void Tesseract::collect_characters_for_adaption(
    WERD_RES *word,
    CHAR_SAMPLES_LIST *char_clusters,
    CHAR_SAMPLE_LIST *chars_waiting) {
  PBLOB_LIST *blobs = word->outword->blob_list ();
  PBLOB_IT blob_it(blobs);
  inT16 i;
  CHAR_SAMPLE *sample;
  PIXROW_LIST *pixrow_list;
  PIXROW_IT pixrow_it;
  IMAGELINE *imlines;            // lines of the image
  TBOX pix_box;                   // box of imlines
  // extent
  WERD copy_outword;             // copy to denorm
  inT32 resolution = page_image.get_res ();

  if (word->word->bounding_box ().height () > resolution / 3)
    return;

  if (tessedit_demo_adaption)
                                 // Make sure not set
    tessedit_display_mm.set_value (FALSE);

  if ((word_adaptable (word, tessedit_cluster_adaption_mode)
  && word->reject_map.reject_count () == 0) || tessedit_mm_use_rejmap) {
    if (tessedit_test_cluster_input && !tessedit_mm_use_rejmap)
      return;                    // Reject map set to acceptable
    /* Collect information about good matches */
    copy_outword = *(word->outword);
    copy_outword.baseline_denormalise (&word->denorm);
    char_clip_word(&copy_outword, page_image, pixrow_list, imlines, pix_box);
    pixrow_it.set_to_list (pixrow_list);
    pixrow_it.move_to_first ();

    blob_it.move_to_first ();
    for (i = 0;
      word->best_choice->unichar_string()[i] != '\0';
    i++, pixrow_it.forward (), blob_it.forward ()) {

      if (!(tessedit_mm_use_non_adaption_set
        && STRING(tessedit_non_adaption_set).contains(
            word->best_choice->unichar_string()[i]))
      || (tessedit_mm_use_rejmap && word->reject_map[i].accepted ())) {
        #ifndef SECURE_NAMES
        if (tessedit_cluster_debug)
          tprintf ("Sample %c for adaption found in %s, index %d\n",
            word->best_choice->unichar_string()[i],
            word->best_choice->unichar_string().string (), i);
        #endif
        sample = clip_sample (pixrow_it.data (),
          imlines,
          pix_box,
          copy_outword.flag (W_INVERSE),
          word->best_choice->unichar_string()[i]);

        if (sample == NULL) {    //Clip failed
          #ifndef SECURE_NAMES
          tprintf ("Unable to clip sample from %s, index %d\n",
            word->best_choice->unichar_string().string (), i);
          #endif
          continue;
        }
        cluster_sample(sample, char_clusters, chars_waiting);
      }
    }
    delete[]imlines;             // Free array of imlines
    delete pixrow_list;
  }
  else if (tessedit_test_cluster_input && !tessedit_mm_use_rejmap)
    // Set word to all rejects
    word->reject_map.rej_word_tess_failure ();

}


void Tesseract::cluster_sample(CHAR_SAMPLE *sample,
                               CHAR_SAMPLES_LIST *char_clusters,
                               CHAR_SAMPLE_LIST *chars_waiting) {
  CHAR_SAMPLES *best_cluster = NULL;
  CHAR_SAMPLES_IT c_it = char_clusters;
  CHAR_SAMPLE_IT cw_it = chars_waiting;
  float score;
  float best_score = MAX_INT32;

  if (c_it.empty ())
    c_it.add_to_end (new CHAR_SAMPLES (sample));
  else {
    for (c_it.mark_cycle_pt (); !c_it.cycled_list (); c_it.forward ()) {
      score = c_it.data ()->match_score (sample, this);
      if (score < best_score) {
        best_score = score;
        best_cluster = c_it.data ();
      }
    }

    if (tessedit_cluster_debug)
      tprintf ("Sample's best score %f\n", best_score);

    if (best_score < tessedit_cluster_t1) {
      if (best_score > tessedit_cluster_t3 || tessedit_mm_use_prototypes) {
        best_cluster->add_sample (sample, this);
        check_wait_list(chars_waiting, sample, best_cluster);
        #ifndef SECURE_NAMES
        if (tessedit_cluster_debug)
          tprintf ("Sample added to an existing cluster\n");
        #endif
      }
      else {
        #ifndef SECURE_NAMES
        if (tessedit_cluster_debug)
          tprintf
            ("Sample dropped, good match to an existing cluster\n");
        #endif
      }
    }
    else if (best_score > tessedit_cluster_t2) {
      c_it.add_to_end (new CHAR_SAMPLES (sample));
      #ifndef SECURE_NAMES
      if (tessedit_cluster_debug)
        tprintf ("New cluster created for this sample\n");
      #endif
    }
    else {
      cw_it.add_to_end (sample);
      if (tessedit_cluster_debug)
        tprintf ("Sample added to the wait list\n");
    }
  }
}

void Tesseract::check_wait_list(CHAR_SAMPLE_LIST *chars_waiting,
                                CHAR_SAMPLE *sample,
                                CHAR_SAMPLES *best_cluster) {
  CHAR_SAMPLE *wait_sample;
  CHAR_SAMPLE *test_sample = sample;
  CHAR_SAMPLE_IT cw_it = chars_waiting;
  CHAR_SAMPLE_LIST add_list;     //Samples added to best cluster
  CHAR_SAMPLE_IT add_it = &add_list;
  float score;

  add_list.clear ();

  if (!cw_it.empty ()) {
    do {
      if (!add_list.empty ()) {
        add_it.forward ();
        test_sample = add_it.extract ();
        best_cluster->add_sample (test_sample, this);
      }

      for (cw_it.mark_cycle_pt ();
      !cw_it.cycled_list (); cw_it.forward ()) {
        wait_sample = cw_it.data ();
        if (tessedit_mm_use_prototypes)
          score = best_cluster->match_score (wait_sample, this);
        else
          score = sample->match_sample (wait_sample, FALSE, this);
        if (score < tessedit_cluster_t1) {
          if (score > tessedit_cluster_t3
          || tessedit_mm_use_prototypes) {
            add_it.add_after_stay_put (cw_it.extract ());
            #ifndef SECURE_NAMES
            if (tessedit_cluster_debug)
              tprintf
                ("Wait sample added to an existing cluster\n");
            #endif
          }
          else {
            #ifndef SECURE_NAMES
            if (tessedit_cluster_debug)
              tprintf
                ("Wait sample dropped, good match to an existing cluster\n");
            #endif
          }
        }
      }
    }
    while (!add_list.empty ());
  }
}


void Tesseract::complete_clustering(CHAR_SAMPLES_LIST *char_clusters,
                                    CHAR_SAMPLE_LIST *chars_waiting) {
  CHAR_SAMPLES *best_cluster;
  CHAR_SAMPLES_IT c_it = char_clusters;
  CHAR_SAMPLE_IT cw_it = chars_waiting;
  CHAR_SAMPLE *sample;
  inT32 total_sample_count = 0;

  while (!cw_it.empty ()) {
    cw_it.move_to_first ();
    sample = cw_it.extract ();
    best_cluster = new CHAR_SAMPLES (sample);
    c_it.add_to_end (best_cluster);
    check_wait_list(chars_waiting, sample, best_cluster);
  }

  for (c_it.mark_cycle_pt (); !c_it.cycled_list (); c_it.forward ()) {
    c_it.data ()->assign_to_char ();
    if (tessedit_use_best_sample)
      c_it.data ()->find_best_sample ();
    else if (tessedit_mm_adapt_using_prototypes)
      c_it.data ()->build_prototype ();

    if (tessedit_cluster_debug)
      total_sample_count += c_it.data ()->n_samples ();
  }
  #ifndef SECURE_NAMES
  if (tessedit_cluster_debug)
    tprintf ("Clustering completed, %d samples in all\n", total_sample_count);
  #endif

#ifndef GRAPHICS_DISABLED
  if (tessedit_demo_adaption)
    display_cluster_prototypes(char_clusters);
#endif

}

void Tesseract::adapt_to_good_ems(WERD_RES *word,
                                  CHAR_SAMPLES_LIST *char_clusters,
                                  CHAR_SAMPLE_LIST *chars_waiting) {
  PBLOB_LIST *blobs = word->outword->blob_list ();
  PBLOB_IT blob_it(blobs);
  inT16 i;
  CHAR_SAMPLE *sample;
  CHAR_SAMPLES_IT c_it = char_clusters;
  CHAR_SAMPLE_IT cw_it = chars_waiting;
  float score;
  float best_score;
  char best_char;
  CHAR_SAMPLES *best_cluster;
  PIXROW_LIST *pixrow_list;
  PIXROW_IT pixrow_it;
  IMAGELINE *imlines;            // lines of the image
  TBOX pix_box;                   // box of imlines
  // extent
  WERD copy_outword;             // copy to denorm
  TBOX b_box;
  PBLOB_IT copy_blob_it;
  OUTLINE_IT copy_outline_it;
  PIXROW *pixrow = NULL;

  static inT32 word_number = 0;

#ifndef GRAPHICS_DISABLED
  ScrollView* demo_win = NULL;
#endif

  inT32 resolution = page_image.get_res ();

  if (word->word->bounding_box ().height () > resolution / 3)
    return;

  word_number++;

  if (strchr (word->best_choice->unichar_string().string (), 'm') == NULL
    && (tessedit_process_rns
    && strstr (word->best_choice->unichar_string().string (), "rn") == NULL))
    return;

  if (tessedit_reject_ems)
    reject_all_ems(word);
  else if (tessedit_reject_suspect_ems)
    reject_suspect_ems(word);
  else {
    if (char_clusters->length () == 0) {
      #ifndef SECURE_NAMES
      if (tessedit_cluster_debug)
        tprintf ("No clusters to use for em adaption\n");
      #endif
      return;
    }

    if (!cw_it.empty ()) {
      complete_clustering(char_clusters, chars_waiting);
      print_em_stats(char_clusters, chars_waiting);
    }

    if ((!word_adaptable (word, tessedit_em_adaption_mode) ||
      word->reject_map.reject_count () != 0)
      && (strchr (word->best_choice->unichar_string().string (), 'm') != NULL
      || (tessedit_process_rns
      && strstr (word->best_choice->unichar_string().string (),
    "rn") != NULL))) {
      if (tessedit_process_rns
        && strstr (word->best_choice->unichar_string().string (),
      "rn") != NULL) {
        copy_outword = *(word->outword);
        copy_blob_it.set_to_list (copy_outword.blob_list ());
        i = 0;
        while (word->best_choice->unichar_string()[i] != '\0') {
          if (word->best_choice->unichar_string()[i] == 'r'
          && word->best_choice->unichar_string()[i + 1] == 'n') {
            copy_outline_it.set_to_list (copy_blob_it.data ()->
              out_list ());
            copy_outline_it.add_list_after (copy_blob_it.
              data_relative (1)->
              out_list ());
            copy_blob_it.forward ();
            delete (copy_blob_it.extract ());
            i++;
          }
          copy_blob_it.forward ();
          i++;
        }
      }
      else
        copy_outword = *(word->outword);

      copy_outword.baseline_denormalise (&word->denorm);
      copy_blob_it.set_to_list (copy_outword.blob_list ());
      char_clip_word(&copy_outword, page_image, pixrow_list, imlines, pix_box);
      pixrow_it.set_to_list (pixrow_list);
      pixrow_it.move_to_first ();

                                 // For debugging only
      b_box = copy_outword.bounding_box ();
      pixrow = pixrow_it.data ();

      blob_it.move_to_first ();
      copy_blob_it.move_to_first ();
      for (i = 0;
        word->best_choice->unichar_string()[i] != '\0';
        i++, pixrow_it.forward (), blob_it.forward (),
      copy_blob_it.forward ()) {
        if ((word->best_choice->unichar_string()[i] == 'm'
          || (word->best_choice->unichar_string()[i] == 'r'
          && word->best_choice->unichar_string()[i + 1] == 'n'))
        && !word->reject_map[i].perm_rejected ()) {
          if (tessedit_cluster_debug)
            tprintf ("Sample %c to check found in %s, index %d\n",
              word->best_choice->unichar_string()[i],
              word->best_choice->unichar_string().string (), i);

          if (tessedit_demo_adaption)
            tprintf
              ("Sample %c to check found in %s (%d), index %d\n",
              word->best_choice->unichar_string()[i],
              word->best_choice->unichar_string().string (), word_number,
              i);

          if (tessedit_matrix_match) {
            TBOX copy_box = copy_blob_it.data ()->bounding_box ();

            sample = clip_sample (pixrow_it.data (),
              imlines,
              pix_box,
              copy_outword.flag (W_INVERSE),
              word->best_choice->unichar_string()[i]);

                                 //Clip failed
            if (sample == NULL) {
              tprintf
                ("Unable to clip sample from %s, index %d\n",
                word->best_choice->unichar_string().string (), i);
              #ifndef SECURE_NAMES
              if (tessedit_cluster_debug)
                tprintf ("Sample rejected (no sample)\n");
              #endif
              word->reject_map[i].setrej_mm_reject ();
              if (word->best_choice->unichar_string()[i] == 'r') {
                word->reject_map[i + 1].setrej_mm_reject ();
                i++;
              }
              continue;
            }
          }
          else
            sample = new CHAR_SAMPLE(blob_it.data(),
                                     &word->denorm,
                                     word->best_choice->unichar_string()[i]);

          best_score = MAX_INT32;
          best_char = '\0';
          best_cluster = NULL;

          for (c_it.mark_cycle_pt ();
          !c_it.cycled_list (); c_it.forward ()) {
            if (c_it.data ()->character () != '\0') {
              score = c_it.data ()->match_score (sample, this);
              if (score < best_score) {
                best_cluster = c_it.data ();
                best_score = score;
                best_char = c_it.data ()->character ();
              }
            }
          }

          if (best_score > tessedit_cluster_t1) {
            #ifndef SECURE_NAMES
            if (tessedit_cluster_debug)
              tprintf ("Sample rejected (score %f)\n", best_score);
            if (tessedit_demo_adaption)
              tprintf ("Sample rejected (score %f)\n", best_score);
            #endif
            word->reject_map[i].setrej_mm_reject ();
            if (word->best_choice->unichar_string()[i] == 'r')
              word->reject_map[i + 1].setrej_mm_reject ();
          }
          else {
            if (word->best_choice->unichar_string()[i] == best_char) {
              #ifndef SECURE_NAMES
              if (tessedit_cluster_debug)
                tprintf ("Sample accepted (score %f)\n",
                  best_score);
              if (tessedit_demo_adaption)
                tprintf ("Sample accepted (score %f)\n",
                  best_score);
              #endif
              word->reject_map[i].setrej_mm_accept ();
              if (word->best_choice->unichar_string()[i] == 'r')
                word->reject_map[i + 1].setrej_mm_accept ();
            }
            else {
              #ifndef SECURE_NAMES
              if (tessedit_cluster_debug)
                tprintf ("Sample rejected (char %c, score %f)\n",
                  best_char, best_score);
              if (tessedit_demo_adaption)
                tprintf ("Sample rejected (char %c, score %f)\n",
                  best_char, best_score);
              #endif
              word->reject_map[i].setrej_mm_reject ();
              if (word->best_choice->unichar_string()[i] == 'r')
                word->reject_map[i + 1].setrej_mm_reject ();
            }
          }

          if (tessedit_demo_adaption) {
            if (strcmp (imagebasename.string (),
              tessedit_demo_file.string ()) != 0
              || word_number == tessedit_demo_word1
            || word_number == tessedit_demo_word2) {
#ifndef GRAPHICS_DISABLED
              demo_win =
                display_clip_image(&copy_outword,
                                   page_image,
                                   pixrow_list,
                                   pix_box);
#endif
              demo_word = word_number;
              best_cluster->match_score (sample, this);
              demo_word = 0;
            }
          }
          if (word->best_choice->unichar_string()[i] == 'r')
            i++;                 // Skip next character
        }
      }
      delete[]imlines;           // Free array of imlines
      delete pixrow_list;
    }
  }
}



void Tesseract::adapt_to_good_samples(WERD_RES *word,
                                      CHAR_SAMPLES_LIST *char_clusters,
                                      CHAR_SAMPLE_LIST *chars_waiting) {
  PBLOB_LIST *blobs = word->outword->blob_list ();
  PBLOB_IT blob_it(blobs);
  inT16 i;
  CHAR_SAMPLE *sample;
  CHAR_SAMPLES_IT c_it = char_clusters;
  CHAR_SAMPLE_IT cw_it = chars_waiting;
  float score;
  float best_score;
  char best_char;
  CHAR_SAMPLES *best_cluster;
  PIXROW_LIST *pixrow_list;
  PIXROW_IT pixrow_it;
  IMAGELINE *imlines;            // lines of the image
  TBOX pix_box;                   // box of imlines
  // extent
  WERD copy_outword;             // copy to denorm
  TBOX b_box;
  PBLOB_IT copy_blob_it;
  PIXROW *pixrow = NULL;

  static inT32 word_number = 0;

#ifndef GRAPHICS_DISABLED
  ScrollView* demo_win = NULL;
#endif

  inT32 resolution = page_image.get_res ();

  word_number++;

  if (tessedit_test_cluster_input)
    return;

  if (word->word->bounding_box ().height () > resolution / 3)
    return;

  if (char_clusters->length () == 0) {
    #ifndef SECURE_NAMES
    if (tessedit_cluster_debug)
      tprintf ("No clusters to use for adaption\n");
    #endif
    return;
  }

  if (!cw_it.empty ()) {
    complete_clustering(char_clusters, chars_waiting);
    print_em_stats(char_clusters, chars_waiting);
  }

  if ((!word_adaptable (word, tessedit_cluster_adaption_mode)
  && word->reject_map.reject_count () != 0) || tessedit_mm_use_rejmap) {
    if (tessedit_cluster_debug) {
      tprintf ("\nChecking: \"%s\"  MAP ",
        word->best_choice->unichar_string().string ());
      word->reject_map.print (debug_fp);
      tprintf ("\n");
    }

    copy_outword = *(word->outword);
    copy_outword.baseline_denormalise (&word->denorm);
    copy_blob_it.set_to_list (copy_outword.blob_list ());
    char_clip_word(&copy_outword, page_image, pixrow_list, imlines, pix_box);
    pixrow_it.set_to_list (pixrow_list);
    pixrow_it.move_to_first ();

                                 // For debugging only
    b_box = copy_outword.bounding_box ();
    pixrow = pixrow_it.data ();

    blob_it.move_to_first ();
    copy_blob_it.move_to_first ();
    for (i = 0;
      word->best_choice->unichar_string()[i] != '\0';
      i++, pixrow_it.forward (), blob_it.forward (),
    copy_blob_it.forward ()) {
      if (word->reject_map[i].recoverable ()
      || (tessedit_mm_all_rejects && word->reject_map[i].rejected ())) {
        TBOX copy_box = copy_blob_it.data ()->bounding_box ();

        if (tessedit_cluster_debug)
          tprintf ("Sample %c to check found in %s, index %d\n",
            word->best_choice->unichar_string()[i],
            word->best_choice->unichar_string().string (), i);

        if (tessedit_demo_adaption)
          tprintf ("Sample %c to check found in %s (%d), index %d\n",
            word->best_choice->unichar_string()[i],
            word->best_choice->unichar_string().string (),
            word_number, i);

        sample = clip_sample (pixrow_it.data (),
          imlines,
          pix_box,
          copy_outword.flag (W_INVERSE),
          word->best_choice->unichar_string()[i]);

        if (sample == NULL) {    //Clip failed
          tprintf ("Unable to clip sample from %s, index %d\n",
            word->best_choice->unichar_string().string (), i);
          #ifndef SECURE_NAMES
          if (tessedit_cluster_debug)
            tprintf ("Sample rejected (no sample)\n");
          #endif
          word->reject_map[i].setrej_mm_reject ();

          continue;
        }

        best_score = MAX_INT32;
        best_char = '\0';
        best_cluster = NULL;

        for (c_it.mark_cycle_pt ();
        !c_it.cycled_list (); c_it.forward ()) {
          if (c_it.data ()->character () != '\0') {
            score = c_it.data ()->match_score (sample, this);
            if (score < best_score) {
              best_cluster = c_it.data ();
              best_score = score;
              best_char = c_it.data ()->character ();
            }
          }
        }

        if (best_score > tessedit_cluster_t1) {
          #ifndef SECURE_NAMES
          if (tessedit_cluster_debug)
            tprintf ("Sample rejected (score %f)\n", best_score);
          if (tessedit_demo_adaption)
            tprintf ("Sample rejected (score %f)\n", best_score);
          #endif
          word->reject_map[i].setrej_mm_reject ();
        }
        else {
          if (word->best_choice->unichar_string()[i] == best_char) {
            #ifndef SECURE_NAMES
            if (tessedit_cluster_debug)
              tprintf ("Sample accepted (score %f)\n", best_score);
            if (tessedit_demo_adaption)
              tprintf ("Sample accepted (score %f)\n", best_score);
            #endif
            if (tessedit_test_adaption)
              word->reject_map[i].setrej_minimal_rej_accept ();
            else
              word->reject_map[i].setrej_mm_accept ();
          }
          else {
            #ifndef SECURE_NAMES
            if (tessedit_cluster_debug)
              tprintf ("Sample rejected (char %c, score %f)\n",
                best_char, best_score);
            if (tessedit_demo_adaption)
              tprintf ("Sample rejected (char %c, score %f)\n",
                best_char, best_score);
            #endif
            word->reject_map[i].setrej_mm_reject ();
          }
        }

        if (tessedit_demo_adaption) {
          if (strcmp (imagebasename.string (),
            tessedit_demo_file.string ()) != 0
            || word_number == tessedit_demo_word1
          || word_number == tessedit_demo_word2) {
#ifndef GRAPHICS_DISABLED
            demo_win =
              display_clip_image(&copy_outword,
                                 page_image,
                                 pixrow_list,
                                 pix_box);
#endif
            demo_word = word_number;
            best_cluster->match_score (sample, this);
            demo_word = 0;
          }
        }
      }
    }
    delete[]imlines;             // Free array of imlines
    delete pixrow_list;

    if (tessedit_cluster_debug) {
      tprintf ("\nFinal: \"%s\"  MAP ",
        word->best_choice->unichar_string().string ());
      word->reject_map.print (debug_fp);
      tprintf ("\n");
    }
  }
}
}  // namespace tesseract


void print_em_stats(CHAR_SAMPLES_LIST *char_clusters,
                    CHAR_SAMPLE_LIST *chars_waiting) {
  CHAR_SAMPLES_IT c_it = char_clusters;

  if (!tessedit_cluster_debug)
    return;
  #ifndef SECURE_NAMES
  tprintf ("There are %d clusters and %d samples waiting\n",
    char_clusters->length (), chars_waiting->length ());

  for (c_it.mark_cycle_pt (); !c_it.cycled_list (); c_it.forward ())
    c_it.data ()->print (debug_fp);
  #endif
  tprintf ("\n");
}


CHAR_SAMPLE *clip_sample(              //lines of the image
                         PIXROW *pixrow,
                         IMAGELINE *imlines,
                         TBOX pix_box,  //box of imlines extent
                         BOOL8 white_on_black,
                         char c) {
  TBOX b_box = pixrow->bounding_box ();
  float baseline_pos = 0;
  inT32 resolution = page_image.get_res ();

  if (!b_box.null_box ()) {
    ASSERT_HOST (b_box.width () < page_image.get_xsize () &&
      b_box.height () < page_image.get_ysize ());

    if (b_box.width () > resolution || b_box.height () > resolution) {
      tprintf ("clip sample: sample too big (%d x %d)\n",
        b_box.width (), b_box.height ());

      return NULL;
    }

    IMAGE *image = new (IMAGE);
    if (image->create (b_box.width (), b_box.height (), 1) == -1) {
      tprintf ("clip sample: create image failed (%d x %d)\n",
        b_box.width (), b_box.height ());

      delete image;
      return NULL;
    }

    if (!white_on_black)
      invert_image(image);  // Set background to white
    pixrow->char_clip_image (imlines, pix_box, NULL, *image, baseline_pos);
    if (white_on_black)
      invert_image(image);  //invert white on black for scaling &NN
    return new CHAR_SAMPLE (image, c);
  }
  else
    return NULL;
}


#ifndef GRAPHICS_DISABLED
void display_cluster_prototypes(CHAR_SAMPLES_LIST *char_clusters) {
  inT16 proto_number = 0;
  CHAR_SAMPLES_IT c_it = char_clusters;
  char title[WINDOWNAMESIZE];

  for (c_it.mark_cycle_pt (); !c_it.cycled_list (); c_it.forward ()) {
    proto_number++;

    #ifndef SECURE_NAMES
    tprintf ("Displaying proto number %d\n", proto_number);
    #endif

    if (c_it.data ()->prototype () != NULL) {
      sprintf (title, "Proto - %d", proto_number);
      display_image (c_it.data ()->prototype ()->make_image (),
        title, (proto_number - 1) * 400, 0, FALSE);
    }
  }
}
#endif

// *********************************************************************
// Simplistic routines to test the effect of rejecting ems and fullstops
// *********************************************************************

void reject_all_ems(WERD_RES *word) {
  inT16 i;

  for (i = 0; word->best_choice->unichar_string()[i] != '\0'; i++) {
    if (word->best_choice->unichar_string()[i] == 'm')
                                 // reject all ems
      word->reject_map[i].setrej_mm_reject ();
  }
}


void reject_all_fullstops(WERD_RES *word) {
  inT16 i;

  for (i = 0; word->best_choice->unichar_string()[i] != '\0'; i++) {
    if (word->best_choice->unichar_string()[i] == '.')
                                 // reject all fullstops
      word->reject_map[i].setrej_mm_reject ();
  }
}

namespace tesseract {
void Tesseract::reject_suspect_ems(WERD_RES *word) {
  inT16 i;

  if (!word_adaptable (word, tessedit_cluster_adaption_mode))
  for (i = 0; word->best_choice->unichar_string()[i] != '\0'; i++) {
    if (word->best_choice->unichar_string()[i] == 'm' && suspect_em (word, i))
                                 // reject all ems
      word->reject_map[i].setrej_mm_reject ();
  }
}
}  // namespace tesseract


void reject_suspect_fullstops(WERD_RES *word) {
  inT16 i;

  for (i = 0; word->best_choice->unichar_string()[i] != '\0'; i++) {
    if (word->best_choice->unichar_string()[i] == '.'
      && suspect_fullstop (word, i))
                                 // reject all commas
      word->reject_map[i].setrej_mm_reject ();
  }
}


BOOL8 suspect_em(WERD_RES *word, inT16 index) {
  PBLOB_LIST *blobs = word->outword->blob_list ();
  PBLOB_IT blob_it(blobs);
  inT16 j;

  for (j = 0; j < index; j++)
    blob_it.forward ();

  return (blob_it.data ()->out_list ()->length () != 1);
}


BOOL8 suspect_fullstop(WERD_RES *word, inT16 i) {
  float aspect_ratio;
  PBLOB_LIST *blobs = word->outword->blob_list ();
  PBLOB_IT blob_it(blobs);
  inT16 j;
  TBOX box;
  inT16 width;
  inT16 height;

  for (j = 0; j < i; j++)
    blob_it.forward ();

  box = blob_it.data ()->bounding_box ();

  width = box.width ();
  height = box.height ();

  aspect_ratio = ((width > height) ? ((float) width) / height :
  ((float) height) / width);

  return (aspect_ratio > tessed_fullstop_aspect_ratio);
}
