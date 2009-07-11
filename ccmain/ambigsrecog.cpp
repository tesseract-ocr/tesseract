///////////////////////////////////////////////////////////////////////
// File:        genericvector.h
// Description: Functions for producing classifications
//              for the input to ambigstraining.
// Author:      Daria Antonova
// Created:     Mon Jun 23 11:26:43 PDT 2008
//
// (C) Copyright 2007, Google Inc.
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

#include "ambigs.h"

#include "applybox.h"
#include "boxread.h"
#include "control.h"
#include "permute.h"
#include "ratngs.h"
#include "reject.h"
#include "stopper.h"
#include "tesseractclass.h"

namespace tesseract {

// Sets flags necessary for ambigs training mode.
// Opens and returns the pointer to the output file.
FILE *Tesseract::init_ambigs_training(const STRING &fname) {
  permute_only_top = 1;                        // use only top choice permuter
  tessedit_tess_adaption_mode.set_value(0);    // turn off adaption
  tessedit_ok_mode.set_value(0);               // turn off context checking
  tessedit_enable_doc_dict.set_value(0);       // turn off document dictionary
  save_best_choices.set_value(1);              // save individual char choices
  stopper_no_acceptable_choices.set_value(1);  // explore all segmentations
  save_raw_choices.set_value(1);               // save raw choices

  // Open ambigs output file.
  STRING output_fname = fname;
  const char *lastdot = strrchr(output_fname.string(), '.');
  if (lastdot != NULL) {
    output_fname[lastdot - output_fname.string()] = '\0';
  }
  output_fname += ".txt";
  FILE *output_file;
  if (!(output_file = fopen(output_fname.string(), "a+"))) {
    CANTOPENFILE.error("ambigs_training", EXIT,
                       "Can't open box file %s\n", output_fname.string());
  }
  return output_file;
}

// This function takes tif/box pair of files and runs recognition on the image,
// while making sure that the word bounds that tesseract identified roughly
// match to those specified by the input box file. For each word (ngram in a
// single bounding box from the input box file) it outputs the ocred result,
// the correct label, rating and certainty.
void Tesseract::ambigs_training_segmented(const STRING &fname,
                                          PAGE_RES *page_res,
                                          volatile ETEXT_DESC *monitor,
                                          FILE *output_file) {
  STRING box_fname = fname;
  const char *lastdot = strrchr(box_fname.string(), '.');
  if (lastdot != NULL) {
    box_fname[lastdot - box_fname.string()] = '\0';
  }
  box_fname += ".box";
  FILE *box_file;
  if (!(box_file = fopen(box_fname.string(), "r"))) {
    CANTOPENFILE.error("ambigs_training", EXIT,
                       "Can't open box file %s\n", box_fname.string());
  }

  static PAGE_RES_IT page_res_it;
  page_res_it.page_res = page_res;
  page_res_it.restart_page();
  int x_min, y_min, x_max, y_max;
  char label[UNICHAR_LEN * 10];

  // Process all the words on this page.
  while (page_res_it.word() != NULL &&
         read_next_box(applybox_page, box_file, label,
                       &x_min, &y_min, &x_max, &y_max)) {
    // Init bounding box of the current word bounding box and from box file.
    TBOX box = TBOX(ICOORD(x_min, y_min), ICOORD(x_max, y_max));
    TBOX word_box(page_res_it.word()->word->bounding_box());
    bool one_word = true;
    // Check whether the bounding box of the next word overlaps with the
    // current box from box file.
    while (page_res_it.next_word() != NULL &&
           box.x_overlap(page_res_it.next_word()->word->bounding_box())) {
      word_box = word_box.bounding_union(
          page_res_it.next_word()->word->bounding_box());
      page_res_it.forward();
      one_word = false;
    }
    if (!word_box.major_overlap(box)) {
      if (!word_box.x_overlap(box)) {
        // We must be looking at the word that belongs in the "next" bounding
        // box from the box file. The ngram that was supposed to appear in
        // the current box read from the box file must have been dropped by
        // tesseract as noise.
        tprintf("Word %s was dropped as noise.\n", label);
        continue;  // stay on this blob, but read next box from box file
      } else {
        tprintf("Error: Insufficient overlap for word box"
                " and box from file for %s\n", label);
        word_box.print();
        box.print();
        exit(1);
      }
    }
    // Skip recognizing the ngram if tesseract is sure it's not
    // one word, otherwise run one recognition pass on this word.
    if (!one_word) {
      tprintf("Tesseract segmented %s as multiple words\n", label);
    } else {
      ambigs_classify_and_output(&page_res_it, label, output_file);
    }
    page_res_it.forward();
  }
  fclose(box_file);
}

// Run classify_word_pass1() on the current word. Output tesseract's raw choice
// as a result of the classification. For words labeled with a single unichar
// also output all alternatives from blob_choices of the best choice.
void Tesseract::ambigs_classify_and_output(PAGE_RES_IT *page_res_it,
                                           const char *label,
                                           FILE *output_file) {
  int offset;
  // Classify word.
  classify_word_pass1(page_res_it->word(), page_res_it->row()->row,
                      page_res_it->block()->block,
                      FALSE, NULL, NULL);
  WERD_CHOICE *best_choice = page_res_it->word()->best_choice;
  ASSERT_HOST(best_choice != NULL);
  ASSERT_HOST(best_choice->blob_choices() != NULL);

  // Compute the number of unichars in the label.
  int label_num_unichars = 0;
  int step = 1;  // should be non-zero on the first iteration
  for (offset = 0; label[offset] != '\0' && step > 0;
       step = getDict().getUnicharset().step(label + offset),
       offset += step, ++label_num_unichars);
  if (step == 0) {
    tprintf("Not outputting illegal unichar %s\n", label);
    return;
  }

  // Output all classifier choices for the unigrams (1-1 classifications).
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
  // Output the raw choice for succesful non 1-1 classifications.
  getDict().PrintAmbigAlternatives(output_file, label, label_num_unichars);
}

}  // namespace tesseract
