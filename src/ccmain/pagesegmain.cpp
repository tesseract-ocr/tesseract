/**********************************************************************
 * File:        pagesegmain.cpp
 * Description: Top-level page segmenter for Tesseract.
 * Author:      Ray Smith
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
#ifndef unlink
#include <io.h>
#endif
#else
#include <unistd.h>
#endif  // _WIN32

// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif

#include "allheaders.h"
#include "blobbox.h"
#include "blread.h"
#include "colfind.h"
#include "debugpixa.h"
#include "equationdetect.h"
#include "imagefind.h"
#include "linefind.h"
#include "makerow.h"
#include <tesseract/osdetect.h>
#include "tabvector.h"
#include "tesseractclass.h"
#include "tessvars.h"
#include "textord.h"
#include "tordmain.h"
#include "wordseg.h"

namespace tesseract {

// Max erosions to perform in removing an enclosing circle.
const int kMaxCircleErosions = 8;

// Helper to remove an enclosing circle from an image.
// If there isn't one, then the image will most likely get badly mangled.
// The returned pix must be pixDestroyed after use. nullptr may be returned
// if the image doesn't meet the trivial conditions that it uses to determine
// success.
static Pix* RemoveEnclosingCircle(Pix* pixs) {
  Pix* pixsi = pixInvert(nullptr, pixs);
  Pix* pixc = pixCreateTemplate(pixs);
  pixSetOrClearBorder(pixc, 1, 1, 1, 1, PIX_SET);
  pixSeedfillBinary(pixc, pixc, pixsi, 4);
  pixInvert(pixc, pixc);
  pixDestroy(&pixsi);
  Pix* pixt = pixAnd(nullptr, pixs, pixc);
  l_int32 max_count;
  pixCountConnComp(pixt, 8, &max_count);
  // The count has to go up before we start looking for the minimum.
  l_int32 min_count = INT32_MAX;
  Pix* pixout = nullptr;
  for (int i = 1; i < kMaxCircleErosions; i++) {
    pixDestroy(&pixt);
    pixErodeBrick(pixc, pixc, 3, 3);
    pixt = pixAnd(nullptr, pixs, pixc);
    l_int32 count;
    pixCountConnComp(pixt, 8, &count);
    if (i == 1 || count > max_count) {
      max_count = count;
      min_count = count;
    } else if (i > 1 && count < min_count) {
      min_count = count;
      pixDestroy(&pixout);
      pixout = pixCopy(nullptr, pixt);  // Save the best.
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
 * pix_binary_ is used as the source image and should not be nullptr.
 * On return the blocks list owns all the constructed page layout.
 */
int Tesseract::SegmentPage(const STRING* input_file, BLOCK_LIST* blocks,
                           Tesseract* osd_tess, OSResults* osr) {
  ASSERT_HOST(pix_binary_ != nullptr);
  int width = pixGetWidth(pix_binary_);
  int height = pixGetHeight(pix_binary_);
  // Get page segmentation mode.
  auto pageseg_mode = static_cast<PageSegMode>(
      static_cast<int>(tessedit_pageseg_mode));
  // If a UNLV zone file can be found, use that instead of segmentation.
  if (!PSM_COL_FIND_ENABLED(pageseg_mode) &&
      input_file != nullptr && input_file->length() > 0) {
    STRING name = *input_file;
    const char* lastdot = strrchr(name.c_str(), '.');
    if (lastdot != nullptr)
      name[lastdot - name.c_str()] = '\0';
    read_unlv_file(name, width, height, blocks);
  }
  if (blocks->empty()) {
    // No UNLV file present. Work according to the PageSegMode.
    // First make a single block covering the whole image.
    BLOCK_IT block_it(blocks);
    auto* block = new BLOCK("", true, 0, 0, 0, 0, width, height);
    block->set_right_to_left(right_to_left());
    block_it.add_to_end(block);
  } else {
    // UNLV file present. Use PSM_SINGLE_BLOCK.
    pageseg_mode = PSM_SINGLE_BLOCK;
  }
  // The diacritic_blobs holds noise blobs that may be diacritics. They
  // are separated out on areas of the image that seem noisy and short-circuit
  // the layout process, going straight from the initial partition creation
  // right through to after word segmentation, where they are added to the
  // rej_cblobs list of the most appropriate word. From there classification
  // will determine whether they are used.
  BLOBNBOX_LIST diacritic_blobs;
  int auto_page_seg_ret_val = 0;
  TO_BLOCK_LIST to_blocks;
  if (PSM_OSD_ENABLED(pageseg_mode) || PSM_BLOCK_FIND_ENABLED(pageseg_mode) ||
      PSM_SPARSE(pageseg_mode)) {
    auto_page_seg_ret_val = AutoPageSeg(
        pageseg_mode, blocks, &to_blocks,
        enable_noise_removal ? &diacritic_blobs : nullptr, osd_tess, osr);
    if (pageseg_mode == PSM_OSD_ONLY)
      return auto_page_seg_ret_val;
    // To create blobs from the image region bounds uncomment this line:
    //  to_blocks.clear();  // Uncomment to go back to the old mode.
  } else {
    deskew_ = FCOORD(1.0f, 0.0f);
    reskew_ = FCOORD(1.0f, 0.0f);
    if (pageseg_mode == PSM_CIRCLE_WORD) {
      Pix* pixcleaned = RemoveEnclosingCircle(pix_binary_);
      if (pixcleaned != nullptr) {
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
                       &diacritic_blobs, blocks, &to_blocks);
  return auto_page_seg_ret_val;
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
 * If !PSM_COL_FIND_ENABLED(pageseg_mode), then no attempt is made to divide
 * the image into columns, but multiple blocks are still made if the text is
 * of non-uniform linespacing.
 *
 * If diacritic_blobs is non-null, then diacritics/noise blobs, that would
 * confuse layout analysis by causing textline overlap, are placed there,
 * with the expectation that they will be reassigned to words later and
 * noise/diacriticness determined via classification.
 *
 * If osd (orientation and script detection) is true then that is performed
 * as well. If only_osd is true, then only orientation and script detection is
 * performed. If osd is desired, (osd or only_osd) then osr_tess must be
 * another Tesseract that was initialized especially for osd, and the results
 * will be output into osr (orientation and script result).
 */
int Tesseract::AutoPageSeg(PageSegMode pageseg_mode, BLOCK_LIST* blocks,
                           TO_BLOCK_LIST* to_blocks,
                           BLOBNBOX_LIST* diacritic_blobs, Tesseract* osd_tess,
                           OSResults* osr) {
  Pix* photomask_pix = nullptr;
  Pix* musicmask_pix = nullptr;
  // The blocks made by the ColumnFinder. Moved to blocks before return.
  BLOCK_LIST found_blocks;
  TO_BLOCK_LIST temp_blocks;

  ColumnFinder* finder = SetupPageSegAndDetectOrientation(
      pageseg_mode, blocks, osd_tess, osr, &temp_blocks, &photomask_pix,
      pageseg_apply_music_mask ? &musicmask_pix : nullptr);
  int result = 0;
  if (finder != nullptr) {
    TO_BLOCK_IT to_block_it(&temp_blocks);
    TO_BLOCK* to_block = to_block_it.data();
    if (musicmask_pix != nullptr) {
      // TODO(rays) pass the musicmask_pix into FindBlocks and mark music
      // blocks separately. For now combine with photomask_pix.
      pixOr(photomask_pix, photomask_pix, musicmask_pix);
    }
    if (equ_detect_) {
      finder->SetEquationDetect(equ_detect_);
    }
    result = finder->FindBlocks(pageseg_mode, scaled_color_, scaled_factor_,
                                to_block, photomask_pix, pix_thresholds_,
                                pix_grey_, &pixa_debug_, &found_blocks,
                                diacritic_blobs, to_blocks);
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
  return result;
}

// Helper adds all the scripts from sid_set converted to ids from osd_set to
// allowed_ids.
static void AddAllScriptsConverted(const UNICHARSET& sid_set,
                                   const UNICHARSET& osd_set,
                                   GenericVector<int>* allowed_ids) {
  for (int i = 0; i < sid_set.get_script_table_size(); ++i) {
    if (i != sid_set.null_sid()) {
      const char* script = sid_set.get_script_from_script_id(i);
      allowed_ids->push_back(osd_set.get_script_id_from_name(script));
    }
  }
}

/**
 * Sets up auto page segmentation, determines the orientation, and corrects it.
 * Somewhat arbitrary chunk of functionality, factored out of AutoPageSeg to
 * facilitate testing.
 * photo_mask_pix is a pointer to a nullptr pointer that will be filled on return
 * with the leptonica photo mask, which must be pixDestroyed by the caller.
 * to_blocks is an empty list that will be filled with (usually a single)
 * block that is used during layout analysis. This ugly API is required
 * because of the possibility of a unlv zone file.
 * TODO(rays) clean this up.
 * See AutoPageSeg for other arguments.
 * The returned ColumnFinder must be deleted after use.
 */
ColumnFinder* Tesseract::SetupPageSegAndDetectOrientation(
    PageSegMode pageseg_mode, BLOCK_LIST* blocks, Tesseract* osd_tess,
    OSResults* osr, TO_BLOCK_LIST* to_blocks, Pix** photo_mask_pix,
    Pix** music_mask_pix) {
  int vertical_x = 0;
  int vertical_y = 1;
  TabVector_LIST v_lines;
  TabVector_LIST h_lines;
  ICOORD bleft(0, 0);

  ASSERT_HOST(pix_binary_ != nullptr);
  if (tessedit_dump_pageseg_images) {
    pixa_debug_.AddPix(pix_binary_, "PageSegInput");
  }
  // Leptonica is used to find the rule/separator lines in the input.
  LineFinder::FindAndRemoveLines(source_resolution_,
                                 textord_tabfind_show_vlines, pix_binary_,
                                 &vertical_x, &vertical_y, music_mask_pix,
                                 &v_lines, &h_lines);
  if (tessedit_dump_pageseg_images) {
    pixa_debug_.AddPix(pix_binary_, "NoLines");
  }
  // Leptonica is used to find a mask of the photo regions in the input.
  *photo_mask_pix = ImageFind::FindImages(pix_binary_, &pixa_debug_);
  if (tessedit_dump_pageseg_images) {
    Pix* pix_no_image_ = nullptr;
    if (*photo_mask_pix != nullptr) {
      pix_no_image_ = pixSubtract(nullptr, pix_binary_, *photo_mask_pix);
    } else {
      pix_no_image_ = pixClone(pix_binary_);
    }
    pixa_debug_.AddPix(pix_no_image_, "NoImages");
    pixDestroy(&pix_no_image_);
  }
  if (!PSM_COL_FIND_ENABLED(pageseg_mode)) v_lines.clear();

  // The rest of the algorithm uses the usual connected components.
  textord_.find_components(pix_binary_, blocks, to_blocks);

  TO_BLOCK_IT to_block_it(to_blocks);
  // There must be exactly one input block.
  // TODO(rays) handle new textline finding with a UNLV zone file.
  ASSERT_HOST(to_blocks->singleton());
  TO_BLOCK* to_block = to_block_it.data();
  TBOX blkbox = to_block->block->pdblk.bounding_box();
  ColumnFinder* finder = nullptr;
  int estimated_resolution = source_resolution_;
  if (source_resolution_ == kMinCredibleResolution) {
    // Try to estimate resolution from typical body text size.
    int res = IntCastRounded(to_block->line_size * kResolutionEstimationFactor);
    if (res > estimated_resolution && res < kMaxCredibleResolution) {
      estimated_resolution = res;
      tprintf("Estimating resolution as %d\n", estimated_resolution);
    }
  }

  if (to_block->line_size >= 2) {
    finder = new ColumnFinder(static_cast<int>(to_block->line_size),
                              blkbox.botleft(), blkbox.topright(),
                              estimated_resolution, textord_use_cjk_fp_model,
                              textord_tabfind_aligned_gap_fraction, &v_lines,
                              &h_lines, vertical_x, vertical_y);

    finder->SetupAndFilterNoise(pageseg_mode, *photo_mask_pix, to_block);

#ifndef DISABLED_LEGACY_ENGINE

    if (equ_detect_) {
      equ_detect_->LabelSpecialText(to_block);
    }

    BLOBNBOX_CLIST osd_blobs;
    // osd_orientation is the number of 90 degree rotations to make the
    // characters upright. (See tesseract/osdetect.h for precise definition.)
    // We want the text lines horizontal, (vertical text indicates vertical
    // textlines) which may conflict (eg vertically written CJK).
    int osd_orientation = 0;
    bool vertical_text = textord_tabfind_force_vertical_text ||
                         pageseg_mode == PSM_SINGLE_BLOCK_VERT_TEXT;
    if (!vertical_text && textord_tabfind_vertical_text &&
        PSM_ORIENTATION_ENABLED(pageseg_mode)) {
      vertical_text =
          finder->IsVerticallyAlignedText(textord_tabfind_vertical_text_ratio,
                                          to_block, &osd_blobs);
    }
    if (PSM_OSD_ENABLED(pageseg_mode) && osd_tess != nullptr && osr != nullptr) {
      GenericVector<int> osd_scripts;
      if (osd_tess != this) {
        // We are running osd as part of layout analysis, so constrain the
        // scripts to those allowed by *this.
        AddAllScriptsConverted(unicharset, osd_tess->unicharset, &osd_scripts);
        for (int s = 0; s < sub_langs_.size(); ++s) {
          AddAllScriptsConverted(sub_langs_[s]->unicharset,
                                 osd_tess->unicharset, &osd_scripts);
        }
      }
      os_detect_blobs(&osd_scripts, &osd_blobs, osr, osd_tess);
      if (pageseg_mode == PSM_OSD_ONLY) {
        delete finder;
        return nullptr;
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
      int best_script_id = osr->best_result.script_id;
      const char* best_script_str =
          osd_tess->unicharset.get_script_from_script_id(best_script_id);
      bool cjk = best_script_id == osd_tess->unicharset.han_sid() ||
          best_script_id == osd_tess->unicharset.hiragana_sid() ||
          best_script_id == osd_tess->unicharset.katakana_sid() ||
          strcmp("Japanese", best_script_str) == 0 ||
          strcmp("Korean", best_script_str) == 0 ||
          strcmp("Hangul", best_script_str) == 0;
      if (cjk) {
        finder->set_cjk_script(true);
      }
      if (osd_margin < min_orientation_margin) {
        // The margin is weak.
        if (!cjk && !vertical_text && osd_orientation == 2) {
          // upside down latin text is improbable with such a weak margin.
          tprintf("OSD: Weak margin (%.2f), horiz textlines, not CJK: "
                  "Don't rotate.\n", osd_margin);
          osd_orientation = 0;
        } else {
          tprintf(
              "OSD: Weak margin (%.2f) for %d blob text block, "
              "but using orientation anyway: %d\n",
              osd_margin, osd_blobs.length(), osd_orientation);
        }
      }
    }
    osd_blobs.shallow_clear();
    finder->CorrectOrientation(to_block, vertical_text, osd_orientation);

#endif  // ndef DISABLED_LEGACY_ENGINE
  }

  return finder;
}

}  // namespace tesseract.
