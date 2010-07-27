/**********************************************************************
 * File:        pagesegmain.cpp
 * Description: Top-level page segmenter for Tesseract.
 * Author:      Ray Smith
 * Created:     Thu Sep 25 17:12:01 PDT 2008
 *
 * (C) Copyright 2008, Google Inc.
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

#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
#ifdef _MSC_VER
#pragma warning(disable:4244)  // Conversion warnings
#endif

// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif

#ifdef HAVE_LIBLEPT
// Include leptonica library only if autoconf (or makefile etc) tell us to.
#include "allheaders.h"
#endif

#include "tesseractclass.h"
#include "img.h"
#include "blobbox.h"
#include "linefind.h"
#include "imagefind.h"
#include "colfind.h"
#include "tabvector.h"
#include "blread.h"
#include "wordseg.h"
#include "makerow.h"
#include "baseapi.h"
#include "tordmain.h"
#include "tessvars.h"

namespace tesseract {

/// Minimum believable resolution.
const int kMinCredibleResolution = 70;
/// Default resolution used if input in not believable.
const int kDefaultResolution = 300;

/**
 * Segment the page according to the current value of tessedit_pageseg_mode.
 * If the pix_binary_ member is not NULL, it is used as the source image,
 * and copied to image, otherwise it just uses image as the input.
 * On return the blocks list owns all the constructed page layout.
 */
int Tesseract::SegmentPage(const STRING* input_file,
                           IMAGE* image, BLOCK_LIST* blocks) {
  int width = image->get_xsize();
  int height = image->get_ysize();
  int resolution = image->get_res();
#ifdef HAVE_LIBLEPT
  if (pix_binary_ != NULL) {
    width = pixGetWidth(pix_binary_);
    height = pixGetHeight(pix_binary_);
    resolution = pixGetXRes(pix_binary_);
  }
#endif
  // Zero resolution messes up the algorithms, so make sure it is credible.
  if (resolution < kMinCredibleResolution)
    resolution = kDefaultResolution;
  // Get page segmentation mode.
  PageSegMode pageseg_mode = static_cast<PageSegMode>(
      static_cast<int>(tessedit_pageseg_mode));
  // If a UNLV zone file can be found, use that instead of segmentation.
  if (pageseg_mode != tesseract::PSM_AUTO &&
      input_file != NULL && input_file->length() > 0) {
    STRING name = *input_file;
    const char* lastdot = strrchr(name.string(), '.');
    if (lastdot != NULL)
      name[lastdot - name.string()] = '\0';
    read_unlv_file(name, width, height, blocks);
  }
  bool single_column = pageseg_mode > PSM_AUTO;
  if (blocks->empty()) {
    // No UNLV file present. Work according to the PageSegMode.
    // First make a single block covering the whole image.
    BLOCK_IT block_it(blocks);
    BLOCK* block = new BLOCK("", TRUE, 0, 0, 0, 0, width, height);
    block_it.add_to_end(block);
  } else {
    // UNLV file present. Use PSM_SINGLE_COLUMN.
    pageseg_mode = PSM_SINGLE_COLUMN;
  }

  TO_BLOCK_LIST land_blocks, port_blocks;
  TBOX page_box;
  if (pageseg_mode <= PSM_SINGLE_COLUMN) {
    if (AutoPageSeg(width, height, resolution, single_column,
                    image, blocks, &port_blocks) < 0) {
      return -1;
    }
    // To create blobs from the image region bounds uncomment this line:
    //  port_blocks.clear();  // Uncomment to go back to the old mode.
  } else {
#if HAVE_LIBLEPT
    image->FromPix(pix_binary_);
#endif
    deskew_ = FCOORD(1.0f, 0.0f);
    reskew_ = FCOORD(1.0f, 0.0f);
  }
  if (blocks->empty()) {
    tprintf("Empty page\n");
    return 0;  // AutoPageSeg found an empty page.
  }

  if (port_blocks.empty()) {
    // AutoPageSeg was not used, so we need to find_components first.
    find_components(blocks, &land_blocks, &port_blocks, &page_box);
  } else {
    // AutoPageSeg does not need to find_components as it did that already.
    page_box.set_left(0);
    page_box.set_bottom(0);
    page_box.set_right(width);
    page_box.set_top(height);
    // Filter_blobs sets up the TO_BLOCKs the same as find_components does.
    filter_blobs(page_box.topright(), &port_blocks, true);
  }

  TO_BLOCK_IT to_block_it(&port_blocks);
  ASSERT_HOST(!port_blocks.empty());
  TO_BLOCK* to_block = to_block_it.data();
  if (pageseg_mode <= PSM_SINGLE_BLOCK ||
      to_block->line_size < 2) {
    // For now, AUTO, SINGLE_COLUMN and SINGLE_BLOCK all map to the old
    // textord. The difference is the number of blocks and how the are made.
    textord_page(page_box.topright(), blocks, &land_blocks, &port_blocks,
                 this);
  } else {
    // SINGLE_LINE, SINGLE_WORD and SINGLE_CHAR all need a single row.
    float gradient = make_single_row(page_box.topright(),
                                     to_block, &port_blocks, this);
    if (pageseg_mode == PSM_SINGLE_LINE) {
      // SINGLE_LINE uses the old word maker on the single line.
      make_words(page_box.topright(), gradient, blocks,
                 &land_blocks, &port_blocks, this);
    } else {
      // SINGLE_WORD and SINGLE_CHAR cram all the blobs into a
      // single word, and in SINGLE_CHAR mode, all the outlines
      // go in a single blob.
      make_single_word(pageseg_mode == PSM_SINGLE_CHAR,
                       to_block->get_rows(), to_block->block->row_list());
    }
  }
  return 0;
}

/**
 * Auto page segmentation. Divide the page image into blocks of uniform
 * text linespacing and images.
 *
 * Width, height and resolution are derived from the input image.
 *
 * If the pix is non-NULL, then it is assumed to be the input, and it is
 * copied to the image, otherwise the image is used directly.
 *
 * The output goes in the blocks list with corresponding TO_BLOCKs in the
 * to_blocks list.
 *
 * If single_column is true, then no attempt is made to divide the image
 * into columns, but multiple blocks are still made if the text is of
 * non-uniform linespacing.
 */
int Tesseract::AutoPageSeg(int width, int height, int resolution,
                           bool single_column, IMAGE* image,
                           BLOCK_LIST* blocks, TO_BLOCK_LIST* to_blocks) {
  int vertical_x = 0;
  int vertical_y = 1;
  TabVector_LIST v_lines;
  TabVector_LIST h_lines;
  ICOORD bleft(0, 0);
  Boxa* boxa = NULL;
  Pixa* pixa = NULL;
  // The blocks made by the ColumnFinder. Moved to blocks before return.
  BLOCK_LIST found_blocks;

#ifdef HAVE_LIBLEPT
  if (pix_binary_ != NULL) {
    if (textord_debug_images) {
      Pix* grey_pix = pixCreate(width, height, 8);
      // Printable images are light grey on white, but for screen display
      // they are black on dark grey so the other colors show up well.
      if (textord_debug_printable) {
        pixSetAll(grey_pix);
        pixSetMasked(grey_pix, pix_binary_, 192);
      } else {
        pixSetAllArbitrary(grey_pix, 64);
        pixSetMasked(grey_pix, pix_binary_, 0);
      }
      AlignedBlob::IncrementDebugPix();
      pixWrite(AlignedBlob::textord_debug_pix().string(), grey_pix, IFF_PNG);
      pixDestroy(&grey_pix);
    }
    if (tessedit_dump_pageseg_images)
      pixWrite("tessinput.png", pix_binary_, IFF_PNG);
    // Leptonica is used to find the lines and image regions in the input.
    LineFinder::FindVerticalLines(resolution, pix_binary_,
                                  &vertical_x, &vertical_y, &v_lines);
    LineFinder::FindHorizontalLines(resolution, pix_binary_, &h_lines);
    if (tessedit_dump_pageseg_images)
      pixWrite("tessnolines.png", pix_binary_, IFF_PNG);
    ImageFinder::FindImages(pix_binary_, &boxa, &pixa);
    if (tessedit_dump_pageseg_images)
      pixWrite("tessnoimages.png", pix_binary_, IFF_PNG);
    // Copy the Pix to the IMAGE.
    image->FromPix(pix_binary_);
    if (single_column)
      v_lines.clear();
  }
#endif
  TO_BLOCK_LIST land_blocks, port_blocks;
  TBOX page_box;
  // The rest of the algorithm uses the usual connected components.
  find_components(blocks, &land_blocks, &port_blocks, &page_box);

  TO_BLOCK_IT to_block_it(&port_blocks);
  ASSERT_HOST(!to_block_it.empty());
  for (to_block_it.mark_cycle_pt(); !to_block_it.cycled_list();
       to_block_it.forward()) {
    TO_BLOCK* to_block = to_block_it.data();
    TBOX blkbox = to_block->block->bounding_box();
    if (to_block->line_size >= 2) {
      // Note: if there are multiple blocks, then v_lines, boxa, and pixa
      // are empty on the next iteration, but in this case, we assume
      // that there aren't any interesting line separators or images, since
      // it means that we have a pre-defined unlv zone file.
      ColumnFinder finder(static_cast<int>(to_block->line_size),
                          blkbox.botleft(), blkbox.topright(),
                          &v_lines, &h_lines, vertical_x, vertical_y);
      if (finder.FindBlocks(height, resolution, single_column,
                            to_block, boxa, pixa, &found_blocks, to_blocks) < 0)
        return -1;
      finder.ComputeDeskewVectors(&deskew_, &reskew_);
      boxa = NULL;
      pixa = NULL;
    }
  }
#ifdef HAVE_LIBLEPT
  boxaDestroy(&boxa);
  pixaDestroy(&pixa);
#endif
  blocks->clear();
  BLOCK_IT block_it(blocks);
  // Move the found blocks to the input/output blocks.
  block_it.add_list_after(&found_blocks);

  if (textord_debug_images) {
    // The debug image is no longer needed so delete it.
    unlink(AlignedBlob::textord_debug_pix().string());
  }
  return 0;
}

}  // namespace tesseract.
