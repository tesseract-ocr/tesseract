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

#ifdef _WIN32
#ifndef __GNUC__
#include <windows.h>
#endif  /* __GNUC__ */
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

#include "allheaders.h"
#include "blobbox.h"
#include "blread.h"
#include "colfind.h"
#include "equationdetect.h"
#include "imagefind.h"
#include "linefind.h"
#include "makerow.h"
#include "osdetect.h"
#include "tabvector.h"
#include "tesseractclass.h"
#include "tessvars.h"
#include "textord.h"
#include "tordmain.h"
#include "wordseg.h"

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
 * pix_binary_ is used as the source image and should not be NULL.
 * On return the blocks list owns all the constructed page layout.
 */
int Tesseract::SegmentPage(const STRING* input_file, BLOCK_LIST* blocks,
                           Tesseract* osd_tess, OSResults* osr) {
  ASSERT_HOST(pix_binary_ != NULL);
  int width = pixGetWidth(pix_binary_);
  int height = pixGetHeight(pix_binary_);
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
    // UNLV file present. Use PSM_SINGLE_BLOCK.
    pageseg_mode = PSM_SINGLE_BLOCK;
  }
  int auto_page_seg_ret_val = 0;
  TO_BLOCK_LIST to_blocks;
  if (PSM_OSD_ENABLED(pageseg_mode) || PSM_BLOCK_FIND_ENABLED(pageseg_mode) ||
      PSM_SPARSE(pageseg_mode)) {
    auto_page_seg_ret_val =
        AutoPageSeg(pageseg_mode, blocks, &to_blocks, osd_tess, osr);
    if (pageseg_mode == PSM_OSD_ONLY)
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
    if (textord_debug_tabfind)
      tprintf("Empty page\n");
    return 0;  // AutoPageSeg found an empty page.
  }
  bool splitting =
      pageseg_devanagari_split_strategy != ShiroRekhaSplitter::NO_SPLIT;
  bool cjk_mode = textord_use_cjk_fp_model;

  textord_.TextordPage(pageseg_mode, reskew_, width, height, pix_binary_,
                       pix_thresholds_, pix_grey_, splitting || cjk_mode,
                       blocks, &to_blocks);
  return auto_page_seg_ret_val;
}

// Helper writes a grey image to a file for use by scrollviewer.
// Normally for speed we don't display the image in the layout debug windows.
// If textord_debug_images is true, we draw the image as a background to some
// of the debug windows. printable determines whether these
// images are optimized for printing instead of screen display.
static void WriteDebugBackgroundImage(bool printable, Pix* pix_binary) {
  Pix* grey_pix = pixCreate(pixGetWidth(pix_binary),
                            pixGetHeight(pix_binary), 8);
  // Printable images are light grey on white, but for screen display
  // they are black on dark grey so the other colors show up well.
  if (printable) {
    pixSetAll(grey_pix);
    pixSetMasked(grey_pix, pix_binary, 192);
  } else {
    pixSetAllArbitrary(grey_pix, 64);
    pixSetMasked(grey_pix, pix_binary, 0);
  }
  AlignedBlob::IncrementDebugPix();
  pixWrite(AlignedBlob::textord_debug_pix().string(), grey_pix, IFF_PNG);
  pixDestroy(&grey_pix);
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
 * If osd (orientation and script detection) is true then that is performed
 * as well. If only_osd is true, then only orientation and script detection is
 * performed. If osd is desired, (osd or only_osd) then osr_tess must be
 * another Tesseract that was initialized especially for osd, and the results
 * will be output into osr (orientation and script result).
 */
int Tesseract::AutoPageSeg(PageSegMode pageseg_mode,
                           BLOCK_LIST* blocks, TO_BLOCK_LIST* to_blocks,
                           Tesseract* osd_tess, OSResults* osr) {
  if (textord_debug_images) {
    WriteDebugBackgroundImage(textord_debug_printable, pix_binary_);
  }
  Pix* photomask_pix = NULL;
  Pix* musicmask_pix = NULL;
  // The blocks made by the ColumnFinder. Moved to blocks before return.
  BLOCK_LIST found_blocks;
  TO_BLOCK_LIST temp_blocks;

  bool single_column = !PSM_COL_FIND_ENABLED(pageseg_mode);
  bool osd_enabled = PSM_OSD_ENABLED(pageseg_mode);
  bool osd_only = pageseg_mode == PSM_OSD_ONLY;
  ColumnFinder* finder = SetupPageSegAndDetectOrientation(
      single_column, osd_enabled, osd_only, blocks, osd_tess, osr,
      &temp_blocks, &photomask_pix, &musicmask_pix);
  int result = 0;
  if (finder != NULL) {
    TO_BLOCK_IT to_block_it(&temp_blocks);
    TO_BLOCK* to_block = to_block_it.data();
    if (musicmask_pix != NULL) {
      // TODO(rays) pass the musicmask_pix into FindBlocks and mark music
      // blocks separately. For now combine with photomask_pix.
      pixOr(photomask_pix, photomask_pix, musicmask_pix);
    }
    if (equ_detect_) {
      finder->SetEquationDetect(equ_detect_);
    }
    result = finder->FindBlocks(pageseg_mode, scaled_color_, scaled_factor_,
                                to_block, photomask_pix,
                                pix_thresholds_, pix_grey_,
                                &found_blocks, to_blocks);
    if (result >= 0)
      finder->GetDeskewVectors(&deskew_, &reskew_);
    delete finder;
  }
  pixDestroy(&photomask_pix);
  pixDestroy(&musicmask_pix);
  if (result < 0) return result;

  blocks->clear();
  BLOCK_IT block_it(blocks);
  // Move the found blocks to the input/output blocks.
  block_it.add_list_after(&found_blocks);

  if (textord_debug_images) {
    // The debug image is no longer needed so delete it.
    unlink(AlignedBlob::textord_debug_pix().string());
  }
  return result;
}

/**
 * Sets up auto page segmentation, determines the orientation, and corrects it.
 * Somewhat arbitrary chunk of functionality, factored out of AutoPageSeg to
 * facilitate testing.
 * photo_mask_pix is a pointer to a NULL pointer that will be filled on return
 * with the leptonica photo mask, which must be pixDestroyed by the caller.
 * to_blocks is an empty list that will be filled with (usually a single)
 * block that is used during layout analysis. This ugly API is required
 * because of the possibility of a unlv zone file.
 * TODO(rays) clean this up.
 * See AutoPageSeg for other arguments.
 * The returned ColumnFinder must be deleted after use.
 */
ColumnFinder* Tesseract::SetupPageSegAndDetectOrientation(
    bool single_column, bool osd, bool only_osd,
    BLOCK_LIST* blocks, Tesseract* osd_tess, OSResults* osr,
    TO_BLOCK_LIST* to_blocks, Pix** photo_mask_pix, Pix** music_mask_pix) {
  int vertical_x = 0;
  int vertical_y = 1;
  TabVector_LIST v_lines;
  TabVector_LIST h_lines;
  ICOORD bleft(0, 0);

  ASSERT_HOST(pix_binary_ != NULL);
  if (tessedit_dump_pageseg_images) {
    pixWrite("tessinput.png", pix_binary_, IFF_PNG);
  }
  // Leptonica is used to find the rule/separator lines in the input.
  LineFinder::FindAndRemoveLines(source_resolution_,
                                 textord_tabfind_show_vlines, pix_binary_,
                                 &vertical_x, &vertical_y, music_mask_pix,
                                 &v_lines, &h_lines);
  if (tessedit_dump_pageseg_images)
    pixWrite("tessnolines.png", pix_binary_, IFF_PNG);
  // Leptonica is used to find a mask of the photo regions in the input.
  *photo_mask_pix = ImageFind::FindImages(pix_binary_);
  if (tessedit_dump_pageseg_images)
    pixWrite("tessnoimages.png", pix_binary_, IFF_PNG);
  if (single_column)
    v_lines.clear();

  // The rest of the algorithm uses the usual connected components.
  textord_.find_components(pix_binary_, blocks, to_blocks);

  TO_BLOCK_IT to_block_it(to_blocks);
  // There must be exactly one input block.
  // TODO(rays) handle new textline finding with a UNLV zone file.
  ASSERT_HOST(to_blocks->singleton());
  TO_BLOCK* to_block = to_block_it.data();
  TBOX blkbox = to_block->block->bounding_box();
  ColumnFinder* finder = NULL;

  if (to_block->line_size >= 2) {
    finder = new ColumnFinder(static_cast<int>(to_block->line_size),
                              blkbox.botleft(), blkbox.topright(),
                              source_resolution_,
                              &v_lines, &h_lines, vertical_x, vertical_y);

    finder->SetupAndFilterNoise(*photo_mask_pix, to_block);

    if (equ_detect_) {
      equ_detect_->LabelSpecialText(to_block);
    }

    BLOBNBOX_CLIST osd_blobs;
    // osd_orientation is the number of 90 degree rotations to make the
    // characters upright. (See osdetect.h for precise definition.)
    // We want the text lines horizontal, (vertical text indicates vertical
    // textlines) which may conflict (eg vertically written CJK).
    int osd_orientation = 0;
    bool vertical_text = finder->IsVerticallyAlignedText(to_block, &osd_blobs);
    if (osd && osd_tess != NULL && osr != NULL) {
      os_detect_blobs(&osd_blobs, osr, osd_tess);
      if (only_osd) {
        delete finder;
        return NULL;
      }
      osd_orientation = osr->best_result.orientation_id;
      double osd_score = osr->orientations[osd_orientation];
      double osd_margin = min_orientation_margin * 2;
      for (int i = 0; i < 4; ++i) {
        if (i != osd_orientation &&
            osd_score - osr->orientations[i] < osd_margin) {
          osd_margin = osd_score - osr->orientations[i];
        }
      }
      if (osd_margin < min_orientation_margin) {
        // The margin is weak.
        int best_script_id = osr->best_result.script_id;
        bool cjk = (best_script_id == osd_tess->unicharset.han_sid()) ||
            (best_script_id == osd_tess->unicharset.hiragana_sid()) ||
            (best_script_id == osd_tess->unicharset.katakana_sid());

        if (!cjk && !vertical_text && osd_orientation == 2) {
          // upside down latin text is improbable with such a weak margin.
          tprintf("OSD: Weak margin (%.2f), horiz textlines, not CJK: "
                  "Don't rotate.\n", osd_margin);
          osd_orientation = 0;
        } else {
          tprintf("OSD: Weak margin (%.2f) for %d blob text block, "
                  "but using orientation anyway: %d\n",
                  osd_blobs.length(), osd_margin, osd_orientation);
        }
      }
    }
    osd_blobs.shallow_clear();
    finder->CorrectOrientation(to_block, vertical_text, osd_orientation);
  }

  return finder;
}

}  // namespace tesseract.
