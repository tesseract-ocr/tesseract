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
#include "mfcpch.h"
#ifdef __UNIX__
#include <assert.h>
#endif
#include "stderr.h"
#include "globaloc.h"
#include "tessout.h"
#include "blread.h"
#include "blobbox.h"
#include "edgblob.h"
#include "drawtord.h"
#include "makerow.h"
#include "wordseg.h"
#include "ocrclass.h"
#include "genblob.h"
#include "imgs.h"
#include "tordmain.h"
#include "secname.h"
#include "tesseractclass.h"

// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif

// Some of the code in this file is dependent upon leptonica. If you don't
// have it, you don't get this functionality.
#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif
#ifdef HAVE_LIBLEPT
#include "allheaders.h"
#endif

const ERRCODE BLOCKLESS_BLOBS = "Warning:some blobs assigned to no block";

#undef EXTERN
#define EXTERN

EXTERN BOOL_VAR (textord_no_rejects, FALSE, "Don't remove noise blobs");
EXTERN BOOL_VAR (textord_show_blobs, FALSE, "Display unsorted blobs");
EXTERN BOOL_VAR (textord_show_boxes, FALSE, "Display unsorted blobs");
EXTERN BOOL_VAR (textord_new_initial_xheight, TRUE,
"Use test xheight mechanism");
EXTERN BOOL_VAR (textord_exit_after, FALSE, "Exit after completing textord");
EXTERN INT_VAR (textord_max_noise_size, 7, "Pixel size of noise");
EXTERN double_VAR (textord_blob_size_bigile, 95,
"Percentile for large blobs");
EXTERN double_VAR (textord_noise_area_ratio, 0.7,
"Fraction of bounding box for noise");
EXTERN double_VAR (textord_blob_size_smallile, 20,
"Percentile for small blobs");
EXTERN double_VAR (textord_initialx_ile, 0.75,
"Ile of sizes for xheight guess");
EXTERN double_VAR (textord_initialasc_ile, 0.90,
"Ile of sizes for xheight guess");
EXTERN INT_VAR (textord_noise_sizefraction, 10,
"Fraction of size for maxima");
EXTERN double_VAR (textord_noise_sizelimit, 0.5,
"Fraction of x for big t count");
EXTERN INT_VAR (textord_noise_translimit, 16, "Transitions for normal blob");
EXTERN double_VAR (textord_noise_normratio, 2.0,
"Dot to norm ratio for deletion");
EXTERN BOOL_VAR (textord_noise_rejwords, TRUE, "Reject noise-like words");
EXTERN BOOL_VAR (textord_noise_rejrows, TRUE, "Reject noise-like rows");
EXTERN double_VAR (textord_noise_syfract, 0.2,
"xh fract error for norm blobs");
EXTERN double_VAR (textord_noise_sxfract, 0.4,
"xh fract width error for norm blobs");
EXTERN double_VAR(textord_noise_hfract, 1.0/64,
"Height fraction to discard outlines as speckle noise");
EXTERN INT_VAR (textord_noise_sncount, 1, "super norm blobs to save row");
EXTERN double_VAR (textord_noise_rowratio, 6.0,
"Dot to norm ratio for deletion");

EXTERN BOOL_VAR (textord_noise_debug, FALSE, "Debug row garbage detector");
EXTERN double_VAR (textord_blshift_maxshift, 0.00, "Max baseline shift");
EXTERN double_VAR (textord_blshift_xfraction, 9.99,
"Min size of baseline shift");
EXTERN STRING_EVAR (tessedit_image_ext, ".tif", "Externsion for image file");

#ifndef EMBEDDED
EXTERN clock_t previous_cpu;
#endif

extern BOOL_VAR_H (polygon_tess_approximation, TRUE,
"Do tess poly instead of grey scale");

#define MAX_NEAREST_DIST  600    //for block skew stats
#define MAX_BLOB_TRANSITIONS100  //for nois stats

extern IMAGE page_image;         //must be defined somewhere
extern BOOL_VAR_H (interactive_mode, TRUE, "Run interactively?");
extern /*"C" */ ETEXT_DESC *global_monitor;     //progress monitor

/**********************************************************************
 * find_components
 *
 * Find the C_OUTLINEs of the connected components in each block, put them
 * in C_BLOBs, and filter them by size, putting the different size
 * grades on different lists in the matching TO_BLOCK in port_blocks.
 **********************************************************************/

void find_components(
                       BLOCK_LIST *blocks,
                       TO_BLOCK_LIST *land_blocks,
                       TO_BLOCK_LIST *port_blocks,
                       TBOX *page_box) {
  BLOCK *block;                  //current block
  PDBLK_CLIST pd_blocks;         //copy of list
  BLOCK_IT block_it = blocks;    //iterator
  PDBLK_C_IT pd_it = &pd_blocks; //iterator
  IMAGE thresh_image;            //thresholded

  int width = page_image.get_xsize();
  int height = page_image.get_ysize();
  if (width > MAX_INT16 || height > MAX_INT16) {
    tprintf("Input image too large! (%d, %d)\n", width, height);
    return;  // Can't handle it.
  }

  ICOORD page_tr(width, height);
  block_it.set_to_list (blocks);
  if (global_monitor != NULL)
    global_monitor->ocr_alive = TRUE;

  set_global_loc_code(LOC_EDGE_PROG);
  if (!page_image.white_high ())
    invert_image(&page_image);

#ifndef EMBEDDED
  previous_cpu = clock ();
#endif

  for (block_it.mark_cycle_pt(); !block_it.cycled_list();
       block_it.forward()) {
    block = block_it.data();
    if (block->poly_block() == NULL ||
        block->poly_block()->IsText()) {
#ifndef GRAPHICS_DISABLED
      extract_edges(NULL, &page_image, &page_image, page_tr, block);
#else
      extract_edges(&page_image, &page_image, page_tr, block);
#endif
      *page_box += block->bounding_box ();
    }
  }
  if (global_monitor != NULL) {
    global_monitor->ocr_alive = TRUE;
    global_monitor->progress = 10;
  }

  assign_blobs_to_blocks2(blocks, land_blocks, port_blocks);
  if (global_monitor != NULL)
    global_monitor->ocr_alive = TRUE;
  filter_blobs (page_box->topright (), land_blocks, textord_test_landscape);
#ifndef EMBEDDED
  previous_cpu = clock ();
#endif
  filter_blobs (page_box->topright (), port_blocks, !textord_test_landscape);
  if (global_monitor != NULL)
    global_monitor->ocr_alive = TRUE;
}

/**********************************************************************
 * SetBlobStrokeWidth
 *
 * Set the horizontal and vertical stroke widths in the blob.
 **********************************************************************/
void SetBlobStrokeWidth(bool debug, BLOBNBOX* blob) {
#ifdef HAVE_LIBLEPT
  // Cut the blob rectangle into a Pix.
  // TODO(rays) make the page_image a Pix so this is more direct.
  const TBOX& box = blob->bounding_box();
  IMAGE blob_im;
  int width = box.width();
  int height = box.height();
  blob_im.create(width, height, 1);
  copy_sub_image(&page_image, box.left(), box.bottom(), width, height,
                 &blob_im, 0, 0, false);
  Pix* pix = blob_im.ToPix();
  Pix* dist_pix = pixDistanceFunction(pix, 4, 8, L_BOUNDARY_BG);
  if (debug) {
    pixWrite("cutpix.png", pix, IFF_PNG);
    pixWrite("distpix.png", dist_pix, IFF_PNG);
  }
  pixDestroy(&pix);
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
  if (debug) {
    h_stats.print(stderr, true);
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
  if (debug) {
    v_stats.print(stderr, true);
  }
  pixDestroy(&dist_pix);
  // Store the horizontal and vertical width in the blob, keeping both
  // widths if there is enough information, otherwse only the one with
  // the most samples.
  // If there are insufficent samples, store zero, rather than using
  // 2*area/perimeter, as the numbers that gives do not match the numbers
  // from the distance method.
  if (debug) {
    tprintf("box=%d,%d->%d,%d, hcount=%d, vcount=%d, target=%d\n",
            box.left(), box.bottom(), box.right(), box.top(),
            h_stats.get_total(), v_stats.get_total(), (width+height) /4);
    tprintf("hstats median=%f, lq=%f, uq=%f, sd=%f\n",
            h_stats.median(), h_stats.ile(0.25f), h_stats.ile(0.75f),
            h_stats.sd());
    tprintf("vstats median=%f, lq=%f, uq=%f, sd=%f\n",
            v_stats.median(), v_stats.ile(0.25f), v_stats.ile(0.75f),
            v_stats.sd());

  }
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
#else
  // Without leptonica present, use the 2*area/perimeter as an approximation.
  float width = 2.0f * blob->cblob()->area();
  width /= blob->cblob()->perimeter();
  blob->set_horz_stroke_width(width);
  blob->set_vert_stroke_width(width);
#endif
}


/**********************************************************************
 * assign_blobs_to_blocks2
 *
 * Make a list of TO_BLOCKs for portrait and landscape orientation.
 **********************************************************************/

void assign_blobs_to_blocks2(                             // split into groups
                             BLOCK_LIST *blocks,          // blocks to process
                             TO_BLOCK_LIST *land_blocks,  // ** unused **
                             TO_BLOCK_LIST *port_blocks   // output list
                            ) {
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
    block = block_it.data ();
    port_block = new TO_BLOCK(block);

    // Convert the good outlines to block->blob_list
    port_box_it.set_to_list(&port_block->blobs);
    blob_it.set_to_list(block->blob_list());
    for (blob_it.mark_cycle_pt(); !blob_it.cycled_list(); blob_it.forward()) {
      blob = blob_it.extract();
      newblob = new BLOBNBOX(blob);  // Convert blob to BLOBNBOX.
      SetBlobStrokeWidth(false, newblob);
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
      SetBlobStrokeWidth(false, newblob);
      port_box_it.add_after_then_move(newblob);
    }

    port_block_it.add_after_then_move(port_block);
  }
}


/**********************************************************************
 * filter_blobs
 *
 * Sort the blobs into sizes in all the blocks for later work.
 **********************************************************************/

void filter_blobs(                        //split into groups
                  ICOORD page_tr,         //top right
                  TO_BLOCK_LIST *blocks,  //output list
                  BOOL8 testing_on        //for plotting
                 ) {
  TO_BLOCK_IT block_it = blocks; //destination iterator
  TO_BLOCK *block;               //created block

  if (to_win != NULL)
    to_win->Clear();
  for (block_it.mark_cycle_pt (); !block_it.cycled_list ();
  block_it.forward ()) {
    block = block_it.data ();
    block->line_size = filter_noise_blobs (&block->blobs,
      &block->noise_blobs,
      &block->small_blobs,
      &block->large_blobs);
    block->line_spacing =
      block->line_size * (textord_merge_desc + textord_merge_x +
      textord_merge_asc +
      textord_merge_asc) / textord_merge_x;
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
      plot_box_list (to_win, &block->noise_blobs, ScrollView::WHITE);
      plot_box_list (to_win, &block->small_blobs, ScrollView::WHITE);
      plot_box_list (to_win, &block->large_blobs, ScrollView::WHITE);
      plot_box_list (to_win, &block->blobs, ScrollView::WHITE);
    }
#endif
  }
}


/**********************************************************************
 * filter_noise_blobs
 *
 * Move small blobs to a separate list.
 **********************************************************************/

float filter_noise_blobs(                            //separate noise
                         BLOBNBOX_LIST *src_list,    //origonal list
                         BLOBNBOX_LIST *noise_list,  //noise list
                         BLOBNBOX_LIST *small_list,  //small blobs
                         BLOBNBOX_LIST *large_list   //large blobs
                        ) {
  inT16 height;                  //height of blob
  inT16 width;                   //of blob
  BLOBNBOX_IT src_it = src_list; //iterators
  BLOBNBOX_IT noise_it = noise_list;
  BLOBNBOX_IT small_it = small_list;
  BLOBNBOX_IT large_it = large_list;
  STATS size_stats (0, MAX_NEAREST_DIST);
  //blob heights
  if (textord_new_initial_xheight)
    return filter_noise_blobs2 (src_list, noise_list, small_list, large_list);
  float min_y;                   //size limits
  float max_y;
  float max_x;

  for (src_it.mark_cycle_pt (); !src_it.cycled_list (); src_it.forward ()) {
    if (src_it.data ()->bounding_box ().height () < textord_max_noise_size)
      noise_it.add_after_then_move (src_it.extract ());
  }
  for (src_it.mark_cycle_pt (); !src_it.cycled_list (); src_it.forward ()) {
    size_stats.add (src_it.data ()->bounding_box ().height (), 1);
  }
  min_y = floor (size_stats.ile (textord_blob_size_smallile / 100.0));
  max_y = ceil (size_stats.ile (textord_blob_size_bigile / 100.0));
  max_x = ceil (size_stats.ile (0.5) * textord_width_limit);
  for (src_it.mark_cycle_pt (); !src_it.cycled_list (); src_it.forward ()) {
    height = src_it.data ()->bounding_box ().height ();
    width = src_it.data ()->bounding_box ().width ();
    if (height < min_y)
      small_it.add_after_then_move (src_it.extract ());
    else if (height > max_y || width > max_x)
      large_it.add_after_then_move (src_it.extract ());
  }
  return size_stats.ile (textord_initialx_ile);
}


/**********************************************************************
 * filter_noise_blobs2
 *
 * Move small blobs to a separate list.
 **********************************************************************/

float filter_noise_blobs2(                            //separate noise
                          BLOBNBOX_LIST *src_list,    //origonal list
                          BLOBNBOX_LIST *noise_list,  //noise list
                          BLOBNBOX_LIST *small_list,  //small blobs
                          BLOBNBOX_LIST *large_list   //large blobs
                         ) {
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

  for (src_it.mark_cycle_pt (); !src_it.cycled_list (); src_it.forward ()) {
    blob = src_it.data ();
    if (blob->bounding_box ().height () < textord_max_noise_size)
      noise_it.add_after_then_move (src_it.extract ());
    else if (blob->enclosed_area () >= blob->bounding_box ().height ()
      * blob->bounding_box ().width () * textord_noise_area_ratio)
      small_it.add_after_then_move (src_it.extract ());
  }
  for (src_it.mark_cycle_pt (); !src_it.cycled_list (); src_it.forward ()) {
    size_stats.add (src_it.data ()->bounding_box ().height (), 1);
  }
  initial_x = size_stats.ile (textord_initialx_ile);
  max_y =
    ceil (initial_x *
    (textord_merge_desc + textord_merge_x +
    2 * textord_merge_asc) / textord_merge_x);
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
  //      printf("max_y=%g, min_y=%g, initial_x=%g, max_height=%g,",
  //              max_y,min_y,initial_x,max_height);
  max_height *= textord_merge_x / (textord_merge_x + textord_merge_asc);
  if (max_height > initial_x)
    initial_x = max_height;
  //      printf(" ret=%g\n",initial_x);
  return initial_x;
}


/**********************************************************************
 * textord_page
 *
 * Textord the list of blobs and return a list of proper blocks.
 **********************************************************************/

void textord_page(                             //make rows & words
                  ICOORD page_tr,              //top right
                  BLOCK_LIST *blocks,          //block list
                  TO_BLOCK_LIST *land_blocks,  //rotated for landscape
                  TO_BLOCK_LIST *port_blocks,  //output list
                  tesseract::Tesseract* tess
                 ) {
  float gradient;                //global skew

  set_global_loc_code(LOC_TEXT_ORD_ROWS);
  gradient = make_rows (page_tr, blocks, land_blocks, port_blocks, tess);
  if (global_monitor != NULL) {
    global_monitor->ocr_alive = TRUE;
    global_monitor->progress = 20;
  }
  set_global_loc_code(LOC_TEXT_ORD_WORDS);
  make_words(page_tr, gradient, blocks, land_blocks, port_blocks, tess);
  if (global_monitor != NULL) {
    global_monitor->ocr_alive = TRUE;
    global_monitor->progress = 30;
  }
  cleanup_blocks(blocks);  //remove empties
#ifndef GRAPHICS_DISABLED
  close_to_win();
#endif
  if (textord_exit_after && !interactive_mode)
    exit (0);
}

/**********************************************************************
 * cleanup_blocks
 *
 * Delete empty blocks, rows from the page.
 **********************************************************************/

void cleanup_blocks(                    //remove empties
                    BLOCK_LIST *blocks  //list
                   ) {
  BLOCK_IT block_it = blocks;    //iterator
  ROW_IT row_it;                 //row iterator

  for (block_it.mark_cycle_pt (); !block_it.cycled_list ();
  block_it.forward ()) {
    row_it.set_to_list (block_it.data ()->row_list ());
    for (row_it.mark_cycle_pt (); !row_it.cycled_list (); row_it.forward ()) {
      clean_small_noise_from_words(row_it.data());
      if ((textord_noise_rejrows
        && !row_it.data ()->word_list ()->empty ()
        && clean_noise_from_row (row_it.data ()))
        || row_it.data ()->word_list ()->empty ())
        delete row_it.extract ();//lose empty row
      else {
        if (textord_noise_rejwords)
          clean_noise_from_words (row_it.data ());
        if (textord_blshift_maxshift >= 0)
          tweak_row_baseline (row_it.data ());
      }
    }
    if (block_it.data ()->row_list ()->empty ()) {
      delete block_it.extract ();//lose empty block
    }
  }
}


/**********************************************************************
 * clean_noise_from_row
 *
 * Move blobs of words from rows of garbage into the reject blobs list.
 **********************************************************************/

BOOL8 clean_noise_from_row(          //remove empties
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
      #ifndef SECURE_NAMES
      if (testing_on) {
        tprintf
          ("Blob at (%d,%d) -> (%d,%d), ols=%d, tc=%d, bldiff=%g\n",
          blob_box.left (), blob_box.bottom (), blob_box.right (),
          blob_box.top (), blob->out_list ()->length (), trans_count,
          blob_box.bottom () - row->base_line (blob_box.left ()));
      }
      #endif
    }
  }
  #ifndef SECURE_NAMES
  if (textord_noise_debug) {
    tprintf ("Row ending at (%d,%g):",
      blob_box.right (), row->base_line (blob_box.right ()));
    tprintf (" R=%g, dc=%d, nc=%d, %s\n",
      norm_count > 0 ? (float) dot_count / norm_count : 9999,
      dot_count, norm_count,
      dot_count > norm_count * textord_noise_normratio
      && dot_count > 2 ? "REJECTED" : "ACCEPTED");
  }
  #endif
  return super_norm_count < textord_noise_sncount
    && dot_count > norm_count * textord_noise_rowratio && dot_count > 2;
}


/**********************************************************************
 * clean_noise_from_words
 *
 * Move blobs of words from rows of garbage into the reject blobs list.
 **********************************************************************/

void clean_noise_from_words(          //remove empties
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
void clean_small_noise_from_words(ROW *row) {
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


/**********************************************************************
 * tweak_row_baseline
 *
 * Shift baseline to fit the blobs more accurately where they are
 * close enough.
 **********************************************************************/

void tweak_row_baseline(          //remove empties
                        ROW *row  //row to clean
                       ) {
  TBOX blob_box;                  //bounding box
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
      if (ydiff < textord_blshift_maxshift
        && blob_box.height () / row->x_height () >
      textord_blshift_xfraction) {
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

