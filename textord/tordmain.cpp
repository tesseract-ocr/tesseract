/**********************************************************************
 * File:        tordmain.cpp  (Formerly textordp.c)
 * Description: C++ top level textord code.
 * Author:                  Ray Smith
 * Created:                 Tue Jul 28 17:12:33 BST 1992
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

#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif

#ifdef __UNIX__
#include <assert.h>
#endif
#include "stderr.h"
#include "globaloc.h"
#include "blread.h"
#include "blobbox.h"
#include "ccstruct.h"
#include "edgblob.h"
#include "drawtord.h"
#include "makerow.h"
#include "wordseg.h"
#include "textord.h"
#include "tordmain.h"

#include "allheaders.h"

const ERRCODE BLOCKLESS_BLOBS = "Warning:some blobs assigned to no block";

#undef EXTERN
#define EXTERN

#define MAX_NEAREST_DIST  600    //for block skew stats

/**********************************************************************
 * SetBlobStrokeWidth
 *
 * Set the horizontal and vertical stroke widths in the blob.
 **********************************************************************/
void SetBlobStrokeWidth(Pix* pix, BLOBNBOX* blob) {
  // Cut the blob rectangle into a Pix.
  int pix_height = pixGetHeight(pix);
  const TBOX& box = blob->bounding_box();
  int width = box.width();
  int height = box.height();
  Box* blob_pix_box = boxCreate(box.left(), pix_height - box.top(),
                                width, height);
  Pix* pix_blob = pixClipRectangle(pix, blob_pix_box, NULL);
  boxDestroy(&blob_pix_box);
  Pix* dist_pix = pixDistanceFunction(pix_blob, 4, 8, L_BOUNDARY_BG);
  pixDestroy(&pix_blob);
  // Compute the stroke widths.
  uinT32* data = pixGetData(dist_pix);
  int wpl = pixGetWpl(dist_pix);
  // Horizontal width of stroke.
  STATS h_stats(0, width + 1);
  for (int y = 0; y < height; ++y) {
    uinT32* pixels = data + y*wpl;
    int prev_pixel = 0;
    int pixel = GET_DATA_BYTE(pixels, 0);
    for (int x = 1; x < width; ++x) {
      int next_pixel = GET_DATA_BYTE(pixels, x);
      // We are looking for a pixel that is equal to its vertical neighbours,
      // yet greater than its left neighbour.
      if (prev_pixel < pixel &&
          (y == 0 || pixel == GET_DATA_BYTE(pixels - wpl, x - 1)) &&
          (y == height - 1 || pixel == GET_DATA_BYTE(pixels + wpl, x - 1))) {
        if (pixel > next_pixel) {
          // Single local max, so an odd width.
          h_stats.add(pixel * 2 - 1, 1);
        } else if (pixel == next_pixel && x + 1 < width &&
                 pixel > GET_DATA_BYTE(pixels, x + 1)) {
          // Double local max, so an even width.
          h_stats.add(pixel * 2, 1);
        }
      }
      prev_pixel = pixel;
      pixel = next_pixel;
    }
  }
  // Vertical width of stroke.
  STATS v_stats(0, height + 1);
  for (int x = 0; x < width; ++x) {
    int prev_pixel = 0;
    int pixel = GET_DATA_BYTE(data, x);
    for (int y = 1; y < height; ++y) {
      uinT32* pixels = data + y*wpl;
      int next_pixel = GET_DATA_BYTE(pixels, x);
      // We are looking for a pixel that is equal to its horizontal neighbours,
      // yet greater than its upper neighbour.
      if (prev_pixel < pixel &&
          (x == 0 || pixel == GET_DATA_BYTE(pixels - wpl, x - 1)) &&
          (x == width - 1 || pixel == GET_DATA_BYTE(pixels - wpl, x + 1))) {
        if (pixel > next_pixel) {
          // Single local max, so an odd width.
          v_stats.add(pixel * 2 - 1, 1);
        } else if (pixel == next_pixel && y + 1 < height &&
                 pixel > GET_DATA_BYTE(pixels + wpl, x)) {
          // Double local max, so an even width.
          v_stats.add(pixel * 2, 1);
        }
      }
      prev_pixel = pixel;
      pixel = next_pixel;
    }
  }
  pixDestroy(&dist_pix);
  // Store the horizontal and vertical width in the blob, keeping both
  // widths if there is enough information, otherwse only the one with
  // the most samples.
  // If there are insufficent samples, store zero, rather than using
  // 2*area/perimeter, as the numbers that gives do not match the numbers
  // from the distance method.
  if (h_stats.get_total() >= (width + height) / 4) {
    blob->set_horz_stroke_width(h_stats.ile(0.5f));
    if (v_stats.get_total() >= (width + height) / 4)
      blob->set_vert_stroke_width(v_stats.ile(0.5f));
    else
      blob->set_vert_stroke_width(0.0f);
  } else {
    if (v_stats.get_total() >= (width + height) / 4 ||
        v_stats.get_total() > h_stats.get_total()) {
      blob->set_horz_stroke_width(0.0f);
      blob->set_vert_stroke_width(v_stats.ile(0.5f));
    } else {
      blob->set_horz_stroke_width(h_stats.get_total() > 2 ? h_stats.ile(0.5f)
                                                          : 0.0f);
      blob->set_vert_stroke_width(0.0f);
    }
  }
}


/**********************************************************************
 * assign_blobs_to_blocks2
 *
 * Make a list of TO_BLOCKs for portrait and landscape orientation.
 **********************************************************************/

void assign_blobs_to_blocks2(Pix* pix,
                             BLOCK_LIST *blocks,          // blocks to process
                             TO_BLOCK_LIST *port_blocks) {  // output list
  BLOCK *block;                  // current block
  BLOBNBOX *newblob;             // created blob
  C_BLOB *blob;                  // current blob
  BLOCK_IT block_it = blocks;
  C_BLOB_IT blob_it;             // iterator
  BLOBNBOX_IT port_box_it;       // iterator
                                 // destination iterator
  TO_BLOCK_IT port_block_it = port_blocks;
  TO_BLOCK *port_block;          // created block

  for (block_it.mark_cycle_pt(); !block_it.cycled_list(); block_it.forward()) {
    block = block_it.data();
    port_block = new TO_BLOCK(block);

    // Convert the good outlines to block->blob_list
    port_box_it.set_to_list(&port_block->blobs);
    blob_it.set_to_list(block->blob_list());
    for (blob_it.mark_cycle_pt(); !blob_it.cycled_list(); blob_it.forward()) {
      blob = blob_it.extract();
      newblob = new BLOBNBOX(blob);  // Convert blob to BLOBNBOX.
      SetBlobStrokeWidth(pix, newblob);
      port_box_it.add_after_then_move(newblob);
    }

    // Put the rejected outlines in block->noise_blobs, which allows them to
    // be reconsidered and sorted back into rows and recover outlines mistakenly
    // rejected.
    port_box_it.set_to_list(&port_block->noise_blobs);
    blob_it.set_to_list(block->reject_blobs());
    for (blob_it.mark_cycle_pt(); !blob_it.cycled_list(); blob_it.forward()) {
      blob = blob_it.extract();
      newblob = new BLOBNBOX(blob);  // Convert blob to BLOBNBOX.
      SetBlobStrokeWidth(pix, newblob);
      port_box_it.add_after_then_move(newblob);
    }

    port_block_it.add_after_then_move(port_block);
  }
}

namespace tesseract {
/**********************************************************************
 * find_components
 *
 * Find the C_OUTLINEs of the connected components in each block, put them
 * in C_BLOBs, and filter them by size, putting the different size
 * grades on different lists in the matching TO_BLOCK in to_blocks.
 **********************************************************************/

void Textord::find_components(Pix* pix, BLOCK_LIST *blocks,
                              TO_BLOCK_LIST *to_blocks) {
  int width = pixGetWidth(pix);
  int height = pixGetHeight(pix);
  if (width > MAX_INT16 || height > MAX_INT16) {
    tprintf("Input image too large! (%d, %d)\n", width, height);
    return;  // Can't handle it.
  }

  set_global_loc_code(LOC_EDGE_PROG);

  BLOCK_IT block_it(blocks);    // iterator
  for (block_it.mark_cycle_pt(); !block_it.cycled_list();
       block_it.forward()) {
    BLOCK* block = block_it.data();
    if (block->poly_block() == NULL || block->poly_block()->IsText()) {
      extract_edges(pix, block);
    }
  }

  assign_blobs_to_blocks2(pix, blocks, to_blocks);
  ICOORD page_tr(width, height);
  filter_blobs(page_tr, to_blocks, !textord_test_landscape);
}

/**********************************************************************
 * filter_blobs
 *
 * Sort the blobs into sizes in all the blocks for later work.
 **********************************************************************/

void Textord::filter_blobs(ICOORD page_tr,         // top right
                           TO_BLOCK_LIST *blocks,  // output list
                           BOOL8 testing_on) {     // for plotting
  TO_BLOCK_IT block_it = blocks;          // destination iterator
  TO_BLOCK *block;                        // created block

  #ifndef GRAPHICS_DISABLED
  if (to_win != NULL)
    to_win->Clear();
  #endif  // GRAPHICS_DISABLED

  for (block_it.mark_cycle_pt(); !block_it.cycled_list();
       block_it.forward()) {
    block = block_it.data();
    block->line_size = filter_noise_blobs(&block->blobs,
      &block->noise_blobs,
      &block->small_blobs,
      &block->large_blobs);
    block->line_spacing = block->line_size *
        (tesseract::CCStruct::kDescenderFraction +
         tesseract::CCStruct::kXHeightFraction +
         2 * tesseract::CCStruct::kAscenderFraction) /
         tesseract::CCStruct::kXHeightFraction;
    block->line_size *= textord_min_linesize;
    block->max_blob_size = block->line_size * textord_excess_blobsize;

    #ifndef GRAPHICS_DISABLED
    if (textord_show_blobs && testing_on) {
      if (to_win == NULL)
        create_to_win(page_tr);
      block->plot_graded_blobs(to_win);
    }
    if (textord_show_boxes && testing_on) {
      if (to_win == NULL)
        create_to_win(page_tr);
      plot_box_list(to_win, &block->noise_blobs, ScrollView::WHITE);
      plot_box_list(to_win, &block->small_blobs, ScrollView::WHITE);
      plot_box_list(to_win, &block->large_blobs, ScrollView::WHITE);
      plot_box_list(to_win, &block->blobs, ScrollView::WHITE);
    }
    #endif  // GRAPHICS_DISABLED
  }
}

/**********************************************************************
 * filter_noise_blobs
 *
 * Move small blobs to a separate list.
 **********************************************************************/

float Textord::filter_noise_blobs(
    BLOBNBOX_LIST *src_list,      // original list
    BLOBNBOX_LIST *noise_list,    // noise list
    BLOBNBOX_LIST *small_list,    // small blobs
    BLOBNBOX_LIST *large_list) {  // large blobs
  inT16 height;                  //height of blob
  inT16 width;                   //of blob
  BLOBNBOX *blob;                //current blob
  float initial_x;               //first guess
  BLOBNBOX_IT src_it = src_list; //iterators
  BLOBNBOX_IT noise_it = noise_list;
  BLOBNBOX_IT small_it = small_list;
  BLOBNBOX_IT large_it = large_list;
  STATS size_stats (0, MAX_NEAREST_DIST);
  //blob heights
  float min_y;                   //size limits
  float max_y;
  float max_x;
  float max_height;              //of good blobs

  for (src_it.mark_cycle_pt(); !src_it.cycled_list(); src_it.forward()) {
    blob = src_it.data();
    if (blob->bounding_box().height() < textord_max_noise_size)
      noise_it.add_after_then_move(src_it.extract());
    else if (blob->enclosed_area() >= blob->bounding_box().height()
      * blob->bounding_box().width() * textord_noise_area_ratio)
      small_it.add_after_then_move(src_it.extract());
  }
  for (src_it.mark_cycle_pt(); !src_it.cycled_list(); src_it.forward()) {
    size_stats.add(src_it.data()->bounding_box().height(), 1);
  }
  initial_x = size_stats.ile(textord_initialx_ile);
  max_y = ceil(initial_x *
               (tesseract::CCStruct::kDescenderFraction +
                tesseract::CCStruct::kXHeightFraction +
                2 * tesseract::CCStruct::kAscenderFraction) /
               tesseract::CCStruct::kXHeightFraction);
  min_y = floor (initial_x / 2);
  max_x = ceil (initial_x * textord_width_limit);
  small_it.move_to_first ();
  for (small_it.mark_cycle_pt (); !small_it.cycled_list ();
  small_it.forward ()) {
    height = small_it.data()->bounding_box().height();
    if (height > max_y)
      large_it.add_after_then_move(small_it.extract ());
    else if (height >= min_y)
      src_it.add_after_then_move(small_it.extract ());
  }
  size_stats.clear ();
  for (src_it.mark_cycle_pt (); !src_it.cycled_list (); src_it.forward ()) {
    height = src_it.data ()->bounding_box ().height ();
    width = src_it.data ()->bounding_box ().width ();
    if (height < min_y)
      small_it.add_after_then_move (src_it.extract ());
    else if (height > max_y || width > max_x)
      large_it.add_after_then_move (src_it.extract ());
    else
      size_stats.add (height, 1);
  }
  max_height = size_stats.ile (textord_initialasc_ile);
  //      tprintf("max_y=%g, min_y=%g, initial_x=%g, max_height=%g,",
  //              max_y,min_y,initial_x,max_height);
  max_height *= tesseract::CCStruct::kXHeightCapRatio;
  if (max_height > initial_x)
    initial_x = max_height;
  //      tprintf(" ret=%g\n",initial_x);
  return initial_x;
}

// Fixes the block so it obeys all the rules:
// Must have at least one ROW.
// Must have at least one WERD.
// WERDs contain a fake blob.
void Textord::cleanup_nontext_block(BLOCK* block) {
  // Non-text blocks must contain at least one row.
  ROW_IT row_it(block->row_list());
  if (row_it.empty()) {
    float height = block->bounding_box().height();
    inT32 zero = 0;
    ROW* row = new ROW(0, &zero, NULL, height / 2.0f, height / 4.0f,
                       height / 4.0f, 0, 1);
    row_it.add_after_then_move(row);
  }
  // Each row must contain at least one word.
  for (row_it.mark_cycle_pt(); !row_it.cycled_list(); row_it.forward()) {
    ROW* row = row_it.data();
    WERD_IT w_it(row->word_list());
    if (w_it.empty()) {
      // Make a fake blob to put in the word.
      TBOX box = block->row_list()->singleton() ? block->bounding_box()
                                                : row->bounding_box();
      C_BLOB* blob = C_BLOB::FakeBlob(box);
      C_BLOB_LIST blobs;
      C_BLOB_IT blob_it(&blobs);
      blob_it.add_after_then_move(blob);
      WERD* word = new WERD(&blobs, 0, NULL);
      w_it.add_after_then_move(word);
    }
    // Each word must contain a fake blob.
    for (w_it.mark_cycle_pt(); !w_it.cycled_list(); w_it.forward()) {
      WERD* word = w_it.data();
      // Just assert that this is true, as it would be useful to find
      // out why it isn't.
      ASSERT_HOST(!word->cblob_list()->empty());
    }
    row->recalc_bounding_box();
  }
}

/**********************************************************************
 * cleanup_blocks
 *
 * Delete empty blocks, rows from the page.
 **********************************************************************/

void Textord::cleanup_blocks(                    //remove empties
                             BLOCK_LIST *blocks  //list
                            ) {
  BLOCK_IT block_it = blocks;    //iterator
  ROW_IT row_it;                 //row iterator

  int num_rows = 0;
  int num_rows_all = 0;
  int num_blocks = 0;
  int num_blocks_all = 0;
  for (block_it.mark_cycle_pt(); !block_it.cycled_list();
       block_it.forward()) {
    BLOCK* block = block_it.data();
    if (block->poly_block() != NULL && !block->poly_block()->IsText()) {
      cleanup_nontext_block(block);
      continue;
    }
    num_rows = 0;
    num_rows_all = 0;
    row_it.set_to_list(block->row_list());
    for (row_it.mark_cycle_pt(); !row_it.cycled_list(); row_it.forward()) {
      ++num_rows_all;
      clean_small_noise_from_words(row_it.data());
      if ((textord_noise_rejrows && !row_it.data()->word_list()->empty() &&
           clean_noise_from_row(row_it.data())) ||
          row_it.data()->word_list()->empty()) {
        delete row_it.extract();  // lose empty row.
      } else {
        if (textord_noise_rejwords)
          clean_noise_from_words(row_it.data());
        if (textord_blshift_maxshift >= 0)
          tweak_row_baseline(row_it.data(),
                             textord_blshift_maxshift,
                             textord_blshift_xfraction);
        ++num_rows;
      }
    }
    if (block->row_list()->empty()) {
      delete block_it.extract();  // Lose empty text blocks.
    } else {
      ++num_blocks;
    }
    ++num_blocks_all;
    if (textord_noise_debug)
      tprintf("cleanup_blocks: # rows = %d / %d\n", num_rows, num_rows_all);
  }
  if (textord_noise_debug)
    tprintf("cleanup_blocks: # blocks = %d / %d\n", num_blocks, num_blocks_all);
}


/**********************************************************************
 * clean_noise_from_row
 *
 * Move blobs of words from rows of garbage into the reject blobs list.
 **********************************************************************/

BOOL8 Textord::clean_noise_from_row(          //remove empties
                                    ROW *row  //row to clean
                                   ) {
  BOOL8 testing_on;
  TBOX blob_box;                  //bounding box
  C_BLOB *blob;                  //current blob
  C_OUTLINE *outline;            //current outline
  WERD *word;                    //current word
  inT32 blob_size;               //biggest size
  inT32 trans_count = 0;         //no of transitions
  inT32 trans_threshold;         //noise tolerance
  inT32 dot_count;               //small objects
  inT32 norm_count;              //normal objects
  inT32 super_norm_count;        //real char-like
                                 //words of row
  WERD_IT word_it = row->word_list ();
  C_BLOB_IT blob_it;             //blob iterator
  C_OUTLINE_IT out_it;           //outline iterator

  if (textord_test_y > row->base_line (textord_test_x)
    && textord_show_blobs
    && textord_test_y < row->base_line (textord_test_x) + row->x_height ())
    testing_on = TRUE;
  else
    testing_on = FALSE;
  dot_count = 0;
  norm_count = 0;
  super_norm_count = 0;
  for (word_it.mark_cycle_pt (); !word_it.cycled_list (); word_it.forward ()) {
    word = word_it.data ();      //current word
                                 //blobs in word
    blob_it.set_to_list (word->cblob_list ());
    for (blob_it.mark_cycle_pt (); !blob_it.cycled_list ();
    blob_it.forward ()) {
      blob = blob_it.data ();
      if (!word->flag (W_DONT_CHOP)) {
                                 //get outlines
        out_it.set_to_list (blob->out_list ());
        for (out_it.mark_cycle_pt (); !out_it.cycled_list ();
        out_it.forward ()) {
          outline = out_it.data ();
          blob_box = outline->bounding_box ();
          blob_size =
            blob_box.width () >
            blob_box.height ()? blob_box.width () : blob_box.
            height();
          if (blob_size < textord_noise_sizelimit * row->x_height ())
            dot_count++;         //count smal outlines
          if (!outline->child ()->empty ()
            && blob_box.height () <
            (1 + textord_noise_syfract) * row->x_height ()
            && blob_box.height () >
            (1 - textord_noise_syfract) * row->x_height ()
            && blob_box.width () <
            (1 + textord_noise_sxfract) * row->x_height ()
            && blob_box.width () >
            (1 - textord_noise_sxfract) * row->x_height ())
            super_norm_count++;  //count smal outlines
        }
      }
      else
        super_norm_count++;
      blob_box = blob->bounding_box ();
      blob_size =
        blob_box.width () >
        blob_box.height ()? blob_box.width () : blob_box.height ();
      if (blob_size >= textord_noise_sizelimit * row->x_height ()
          && blob_size < row->x_height () * 2) {
        trans_threshold = blob_size / textord_noise_sizefraction;
        trans_count = blob->count_transitions (trans_threshold);
        if (trans_count < textord_noise_translimit)
          norm_count++;
      }
      else if (blob_box.height () > row->x_height () * 2
        && (!word_it.at_first () || !blob_it.at_first ()))
        dot_count += 2;
      if (testing_on) {
        tprintf
          ("Blob at (%d,%d) -> (%d,%d), ols=%d, tc=%d, bldiff=%g\n",
          blob_box.left (), blob_box.bottom (), blob_box.right (),
          blob_box.top (), blob->out_list ()->length (), trans_count,
          blob_box.bottom () - row->base_line (blob_box.left ()));
      }
    }
  }
  if (textord_noise_debug) {
    tprintf ("Row ending at (%d,%g):",
      blob_box.right (), row->base_line (blob_box.right ()));
    tprintf (" R=%g, dc=%d, nc=%d, %s\n",
      norm_count > 0 ? (float) dot_count / norm_count : 9999,
      dot_count, norm_count,
      dot_count > norm_count * textord_noise_normratio
      && dot_count > 2 ? "REJECTED" : "ACCEPTED");
  }
  return super_norm_count < textord_noise_sncount
    && dot_count > norm_count * textord_noise_rowratio && dot_count > 2;
}

/**********************************************************************
 * clean_noise_from_words
 *
 * Move blobs of words from rows of garbage into the reject blobs list.
 **********************************************************************/

void Textord::clean_noise_from_words(          //remove empties
                                     ROW *row  //row to clean
                                    ) {
  TBOX blob_box;                  //bounding box
  inT8 *word_dud;                //was it chucked
  C_BLOB *blob;                  //current blob
  C_OUTLINE *outline;            //current outline
  WERD *word;                    //current word
  inT32 blob_size;               //biggest size
  inT32 trans_count;             //no of transitions
  inT32 trans_threshold;         //noise tolerance
  inT32 dot_count;               //small objects
  inT32 norm_count;              //normal objects
  inT32 dud_words;               //number discarded
  inT32 ok_words;                //number remaining
  inT32 word_index;              //current word
                                 //words of row
  WERD_IT word_it = row->word_list ();
  C_BLOB_IT blob_it;             //blob iterator
  C_OUTLINE_IT out_it;           //outline iterator

  ok_words = word_it.length ();
  if (ok_words == 0 || textord_no_rejects)
    return;
  word_dud = (inT8 *) alloc_mem (ok_words * sizeof (inT8));
  dud_words = 0;
  ok_words = 0;
  word_index = 0;
  for (word_it.mark_cycle_pt (); !word_it.cycled_list (); word_it.forward ()) {
    word = word_it.data ();      //current word
    dot_count = 0;
    norm_count = 0;
                                 //blobs in word
    blob_it.set_to_list (word->cblob_list ());
    for (blob_it.mark_cycle_pt (); !blob_it.cycled_list ();
    blob_it.forward ()) {
      blob = blob_it.data ();
      if (!word->flag (W_DONT_CHOP)) {
                                 //get outlines
        out_it.set_to_list (blob->out_list ());
        for (out_it.mark_cycle_pt (); !out_it.cycled_list ();
        out_it.forward ()) {
          outline = out_it.data ();
          blob_box = outline->bounding_box ();
          blob_size =
            blob_box.width () >
            blob_box.height ()? blob_box.width () : blob_box.
            height();
          if (blob_size < textord_noise_sizelimit * row->x_height ())
            dot_count++;         //count smal outlines
          if (!outline->child ()->empty ()
            && blob_box.height () <
            (1 + textord_noise_syfract) * row->x_height ()
            && blob_box.height () >
            (1 - textord_noise_syfract) * row->x_height ()
            && blob_box.width () <
            (1 + textord_noise_sxfract) * row->x_height ()
            && blob_box.width () >
            (1 - textord_noise_sxfract) * row->x_height ())
            norm_count++;        //count smal outlines
        }
      }
      else
        norm_count++;
      blob_box = blob->bounding_box ();
      blob_size =
        blob_box.width () >
        blob_box.height ()? blob_box.width () : blob_box.height ();
      if (blob_size >= textord_noise_sizelimit * row->x_height ()
      && blob_size < row->x_height () * 2) {
        trans_threshold = blob_size / textord_noise_sizefraction;
        trans_count = blob->count_transitions (trans_threshold);
        if (trans_count < textord_noise_translimit)
          norm_count++;
      }
      else if (blob_box.height () > row->x_height () * 2
        && (!word_it.at_first () || !blob_it.at_first ()))
        dot_count += 2;
    }
    if (dot_count > 2) {
      if (dot_count > norm_count * textord_noise_normratio * 2)
        word_dud[word_index] = 2;
      else if (dot_count > norm_count * textord_noise_normratio)
        word_dud[word_index] = 1;
      else
        word_dud[word_index] = 0;
    }
    else
      word_dud[word_index] = 0;
    if (word_dud[word_index] == 2)
      dud_words++;
    else
      ok_words++;
    word_index++;
  }

  word_index = 0;
  for (word_it.mark_cycle_pt (); !word_it.cycled_list (); word_it.forward ()) {
    if (word_dud[word_index] == 2
    || (word_dud[word_index] == 1 && dud_words > ok_words)) {
      word = word_it.data ();    //current word
                                 //rejected blobs
      blob_it.set_to_list (word->rej_cblob_list ());
                                 //move from blobs
      blob_it.add_list_after (word->cblob_list ());
    }
    word_index++;
  }
  free_mem(word_dud);
}

// Remove outlines that are a tiny fraction in either width or height
// of the word height.
void Textord::clean_small_noise_from_words(ROW *row) {
  WERD_IT word_it(row->word_list());
  for (word_it.mark_cycle_pt(); !word_it.cycled_list(); word_it.forward()) {
    WERD* word = word_it.data();
    int min_size = static_cast<int>(
      textord_noise_hfract * word->bounding_box().height() + 0.5);
    C_BLOB_IT blob_it(word->cblob_list());
    for (blob_it.mark_cycle_pt(); !blob_it.cycled_list(); blob_it.forward()) {
      C_BLOB* blob = blob_it.data();
      C_OUTLINE_IT out_it(blob->out_list());
      for (out_it.mark_cycle_pt(); !out_it.cycled_list(); out_it.forward()) {
        C_OUTLINE* outline = out_it.data();
        outline->RemoveSmallRecursive(min_size, &out_it);
      }
      if (blob->out_list()->empty()) {
        delete blob_it.extract();
      }
    }
    if (word->cblob_list()->empty()) {
      if (!word_it.at_last()) {
        // The next word is no longer a fuzzy non space if it was before,
        // since the word before is about to be deleted.
        WERD* next_word = word_it.data_relative(1);
        if (next_word->flag(W_FUZZY_NON)) {
          next_word->set_flag(W_FUZZY_NON, false);
        }
      }
      delete word_it.extract();
    }
  }
}
}  // tesseract

/**********************************************************************
 * tweak_row_baseline
 *
 * Shift baseline to fit the blobs more accurately where they are
 * close enough.
 **********************************************************************/

void tweak_row_baseline(ROW *row,
                        double blshift_maxshift,
                        double blshift_xfraction) {
  TBOX blob_box;                 //bounding box
  C_BLOB *blob;                  //current blob
  WERD *word;                    //current word
  inT32 blob_count;              //no of blobs
  inT32 src_index;               //source segment
  inT32 dest_index;              //destination segment
  inT32 *xstarts;                //spline segments
  double *coeffs;                //spline coeffs
  float ydiff;                   //baseline error
  float x_centre;                //centre of blob
                                 //words of row
  WERD_IT word_it = row->word_list ();
  C_BLOB_IT blob_it;             //blob iterator

  blob_count = 0;
  for (word_it.mark_cycle_pt (); !word_it.cycled_list (); word_it.forward ()) {
    word = word_it.data ();      //current word
                                 //get total blobs
    blob_count += word->cblob_list ()->length ();
  }
  if (blob_count == 0)
    return;
  xstarts =
    (inT32 *) alloc_mem ((blob_count + row->baseline.segments + 1) *
    sizeof (inT32));
  coeffs =
    (double *) alloc_mem ((blob_count + row->baseline.segments) * 3 *
    sizeof (double));

  src_index = 0;
  dest_index = 0;
  xstarts[0] = row->baseline.xcoords[0];
  for (word_it.mark_cycle_pt (); !word_it.cycled_list (); word_it.forward ()) {
    word = word_it.data ();      //current word
                                 //blobs in word
    blob_it.set_to_list (word->cblob_list ());
    for (blob_it.mark_cycle_pt (); !blob_it.cycled_list ();
    blob_it.forward ()) {
      blob = blob_it.data ();
      blob_box = blob->bounding_box ();
      x_centre = (blob_box.left () + blob_box.right ()) / 2.0;
      ydiff = blob_box.bottom () - row->base_line (x_centre);
      if (ydiff < 0)
        ydiff = -ydiff / row->x_height ();
      else
        ydiff = ydiff / row->x_height ();
      if (ydiff < blshift_maxshift
        && blob_box.height () / row->x_height () > blshift_xfraction) {
        if (xstarts[dest_index] >= x_centre)
          xstarts[dest_index] = blob_box.left ();
        coeffs[dest_index * 3] = 0;
        coeffs[dest_index * 3 + 1] = 0;
        coeffs[dest_index * 3 + 2] = blob_box.bottom ();
        //shift it
        dest_index++;
        xstarts[dest_index] = blob_box.right () + 1;
      }
      else {
        if (xstarts[dest_index] <= x_centre) {
          while (row->baseline.xcoords[src_index + 1] <= x_centre
          && src_index < row->baseline.segments - 1) {
            if (row->baseline.xcoords[src_index + 1] >
            xstarts[dest_index]) {
              coeffs[dest_index * 3] =
                row->baseline.quadratics[src_index].a;
              coeffs[dest_index * 3 + 1] =
                row->baseline.quadratics[src_index].b;
              coeffs[dest_index * 3 + 2] =
                row->baseline.quadratics[src_index].c;
              dest_index++;
              xstarts[dest_index] =
                row->baseline.xcoords[src_index + 1];
            }
            src_index++;
          }
          coeffs[dest_index * 3] =
            row->baseline.quadratics[src_index].a;
          coeffs[dest_index * 3 + 1] =
            row->baseline.quadratics[src_index].b;
          coeffs[dest_index * 3 + 2] =
            row->baseline.quadratics[src_index].c;
          dest_index++;
          xstarts[dest_index] = row->baseline.xcoords[src_index + 1];
        }
      }
    }
  }
  while (src_index < row->baseline.segments
    && row->baseline.xcoords[src_index + 1] <= xstarts[dest_index])
    src_index++;
  while (src_index < row->baseline.segments) {
    coeffs[dest_index * 3] = row->baseline.quadratics[src_index].a;
    coeffs[dest_index * 3 + 1] = row->baseline.quadratics[src_index].b;
    coeffs[dest_index * 3 + 2] = row->baseline.quadratics[src_index].c;
    dest_index++;
    src_index++;
    xstarts[dest_index] = row->baseline.xcoords[src_index];
  }
                                 //turn to spline
  row->baseline = QSPLINE (dest_index, xstarts, coeffs);
  free_mem(xstarts);
  free_mem(coeffs);
}

/**********************************************************************
 * blob_y_order
 *
 * Sort function to sort blobs in y from page top.
 **********************************************************************/

inT32 blob_y_order(              //sort function
                   void *item1,  //items to compare
                   void *item2) {
                                 //converted ptr
  BLOBNBOX *blob1 = *(BLOBNBOX **) item1;
                                 //converted ptr
  BLOBNBOX *blob2 = *(BLOBNBOX **) item2;

  if (blob1->bounding_box ().bottom () > blob2->bounding_box ().bottom ())
    return -1;
  else if (blob1->bounding_box ().bottom () <
    blob2->bounding_box ().bottom ())
    return 1;
  else {
    if (blob1->bounding_box ().left () < blob2->bounding_box ().left ())
      return -1;
    else if (blob1->bounding_box ().left () >
      blob2->bounding_box ().left ())
      return 1;
    else
      return 0;
  }
}
