///////////////////////////////////////////////////////////////////////
// File:        tablefind.h
// Description: Helper classes to find tables from ColPartitions.
// Author:      Faisal Shafait (faisal.shafait@dfki.de)
// Created:     Tue Jan 06 11:13:01 PST 2009
//
// (C) Copyright 2009, Google Inc.
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

#ifndef TESSERACT_TEXTORD_TABLEFIND_H__
#define TESSERACT_TEXTORD_TABLEFIND_H__

#include "colpartitiongrid.h"
#include "elst.h"
#include "rect.h"

namespace tesseract {

// Possible types for a column segment.
enum ColSegType {
  COL_UNKNOWN,
  COL_TEXT,
  COL_TABLE,
  COL_MIXED,
  COL_COUNT
};

class ColPartitionSet;

// ColSegment holds rectangular blocks that represent segmentation of a page
// into regions containing single column text/table.
class ColSegment;
ELISTIZEH(ColSegment)
CLISTIZEH(ColSegment)

class ColSegment : public ELIST_LINK {
 public:
  ColSegment();
  ~ColSegment();

  // Simple accessors and mutators
  const TBOX& bounding_box() const {
    return bounding_box_;
  }

  void set_top(int y) {
    bounding_box_.set_top(y);
  }

  void set_bottom(int y) {
    bounding_box_.set_bottom(y);
  }

  void set_left(int x) {
    bounding_box_.set_left(x);
  }

  void set_right(int x) {
    bounding_box_.set_right(x);
  }

  void set_bounding_box(const TBOX& other) {
    bounding_box_ = other;
  }

  int get_num_table_cells() const {
    return num_table_cells_;
  }

  // set the number of table colpartitions covered by the bounding_box_
  void set_num_table_cells(int n) {
    num_table_cells_ = n;
  }

  int get_num_text_cells() const {
    return num_text_cells_;
  }

  // set the number of text colpartitions covered by the bounding_box_
  void set_num_text_cells(int n) {
    num_text_cells_ = n;
  }

  ColSegType type() const {
    return type_;
  }

  // set the type of the block based on the ratio of table to text
  // colpartitions covered by it.
  void set_type();

  // Provides a color for BBGrid to draw the rectangle.
  ScrollView::Color  BoxColor() const;

  // Insert a rectangle into bounding_box_
  void InsertBox(const TBOX& other);

 private:
  TBOX bounding_box_;                    // bounding box
  int num_table_cells_;
  int num_text_cells_;
  ColSegType type_;
};

// Typedef BBGrid of ColSegments
typedef BBGrid<ColSegment,
               ColSegment_CLIST,
               ColSegment_C_IT> ColSegmentGrid;
typedef GridSearch<ColSegment,
                   ColSegment_CLIST,
                   ColSegment_C_IT> ColSegmentGridSearch;

// TableFinder is a utility class to find a set of tables given a set of
// ColPartitions and Columns. The TableFinder will mark candidate ColPartitions
// based on research in "Table Detection in Heterogeneous Documents".
// Usage flow is as follows:
//   TableFinder finder;
//   finder.InsertCleanPartitions(/* grid info */)
//   finder.LocateTables(/* ColPartitions and Columns */);
//   finder.Update TODO(nbeato)
class TableFinder {
 public:
  // Constructor is simple initializations
  TableFinder();
  ~TableFinder();

  // Set the resolution of the connected components in ppi.
  void set_resolution(int resolution) {
    resolution_ = resolution;
  }
  // Change the reading order. Initially it is left to right.
  void set_left_to_right_language(bool order);

  // Initialize
  void Init(int grid_size, const ICOORD& bottom_left, const ICOORD& top_right);

  // Copy cleaned partitions from ColumnFinder's part_grid_ to this
  // clean_part_grid_ and insert dot-like noise into period_grid_.
  // It resizes the grids in this object to the dimensions of grid.
  void InsertCleanPartitions(ColPartitionGrid* grid, TO_BLOCK* block);

  // High level function to perform table detection
  // Finds tables and updates the grid object with new partitions for the
  // tables. The columns and width callbacks are used to merge tables.
  // The reskew argument is only used to write the tables to the out.png
  // if that feature is enabled.
  void LocateTables(ColPartitionGrid* grid,
                    ColPartitionSet** columns,
                    WidthCallback* width_cb,
                    const FCOORD& reskew);

 protected:
  // Access for the grid dimensions.
  // The results will not be correct until InsertCleanPartitions
  // has been called. The values are taken from the grid passed as an argument
  // to that function.
  int gridsize() const;
  int gridwidth() const;
  int gridheight() const;
  const ICOORD& bleft() const;
  const ICOORD& tright() const;

  // Makes a window for debugging, see BBGrid
  ScrollView* MakeWindow(int x, int y, const char* window_name);

  //////// Functions to insert objects from the grid into the table finder.
  //////// In all cases, ownership is transferred to the table finder.
  // Inserts text into the table finder.
  void InsertTextPartition(ColPartition* part);
  void InsertFragmentedTextPartition(ColPartition* part);
  void InsertLeaderPartition(ColPartition* part);
  void InsertRulingPartition(ColPartition* part);
  void InsertImagePartition(ColPartition* part);
  void SplitAndInsertFragmentedTextPartition(ColPartition* part);
  bool AllowTextPartition(const ColPartition& part) const;
  bool AllowBlob(const BLOBNBOX& blob) const;

  //////// Functions that manipulate ColPartitions in the part_grid_ /////
  //////// to find tables.
  ////////

  // Utility function to move segments to col_seg_grid
  // Note: Move includes ownership,
  // so segments will be be owned by col_seg_grid
  void MoveColSegmentsToGrid(ColSegment_LIST* segments,
                             ColSegmentGrid* col_seg_grid);

  //////// Set up code to run during table detection to correctly
  //////// initialize variables on column partitions that are used later.
  ////////

  // Initialize the grid and partitions
  void InitializePartitions(ColPartitionSet** all_columns);

  // Set left, right and top, bottom spacings of each colpartition.
  // Left/right spacings are w.r.t the column boundaries
  // Top/bottom spacings are w.r.t. previous and next colpartitions
  static void SetPartitionSpacings(ColPartitionGrid* grid,
                                   ColPartitionSet** all_columns);

  // Set spacing and closest neighbors above and below a given colpartition.
  void SetVerticalSpacing(ColPartition* part);

  // Set global spacing estimates. This function is dependent on the
  // partition spacings. So make sure SetPartitionSpacings is called
  // on the same grid before this.
  void SetGlobalSpacings(ColPartitionGrid* grid);
  // Access to the global median xheight. The xheight is the height
  // of a lowercase 'x' character on the page. This can be viewed as the
  // average height of a lowercase letter in a textline. As a result
  // it is used to make assumptions about spacing between words and
  // table cells.
  void set_global_median_xheight(int xheight);
  // Access to the global median blob width. The width is useful
  // when deciding if a partition is noise.
  void set_global_median_blob_width(int width);
  // Access to the global median ledding. The ledding is the distance between
  // two adjacent text lines. This value can be used to get a rough estimate
  // for the amount of space between two lines of text. As a result, it
  // is used to calculate appropriate spacing between adjacent rows of text.
  void set_global_median_ledding(int ledding);

  // Updates the nearest neighbors for each ColPartition in clean_part_grid_.
  // The neighbors are most likely SingletonPartner calls after the neighbors
  // are assigned. This is hear until it is decided to remove the
  // nearest_neighbor code in ColPartition
  void FindNeighbors();

  //////// Functions to mark candidate column partitions as tables.
  //////// Tables are marked as described in
  ////////   Table Detection in Heterogeneous Documents (2010, Shafait & Smith)
  ////////

  // High level function to mark partitions as table rows/cells.
  // When this function is done, the column partitions in clean_part_grid_
  // should mostly be marked as tables.
  void MarkTablePartitions();
  // Marks partitions given a local view of a single partition
  void MarkPartitionsUsingLocalInformation();
  /////// Heuristics for local marking
  // Check if the partition has at least one large gap between words or no
  // significant gap at all
  // TODO(nbeato): Make const, prevented because blobnbox array access
  bool HasWideOrNoInterWordGap(ColPartition* part) const;
  // Checks if a partition is adjacent to leaders on the page
  bool HasLeaderAdjacent(const ColPartition& part);
  // Filter individual text partitions marked as table partitions
  // consisting of paragraph endings, small section headings, and
  // headers and footers.
  void FilterFalseAlarms();
  void FilterParagraphEndings();
  void FilterHeaderAndFooter();
  // Mark all ColPartitions as table cells that have a table cell above
  // and below them
  void SmoothTablePartitionRuns();

  //////// Functions to create bounding boxes (ColSegment) objects for
  //////// the columns on the page. The columns are not necessarily
  //////// vertical lines, meaning if tab stops strongly suggests that
  //////// a column changes horizontal position, as in the case below,
  //////// The ColSegment objects will respect that after processing.
  ////////
  ////////     _____________
  //////// Ex. |     |      |
  ////////     |_____|______|  5 boxes: 2 on this line
  ////////     |   |    |   |           3 on this line
  ////////     |___|____|___|
  ////////

  // Get Column segments from best_columns_
  void GetColumnBlocks(ColPartitionSet** columns,
                       ColSegment_LIST *col_segments);

  // Group Column segments into consecutive single column regions.
  void GroupColumnBlocks(ColSegment_LIST *current_segments,
                        ColSegment_LIST *col_segments);

  // Check if two boxes are consecutive within the same column
  bool ConsecutiveBoxes(const TBOX &b1, const TBOX &b2);

  // Set the ratio of candidate table partitions in each column
  void SetColumnsType(ColSegment_LIST* col_segments);

  // Merge Column Blocks that were split due to the presence of a table
  void GridMergeColumnBlocks();

  //////// Functions to turn marked ColPartitions into candidate tables
  //////// using a modified T-Recs++ algorithm described in
  ////////   Applying The T-Recs Table Recognition System
  ////////   To The Business Letter Domain (2001, Kieninger & Dengel)
  ////////

  // Merge partititons cells into table columns
  // Differs from paper by just looking at marked table partitions
  // instead of similarity metric.
  // Modified section 4.1 of paper.
  void GetTableColumns(ColSegment_LIST *table_columns);

  // Finds regions within a column that potentially contain a table.
  // Ie, the table columns from GetTableColumns are turned into boxes
  // that span the entire page column (using ColumnBlocks found in
  // earlier functions) in the x direction and the min/max extent of
  // overlapping table columns in the y direction.
  // Section 4.2 of paper.
  void GetTableRegions(ColSegment_LIST *table_columns,
                       ColSegment_LIST *table_regions);


  //////// Functions to "patch up" found tables
  ////////

  // Merge table regions corresponding to tables spanning multiple columns
  void GridMergeTableRegions();
  bool BelongToOneTable(const TBOX &box1, const TBOX &box2);

  // Adjust table boundaries by building a tight bounding box around all
  // ColPartitions contained in it.
  void AdjustTableBoundaries();

  // Grows a table to include partitions that are partially covered
  // by the table. This includes lines and text. It does not include
  // noise or images.
  // On entry, result_box is the minimum size of the result. The results of the
  // function will union the actual result with result_box.
  void GrowTableBox(const TBOX& table_box, TBOX* result_box);
  // Grow a table by increasing the size of the box to include
  // partitions with significant overlap with the table.
  void GrowTableToIncludePartials(const TBOX& table_box,
                                  const TBOX& search_range,
                                  TBOX* result_box);
  // Grow a table by expanding to the extents of significantly
  // overlapping lines.
  void GrowTableToIncludeLines(const TBOX& table_box, const TBOX& search_range,
                               TBOX* result_box);
  // Checks whether the horizontal line belong to the table by looking at the
  // side spacing of extra ColParitions that will be included in the table
  // due to expansion
  bool HLineBelongsToTable(const ColPartition& part, const TBOX& table_box);

  // Look for isolated column headers above the given table box and
  // include them in the table
  void IncludeLeftOutColumnHeaders(TBOX* table_box);

  // Remove false alarms consiting of a single column
  void DeleteSingleColumnTables();

  // Return true if at least one gap larger than the global x-height
  // exists in the horizontal projection
  bool GapInXProjection(int* xprojection, int length);

  //////// Recognize the tables.
  ////////
  // This function will run the table recognizer and try to find better
  // bounding boxes. The structures of the tables never leave this function
  // right now. It just tries to prune and merge tables based on info it
  // has available.
  void RecognizeTables();

  //////// Debugging functions. Render different structures to GUI
  //////// for visual debugging / intuition.
  ////////

  // Displays Colpartitions marked as table row. Overlays them on top of
  // part_grid_.
  void DisplayColSegments(ScrollView* win, ColSegment_LIST *cols,
                          ScrollView::Color color);

  // Displays the colpartitions using a new coloring on an existing window.
  // Note: This method is only for debug purpose during development and
  // would not be part of checked in code
  void DisplayColPartitions(ScrollView* win, ColPartitionGrid* grid,
                            ScrollView::Color text_color,
                            ScrollView::Color table_color);
  void DisplayColPartitions(ScrollView* win, ColPartitionGrid* grid,
                            ScrollView::Color default_color);
  void DisplayColPartitionConnections(ScrollView* win,
                                      ColPartitionGrid* grid,
                                      ScrollView::Color default_color);
  void DisplayColSegmentGrid(ScrollView* win, ColSegmentGrid* grid,
                             ScrollView::Color color);

  // Write ColParitions and Tables to a PIX image
  // Note: This method is only for debug purpose during development and
  // would not be part of checked in code
  void WriteToPix(const FCOORD& reskew);

  // Merge all colpartitions in table regions to make them a single
  // colpartition and revert types of isolated table cells not
  // assigned to any table to their original types.
  void MakeTableBlocks(ColPartitionGrid* grid,
                       ColPartitionSet** columns,
                       WidthCallback* width_cb);

  /////////////////////////////////////////////////
  // Useful objects used during table find process.
  /////////////////////////////////////////////////
  // Resolution of the connected components in ppi.
  int resolution_;
  // Estimate of median x-height over the page
  int global_median_xheight_;
  // Estimate of the median blob width on the page
  int global_median_blob_width_;
  // Estimate of median leading on the page
  int global_median_ledding_;
  // Grid to hold cleaned colpartitions after removing all
  // colpartitions that consist of only noise blobs, and removing
  // noise blobs from remaining colpartitions.
  ColPartitionGrid clean_part_grid_;
  // Grid contains the leaders and ruling lines.
  ColPartitionGrid leader_and_ruling_grid_;
  // Grid contains the broken down column partitions. It can be thought
  // of as a "word" grid. However, it usually doesn't break apart text lines.
  // It does break apart table data (most of the time).
  ColPartitionGrid fragmented_text_grid_;
  // Grid of page column blocks
  ColSegmentGrid col_seg_grid_;
  // Grid of detected tables
  ColSegmentGrid table_grid_;
  // The reading order of text. Defaults to true, for languages such as English.
  bool left_to_right_language_;
};

}  // namespace tesseract.

#endif  // TESSERACT_TEXTORD_TABLEFIND_H__
