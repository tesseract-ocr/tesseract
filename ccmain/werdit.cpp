/**********************************************************************
 * File:        werdit.cpp  (Formerly wordit.c)
 * Description: An iterator for passing over all the words in a document.
 * Author:		Ray Smith
 * Created:		Mon Apr 27 08:51:22 BST 1992
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
#include          "werdit.h"

#define EXTERN

//EXTERN BOOL_VAR(wordit_linearc,FALSE,"Pass poly of linearc to Tess");

/**********************************************************************
 * WERDIT::start_page
 *
 * Get ready to iterate over the page by setting the iterators.
 **********************************************************************/

void WERDIT::start_page(                        //set iterators
                        BLOCK_LIST *block_list  //blocks to check
                       ) {
  block_it.set_to_list (block_list);
  block_it.mark_cycle_pt ();
  do {
    while (block_it.data ()->row_list ()->empty ()
    && !block_it.cycled_list ()) {
      block_it.forward ();
    }
    if (!block_it.data ()->row_list ()->empty ()) {
      row_it.set_to_list (block_it.data ()->row_list ());
      row_it.mark_cycle_pt ();
      while (row_it.data ()->word_list ()->empty ()
      && !row_it.cycled_list ()) {
        row_it.forward ();
      }
      if (!row_it.data ()->word_list ()->empty ()) {
        word_it.set_to_list (row_it.data ()->word_list ());
        word_it.mark_cycle_pt ();
      }
    }
  }
  while (!block_it.cycled_list () && row_it.data ()->word_list ()->empty ());
}


/**********************************************************************
 * WERDIT::forward
 *
 * Give the next word on the page, or NULL if none left.
 * This code assumes all rows to be non-empty, but blocks are allowed
 * to be empty as eventually we will have non-text blocks.
 * The output is always a copy and needs to be deleted by somebody.
 **********************************************************************/

WERD *WERDIT::forward() {  //use iterators
  WERD *word;                    //actual word
  //      WERD                                                    *larc_word;                                                     //linearc copy
  WERD *result;                  //output word
  ROW *row;                      //row of word

  if (word_it.cycled_list ()) {
    return NULL;                 //finished page
  }
  else {
    word = word_it.data ();
    row = row_it.data ();
    word_it.forward ();
    if (word_it.cycled_list ()) {
      row_it.forward ();         //finished row
      if (row_it.cycled_list ()) {
        do {
          block_it.forward ();   //finished block
          if (!block_it.cycled_list ()) {
            row_it.set_to_list (block_it.data ()->row_list ());
            row_it.mark_cycle_pt ();
          }
        }
                                 //find non-empty block
        while (!block_it.cycled_list ()
          && row_it.cycled_list ());
      }
      if (!row_it.cycled_list ()) {
        word_it.set_to_list (row_it.data ()->word_list ());
        word_it.mark_cycle_pt ();
      }
    }

    //              if (wordit_linearc && !word->flag(W_POLYGON))
    //              {
    //                      larc_word=word->larc_copy(row->x_height());
    //                      result=larc_word->poly_copy(row->x_height());
    //                      delete larc_word;
    //              }
    //              else
    result = word->poly_copy (row->x_height ());
    return result;
  }
}


/**********************************************************************
 * make_pseudo_word
 *
 * Make all the blobs inside a selection into a single word.
 * The word is always a copy and needs to be deleted.
 **********************************************************************/

WERD *make_pseudo_word(                         //make fake word
                       BLOCK_LIST *block_list,  //blocks to check //block of selection
                       TBOX &selection_box,
                       BLOCK *&pseudo_block,
                       ROW *&pseudo_row         //row of selection
                      ) {
  BLOCK_IT block_it(block_list);
  BLOCK *block;
  ROW_IT row_it;
  ROW *row;
  WERD_IT word_it;
  WERD *word;
  PBLOB_IT blob_it;
  PBLOB *blob;
  PBLOB_LIST new_blobs;          //list of gathered blobs
                                 //iterator
  PBLOB_IT new_blob_it = &new_blobs;
  WERD *pseudo_word;             //fabricated word
  WERD *poly_word;               //poly copy of word
  //      WERD                                                    *larc_word;                                                     //linearc copy

  for (block_it.mark_cycle_pt ();
  !block_it.cycled_list (); block_it.forward ()) {
    block = block_it.data ();
    if (block->bounding_box ().overlap (selection_box)) {
      pseudo_block = block;
      row_it.set_to_list (block->row_list ());
      for (row_it.mark_cycle_pt ();
      !row_it.cycled_list (); row_it.forward ()) {
        row = row_it.data ();
        if (row->bounding_box ().overlap (selection_box)) {
          word_it.set_to_list (row->word_list ());
          for (word_it.mark_cycle_pt ();
          !word_it.cycled_list (); word_it.forward ()) {
            word = word_it.data ();
            if (word->bounding_box ().overlap (selection_box)) {
              //                                                      if (wordit_linearc && !word->flag(W_POLYGON))
              //                                                      {
              //                                                              larc_word=word->larc_copy(row->x_height());
              //                                                              poly_word=larc_word->poly_copy(row->x_height());
              //                                                              delete larc_word;
              //                                                      }
              //                                                      else
              poly_word = word->poly_copy (row->x_height ());
              blob_it.set_to_list (poly_word->blob_list ());
              for (blob_it.mark_cycle_pt ();
              !blob_it.cycled_list (); blob_it.forward ()) {
                blob = blob_it.data ();
                if (blob->bounding_box ().
                overlap (selection_box)) {
                  new_blob_it.add_after_then_move (blob_it.
                    extract
                    ());
                                 //steal off list
                  pseudo_row = row;
                }
              }
              delete poly_word;  //get rid of it
            }
          }
        }
      }
    }
  }
  if (!new_blobs.empty ()) {
                                 //make new word
    pseudo_word = new WERD (&new_blobs, 1, NULL);
  }
  else
    pseudo_word = NULL;
  return pseudo_word;
}
