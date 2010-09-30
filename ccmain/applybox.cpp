/**********************************************************************
 * File:        applybox.cpp  (Formerly applybox.c)
 * Description: Re segment rows according to box file data
 * Author:      Phil Cheatle
 * Created:     Wed Nov 24 09:11:23 GMT 1993
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
#include "mfcpch.h"

#ifdef _MSC_VER
#pragma warning(disable:4244)  // Conversion warnings
#endif

#include "applybox.h"
#include <ctype.h>
#include <string.h>
#ifdef __UNIX__
#include <assert.h>
#include <errno.h>
#endif
#include "boxread.h"
#include "control.h"
#include "genblob.h"
#include "globals.h"
#include "fixxht.h"
#include "mainblk.h"
#include "matchdefs.h"
#include "secname.h"
#include "tessbox.h"
#include "unichar.h"
#include "unicharset.h"
#include "matchdefs.h"
#include "tesseractclass.h"

#define SECURE_NAMES
#ifndef SECURE_NAMES
#include          "wordstats.h"
#endif

// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif

#ifdef HAVE_LIBLEPT
// Include leptonica library only if autoconf (or makefile etc) tell us to.
#include "allheaders.h"
#endif

#define EXTERN
EXTERN BOOL_VAR (applybox_rebalance, TRUE, "Drop dead");
EXTERN INT_VAR (applybox_debug, 5, "Debug level");
EXTERN INT_VAR (applybox_page, 0, "Page number to apply boxes from");
EXTERN STRING_VAR (applybox_test_exclusions, "",
                   "Chars ignored for testing");
EXTERN double_VAR (applybox_error_band, 0.15, "Err band as fract of xht");

EXTERN STRING_VAR(exposure_pattern, ".exp",
                  "Exposure value follows this pattern in the image"
                  " filename. The name of the image files are expected"
                  " to be in the form [lang].[fontname].exp[num].tif");

EXTERN BOOL_VAR(learn_chars_and_char_frags_mode, FALSE,
                "Learn both character fragments (as is done in the"
                " special low exposure mode) as well as unfragmented"
                " characters.");

extern IMAGE page_image;

// The unicharset used during box training
static UNICHARSET unicharset_boxes;

static void clear_any_old_text(BLOCK_LIST *block_list);

// Register uch with unicharset_boxes.
static UNICHAR_ID register_char(const char *uch);

static BOOL8 read_next_box(int page,
                           FILE* box_file,
                           TBOX *box,
                           UNICHAR_ID *uch_id);


/*************************************************************************
 * The code re-assigns outlines to form words each with ONE labelled blob.
 * Noise is left in UNLABELLED words. The chars on the page are checked crudely
 * for sensible position relative to baseline and xht. Failed boxes are
 * compensated for by duplicating other believable instances of the character.
 *
 * The box file is assumed to contain box definitions, one per line, of the
 * following format:
 *   <Char> <left> <bottom> <right> <top> <page> ... arbitrary trailing fields unused
 *
 * The approach taken is to search the WHOLE page for stuff overlapping each box.
 *  - This is not too inefficient and is SAFE.
 *    - We can detect overlapping blobs as we will be attempting to put a blob
 *      from a LABELLED word into the current word.
 *    - When all the boxes have been processed we can detect any stuff which is
 *      being ignored - it is the unlabelled words left on the page.
 *
 * A box should only overlap one row.
 *
 * A warning is given if the box is on the same row as the previous box, but NOT
 * on the same row as the previous blob.
 *
 * Any OUTLINE which overlaps the box is put into the new word.
 *
 * ascender chars must ascend above xht significantly
 * xht chars must not rise above row xht significantly
 * bl chars must not descend below baseline significantly
 * descender chars must descend below baseline significantly
 *
 * ?? Certain chars are DROPPED - to limit the training data.
 *
 *************************************************************************/
namespace tesseract {
void Tesseract::apply_boxes(const STRING& fname,
                            BLOCK_LIST *block_list    //real blocks
                           ) {
  inT16 boxfile_lineno = 0;
  inT16 boxfile_charno = 0;
  TBOX box;                       //boxfile box
  UNICHAR_ID uch_id;             //correct ch from boxfile
  ROW *row;
  ROW *prev_row = NULL;
  inT16 prev_box_right = MAX_INT16;
  inT16 block_id;
  inT16 row_id;
  inT16 box_count = 0;
  inT16 box_failures = 0;
  inT16 labels_ok;
  inT16 rows_ok;
  inT16 bad_blobs;
  inT16 *tgt_char_counts = NULL; // No. of box samples
  inT16 i;
  inT16 rebalance_count = 0;
  UNICHAR_ID min_uch_id = INVALID_UNICHAR_ID;
  inT16 min_samples;
  inT16 final_labelled_blob_count;
  bool low_exposure = false;

  // Clean the unichar set
  unicharset_boxes.clear();
  // Space character needed to represent NIL classification
  unicharset_boxes.unichar_insert(" ");

  // Figure out whether this image file's exposure is less than 1, in which
  // case when learning we will only pay attention to character fragments.
  const char *ptr = strstr(imagefile.string(), exposure_pattern.string());
  if (ptr != NULL &&
      strtol(ptr += strlen(exposure_pattern.string()), NULL, 10) < 0) {
    low_exposure = true;
  }

  FILE* box_file;
  STRING filename = fname;
  const char *lastdot;           //of name

  lastdot = strrchr (filename.string (), '.');
  if (lastdot != NULL)
    filename[lastdot - filename.string()] = '\0';

  filename += ".box";
  if (!(box_file = fopen (filename.string(), "r"))) {
    CANTOPENFILE.error ("read_next_box", EXIT,
      "Cant open box file %s %d",
      filename.string(), errno);
  }

  tgt_char_counts = new inT16[MAX_NUM_CLASSES];
  for (i = 0; i < MAX_NUM_CLASSES; i++)
    tgt_char_counts[i] = 0;

  clear_any_old_text(block_list);
  while (read_next_box(applybox_page, box_file, &box, &uch_id)) {
    box_count++;
    if (!low_exposure || learn_chars_and_char_frags_mode) {
      tgt_char_counts[uch_id]++;
    }
    row = find_row_of_box (block_list, box, block_id, row_id);
    if (box.left () < prev_box_right) {
      boxfile_lineno++;
      boxfile_charno = 1;
    }
    else
      boxfile_charno++;

    if (row == NULL) {
      box_failures++;
      report_failed_box (boxfile_lineno, boxfile_charno, box,
                         unicharset_boxes.id_to_unichar(uch_id),
        "FAILURE! box overlaps no blobs or blobs in multiple rows");
    }
    else {
      if ((box.left () >= prev_box_right) && (row != prev_row))
        report_failed_box (boxfile_lineno, boxfile_charno, box,
                           unicharset_boxes.id_to_unichar(uch_id),
          "WARNING! false row break");
      box_failures += resegment_box (row, box, uch_id, block_id, row_id,
        boxfile_lineno, boxfile_charno, tgt_char_counts, low_exposure, true);
      prev_row = row;
    }
    prev_box_right = box.right ();
  }
  tidy_up(block_list,
          labels_ok,
          rows_ok,
          bad_blobs,
          tgt_char_counts,
          rebalance_count,
          &min_uch_id,
          min_samples,
          final_labelled_blob_count,
          low_exposure,
          true);
  tprintf ("APPLY_BOXES:\n");
  tprintf ("   Boxes read from boxfile:  %6d\n", box_count);
  tprintf ("   Initially labelled blobs: %6d in %d rows\n",
    labels_ok, rows_ok);
  tprintf ("   Box failures detected:       %6d\n", box_failures);
  tprintf ("   Duped blobs for rebalance:%6d\n", rebalance_count);
  tprintf ("   \"%s\" has fewest samples:%6d\n",
           unicharset_boxes.id_to_unichar(min_uch_id), min_samples);
  tprintf ("                Total unlabelled words:   %6d\n",
    bad_blobs);
  tprintf ("                Final labelled words:     %6d\n",
    final_labelled_blob_count);

  // Clean up.
  delete[] tgt_char_counts;
}

int Tesseract::Boxes2BlockList(int box_cnt, TBOX *boxes,
                               BLOCK_LIST *block_list,
                               bool right2left) {
  inT16 boxfile_lineno = 0;
  inT16 boxfile_charno = 0;
  TBOX box;
  ROW *row;
  ROW *prev_row = NULL;
  inT16 prev_box_right = MAX_INT16;
  inT16 prev_box_left = 0;
  inT16 block_id;
  inT16 row_id;
  inT16 box_failures = 0;
  inT16 labels_ok;
  inT16 rows_ok;
  inT16 bad_blobs;
  inT16 rebalance_count = 0;
  UNICHAR_ID min_uch_id;
  inT16 min_samples;
  inT16 final_labelled_blob_count;

  clear_any_old_text(block_list);
  for (int box_idx = 0; box_idx < box_cnt; box_idx++) {
    box = boxes[box_idx];

    row = find_row_of_box(block_list, box, block_id, row_id);
    // check for a new row
    if ((right2left && box.right () > prev_box_left) ||
        (!right2left && box.left () < prev_box_right)) {
      boxfile_lineno++;
      boxfile_charno = 1;
    }
    else {
      boxfile_charno++;
    }

    if (row == NULL) {
      box_failures++;
    }
    else {
      box_failures += resegment_box(row, box, 0, block_id, row_id,
                                    boxfile_lineno, boxfile_charno,
                                    NULL, false, false);
      prev_row = row;
    }
    prev_box_right = box.right ();
    prev_box_left = box.left ();
  }

  tidy_up(block_list, labels_ok, rows_ok, bad_blobs, NULL,
          rebalance_count, &min_uch_id, min_samples, final_labelled_blob_count,
          false, false);

  return box_failures;
}

}  // namespace tesseract


static
void clear_any_old_text(                        //remove correct text
                        BLOCK_LIST *block_list  //real blocks
                       ) {
  BLOCK_IT block_it(block_list);
  ROW_IT row_it;
  WERD_IT word_it;

  for (block_it.mark_cycle_pt ();
  !block_it.cycled_list (); block_it.forward ()) {
    row_it.set_to_list (block_it.data ()->row_list ());
    for (row_it.mark_cycle_pt (); !row_it.cycled_list (); row_it.forward ()) {
      word_it.set_to_list (row_it.data ()->word_list ());
      for (word_it.mark_cycle_pt ();
      !word_it.cycled_list (); word_it.forward ()) {
        word_it.data ()->set_text ("");
      }
    }
  }
}

static
UNICHAR_ID register_char(const char *uch) {
  if (!unicharset_boxes.contains_unichar(uch)) {
    unicharset_boxes.unichar_insert(uch);
    if (unicharset_boxes.size() > MAX_NUM_CLASSES) {
      tprintf("Error: Size of unicharset of boxes is "
              "greater than MAX_NUM_CLASSES (%d)\n", MAX_NUM_CLASSES);
      exit(1);
    }
  }
  return unicharset_boxes.unichar_to_id(uch);
}

static
BOOL8 read_next_box(int page,
                    FILE* box_file,
                    TBOX *box,
                    UNICHAR_ID *uch_id) {
  int x_min;
  int y_min;
  int x_max;
  int y_max;
  char uch[kBoxReadBufSize];

  if (read_next_box(page, box_file, uch, &x_min, &y_min, &x_max, &y_max)) {
    *uch_id = register_char(uch);
    *box = TBOX (ICOORD (x_min, y_min), ICOORD (x_max, y_max));
    return TRUE;  // read a box ok
  } else {
    return FALSE;  // EOF
  }
}


ROW *find_row_of_box(                         //
                     BLOCK_LIST *block_list,  //real blocks
                     const TBOX &box,                 //from boxfile
                     inT16 &block_id,
                     inT16 &row_id_to_process) {
  BLOCK_IT block_it(block_list);
  BLOCK *block;
  ROW_IT row_it;
  ROW *row;
  ROW *row_to_process = NULL;
  inT16 row_id;
  WERD_IT word_it;
  WERD *word;
  BOOL8 polyg;
  PBLOB_IT blob_it;
  PBLOB *blob;
  OUTLINE_IT outline_it;
  OUTLINE *outline;

  /*
    Find row to process - error if box REALLY overlaps more than one row. (I.e
    it overlaps blobs in the row - not just overlaps the bounding box of the
    whole row.)
  */

  block_id = 0;
  for (block_it.mark_cycle_pt ();
  !block_it.cycled_list (); block_it.forward ()) {
    block_id++;
    row_id = 0;
    block = block_it.data ();
    if (block->bounding_box ().overlap (box)) {
      row_it.set_to_list (block->row_list ());
      for (row_it.mark_cycle_pt ();
      !row_it.cycled_list (); row_it.forward ()) {
        row_id++;
        row = row_it.data ();
        if (row->bounding_box ().overlap (box)) {
          word_it.set_to_list (row->word_list ());
          for (word_it.mark_cycle_pt ();
          !word_it.cycled_list (); word_it.forward ()) {
            word = word_it.data ();
            polyg = word->flag (W_POLYGON);
            if (word->bounding_box ().overlap (box)) {
              blob_it.set_to_list (word->gblob_list ());
              for (blob_it.mark_cycle_pt ();
              !blob_it.cycled_list (); blob_it.forward ()) {
                blob = blob_it.data ();
                if (gblob_bounding_box (blob, polyg).
                overlap (box)) {
                  outline_it.
                    set_to_list (gblob_out_list
                    (blob, polyg));
                  for (outline_it.mark_cycle_pt ();
                    !outline_it.cycled_list ();
                  outline_it.forward ()) {
                    outline = outline_it.data ();
                    if (goutline_bounding_box
                    (outline, polyg).major_overlap (box)) {
                      if ((row_to_process == NULL) ||
                      (row_to_process == row)) {
                        row_to_process = row;
                        row_id_to_process = row_id;
                      }
                      else
                        /* RETURN ERROR Box overlaps blobs in more than one row  */
                        return NULL;
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }
  return row_to_process;
}


inT16 resegment_box(  //
                    ROW *row,
                    TBOX &box,
                    UNICHAR_ID uch_id,
                    inT16 block_id,
                    inT16 row_id,
                    inT16 boxfile_lineno,
                    inT16 boxfile_charno,
                    inT16 *tgt_char_counts,
                    bool learn_char_fragments,
                    bool learning) {
  WERD_LIST new_word_list;
  WERD_IT word_it;
  WERD_IT new_word_it(&new_word_list);
  WERD *word = NULL;
  WERD *new_word = NULL;
  BOOL8 polyg = false;
  PBLOB_IT blob_it;
  PBLOB_IT new_blob_it;
  PBLOB *blob;
  PBLOB *new_blob;
  OUTLINE_IT outline_it;
  OUTLINE_LIST dummy;  // Just to initialize new_outline_it.
  OUTLINE_IT new_outline_it = &dummy;
  OUTLINE *outline;
  TBOX new_word_box;
  TBOX curr_outline_box;
  TBOX prev_outline_box;
  float word_x_centre;
  float baseline;
  inT16 error_count = 0;         //number of chars lost
  STRING label;
  UNICHAR_ID fragment_uch_id;
  int fragment_index;
  int new_word_it_len;

  if (learning && applybox_debug > 6) {
    tprintf("\nAPPLY_BOX: in resegment_box() for %s(%d)\n",
            unicharset_boxes.id_to_unichar(uch_id), uch_id);
  }
  word_it.set_to_list (row->word_list ());
  for (word_it.mark_cycle_pt (); !word_it.cycled_list (); word_it.forward ()) {
    word = word_it.data ();
    polyg = word->flag (W_POLYGON);
    if (word->bounding_box ().overlap (box)) {
      blob_it.set_to_list (word->gblob_list ());
      prev_outline_box = TBOX();  // clear prev_outline_box
      curr_outline_box = TBOX();  // clear curr_outline_box
      for (blob_it.mark_cycle_pt ();
      !blob_it.cycled_list (); blob_it.forward ()) {
        blob = blob_it.data ();
        if (gblob_bounding_box (blob, polyg).overlap (box)) {
          outline_it.set_to_list (gblob_out_list (blob, polyg));
          for (outline_it.mark_cycle_pt ();
          !outline_it.cycled_list (); outline_it.forward ()) {
            outline = outline_it.data ();
            prev_outline_box += curr_outline_box;
            curr_outline_box = goutline_bounding_box(outline, polyg);
            if (curr_outline_box.major_overlap (box)) {
              if (strlen (word->text ()) > 0) {
                if (error_count == 0) {
                  error_count = 1;
                  if (learning && applybox_debug > 4)
                    report_failed_box (boxfile_lineno,
                      boxfile_charno,
                      box, unicharset_boxes.id_to_unichar(uch_id),
                      "FAILURE! box overlaps blob in labelled word");
                }
                if (learning && applybox_debug > 4)
                  tprintf ("APPLY_BOXES: ALSO ignoring corrupted char"
                           " blk:%d row:%d \"%s\"\n",
                           block_id, row_id, word_it.data()->text());
                word_it.data ()->set_text ("");  // UN label it
                error_count++;
              }
              // Do not learn from fragments of characters that are broken
              // into very small pieces to avoid picking up noise.
              if ((learn_char_fragments || learn_chars_and_char_frags_mode) &&
                  ((C_OUTLINE *)outline)->area() < kMinFragmentOutlineArea) {
                if (applybox_debug > 6) {
                  tprintf("APPLY_BOX: fragment outline area %d is too small"
                          " - not recording fragments of this character\n",
                          ((C_OUTLINE *)outline)->area());
                }
                error_count++;
              }

              if (error_count == 0) {
                if (applybox_debug > 6 ) {
                  tprintf("APPLY_BOX: Previous ");
                  prev_outline_box.print();
                  tprintf("APPLY_BOX: Current area: %d ",
                          ((C_OUTLINE *)outline)->area());
                  curr_outline_box.print();
                }
                // When learning character fragments is enabled, we put
                // outlines that do not overlap on x axis in separate WERDs.
                bool start_new_word =
                    (learn_char_fragments || learn_chars_and_char_frags_mode) &&
                    !curr_outline_box.major_x_overlap(prev_outline_box);
                if (new_word == NULL || start_new_word) {
                  if (new_word != NULL) {  // add prev new_word to new_word_list
                    new_word_it.add_to_end(new_word);
                  }
                  // Make a new word with a single blob.
                  new_word = word->shallow_copy();
                  new_word->set_flag(W_FUZZY_NON, false);
                  new_word->set_flag(W_FUZZY_SP, false);
                  if (polyg){
                    new_blob = new PBLOB;
                  } else {
                    new_blob = (PBLOB *) new C_BLOB;
                  }
                  new_blob_it.set_to_list(new_word->gblob_list());
                  new_blob_it.add_to_end(new_blob);
                  new_outline_it.set_to_list(
                      gblob_out_list(new_blob, polyg));
                }
                new_outline_it.add_to_end(outline_it.extract());  // move blob
              }
            }
          }
          if (outline_it.empty())      // no outlines in blob
            delete blob_it.extract();  // so delete blob
        }
      }
      if (blob_it.empty())         // no blobs in word
        delete word_it.extract();  // so delete word
    }
  }
  if (new_word != NULL) {  // add prev new_word to new_word_list
    new_word_it.add_to_end(new_word);
  }
  new_word_it_len = new_word_it.length();

  // Check for failures.
  if (error_count > 0)
    return error_count;
  if (learning && new_word_it_len <= 0) {
    report_failed_box(boxfile_lineno, boxfile_charno, box,
                      unicharset_boxes.id_to_unichar(uch_id),
                      "FAILURE! Couldn't find any blobs");
    return 1;  // failure
  }

  if (learning && new_word_it_len > CHAR_FRAGMENT::kMaxChunks) {
    tprintf("APPLY_BOXES: too many fragments (%d) for char %s\n",
            new_word_it_len, unicharset_boxes.id_to_unichar(uch_id));
    return 1;  // failure
  }

  // Add labelled character or character fragments to the word list.
  fragment_index = 0;
  new_word_it.move_to_first();
  for (new_word_it.mark_cycle_pt(); !new_word_it.cycled_list();
       new_word_it.forward()) {
    new_word = new_word_it.extract();
    if (new_word_it_len > 1) {  // deal with a fragment
      if (learning) {
        label = CHAR_FRAGMENT::to_string(unicharset_boxes.id_to_unichar(uch_id),
                                         fragment_index, new_word_it_len);
        fragment_uch_id = register_char(label.string());
        new_word->set_text(label.string());
        ++fragment_index;
        // For now we cheat by setting the expected number of char fragments
        // to the number of char fragments actually parsed and labelled.
        // TODO(daria): find out whether this can be improved.
        tgt_char_counts[fragment_uch_id]++;
      } else {
        // No learning involved, Just stick a place-holder string
        new_word->set_text("*");
      }
      if (applybox_debug > 5) {
        tprintf("APPLY_BOX: adding char fragment %s\n", label.string());
      }
    } else {  // deal with a regular character
      if (learning) {
        if (!learn_char_fragments || learn_chars_and_char_frags_mode) {
          new_word->set_text(unicharset_boxes.id_to_unichar(uch_id));
        } else {
          // not interested in non-fragmented chars if learning fragments, so
          // unlabel it.
          new_word->set_text("");
        }
      } else {
        // No learning involved here. Just stick a place holder string
        new_word->set_text("*");
      }
    }
    gblob_sort_list(new_word->gblob_list(), polyg);
    word_it.add_to_end(new_word);
    new_word_box = new_word->bounding_box();
    word_x_centre = (new_word_box.left() + new_word_box.right()) / 2.0f;
    baseline = row->base_line(word_x_centre);
  }

  // All done. Now check if the EOL, BOL flags are set correctly.
  word_it.move_to_first();
  for (word_it.mark_cycle_pt(); !word_it.cycled_list(); word_it.forward()) {
    word = word_it.data();
    word->set_flag(W_BOL, false);
    word->set_flag(W_EOL, false);
  }
  word->set_flag(W_EOL, true);
  word_it.move_to_first();
  word_it.data()->set_flag(W_BOL, true);
  return 0;  //success

#if 0
    if (strlen(unicharset_boxes.id_to_unichar(uch_id)) == 1) {
      if (STRING (chs_caps_ht).contains (unicharset_boxes.id_to_unichar(uch_id)[0]) &&
          (new_word_box.top () <
           baseline + (1 + applybox_error_band) * row->x_height ())) {
        report_failed_box (boxfile_lineno, boxfile_charno, box,
                           unicharset_boxes.id_to_unichar(uch_id),
                           "FAILURE! caps-ht char didn't ascend");
        new_word->set_text ("");
        return 1;
      }
      if (STRING (chs_odd_top).contains (unicharset_boxes.id_to_unichar(uch_id)[0]) &&
          (new_word_box.top () <
           baseline + (1 - applybox_error_band) * row->x_height ())) {
        report_failed_box (boxfile_lineno, boxfile_charno, box,
                           unicharset_boxes.id_to_unichar(uch_id),
                           "FAILURE! Odd top char below xht");
        new_word->set_text ("");
        return 1;
      }
      if (STRING (chs_x_ht).contains (unicharset_boxes.id_to_unichar(uch_id)[0]) &&
          ((new_word_box.top () >
            baseline + (1 + applybox_error_band) * row->x_height ()) ||
           (new_word_box.top () <
            baseline + (1 - applybox_error_band) * row->x_height ()))) {
        report_failed_box (boxfile_lineno, boxfile_charno, box,
                           unicharset_boxes.id_to_unichar(uch_id),
                           "FAILURE! x-ht char didn't have top near xht");
        new_word->set_text ("");
        return 1;
      }
      if (STRING (chs_non_ambig_bl).contains
          (unicharset_boxes.id_to_unichar(uch_id)[0]) &&
          ((new_word_box.bottom () <
            baseline - applybox_error_band * row->x_height ()) ||
           (new_word_box.bottom () >
            baseline + applybox_error_band * row->x_height ()))) {
        report_failed_box (boxfile_lineno, boxfile_charno, box,
                           unicharset_boxes.id_to_unichar(uch_id),
                           "FAILURE! non ambig BL char didnt have bottom near baseline");
        new_word->set_text ("");
        return 1;
      }
      if (STRING (chs_odd_bot).contains (unicharset_boxes.id_to_unichar(uch_id)[0]) &&
          (new_word_box.bottom () >
           baseline + applybox_error_band * row->x_height ())) {
        report_failed_box (boxfile_lineno, boxfile_charno, box,
                           unicharset_boxes.id_to_unichar(uch_id),
                           "FAILURE! Odd bottom char above baseline");
        new_word->set_text ("");
        return 1;
      }
      if (STRING (chs_desc).contains (unicharset_boxes.id_to_unichar(uch_id)[0]) &&
          (new_word_box.bottom () >
           baseline - applybox_error_band * row->x_height ())) {
        report_failed_box (boxfile_lineno, boxfile_charno, box,
                           unicharset_boxes.id_to_unichar(uch_id),
        "FAILURE! Descender doesn't descend");
        new_word->set_text ("");
        return 1;
      }
    }
#endif
}


/*************************************************************************
 * tidy_up()
 *   - report >1 block
 *   - sort the words in each row.
 *   - report any rows with no labelled words.
 *   - report any remaining unlabelled words
 *   - report total labelled words
 *
 *************************************************************************/
void tidy_up(                         //
             BLOCK_LIST *block_list,  //real blocks
             inT16 &ok_char_count,
             inT16 &ok_row_count,
             inT16 &unlabelled_words,
             inT16 *tgt_char_counts,
             inT16 &rebalance_count,
             UNICHAR_ID *min_uch_id,
             inT16 &min_samples,
             inT16 &final_labelled_blob_count,
             bool learn_character_fragments,
             bool learning) {
  BLOCK_IT block_it(block_list);
  ROW_IT row_it;
  ROW *row;
  WERD_IT word_it;
  WERD *word;
  WERD *duplicate_word;
  inT16 block_idx = 0;
  inT16 row_idx;
  inT16 all_row_idx = 0;
  BOOL8 row_ok;
  BOOL8 rebalance_needed = FALSE;
  inT16 *labelled_char_counts = NULL;  // num unique labelled samples
  inT16 i;
  UNICHAR_ID uch_id;
  UNICHAR_ID prev_uch_id = -1;
  BOOL8 at_dupe_of_prev_word;
  ROW *prev_row = NULL;
  inT16 left;
  inT16 prev_left = -1;

  labelled_char_counts = new inT16[MAX_NUM_CLASSES];
  for (i = 0; i < MAX_NUM_CLASSES; i++)
    labelled_char_counts[i] = 0;

  ok_char_count = 0;
  ok_row_count = 0;
  unlabelled_words = 0;
  if (learning && (applybox_debug > 4) && (block_it.length () != 1)) {
    if (block_it.length() > 1) {
      tprintf("APPLY_BOXES: More than one block??\n");
    } else {
      tprintf("APPLY_BOXES: No blocks identified.\n");
    }
  }

  for (block_it.mark_cycle_pt ();
  !block_it.cycled_list (); block_it.forward ()) {
    block_idx++;
    row_idx = 0;
    row_ok = FALSE;
    row_it.set_to_list (block_it.data ()->row_list ());
    for (row_it.mark_cycle_pt (); !row_it.cycled_list (); row_it.forward ()) {
      row_idx++;
      all_row_idx++;
      row = row_it.data ();
      word_it.set_to_list (row->word_list ());
      word_it.sort (word_comparator);
      for (word_it.mark_cycle_pt ();
      !word_it.cycled_list (); word_it.forward ()) {
        word = word_it.data ();
        if (strlen (word->text ()) == 0 ||
            unicharset_boxes.unichar_to_id(word->text()) < 0) {
          unlabelled_words++;
          if (learning && applybox_debug > 4 && !learn_character_fragments) {
            tprintf("APPLY_BOXES: Unlabelled word blk:%d row:%d allrows:%d\n",
                     block_idx, row_idx, all_row_idx);
          }
        } else {
          if (word->gblob_list ()->length () != 1)
            tprintf ("APPLY_BOXES: FATALITY - MULTIBLOB Labelled word blk:%d"
                     " row:%d allrows:%d\n", block_idx, row_idx, all_row_idx);

          ok_char_count++;
          ++labelled_char_counts[unicharset_boxes.unichar_to_id(word->text())];
          row_ok = TRUE;
        }
      }
      if ((applybox_debug > 6) && (!row_ok)) {
        tprintf("APPLY_BOXES: Row with no labelled words blk:%d row:%d"
                " allrows:%d\n", block_idx, row_idx, all_row_idx);
      }
      else
        ok_row_count++;
    }
  }

  min_samples = 9999;
  for (i = 0; i < unicharset_boxes.size(); i++) {
    if (tgt_char_counts[i] > labelled_char_counts[i]) {
      if (labelled_char_counts[i] <= 1) {
        tprintf("APPLY_BOXES: FATALITY - %d labelled samples of \"%s\" -"
                " target is %d:\n",
                labelled_char_counts[i], unicharset_boxes.debug_str(i).string(),
                tgt_char_counts[i]);
      }
      else {
        rebalance_needed = TRUE;
        if (applybox_debug > 0)
          tprintf("APPLY_BOXES: REBALANCE REQD \"%s\" - target of"
                  " %d from %d labelled samples\n",
                  unicharset_boxes.debug_str(i).string(), tgt_char_counts[i],
                  labelled_char_counts[i]);
      }
    }
    if ((min_samples > labelled_char_counts[i]) && (tgt_char_counts[i] > 0)) {
      min_samples = labelled_char_counts[i];
      *min_uch_id = i;
    }
  }

  while (applybox_rebalance && rebalance_needed) {
    block_it.set_to_list (block_list);
    for (block_it.mark_cycle_pt ();
    !block_it.cycled_list (); block_it.forward ()) {
      row_it.set_to_list (block_it.data ()->row_list ());
      for (row_it.mark_cycle_pt ();
      !row_it.cycled_list (); row_it.forward ()) {
        row = row_it.data ();
        word_it.set_to_list (row->word_list ());
        for (word_it.mark_cycle_pt ();
        !word_it.cycled_list (); word_it.forward ()) {
          word = word_it.data ();
          left = word->bounding_box ().left ();
          if (*word->text () != '\0')
            uch_id = unicharset_boxes.unichar_to_id(word->text ());
          else
            uch_id = -1;
          at_dupe_of_prev_word = ((row == prev_row) &&
            (left = prev_left) &&
            (uch_id == prev_uch_id));
          if ((uch_id != -1) &&
            (labelled_char_counts[uch_id] > 1) &&
            (tgt_char_counts[uch_id] > labelled_char_counts[uch_id]) &&
          (!at_dupe_of_prev_word)) {
            /* Duplicate the word to rebalance the labelled samples */
            if (applybox_debug > 9) {
              tprintf ("Duping \"%s\" from ", unicharset_boxes.id_to_unichar(uch_id));
              word->bounding_box ().print ();
            }
            duplicate_word = new WERD;
            *duplicate_word = *word;
            word_it.add_after_then_move (duplicate_word);
            rebalance_count++;
            labelled_char_counts[uch_id]++;
          }
          prev_row = row;
          prev_left = left;
          prev_uch_id = uch_id;
        }
      }
    }
    rebalance_needed = FALSE;
    for (i = 0; i < unicharset_boxes.size(); i++) {
      if ((tgt_char_counts[i] > labelled_char_counts[i]) &&
      (labelled_char_counts[i] > 1)) {
        rebalance_needed = TRUE;
        break;
      }
    }
  }

  /* Now final check - count labeled blobs */
  final_labelled_blob_count = 0;
  block_it.set_to_list (block_list);
  for (block_it.mark_cycle_pt ();
  !block_it.cycled_list (); block_it.forward ()) {
    row_it.set_to_list (block_it.data ()->row_list ());
    for (row_it.mark_cycle_pt (); !row_it.cycled_list (); row_it.forward ()) {
      row = row_it.data ();
      word_it.set_to_list (row->word_list ());
      word_it.sort (word_comparator);
      for (word_it.mark_cycle_pt ();
      !word_it.cycled_list (); word_it.forward ()) {
        word = word_it.data ();
        if ((strlen(word->text ()) > 0) &&
            (word->gblob_list()->length() == 1)) {
          final_labelled_blob_count++;
        } else {
          delete word_it.extract();
        }
      }
      // delete the row if empty
      if (row->word_list()->empty()) {
        delete row_it.extract();
      }
    }
  }

  // Clean up.
  delete[] labelled_char_counts;
}


void report_failed_box(inT16 boxfile_lineno,
                       inT16 boxfile_charno,
                       TBOX box,
                       const char *box_ch,
                       const char *err_msg) {
  if (applybox_debug > 4)
    tprintf ("APPLY_BOXES: boxfile %1d/%1d/%s ((%1d,%1d),(%1d,%1d)): %s\n",
      boxfile_lineno,
      boxfile_charno,
      box_ch,
      box.left (), box.bottom (), box.right (), box.top (), err_msg);
}


void apply_box_training(const STRING& filename, BLOCK_LIST *block_list) {
  BLOCK_IT block_it(block_list);
  ROW_IT row_it;
  ROW *row;
  WERD_IT word_it;
  WERD *word;
  WERD *bln_word;
  WERD copy_outword;             // copy to denorm
  PBLOB_IT blob_it;
  DENORM denorm;
  inT16 count = 0;
  char unichar[UNICHAR_LEN + 1];

  unichar[UNICHAR_LEN] = '\0';
  tprintf ("Generating training data\n");
  for (block_it.mark_cycle_pt ();
  !block_it.cycled_list (); block_it.forward ()) {
    row_it.set_to_list (block_it.data ()->row_list ());
    for (row_it.mark_cycle_pt (); !row_it.cycled_list (); row_it.forward ()) {
      row = row_it.data ();
      word_it.set_to_list (row->word_list ());
      for (word_it.mark_cycle_pt ();
      !word_it.cycled_list (); word_it.forward ()) {
        word = word_it.data ();
        if ((strlen (word->text ()) > 0) &&
        (word->gblob_list ()->length () == 1)) {
          // Here is a word with a single unichar label and a single blob so train on it.
          bln_word = make_bln_copy(word, row, NULL, row->x_height (), &denorm);
          blob_it.set_to_list (bln_word->blob_list ());
          strncpy(unichar, word->text (), UNICHAR_LEN);
          tess_training_tester (filename,
            blob_it.data (),     //single blob
            &denorm, TRUE,       //correct
            unichar,             //correct character
            strlen(unichar),     //character length
            NULL);
          copy_outword = *(bln_word);
          copy_outword.baseline_denormalise (&denorm);
          blob_it.set_to_list (copy_outword.blob_list ());
          delete bln_word;
          count++;
        }
      }
    }
  }
  tprintf ("Generated training data for %d blobs\n", count);
}

namespace tesseract {
void Tesseract::apply_box_testing(BLOCK_LIST *block_list) {
  BLOCK_IT block_it(block_list);
  ROW_IT row_it;
  ROW *row;
  inT16 row_count = 0;
  WERD_IT word_it;
  WERD *word;
  WERD *bln_word;
  inT16 word_count = 0;
  PBLOB_IT blob_it;
  DENORM denorm;
  inT16 count = 0;
  char ch[2];
  WERD *outword;                 //bln best choice
  //segmentation
  WERD_CHOICE *best_choice;      //tess output
  WERD_CHOICE *raw_choice;       //top choice permuter
                                 //detailed results
  BLOB_CHOICE_LIST_CLIST blob_choices;
  inT16 char_count = 0;
  inT16 correct_count = 0;
  inT16 err_count = 0;
  inT16 rej_count = 0;
  #ifndef SECURE_NAMES
  WERDSTATS wordstats;           //As from newdiff
  #endif
  char tess_rej_str[3];
  char tess_long_str[3];

  ch[1] = '\0';
  strcpy (tess_rej_str, "|A");
  strcpy (tess_long_str, "|B");

  for (block_it.mark_cycle_pt ();
  !block_it.cycled_list (); block_it.forward ()) {
    row_it.set_to_list (block_it.data ()->row_list ());
    for (row_it.mark_cycle_pt (); !row_it.cycled_list (); row_it.forward ()) {
      row = row_it.data ();
      row_count++;
      word_count = 0;
      word_it.set_to_list (row->word_list ());
      for (word_it.mark_cycle_pt ();
      !word_it.cycled_list (); word_it.forward ()) {
        word = word_it.data ();
        word_count++;
        if ((strlen (word->text ()) == 1) &&
          !STRING (applybox_test_exclusions).contains (*word->text ())
        && (word->gblob_list ()->length () == 1)) {
          // Here is a word with a single char label and a single blob so test it.
          bln_word = make_bln_copy(word, row, NULL, row->x_height (), &denorm);
          blob_it.set_to_list (bln_word->blob_list ());
          ch[0] = *word->text ();
          char_count++;
          best_choice = tess_segment_pass1 (bln_word,
            &denorm,
            &Tesseract::tess_default_matcher,
            raw_choice,
            &blob_choices, outword);

          /*
            Test for TESS screw up on word. Recog_word has already ensured that the
            choice list, outword blob lists and best_choice string are the same
            length. A TESS screw up is indicated by a blank filled or 0 length string.
          */
          if ((best_choice->length() == 0) ||
            (strspn(best_choice->unichar_string().string(), " ") ==
             best_choice->unichar_string().length())) {
            rej_count++;
            tprintf ("%d:%d: \"%s\" -> TESS FAILED\n",
              row_count, word_count, ch);
            #ifndef SECURE_NAMES
            wordstats.word (tess_rej_str, 2, ch, 1);
            #endif
          }
          else {
            if ((best_choice->length() != outword->blob_list()->length()) ||
                (best_choice->length() != blob_choices.length())) {
              tprintf
                ("ASSERT FAIL String:\"%s\"; Strlen=%d; #Blobs=%d; #Choices=%d\n",
                best_choice->unichar_string().string(),
                best_choice->length(),
                outword->blob_list ()->length(),
                blob_choices.length());
            }
            ASSERT_HOST(best_choice->length() ==
                        outword->blob_list()->length());
            ASSERT_HOST(best_choice->length() == blob_choices.length());
            fix_quotes (best_choice,
                                 //turn to double
              outword, &blob_choices);
            if (strcmp (best_choice->unichar_string().string(), ch) != 0) {
              err_count++;
              tprintf ("%d:%d: \"%s\" -> \"%s\"\n",
                row_count, word_count, ch,
                best_choice->unichar_string().string());
            }
            else
              correct_count++;
            #ifndef SECURE_NAMES
            if (best_choice->unichar_string().length() > 2)
              wordstats.word(tess_long_str, 2, ch, 1);
            else
              wordstats.word(best_choice->unichar_string().string(),
                             best_choice->unichar_string().length(),
                             ch, 1);
            #endif
          }
          delete bln_word;
          delete outword;
          delete best_choice;
          delete raw_choice;
          blob_choices.deep_clear ();
          count++;
        }
      }
    }
  }
  #ifndef SECURE_NAMES
  wordstats.print (1, 100.0);
  wordstats.conf_matrix ();
  tprintf ("Tested %d chars: %d correct; %d rejected by tess; %d errs\n",
    char_count, correct_count, rej_count, err_count);
  #endif
}

}  // namespace tesseract
