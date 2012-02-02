///////////////////////////////////////////////////////////////////////
// File:        recogtraining.cpp
// Description: Functions for ambiguity and parameter training.
// Author:      Daria Antonova
// Created:     Mon Aug 13 11:26:43 PDT 2009
//
// (C) Copyright 2009, Google Inc.
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
///////////////////////////////////////////////////////////////////////

#include "tesseractclass.h"

#include "boxread.h"
#include "control.h"
#include "cutil.h"
#include "host.h"
#include "permute.h"
#include "ratngs.h"
#include "reject.h"
#include "stopper.h"

namespace tesseract {

const inT16 kMaxBoxEdgeDiff = 2;

// Sets flags necessary for recognition in the training mode.
// Opens and returns the pointer to the output file.
FILE *Tesseract::init_recog_training(const STRING &fname) {
  if (tessedit_ambigs_training) {
    tessedit_tess_adaption_mode.set_value(0);    // turn off adaption
    tessedit_enable_doc_dict.set_value(0);       // turn off document dictionary
    save_blob_choices.set_value(1);              // save individual char choices
    getDict().save_raw_choices.set_value(1);     // save raw choices
    getDict().permute_only_top.set_value(true);  // use only top choice permuter
    tessedit_ok_mode.set_value(0);               // turn off context checking
    // Explore all segmentations.
    getDict().stopper_no_acceptable_choices.set_value(1);
  }

  STRING output_fname = fname;
  const char *lastdot = strrchr(output_fname.string(), '.');
  if (lastdot != NULL) output_fname[lastdot - output_fname.string()] = '\0';
  output_fname += ".txt";
  FILE *output_file = open_file(output_fname.string(), "a+");
  return output_file;
}

// Copies the bounding box from page_res_it->word() to the given TBOX.
bool read_t(PAGE_RES_IT *page_res_it, TBOX *tbox) {
  while (page_res_it->block() != NULL) {
    if (page_res_it->word() != NULL)
      break;
    page_res_it->forward();
  }

  if (page_res_it->word() != NULL) {
    *tbox = page_res_it->word()->word->bounding_box();
    page_res_it->forward();

    // If tbox->left() is negative, the training image has vertical text and
    // all the coordinates of bounding boxes of page_res are rotated by 90
    // degrees in a counterclockwise direction. We need to rotate the TBOX back
    // in order to compare with the TBOXes of box files.
    if (tbox->left() < 0) {
      tbox->rotate(FCOORD(0.0, -1.0));
    }

    return true;
  } else {
    return false;
  }
}

// This function takes tif/box pair of files and runs recognition on the image,
// while making sure that the word bounds that tesseract identified roughly
// match to those specified by the input box file. For each word (ngram in a
// single bounding box from the input box file) it outputs the ocred result,
// the correct label, rating and certainty.
void Tesseract::recog_training_segmented(const STRING &fname,
                                         PAGE_RES *page_res,
                                         volatile ETEXT_DESC *monitor,
                                         FILE *output_file) {
  STRING box_fname = fname;
  const char *lastdot = strrchr(box_fname.string(), '.');
  if (lastdot != NULL) box_fname[lastdot - box_fname.string()] = '\0';
  box_fname += ".box";
  // read_next_box() will close box_file
  FILE *box_file = open_file(box_fname.string(), "r");

  PAGE_RES_IT page_res_it;
  page_res_it.page_res = page_res;
  page_res_it.restart_page();
  STRING label;

  // Process all the words on this page.
  TBOX tbox;  // tesseract-identified box
  TBOX bbox;  // box from the box file
  bool keep_going;
  int line_number = 0;
  int examined_words = 0;
  do {
    keep_going = read_t(&page_res_it, &tbox);
    keep_going &= ReadNextBox(applybox_page, &line_number, box_file, &label,
                              &bbox);
    // Align bottom left points of the TBOXes.
    while (keep_going &&
           !NearlyEqual<int>(tbox.bottom(), bbox.bottom(), kMaxBoxEdgeDiff)) {
      keep_going = (bbox.bottom() < tbox.bottom()) ?
          read_t(&page_res_it, &tbox) :
            ReadNextBox(applybox_page, &line_number, box_file, &label, &bbox);
    }
    while (keep_going &&
           !NearlyEqual<int>(tbox.left(), bbox.left(), kMaxBoxEdgeDiff)) {
      keep_going = (bbox.left() > tbox.left()) ? read_t(&page_res_it, &tbox) :
          ReadNextBox(applybox_page, &line_number, box_file, &label, &bbox);
    }
    // OCR the word if top right points of the TBOXes are similar.
    if (keep_going &&
        NearlyEqual<int>(tbox.right(), bbox.right(), kMaxBoxEdgeDiff) &&
        NearlyEqual<int>(tbox.top(), bbox.top(), kMaxBoxEdgeDiff)) {
        ambigs_classify_and_output(page_res_it.prev_word(),
                                   page_res_it.prev_row(),
                                   page_res_it.prev_block(),
                                   label.string(), output_file);
        examined_words++;
    }
  } while (keep_going);

  // Set up scripts on all of the words that did not get sent to
  // ambigs_classify_and_output.  They all should have, but if all the
  // werd_res's don't get uch_sets, tesseract will crash when you try
  // to iterate over them. :-(
  int total_words = 0;
  for (page_res_it.restart_page(); page_res_it.block() != NULL;
       page_res_it.forward()) {
    if (page_res_it.word()) {
      if (page_res_it.word()->uch_set == NULL)
        page_res_it.word()->SetupFake(unicharset);
      total_words++;
    }
  }
  if (examined_words < 0.85 * total_words) {
    tprintf("TODO(antonova): clean up recog_training_segmented; "
            " It examined only a small fraction of the ambigs image.\n");
  }
  tprintf("recog_training_segmented: examined %d / %d words.\n",
          examined_words, total_words);
}

// Runs classify_word_pass1() on the current word. Outputs Tesseract's
// raw choice as a result of the classification. For words labeled with a
// single unichar also outputs all alternatives from blob_choices of the
// best choice.
void Tesseract::ambigs_classify_and_output(WERD_RES *werd_res,
                                           ROW_RES *row_res,
                                           BLOCK_RES *block_res,
                                           const char *label,
                                           FILE *output_file) {
  int offset;
  // Classify word.
  fflush(stdout);
  classify_word_pass1(block_res->block, row_res->row, werd_res);
  WERD_CHOICE *best_choice = werd_res->best_choice;
  ASSERT_HOST(best_choice != NULL);
  ASSERT_HOST(best_choice->blob_choices() != NULL);

  // Compute the number of unichars in the label.
  int label_num_unichars = 0;
  int step = 1;  // should be non-zero on the first iteration
  for (offset = 0; label[offset] != '\0' && step > 0;
       step = werd_res->uch_set->step(label + offset),
       offset += step, ++label_num_unichars);
  if (step == 0) {
    tprintf("Not outputting illegal unichar %s\n", label);
    return;
  }

  // Output all classifier choices for the unigrams (1->1 classifications).
  if (label_num_unichars == 1 && best_choice->blob_choices()->length() == 1) {
    BLOB_CHOICE_LIST_C_IT outer_blob_choice_it;
    outer_blob_choice_it.set_to_list(best_choice->blob_choices());
    BLOB_CHOICE_IT blob_choice_it;
    blob_choice_it.set_to_list(outer_blob_choice_it.data());
    for (blob_choice_it.mark_cycle_pt();
         !blob_choice_it.cycled_list();
         blob_choice_it.forward()) {
      BLOB_CHOICE *blob_choice = blob_choice_it.data();
      if (blob_choice->unichar_id() != INVALID_UNICHAR_ID) {
        fprintf(output_file, "%s\t%s\t%.4f\t%.4f\n",
               unicharset.id_to_unichar(blob_choice->unichar_id()),
               label, blob_choice->rating(), blob_choice->certainty());
      }
    }
  }
  // Output raw choices for many->many and 1->many classifications.
  getDict().PrintAmbigAlternatives(output_file, label, label_num_unichars);
}

}  // namespace tesseract
