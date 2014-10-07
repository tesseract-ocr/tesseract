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

#include "blobbox.h"        // BlobNeighourDir.
#include "blobgrid.h"         // Base class.
#include "colpartitiongrid.h"
#include "textlineprojection.h"

class DENORM;
class ScrollView;
class TO_BLOCK;

namespace tesseract {

class ColPartition_LIST;
class TabFind;
class TextlineProjection;

// Misc enums to clarify bool arguments for direction-controlling args.
enum LeftOrRight {
  LR_LEFT,
  LR_RIGHT
};

/**
 * The StrokeWidth class holds all the normal and large blobs.
 * It is used to find good large blobs and move them to the normal blobs
 * by virtue of having a reasonable strokewidth compatible neighbour.
 */
class StrokeWidth : public BlobGrid {
 public:
  StrokeWidth(int gridsize, const ICOORD& bleft, const ICOORD& tright);
  virtual ~StrokeWidth();

  // Sets the neighbours member of the medium-sized blobs in the block.
  // Searches on 4 sides of each blob for similar-sized, similar-strokewidth
  // blobs and sets pointers to the good neighbours.
  void SetNeighboursOnMediumBlobs(TO_BLOCK* block);

  // Sets the neighbour/textline writing direction members of the medium
  // and large blobs with optional repair of broken CJK characters first.
  // Repair of broken CJK is needed here because broken CJK characters
  // can fool the textline direction detection algorithm.
  void FindTextlineDirectionAndFixBrokenCJK(bool cjk_merge,
                                            TO_BLOCK* input_block);

  // To save computation, the process of generating partitions is broken
  // into the following 4 steps:
  // TestVerticalTextDirection
  // CorrectForRotation (used only if a rotation is to be applied)
  // FindLeaderPartitions
  // GradeBlobsIntoPartitions.
  // These functions are all required, in sequence, except for
  // CorrectForRotation, which is not needed if no rotation is applied.

  // Types all the blobs as vertical or horizontal text or unknown and
  // returns true if the majority are vertical.
  // If the blobs are rotated, it is necessary to call CorrectForRotation
  // after rotating everything, otherwise the work done here will be enough.
  // If osd_blobs is not null, a list of blobs from the dominant textline
  // direction are returned for use in orientation and script detection.
  // find_vertical_text_ratio should be textord_tabfind_vertical_text_ratio.
  bool TestVerticalTextDirection(double find_vertical_text_ratio,
                                 TO_BLOCK* block,
                                 BLOBNBOX_CLIST* osd_blobs);

  // Corrects the data structures for the given rotation.
  void CorrectForRotation(const FCOORD& rerotation,
                          ColPartitionGrid* part_grid);

  // Finds leader partitions and inserts them into the give grid.
  void FindLeaderPartitions(TO_BLOCK* block,
                            ColPartitionGrid* part_grid);

  // Finds and marks noise those blobs that look like bits of vertical lines
  // that would otherwise screw up layout analysis.
  void RemoveLineResidue(ColPartition_LIST* big_part_list);

  // Types all the blobs as vertical text or horizontal text or unknown and
  // puts them into initial ColPartitions in the supplied part_grid.
  // rerotation determines how to get back to the image coordinates from the
  // blob coordinates (since they may have been rotated for vertical text).
  // block is the single block for the whole page or rectangle to be OCRed.
  // nontext_pix (full-size), is a binary mask used to prevent merges across
  // photo/text boundaries. It is not kept beyond this function.
  // denorm provides a mapping back to the image from the current blob
  // coordinate space.
  // projection provides a measure of textline density over the image and
  // provides functions to assist with diacritic detection. It should be a
  // pointer to a new TextlineProjection, and will be setup here.
  // part_grid is the output grid of textline partitions.
  // Large blobs that cause overlap are put in separate partitions and added
  // to the big_parts list.
  void GradeBlobsIntoPartitions(const FCOORD& rerotation,
                                TO_BLOCK* block,
                                Pix* nontext_pix,
                                const DENORM* denorm,
                                bool cjk_script,
                                TextlineProjection* projection,
                                ColPartitionGrid* part_grid,
                                ColPartition_LIST* big_parts);

  // Handles a click event in a display window.
  virtual void HandleClick(int x, int y);

 private:
  // Computes the noise_density_ by summing the number of elements in a
  // neighbourhood of each grid cell.
  void ComputeNoiseDensity(TO_BLOCK* block, TabFind* line_grid);

  // Detects and marks leader dots/dashes.
  //    Leaders are horizontal chains of small or noise blobs that look
  //    monospace according to ColPartition::MarkAsLeaderIfMonospaced().
  // Detected leaders become the only occupants of the block->small_blobs list.
  // Non-leader small blobs get moved to the blobs list.
  // Non-leader noise blobs remain singletons in the noise list.
  // All small and noise blobs in high density regions are marked BTFT_NONTEXT.
  // block is the single block for the whole page or rectangle to be OCRed.
  // leader_parts is the output.
  void FindLeadersAndMarkNoise(TO_BLOCK* block,
                               ColPartition_LIST* leader_parts);

  /** Inserts the block blobs (normal and large) into this grid.
   * Blobs remain owned by the block. */
  void InsertBlobs(TO_BLOCK* block);

  // Fix broken CJK characters, using the fake joined blobs mechanism.
  // Blobs are really merged, ie the master takes all the outlines and the
  // others are deleted.
  // Returns true if sufficient blobs are merged that it may be worth running
  // again, due to a better estimate of character size.
  bool FixBrokenCJK(TO_BLOCK* block);

  // Collect blobs that overlap or are within max_dist of the input bbox.
  // Return them in the list of blobs and expand the bbox to be the union
  // of all the boxes. not_this is excluded from the search, as are blobs
  // that cause the merged box to exceed max_size in either dimension.
  void AccumulateOverlaps(const BLOBNBOX* not_this, bool debug,
                          int max_size, int max_dist,
                          TBOX* bbox, BLOBNBOX_CLIST* blobs);

  // For each blob in this grid, Finds the textline direction to be horizontal
  // or vertical according to distance to neighbours and 1st and 2nd order
  // neighbours. Non-text tends to end up without a definite direction.
  // Result is setting of the neighbours and vert_possible/horz_possible
  // flags in the BLOBNBOXes currently in this grid.
  // This function is called more than once if page orientation is uncertain,
  // so display_if_debugging is true on the final call to display the results.
  void FindTextlineFlowDirection(bool display_if_debugging);

  // Sets the neighbours and good_stroke_neighbours members of the blob by
  // searching close on all 4 sides.
  // When finding leader dots/dashes, there is a slightly different rule for
  // what makes a good neighbour.
  // If activate_line_trap, then line-like objects are found and isolated.
  void SetNeighbours(bool leaders, bool activate_line_trap, BLOBNBOX* blob);

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

  // Checks the left or right side of the given leader partition and sets the
  // (opposite) leader_on_right or leader_on_left flags for blobs
  // that are next to the given side of the given leader partition.
  void MarkLeaderNeighbours(const ColPartition* part, LeftOrRight side);

  // Partition creation. Accumulates vertical and horizontal text chains,
  // puts the remaining blobs in as unknowns, and then merges/splits to
  // minimize overlap and smoothes the types with neighbours and the color
  // image if provided. rerotation is used to rotate the coordinate space
  // back to the nontext_map_ image.
  void FindInitialPartitions(const FCOORD& rerotation,
                             TO_BLOCK* block,
                             ColPartitionGrid* part_grid,
                             ColPartition_LIST* big_parts);
  // Finds vertical chains of text-like blobs and puts them in ColPartitions.
  void FindVerticalTextChains(ColPartitionGrid* part_grid);
  // Finds horizontal chains of text-like blobs and puts them in ColPartitions.
  void FindHorizontalTextChains(ColPartitionGrid* part_grid);
  // Finds diacritics and saves their base character in the blob.
  void TestDiacritics(ColPartitionGrid* part_grid, TO_BLOCK* block);
  // Searches this grid for an appropriately close and sized neighbour of the
  // given [small] blob. If such a blob is found, the diacritic base is saved
  // in the blob and true is returned.
  // The small_grid is a secondary grid that contains the small/noise objects
  // that are not in this grid, but may be useful for determining a connection
  // between blob and its potential base character. (See DiacriticXGapFilled.)
  bool DiacriticBlob(BlobGrid* small_grid, BLOBNBOX* blob);
  // Returns true if there is no gap between the base char and the diacritic
  // bigger than a fraction of the height of the base char:
  // Eg: line end.....'
  // The quote is a long way from the end of the line, yet it needs to be a
  // diacritic. To determine that the quote is not part of an image, or
  // a different text block, we check for other marks in the gap between
  // the base char and the diacritic.
  //                          '<--Diacritic
  // |---------|
  // |         |<-toobig-gap->
  // | Base    |<ok gap>
  // |---------|        x<-----Dot occupying gap
  // The grid is const really.
  bool DiacriticXGapFilled(BlobGrid* grid, const TBOX& diacritic_box,
                           const TBOX& base_box);
  // Merges diacritics with the ColPartition of the base character blob.
  void MergeDiacritics(TO_BLOCK* block, ColPartitionGrid* part_grid);
  // Any blobs on the large_blobs list of block that are still unowned by a
  // ColPartition, are probably drop-cap or vertically touching so the blobs
  // are removed to the big_parts list and treated separately.
  void RemoveLargeUnusedBlobs(TO_BLOCK* block,
                              ColPartitionGrid* part_grid,
                              ColPartition_LIST* big_parts);

    // All remaining unused blobs are put in individual ColPartitions.
  void PartitionRemainingBlobs(ColPartitionGrid* part_grid);

  // If combine, put all blobs in the cell_list into a single partition,
  // otherwise put each one into its own partition.
  void MakePartitionsFromCellList(bool combine,
                                  ColPartitionGrid* part_grid,
                                  BLOBNBOX_CLIST* cell_list);

  // Helper function to finish setting up a ColPartition and insert into
  // part_grid.
  void CompletePartition(ColPartition* part, ColPartitionGrid* part_grid);

  // Merge partitions where the merge appears harmless.
  void EasyMerges(ColPartitionGrid* part_grid);

  // Compute a search box based on the orientation of the partition.
  // Returns true if a suitable box can be calculated.
  // Callback for EasyMerges.
  bool OrientationSearchBox(ColPartition* part, TBOX* box);

  // Merge confirmation callback for EasyMerges.
  bool ConfirmEasyMerge(const ColPartition* p1, const ColPartition* p2);

  // Returns true if there is no significant noise in between the boxes.
  bool NoNoiseInBetween(const TBOX& box1, const TBOX& box2) const;

  // Displays the blobs colored according to the number of good neighbours
  // and the vertical/horizontal flow.
  ScrollView* DisplayGoodBlobs(const char* window_name, int x, int y);

  // Displays blobs colored according to whether or not they are diacritics.
  ScrollView* DisplayDiacritics(const char* window_name,
                                int x, int y, TO_BLOCK* block);

 private:
  // Image map of photo/noise areas on the page. Borrowed pointer (not owned.)
  Pix* nontext_map_;
  // Textline projection map. Borrowed pointer.
  TextlineProjection* projection_;
  // DENORM used by projection_ to get back to image coords. Borrowed pointer.
  const DENORM* denorm_;
  // Bounding box of the grid.
  TBOX grid_box_;
  // Rerotation to get back to the original image.
  FCOORD rerotation_;
  // Windows for debug display.
  ScrollView* leaders_win_;
  ScrollView* initial_widths_win_;
  ScrollView* widths_win_;
  ScrollView* chains_win_;
  ScrollView* diacritics_win_;
  ScrollView* textlines_win_;
  ScrollView* smoothed_win_;
};

}  // namespace tesseract.

#endif  // TESSERACT_TEXTORD_STROKEWIDTH_H__
