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
#include "osdetect.h"
#include "textord.h"
#include "tordmain.h"
#include "tessvars.h"

namespace tesseract {

/// Minimum believable resolution.
const int kMinCredibleResolution = 70;
/// Default resolution used if input in not believable.
const int kDefaultResolution = 300;
// Max erosions to perform in removing an enclosing circle.
const int kMaxCircleErosions = 8;

// Helper to remove an enclosing circle from an image.
// If there isn't one, then the image will most likely get badly mangled.
// The returned pix must be pixDestroyed after use. NULL may be returned
// if the image doesn't meet the trivial conditions that it uses to determine
// success.
static Pix* RemoveEnclosingCircle(Pix* pixs) {
  Pix* pixsi = pixInvert(NULL, pixs);
  Pix* pixc = pixCreateTemplate(pixs);
  pixSetOrClearBorder(pixc, 1, 1, 1, 1, PIX_SET);
  pixSeedfillBinary(pixc, pixc, pixsi, 4);
  pixInvert(pixc, pixc);
  pixDestroy(&pixsi);
  Pix* pixt = pixAnd(NULL, pixs, pixc);
  l_int32 max_count;
  pixCountConnComp(pixt, 8, &max_count);
  // The count has to go up before we start looking for the minimum.
  l_int32 min_count = MAX_INT32;
  Pix* pixout = NULL;
  for (int i = 1; i < kMaxCircleErosions; i++) {
    pixDestroy(&pixt);
    pixErodeBrick(pixc, pixc, 3, 3);
    pixt = pixAnd(NULL, pixs, pixc);
    l_int32 count;
    pixCountConnComp(pixt, 8, &count);
    if (i == 1 || count > max_count) {
      max_count = count;
      min_count = count;
    } else if (i > 1 && count < min_count) {
      min_count = count;
      pixDestroy(&pixout);
      pixout = pixCopy(NULL, pixt);  // Save the best.
    } else if (count >= min_count) {
      break;  // We have passed by the best.
    }
  }
  pixDestroy(&pixt);
  pixDestroy(&pixc);
  return pixout;
}

/**
 * Segment the page according to the current value of tessedit_pageseg_mode.
 * If the pix_binary_ member is not NULL, it is used as the source image,
 * and copied to image, otherwise it just uses image as the input.
 * On return the blocks list owns all the constructed page layout.
 */
int Tesseract::SegmentPage(const STRING* input_file, BLOCK_LIST* blocks,
                           Tesseract* osd_tess, OSResults* osr) {
  ASSERT_HOST(pix_binary_ != NULL);
  int width = pixGetWidth(pix_binary_);
  int height = pixGetHeight(pix_binary_);
  int resolution = pixGetXRes(pix_binary_);
  // Zero resolution messes up the algorithms, so make sure it is credible.
  if (resolution < kMinCredibleResolution)
    resolution = kDefaultResolution;
  // Get page segmentation mode.
  PageSegMode pageseg_mode = static_cast<PageSegMode>(
      static_cast<int>(tessedit_pageseg_mode));
  // If a UNLV zone file can be found, use that instead of segmentation.
  if (!PSM_COL_FIND_ENABLED(pageseg_mode) &&
      input_file != NULL && input_file->length() > 0) {
    STRING name = *input_file;
    const char* lastdot = strrchr(name.string(), '.');
    if (lastdot != NULL)
      name[lastdot - name.string()] = '\0';
    read_unlv_file(name, width, height, blocks);
  }
  if (blocks->empty()) {
    // No UNLV file present. Work according to the PageSegMode.
    // First make a single block covering the whole image.
    BLOCK_IT block_it(blocks);
    BLOCK* block = new BLOCK("", TRUE, 0, 0, 0, 0, width, height);
    block->set_right_to_left(right_to_left());
    block_it.add_to_end(block);
  } else {
    // UNLV file present. Use PSM_SINGLE_COLUMN.
    pageseg_mode = PSM_SINGLE_COLUMN;
  }
  bool single_column = !PSM_COL_FIND_ENABLED(pageseg_mode);
  bool osd_enabled = PSM_OSD_ENABLED(pageseg_mode);
  bool osd_only = pageseg_mode == PSM_OSD_ONLY;

  int auto_page_seg_ret_val = 0;
  TO_BLOCK_LIST to_blocks;
  if (osd_enabled || PSM_BLOCK_FIND_ENABLED(pageseg_mode)) {
    auto_page_seg_ret_val =
        AutoPageSeg(resolution, single_column, osd_enabled, osd_only,
                    blocks, &to_blocks, osd_tess, osr);
    if (osd_only)
      return auto_page_seg_ret_val;
    // To create blobs from the image region bounds uncomment this line:
    //  to_blocks.clear();  // Uncomment to go back to the old mode.
  } else {
    deskew_ = FCOORD(1.0f, 0.0f);
    reskew_ = FCOORD(1.0f, 0.0f);
    if (pageseg_mode == PSM_CIRCLE_WORD) {
      Pix* pixcleaned = RemoveEnclosingCircle(pix_binary_);
      if (pixcleaned != NULL) {
        pixDestroy(&pix_binary_);
        pix_binary_ = pixcleaned;
      }
    }
  }

  if (auto_page_seg_ret_val < 0) {
    return -1;
  }

  if (blocks->empty()) {
    tprintf("Empty page\n");
    return 0;  // AutoPageSeg found an empty page.
  }

  textord_.TextordPage(pageseg_mode, width, height, pix_binary_,
                       blocks, &to_blocks);
  SetupWordScripts(blocks);
  return auto_page_seg_ret_val;
}

// TODO(rays) This is a hack to set all the words with a default script.
// In the future this will be set by a preliminary pass over the document.
void Tesseract::SetupWordScripts(BLOCK_LIST* blocks) {
  int script = unicharset.default_sid();
  bool has_x_height = unicharset.script_has_xheight();
  bool is_latin = script == unicharset.latin_sid();
  BLOCK_IT b_it(blocks);
  for (b_it.mark_cycle_pt(); !b_it.cycled_list(); b_it.forward()) {
    ROW_IT r_it(b_it.data()->row_list());
    for (r_it.mark_cycle_pt(); !r_it.cycled_list(); r_it.forward()) {
      WERD_IT w_it(r_it.data()->word_list());
      for (w_it.mark_cycle_pt(); !w_it.cycled_list(); w_it.forward()) {
        WERD* word = w_it.data();
        word->set_script_id(script);
        word->set_flag(W_SCRIPT_HAS_XHEIGHT, has_x_height);
        word->set_flag(W_SCRIPT_IS_LATIN, is_latin);
      }
    }
  }
}


/**
 * Auto page segmentation. Divide the page image into blocks of uniform
 * text linespacing and images.
 *
 * Resolution (in ppi) is derived from the input image.
 *
 * The output goes in the blocks list with corresponding TO_BLOCKs in the
 * to_blocks list.
 *
 * If single_column is true, then no attempt is made to divide the image
 * into columns, but multiple blocks are still made if the text is of
 * non-uniform linespacing.
 *
 * If osd is true, then orientation and script detection is performed as well.
 * If only_osd is true, then only orientation and script detection is
 * performed. If osr is desired, the osr_tess must be another Tesseract
 * that was initialized especially for osd, and the results will be output
 * into osr.
 */
int Tesseract::AutoPageSeg(int resolution, bool single_column,
                           bool osd, bool only_osd,
                           BLOCK_LIST* blocks, TO_BLOCK_LIST* to_blocks,
                           Tesseract* osd_tess, OSResults* osr) {
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
      Pix* grey_pix = pixCreate(pixGetWidth(pix_binary_),
                                pixGetHeight(pix_binary_), 8);
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
    if (tessedit_dump_pageseg_images) {
      pixWrite("tessinput.png", pix_binary_, IFF_PNG);
    }
    // Leptonica is used to find the lines and image regions in the input.
    LineFinder::FindVerticalLines(resolution, pix_binary_,
                                  &vertical_x, &vertical_y, &v_lines);
    LineFinder::FindHorizontalLines(resolution, pix_binary_, &h_lines);
    if (tessedit_dump_pageseg_images)
      pixWrite("tessnolines.png", pix_binary_, IFF_PNG);
    ImageFinder::FindImages(pix_binary_, &boxa, &pixa);
    if (tessedit_dump_pageseg_images)
      pixWrite("tessnoimages.png", pix_binary_, IFF_PNG);
    if (single_column)
      v_lines.clear();
  }
#endif
  TO_BLOCK_LIST port_blocks;
  // The rest of the algorithm uses the usual connected components.
  textord_.find_components(pix_binary_, blocks, &port_blocks);

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
                          blkbox.botleft(), blkbox.topright(), resolution,
                          &v_lines, &h_lines, vertical_x, vertical_y);
      BLOBNBOX_CLIST osd_blobs;
      int osd_orientation = 0;
      bool vertical_text = finder.IsVerticallyAlignedText(to_block, &osd_blobs);
      if (osd && osd_tess != NULL && osr != NULL) {
        os_detect_blobs(&osd_blobs, osr, osd_tess);
        if (only_osd) continue;
        osd_orientation = osr->best_result.orientation_id;
        double osd_score = osr->orientations[osd_orientation];
        double osd_margin = min_orientation_margin * 2;
        // tprintf("Orientation scores:");
        for (int i = 0; i < 4; ++i) {
          if (i != osd_orientation &&
              osd_score - osr->orientations[i] < osd_margin) {
            osd_margin = osd_score - osr->orientations[i];
          }
          // tprintf(" %d:%f", i, osr->orientations[i]);
        }
        // tprintf("\n");
        if (osd_margin < min_orientation_margin) {
          // Margin insufficient - dream up a suitable default.
          if (vertical_text && (osd_orientation & 1))
            osd_orientation = 3;
          else
            osd_orientation = 0;
          tprintf("Score margin insufficient:%.2f, using %d as a default\n",
                  osd_margin, osd_orientation);
        }
      }
      osd_blobs.shallow_clear();
      finder.CorrectOrientation(to_block, vertical_text, osd_orientation);
      if (finder.FindBlocks(single_column, pixGetHeight(pix_binary_),
                            to_block, boxa, pixa, &found_blocks, to_blocks) < 0)
        return -1;
      finder.GetDeskewVectors(&deskew_, &reskew_);
      boxa = NULL;
      pixa = NULL;
    }
  }
  boxaDestroy(&boxa);
  pixaDestroy(&pixa);
  if (only_osd) return 0;

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
