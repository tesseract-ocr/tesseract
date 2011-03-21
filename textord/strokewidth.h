///////////////////////////////////////////////////////////////////////
// File:        strokewidth.h
// Description: Subclass of BBGrid to find uniformity of strokewidth.
// Author:      Ray Smith
// Created:     Mon Mar 31 16:17:01 PST 2008
//
// (C) Copyright 2008, Google Inc.
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

#ifndef TESSERACT_TEXTORD_STROKEWIDTH_H__
#define TESSERACT_TEXTORD_STROKEWIDTH_H__

#include "bbgrid.h"         // Base class.
#include "blobbox.h"        // BlobNeighourDir.
#include "tabvector.h"      // For BLOBNBOX_CLIST.

class TO_BLOCK;
class ScrollView;

namespace tesseract {

class ColPartition_LIST;
class TabFind;

/**
 * The StrokeWidth class holds all the normal and large blobs.
 * It is used to find good large blobs and move them to the normal blobs
 * by virtue of having a reasonable strokewidth compatible neighbour.
 */
class StrokeWidth : public BBGrid<BLOBNBOX, BLOBNBOX_CLIST, BLOBNBOX_C_IT> {
 public:
  StrokeWidth(int gridsize, const ICOORD& bleft, const ICOORD& tright);
  virtual ~StrokeWidth();

  // To save computation, the process of generating partitions is broken
  // into the following 4 steps:
  // TestVerticalTextDirection
  // CorrectForRotation (used only if a rotation is to be applied)
  // FindLeaderPartitions
  // TODO(rays) Coming soon:
  // GradeBlobsIntoPartitions.
  // which will replace entirely the old call sequence of:
  // InsertBlobsOld
  // MoveGoodLargeBlobs.
  // These functions are all required, in sequence, except for
  // CorrectForRotation, which is not needed if no rotation is applied.

  // Types all the blobs as vertical or horizontal text or unknown and
  // returns true if the majority are vertical.
  // If the blobs are rotated, it is necessary to call CorrectForRotation
  // after rotating everything, otherwise the work done here will be enough.
  // If cjk_merge is true, it will attempt to merge broken cjk characters.
  // If osd_blobs is not null, a list of blobs from the dominant textline
  // direction are returned for use in orientation and script detection.
  bool TestVerticalTextDirection(bool cjk_merge,
                                 TO_BLOCK* block, TabFind* line_grid,
                                 BLOBNBOX_CLIST* osd_blobs);

  // Corrects the data structures for the given rotation.
  void CorrectForRotation(const FCOORD& rotation, TO_BLOCK* block);

  // Finds leader partitions and inserts them into the give grid.
  void FindLeaderPartitions(TO_BLOCK* block, TabFind* line_grid);

  // Handles a click event in a display window.
  virtual void HandleClick(int x, int y);

  // Puts the block blobs (normal and large) into the grid.
  void InsertBlobsOld(TO_BLOCK* block, TabFind* line_grid);

  // Moves the large blobs that have good stroke-width neighbours to the normal
  // blobs list.
  void MoveGoodLargeBlobs(int resolution, TO_BLOCK* block);

 private:
  // Reorganize the blob lists with a different definition of small, medium
  // and large, compared to the original definition.
  // Height is still the primary filter key, but medium width blobs of small
  // height become medium, and very wide blobs of small height stay small.
  void ReFilterBlobs(TO_BLOCK* block);

  // Computes the noise_density_ by summing the number of elements in a
  // neighbourhood of each grid cell.
  void ComputeNoiseDensity(TO_BLOCK* block, TabFind* line_grid);

  // Detects and marks leader dots/dashes.
  //    Leaders are horizontal chains of small or noise blobs that look
  //    monospace according to ColPartition::MarkAsLeaderIfMonospaced().
  // Detected leaders become the only occupants of small_blobs list.
  // Non-leader small blobs get moved to the blobs list.
  // Non-leader noise blobs remain singletons in the noise list.
  // All small and noise blobs in high density regions are marked BTFT_NONTEXT.
  void FindLeadersAndMarkNoise(bool final, TO_BLOCK* block, TabFind* line_grid,
                               ColPartition_LIST* leader_parts);

  // Puts the block blobs (normal and large) into the grid.
  void InsertBlobs(TO_BLOCK* block, TabFind* line_grid);

  // Fix broken CJK characters, using the fake joined blobs mechanism.
  // Blobs are really merged, ie the master takes all the outlines and the
  // others are deleted.
  void FixBrokenCJK(BLOBNBOX_LIST* blobs, TabFind* line_grid);

  // Collect blobs that overlap or are within max_dist of the input bbox.
  // Return them in the list of blobs and expand the bbox to be the union
  // of all the boxes. not_this is excluded from the search, as are blobs
  // that cause the merged box to exceed max_size in either dimension.
  void AccumulateOverlaps(const BLOBNBOX* not_this, bool debug,
                          int max_size, int max_dist,
                          TBOX* bbox, BLOBNBOX_CLIST* blobs);

  // Finds the textline direction to be horizontal or vertical according
  // to distance to neighbours and 1st and 2nd order neighbours.
  // Non-text tends to end up without a definite direction.
  void FindTextlineFlowDirection(bool final);

  // Sets the neighbours and good_stroke_neighbours members of the blob by
  // searching close on all 4 sides.
  // When finding leader dots/dashes, there is a slightly different rule for
  // what makes a good neighbour.
  void SetNeighbours(bool leaders, BLOBNBOX* blob);

  // Sets the good_stroke_neighbours member of the blob if it has a
  // GoodNeighbour on the given side.
  // Also sets the neighbour in the blob, whether or not a good one is found.
  // Return value is the number of neighbours in the line trap size range.
  // Leaders get extra special lenient treatment.
  int FindGoodNeighbour(BlobNeighbourDir dir, bool leaders, BLOBNBOX* blob);

  // Makes the blob to be only horizontal or vertical where evidence
  // is clear based on gaps of 2nd order neighbours.
  void SetNeighbourFlows(BLOBNBOX* blob);

  // Nullify the neighbours in the wrong directions where the direction
  // is clear-cut based on a distance margin. Good for isolating vertical
  // text from neighbouring horizontal text.
  void SimplifyObviousNeighbours(BLOBNBOX* blob);

  // Smoothes the vertical/horizontal type of the blob based on the
  // 2nd-order neighbours. If reset_all is true, then all blobs are
  // changed. Otherwise, only ambiguous blobs are processed.
  void SmoothNeighbourTypes(BLOBNBOX* blob, bool desperate);

  // Sets the leader_on_left or leader_on_right flags for blobs
  // that are next to one end of the given leader partition.
  // If left_of_part is true, then look at the left side of the partition for
  // blobs on which to set the leader_on_right flag.
  void MarkLeaderNeighbours(const ColPartition* part, bool left_of_part);

  // Displays the blobs colored according to the number of good neighbours
  // and the vertical/horizontal flow.
  ScrollView* DisplayGoodBlobs(const char* window_name, int x, int y);

 private:
  // Returns true if there is at least one side neighbour that has a similar
  // stroke width.
  bool GoodTextBlob(BLOBNBOX* blob);
  // Grid to indicate the dot noise density at each grid coord.
  IntGrid* noise_density_;
  // Windows for debug display.
  ScrollView* leaders_win_;
  ScrollView* initial_widths_win_;
  ScrollView* widths_win_;
};

}  // namespace tesseract.

#endif  // TESSERACT_TEXTORD_STROKEWIDTH_H__
