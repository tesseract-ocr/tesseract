/**********************************************************************
 * File:        pageres.cpp  (Formerly page_res.c)
 * Description: Results classes used by control.c
 * Author:		Phil Cheatle
 * Created:     Tue Sep 22 08:42:49 BST 1992
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
#include "mfcpch.h"
#include          <stdlib.h>
#ifdef __UNIX__
#include          <assert.h>
#endif
#include          "pageres.h"
#include          "notdll.h"

ELISTIZE (BLOCK_RES)
CLISTIZE (BLOCK_RES) ELISTIZE (ROW_RES) ELISTIZE (WERD_RES)
/*************************************************************************
 * PAGE_RES::PAGE_RES
 *
 * Constructor for page results
 *************************************************************************/
PAGE_RES::PAGE_RES(                            //recursive construct
                   BLOCK_LIST *the_block_list  //real page
                  ) {
  BLOCK_IT block_it(the_block_list);
  BLOCK_RES_IT block_res_it(&block_res_list);

  char_count = 0;
  rej_count = 0;
  rejected = FALSE;

  for (block_it.mark_cycle_pt ();
  !block_it.cycled_list (); block_it.forward ()) {
    block_res_it.add_to_end (new BLOCK_RES (block_it.data ()));
  }
}


/*************************************************************************
 * BLOCK_RES::BLOCK_RES
 *
 * Constructor for BLOCK results
 *************************************************************************/

BLOCK_RES::BLOCK_RES(                  //recursive construct
                     BLOCK *the_block  //real BLOCK
                    ) {
  ROW_IT row_it (the_block->row_list ());
  ROW_RES_IT row_res_it(&row_res_list);

  char_count = 0;
  rej_count = 0;
  font_class = -1;               //not assigned
  x_height = -1.0;
  font_assigned = FALSE;
  bold = FALSE;
  italic = FALSE;
  row_count = 0;

  block = the_block;

  for (row_it.mark_cycle_pt (); !row_it.cycled_list (); row_it.forward ()) {
    row_res_it.add_to_end (new ROW_RES (row_it.data ()));
  }
}


/*************************************************************************
 * ROW_RES::ROW_RES
 *
 * Constructor for ROW results
 *************************************************************************/

ROW_RES::ROW_RES(              //recursive construct
                 ROW *the_row  //real ROW
                ) {
  WERD_IT word_it (the_row->word_list ());
  WERD_RES_IT word_res_it(&word_res_list);
  WERD_RES *combo = NULL;        //current combination of fuzzies
  WERD_RES *word_res;            //current word
  WERD *copy_word;

  char_count = 0;
  rej_count = 0;
  whole_word_rej_count = 0;
  font_class = -1;
  font_class_score = -1.0;
  bold = FALSE;
  italic = FALSE;

  row = the_row;

  for (word_it.mark_cycle_pt (); !word_it.cycled_list (); word_it.forward ()) {
    word_res = new WERD_RES (word_it.data ());
    word_res->x_height = the_row->x_height();

    if (word_res->word->flag (W_FUZZY_NON)) {
      ASSERT_HOST (combo != NULL);
      word_res->part_of_combo = TRUE;
      combo->copy_on (word_res);
    }
    if (word_it.data_relative (1)->flag (W_FUZZY_NON)) {
      if (combo == NULL) {
        copy_word = new WERD;
                                 //deep copy
        *copy_word = *(word_it.data ());
        combo = new WERD_RES (copy_word);
        combo->x_height = the_row->x_height();
        combo->combination = TRUE;
        word_res_it.add_to_end (combo);
      }
      word_res->part_of_combo = TRUE;
    }
    else
      combo = NULL;
    word_res_it.add_to_end (word_res);
  }
}


WERD_RES & WERD_RES::operator= ( //assign word_res
const WERD_RES & source          //from this
) {
  this->ELIST_LINK::operator= (source);
  if (source.combination) {
    word = new WERD;
    *word = *(source.word);      //deep copy
  }
  else
    word = source.word;          //pt to same word

  if (source.outword != NULL) {
    outword = new WERD;
    *outword = *(source.outword);//deep copy
  }
  else
    outword = NULL;

  denorm = source.denorm;
  if (source.best_choice != NULL) {
    best_choice = new WERD_CHOICE;
    *best_choice = *(source.best_choice);
    raw_choice = new WERD_CHOICE;
    *raw_choice = *(source.raw_choice);
  }
  else {
    best_choice = NULL;
    raw_choice = NULL;
  }
  if (source.ep_choice != NULL) {
    ep_choice = new WERD_CHOICE;
    *ep_choice = *(source.ep_choice);
  }
  else
    ep_choice = NULL;
  reject_map = source.reject_map;
  tess_failed = source.tess_failed;
  tess_accepted = source.tess_accepted;
  tess_would_adapt = source.tess_would_adapt;
  done = source.done;
  unlv_crunch_mode = source.unlv_crunch_mode;
  italic = source.italic;
  bold = source.bold;
  font1 = source.font1;
  font1_count = source.font1_count;
  font2 = source.font2;
  font2_count = source.font2_count;
  x_height = source.x_height;
  caps_height = source.caps_height;
  guessed_x_ht = source.guessed_x_ht;
  guessed_caps_ht = source.guessed_caps_ht;
  combination = source.combination;
  part_of_combo = source.part_of_combo;
  reject_spaces = source.reject_spaces;
  return *this;
}


WERD_RES::~WERD_RES () {
  if (combination)
    delete word;
  if (outword != NULL)
    delete outword;
  if (best_choice != NULL) {
    delete best_choice;
    delete raw_choice;
  }
  if (ep_choice != NULL) {
    delete ep_choice;
  }
}


/*************************************************************************
 * PAGE_RES_IT::restart_page
 *
 * Set things up at the start of the page
 *************************************************************************/

WERD_RES *PAGE_RES_IT::restart_page() {
  block_res_it.set_to_list (&page_res->block_res_list);
  block_res_it.mark_cycle_pt ();
  prev_block_res = NULL;
  prev_row_res = NULL;
  prev_word_res = NULL;
  block_res = NULL;
  row_res = NULL;
  word_res = NULL;
  next_block_res = NULL;
  next_row_res = NULL;
  next_word_res = NULL;
  internal_forward(TRUE);
  return internal_forward (FALSE);
}


/*************************************************************************
 * PAGE_RES_IT::internal_forward
 *
 * Find the next word on the page. Empty blocks and rows are skipped.
 * The iterator maintains pointers to block, row and word for the previous,
 * current and next words.  These are correct, regardless of block/row
 * boundaries. NULL values denote start and end of the page.
 *************************************************************************/

WERD_RES *PAGE_RES_IT::internal_forward(BOOL8 new_block) {
  BOOL8 found_next_word = FALSE;
  BOOL8 new_row = FALSE;

  prev_block_res = block_res;
  prev_row_res = row_res;
  prev_word_res = word_res;
  block_res = next_block_res;
  row_res = next_row_res;
  word_res = next_word_res;

  while (!found_next_word && !block_res_it.cycled_list ()) {
    if (new_block) {
      new_block = FALSE;
      row_res_it.set_to_list (&block_res_it.data ()->row_res_list);
      row_res_it.mark_cycle_pt ();
      new_row = TRUE;
    }
    while (!found_next_word && !row_res_it.cycled_list ()) {
      if (new_row) {
        new_row = FALSE;
        word_res_it.set_to_list (&row_res_it.data ()->word_res_list);
        word_res_it.mark_cycle_pt ();
      }
      while (!found_next_word && !word_res_it.cycled_list ()) {
        next_block_res = block_res_it.data ();
        next_row_res = row_res_it.data ();
        next_word_res = word_res_it.data ();
        found_next_word = TRUE;
        do {
          word_res_it.forward ();
        }
        while (word_res_it.data ()->part_of_combo);
      }
      if (!found_next_word) {    //end of row reached
        row_res_it.forward ();
        new_row = TRUE;
      }
    }
    if (!found_next_word) {      //end of block reached
      block_res_it.forward ();
      new_block = TRUE;
    }
  }
  if (!found_next_word) {        //end of page reached
    next_block_res = NULL;
    next_row_res = NULL;
    next_word_res = NULL;
  }
  return word_res;
}


/*************************************************************************
 * PAGE_RES_IT::forward_block
 *
 * Move to the first word of the next block
 * Can be followed by subsequent calls to forward() BUT at the first word in
 * the block, the prev block, row and word are all NULL.
 *************************************************************************/

WERD_RES *PAGE_RES_IT::forward_block() {
  if (block_res == next_block_res) {
    block_res_it.forward ();;
    block_res = NULL;
    row_res = NULL;
    word_res = NULL;
    next_block_res = NULL;
    next_row_res = NULL;
    next_word_res = NULL;
    internal_forward(TRUE);
  }
  return internal_forward (FALSE);
}


void PAGE_RES_IT::rej_stat_word() {
  inT16 chars_in_word;
  inT16 rejects_in_word = 0;

  chars_in_word = word_res->reject_map.length ();
  page_res->char_count += chars_in_word;
  block_res->char_count += chars_in_word;
  row_res->char_count += chars_in_word;

  rejects_in_word = word_res->reject_map.reject_count ();

  page_res->rej_count += rejects_in_word;
  block_res->rej_count += rejects_in_word;
  row_res->rej_count += rejects_in_word;
  if (chars_in_word == rejects_in_word)
    row_res->whole_word_rej_count += rejects_in_word;
}
