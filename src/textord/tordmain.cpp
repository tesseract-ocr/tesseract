/**********************************************************************
 * File:        tordmain.cpp  (Formerly textordp.c)
 * Description: C++ top level textord code.
 * Author:      Ray Smith
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

#define _USE_MATH_DEFINES // for M_PI

#ifdef HAVE_CONFIG_H
#  include "config_auto.h"
#endif

#include "tordmain.h"

#include "arrayaccess.h" // for GET_DATA_BYTE
#include "blobbox.h"     // for BLOBNBOX_IT, BLOBNBOX, TO_BLOCK, TO_B...
#include "ccstruct.h"    // for CCStruct, CCStruct::kXHeightFraction
#include "clst.h"        // for CLISTIZE
#include "coutln.h"      // for C_OUTLINE_IT, C_OUTLINE_LIST, C_OUTLINE
#include "drawtord.h"    // for plot_box_list, to_win, create_to_win
#include "edgblob.h"     // for extract_edges
#include "errcode.h"     // for ASSERT_HOST, ...
#include "makerow.h"     // for textord_test_x, textord_test_y, texto...
#include "ocrblock.h"    // for BLOCK_IT, BLOCK, BLOCK_LIST (ptr only)
#include "ocrrow.h"      // for ROW, ROW_IT, ROW_LIST, tweak_row_base...
#include "params.h"      // for DoubleParam, BoolParam, IntParam
#include "pdblock.h"     // for PDBLK
#include "points.h"      // for FCOORD, ICOORD
#include "polyblk.h"     // for POLY_BLOCK
#include "quadratc.h"    // for QUAD_COEFFS
#include "quspline.h"    // for QSPLINE, tweak_row_baseline
#include "rect.h"        // for TBOX
#include "scrollview.h"  // for ScrollView, ScrollView::WHITE
#include "statistc.h"    // for STATS
#include "stepblob.h"    // for C_BLOB_IT, C_BLOB, C_BLOB_LIST
#include "textord.h"     // for Textord, WordWithBox, WordGrid, WordS...
#include "tprintf.h"     // for tprintf
#include "werd.h"        // for WERD_IT, WERD, WERD_LIST, W_DONT_CHOP

#include <allheaders.h> // for pixDestroy, pixGetHeight, boxCreate

#include <cfloat>  // for FLT_MAX
#include <cmath>   // for ceil, floor, M_PI
#include <cstdint> // for INT16_MAX, uint32_t, int32_t, int16_t
#include <memory>

namespace tesseract {

#define MAX_NEAREST_DIST 600 // for block skew stats

/**********************************************************************
 * SetBlobStrokeWidth
 *
 * Set the horizontal and vertical stroke widths in the blob.
 **********************************************************************/
void SetBlobStrokeWidth(Image pix, BLOBNBOX *blob) {
  // Cut the blob rectangle into a Pix.
  int pix_height = pixGetHeight(pix);
  const TBOX &box = blob->bounding_box();
  int width = box.width();
  int height = box.height();
  Box *blob_pix_box = boxCreate(box.left(), pix_height - box.top(), width, height);
  Image pix_blob = pixClipRectangle(pix, blob_pix_box, nullptr);
  boxDestroy(&blob_pix_box);
  Image dist_pix = pixDistanceFunction(pix_blob, 4, 8, L_BOUNDARY_BG);
  pix_blob.destroy();
  // Compute the stroke widths.
  uint32_t *data = pixGetData(dist_pix);
  int wpl = pixGetWpl(dist_pix);
  // Horizontal width of stroke.
  STATS h_stats(0, width);
  for (int y = 0; y < height; ++y) {
    uint32_t *pixels = data + y * wpl;
    int prev_pixel = 0;
    int pixel = GET_DATA_BYTE(pixels, 0);
    for (int x = 1; x < width; ++x) {
      int next_pixel = GET_DATA_BYTE(pixels, x);
      // We are looking for a pixel that is equal to its vertical neighbours,
      // yet greater than its left neighbour.
      if (prev_pixel < pixel && (y == 0 || pixel == GET_DATA_BYTE(pixels - wpl, x - 1)) &&
          (y == height - 1 || pixel == GET_DATA_BYTE(pixels + wpl, x - 1))) {
        if (pixel > next_pixel) {
          // Single local max, so an odd width.
          h_stats.add(pixel * 2 - 1, 1);
        } else if (pixel == next_pixel && x + 1 < width && pixel > GET_DATA_BYTE(pixels, x + 1)) {
          // Double local max, so an even width.
          h_stats.add(pixel * 2, 1);
        }
      }
      prev_pixel = pixel;
      pixel = next_pixel;
    }
  }
  // Vertical width of stroke.
  STATS v_stats(0, height);
  for (int x = 0; x < width; ++x) {
    int prev_pixel = 0;
    int pixel = GET_DATA_BYTE(data, x);
    for (int y = 1; y < height; ++y) {
      uint32_t *pixels = data + y * wpl;
      int next_pixel = GET_DATA_BYTE(pixels, x);
      // We are looking for a pixel that is equal to its horizontal neighbours,
      // yet greater than its upper neighbour.
      if (prev_pixel < pixel && (x == 0 || pixel == GET_DATA_BYTE(pixels - wpl, x - 1)) &&
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
  dist_pix.destroy();
  // Store the horizontal and vertical width in the blob, keeping both
  // widths if there is enough information, otherwise only the one with
  // the most samples.
  // If there are insufficient samples, store zero, rather than using
  // 2*area/perimeter, as the numbers that gives do not match the numbers
  // from the distance method.
  if (h_stats.get_total() >= (width + height) / 4) {
    blob->set_horz_stroke_width(h_stats.ile(0.5f));
    if (v_stats.get_total() >= (width + height) / 4) {
      blob->set_vert_stroke_width(v_stats.ile(0.5f));
    } else {
      blob->set_vert_stroke_width(0.0f);
    }
  } else {
    if (v_stats.get_total() >= (width + height) / 4 || v_stats.get_total() > h_stats.get_total()) {
      blob->set_horz_stroke_width(0.0f);
      blob->set_vert_stroke_width(v_stats.ile(0.5f));
    } else {
      blob->set_horz_stroke_width(h_stats.get_total() > 2 ? h_stats.ile(0.5f) : 0.0f);
      blob->set_vert_stroke_width(0.0f);
    }
  }
}

/**********************************************************************
 * assign_blobs_to_blocks2
 *
 * Make a list of TO_BLOCKs for portrait and landscape orientation.
 **********************************************************************/

void assign_blobs_to_blocks2(Image pix,
                             BLOCK_LIST *blocks,           // blocks to process
                             TO_BLOCK_LIST *port_blocks) { // output list
  BLOCK_IT block_it = blocks;
  C_BLOB_IT blob_it;       // iterator
  BLOBNBOX_IT port_box_it; // iterator
                           // destination iterator
  TO_BLOCK_IT port_block_it = port_blocks;

  for (block_it.mark_cycle_pt(); !block_it.cycled_list(); block_it.forward()) {
    auto block = block_it.data();
    auto port_block = new TO_BLOCK(block);

    // Convert the good outlines to block->blob_list
    port_box_it.set_to_list(&port_block->blobs);
    blob_it.set_to_list(block->blob_list());
    for (blob_it.mark_cycle_pt(); !blob_it.cycled_list(); blob_it.forward()) {
      auto blob = blob_it.extract();
      auto newblob = new BLOBNBOX(blob); // Convert blob to BLOBNBOX.
      newblob->set_owns_cblob(true);
      SetBlobStrokeWidth(pix, newblob);
      port_box_it.add_after_then_move(newblob);
    }

    // Put the rejected outlines in block->noise_blobs, which allows them to
    // be reconsidered and sorted back into rows and recover outlines mistakenly
    // rejected.
    port_box_it.set_to_list(&port_block->noise_blobs);
    blob_it.set_to_list(block->reject_blobs());
    for (blob_it.mark_cycle_pt(); !blob_it.cycled_list(); blob_it.forward()) {
      auto blob = blob_it.extract();
      auto newblob = new BLOBNBOX(blob); // Convert blob to BLOBNBOX.
      newblob->set_owns_cblob(true);
      SetBlobStrokeWidth(pix, newblob);
      port_box_it.add_after_then_move(newblob);
    }

    port_block_it.add_after_then_move(port_block);
  }
}

/**********************************************************************
 * find_components
 *
 * Find the C_OUTLINEs of the connected components in each block, put them
 * in C_BLOBs, and filter them by size, putting the different size
 * grades on different lists in the matching TO_BLOCK in to_blocks.
 **********************************************************************/

void Textord::find_components(Image pix, BLOCK_LIST *blocks, TO_BLOCK_LIST *to_blocks) {
  auto width = pixGetWidth(pix);
  auto height = pixGetHeight(pix);

  BLOCK_IT block_it(blocks); // iterator
  for (block_it.mark_cycle_pt(); !block_it.cycled_list(); block_it.forward()) {
    BLOCK *block = block_it.data();
    if (block->pdblk.poly_block() == nullptr || block->pdblk.poly_block()->IsText()) {
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

void Textord::filter_blobs(ICOORD page_tr,        // top right
                           TO_BLOCK_LIST *blocks, // output list
                           bool testing_on) {     // for plotting
  TO_BLOCK_IT block_it = blocks;                  // destination iterator
  TO_BLOCK *block;                                // created block

#ifndef GRAPHICS_DISABLED
  if (to_win != nullptr) {
    to_win->Clear();
  }
#endif // !GRAPHICS_DISABLED

  for (block_it.mark_cycle_pt(); !block_it.cycled_list(); block_it.forward()) {
    block = block_it.data();
    block->line_size = filter_noise_blobs(&block->blobs, &block->noise_blobs, &block->small_blobs,
                                          &block->large_blobs);
    if (block->line_size == 0) {
      block->line_size = 1;
    }
    block->line_spacing =
        block->line_size *
        (tesseract::CCStruct::kDescenderFraction + tesseract::CCStruct::kXHeightFraction +
         2 * tesseract::CCStruct::kAscenderFraction) /
        tesseract::CCStruct::kXHeightFraction;
    block->line_size *= textord_min_linesize;
    block->max_blob_size = block->line_size * textord_excess_blobsize;

#ifndef GRAPHICS_DISABLED
    if (textord_show_blobs && testing_on) {
      if (to_win == nullptr) {
        create_to_win(page_tr);
      }
      block->plot_graded_blobs(to_win);
    }
    if (textord_show_boxes && testing_on) {
      if (to_win == nullptr) {
        create_to_win(page_tr);
      }
      plot_box_list(to_win, &block->noise_blobs, ScrollView::WHITE);
      plot_box_list(to_win, &block->small_blobs, ScrollView::WHITE);
      plot_box_list(to_win, &block->large_blobs, ScrollView::WHITE);
      plot_box_list(to_win, &block->blobs, ScrollView::WHITE);
    }
#endif // !GRAPHICS_DISABLED
  }
}

/**********************************************************************
 * filter_noise_blobs
 *
 * Move small blobs to a separate list.
 **********************************************************************/

float Textord::filter_noise_blobs(BLOBNBOX_LIST *src_list,     // original list
                                  BLOBNBOX_LIST *noise_list,   // noise list
                                  BLOBNBOX_LIST *small_list,   // small blobs
                                  BLOBNBOX_LIST *large_list) { // large blobs
  int16_t height;                                              // height of blob
  int16_t width;                                               // of blob
  BLOBNBOX *blob;                                              // current blob
  float initial_x;                                             // first guess
  BLOBNBOX_IT src_it = src_list;                               // iterators
  BLOBNBOX_IT noise_it = noise_list;
  BLOBNBOX_IT small_it = small_list;
  BLOBNBOX_IT large_it = large_list;
  STATS size_stats(0, MAX_NEAREST_DIST - 1);
  // blob heights
  float min_y; // size limits
  float max_y;
  float max_x;
  float max_height; // of good blobs

  for (src_it.mark_cycle_pt(); !src_it.cycled_list(); src_it.forward()) {
    blob = src_it.data();
    if (blob->bounding_box().height() < textord_max_noise_size) {
      noise_it.add_after_then_move(src_it.extract());
    } else if (blob->enclosed_area() >= blob->bounding_box().height() *
                                            blob->bounding_box().width() *
                                            textord_noise_area_ratio) {
      small_it.add_after_then_move(src_it.extract());
    }
  }
  for (src_it.mark_cycle_pt(); !src_it.cycled_list(); src_it.forward()) {
    size_stats.add(src_it.data()->bounding_box().height(), 1);
  }
  initial_x = size_stats.ile(textord_initialx_ile);
  max_y = ceil(initial_x *
               (tesseract::CCStruct::kDescenderFraction + tesseract::CCStruct::kXHeightFraction +
                2 * tesseract::CCStruct::kAscenderFraction) /
               tesseract::CCStruct::kXHeightFraction);
  min_y = std::floor(initial_x / 2);
  max_x = ceil(initial_x * textord_width_limit);
  small_it.move_to_first();
  for (small_it.mark_cycle_pt(); !small_it.cycled_list(); small_it.forward()) {
    height = small_it.data()->bounding_box().height();
    if (height > max_y) {
      large_it.add_after_then_move(small_it.extract());
    } else if (height >= min_y) {
      src_it.add_after_then_move(small_it.extract());
    }
  }
  size_stats.clear();
  for (src_it.mark_cycle_pt(); !src_it.cycled_list(); src_it.forward()) {
    height = src_it.data()->bounding_box().height();
    width = src_it.data()->bounding_box().width();
    if (height < min_y) {
      small_it.add_after_then_move(src_it.extract());
    } else if (height > max_y || width > max_x) {
      large_it.add_after_then_move(src_it.extract());
    } else {
      size_stats.add(height, 1);
    }
  }
  max_height = size_stats.ile(textord_initialasc_ile);
  //      tprintf("max_y=%g, min_y=%g, initial_x=%g, max_height=%g,",
  //              max_y,min_y,initial_x,max_height);
  max_height *= tesseract::CCStruct::kXHeightCapRatio;
  if (max_height > initial_x) {
    initial_x = max_height;
  }
  //      tprintf(" ret=%g\n",initial_x);
  return initial_x;
}

// Fixes the block so it obeys all the rules:
// Must have at least one ROW.
// Must have at least one WERD.
// WERDs contain a fake blob.
void Textord::cleanup_nontext_block(BLOCK *block) {
  // Non-text blocks must contain at least one row.
  ROW_IT row_it(block->row_list());
  if (row_it.empty()) {
    const TBOX &box = block->pdblk.bounding_box();
    float height = box.height();
    int32_t xstarts[2] = {box.left(), box.right()};
    double coeffs[3] = {0.0, 0.0, static_cast<double>(box.bottom())};
    ROW *row = new ROW(1, xstarts, coeffs, height / 2.0f, height / 4.0f, height / 4.0f, 0, 1);
    row_it.add_after_then_move(row);
  }
  // Each row must contain at least one word.
  for (row_it.mark_cycle_pt(); !row_it.cycled_list(); row_it.forward()) {
    ROW *row = row_it.data();
    WERD_IT w_it(row->word_list());
    if (w_it.empty()) {
      // Make a fake blob to put in the word.
      TBOX box = block->row_list()->singleton() ? block->pdblk.bounding_box() : row->bounding_box();
      C_BLOB *blob = C_BLOB::FakeBlob(box);
      C_BLOB_LIST blobs;
      C_BLOB_IT blob_it(&blobs);
      blob_it.add_after_then_move(blob);
      WERD *word = new WERD(&blobs, 0, nullptr);
      w_it.add_after_then_move(word);
    }
    // Each word must contain a fake blob.
    for (w_it.mark_cycle_pt(); !w_it.cycled_list(); w_it.forward()) {
      WERD *word = w_it.data();
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

void Textord::cleanup_blocks(bool clean_noise, BLOCK_LIST *blocks) {
  BLOCK_IT block_it = blocks; // iterator
  ROW_IT row_it;              // row iterator

  int num_rows = 0;
  int num_rows_all = 0;
  int num_blocks = 0;
  int num_blocks_all = 0;
  for (block_it.mark_cycle_pt(); !block_it.cycled_list(); block_it.forward()) {
    BLOCK *block = block_it.data();
    if (block->pdblk.poly_block() != nullptr && !block->pdblk.poly_block()->IsText()) {
      cleanup_nontext_block(block);
      continue;
    }
    num_rows = 0;
    num_rows_all = 0;
    if (clean_noise) {
      row_it.set_to_list(block->row_list());
      for (row_it.mark_cycle_pt(); !row_it.cycled_list(); row_it.forward()) {
        ROW *row = row_it.data();
        ++num_rows_all;
        clean_small_noise_from_words(row);
        if ((textord_noise_rejrows && !row->word_list()->empty() && clean_noise_from_row(row)) ||
            row->word_list()->empty()) {
          delete row_it.extract(); // lose empty row.
        } else {
          if (textord_noise_rejwords) {
            clean_noise_from_words(row_it.data());
          }
          if (textord_blshift_maxshift >= 0) {
            tweak_row_baseline(row, textord_blshift_maxshift, textord_blshift_xfraction);
          }
          ++num_rows;
        }
      }
    }
    if (block->row_list()->empty()) {
      delete block_it.extract(); // Lose empty text blocks.
    } else {
      ++num_blocks;
    }
    ++num_blocks_all;
    if (textord_noise_debug) {
      tprintf("cleanup_blocks: # rows = %d / %d\n", num_rows, num_rows_all);
    }
  }
  if (textord_noise_debug) {
    tprintf("cleanup_blocks: # blocks = %d / %d\n", num_blocks, num_blocks_all);
  }
}

/**********************************************************************
 * clean_noise_from_row
 *
 * Move blobs of words from rows of garbage into the reject blobs list.
 **********************************************************************/

bool Textord::clean_noise_from_row( // remove empties
    ROW *row                        // row to clean
) {
  bool testing_on;
  TBOX blob_box;            // bounding box
  C_BLOB *blob;             // current blob
  C_OUTLINE *outline;       // current outline
  WERD *word;               // current word
  int32_t blob_size;        // biggest size
  int32_t trans_count = 0;  // no of transitions
  int32_t trans_threshold;  // noise tolerance
  int32_t dot_count;        // small objects
  int32_t norm_count;       // normal objects
  int32_t super_norm_count; // real char-like
                            // words of row
  WERD_IT word_it = row->word_list();
  C_BLOB_IT blob_it;   // blob iterator
  C_OUTLINE_IT out_it; // outline iterator

  testing_on = textord_test_y > row->base_line(textord_test_x) && textord_show_blobs &&
               textord_test_y < row->base_line(textord_test_x) + row->x_height();
  dot_count = 0;
  norm_count = 0;
  super_norm_count = 0;
  for (word_it.mark_cycle_pt(); !word_it.cycled_list(); word_it.forward()) {
    word = word_it.data(); // current word
                           // blobs in word
    blob_it.set_to_list(word->cblob_list());
    for (blob_it.mark_cycle_pt(); !blob_it.cycled_list(); blob_it.forward()) {
      blob = blob_it.data();
      if (!word->flag(W_DONT_CHOP)) {
        // get outlines
        out_it.set_to_list(blob->out_list());
        for (out_it.mark_cycle_pt(); !out_it.cycled_list(); out_it.forward()) {
          outline = out_it.data();
          blob_box = outline->bounding_box();
          blob_size = blob_box.width() > blob_box.height() ? blob_box.width() : blob_box.height();
          if (blob_size < textord_noise_sizelimit * row->x_height()) {
            dot_count++; // count small outlines
          }
          if (!outline->child()->empty() &&
              blob_box.height() < (1 + textord_noise_syfract) * row->x_height() &&
              blob_box.height() > (1 - textord_noise_syfract) * row->x_height() &&
              blob_box.width() < (1 + textord_noise_sxfract) * row->x_height() &&
              blob_box.width() > (1 - textord_noise_sxfract) * row->x_height()) {
            super_norm_count++; // count small outlines
          }
        }
      } else {
        super_norm_count++;
      }
      blob_box = blob->bounding_box();
      blob_size = blob_box.width() > blob_box.height() ? blob_box.width() : blob_box.height();
      if (blob_size >= textord_noise_sizelimit * row->x_height() &&
          blob_size < row->x_height() * 2) {
        trans_threshold = blob_size / textord_noise_sizefraction;
        trans_count = blob->count_transitions(trans_threshold);
        if (trans_count < textord_noise_translimit) {
          norm_count++;
        }
      } else if (blob_box.height() > row->x_height() * 2 &&
                 (!word_it.at_first() || !blob_it.at_first())) {
        dot_count += 2;
      }
      if (testing_on) {
        tprintf("Blob at (%d,%d) -> (%d,%d), ols=%d, tc=%d, bldiff=%g\n", blob_box.left(),
                blob_box.bottom(), blob_box.right(), blob_box.top(), blob->out_list()->length(),
                trans_count, blob_box.bottom() - row->base_line(blob_box.left()));
      }
    }
  }
  // TODO: check whether `&& super_norm_count < textord_noise_sncount`should always be added here.
  bool rejected = dot_count > norm_count * textord_noise_normratio &&
                  dot_count > 2;
  if (textord_noise_debug) {
    tprintf("Row ending at (%d,%g):", blob_box.right(), row->base_line(blob_box.right()));
    tprintf(" R=%g, dc=%d, nc=%d, %s\n",
            norm_count > 0 ? static_cast<float>(dot_count) / norm_count : 9999, dot_count,
            norm_count,
            rejected? "REJECTED": "ACCEPTED");
  }
  return super_norm_count < textord_noise_sncount && rejected;
}

/**********************************************************************
 * clean_noise_from_words
 *
 * Move blobs of words from rows of garbage into the reject blobs list.
 **********************************************************************/

void Textord::clean_noise_from_words( // remove empties
    ROW *row                          // row to clean
) {
  TBOX blob_box;           // bounding box
  C_BLOB *blob;            // current blob
  C_OUTLINE *outline;      // current outline
  WERD *word;              // current word
  int32_t blob_size;       // biggest size
  int32_t trans_count;     // no of transitions
  int32_t trans_threshold; // noise tolerance
  int32_t dot_count;       // small objects
  int32_t norm_count;      // normal objects
  int32_t dud_words;       // number discarded
  int32_t ok_words;        // number remaining
  int32_t word_index;      // current word
                           // words of row
  WERD_IT word_it = row->word_list();
  C_BLOB_IT blob_it;   // blob iterator
  C_OUTLINE_IT out_it; // outline iterator

  ok_words = word_it.length();
  if (ok_words == 0 || textord_no_rejects) {
    return;
  }
  // was it chucked
  std::vector<int8_t> word_dud(ok_words);
  dud_words = 0;
  ok_words = 0;
  word_index = 0;
  for (word_it.mark_cycle_pt(); !word_it.cycled_list(); word_it.forward()) {
    word = word_it.data(); // current word
    dot_count = 0;
    norm_count = 0;
    // blobs in word
    blob_it.set_to_list(word->cblob_list());
    for (blob_it.mark_cycle_pt(); !blob_it.cycled_list(); blob_it.forward()) {
      blob = blob_it.data();
      if (!word->flag(W_DONT_CHOP)) {
        // get outlines
        out_it.set_to_list(blob->out_list());
        for (out_it.mark_cycle_pt(); !out_it.cycled_list(); out_it.forward()) {
          outline = out_it.data();
          blob_box = outline->bounding_box();
          blob_size = blob_box.width() > blob_box.height() ? blob_box.width() : blob_box.height();
          if (blob_size < textord_noise_sizelimit * row->x_height()) {
            dot_count++; // count small outlines
          }
          if (!outline->child()->empty() &&
              blob_box.height() < (1 + textord_noise_syfract) * row->x_height() &&
              blob_box.height() > (1 - textord_noise_syfract) * row->x_height() &&
              blob_box.width() < (1 + textord_noise_sxfract) * row->x_height() &&
              blob_box.width() > (1 - textord_noise_sxfract) * row->x_height()) {
            norm_count++; // count small outlines
          }
        }
      } else {
        norm_count++;
      }
      blob_box = blob->bounding_box();
      blob_size = blob_box.width() > blob_box.height() ? blob_box.width() : blob_box.height();
      if (blob_size >= textord_noise_sizelimit * row->x_height() &&
          blob_size < row->x_height() * 2) {
        trans_threshold = blob_size / textord_noise_sizefraction;
        trans_count = blob->count_transitions(trans_threshold);
        if (trans_count < textord_noise_translimit) {
          norm_count++;
        }
      } else if (blob_box.height() > row->x_height() * 2 &&
                 (!word_it.at_first() || !blob_it.at_first())) {
        dot_count += 2;
      }
    }
    if (dot_count > 2 && !word->flag(W_REP_CHAR)) {
      if (dot_count > norm_count * textord_noise_normratio * 2) {
        word_dud[word_index] = 2;
      } else if (dot_count > norm_count * textord_noise_normratio) {
        word_dud[word_index] = 1;
      } else {
        word_dud[word_index] = 0;
      }
    } else {
      word_dud[word_index] = 0;
    }
    if (word_dud[word_index] == 2) {
      dud_words++;
    } else {
      ok_words++;
    }
    word_index++;
  }

  word_index = 0;
  for (word_it.mark_cycle_pt(); !word_it.cycled_list(); word_it.forward()) {
    if (word_dud[word_index] == 2 || (word_dud[word_index] == 1 && dud_words > ok_words)) {
      word = word_it.data(); // Current word.
      // Previously we threw away the entire word.
      // Now just aggressively throw all small blobs into the reject list, where
      // the classifier can decide whether they are actually needed.
      word->CleanNoise(textord_noise_sizelimit * row->x_height());
    }
    word_index++;
  }
}

// Remove outlines that are a tiny fraction in either width or height
// of the word height.
void Textord::clean_small_noise_from_words(ROW *row) {
  WERD_IT word_it(row->word_list());
  for (word_it.mark_cycle_pt(); !word_it.cycled_list(); word_it.forward()) {
    WERD *word = word_it.data();
    int min_size = static_cast<int>(textord_noise_hfract * word->bounding_box().height() + 0.5);
    C_BLOB_IT blob_it(word->cblob_list());
    for (blob_it.mark_cycle_pt(); !blob_it.cycled_list(); blob_it.forward()) {
      C_BLOB *blob = blob_it.data();
      C_OUTLINE_IT out_it(blob->out_list());
      for (out_it.mark_cycle_pt(); !out_it.cycled_list(); out_it.forward()) {
        C_OUTLINE *outline = out_it.data();
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
        WERD *next_word = word_it.data_relative(1);
        if (next_word->flag(W_FUZZY_NON)) {
          next_word->set_flag(W_FUZZY_NON, false);
        }
      }
      delete word_it.extract();
    }
  }
}

// Local struct to hold a group of blocks.
struct BlockGroup {
  BlockGroup() : rotation(1.0f, 0.0f), angle(0.0f), min_xheight(1.0f) {}
  explicit BlockGroup(BLOCK *block)
      : bounding_box(block->pdblk.bounding_box())
      , rotation(block->re_rotation())
      , angle(block->re_rotation().angle())
      , min_xheight(block->x_height()) {
    blocks.push_back(block);
  }
  // Union of block bounding boxes.
  TBOX bounding_box;
  // Common rotation of the blocks.
  FCOORD rotation;
  // Angle of rotation.
  float angle;
  // Min xheight of the blocks.
  float min_xheight;
  // Collection of borrowed pointers to the blocks in the group.
  std::vector<BLOCK *> blocks;
};

// Groups blocks by rotation, then, for each group, makes a WordGrid and calls
// TransferDiacriticsToWords to copy the diacritic blobs to the most
// appropriate words in the group of blocks. Source blobs are not touched.
void Textord::TransferDiacriticsToBlockGroups(BLOBNBOX_LIST *diacritic_blobs, BLOCK_LIST *blocks) {
  // Angle difference larger than this is too much to consider equal.
  // They should only be in multiples of M_PI/2 anyway.
  const double kMaxAngleDiff = 0.01; // About 0.6 degrees.
  std::vector<std::unique_ptr<BlockGroup>> groups;
  BLOCK_IT bk_it(blocks);
  for (bk_it.mark_cycle_pt(); !bk_it.cycled_list(); bk_it.forward()) {
    BLOCK *block = bk_it.data();
    if (block->pdblk.poly_block() != nullptr && !block->pdblk.poly_block()->IsText()) {
      continue;
    }
    // Linear search of the groups to find a matching rotation.
    float block_angle = block->re_rotation().angle();
    int best_g = 0;
    float best_angle_diff = FLT_MAX;
    for (const auto &group : groups) {
      double angle_diff = std::fabs(block_angle - group->angle);
      if (angle_diff > M_PI) {
        angle_diff = fabs(angle_diff - 2.0 * M_PI);
      }
      if (angle_diff < best_angle_diff) {
        best_angle_diff = angle_diff;
        best_g = &group - &groups[0];
      }
    }
    if (best_angle_diff > kMaxAngleDiff) {
      groups.push_back(std::make_unique<BlockGroup>(block));
    } else {
      groups[best_g]->blocks.push_back(block);
      groups[best_g]->bounding_box += block->pdblk.bounding_box();
      float x_height = block->x_height();
      if (x_height < groups[best_g]->min_xheight) {
        groups[best_g]->min_xheight = x_height;
      }
    }
  }
  // Now process each group of blocks.
  std::vector<std::unique_ptr<WordWithBox>> word_ptrs;
  for (const auto &group : groups) {
    if (group->bounding_box.null_box()) {
      continue;
    }
    WordGrid word_grid(group->min_xheight, group->bounding_box.botleft(),
                       group->bounding_box.topright());
    for (auto b : group->blocks) {
      ROW_IT row_it(b->row_list());
      for (row_it.mark_cycle_pt(); !row_it.cycled_list(); row_it.forward()) {
        ROW *row = row_it.data();
        // Put the words of the row into the grid.
        WERD_IT w_it(row->word_list());
        for (w_it.mark_cycle_pt(); !w_it.cycled_list(); w_it.forward()) {
          WERD *word = w_it.data();
          auto box_word = std::make_unique<WordWithBox>(word);
          word_grid.InsertBBox(true, true, box_word.get());
          // Save the pointer where it will be auto-deleted.
          word_ptrs.emplace_back(std::move(box_word));
        }
      }
    }
    FCOORD rotation = group->rotation;
    // Make it a forward rotation that will transform blob coords to block.
    rotation.set_y(-rotation.y());
    TransferDiacriticsToWords(diacritic_blobs, rotation, &word_grid);
  }
}

// Places a copy of blobs that are near a word (after applying rotation to the
// blob) in the most appropriate word, unless there is doubt, in which case a
// blob can end up in two words. Source blobs are not touched.
void Textord::TransferDiacriticsToWords(BLOBNBOX_LIST *diacritic_blobs, const FCOORD &rotation,
                                        WordGrid *word_grid) {
  WordSearch ws(word_grid);
  BLOBNBOX_IT b_it(diacritic_blobs);
  // Apply rotation to each blob before finding the nearest words. The rotation
  // allows us to only consider above/below placement and not left/right on
  // vertical text, because all text is horizontal here.
  for (b_it.mark_cycle_pt(); !b_it.cycled_list(); b_it.forward()) {
    BLOBNBOX *blobnbox = b_it.data();
    TBOX blob_box = blobnbox->bounding_box();
    blob_box.rotate(rotation);
    ws.StartRectSearch(blob_box);
    // Above/below refer to word position relative to diacritic. Since some
    // scripts eg Kannada/Telugu habitually put diacritics below words, and
    // others eg Thai/Vietnamese/Latin put most diacritics above words, try
    // for both if there isn't much in it.
    WordWithBox *best_above_word = nullptr;
    WordWithBox *best_below_word = nullptr;
    int best_above_distance = 0;
    int best_below_distance = 0;
    for (WordWithBox *word = ws.NextRectSearch(); word != nullptr; word = ws.NextRectSearch()) {
      if (word->word()->flag(W_REP_CHAR)) {
        continue;
      }
      TBOX word_box = word->true_bounding_box();
      int x_distance = blob_box.x_gap(word_box);
      int y_distance = blob_box.y_gap(word_box);
      if (x_distance > 0) {
        // Arbitrarily divide x-distance by 2 if there is a major y overlap,
        // and the word is to the left of the diacritic. If the
        // diacritic is a dropped broken character between two words, this will
        // help send all the pieces to a single word, instead of splitting them
        // over the 2 words.
        if (word_box.major_y_overlap(blob_box) && blob_box.left() > word_box.right()) {
          x_distance /= 2;
        }
        y_distance += x_distance;
      }
      if (word_box.y_middle() > blob_box.y_middle() &&
          (best_above_word == nullptr || y_distance < best_above_distance)) {
        best_above_word = word;
        best_above_distance = y_distance;
      }
      if (word_box.y_middle() <= blob_box.y_middle() &&
          (best_below_word == nullptr || y_distance < best_below_distance)) {
        best_below_word = word;
        best_below_distance = y_distance;
      }
    }
    bool above_good = best_above_word != nullptr &&
                      (best_below_word == nullptr ||
                       best_above_distance < best_below_distance + blob_box.height());
    bool below_good = best_below_word != nullptr && best_below_word != best_above_word &&
                      (best_above_word == nullptr ||
                       best_below_distance < best_above_distance + blob_box.height());
    if (below_good) {
      C_BLOB *copied_blob = C_BLOB::deep_copy(blobnbox->cblob());
      copied_blob->rotate(rotation);
      // Put the blob into the word's reject blobs list.
      C_BLOB_IT blob_it(best_below_word->RejBlobs());
      blob_it.add_to_end(copied_blob);
    }
    if (above_good) {
      C_BLOB *copied_blob = C_BLOB::deep_copy(blobnbox->cblob());
      copied_blob->rotate(rotation);
      // Put the blob into the word's reject blobs list.
      C_BLOB_IT blob_it(best_above_word->RejBlobs());
      blob_it.add_to_end(copied_blob);
    }
  }
}

/**********************************************************************
 * tweak_row_baseline
 *
 * Shift baseline to fit the blobs more accurately where they are
 * close enough.
 **********************************************************************/

void tweak_row_baseline(ROW *row, double blshift_maxshift, double blshift_xfraction) {
  TBOX blob_box;      // bounding box
  C_BLOB *blob;       // current blob
  WERD *word;         // current word
  int32_t blob_count; // no of blobs
  int32_t src_index;  // source segment
  int32_t dest_index; // destination segment
  float ydiff;        // baseline error
  float x_centre;     // centre of blob
                      // words of row
  WERD_IT word_it = row->word_list();
  C_BLOB_IT blob_it; // blob iterator

  blob_count = 0;
  for (word_it.mark_cycle_pt(); !word_it.cycled_list(); word_it.forward()) {
    word = word_it.data(); // current word
                           // get total blobs
    blob_count += word->cblob_list()->length();
  }
  if (blob_count == 0) {
    return;
  }
  // spline segments
  std::vector<int32_t> xstarts(blob_count + row->baseline.segments + 1);
  // spline coeffs
  std::vector<double> coeffs((blob_count + row->baseline.segments) * 3);

  src_index = 0;
  dest_index = 0;
  xstarts[0] = row->baseline.xcoords[0];
  for (word_it.mark_cycle_pt(); !word_it.cycled_list(); word_it.forward()) {
    word = word_it.data(); // current word
                           // blobs in word
    blob_it.set_to_list(word->cblob_list());
    for (blob_it.mark_cycle_pt(); !blob_it.cycled_list(); blob_it.forward()) {
      blob = blob_it.data();
      blob_box = blob->bounding_box();
      x_centre = (blob_box.left() + blob_box.right()) / 2.0;
      ydiff = blob_box.bottom() - row->base_line(x_centre);
      if (ydiff < 0) {
        ydiff = -ydiff / row->x_height();
      } else {
        ydiff = ydiff / row->x_height();
      }
      if (ydiff < blshift_maxshift && blob_box.height() / row->x_height() > blshift_xfraction) {
        if (xstarts[dest_index] >= x_centre) {
          xstarts[dest_index] = blob_box.left();
        }
        coeffs[dest_index * 3] = 0;
        coeffs[dest_index * 3 + 1] = 0;
        coeffs[dest_index * 3 + 2] = blob_box.bottom();
        // shift it
        dest_index++;
        xstarts[dest_index] = blob_box.right() + 1;
      } else {
        if (xstarts[dest_index] <= x_centre) {
          while (row->baseline.xcoords[src_index + 1] <= x_centre &&
                 src_index < row->baseline.segments - 1) {
            if (row->baseline.xcoords[src_index + 1] > xstarts[dest_index]) {
              coeffs[dest_index * 3] = row->baseline.quadratics[src_index].a;
              coeffs[dest_index * 3 + 1] = row->baseline.quadratics[src_index].b;
              coeffs[dest_index * 3 + 2] = row->baseline.quadratics[src_index].c;
              dest_index++;
              xstarts[dest_index] = row->baseline.xcoords[src_index + 1];
            }
            src_index++;
          }
          coeffs[dest_index * 3] = row->baseline.quadratics[src_index].a;
          coeffs[dest_index * 3 + 1] = row->baseline.quadratics[src_index].b;
          coeffs[dest_index * 3 + 2] = row->baseline.quadratics[src_index].c;
          dest_index++;
          xstarts[dest_index] = row->baseline.xcoords[src_index + 1];
        }
      }
    }
  }
  while (src_index < row->baseline.segments &&
         row->baseline.xcoords[src_index + 1] <= xstarts[dest_index]) {
    src_index++;
  }
  while (src_index < row->baseline.segments) {
    coeffs[dest_index * 3] = row->baseline.quadratics[src_index].a;
    coeffs[dest_index * 3 + 1] = row->baseline.quadratics[src_index].b;
    coeffs[dest_index * 3 + 2] = row->baseline.quadratics[src_index].c;
    dest_index++;
    src_index++;
    xstarts[dest_index] = row->baseline.xcoords[src_index];
  }
  // turn to spline
  row->baseline = QSPLINE(dest_index, &xstarts[0], &coeffs[0]);
}

} // namespace tesseract
