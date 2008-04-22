///////////////////////////////////////////////////////////////////////
// File:        pageseg.cpp
// Description: Page Segmenter
// Author:      Thomas Kielbus
// Created:     Wed Jul 18 10:05:01 PDT 2007
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

// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif

#include "pageseg.h"

#include "edgloop.h"
#include "imgs.h"
#ifdef HAVE_LIBLEPT
#include "leptonica_pageseg_interface.h"
#endif
#include "polyaprx.h"
#include "tessvars.h"
#include "varable.h"

BOOL_VAR(pageseg_leptonica_activated, FALSE,
         "Activate Leptonica-based segmentation");

// Create a new full-page BLOCK and append it to the given list
static void append_full_page_block(BLOCK_LIST *blocks);

// Create a new BLOCK with the given coordinates and append it to the given list
static void append_block(BLOCK_LIST *blocks,
                         int xmin, int ymin,
                         int xmax, int ymax,
                         POLY_BLOCK* outline);

#ifndef HAVE_LIBLEPT
// Leptonica-based page segmentation is not enabled


void segment_page(BLOCK_LIST *blocks) {
  append_full_page_block(blocks);
}


#else  // HAVE_LIBLEPT Leptonica-based page segmentation is enabled


// Create a list of BLOCKs from the given mask image and append them to the
// given BLOCK_LIST
static void mask_to_blocks(IMAGE* textblock_mask_image,
                            BLOCK_LIST *blocks);

// Extract the OUTLINEs from the image mask and append them to the given
// OUTLINE_LIST
static void mask_to_outlines(IMAGE* image, OUTLINE_LIST* outlines);

// Convert the OUTLINEs into BLOCKs and append them to the given BLOCK_LIST
static void outlines_to_blocks(OUTLINE_LIST* outlines, BLOCK_LIST* blocks);

void segment_page(BLOCK_LIST *blocks) {
  IMAGE textblock_mask;
  if (pageseg_leptonica_activated &&
      leptonica_pageseg_get_textblock_mask(&page_image, &textblock_mask))
    mask_to_blocks(&textblock_mask, blocks);
  else
    append_full_page_block(blocks);
}

void mask_to_outlines(IMAGE* image,
                       OUTLINE_LIST* outlines) {
  // Get outlines of the textblock_mask_image
  ICOORD page_tr(image->get_xsize(), image->get_ysize());
  PDBLK full_page_block(0, 0, image->get_xsize(), image->get_ysize());
  C_OUTLINE_LIST c_outlines;
  C_OUTLINE_IT c_outline_it(&c_outlines);

  get_outlines(NULL, image, image,
               page_tr, &full_page_block, &c_outline_it);

  // Convert each C_OUTLINE to an OUTLINE
  OUTLINE_IT outline_it(outlines);
  outline_it.move_to_last();
  for (c_outline_it.mark_cycle_pt();
       !c_outline_it.cycled_list();
       c_outline_it.forward()) {
    OUTLINE* outline;
    outline = tesspoly_outline(c_outline_it.data(), 0);
    outline_it.add_after_then_move(outline);
  }
  c_outlines.clear();
}

void outlines_to_blocks(OUTLINE_LIST* outlines, BLOCK_LIST* blocks) {
  OUTLINE_IT outline_it(outlines);

  // Convert each OUTLINE to a POLY_BLOCK and add it to the block list
  for (outline_it.mark_cycle_pt();
       !outline_it.cycled_list();
       outline_it.forward()) {
    // Iterate over outline POLYPT_LIST and create the corresponding
    // ICOORDELT_LIST
    ICOORDELT_LIST icoordelts;
    ICOORDELT_IT icoordelt_it(&icoordelts);
    POLYPT_LIST* polypts = outline_it.data()->polypts();
    POLYPT_IT polypt_it(polypts);

    for (polypt_it.mark_cycle_pt();
         !polypt_it.cycled_list();
         polypt_it.forward()) {
      ICOORD current_icoord = ICOORD((inT16) polypt_it.data()->pos.x(),
                                     (inT16) polypt_it.data()->pos.y());
      icoordelt_it.add_after_then_move(new ICOORDELT(current_icoord));
    }

    // Create the POLY_BLOCK from the ICOORDELT_LIST
    POLY_BLOCK* poly_block_outline = new POLY_BLOCK(&icoordelts, POLY_X);
    icoordelts.clear();

    // Add this text block to the block list
    int xmin = poly_block_outline->bounding_box()->left();
    int ymin = poly_block_outline->bounding_box()->bottom();
    int xmax = poly_block_outline->bounding_box()->right();
    int ymax = poly_block_outline->bounding_box()->top();
    append_block(blocks, xmin, ymin, xmax, ymax, poly_block_outline);
  }
}

void mask_to_blocks(IMAGE* textblock_mask_image, BLOCK_LIST *blocks) {
  OUTLINE_LIST outlines;

  mask_to_outlines(textblock_mask_image, &outlines);
  outlines_to_blocks(&outlines, blocks);
  outlines.clear();
}


#endif  // HAVE_LIBLEPT


void append_block(BLOCK_LIST *blocks,
                  int xmin, int ymin,
                  int xmax, int ymax,
                  POLY_BLOCK* outline) {
  BLOCK *block;
  BLOCK_IT block_it = blocks;

  block = new BLOCK("", TRUE, 0, 0, xmin, ymin, xmax, ymax);
  if (outline != NULL)
    block->set_poly_block(outline);
  block_it.add_to_end(block);
}

void append_full_page_block(BLOCK_LIST *blocks) {
  append_block(blocks,
               0, 0,
               page_image.get_xsize(), page_image.get_ysize(),
               NULL);
}
