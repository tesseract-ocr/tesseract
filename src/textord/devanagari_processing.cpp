/**********************************************************************
 * File:        devanagari_processing.cpp
 * Description: Methods to process images containing devanagari symbols,
 *              prior to classification.
 * Author:      Shobhit Saxena
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

#ifdef HAVE_CONFIG_H
#  include "config_auto.h"
#endif

#include "devanagari_processing.h"

#include "debugpixa.h"
#include "statistc.h"
#include "tordmain.h"

#include <allheaders.h>

namespace tesseract {

// Flags controlling the debugging information for shiro-rekha splitting
// strategies.
INT_VAR(devanagari_split_debuglevel, 0, "Debug level for split shiro-rekha process.");

BOOL_VAR(devanagari_split_debugimage, 0,
         "Whether to create a debug image for split shiro-rekha process.");

ShiroRekhaSplitter::ShiroRekhaSplitter() :
  orig_pix_(nullptr),
  splitted_image_(nullptr),
  pageseg_split_strategy_(NO_SPLIT),
  ocr_split_strategy_(NO_SPLIT),
  debug_image_(nullptr),
  segmentation_block_list_(nullptr),
  global_xheight_(kUnspecifiedXheight),
  perform_close_(false)
{
}

ShiroRekhaSplitter::~ShiroRekhaSplitter() {
  Clear();
}

void ShiroRekhaSplitter::Clear() {
  orig_pix_.destroy();
  splitted_image_.destroy();
  pageseg_split_strategy_ = NO_SPLIT;
  ocr_split_strategy_ = NO_SPLIT;
  debug_image_.destroy();
  segmentation_block_list_ = nullptr;
  global_xheight_ = kUnspecifiedXheight;
  perform_close_ = false;
}

// On setting the input image, a clone of it is owned by this class.
void ShiroRekhaSplitter::set_orig_pix(Image pix) {
  if (orig_pix_) {
    orig_pix_.destroy();
  }
  orig_pix_ = pix.clone();
}

// Top-level method to perform splitting based on current settings.
// Returns true if a split was actually performed.
// split_for_pageseg should be true if the splitting is being done prior to
// page segmentation. This mode uses the flag
// pageseg_devanagari_split_strategy to determine the splitting strategy.
bool ShiroRekhaSplitter::Split(bool split_for_pageseg, DebugPixa *pixa_debug) {
  SplitStrategy split_strategy = split_for_pageseg ? pageseg_split_strategy_ : ocr_split_strategy_;
  if (split_strategy == NO_SPLIT) {
    return false; // Nothing to do.
  }
  ASSERT_HOST(split_strategy == MINIMAL_SPLIT || split_strategy == MAXIMAL_SPLIT);
  ASSERT_HOST(orig_pix_);
  if (devanagari_split_debuglevel > 0) {
    tprintf("Splitting shiro-rekha ...\n");
    tprintf("Split strategy = %s\n", split_strategy == MINIMAL_SPLIT ? "Minimal" : "Maximal");
    tprintf("Initial pageseg available = %s\n", segmentation_block_list_ ? "yes" : "no");
  }
  // Create a copy of original image to store the splitting output.
  splitted_image_.destroy();
  splitted_image_ = orig_pix_.copy();

  // Initialize debug image if required.
  if (devanagari_split_debugimage) {
    debug_image_.destroy();
    debug_image_ = pixConvertTo32(orig_pix_);
  }

  // Determine all connected components in the input image. A close operation
  // may be required prior to this, depending on the current settings.
  Image pix_for_ccs = orig_pix_.clone();
  if (perform_close_ && global_xheight_ != kUnspecifiedXheight && !segmentation_block_list_) {
    if (devanagari_split_debuglevel > 0) {
      tprintf("Performing a global close operation..\n");
    }
    // A global measure is available for xheight, but no local information
    // exists.
    pix_for_ccs.destroy();
    pix_for_ccs = orig_pix_.copy();
    PerformClose(pix_for_ccs, global_xheight_);
  }
  Pixa *ccs;
  Boxa *tmp_boxa = pixConnComp(pix_for_ccs, &ccs, 8);
  boxaDestroy(&tmp_boxa);
  pix_for_ccs.destroy();

  // Iterate over all connected components. Get their bounding boxes and clip
  // out the image regions corresponding to these boxes from the original image.
  // Conditionally run splitting on each of them.
  Boxa *regions_to_clear = boxaCreate(0);
  int num_ccs = 0;
  if (ccs != nullptr) {
    num_ccs = pixaGetCount(ccs);
  }
  for (int i = 0; i < num_ccs; ++i) {
    Box *box = pixaGetBox(ccs, i, L_CLONE);
    Image word_pix = pixClipRectangle(orig_pix_, box, nullptr);
    ASSERT_HOST(word_pix);
    int xheight = GetXheightForCC(box);
    if (xheight == kUnspecifiedXheight && segmentation_block_list_ && devanagari_split_debugimage) {
      pixRenderBoxArb(debug_image_, box, 1, 255, 0, 0);
    }
    // If some xheight measure is available, attempt to pre-eliminate small
    // blobs from the shiro-rekha process. This is primarily to save the CCs
    // corresponding to punctuation marks/small dots etc which are part of
    // larger graphemes.
    l_int32 x, y, w, h;
    boxGetGeometry(box, &x, &y, &w, &h);
    if (xheight == kUnspecifiedXheight || (w > xheight / 3 && h > xheight / 2)) {
      SplitWordShiroRekha(split_strategy, word_pix, xheight, x, y, regions_to_clear);
    } else if (devanagari_split_debuglevel > 0) {
      tprintf("CC dropped from splitting: %d,%d (%d, %d)\n", x, y, w, h);
    }
    word_pix.destroy();
    boxDestroy(&box);
  }
  // Actually clear the boxes now.
  for (int i = 0; i < boxaGetCount(regions_to_clear); ++i) {
    Box *box = boxaGetBox(regions_to_clear, i, L_CLONE);
    pixClearInRect(splitted_image_, box);
    boxDestroy(&box);
  }
  boxaDestroy(&regions_to_clear);
  pixaDestroy(&ccs);
  if (devanagari_split_debugimage && pixa_debug != nullptr) {
    pixa_debug->AddPix(debug_image_, split_for_pageseg ? "pageseg_split" : "ocr_split");
  }
  return true;
}

// Method to perform a close operation on the input image. The xheight
// estimate decides the size of sel used.
void ShiroRekhaSplitter::PerformClose(Image pix, int xheight_estimate) {
  pixCloseBrick(pix, pix, xheight_estimate / 8, xheight_estimate / 3);
}

// This method resolves the cc bbox to a particular row and returns the row's
// xheight.
int ShiroRekhaSplitter::GetXheightForCC(Box *cc_bbox) {
  if (!segmentation_block_list_) {
    return global_xheight_;
  }
  // Compute the box coordinates in Tesseract's coordinate system.
  l_int32 x, y, w, h;
  boxGetGeometry(cc_bbox, &x, &y, &w, &h);
  TBOX bbox(x, pixGetHeight(orig_pix_) - y - h - 1,
            x + w, pixGetHeight(orig_pix_) - y - 1);
  // Iterate over all blocks.
  BLOCK_IT block_it(segmentation_block_list_);
  for (block_it.mark_cycle_pt(); !block_it.cycled_list(); block_it.forward()) {
    BLOCK *block = block_it.data();
    // Iterate over all rows in the block.
    ROW_IT row_it(block->row_list());
    for (row_it.mark_cycle_pt(); !row_it.cycled_list(); row_it.forward()) {
      ROW *row = row_it.data();
      if (!row->bounding_box().major_overlap(bbox)) {
        continue;
      }
      // Row could be skewed, warped, etc. Use the position of the box to
      // determine the baseline position of the row for that x-coordinate.
      // Create a square TBOX whose baseline's mid-point lies at this point
      // and side is row's xheight. Take the overlap of this box with the input
      // box and check if it is a 'major overlap'. If so, this box lies in this
      // row. In that case, return the xheight for this row.
      float box_middle = 0.5 * (bbox.left() + bbox.right());
      int baseline = static_cast<int>(row->base_line(box_middle) + 0.5);
      TBOX test_box(box_middle - row->x_height() / 2, baseline, box_middle + row->x_height() / 2,
                    static_cast<int>(baseline + row->x_height()));
      // Compute overlap. If it is a major overlap, this is the right row.
      if (bbox.major_overlap(test_box)) {
        return row->x_height();
      }
    }
  }
  // No row found for this bbox.
  return kUnspecifiedXheight;
}

// Returns a list of regions (boxes) which should be cleared in the original
// image so as to perform shiro-rekha splitting. Pix is assumed to carry one
// (or less) word only. Xheight measure could be the global estimate, the row
// estimate, or unspecified. If unspecified, over splitting may occur, since a
// conservative estimate of stroke width along with an associated multiplier
// is used in its place. It is advisable to have a specified xheight when
// splitting for classification/training.
// A vertical projection histogram of all the on-pixels in the input pix is
// computed. The maxima of this histogram is regarded as an approximate location
// of the shiro-rekha. By descending on the maxima's peak on both sides,
// stroke width of shiro-rekha is estimated.
// A horizontal projection histogram is computed for a sub-image of the input
// image, which extends from just below the shiro-rekha down to a certain
// leeway. The leeway depends on the input xheight, if provided, else a
// conservative multiplier on approximate stroke width is used (which may lead
// to over-splitting).
void ShiroRekhaSplitter::SplitWordShiroRekha(SplitStrategy split_strategy, Image pix, int xheight,
                                             int word_left, int word_top, Boxa *regions_to_clear) {
  if (split_strategy == NO_SPLIT) {
    return;
  }
  int width = pixGetWidth(pix);
  int height = pixGetHeight(pix);
  // Statistically determine the yextents of the shiro-rekha.
  int shirorekha_top, shirorekha_bottom, shirorekha_ylevel;
  GetShiroRekhaYExtents(pix, &shirorekha_top, &shirorekha_bottom, &shirorekha_ylevel);
  // Since the shiro rekha is also a stroke, its width is equal to the stroke
  // width.
  int stroke_width = shirorekha_bottom - shirorekha_top + 1;

  // Some safeguards to protect CCs we do not want to be split.
  // These are particularly useful when the word wasn't eliminated earlier
  // because xheight information was unavailable.
  if (shirorekha_ylevel > height / 2) {
    // Shirorekha shouldn't be in the bottom half of the word.
    if (devanagari_split_debuglevel > 0) {
      tprintf("Skipping splitting CC at (%d, %d): shirorekha in lower half..\n", word_left,
              word_top);
    }
    return;
  }
  if (stroke_width > height / 3) {
    // Even the boldest of fonts shouldn't do this.
    if (devanagari_split_debuglevel > 0) {
      tprintf("Skipping splitting CC at (%d, %d): stroke width too huge..\n", word_left, word_top);
    }
    return;
  }

  // Clear the ascender and descender regions of the word.
  // Obtain a vertical projection histogram for the resulting image.
  Box *box_to_clear = boxCreate(0, shirorekha_top - stroke_width / 3, width, 5 * stroke_width / 3);
  Image word_in_xheight = pix.copy();
  pixClearInRect(word_in_xheight, box_to_clear);
  // Also clear any pixels which are below shirorekha_bottom + some leeway.
  // The leeway is set to xheight if the information is available, else it is a
  // multiplier applied to the stroke width.
  int leeway_to_keep = stroke_width * 3;
  if (xheight != kUnspecifiedXheight) {
    // This is because the xheight-region typically includes the shiro-rekha
    // inside it, i.e., the top of the xheight range corresponds to the top of
    // shiro-rekha.
    leeway_to_keep = xheight - stroke_width;
  }
  auto y = shirorekha_bottom + leeway_to_keep;
  boxSetGeometry(box_to_clear, -1, y, -1, height - y);
  pixClearInRect(word_in_xheight, box_to_clear);
  boxDestroy(&box_to_clear);

  PixelHistogram vert_hist;
  vert_hist.ConstructVerticalCountHist(word_in_xheight);
  word_in_xheight.destroy();

  // If the number of black pixel in any column of the image is less than a
  // fraction of the stroke width, treat it as noise / a stray mark. Perform
  // these changes inside the vert_hist data itself, as that is used later on as
  // a bit vector for the final split decision at every column.
  for (int i = 0; i < width; ++i) {
    if (vert_hist.hist()[i] <= stroke_width / 4) {
      vert_hist.hist()[i] = 0;
    } else {
      vert_hist.hist()[i] = 1;
    }
  }
  // In order to split the line at any point, we make sure that the width of the
  // gap is at least half the stroke width.
  int i = 0;
  int cur_component_width = 0;
  while (i < width) {
    if (!vert_hist.hist()[i]) {
      int j = 0;
      while (i + j < width && !vert_hist.hist()[i + j]) {
        ++j;
      }
      if (j >= stroke_width / 2 && cur_component_width >= stroke_width / 2) {
        // Perform a shiro-rekha split. The intervening region lies from i to
        // i+j-1.
        // A minimal single-pixel split makes the estimation of intra- and
        // inter-word spacing easier during page layout analysis,
        // whereas a maximal split may be needed for OCR, depending on
        // how the engine was trained.
        bool minimal_split = (split_strategy == MINIMAL_SPLIT);
        int split_width = minimal_split ? 1 : j;
        int split_left = minimal_split ? i + (j / 2) - (split_width / 2) : i;
        if (!minimal_split || (i != 0 && i + j != width)) {
          Box *box_to_clear =
              boxCreate(word_left + split_left, word_top + shirorekha_top - stroke_width / 3,
                        split_width, 5 * stroke_width / 3);
          if (box_to_clear) {
            boxaAddBox(regions_to_clear, box_to_clear, L_CLONE);
            // Mark this in the debug image if needed.
            if (devanagari_split_debugimage) {
              pixRenderBoxArb(debug_image_, box_to_clear, 1, 128, 255, 128);
            }
            boxDestroy(&box_to_clear);
            cur_component_width = 0;
          }
        }
      }
      i += j;
    } else {
      ++i;
      ++cur_component_width;
    }
  }
}

// Refreshes the words in the segmentation block list by using blobs in the
// input block list.
// The segmentation block list must be set.
void ShiroRekhaSplitter::RefreshSegmentationWithNewBlobs(C_BLOB_LIST *new_blobs) {
  // The segmentation block list must have been specified.
  ASSERT_HOST(segmentation_block_list_);
  if (devanagari_split_debuglevel > 0) {
    tprintf("Before refreshing blobs:\n");
    PrintSegmentationStats(segmentation_block_list_);
    tprintf("New Blobs found: %d\n", new_blobs->length());
  }

  C_BLOB_LIST not_found_blobs;
  RefreshWordBlobsFromNewBlobs(
      segmentation_block_list_, new_blobs,
      ((devanagari_split_debugimage && debug_image_) ? &not_found_blobs : nullptr));

  if (devanagari_split_debuglevel > 0) {
    tprintf("After refreshing blobs:\n");
    PrintSegmentationStats(segmentation_block_list_);
  }
  if (devanagari_split_debugimage && debug_image_) {
    // Plot out the original blobs for which no match was found in the new
    // all_blobs list.
    C_BLOB_IT not_found_it(&not_found_blobs);
    for (not_found_it.mark_cycle_pt(); !not_found_it.cycled_list(); not_found_it.forward()) {
      C_BLOB *not_found = not_found_it.data();
      TBOX not_found_box = not_found->bounding_box();
      Box *box_to_plot = GetBoxForTBOX(not_found_box);
      pixRenderBoxArb(debug_image_, box_to_plot, 1, 255, 0, 255);
      boxDestroy(&box_to_plot);
    }

    // Plot out the blobs unused from all blobs.
    C_BLOB_IT all_blobs_it(new_blobs);
    for (all_blobs_it.mark_cycle_pt(); !all_blobs_it.cycled_list(); all_blobs_it.forward()) {
      C_BLOB *a_blob = all_blobs_it.data();
      Box *box_to_plot = GetBoxForTBOX(a_blob->bounding_box());
      pixRenderBoxArb(debug_image_, box_to_plot, 3, 0, 127, 0);
      boxDestroy(&box_to_plot);
    }
  }
}

// Returns a new box object for the corresponding TBOX, based on the original
// image's coordinate system.
Box *ShiroRekhaSplitter::GetBoxForTBOX(const TBOX &tbox) const {
  return boxCreate(tbox.left(), pixGetHeight(orig_pix_) - tbox.top() - 1, tbox.width(),
                   tbox.height());
}

// This method returns the computed mode-height of blobs in the pix.
// It also prunes very small blobs from calculation.
int ShiroRekhaSplitter::GetModeHeight(Image pix) {
  Boxa *boxa = pixConnComp(pix, nullptr, 8);
  STATS heights(0, pixGetHeight(pix) - 1);
  heights.clear();
  for (int i = 0; i < boxaGetCount(boxa); ++i) {
    Box *box = boxaGetBox(boxa, i, L_CLONE);
    l_int32 x, y, w, h;
    boxGetGeometry(box, &x, &y, &w, &h);
    if (h >= 3 || w >= 3) {
      heights.add(h, 1);
    }
    boxDestroy(&box);
  }
  boxaDestroy(&boxa);
  return heights.mode();
}

// This method returns y-extents of the shiro-rekha computed from the input
// word image.
void ShiroRekhaSplitter::GetShiroRekhaYExtents(Image word_pix, int *shirorekha_top,
                                               int *shirorekha_bottom, int *shirorekha_ylevel) {
  // Compute a histogram from projecting the word on a vertical line.
  PixelHistogram hist_horiz;
  hist_horiz.ConstructHorizontalCountHist(word_pix);
  // Get the ylevel where the top-line exists. This is basically the global
  // maxima in the horizontal histogram.
  int topline_onpixel_count = 0;
  int topline_ylevel = hist_horiz.GetHistogramMaximum(&topline_onpixel_count);

  // Get the upper and lower extents of the shiro rekha.
  int thresh = (topline_onpixel_count * 70) / 100;
  int ulimit = topline_ylevel;
  int llimit = topline_ylevel;
  while (ulimit > 0 && hist_horiz.hist()[ulimit] >= thresh) {
    --ulimit;
  }
  while (llimit < pixGetHeight(word_pix) && hist_horiz.hist()[llimit] >= thresh) {
    ++llimit;
  }

  if (shirorekha_top) {
    *shirorekha_top = ulimit;
  }
  if (shirorekha_bottom) {
    *shirorekha_bottom = llimit;
  }
  if (shirorekha_ylevel) {
    *shirorekha_ylevel = topline_ylevel;
  }
}

// This method returns the global-maxima for the histogram. The frequency of
// the global maxima is returned in count, if specified.
int PixelHistogram::GetHistogramMaximum(int *count) const {
  int best_value = 0;
  for (int i = 0; i < length_; ++i) {
    if (hist_[i] > hist_[best_value]) {
      best_value = i;
    }
  }
  if (count) {
    *count = hist_[best_value];
  }
  return best_value;
}

// Methods to construct histograms from images.
void PixelHistogram::ConstructVerticalCountHist(Image pix) {
  Clear();
  int width = pixGetWidth(pix);
  int height = pixGetHeight(pix);
  hist_ = new int[width];
  length_ = width;
  int wpl = pixGetWpl(pix);
  l_uint32 *data = pixGetData(pix);
  for (int i = 0; i < width; ++i) {
    hist_[i] = 0;
  }
  for (int i = 0; i < height; ++i) {
    l_uint32 *line = data + i * wpl;
    for (int j = 0; j < width; ++j) {
      if (GET_DATA_BIT(line, j)) {
        ++(hist_[j]);
      }
    }
  }
}

void PixelHistogram::ConstructHorizontalCountHist(Image pix) {
  Clear();
  Numa *counts = pixCountPixelsByRow(pix, nullptr);
  length_ = numaGetCount(counts);
  hist_ = new int[length_];
  for (int i = 0; i < length_; ++i) {
    l_int32 val = 0;
    numaGetIValue(counts, i, &val);
    hist_[i] = val;
  }
  numaDestroy(&counts);
}

} // namespace tesseract.
