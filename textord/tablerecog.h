///////////////////////////////////////////////////////////////////////
// File:        tablerecog.h
// Description: Functions to detect structure of tables.
// Author:    Nicholas Beato
// Created:   Aug 17, 2010
//
// (C) Copyright 2010, Google Inc.
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

#ifndef TABLERECOG_H_
#define TABLERECOG_H_

#include "colpartitiongrid.h"
#include "genericvector.h"

namespace tesseract {

// There are 2 classes in this file. They have 2 different purposes.
//  - StructuredTable contains the methods to find the structure given
//    a specific bounding box and grow that structure.
//  - TableRecognizer contains the methods to adjust the possible positions
//    of a table without worrying about structure.
//
// To use these classes, the assumption is that the TableFinder will
// have a guess of the location of a table (or possibly over/undersegmented
// tables). The TableRecognizer is responsible for finding the table boundaries
// at a high level. The StructuredTable class is responsible for determining
// the structure of the table and trying to maximize its bounds while retaining
// the structure.
// (The latter part is not implemented yet, but that was the goal).
//
// While on the boundary discussion, keep in mind that this is a first pass.
// There should eventually be some things like internal structure checks,
// and, more importantly, surrounding text flow checks.
//

// Usage:
// The StructuredTable class contains methods to query a potential table.
// It has functions to find structure, count rows, find ColPartitions that
// intersect gridlines, etc. It is not meant to blindly find a table. It
// is meant to start with a known table location and enhance it.
// Usage:
//    ColPartitionGrid text_grid, line_grid;  // init
//    TBOX table_box;  // known location of table location
//
//    StructuredTable table;
//    table.Init();  // construction code
//    table.set_text_grid(/* text */);  // These 2 grids can be the same!
//    table.set_line_grid(/* lines */);
//    table.set_min_text_height(10);    // Filter vertical and tall text.
//    // IMPORTANT! The table needs to be told where it is!
//    table.set_bounding_box(table_box);  // Set initial table location.
//    if (table.FindWhitespacedStructure()) {
//      // process table
//      table.column_count();  // number of columns
//      table.row_count();     // number of rows
//      table.cells_count();   // number of cells
//      table.bounding_box();  // updated bounding box
//      // etc.
//    }
//
class StructuredTable {
 public:
  StructuredTable();
  ~StructuredTable();

  // Initialization code. Must be called after the constructor.
  void Init();

  // Sets the grids used by the table. These can be changed between
  // calls to Recognize. They are treated as read-only data.
  void set_text_grid(ColPartitionGrid* text);
  void set_line_grid(ColPartitionGrid* lines);
  // Filters text partitions that are ridiculously tall to prevent
  // merging rows.
  void set_max_text_height(int height);

  // Basic accessors. Some are treated as attributes despite having indirect
  // representation.
  bool is_lined() const;
  int row_count() const;
  int column_count() const;
  int cell_count() const;
  void set_bounding_box(const TBOX& box);
  const TBOX& bounding_box() const;
  int median_cell_height();
  int median_cell_width();
  int row_height(int row) const;
  int column_width(int column) const;
  int space_above() const;
  int space_below() const;

  // Given enough horizontal and vertical lines in a region, create this table
  // based on the structure given by the lines. Return true if it worked out.
  // Code assumes the lines exist. It is the caller's responsibility to check
  // for lines and find an appropriate bounding box.
  bool FindLinedStructure();

  // The main subroutine for finding generic table structure. The function
  // finds the grid structure in the given box. Returns true if a good grid
  // exists, implying that "this" table is valid.
  bool FindWhitespacedStructure();

  ////////
  //////// Functions to query table info.
  ////////

  // Returns true if inserting part into the table does not cause any
  // cell merges.
  bool DoesPartitionFit(const ColPartition& part) const;
  // Checks if a sub-table has multiple data cells filled.
  int CountFilledCells();
  int CountFilledCellsInRow(int row);
  int CountFilledCellsInColumn(int column);
  int CountFilledCells(int row_start, int row_end,
                       int column_start, int column_end);

  // Makes sure that at least one cell in a row has substantial area filled.
  // This can filter out large whitespace caused by growing tables too far
  // and page numbers.
  // (currently bugged for some reason).
  bool VerifyRowFilled(int row);
  // Finds the filled area in a cell.
  double CalculateCellFilledPercentage(int row, int column);

  // Debug display, draws the table in the given color. If the table is not
  // valid, the table and "best" grid lines are still drawn in the given color.
  void Display(ScrollView* window, ScrollView::Color color);

 protected:
  // Clear the structure information.
  void ClearStructure();

  ////////
  //////// Lined tables
  ////////

  // Verifies the lines do not intersect partitions. This happens when
  // the lines are in column boundaries and extend the full page. As a result,
  // the grid lines go through column text. The condition is detectable.
  bool VerifyLinedTableCells();

  ////////
  //////// Tables with whitespace
  ////////

  // This is the function to change if you want to filter resulting tables
  // better. Right now it just checks for a minimum cell count and such.
  // You could add things like maximum number of ColPartitions per cell or
  // similar.
  bool VerifyWhitespacedTable();
  // Find the columns of a table using whitespace.
  void FindWhitespacedColumns();
  // Find the rows of a table using whitespace.
  void FindWhitespacedRows();

  ////////
  //////// Functions to provide information about the table.
  ////////

  // Calculates the whitespace around the table using the table boundary and
  // the supplied grids (set_text_grid and set_line_grid).
  void CalculateMargins();
  // Update the table margins with the supplied grid. This is
  // only called by calculate margins to use multiple grid sources.
  void UpdateMargins(ColPartitionGrid* grid);
  int FindVerticalMargin(ColPartitionGrid* grid, int start_x,
                         bool decrease) const;
  int FindHorizontalMargin(ColPartitionGrid* grid, int start_y,
                           bool decrease) const;
  // Calculates stats on the table, namely the median cell height and width.
  void CalculateStats();

  ////////
  //////// Functions to try to "fix" some table errors.
  ////////

  // Given a whitespaced table, this looks for bordering lines that might
  // be page layout boxes around the table. It is necessary to get the margins
  // correct on the table. If the lines are not joined, the margins will be
  // the distance to the line, which is not right.
  void AbsorbNearbyLines();

  // Nice utility function for finding partition gaps. You feed it a sorted
  // list of all of the mins/maxes of the partitions in the table, and it gives
  // you the gaps (middle). This works for both vertical and horizontal
  // gaps.
  //
  // If you want to allow slight overlap in the division and the partitions,
  // just scale down the partitions before inserting them in the list.
  // Likewise, you can force at least some space between partitions.
  // This trick is how the horizontal partitions are done (since the page
  // skew could make it hard to find splits in the text).
  //
  // As a result, "0 distance" between closest partitions causes a gap.
  // This is not a programmatic assumption. It is intentional and simplifies
  // things.
  //
  // "max_merged" indicates both the minimum number of stacked partitions
  // to cause a cell (add 1 to it), and the maximum number of partitions that
  // a grid line can intersect. For example, if max_merged is 0, then lines
  // are inserted wherever space exists between partitions. If it is 2,
  // lines may intersect 2 partitions at most, but you also need at least
  // 2 partitions to generate a line.
  static void FindCellSplitLocations(const GenericVector<int>& min_list,
                                     const GenericVector<int>& max_list,
                                     int max_merged,
                                     GenericVector<int>* locations);

  ////////
  //////// Utility function for table queries
  ////////

  // Counts the number of ColPartitions that intersect vertical cell
  // division at this x value. Used by VerifyLinedTable.
  int CountVerticalIntersections(int x);
  int CountHorizontalIntersections(int y);

  // Counts how many text partitions are in this box.
  int CountPartitions(const TBOX& box);

  ////////
  //////// Data members.
  ////////

  // Input data, used as read only data to make decisions.
  ColPartitionGrid* text_grid_;    // Text ColPartitions
  ColPartitionGrid* line_grid_;    // Line ColPartitions
  // Table structure.
  // bounding box is a convenient external representation.
  // cell_x_ and cell_y_ indicate the grid lines.
  TBOX bounding_box_;              // Bounding box
  GenericVectorEqEq<int> cell_x_;  // Locations of vertical divisions (sorted)
  GenericVectorEqEq<int> cell_y_;  // Locations of horizontal divisions (sorted)
  bool is_lined_;                  // Is the table backed up by a line structure
  // Table margins, set via CalculateMargins
  int space_above_;
  int space_below_;
  int space_left_;
  int space_right_;
  int median_cell_height_;
  int median_cell_width_;
  // Filters, used to prevent awkward partitions from destroying structure.
  int max_text_height_;
};

class TableRecognizer {
 public:
  TableRecognizer();
  ~TableRecognizer();

  // Initialization code. Must be called after the constructor.
  void Init();

  ////////
  //////// Pre-recognize methods to initial table constraints.
  ////////

  // Sets the grids used by the table. These can be changed between
  // calls to Recognize. They are treated as read-only data.
  void set_text_grid(ColPartitionGrid* text);
  void set_line_grid(ColPartitionGrid* lines);
  // Sets some additional constraints on the table.
  void set_min_height(int height);
  void set_min_width(int width);
  // Filters text partitions that are ridiculously tall to prevent
  // merging rows. Note that "filters" refers to allowing horizontal
  // cells to slice through them on the premise that they were
  // merged text rows during previous layout.
  void set_max_text_height(int height);

  // Given a guess location, the RecognizeTable function will try to find a
  // structured grid in the area. On success, it will return a new
  // StructuredTable (and assumes you will delete it). Otherwise,
  // NULL is returned.
  //
  // Keep in mind, this may "overgrow" or "undergrow" the size of guess.
  // Ideally, there is a either a one-to-one correspondence between
  // the guess and table or no table at all. This is not the best of
  // assumptions right now, but was made to try to keep things simple in
  // the first pass.
  //
  // If a line structure is available on the page in the given region,
  // the table will use the linear structure as it is.
  // Otherwise, it will try to maximize the whitespace around it while keeping
  // a grid structure. This is somewhat working.
  //
  // Since the combination of adjustments can get high, effort was
  // originally made to keep the number of adjustments linear in the number
  // of partitions. The underlying structure finding code used to be
  // much more complex. I don't know how necessary this constraint is anymore.
  // The evaluation of a possible table is kept within O(nlogn) in the size of
  // the table (where size is the number of partitions in the table).
  // As a result, the algorithm is capable of O(n^2 log n). Depending
  // on the grid search size, it may be higher.
  //
  // Last note: it is possible to just try all partition boundaries at a high
  // level O(n^4) and do a verification scheme (at least O(nlogn)). If there
  // area 200 partitions on a page, this could be too costly. Effort could go
  // into pruning the search, but I opted for something quicker. I'm confident
  // that the independent adjustments can get similar results and keep the
  // complextiy down. However, the other approach could work without using
  // TableFinder at all if it is fast enough.  It comes down to properly
  // deciding what is a table. The code currently relies on TableFinder's
  // guess to the location of a table for that.
  StructuredTable* RecognizeTable(const TBOX& guess_box);

 protected:
  ////////
  //////// Lined tables
  ////////

  // Returns true if the given box has a lined table within it. The
  // table argument will be updated with the table if the table exists.
  bool RecognizeLinedTable(const TBOX& guess_box, StructuredTable* table);
  // Returns true if the given box has a large number of horizontal and
  // vertical lines present. If so, we assume the extent of these lines
  // uniquely defines a table and find that table via SolveLinedTable.
  bool HasSignificantLines(const TBOX& guess);

  // Given enough horizontal and vertical lines in a region, find a bounding
  // box that encloses all of them (as well as newly introduced lines).
  // The bounding box is the smallest box that encloses the lines in guess
  // without having any lines sticking out of it.
  // bounding_box is an in/out parameter.
  // On input, it in the extents of the box to search.
  // On output, it is the resulting bounding box.
  bool FindLinesBoundingBox(TBOX* bounding_box);
  // Iteration in above search.
  // bounding_box is an in/out parameter.
  // On input, it in the extents of the box to search.
  // On output, it is the resulting bounding box.
  bool FindLinesBoundingBoxIteration(TBOX* bounding_box);

  ////////
  //////// Generic "whitespaced" tables
  ////////

  // Returns true if the given box has a whitespaced table within it. The
  // table argument will be updated if the table exists. Also note
  // that this method will fail if the guess_box center is not
  // mostly within the table.
  bool RecognizeWhitespacedTable(const TBOX& guess_box, StructuredTable* table);

  // Finds the location of a horizontal split relative to y.
  // This function is mostly unused now. If the SolveWhitespacedTable
  // changes much, it can be removed. Note, it isn't really as reliable
  // as I thought. I went with alternatives for most of the other uses.
  int NextHorizontalSplit(int left, int right, int y, bool top_to_bottom);

  // Indicates that a table row is weak. This means that it has
  // many missing data cells or very large cell heights compared.
  // to the rest of the table.
  static bool IsWeakTableRow(StructuredTable* table, int row);

  // Input data, used as read only data to make decisions.
  ColPartitionGrid* text_grid_;    // Text ColPartitions
  ColPartitionGrid* line_grid_;    // Line ColPartitions
  // Table constraints, a "good" table must satisfy these.
  int min_height_;
  int min_width_;
  // Filters, used to prevent awkward partitions from destroying structure.
  int max_text_height_;  // Horizontal lines may intersect taller text.
};

}  // namespace tesseract

#endif  /* TABLERECOG_H_ */
