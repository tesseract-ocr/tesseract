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

class ScrollView;
class TO_BLOCK;
class STATS;
class BLOCK_LIST;
struct Boxa;
struct Pixa;

namespace tesseract {

extern BOOL_VAR_H(textord_tabfind_find_tables, false, "run table detection");

class StrokeWidth;
class LineSpacing;
class TempColumn_LIST;
class ColSegment_LIST;
class ColumnGroup_LIST;
class ColPartitionSet;
class ColPartitionSet_LIST;

// The ColumnFinder class finds columns in the grid.
class ColumnFinder : public TabFind {
 public:
  // Gridsize is an estimate of the text size in the image. A suitable value
  // is in TO_BLOCK::line_size after find_components has been used to make
  // the blobs.
  // bleft and tright are the bounds of the image (rectangle) being processed.
  // vlines is a (possibly empty) list of TabVector and vertical_x and y are
  // the sum logical vertical vector produced by LineFinder::FindVerticalLines.
  ColumnFinder(int gridsize, const ICOORD& bleft, const ICOORD& tright,
               int resolution, TabVector_LIST* vlines, TabVector_LIST* hlines,
               int vertical_x, int vertical_y);
  virtual ~ColumnFinder();

  // ======================================================================
  // The main function of ColumnFinder is broken into pieces to facilitate
  // optional insertion of orientation and script detection in an efficient
  // way. The calling sequence IS MANDATORY however, whether or not
  // OSD is being used:
  // 1. Construction.
  // 2. IsVerticallyAlignedText.
  // 3. CorrectOrientation.
  // 4. FindBlocks.
  // 5. Destruction. Use of a single column finder for multiple images does not
  //    make sense.
  // TODO(rays) break up column finder further into smaller classes, as
  // there is a lot more to it than column finding now.
  // ======================================================================

  // Tests for vertical alignment of text (returning true if so), and
  // generates a list of blobs for orientation and script detection. Note that
  // the vertical alignment may be due to text whose writing direction is
  // vertical, like say Japanese, or due to text whose writing direction is
  // horizontal but whose text appears vertically aligned because the image is
  // not the right way up.
  bool IsVerticallyAlignedText(TO_BLOCK* block, BLOBNBOX_CLIST* osd_blobs);

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

  // Finds the text and image blocks, returning them in the blocks and to_blocks
  // lists. (Each TO_BLOCK points to the basic BLOCK and adds more information.)
  // If boxa and pixa are not NULL, they are assumed to be the output of
  // ImageFinder::FindImages, and are used to generate image blocks.
  // The input boxa and pixa are destroyed.
  // Imageheight should be the pixel height of the original image.
  // The input block is the result of a call to find_components, and contains
  // the blobs found in the image. These blobs will be removed and placed
  // in the output blocks, while unused ones will be deleted.
  // If single_column is true, the input is treated as single column, but
  // it is still divided into blocks of equal line spacing/text size.
  // Returns -1 if the user requested retry with more debug info.
  int FindBlocks(bool single_column, int imageheight,
                 TO_BLOCK* block, Boxa* boxa, Pixa* pixa,
                 BLOCK_LIST* blocks, TO_BLOCK_LIST* to_blocks);

  // Get the rotation required to deskew, and its inverse rotation.
  void GetDeskewVectors(FCOORD* deskew, FCOORD* reskew);

 private:
  // Displays the blob and block bounding boxes in a window called Blocks.
  void DisplayBlocks(BLOCK_LIST* blocks);
  // Displays the column edges at each grid y coordinate defined by
  // best_columns_.
  void DisplayColumnBounds(PartSetVector* sets);

  // Converts the arrays of Box/Pix to a list of C_OUTLINE, and then to blobs.
  // The output is a list of C_BLOBs for the images, but the C_OUTLINEs
  // contain no data.
  void ExtractImageBlobs(int image_height, Boxa* boxa, Pixa* pixa);

  ////// Functions involved in making the initial ColPartitions. /////

  // Creates the initial ColPartitions, and puts them in a ColPartitionSet
  // for each grid y coordinate, storing the ColPartitionSets in part_sets_.
  // After creating the ColPartitonSets, attempts to merge them where they
  // overlap and unique the BLOBNBOXes within.
  // The return value is the number of ColPartitionSets made.
  int MakeColumnPartitions();
  // Partition the BLOBNBOXES horizontally at the given grid y, creating a
  // ColPartitionSet which is returned. NULL is returned if there are no
  // BLOBNBOXES at the given grid y.
  ColPartitionSet* PartitionsAtGridY(int grid_y);
  // Insert the blobs in the given list into the main grid and for
  // each one also make it a separate unknown partition.
  // If filter is true, use only the blobs that are above a threshold in
  // size or a non-isolated.
  void InsertSmallBlobsAsUnknowns(bool filter, BLOBNBOX_LIST* blobs);
  // Helper function for PartitionsAtGridY, with a long argument list.
  // This bbox is of unknown type, so it is added to an unk_partition.
  // If the edge is past the unk_right_margin then unk_partition has to be
  // completed and a new one made. See CompletePartition and StartPartition
  // for the other args.
  void ProcessUnknownBlob(int page_edge, BLOBNBOX* bbox,
                          ColPartition** unk_partition,
                          ColPartition_IT* unk_part_it,
                          TabVector** unk_right_line,
                          int* unk_right_margin,
                          int* unk_prev_margin,
                          bool* unk_edge_is_left);
  // Creates and returns a new ColPartition of the given start_type
  // and adds the given bbox to it.
  // Also finds the left and right tabvectors that bound the textline, setting
  // the members of the returned ColPartition appropriately:
  // If the left tabvector is less constraining than the input left_margin
  // (assumed to be the right edge of the previous partition), then the
  // tabvector is ignored and the left_margin used instead.
  // If the right tabvector is more constraining than the input *right_margin,
  // (probably the right edge of the page), then the *right_margin is adjusted
  // to use the tabvector.
  // *edge_is_left is set to true if the right tabvector is good and used as the
  // margin, so we can include blobs that overhang the tabvector in this
  // partition.
  ColPartition* StartPartition(BlobRegionType start_type, int left_margin,
                               BLOBNBOX* bbox, TabVector** right_line,
                               int* right_margin, bool* edge_is_left);
  // Completes the given partition, and adds it to the given iterator.
  // The right_margin on input is the left edge of the next blob if there is
  // one. The right tab vector plus a margin is used as the right margin if
  // it is more constraining than the next blob, but if there are no more
  // blobs, we want the right margin to make it to the page edge.
  // The return value is the next left margin, being the right edge of the
  // bounding box of blobs.
  int CompletePartition(bool no_more_blobs, int page_edge,
                        TabVector* right_line, int* right_margin,
                        ColPartition** partition, ColPartition_IT* part_it);


  ////// Functions involved in determining the columns used on the page. /////

  // Makes an ordered list of candidates to partition the width of the page
  // into columns using the part_sets_.
  // See AddToColumnSetsIfUnique for the ordering.
  // If single_column, then it just makes a single page-wide fake column.
  void MakeColumnCandidates(bool single_column);
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
  void AssignColumns();
  // Finds the biggest range in part_sets_ that has no assigned column, but
  // column assignment is possible.
  bool BiggestUnassignedRange(const bool* any_columns_possible,
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
  void ComputeMeanColumnGap();

  //////// Functions that manipulate ColPartitions in the part_grid_ /////
  //////// to split, merge, find margins, and find types.  //////////////

  // Removes the ColPartitions from part_sets_, the ColPartitionSets that
  // contain them, and puts them in the part_grid_ after ensuring that no
  // BLOBNBOX is owned by more than one of them.
  void MovePartitionsToGrid();
  // Splits partitions that cross columns where they have nothing in the gap.
  void GridSplitPartitions();
  // Merges partitions where there is vertical overlap, within a single column,
  // and the horizontal gap is small enough.
  void GridMergePartitions();
  // Resolves unknown partitions from the unknown_parts_ list by merging them
  // with a close neighbour, inserting them into the grid with a known type,
  // or declaring them to be noise.
  void GridInsertUnknowns();
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

  // Undo the deskew that was done in FindTabVectors, as recognition is done
  // without correcting blobs or blob outlines for skew.
  // Reskew the completed blocks to put them back to the original rotated coords
  // that were created by CorrectOrientation.
  // Blocks that were identified as vertical text (relative to the rotated
  // coordinates) are further rotated so the text lines are horizontal.
  // blob polygonal outlines are rotated to match the position of the blocks
  // that they are in, and their bounding boxes are recalculated to be accurate.
  // Record appropriate inverse transformations and required
  // classifier transformation in the blocks.
  void RotateAndReskewBlocks(TO_BLOCK_LIST* to_blocks);


  // Move all the small and noise blobs into the main blobs list of
  // the block from the to_blocks list that contains them.
  void MoveSmallBlobs(BLOBNBOX_LIST* bblobs, TO_BLOCK_LIST* to_blocks);

  // The minimum gutter width to apply for finding columns.
  // Modified when vertical text is detected to prevent detection of
  // vertical text lines as columns.
  int min_gutter_width_;
  // The mean gap between columns over the page.
  int mean_column_gap_;
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
  // The part_sets_ are the initial text-line-like partition of the grid,
  // and is a vector of ColPartitionSets.
  PartSetVector part_sets_;
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
  // List of ColPartitions of unknown type.
  ColPartition_LIST unknown_parts_;
  // List of ColPartitions that have been declared noise.
  ColPartition_LIST noise_parts_;
  // The fake blobs that are made from the input boxa/pixa pair.
  BLOBNBOX_LIST image_bblobs_;
  // Horizontal line separators.
  TabVector_LIST horizontal_lines_;
  // Allow a subsequent instance to reuse the blocks window.
  // Not thread-safe, but multiple threads shouldn't be using windows anyway.
  static ScrollView* blocks_win_;
};

}  // namespace tesseract.

#endif  // TESSERACT_TEXTORD_COLFIND_H__
