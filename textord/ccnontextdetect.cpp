///////////////////////////////////////////////////////////////////////
// File:        ccnontextdetect.cpp
// Description: Connected-Component-based photo (non-text) detection.
// Copyright 2011 Google Inc. All Rights Reserved.
// Author: rays@google.com (Ray Smith)
// Created:     Sat Jun 11 10:12:01 PST 2011
//
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

#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif

#include "ccnontextdetect.h"
#include "imagefind.h"
#include "strokewidth.h"

namespace tesseract {

// Max number of neighbour small objects per squared gridsize before a grid
// cell becomes image.
const double kMaxSmallNeighboursPerPix = 1.0 / 32;
// Max number of small blobs a large blob may overlap before it is rejected
// and determined to be image.
const int kMaxLargeOverlapsWithSmall = 3;
// Max number of small blobs a medium blob may overlap before it is rejected
// and determined to be image. Larger than for large blobs as medium blobs
// may be complex Chinese characters. Very large Chinese characters are going
// to overlap more medium blobs than small.
const int kMaxMediumOverlapsWithSmall = 12;
// Max number of normal blobs a large blob may overlap before it is rejected
// and determined to be image. This is set higher to allow for drop caps, which
// may overlap a lot of good text blobs.
const int kMaxLargeOverlapsWithMedium = 12;
// Multiplier of original noise_count used to test for the case of spreading
// noise beyond where it should really be.
const int kOriginalNoiseMultiple = 8;
// Pixel padding for noise blobs when rendering on the image
// mask to encourage them to join together. Make it too big and images
// will fatten out too much and have to be clipped to text.
const int kNoisePadding = 4;
// Fraction of max_noise_count_ to be added to the noise count if there is
// photo mask in the background.
const double kPhotoOffsetFraction = 0.375;
// Min ratio of perimeter^2/16area for a "good" blob in estimating noise
// density. Good blobs are supposed to be highly likely real text.
// We consider a square to have unit ratio, where A=(p/4)^2, hence the factor
// of 16. Digital circles are weird and have a minimum ratio of pi/64, not
// the 1/(4pi) that you would expect.
const double kMinGoodTextPARatio = 1.5;

CCNonTextDetect::CCNonTextDetect(int gridsize,
                             const ICOORD& bleft, const ICOORD& tright)
  : BlobGrid(gridsize, bleft, tright),
    max_noise_count_(static_cast<int>(kMaxSmallNeighboursPerPix *
                                      gridsize * gridsize)),
    noise_density_(NULL) {
  // TODO(rays) break max_noise_count_ out into an area-proportional
  // value, as now plus an additive constant for the number of text blobs
  // in the 3x3 neighbourhood - maybe 9.
}

CCNonTextDetect::~CCNonTextDetect() {
  delete noise_density_;
}

// Creates and returns a Pix with the same resolution as the original
// in which 1 (black) pixels represent likely non text (photo, line drawing)
// areas of the page, deleting from the blob_block the blobs that were
// determined to be non-text.
// The photo_map is used to bias the decision towards non-text, rather than
// supplying definite decision.
// The blob_block is the usual result of connected component analysis,
// holding the detected blobs.
// The returned Pix should be PixDestroyed after use.
Pix* CCNonTextDetect::ComputeNonTextMask(bool debug, Pix* photo_map,
                                         TO_BLOCK* blob_block) {
  // Insert the smallest blobs into the grid.
  InsertBlobList(&blob_block->small_blobs);
  InsertBlobList(&blob_block->noise_blobs);
  // Add the medium blobs that don't have a good strokewidth neighbour.
  // Those that do go into good_grid as an antidote to spreading beyond the
  // real reaches of a noise region.
  BlobGrid good_grid(gridsize(), bleft(), tright());
  BLOBNBOX_IT blob_it(&blob_block->blobs);
  for (blob_it.mark_cycle_pt(); !blob_it.cycled_list(); blob_it.forward()) {
    BLOBNBOX* blob = blob_it.data();
    double perimeter_area_ratio = blob->cblob()->perimeter() / 4.0;
    perimeter_area_ratio *= perimeter_area_ratio / blob->enclosed_area();
    if (blob->GoodTextBlob() == 0 || perimeter_area_ratio < kMinGoodTextPARatio)
      InsertBBox(true, true, blob);
    else
      good_grid.InsertBBox(true, true, blob);
  }
  noise_density_ = ComputeNoiseDensity(debug, photo_map, &good_grid);
  good_grid.Clear();  // Not needed any more.
  Pix* pix = noise_density_->ThresholdToPix(max_noise_count_);
  if (debug) {
    pixWrite("junknoisemask.png", pix, IFF_PNG);
  }
  ScrollView* win = NULL;
  #ifndef GRAPHICS_DISABLED
  if (debug) {
    win = MakeWindow(0, 400, "Photo Mask Blobs");
  }
  #endif  // GRAPHICS_DISABLED
  // Large and medium blobs are not text if they overlap with "a lot" of small
  // blobs.
  MarkAndDeleteNonTextBlobs(&blob_block->large_blobs,
                            kMaxLargeOverlapsWithSmall,
                            win, ScrollView::DARK_GREEN, pix);
  MarkAndDeleteNonTextBlobs(&blob_block->blobs, kMaxMediumOverlapsWithSmall,
                          win, ScrollView::WHITE, pix);
  // Clear the grid of small blobs and insert the medium blobs.
  Clear();
  InsertBlobList(&blob_block->blobs);
  MarkAndDeleteNonTextBlobs(&blob_block->large_blobs,
                            kMaxLargeOverlapsWithMedium,
                            win, ScrollView::DARK_GREEN, pix);
  // Clear again before we start deleting the blobs in the grid.
  Clear();
  MarkAndDeleteNonTextBlobs(&blob_block->noise_blobs, -1,
                            win, ScrollView::CORAL, pix);
  MarkAndDeleteNonTextBlobs(&blob_block->small_blobs, -1,
                            win, ScrollView::GOLDENROD, pix);
  MarkAndDeleteNonTextBlobs(&blob_block->blobs, -1,
                            win, ScrollView::WHITE, pix);
  if (debug) {
    #ifndef GRAPHICS_DISABLED
    win->Update();
    #endif  // GRAPHICS_DISABLED
    pixWrite("junkccphotomask.png", pix, IFF_PNG);
    #ifndef GRAPHICS_DISABLED
    delete win->AwaitEvent(SVET_DESTROY);
    delete win;
    #endif  // GRAPHICS_DISABLED
  }
  return pix;
}

// Computes and returns the noise_density IntGrid, at the same gridsize as
// this by summing the number of small elements in a 3x3 neighbourhood of
// each grid cell. good_grid is filled with blobs that are considered most
// likely good text, and this is filled with small and medium blobs that are
// more likely non-text.
// The photo_map is used to bias the decision towards non-text, rather than
// supplying definite decision.
IntGrid* CCNonTextDetect::ComputeNoiseDensity(bool debug, Pix* photo_map,
                                              BlobGrid* good_grid) {
  IntGrid* noise_counts = CountCellElements();
  IntGrid* noise_density = noise_counts->NeighbourhoodSum();
  IntGrid* good_counts = good_grid->CountCellElements();
  // Now increase noise density in photo areas, to bias the decision and
  // minimize hallucinated text on image, but trim the noise_density where
  // there are good blobs and the original count is low in non-photo areas,
  // indicating that most of the result came from neighbouring cells.
  int height = pixGetHeight(photo_map);
  int photo_offset = IntCastRounded(max_noise_count_ * kPhotoOffsetFraction);
  for (int y = 0; y < gridheight(); ++y) {
    for (int x = 0; x < gridwidth(); ++x) {
      int noise = noise_density->GridCellValue(x, y);
      if (max_noise_count_ < noise + photo_offset &&
          noise <= max_noise_count_) {
        // Test for photo.
        int left = x * gridsize();
        int right = left + gridsize();
        int bottom = height - y * gridsize();
        int top = bottom - gridsize();
        if (ImageFind::BoundsWithinRect(photo_map, &left, &top, &right,
                                        &bottom)) {
          noise_density->SetGridCell(x, y, noise + photo_offset);
        }
      }
      if (debug && noise > max_noise_count_ &&
          good_counts->GridCellValue(x, y) > 0) {
        tprintf("At %d, %d, noise = %d, good=%d, orig=%d, thr=%d\n",
                x * gridsize(), y * gridsize(),
                noise_density->GridCellValue(x, y),
                good_counts->GridCellValue(x, y),
                noise_counts->GridCellValue(x, y), max_noise_count_);
      }
      if (noise > max_noise_count_ &&
          good_counts->GridCellValue(x, y) > 0 &&
          noise_counts->GridCellValue(x, y) * kOriginalNoiseMultiple <=
              max_noise_count_) {
        noise_density->SetGridCell(x, y, 0);
      }
    }
  }
  delete noise_counts;
  delete good_counts;
  return noise_density;
}

// Helper to expand a box in one of the 4 directions by the given pad,
// provided it does not expand into any cell with a zero noise density.
// If that is not possible, try expanding all round by a small constant.
static TBOX AttemptBoxExpansion(const TBOX& box, const IntGrid& noise_density,
                                int pad) {
  TBOX expanded_box(box);
  expanded_box.set_right(box.right() + pad);
  if (!noise_density.AnyZeroInRect(expanded_box))
    return expanded_box;
  expanded_box = box;
  expanded_box.set_left(box.left() - pad);
  if (!noise_density.AnyZeroInRect(expanded_box))
    return expanded_box;
  expanded_box = box;
  expanded_box.set_top(box.top() + pad);
  if (!noise_density.AnyZeroInRect(expanded_box))
    return expanded_box;
  expanded_box = box;
  expanded_box.set_bottom(box.bottom() + pad);
  if (!noise_density.AnyZeroInRect(expanded_box))
    return expanded_box;
  expanded_box = box;
  expanded_box.pad(kNoisePadding, kNoisePadding);
  if (!noise_density.AnyZeroInRect(expanded_box))
    return expanded_box;
  return box;
}

// Tests each blob in the list to see if it is certain non-text using 2
// conditions:
// 1. blob overlaps a cell with high value in noise_density_ (previously set
// by ComputeNoiseDensity).
// OR 2. The blob overlaps more than max_blob_overlaps in *this grid. This
// condition is disabled with max_blob_overlaps == -1.
// If it does, the blob is declared non-text, and is used to mark up the
// nontext_mask. Such blobs are fully deleted, and non-noise blobs have their
// neighbours reset, as they may now point to deleted data.
// WARNING: The blobs list blobs may be in the *this grid, but they are
// not removed. If any deleted blobs might be in *this, then this must be
// Clear()ed immediately after MarkAndDeleteNonTextBlobs is called.
// If the win is not NULL, deleted blobs are drawn on it in red, and kept
// blobs are drawn on it in ok_color.
void CCNonTextDetect::MarkAndDeleteNonTextBlobs(BLOBNBOX_LIST* blobs,
                                                int max_blob_overlaps,
                                                ScrollView* win,
                                                ScrollView::Color ok_color,
                                                Pix* nontext_mask) {
  int imageheight = tright().y() - bleft().x();
  BLOBNBOX_IT blob_it(blobs);
  BLOBNBOX_LIST dead_blobs;
  BLOBNBOX_IT dead_it(&dead_blobs);
  for (blob_it.mark_cycle_pt(); !blob_it.cycled_list(); blob_it.forward()) {
    BLOBNBOX* blob = blob_it.data();
    TBOX box = blob->bounding_box();
    if (!noise_density_->RectMostlyOverThreshold(box, max_noise_count_) &&
        (max_blob_overlaps < 0 ||
            !BlobOverlapsTooMuch(blob, max_blob_overlaps))) {
      blob->ClearNeighbours();
      #ifndef GRAPHICS_DISABLED
      if (win != NULL)
        blob->plot(win, ok_color, ok_color);
      #endif  // GRAPHICS_DISABLED
    } else {
      if (noise_density_->AnyZeroInRect(box)) {
        // There is a danger that the bounding box may overlap real text, so
        // we need to render the outline.
        Pix* blob_pix = blob->cblob()->render_outline();
        pixRasterop(nontext_mask, box.left(), imageheight - box.top(),
                    box.width(), box.height(), PIX_SRC | PIX_DST,
                    blob_pix, 0, 0);
        pixDestroy(&blob_pix);
      } else {
        if (box.area() < gridsize() * gridsize()) {
          // It is a really bad idea to make lots of small components in the
          // photo mask, so try to join it to a bigger area by expanding the
          // box in a way that does not touch any zero noise density cell.
          box = AttemptBoxExpansion(box, *noise_density_, gridsize());
        }
        // All overlapped cells are non-zero, so just mark the rectangle.
        pixRasterop(nontext_mask, box.left(), imageheight - box.top(),
                    box.width(), box.height(), PIX_SET, NULL, 0, 0);
      }
      #ifndef GRAPHICS_DISABLED
      if (win != NULL)
        blob->plot(win, ScrollView::RED, ScrollView::RED);
      #endif  // GRAPHICS_DISABLED
      // It is safe to delete the cblob now, as it isn't used by the grid
      // or BlobOverlapsTooMuch, and the BLOBNBOXes will go away with the
      // dead_blobs list.
      // TODO(rays) delete the delete when the BLOBNBOX destructor deletes
      // the cblob.
      delete blob->cblob();
      dead_it.add_to_end(blob_it.extract());
    }
  }
}

// Returns true if the given blob overlaps more than max_overlaps blobs
// in the current grid.
bool CCNonTextDetect::BlobOverlapsTooMuch(BLOBNBOX* blob, int max_overlaps) {
  // Search the grid to see what intersects it.
  // Setup a Rectangle search for overlapping this blob.
  BlobGridSearch rsearch(this);
  const TBOX& box = blob->bounding_box();
  rsearch.StartRectSearch(box);
  rsearch.SetUniqueMode(true);
  BLOBNBOX* neighbour;
  int overlap_count = 0;
  while (overlap_count <= max_overlaps &&
         (neighbour = rsearch.NextRectSearch()) != NULL) {
    if (box.major_overlap(neighbour->bounding_box())) {
      ++overlap_count;
      if (overlap_count > max_overlaps)
        return true;
    }
  }
  return false;
}

}  // namespace tesseract.
