///////////////////////////////////////////////////////////////////////
// File:        colfind.h
// Description: Class to find columns in the grid of BLOBNBOXes.
// Author:      Ray Smith
// Created:     Thu Feb 21 14:04:01 PST 2008
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

#ifndef TESSERACT_TEXTORD_COLFIND_H__
#define TESSERACT_TEXTORD_COLFIND_H__

#include "tabfind.h"
#include "imagefind.h"
#include "colpartitiongrid.h"
#include "colpartitionset.h"
#include "ocrblock.h"
#include "textlineprojection.h"

class BLOCK_LIST;
struct Boxa;
struct Pixa;
class DENORM;
class ScrollView;
class STATS;
class TO_BLOCK;

namespace tesseract {

extern BOOL_VAR_H(textord_tabfind_find_tables, false, "run table detection");

class ColPartitionSet;
class ColPartitionSet_LIST;
class ColSegment_LIST;
class ColumnGroup_LIST;
class LineSpacing;
class StrokeWidth;
class TempColumn_LIST;
class EquationDetectBase;

// The ColumnFinder class finds columns in the grid.
class ColumnFinder : public TabFind {
 public:
  // Gridsize is an estimate of the text size in the image. A suitable value
  // is in TO_BLOCK::line_size after find_components has been used to make
  // the blobs.
  // bleft and tright are the bounds of the image (rectangle) being processed.
  // vlines is a (possibly empty) list of TabVector and vertical_x and y are
  // the sum logical vertical vector produced by LineFinder::FindVerticalLines.
  // If cjk_script is true, then broken CJK characters are fixed during
  // layout analysis to assist in detecting horizontal vs vertically written
  // textlines.
  ColumnFinder(int gridsize, const ICOORD& bleft, const ICOORD& tright,
               int resolution, bool cjk_script, double aligned_gap_fraction,
               TabVector_LIST* vlines, TabVector_LIST* hlines,
               int vertical_x, int vertical_y);
  virtual ~ColumnFinder();

  // Accessors for testing
  const DENORM* denorm() const {
    return denorm_;
  }
  const TextlineProjection* projection() const {
    return &projection_;
  }
  void set_cjk_script(bool is_cjk) {
    cjk_script_ = is_cjk;
  }

  // ======================================================================
  // The main function of ColumnFinder is broken into pieces to facilitate
  // optional insertion of orientation and script detection in an efficient
  // way. The calling sequence IS MANDATORY however, whether or not
  // OSD is being used:
  // 1. Construction.
  // 2. SetupAndFilterNoise.
  // 3. IsVerticallyAlignedText.
  // 4. CorrectOrientation.
  // 5. FindBlocks.
  // 6. Destruction. Use of a single column finder for multiple images does not
  //    make sense.
  // Throughout these steps, the ColPartitions are owned by part_grid_, which
  // means that that it must be kept correct. Exception: big_parts_ owns its
  // own ColPartitions.
  // The BLOBNBOXes are owned by the input TO_BLOCK for the whole time, except
  // for a phase in FindBlocks before TransformToBlocks, when they become
  // owned by the ColPartitions. The owner() ColPartition of a BLOBNBOX
  // indicates more of a betrothal for the majority of layout analysis, ie
  // which ColPartition will take ownership when the blobs are release from
  // the input TO_BLOCK. Exception: image_bblobs_ owns the fake blobs that
  // are part of the image regions, as they are not on any TO_BLOCK list.
  // TODO(rays) break up column finder further into smaller classes, as
  // there is a lot more to it than column finding now.
  // ======================================================================

  // Performs initial processing on the blobs in the input_block:
  // Setup the part_grid, stroke_width_, nontext_map_.
  // Obvious noise blobs are filtered out and used to mark the nontext_map_.
  // Initial stroke-width analysis is used to get local text alignment
  // direction, so the textline projection_ map can be setup.
  // On return, IsVerticallyAlignedText may be called (now optionally) to
  // determine the gross textline alignment of the page.
  void SetupAndFilterNoise(Pix* photo_mask_pix, TO_BLOCK* input_block);

  // Tests for vertical alignment of text (returning true if so), and generates
  // a list of blobs (in osd_blobs) for orientation and script detection.
  // block is the single block for the whole page or rectangle to be OCRed.
  // Note that the vertical alignment may be due to text whose writing direction
  // is vertical, like say Japanese, or due to text whose writing direction is
  // horizontal but whose text appears vertically aligned because the image is
  // not the right way up.
  // find_vertical_text_ratio should be textord_tabfind_vertical_text_ratio.
  bool IsVerticallyAlignedText(double find_vertical_text_ratio,
                               TO_BLOCK* block, BLOBNBOX_CLIST* osd_blobs);

  // Rotates the blobs and the TabVectors so that the gross writing direction
  // (text lines) are horizontal and lines are read down the page.
  // Applied rotation stored in rotation_.
  // A second rotation is calculated for application during recognition to
  // make the rotated blobs upright for recognition.
  // Subsequent rotation stored in text_rotation_.
  //
  // Arguments:
  //   vertical_text_lines is true if the text lines are vertical.
  //   recognition_rotation [0..3] is the number of anti-clockwise 90 degree
  //   rotations from osd required for the text to be upright and readable.
  void CorrectOrientation(TO_BLOCK* block, bool vertical_text_lines,
                          int recognition_rotation);

  // Finds blocks of text, image, rule line, table etc, returning them in the
  // blocks and to_blocks
  // (Each TO_BLOCK points to the basic BLOCK and adds more information.)
  // Image blocks are generated by a combination of photo_mask_pix (which may
  // NOT be NULL) and the rejected text found during preliminary textline
  // finding.
  // The input_block is the result of a call to find_components, and contains
  // the blobs found in the image or rectangle to be OCRed. These blobs will be
  // removed and placed in the output blocks, while unused ones will be deleted.
  // If single_column is true, the input is treated as single column, but
  // it is still divided into blocks of equal line spacing/text size.
  // scaled_color is scaled down by scaled_factor from the input color image,
  // and may be NULL if the input was not color.
  // grey_pix is optional, but if present must match the photo_mask_pix in size,
  // and must be a *real* grey image instead of binary_pix * 255.
  // thresholds_pix is expected to be present iff grey_pix is present and
  // can be an integer factor reduction of the grey_pix. It represents the
  // thresholds that were used to create the binary_pix from the grey_pix.
  // Returns -1 if the user hits the 'd' key in the blocks window while running
  // in debug mode, which requests a retry with more debug info.
  int FindBlocks(PageSegMode pageseg_mode,
                 Pix* scaled_color, int scaled_factor,
                 TO_BLOCK* block, Pix* photo_mask_pix,
                 Pix* thresholds_pix, Pix* grey_pix,
                 BLOCK_LIST* blocks, TO_BLOCK_LIST* to_blocks);

  // Get the rotation required to deskew, and its inverse rotation.
  void GetDeskewVectors(FCOORD* deskew, FCOORD* reskew);

  // Set the equation detection pointer.
  void SetEquationDetect(EquationDetectBase* detect);

 private:
  // Displays the blob and block bounding boxes in a window called Blocks.
  void DisplayBlocks(BLOCK_LIST* blocks);
  // Displays the column edges at each grid y coordinate defined by
  // best_columns_.
  void DisplayColumnBounds(PartSetVector* sets);

  ////// Functions involved in determining the columns used on the page. /////

  // Sets up column_sets_ (the determined column layout at each horizontal
  // slice). Returns false if the page is empty.
  bool MakeColumns(bool single_column);
  // Attempt to improve the column_candidates by expanding the columns
  // and adding new partitions from the partition sets in src_sets.
  // Src_sets may be equal to column_candidates, in which case it will
  // use them as a source to improve themselves.
  void ImproveColumnCandidates(PartSetVector* src_sets,
                               PartSetVector* column_sets);
  // Prints debug information on the column candidates.
  void PrintColumnCandidates(const char* title);
  // Finds the optimal set of columns that cover the entire image with as
  // few changes in column partition as possible.
  // Returns true if any part of the page is multi-column.
  bool AssignColumns(const PartSetVector& part_sets);
  // Finds the biggest range in part_sets_ that has no assigned column, but
  // column assignment is possible.
  bool BiggestUnassignedRange(int set_count, const bool* any_columns_possible,
                              int* start, int* end);
  // Finds the modal compatible column_set_ index within the given range.
  int RangeModalColumnSet(int** column_set_costs, const int* assigned_costs,
                          int start, int end);
  // Given that there are many column_set_id compatible columns in the range,
  // shrinks the range to the longest contiguous run of compatibility, allowing
  // gaps where no columns are possible, but not where competing columns are
  // possible.
  void ShrinkRangeToLongestRun(int** column_set_costs,
                               const int* assigned_costs,
                               const bool* any_columns_possible,
                               int column_set_id,
                               int* best_start, int* best_end);
  // Moves start in the direction of step, upto, but not including end while
  // the only incompatible regions are no more than kMaxIncompatibleColumnCount
  // in size, and the compatible regions beyond are bigger.
  void ExtendRangePastSmallGaps(int** column_set_costs,
                                const int* assigned_costs,
                                const bool* any_columns_possible,
                                int column_set_id,
                                int step, int end, int* start);
  // Assigns the given column_set_id to the part_sets_ in the given range.
  void AssignColumnToRange(int column_set_id, int start, int end,
                           int** column_set_costs, int* assigned_costs);

  // Computes the mean_column_gap_.
  void ComputeMeanColumnGap(bool any_multi_column);

  //////// Functions that manipulate ColPartitions in the part_grid_ /////
  //////// to split, merge, find margins, and find types.  //////////////

  // Hoovers up all un-owned blobs and deletes them.
  // The rest get released from the block so the ColPartitions can pass
  // ownership to the output blocks.
  void ReleaseBlobsAndCleanupUnused(TO_BLOCK* block);
  // Splits partitions that cross columns where they have nothing in the gap.
  void GridSplitPartitions();
  // Merges partitions where there is vertical overlap, within a single column,
  // and the horizontal gap is small enough.
  void GridMergePartitions();
  // Inserts remaining noise blobs into the most applicable partition if any.
  // If there is no applicable partition, then the blobs are deleted.
  void InsertRemainingNoise(TO_BLOCK* block);
  // Remove partitions that come from horizontal lines that look like
  // underlines, but are not part of a table.
  void GridRemoveUnderlinePartitions();
  // Add horizontal line separators as partitions.
  void GridInsertHLinePartitions();
  // Add vertical line separators as partitions.
  void GridInsertVLinePartitions();
  // For every ColPartition in the grid, sets its type based on position
  // in the columns.
  void SetPartitionTypes();
  // Only images remain with multiple types in a run of partners.
  // Sets the type of all in the group to the maximum of the group.
  void SmoothPartnerRuns();

  //////// Functions that make the final output blocks             ///////

  // Helper functions for TransformToBlocks.
  // Add the part to the temp list in the correct order.
  void AddToTempPartList(ColPartition* part, ColPartition_CLIST* temp_list);
  // Add everything from the temp list to the work_set assuming correct order.
  void EmptyTempPartList(ColPartition_CLIST* temp_list,
                         WorkingPartSet_LIST* work_set);

  // Transform the grid of partitions to the output blocks.
  void TransformToBlocks(BLOCK_LIST* blocks, TO_BLOCK_LIST* to_blocks);

  // Reflect the blob boxes (but not the outlines) in the y-axis so that
  // the blocks get created in the correct RTL order. Rotates the blobs
  // in the input_block and the bblobs list.
  // The reflection is undone in RotateAndReskewBlocks by
  // reflecting the blocks themselves, and then recomputing the blob bounding
  //  boxes.
  void ReflectForRtl(TO_BLOCK* input_block, BLOBNBOX_LIST* bblobs);

  // Undo the deskew that was done in FindTabVectors, as recognition is done
  // without correcting blobs or blob outlines for skew.
  // Reskew the completed blocks to put them back to the original rotated coords
  // that were created by CorrectOrientation.
  // If the input_is_rtl, then reflect the blocks in the y-axis to undo the
  // reflection that was done before FindTabVectors.
  // Blocks that were identified as vertical text (relative to the rotated
  // coordinates) are further rotated so the text lines are horizontal.
  // blob polygonal outlines are rotated to match the position of the blocks
  // that they are in, and their bounding boxes are recalculated to be accurate.
  // Record appropriate inverse transformations and required
  // classifier transformation in the blocks.
  void RotateAndReskewBlocks(bool input_is_rtl, TO_BLOCK_LIST* to_blocks);

  // Computes the rotations for the block (to make textlines horizontal) and
  // for the blobs (for classification) and sets the appropriate members
  // of the given block.
  // Returns the rotation that needs to be applied to the blobs to make
  // them sit in the rotated block.
  FCOORD ComputeBlockAndClassifyRotation(BLOCK* block);

  // If true then the page language is cjk, so it is safe to perform
  // FixBrokenCJK.
  bool cjk_script_;
  // The minimum gutter width to apply for finding columns.
  // Modified when vertical text is detected to prevent detection of
  // vertical text lines as columns.
  int min_gutter_width_;
  // The mean gap between columns over the page.
  int mean_column_gap_;
  // Config param saved at construction time. Modifies min_gutter_width_ with
  // vertical text to prevent detection of vertical text as columns.
  double tabfind_aligned_gap_fraction_;
  // The rotation vector needed to convert original coords to deskewed.
  FCOORD deskew_;
  // The rotation vector needed to convert deskewed back to original coords.
  FCOORD reskew_;
  // The rotation vector used to rotate vertically oriented pages.
  FCOORD rotation_;
  // The rotation vector needed to convert the rotated back to original coords.
  FCOORD rerotate_;
  // The additional rotation vector needed to rotate text for recognition.
  FCOORD text_rotation_;
  // The column_sets_ contain the ordered candidate ColPartitionSets that
  // define the possible divisions of the page into columns.
  PartSetVector column_sets_;
  // A simple array of pointers to the best assigned column division at
  // each grid y coordinate.
  ColPartitionSet** best_columns_;
  // The grid used for creating initial partitions with strokewidth.
  StrokeWidth* stroke_width_;
  // The grid used to hold ColPartitions after the columns have been determined.
  ColPartitionGrid part_grid_;
  // List of ColPartitions that are no longer needed after they have been
  // turned into regions, but are kept around because they are referenced
  // by the part_grid_.
  ColPartition_LIST good_parts_;
  // List of ColPartitions that are big and might be dropcap or vertically
  // joined.
  ColPartition_LIST big_parts_;
  // List of ColPartitions that have been declared noise.
  ColPartition_LIST noise_parts_;
  // The fake blobs that are made from the images.
  BLOBNBOX_LIST image_bblobs_;
  // Horizontal line separators.
  TabVector_LIST horizontal_lines_;
  // Image map of photo/noise areas on the page.
  Pix* nontext_map_;
  // Textline projection map.
  TextlineProjection projection_;
  // Sequence of DENORMS that indicate how to get back to the original image
  // coordinate space. The destructor must delete all the DENORMs in the chain.
  DENORM* denorm_;

  // Various debug windows that automatically go away on completion.
  ScrollView* input_blobs_win_;

  // The equation region detector pointer. Note: This pointer is passed in by
  // member function SetEquationDetect, and releasing it is NOT owned by this
  // class.
  EquationDetectBase* equation_detect_;

  // Allow a subsequent instance to reuse the blocks window.
  // Not thread-safe, but multiple threads shouldn't be using windows anyway.
  static ScrollView* blocks_win_;
};

}  // namespace tesseract.

#endif  // TESSERACT_TEXTORD_COLFIND_H__
