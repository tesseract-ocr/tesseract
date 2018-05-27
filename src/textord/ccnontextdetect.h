///////////////////////////////////////////////////////////////////////
// File:        ccnontextdetect.h
// Description: Connected-Component-based non-text detection.
// Copyright 2011 Google Inc. All Rights Reserved.
// Author: rays@google.com (Ray Smith)
// Created:     Sat Jun 11 09:52:01 PST 2011
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

#ifndef TESSERACT_TEXTORD_CCPHOTODETECT_H_
#define TESSERACT_TEXTORD_CCPHOTODETECT_H_

#include "blobgrid.h"
#include "scrollview.h"

namespace tesseract {

// The CCNonTextDetect class contains grid-based operations on blobs to create
// a full-resolution image mask analogous yet complementary to
// pixGenHalftoneMask as it is better at line-drawings, graphs and charts.
class CCNonTextDetect : public BlobGrid {
 public:
  CCNonTextDetect(int gridsize, const ICOORD& bleft, const ICOORD& tright);
  ~CCNonTextDetect() override;

  // Creates and returns a Pix with the same resolution as the original
  // in which 1 (black) pixels represent likely non text (photo, line drawing)
  // areas of the page, deleting from the blob_block the blobs that were
  // determined to be non-text.
  // The photo_map (binary image mask) is used to bias the decision towards
  // non-text, rather than supplying a definite decision.
  // The blob_block is the usual result of connected component analysis,
  // holding the detected blobs.
  // The returned Pix should be PixDestroyed after use.
  Pix* ComputeNonTextMask(bool debug, Pix* photo_map, TO_BLOCK* blob_block);

 private:
  // Computes and returns the noise_density IntGrid, at the same gridsize as
  // this by summing the number of small elements in a 3x3 neighbourhood of
  // each grid cell. good_grid is filled with blobs that are considered most
  // likely good text, and this is filled with small and medium blobs that are
  // more likely non-text.
  // The photo_map is used to bias the decision towards non-text, rather than
  // supplying definite decision.
  IntGrid* ComputeNoiseDensity(bool debug, Pix* photo_map, BlobGrid* good_grid);

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
  // If the win is not nullptr, deleted blobs are drawn on it in red, and kept
  void MarkAndDeleteNonTextBlobs(BLOBNBOX_LIST* blobs,
                                 int max_blob_overlaps,
                                 ScrollView* win, ScrollView::Color ok_color,
                                 Pix* nontext_mask);
  // Returns true if the given blob overlaps more than max_overlaps blobs
  // in the current grid.
  bool BlobOverlapsTooMuch(BLOBNBOX* blob, int max_overlaps);

  // Max entry in noise_density_ before the cell is declared noisy.
  int max_noise_count_;
  // Completed noise density map, which we keep around to use for secondary
  // noise detection.
  IntGrid* noise_density_;
};

}  // namespace tesseract.

#endif  // TESSERACT_TEXTORD_CCPHOTODETECT_H_
