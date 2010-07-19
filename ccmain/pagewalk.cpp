/**********************************************************************
 * File:        pagewalk.cpp  (Formerly walkers.c)
 * Description: Block list processors
 * Author:      Phil Cheatle
 * Created:     Thu Oct 10 16:25:24 BST 1991
 *
 * (C) Copyright 1991, Hewlett-Packard Ltd.
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
#include "pagewalk.h"
#include "tesseractclass.h"

#define EXTERN

EXTERN BOOL_VAR (current_word_quit, FALSE, "Stop processing this word");
DLLSYM BOOL_VAR (selection_quit, FALSE, "Stop processing this selection");

/**
 *  block_list_bounding_box()
 *
 *  Scan block list to find the bounding box of all blocks.
 *  @param block_list the block list to find the bounding box of
 */

TBOX block_list_bounding_box(BLOCK_LIST *block_list) 
{
  BLOCK_IT block_it(block_list);
  TBOX enclosing_box;

  for (block_it.mark_cycle_pt (); !block_it.cycled_list ();
    block_it.forward ())
  enclosing_box += block_it.data ()->bounding_box ();
  return enclosing_box;
}


/**
 *  block_list_compress()
 *
 *  Pack a block list to occupy a smaller space by compressing each block and
 *  moving the compressed blocks one above the other.
 *  The compressed block list has the same top left point as the uncompressed
 *  first.  Blocks are reordered so that the source names are in alphabetic
 *  order. (This gathers together, but does not combine, blocks from the same
 *  file.)
 *
 *  The enclosing box of the compressed block list is returned.
 */

const TBOX block_list_compress(BLOCK_LIST *block_list) 
{
  BLOCK_IT block_it(block_list);
  BLOCK *block;
  ICOORD initial_top_left;
  ICOORD block_spacing (0, BLOCK_SPACING);
  TBOX enclosing_box;             //for full display

  initial_top_left = block_it.data()->bounding_box().topleft();
                                 //group srcfile blks
  block_it.sort (block_name_order);

  /* Compress the target block list into an area starting from the top left of
    the first block on the list */

  enclosing_box = TBOX (initial_top_left, initial_top_left);
  enclosing_box.move_bottom_edge (BLOCK_SPACING);

  for (block_it.mark_cycle_pt ();
  !block_it.cycled_list (); block_it.forward ()) {
    block = block_it.data ();
    block->compress (enclosing_box.botleft () - block_spacing -
      block->bounding_box ().topleft ());
    enclosing_box += block->bounding_box ();
  }
  return enclosing_box;
}


/**
 * block_list_move()
 *
 * Move all the blocks in the list by a vector
 *
 * @param block_list the block list to move
 * @param vec the vector to move it by
 */

void block_list_move(BLOCK_LIST *block_list,
                     ICOORD vec) 
{
  BLOCK_IT block_it(block_list);

  for (block_it.mark_cycle_pt (); !block_it.cycled_list ();
       block_it.forward ())
    block_it.data ()->move (vec);
}


/**
 *  block_name_order()
 *
 *  Block comparator used to sort a block list so that blocks from the same
 *  filename are located together, and blocks from the same file are ordered
 *  by vertical position.
 */

int block_name_order(const void *block1p,
                     const void *block2p) 
{
  int result;
  BLOCK *block1 = *(BLOCK **) block1p;
  BLOCK *block2 = *(BLOCK **) block2p;

  result = strcmp (block1->name (), block2->name ());
  if (result == 0)
    result = block2->bounding_box ().top () - block1->bounding_box ().top ();
  return result;
}


/**
 * process_all_blobs()
 *
 * Walk the current block list applying the specified blob processor function
 * to all blobs
 * @param block_list the blocks to check
 * @param blob_processor function to call
 * @param c_blob_processor function to call
 */

void
process_all_blobs (BLOCK_LIST * block_list,
                   BOOL8 blob_processor (BLOCK *, ROW *, WERD *, PBLOB *), 
                   BOOL8 c_blob_processor (BLOCK *, ROW *, WERD *, C_BLOB *)) 
{
  BLOCK_IT block_it(block_list);
  BLOCK *block;
  ROW_IT row_it;
  ROW *row;
  WERD_IT word_it;
  WERD *word;
  PBLOB_IT blob_it;
  PBLOB *blob;
  C_BLOB_IT c_blob_it;
  C_BLOB *c_blob;

  for (block_it.mark_cycle_pt (); !block_it.cycled_list (); block_it.forward ()) {
    block = block_it.data ();
    row_it.set_to_list (block->row_list ());
    for (row_it.mark_cycle_pt (); !row_it.cycled_list (); row_it.forward ()) {
      row = row_it.data ();
      word_it.set_to_list (row->word_list ());
      for (word_it.mark_cycle_pt (); !word_it.cycled_list (); word_it.forward ()) {
        word = word_it.data ();
        if (word->flag (W_POLYGON)) {
          if (blob_processor != NULL) {
            blob_it.set_to_list (word->blob_list ());
            for (blob_it.mark_cycle_pt (); !blob_it.cycled_list (); blob_it.forward ()) {
              blob = blob_it.data ();
              if (!blob_processor (block, row, word, blob) || selection_quit)
                return;
            }
          }
        }
        else {
          if (c_blob_processor != NULL) {
            c_blob_it.set_to_list (word->cblob_list ());
            for (c_blob_it.mark_cycle_pt (); !c_blob_it.cycled_list (); c_blob_it.forward ()) {
              c_blob = c_blob_it.data ();
              if (!c_blob_processor (block, row, word, c_blob) || selection_quit)
                return;
            }
          }
        }
      }
    }
  }
}


/**
 * process_selected_blobs()
 *
 * Walk the current block list applying the specified blob processor function
 * to each selected blob
 * @param block_list the blocks to check
 * @param selection_box within this box(?)
 * @param blob_processor function to call
 * @param c_blob_processor function to call
 */

void
process_selected_blobs (BLOCK_LIST * block_list,
                        TBOX & selection_box, 
                        BOOL8 blob_processor (BLOCK *, ROW *, WERD *, PBLOB *), 
                        BOOL8 c_blob_processor (BLOCK *, ROW *, WERD *, C_BLOB *)) 
{
  BLOCK_IT block_it(block_list);
  BLOCK *block;
  ROW_IT row_it;
  ROW *row;
  WERD_IT word_it;
  WERD *word;
  PBLOB_IT blob_it;
  PBLOB *blob;
  C_BLOB_IT c_blob_it;
  C_BLOB *c_blob;

  for (block_it.mark_cycle_pt (); !block_it.cycled_list (); block_it.forward ()) {
    block = block_it.data ();
    if (block->bounding_box ().overlap (selection_box)) {
      row_it.set_to_list (block->row_list ());
      for (row_it.mark_cycle_pt (); !row_it.cycled_list (); row_it.forward ()) {
        row = row_it.data ();
        if (row->bounding_box ().overlap (selection_box)) {
          word_it.set_to_list (row->word_list ());
          for (word_it.mark_cycle_pt (); !word_it.cycled_list (); word_it.forward ()) {
            word = word_it.data ();
            if (word->bounding_box ().overlap (selection_box)) {
              if (word->flag (W_POLYGON)) {
                if (blob_processor != NULL) {
                  blob_it.set_to_list (word->blob_list ());
                  for (blob_it.mark_cycle_pt (); !blob_it.cycled_list (); blob_it.forward ()) {
                    blob = blob_it.data ();
                    if (blob->bounding_box().overlap (selection_box)) {
                      if (!blob_processor(block, row, word, blob) || selection_quit)
                        return;
                    }
                  }
                }
              }
              else {
                if (c_blob_processor != NULL) {
                  c_blob_it.set_to_list (word->cblob_list ());
                  for (c_blob_it.mark_cycle_pt (); !c_blob_it.cycled_list (); c_blob_it.forward ()) {
                    c_blob = c_blob_it.data ();
                    if (c_blob->bounding_box ().overlap (selection_box)) {
                      if (!c_blob_processor(block, row, word, c_blob) || selection_quit)
                        return;
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
}


/**
 * process_all_words()
 *
 * Walk the current block list applying the specified word processor function
 * to all words
 */
void
process_all_words (BLOCK_LIST * block_list,
                   BOOL8 word_processor (BLOCK *, ROW *, WERD *)) 
{
  BLOCK_IT block_it(block_list);
  BLOCK *block;
  ROW_IT row_it;
  ROW *row;
  WERD_IT word_it;
  WERD *word;

  for (block_it.mark_cycle_pt (); !block_it.cycled_list (); block_it.forward ()) {
    block = block_it.data ();
    row_it.set_to_list (block->row_list ());
    for (row_it.mark_cycle_pt (); !row_it.cycled_list (); row_it.forward ()) {
      row = row_it.data ();
      word_it.set_to_list (row->word_list ());
      for (word_it.mark_cycle_pt (); !word_it.cycled_list (); word_it.forward ()) {
        word = word_it.data ();
        if (!word_processor (block, row, word) || selection_quit)
          return;
      }
    }
  }
}


/**
 * process_selected_words()
 *
 * Walk the current block list applying the specified word processor function
 * to each word selected.
 */

void
process_selected_words (BLOCK_LIST * block_list,
                        TBOX & selection_box,
                        BOOL8 word_processor (BLOCK *, ROW *, WERD *)) 
{
  BLOCK_IT block_it(block_list);
  BLOCK *block;
  ROW_IT row_it;
  ROW *row;
  WERD_IT word_it;
  WERD *word;

  for (block_it.mark_cycle_pt (); !block_it.cycled_list (); block_it.forward ()) {
    block = block_it.data ();
    if (block->bounding_box ().overlap (selection_box)) {
      row_it.set_to_list (block->row_list ());
      for (row_it.mark_cycle_pt (); !row_it.cycled_list (); row_it.forward ()) {
        row = row_it.data ();
        if (row->bounding_box ().overlap (selection_box)) {
          word_it.set_to_list (row->word_list ());
          for (word_it.mark_cycle_pt (); !word_it.cycled_list (); word_it.forward ()) {
            word = word_it.data ();
            if (word->bounding_box ().overlap (selection_box)) {
              if (!word_processor (block, row, word) || selection_quit)
                return;
            }
          }
        }
      }
    }
  }
}

namespace tesseract {
void
Tesseract::process_selected_words (BLOCK_LIST * block_list,
                                   TBOX & selection_box,
                                   BOOL8 (tesseract::Tesseract::*word_processor) (BLOCK *, ROW *, WERD *)) 
{
  BLOCK_IT block_it(block_list);
  BLOCK *block;
  ROW_IT row_it;
  ROW *row;
  WERD_IT word_it;
  WERD *word;

  for (block_it.mark_cycle_pt (); !block_it.cycled_list (); block_it.forward ()) {
    block = block_it.data ();
    if (block->bounding_box ().overlap (selection_box)) {
      row_it.set_to_list (block->row_list ());
      for (row_it.mark_cycle_pt (); !row_it.cycled_list (); row_it.forward ()) {
        row = row_it.data ();
        if (row->bounding_box ().overlap (selection_box)) {
          word_it.set_to_list (row->word_list ());
          for (word_it.mark_cycle_pt (); !word_it.cycled_list (); word_it.forward ()) {
            word = word_it.data ();
            if (word->bounding_box ().overlap (selection_box)) {
              if (!((this->*word_processor) (block, row, word)) || selection_quit)
                return;
            }
          }
        }
      }
    }
  }
}
}  // namespace tesseract


/**
 * process_all_words_it()   PASS ITERATORS
 *
 * Walk the current block list applying the specified word processor function
 * to all words
 */

void
process_all_words_it (BLOCK_LIST * block_list,
                      BOOL8 word_processor (BLOCK *, ROW *, WERD *, BLOCK_IT &, ROW_IT &, WERD_IT &)) 
{
  BLOCK_IT block_it(block_list);
  BLOCK *block;
  ROW_IT row_it;
  ROW *row;
  WERD_IT word_it;
  WERD *word;

  for (block_it.mark_cycle_pt (); !block_it.cycled_list (); block_it.forward ()) {
    block = block_it.data ();
    row_it.set_to_list (block->row_list ());
    for (row_it.mark_cycle_pt (); !row_it.cycled_list (); row_it.forward ()) {
      row = row_it.data ();
      word_it.set_to_list (row->word_list ());
      for (word_it.mark_cycle_pt (); !word_it.cycled_list (); word_it.forward ()) {
        word = word_it.data ();
        if (!word_processor (block, row, word, block_it, row_it, word_it) || selection_quit)
          return;
      }
    }
  }
}


/**
 * process_selected_words_it()   PASS ITERATORS
 *
 * Walk the current block list applying the specified word processor function
 * to each word selected.
 */

void
process_selected_words_it (BLOCK_LIST * block_list,
                           TBOX & selection_box, 
                           BOOL8 word_processor (BLOCK *, ROW *, WERD *, BLOCK_IT &, ROW_IT &, WERD_IT &)) 
{
  BLOCK_IT block_it(block_list);
  BLOCK *block;
  ROW_IT row_it;
  ROW *row;
  WERD_IT word_it;
  WERD *word;

  for (block_it.mark_cycle_pt (); !block_it.cycled_list (); block_it.forward ()) {
    block = block_it.data ();
    if (block->bounding_box ().overlap (selection_box)) {
      row_it.set_to_list (block->row_list ());
      for (row_it.mark_cycle_pt (); !row_it.cycled_list (); row_it.forward ()) {
        row = row_it.data ();
        if (row->bounding_box ().overlap (selection_box)) {
          word_it.set_to_list (row->word_list ());
          for (word_it.mark_cycle_pt (); !word_it.cycled_list (); word_it.forward ()) {
            word = word_it.data ();
            if (word->bounding_box ().overlap (selection_box)) {
              if (!word_processor (block, row, word, block_it, row_it, word_it) || selection_quit)
                return;
            }
          }
        }
      }
    }
  }
}


/**
 * process_all_blocks()
 *
 * Walk the current block list applying the specified block processor function
 * to each block.
 */

void
process_all_blocks (BLOCK_LIST * block_list,
                    BOOL8 block_processor (BLOCK *)) 
{
  BLOCK_IT block_it(block_list);
  BLOCK *block;

  for (block_it.mark_cycle_pt (); !block_it.cycled_list (); block_it.forward ()) {
    block = block_it.data ();
    if (!block_processor (block) || selection_quit)
      return;
  }
}


/**
 * process_selected_blocks()
 *
 * Walk the current block list applying the specified block processor function
 * to each block selected.
 */

void
process_selected_blocks (BLOCK_LIST * block_list,
                         TBOX & selection_box, 
                         BOOL8 block_processor (BLOCK *)) 
{
  BLOCK_IT block_it(block_list);
  BLOCK *block;

  for (block_it.mark_cycle_pt (); !block_it.cycled_list (); block_it.forward ()) {
    block = block_it.data ();
    if (block->bounding_box ().overlap (selection_box)) {
      if (!block_processor (block) || selection_quit)
        return;
    }
  }
}


/**
 * process_all_rows()
 *
 * Walk the current block list applying the specified row processor function
 * to all rows
 */

void
process_all_rows (BLOCK_LIST * block_list,
                  BOOL8 row_processor (BLOCK *, ROW *)) 
{
  BLOCK_IT block_it(block_list);
  BLOCK *block;
  ROW_IT row_it;
  ROW *row;

  for (block_it.mark_cycle_pt (); !block_it.cycled_list (); block_it.forward ()) {
    block = block_it.data ();
    row_it.set_to_list (block->row_list ());
    for (row_it.mark_cycle_pt (); !row_it.cycled_list (); row_it.forward ()) {
      row = row_it.data ();
      if (!row_processor (block, row) || selection_quit)
        return;
    }
  }
}


/**
 * process_selected_rows()
 *
 * Walk the current block list applying the specified row processor function
 * to each row selected.
 */

void
process_selected_rows (BLOCK_LIST * block_list,
                       TBOX & selection_box, 
                       BOOL8 row_processor (BLOCK *, ROW *)) 
{
  BLOCK_IT block_it(block_list);
  BLOCK *block;
  ROW_IT row_it;
  ROW *row;

  for (block_it.mark_cycle_pt (); !block_it.cycled_list (); block_it.forward ()) {
    block = block_it.data ();
    if (block->bounding_box ().overlap (selection_box)) {
      row_it.set_to_list (block->row_list ());
      for (row_it.mark_cycle_pt (); !row_it.cycled_list (); row_it.forward ()) {
        row = row_it.data ();
        if (row->bounding_box ().overlap (selection_box)) {
          if (!row_processor (block, row) || selection_quit)
            return;
        }
      }
    }
  }
}


/**
 * process_all_rows_it()   PASS ITERATORS
 *
 * Walk the current block list applying the specified row processor function
 * to all rows
 */

void
process_all_rows_it (BLOCK_LIST * block_list,
                     BOOL8 row_processor (BLOCK *, ROW *, BLOCK_IT &, ROW_IT &)) 
{
  BLOCK_IT block_it(block_list);
  BLOCK *block;
  ROW_IT row_it;
  ROW *row;

  for (block_it.mark_cycle_pt (); !block_it.cycled_list (); block_it.forward ()) {
    block = block_it.data ();
    row_it.set_to_list (block->row_list ());
    for (row_it.mark_cycle_pt (); !row_it.cycled_list (); row_it.forward ()) {
      row = row_it.data ();
      if (!row_processor (block, row, block_it, row_it) || selection_quit)
        return;
    }
  }
}


/**
 * process_selected_rows_it()   PASS ITERATORS
 *
 * Walk the current block list applying the specified row processor function
 * to each row selected.
 */

void
process_selected_rows_it (BLOCK_LIST * block_list,
                          TBOX & selection_box, 
                          BOOL8 row_processor (BLOCK *, ROW *, BLOCK_IT &, ROW_IT &)) 
{
  BLOCK_IT block_it(block_list);
  BLOCK *block;
  ROW_IT row_it;
  ROW *row;

  for (block_it.mark_cycle_pt (); !block_it.cycled_list (); block_it.forward ()) {
    block = block_it.data ();
    if (block->bounding_box ().overlap (selection_box)) {
      row_it.set_to_list (block->row_list ());
      for (row_it.mark_cycle_pt (); !row_it.cycled_list (); row_it.forward ()) {
        row = row_it.data ();
        if (row->bounding_box ().overlap (selection_box)) {
          if (!row_processor (block, row, block_it, row_it) || selection_quit)
            return;
        }
      }
    }
  }
}
