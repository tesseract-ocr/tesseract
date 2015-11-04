///////////////////////////////////////////////////////////////////////
// File:        tablefind.cpp
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

#ifdef _MSC_VER
#pragma warning(disable:4244)  // Conversion warnings
#endif

#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif

#include "tablefind.h"
#include <math.h>

#include "allheaders.h"

#include "colpartitionset.h"
#include "tablerecog.h"

namespace tesseract {

// These numbers are used to calculate the global median stats.
// They just set an upper bound on the stats objects.
// Maximum vertical spacing between neighbor partitions.
const int kMaxVerticalSpacing = 500;
// Maximum width of a blob in a partition.
const int kMaxBlobWidth = 500;

// Minimum whitespace size to split a partition (measured as a multiple
// of a partition's median width).
const double kSplitPartitionSize = 2.0;
// To insert text, the partition must satisfy these size constraints
// in AllowTextPartition(). The idea is to filter noise partitions
// determined by the size compared to the global medians.
// TODO(nbeato): Need to find good numbers again.
const double kAllowTextHeight = 0.5;
const double kAllowTextWidth = 0.6;
const double kAllowTextArea = 0.8;
// The same thing applies to blobs (to filter noise).
// TODO(nbeato): These numbers are a shot in the dark...
// height and width are 0.5 * gridsize() in colfind.cpp
// area is a rough guess for the size of a period.
const double kAllowBlobHeight = 0.3;
const double kAllowBlobWidth = 0.4;
const double kAllowBlobArea = 0.05;

// Minimum number of components in a text partition. A partition having fewer
// components than that is more likely a data partition and is a candidate
// table cell.
const int kMinBoxesInTextPartition = 10;

// Maximum number of components that a data partition can have
const int kMaxBoxesInDataPartition = 20;

// Maximum allowed gap in a text partitions as a multiple of its median size.
const double kMaxGapInTextPartition = 4.0;

// Minimum value that the maximum gap in a text partition should have as a
// factor of its median size.
const double kMinMaxGapInTextPartition = 0.5;

// The amount of overlap that is "normal" for adjacent blobs in a text
// partition. This is used to calculate gap between overlapping blobs.
const double kMaxBlobOverlapFactor = 4.0;

// Maximum x-height a table partition can have as a multiple of global
// median x-height
const double kMaxTableCellXheight = 2.0;

// Maximum line spacing between a table column header and column contents
// for merging the two (as a multiple of the partition's median_size).
const int kMaxColumnHeaderDistance = 4;

// Minimum ratio of num_table_partitions to num_text_partitions in a column
// block to be called it a table column
const double kTableColumnThreshold = 3.0;

// Search for horizontal ruling lines within the vertical margin as a
// multiple of grid size
const int kRulingVerticalMargin = 3;

// Minimum overlap that a colpartition must have with a table region
// to become part of that table
const double kMinOverlapWithTable = 0.6;

// Maximum side space (distance from column boundary) that a typical
// text-line in flowing text should have as a multiple of its x-height
// (Median size).
const int kSideSpaceMargin = 10;

// Fraction of the peak of x-projection of a table region to set the
// threshold for the x-projection histogram
const double kSmallTableProjectionThreshold = 0.35;
const double kLargeTableProjectionThreshold = 0.45;
// Minimum number of rows required to look for more rows in the projection.
const int kLargeTableRowCount = 6;

// Minimum number of rows in a table
const int kMinRowsInTable = 3;

// The number of "whitespace blobs" that should appear between the
// ColPartition's bounding box and the column tab stops to the left/right
// when looking for center justified tab stops.
const double kRequiredFullJustifiedSpacing = 4.0;

// The amount of padding (multiplied by global_median_xheight_ during use)
// that is vertically added to the search adjacent leader search during
// ColPartition marking.
const int kAdjacentLeaderSearchPadding = 2;

// Used when filtering false positives. When finding the last line
// of a paragraph (typically left-aligned), the previous line should have
// its center to the right of the last line by this scaled amount.
const double kParagraphEndingPreviousLineRatio = 1.3;

// The maximum amount of whitespace allowed left of a paragraph ending.
// Do not filter a ColPartition with more than this space left of it.
const double kMaxParagraphEndingLeftSpaceMultiple = 3.0;

// Used when filtering false positives. The last line of a paragraph
// should be preceded by a line that is predominantly text. This is the
// ratio of text to whitespace (to the right of the text) that is required
// for the previous line to be a text.
const double kMinParagraphEndingTextToWhitespaceRatio = 3.0;

// When counting table columns, this is the required gap between two columns
// (it is multiplied by global_median_xheight_).
const double kMaxXProjectionGapFactor = 2.0;

// Used for similarity in partitions using stroke width. Values copied
// from ColFind.cpp in Ray's CL.
const double kStrokeWidthFractionalTolerance = 0.25;
const double kStrokeWidthConstantTolerance = 2.0;

BOOL_VAR(textord_dump_table_images, false, "Paint table detection output");
BOOL_VAR(textord_show_tables, false, "Show table regions");
BOOL_VAR(textord_tablefind_show_mark, false,
         "Debug table marking steps in detail");
BOOL_VAR(textord_tablefind_show_stats, false,
         "Show page stats used in table finding");
BOOL_VAR(textord_tablefind_recognize_tables, false,
         "Enables the table recognizer for table layout and filtering.");

ELISTIZE(ColSegment)
CLISTIZE(ColSegment)

// Templated helper function used to create destructor callbacks for the
// BBGrid::ClearGridData() method.
template <typename T> void DeleteObject(T *object) {
  delete object;
}

TableFinder::TableFinder()
    : resolution_(0),
      global_median_xheight_(0),
      global_median_blob_width_(0),
      global_median_ledding_(0),
      left_to_right_language_(true) {
}

TableFinder::~TableFinder() {
  // ColPartitions and ColSegments created by this class for storage in grids
  // need to be deleted explicitly.
  clean_part_grid_.ClearGridData(&DeleteObject<ColPartition>);
  leader_and_ruling_grid_.ClearGridData(&DeleteObject<ColPartition>);
  fragmented_text_grid_.ClearGridData(&DeleteObject<ColPartition>);
  col_seg_grid_.ClearGridData(&DeleteObject<ColSegment>);
  table_grid_.ClearGridData(&DeleteObject<ColSegment>);
}

void TableFinder::set_left_to_right_language(bool order) {
  left_to_right_language_ = order;
}

void TableFinder::Init(int grid_size, const ICOORD& bottom_left,
                       const ICOORD& top_right) {
  // Initialize clean partitions list and grid
  clean_part_grid_.Init(grid_size, bottom_left, top_right);
  leader_and_ruling_grid_.Init(grid_size, bottom_left, top_right);
  fragmented_text_grid_.Init(grid_size, bottom_left, top_right);
  col_seg_grid_.Init(grid_size, bottom_left, top_right);
  table_grid_.Init(grid_size, bottom_left, top_right);
}

// Copy cleaned partitions from part_grid_ to clean_part_grid_ and
// insert leaders and rulers into the leader_and_ruling_grid_
void TableFinder::InsertCleanPartitions(ColPartitionGrid* grid,
                                        TO_BLOCK* block) {
  // Calculate stats. This lets us filter partitions in AllowTextPartition()
  // and filter blobs in AllowBlob().
  SetGlobalSpacings(grid);

  // Iterate the ColPartitions in the grid.
  ColPartitionGridSearch gsearch(grid);
  gsearch.SetUniqueMode(true);
  gsearch.StartFullSearch();
  ColPartition* part = NULL;
  while ((part = gsearch.NextFullSearch()) != NULL) {
    // Reject partitions with nothing useful inside of them.
    if (part->blob_type() == BRT_NOISE || part->bounding_box().area() <= 0)
      continue;
    ColPartition* clean_part = part->ShallowCopy();
    ColPartition* leader_part = NULL;
    if (part->IsLineType()) {
      InsertRulingPartition(clean_part);
      continue;
    }
    // Insert all non-text partitions to clean_parts
    if (!part->IsTextType()) {
      InsertImagePartition(clean_part);
      continue;
    }
    // Insert text colpartitions after removing noisy components from them
    // The leaders are split into a separate grid.
    BLOBNBOX_CLIST* part_boxes = part->boxes();
    BLOBNBOX_C_IT pit(part_boxes);
    for (pit.mark_cycle_pt(); !pit.cycled_list(); pit.forward()) {
      BLOBNBOX *pblob = pit.data();
      // Bad blobs... happens in UNLV set.
      // news.3G1, page 17 (around x=6)
      if (!AllowBlob(*pblob))
        continue;
      if (pblob->flow() == BTFT_LEADER) {
        if (leader_part == NULL) {
          leader_part = part->ShallowCopy();
          leader_part->set_flow(BTFT_LEADER);
        }
        leader_part->AddBox(pblob);
      } else if (pblob->region_type() != BRT_NOISE) {
        clean_part->AddBox(pblob);
      }
    }
    clean_part->ComputeLimits();
    ColPartition* fragmented = clean_part->CopyButDontOwnBlobs();
    InsertTextPartition(clean_part);
    SplitAndInsertFragmentedTextPartition(fragmented);
    if (leader_part != NULL) {
      // TODO(nbeato): Note that ComputeLimits does not update the column
      // information. So the leader may appear to span more columns than it
      // really does later on when IsInSameColumnAs gets called to test
      // for adjacent leaders.
      leader_part->ComputeLimits();
      InsertLeaderPartition(leader_part);
    }
  }

  // Make the partition partners better for upper and lower neighbors.
  clean_part_grid_.FindPartitionPartners();
  clean_part_grid_.RefinePartitionPartners(false);
}

// High level function to perform table detection
void TableFinder::LocateTables(ColPartitionGrid* grid,
                               ColPartitionSet** all_columns,
                               WidthCallback* width_cb,
                               const FCOORD& reskew) {
  // initialize spacing, neighbors, and columns
  InitializePartitions(all_columns);

#ifndef GRAPHICS_DISABLED
  if (textord_show_tables) {
    ScrollView* table_win = MakeWindow(0, 300, "Column Partitions & Neighbors");
    DisplayColPartitions(table_win, &clean_part_grid_, ScrollView::BLUE);
    DisplayColPartitions(table_win, &leader_and_ruling_grid_,
                         ScrollView::AQUAMARINE);
    DisplayColPartitionConnections(table_win, &clean_part_grid_,
                                   ScrollView::ORANGE);

    table_win = MakeWindow(100, 300, "Fragmented Text");
    DisplayColPartitions(table_win, &fragmented_text_grid_, ScrollView::BLUE);
  }
#endif  // GRAPHICS_DISABLED

  // mark, filter, and smooth candidate table partitions
  MarkTablePartitions();

  // Make single-column blocks from good_columns_ partitions. col_segments are
  // moved to a grid later which takes the ownership
  ColSegment_LIST column_blocks;
  GetColumnBlocks(all_columns, &column_blocks);
  // Set the ratio of candidate table partitions in each column
  SetColumnsType(&column_blocks);

  // Move column segments to col_seg_grid_
  MoveColSegmentsToGrid(&column_blocks, &col_seg_grid_);

  // Detect split in column layout that might have occurred due to the
  // presence of a table. In such a case, merge the corresponding columns.
  GridMergeColumnBlocks();

  // Group horizontally overlapping table partitions into table columns.
  // table_columns created here get deleted at the end of this method.
  ColSegment_LIST table_columns;
  GetTableColumns(&table_columns);

  // Within each column, mark the range table regions occupy based on the
  // table columns detected. table_regions are moved to a grid later which
  // takes the ownership
  ColSegment_LIST table_regions;
  GetTableRegions(&table_columns, &table_regions);

#ifndef GRAPHICS_DISABLED
  if (textord_tablefind_show_mark) {
    ScrollView* table_win = MakeWindow(1200, 300, "Table Columns and Regions");
    DisplayColSegments(table_win, &table_columns, ScrollView::DARK_TURQUOISE);
    DisplayColSegments(table_win, &table_regions, ScrollView::YELLOW);
  }
#endif  // GRAPHICS_DISABLED

  // Merge table regions across columns for tables spanning multiple
  // columns
  MoveColSegmentsToGrid(&table_regions, &table_grid_);
  GridMergeTableRegions();

  // Adjust table boundaries by including nearby horizontal lines and left
  // out column headers
  AdjustTableBoundaries();
  GridMergeTableRegions();

  if (textord_tablefind_recognize_tables) {
    // Remove false alarms consiting of a single column
    DeleteSingleColumnTables();

#ifndef GRAPHICS_DISABLED
    if (textord_show_tables) {
      ScrollView* table_win = MakeWindow(1200, 300, "Detected Table Locations");
      DisplayColPartitions(table_win, &clean_part_grid_, ScrollView::BLUE);
      DisplayColSegments(table_win, &table_columns, ScrollView::KHAKI);
      table_grid_.DisplayBoxes(table_win);
    }
#endif  // GRAPHICS_DISABLED

    // Find table grid structure and reject tables that are malformed.
    RecognizeTables();
    GridMergeTableRegions();
    RecognizeTables();

#ifndef GRAPHICS_DISABLED
    if (textord_show_tables) {
      ScrollView* table_win = MakeWindow(1400, 600, "Recognized Tables");
      DisplayColPartitions(table_win, &clean_part_grid_,
                           ScrollView::BLUE, ScrollView::BLUE);
      table_grid_.DisplayBoxes(table_win);
    }
#endif  // GRAPHICS_DISABLED
  } else {
    // Remove false alarms consiting of a single column
    // TODO(nbeato): verify this is a NOP after structured table rejection.
    // Right now it isn't. If the recognize function is doing what it is
    // supposed to do, this function is obsolete.
    DeleteSingleColumnTables();

#ifndef GRAPHICS_DISABLED
    if (textord_show_tables) {
      ScrollView* table_win = MakeWindow(1500, 300, "Detected Tables");
      DisplayColPartitions(table_win, &clean_part_grid_,
                           ScrollView::BLUE, ScrollView::BLUE);
      table_grid_.DisplayBoxes(table_win);
    }
#endif  // GRAPHICS_DISABLED
  }

  if (textord_dump_table_images)
    WriteToPix(reskew);

  // Merge all colpartitions in table regions to make them a single
  // colpartition and revert types of isolated table cells not
  // assigned to any table to their original types.
  MakeTableBlocks(grid, all_columns, width_cb);
}
// All grids have the same dimensions. The clean_part_grid_ sizes are set from
// the part_grid_ that is passed to InsertCleanPartitions, which was the same as
// the grid that is the base of ColumnFinder. Just return the clean_part_grid_
// dimensions instead of duplicated memory.
int TableFinder::gridsize() const {
  return clean_part_grid_.gridsize();
}
int TableFinder::gridwidth() const {
  return clean_part_grid_.gridwidth();
}
int TableFinder::gridheight() const {
  return clean_part_grid_.gridheight();
}
const ICOORD& TableFinder::bleft() const {
  return clean_part_grid_.bleft();
}
const ICOORD& TableFinder::tright() const {
  return clean_part_grid_.tright();
}

void TableFinder::InsertTextPartition(ColPartition* part) {
  ASSERT_HOST(part != NULL);
  if (AllowTextPartition(*part)) {
    clean_part_grid_.InsertBBox(true, true, part);
  } else {
    delete part;
  }
}
void TableFinder::InsertFragmentedTextPartition(ColPartition* part) {
  ASSERT_HOST(part != NULL);
  if (AllowTextPartition(*part)) {
    fragmented_text_grid_.InsertBBox(true, true, part);
  } else {
    delete part;
  }
}
void TableFinder::InsertLeaderPartition(ColPartition* part) {
  ASSERT_HOST(part != NULL);
  if (!part->IsEmpty() && part->bounding_box().area() > 0) {
    leader_and_ruling_grid_.InsertBBox(true, true, part);
  } else {
    delete part;
  }
}
void TableFinder::InsertRulingPartition(ColPartition* part) {
  leader_and_ruling_grid_.InsertBBox(true, true, part);
}
void TableFinder::InsertImagePartition(ColPartition* part) {
  // NOTE: If images are placed into a different grid in the future,
  // the function SetPartitionSpacings needs to be updated. It should
  // be the only thing that cares about image partitions.
  clean_part_grid_.InsertBBox(true, true, part);
}

// Splits a partition into its "words". The splits happen
// at locations with wide inter-blob spacing. This is useful
// because it allows the table recognize to "cut through" the
// text lines on the page. The assumption is that a table
// will have several lines with similar overlapping whitespace
// whereas text will not have this type of property.
// Note: The code Assumes that blobs are sorted by the left side x!
// This will not work (as well) if the blobs are sorted by center/right.
void TableFinder::SplitAndInsertFragmentedTextPartition(ColPartition* part) {
  ASSERT_HOST(part != NULL);
  // Bye bye empty partitions!
  if (part->boxes()->empty()) {
    delete part;
    return;
  }

  // The AllowBlob function prevents this.
  ASSERT_HOST(part->median_width() > 0);
  const double kThreshold = part->median_width() * kSplitPartitionSize;

  ColPartition* right_part = part;
  bool found_split = true;
  while (found_split) {
    found_split = false;
    BLOBNBOX_C_IT box_it(right_part->boxes());
    // Blobs are sorted left side first. If blobs overlap,
    // the previous blob may have a "more right" right side.
    // Account for this by always keeping the largest "right"
    // so far.
    int previous_right = MIN_INT32;

    // Look for the next split in the partition.
    for (box_it.mark_cycle_pt(); !box_it.cycled_list(); box_it.forward()) {
      const TBOX& box = box_it.data()->bounding_box();
      if (previous_right != MIN_INT32 &&
          box.left() - previous_right > kThreshold) {
        // We have a split position. Split the partition in two pieces.
        // Insert the left piece in the grid and keep processing the right.
        int mid_x = (box.left() + previous_right) / 2;
        ColPartition* left_part = right_part;
        right_part = left_part->SplitAt(mid_x);

        InsertFragmentedTextPartition(left_part);
        found_split = true;
        break;
      }

      // The right side of the previous blobs.
      previous_right = MAX(previous_right, box.right());
    }
  }
  // When a split is not found, the right part is minimized
  // as much as possible, so process it.
  InsertFragmentedTextPartition(right_part);
}

// Some simple criteria to filter out now. We want to make sure the
// average blob size in the partition is consistent with the
// global page stats.
// The area metric will almost always pass for multi-blob partitions.
// It is useful when filtering out noise caused by an isolated blob.
bool TableFinder::AllowTextPartition(const ColPartition& part) const {
  const double kHeightRequired = global_median_xheight_ * kAllowTextHeight;
  const double kWidthRequired = global_median_blob_width_ * kAllowTextWidth;
  const int median_area = global_median_xheight_ * global_median_blob_width_;
  const double kAreaPerBlobRequired = median_area * kAllowTextArea;
  // Keep comparisons strictly greater to disallow 0!
  return part.median_size() > kHeightRequired &&
         part.median_width() > kWidthRequired &&
         part.bounding_box().area() > kAreaPerBlobRequired * part.boxes_count();
}

// Same as above, applied to blobs. Keep in mind that
// leaders, commas, and periods are important in tables.
bool TableFinder::AllowBlob(const BLOBNBOX& blob) const {
  const TBOX& box = blob.bounding_box();
  const double kHeightRequired = global_median_xheight_ * kAllowBlobHeight;
  const double kWidthRequired = global_median_blob_width_ * kAllowBlobWidth;
  const int median_area = global_median_xheight_ * global_median_blob_width_;
  const double kAreaRequired = median_area * kAllowBlobArea;
  // Keep comparisons strictly greater to disallow 0!
  return box.height() > kHeightRequired &&
         box.width() > kWidthRequired &&
         box.area() > kAreaRequired;
}

// TODO(nbeato): The grid that makes the window doesn't seem to matter.
// The only downside is that window messages will be caught by
// clean_part_grid_ instead of a useful object. This is a temporary solution
// for the debug windows created by the TableFinder.
ScrollView* TableFinder::MakeWindow(int x, int y, const char* window_name) {
  return clean_part_grid_.MakeWindow(x, y, window_name);
}

// Make single-column blocks from good_columns_ partitions.
void TableFinder::GetColumnBlocks(ColPartitionSet** all_columns,
                                  ColSegment_LIST* column_blocks) {
  for (int i = 0; i < gridheight(); ++i) {
    ColPartitionSet* columns = all_columns[i];
    if (columns != NULL) {
      ColSegment_LIST new_blocks;
      // Get boxes from the current vertical position on the grid
      columns->GetColumnBoxes(i * gridsize(), (i+1) * gridsize(), &new_blocks);
      // Merge the new_blocks boxes into column_blocks if they are well-aligned
      GroupColumnBlocks(&new_blocks, column_blocks);
    }
  }
}

// Merge column segments into the current list if they are well aligned.
void TableFinder::GroupColumnBlocks(ColSegment_LIST* new_blocks,
                                    ColSegment_LIST* column_blocks) {
  ColSegment_IT src_it(new_blocks);
  ColSegment_IT dest_it(column_blocks);
  // iterate through the source list
  for (src_it.mark_cycle_pt(); !src_it.cycled_list(); src_it.forward()) {
    ColSegment* src_seg = src_it.data();
    TBOX src_box = src_seg->bounding_box();
    bool match_found = false;
    // iterate through the destination list to find a matching column block
    for (dest_it.mark_cycle_pt(); !dest_it.cycled_list(); dest_it.forward()) {
      ColSegment* dest_seg = dest_it.data();
      TBOX dest_box = dest_seg->bounding_box();
      if (ConsecutiveBoxes(src_box, dest_box)) {
        // If matching block is found, insert the current block into it
        // and delete the soure block
        dest_seg->InsertBox(src_box);
        match_found = true;
        delete src_it.extract();
        break;
      }
    }
    // If no match is found, just append the source block to column_blocks
    if (!match_found) {
      dest_it.add_after_then_move(src_it.extract());
    }
  }
}

// are the two boxes immediate neighbors along the vertical direction
bool TableFinder::ConsecutiveBoxes(const TBOX &b1, const TBOX &b2) {
  int x_margin = 20;
  int y_margin = 5;
  return (abs(b1.left() - b2.left()) < x_margin) &&
      (abs(b1.right() - b2.right()) < x_margin) &&
      (abs(b1.top()-b2.bottom()) < y_margin ||
       abs(b2.top()-b1.bottom()) < y_margin);
}

// Set up info for clean_part_grid_ partitions to be valid during detection
// code.
void TableFinder::InitializePartitions(ColPartitionSet** all_columns) {
  FindNeighbors();
  SetPartitionSpacings(&clean_part_grid_, all_columns);
  SetGlobalSpacings(&clean_part_grid_);
}

// Set left, right and top, bottom spacings of each colpartition.
void TableFinder::SetPartitionSpacings(ColPartitionGrid* grid,
                                       ColPartitionSet** all_columns) {
  // Iterate the ColPartitions in the grid.
  ColPartitionGridSearch gsearch(grid);
  gsearch.StartFullSearch();
  ColPartition* part = NULL;
  while ((part = gsearch.NextFullSearch()) != NULL) {
    ColPartitionSet* columns = all_columns[gsearch.GridY()];
    TBOX box = part->bounding_box();
    int y = part->MidY();
    ColPartition* left_column = columns->ColumnContaining(box.left(), y);
    ColPartition* right_column = columns->ColumnContaining(box.right(), y);
    // set distance from left column as space to the left
    if (left_column) {
      int left_space = MAX(0, box.left() - left_column->LeftAtY(y));
      part->set_space_to_left(left_space);
    }
    // set distance from right column as space to the right
    if (right_column) {
      int right_space = MAX(0, right_column->RightAtY(y) - box.right());
      part->set_space_to_right(right_space);
    }

    // Look for images that may be closer.
    // NOTE: used to be part_grid_, might cause issues now
    ColPartitionGridSearch hsearch(grid);
    hsearch.StartSideSearch(box.left(), box.bottom(), box.top());
    ColPartition* neighbor = NULL;
    while ((neighbor = hsearch.NextSideSearch(true)) != NULL) {
      if (neighbor->type() == PT_PULLOUT_IMAGE ||
          neighbor->type() == PT_FLOWING_IMAGE ||
          neighbor->type() == PT_HEADING_IMAGE) {
        int right = neighbor->bounding_box().right();
        if (right < box.left()) {
          int space = MIN(box.left() - right, part->space_to_left());
          part->set_space_to_left(space);
        }
      }
    }
    hsearch.StartSideSearch(box.left(), box.bottom(), box.top());
    neighbor = NULL;
    while ((neighbor = hsearch.NextSideSearch(false)) != NULL) {
      if (neighbor->type() == PT_PULLOUT_IMAGE ||
          neighbor->type() == PT_FLOWING_IMAGE ||
          neighbor->type() == PT_HEADING_IMAGE) {
        int left = neighbor->bounding_box().left();
        if (left > box.right()) {
          int space = MIN(left - box.right(), part->space_to_right());
          part->set_space_to_right(space);
        }
      }
    }

    ColPartition* upper_part = part->SingletonPartner(true);
    if (upper_part) {
      int space = MAX(0, upper_part->bounding_box().bottom() -
                         part->bounding_box().bottom());
      part->set_space_above(space);
    } else {
      // TODO(nbeato): What constitutes a good value?
      // 0 is the default value when not set, explicitly noting it needs to
      // be something else.
      part->set_space_above(MAX_INT32);
    }

    ColPartition* lower_part = part->SingletonPartner(false);
    if (lower_part) {
      int space = MAX(0, part->bounding_box().bottom() -
                         lower_part->bounding_box().bottom());
      part->set_space_below(space);
    } else {
      // TODO(nbeato): What constitutes a good value?
      // 0 is the default value when not set, explicitly noting it needs to
      // be something else.
      part->set_space_below(MAX_INT32);
    }
  }
}

// Set spacing and closest neighbors above and below a given colpartition.
void TableFinder::SetVerticalSpacing(ColPartition* part) {
  TBOX box = part->bounding_box();
  int top_range = MIN(box.top() + kMaxVerticalSpacing, tright().y());
  int bottom_range = MAX(box.bottom() - kMaxVerticalSpacing, bleft().y());
  box.set_top(top_range);
  box.set_bottom(bottom_range);

  TBOX part_box = part->bounding_box();
  // Start a rect search
  GridSearch<ColPartition, ColPartition_CLIST, ColPartition_C_IT>
      rectsearch(&clean_part_grid_);
  rectsearch.StartRectSearch(box);
  ColPartition* neighbor;
  int min_space_above = kMaxVerticalSpacing;
  int min_space_below = kMaxVerticalSpacing;
  ColPartition* above_neighbor = NULL;
  ColPartition* below_neighbor = NULL;
  while ((neighbor = rectsearch.NextRectSearch()) != NULL) {
    if (neighbor == part)
      continue;
    TBOX neighbor_box = neighbor->bounding_box();
    if (neighbor_box.major_x_overlap(part_box)) {
      int gap = abs(part->median_bottom() - neighbor->median_bottom());
      // If neighbor is below current partition
      if (neighbor_box.top() < part_box.bottom() &&
          gap < min_space_below) {
        min_space_below = gap;
        below_neighbor = neighbor;
      }  // If neighbor is above current partition
      else if (part_box.top() < neighbor_box.bottom() &&
               gap < min_space_above) {
        min_space_above = gap;
        above_neighbor = neighbor;
       }
    }
  }
  part->set_space_above(min_space_above);
  part->set_space_below(min_space_below);
  part->set_nearest_neighbor_above(above_neighbor);
  part->set_nearest_neighbor_below(below_neighbor);
}

// Set global spacing and x-height estimates
void TableFinder::SetGlobalSpacings(ColPartitionGrid* grid) {
  STATS xheight_stats(0, kMaxVerticalSpacing + 1);
  STATS width_stats(0, kMaxBlobWidth + 1);
  STATS ledding_stats(0, kMaxVerticalSpacing + 1);
  // Iterate the ColPartitions in the grid.
  ColPartitionGridSearch gsearch(grid);
  gsearch.SetUniqueMode(true);
  gsearch.StartFullSearch();
  ColPartition* part = NULL;
  while ((part = gsearch.NextFullSearch()) != NULL) {
    // TODO(nbeato): HACK HACK HACK! medians are equal to partition length.
    // ComputeLimits needs to get called somewhere outside of TableFinder
    // to make sure the partitions are properly initialized.
    // When this is called, SmoothPartitionPartners dies in an assert after
    // table find runs. Alternative solution.
    // part->ComputeLimits();
    if (part->IsTextType()) {
      // xheight_stats.add(part->median_size(), part->boxes_count());
      // width_stats.add(part->median_width(), part->boxes_count());

      // This loop can be removed when above issues are fixed.
      // Replace it with the 2 lines commented out above.
      BLOBNBOX_C_IT it(part->boxes());
      for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
        xheight_stats.add(it.data()->bounding_box().height(), 1);
        width_stats.add(it.data()->bounding_box().width(), 1);
      }

      ledding_stats.add(part->space_above(), 1);
      ledding_stats.add(part->space_below(), 1);
    }
  }
  // Set estimates based on median of statistics obtained
  set_global_median_xheight(static_cast<int>(xheight_stats.median() + 0.5));
  set_global_median_blob_width(static_cast<int>(width_stats.median() + 0.5));
  set_global_median_ledding(static_cast<int>(ledding_stats.median() + 0.5));
  #ifndef GRAPHICS_DISABLED
  if (textord_tablefind_show_stats) {
    const char* kWindowName = "X-height (R), X-width (G), and ledding (B)";
    ScrollView* stats_win = MakeWindow(500, 10, kWindowName);
    xheight_stats.plot(stats_win, 10, 200, 2, 15, ScrollView::RED);
    width_stats.plot(stats_win, 10, 200, 2, 15, ScrollView::GREEN);
    ledding_stats.plot(stats_win, 10, 200, 2, 15, ScrollView::BLUE);
  }
  #endif  // GRAPHICS_DISABLED
}

void TableFinder::set_global_median_xheight(int xheight) {
  global_median_xheight_ = xheight;
}
void TableFinder::set_global_median_blob_width(int width) {
  global_median_blob_width_ = width;
}
void TableFinder::set_global_median_ledding(int ledding) {
  global_median_ledding_ = ledding;
}

void TableFinder::FindNeighbors() {
  ColPartitionGridSearch gsearch(&clean_part_grid_);
  gsearch.StartFullSearch();
  ColPartition* part = NULL;
  while ((part = gsearch.NextFullSearch()) != NULL) {
    // TODO(nbeato): Rename this function, meaning is different now.
    // IT is finding nearest neighbors its own way
    //SetVerticalSpacing(part);

    ColPartition* upper = part->SingletonPartner(true);
    if (upper)
      part->set_nearest_neighbor_above(upper);

    ColPartition* lower = part->SingletonPartner(false);
    if (lower)
      part->set_nearest_neighbor_below(lower);
  }
}

// High level interface. Input is an unmarked ColPartitionGrid
// (namely, clean_part_grid_). Partitions are identified using local
// information and filter/smoothed. The function exit should contain
// a good sampling of the table partitions.
void TableFinder::MarkTablePartitions() {
  MarkPartitionsUsingLocalInformation();
  if (textord_tablefind_show_mark) {
    ScrollView* table_win = MakeWindow(300, 300, "Initial Table Partitions");
    DisplayColPartitions(table_win, &clean_part_grid_, ScrollView::BLUE);
    DisplayColPartitions(table_win, &leader_and_ruling_grid_,
                         ScrollView::AQUAMARINE);
  }
  FilterFalseAlarms();
  if (textord_tablefind_show_mark) {
    ScrollView* table_win = MakeWindow(600, 300, "Filtered Table Partitions");
    DisplayColPartitions(table_win, &clean_part_grid_, ScrollView::BLUE);
    DisplayColPartitions(table_win, &leader_and_ruling_grid_,
                         ScrollView::AQUAMARINE);
  }
  SmoothTablePartitionRuns();
  if (textord_tablefind_show_mark) {
    ScrollView* table_win = MakeWindow(900, 300, "Smoothed Table Partitions");
    DisplayColPartitions(table_win, &clean_part_grid_, ScrollView::BLUE);
    DisplayColPartitions(table_win, &leader_and_ruling_grid_,
                         ScrollView::AQUAMARINE);
  }
  FilterFalseAlarms();
  if (textord_tablefind_show_mark || textord_show_tables) {
    ScrollView* table_win = MakeWindow(900, 300, "Final Table Partitions");
    DisplayColPartitions(table_win, &clean_part_grid_, ScrollView::BLUE);
    DisplayColPartitions(table_win, &leader_and_ruling_grid_,
                         ScrollView::AQUAMARINE);
  }
}

// These types of partitions are marked as table partitions:
//  1- Partitions that have at lease one large gap between words
//  2- Partitions that consist of only one word (no significant gap
//     between components)
//  3- Partitions that vertically overlap with other partitions within the
//     same column.
//  4- Partitions with leaders before/after them.
void TableFinder::MarkPartitionsUsingLocalInformation() {
  // Iterate the ColPartitions in the grid.
  GridSearch<ColPartition, ColPartition_CLIST, ColPartition_C_IT>
    gsearch(&clean_part_grid_);
  gsearch.StartFullSearch();
  ColPartition* part = NULL;
  while ((part = gsearch.NextFullSearch()) != NULL) {
    if (!part->IsTextType())  // Only consider text partitions
      continue;
    // Only consider partitions in dominant font size or smaller
    if (part->median_size() > kMaxTableCellXheight * global_median_xheight_)
      continue;
    // Mark partitions with a large gap, or no significant gap as
    // table partitions.
    // Comments: It produces several false alarms at:
    //  - last line of a paragraph (fixed)
    //  - single word section headings
    //  - page headers and footers
    //  - numbered equations
    //  - line drawing regions
    // TODO(faisal): detect and fix above-mentioned cases
    if (HasWideOrNoInterWordGap(part) ||
        HasLeaderAdjacent(*part)) {
      part->set_table_type();
    }
  }
}

// Check if the partition has at least one large gap between words or no
// significant gap at all
bool TableFinder::HasWideOrNoInterWordGap(ColPartition* part) const {
  // Should only get text partitions.
  ASSERT_HOST(part->IsTextType());
  // Blob access
  BLOBNBOX_CLIST* part_boxes = part->boxes();
  BLOBNBOX_C_IT it(part_boxes);
  // Check if this is a relatively small partition (such as a single word)
  if (part->bounding_box().width() <
      kMinBoxesInTextPartition * part->median_size() &&
      part_boxes->length() < kMinBoxesInTextPartition)
    return true;

  // Variables used to compute inter-blob spacing.
  int current_x0 = -1;
  int current_x1 = -1;
  int previous_x1 = -1;
  // Stores the maximum gap detected.
  int largest_partition_gap_found = -1;
  // Text partition gap limits. If this is text (and not a table),
  // there should be at least one gap larger than min_gap and no gap
  // larger than max_gap.
  const double max_gap = kMaxGapInTextPartition * part->median_size();
  const double min_gap = kMinMaxGapInTextPartition * part->median_size();

  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    BLOBNBOX* blob = it.data();
    current_x0 = blob->bounding_box().left();
    current_x1 = blob->bounding_box().right();
    if (previous_x1 != -1) {
      int gap = current_x0 - previous_x1;

      // TODO(nbeato): Boxes may overlap? Huh?
      // For example, mag.3B 8003_033.3B.tif in UNLV data. The titles/authors
      // on the top right of the page are filtered out with this line.
      // Note 2: Iterating over blobs in a partition, so we are looking for
      // spacing between the words.
      if (gap < 0) {
        // More likely case, the blobs slightly overlap. This can happen
        // with diacritics (accents) or broken alphabet symbols (characters).
        // Merge boxes together by taking max of right sides.
        if (-gap < part->median_size() * kMaxBlobOverlapFactor) {
          previous_x1 = MAX(previous_x1, current_x1);
          continue;
        }
        // Extreme case, blobs overlap significantly in the same partition...
        // This should not happen often (if at all), but it does.
        // TODO(nbeato): investigate cases when this happens.
        else {
          // The behavior before was to completely ignore this case.
        }
      }

      // If a large enough gap is found, mark it as a table cell (return true)
      if (gap > max_gap)
        return true;
      if (gap > largest_partition_gap_found)
        largest_partition_gap_found = gap;
    }
    previous_x1 = current_x1;
  }
  // Since no large gap was found, return false if the partition is too
  // long to be a data cell
  if (part->bounding_box().width() >
      kMaxBoxesInDataPartition * part->median_size() ||
      part_boxes->length() > kMaxBoxesInDataPartition)
    return false;

  // A partition may be a single blob. In this case, it's an isolated symbol
  // or non-text (such as a ruling or image).
  // Detect these as table partitions? Shouldn't this be case by case?
  // The behavior before was to ignore this, making max_partition_gap < 0
  // and implicitly return true. Just making it explicit.
  if (largest_partition_gap_found == -1)
    return true;

  // return true if the maximum gap found is smaller than the minimum allowed
  // max_gap in a text partition. This indicates that there is no significant
  // space in the partition, hence it is likely a single word.
  return largest_partition_gap_found < min_gap;
}

// A criteria for possible tables is that a table may have leaders
// between data cells. An aggressive solution to find such tables is to
// explicitly mark partitions that have adjacent leaders.
// Note that this includes overlapping leaders. However, it does not
// include leaders in different columns on the page.
// Possible false-positive will include lists, such as a table of contents.
// As these arise, the aggressive nature of this search may need to be
// trimmed down.
bool TableFinder::HasLeaderAdjacent(const ColPartition& part) {
  if (part.flow() == BTFT_LEADER)
    return true;
  // Search range is left and right bounded by an offset of the
  // median xheight. This offset is to allow some tolerance to the
  // the leaders on the page in the event that the alignment is still
  // a bit off.
  const TBOX& box = part.bounding_box();
  const int search_size = kAdjacentLeaderSearchPadding * global_median_xheight_;
  const int top = box.top() + search_size;
  const int bottom = box.bottom() - search_size;
  ColPartitionGridSearch hsearch(&leader_and_ruling_grid_);
  for (int direction = 0; direction < 2; ++direction) {
    bool right_to_left = (direction == 0);
    int x = right_to_left ? box.right() : box.left();
    hsearch.StartSideSearch(x, bottom, top);
    ColPartition* leader = NULL;
    while ((leader = hsearch.NextSideSearch(right_to_left)) != NULL) {
      // The leader could be a horizontal ruling in the grid.
      // Make sure it is actually a leader.
      if (leader->flow() != BTFT_LEADER)
        continue;
      // This should not happen, they are in different grids.
      ASSERT_HOST(&part != leader);
      // Make sure the leader shares a page column with the partition,
      // otherwise we are spreading across columns.
      if (!part.IsInSameColumnAs(*leader))
        break;
      // There should be a significant vertical overlap
      if (!leader->VSignificantCoreOverlap(part))
        continue;
      // Leader passed all tests, so it is adjacent.
      return true;
    }
  }
  // No leaders are adjacent to the given partition.
  return false;
}

// Filter individual text partitions marked as table partitions
// consisting of paragraph endings, small section headings, and
// headers and footers.
void TableFinder::FilterFalseAlarms() {
  FilterParagraphEndings();
  FilterHeaderAndFooter();
  // TODO(nbeato): Fully justified text as non-table?
}

void TableFinder::FilterParagraphEndings() {
  // Detect last line of paragraph
  // Iterate the ColPartitions in the grid.
  ColPartitionGridSearch gsearch(&clean_part_grid_);
  gsearch.StartFullSearch();
  ColPartition* part = NULL;
  while ((part = gsearch.NextFullSearch()) != NULL) {
    if (part->type() != PT_TABLE)
      continue;  // Consider only table partitions

    // Paragraph ending should have flowing text above it.
    ColPartition* upper_part = part->nearest_neighbor_above();
    if (!upper_part)
      continue;
    if (upper_part->type() != PT_FLOWING_TEXT)
      continue;
    if (upper_part->bounding_box().width() <
        2 * part->bounding_box().width())
      continue;
    // Check if its the last line of a paragraph.
    // In most cases, a paragraph ending should be left-aligned to text line
    // above it. Sometimes, it could be a 2 line paragraph, in which case
    // the line above it is indented.
    // To account for that, check if the partition center is to
    // the left of the one above it.
    int mid = (part->bounding_box().left() + part->bounding_box().right()) / 2;
    int upper_mid = (upper_part->bounding_box().left() +
                     upper_part->bounding_box().right()) / 2;
    int current_spacing = 0;  // spacing of the current line to margin
    int upper_spacing = 0;    // spacing of the previous line to the margin
    if (left_to_right_language_) {
      // Left to right languages, use mid - left to figure out the distance
      // the middle is from the left margin.
      int left = MIN(part->bounding_box().left(),
                     upper_part->bounding_box().left());
      current_spacing = mid - left;
      upper_spacing = upper_mid - left;
    } else {
      // Right to left languages, use right - mid to figure out the distance
      // the middle is from the right margin.
      int right = MAX(part->bounding_box().right(),
                      upper_part->bounding_box().right());
      current_spacing = right - mid;
      upper_spacing = right - upper_mid;
    }
    if (current_spacing * kParagraphEndingPreviousLineRatio > upper_spacing)
      continue;

    // Paragraphs should have similar fonts.
    if (!part->MatchingSizes(*upper_part) ||
        !part->MatchingStrokeWidth(*upper_part, kStrokeWidthFractionalTolerance,
                                   kStrokeWidthConstantTolerance)) {
      continue;
    }

    // The last line of a paragraph should be left aligned.
    // TODO(nbeato): This would be untrue if the text was right aligned.
    // How often is that?
    if (part->space_to_left() >
        kMaxParagraphEndingLeftSpaceMultiple * part->median_size())
      continue;
    // The line above it should be right aligned (assuming justified format).
    // Since we can't assume justified text, we compare whitespace to text.
    // The above line should have majority spanning text (or the current
    // line could have fit on the previous line). So compare
    // whitespace to text.
    if (upper_part->bounding_box().width() <
        kMinParagraphEndingTextToWhitespaceRatio * upper_part->space_to_right())
      continue;

    // Ledding above the line should be less than ledding below
    if (part->space_above() >= part->space_below() ||
        part->space_above() > 2 * global_median_ledding_)
      continue;

    // If all checks failed, it is probably text.
    part->clear_table_type();
  }
}

void TableFinder::FilterHeaderAndFooter() {
  // Consider top-most text colpartition as header and bottom most as footer
  ColPartition* header = NULL;
  ColPartition* footer = NULL;
  int max_top = MIN_INT32;
  int min_bottom = MAX_INT32;
  ColPartitionGridSearch gsearch(&clean_part_grid_);
  gsearch.StartFullSearch();
  ColPartition* part = NULL;
  while ((part = gsearch.NextFullSearch()) != NULL) {
    if (!part->IsTextType())
      continue;  // Consider only text partitions
    int top = part->bounding_box().top();
    int bottom = part->bounding_box().bottom();
    if (top > max_top) {
      max_top = top;
      header = part;
    }
    if (bottom < min_bottom) {
      min_bottom = bottom;
      footer = part;
    }
  }
  if (header)
    header->clear_table_type();
  if (footer)
    footer->clear_table_type();
}

// Mark all ColPartitions as table cells that have a table cell above
// and below them
// TODO(faisal): This is too aggressive at the moment. The method needs to
// consider spacing and alignment as well. Detection of false alarm table cells
// should also be done as part of it.
void TableFinder::SmoothTablePartitionRuns() {
  // Iterate the ColPartitions in the grid.
  ColPartitionGridSearch gsearch(&clean_part_grid_);
  gsearch.StartFullSearch();
  ColPartition* part = NULL;
  while ((part = gsearch.NextFullSearch()) != NULL) {
    if (part->type() >= PT_TABLE || part->type() == PT_UNKNOWN)
      continue;  // Consider only text partitions
    ColPartition* upper_part = part->nearest_neighbor_above();
    ColPartition* lower_part = part->nearest_neighbor_below();
    if (!upper_part || !lower_part)
      continue;
    if (upper_part->type() == PT_TABLE && lower_part->type() == PT_TABLE)
      part->set_table_type();
  }

  // Pass 2, do the opposite. If both the upper and lower neighbors
  // exist and are not tables, this probably shouldn't be a table.
  gsearch.StartFullSearch();
  part = NULL;
  while ((part = gsearch.NextFullSearch()) != NULL) {
    if (part->type() != PT_TABLE)
      continue;  // Consider only text partitions
    ColPartition* upper_part = part->nearest_neighbor_above();
    ColPartition* lower_part = part->nearest_neighbor_below();

    // table can't be by itself
    if ((upper_part && upper_part->type() != PT_TABLE) &&
        (lower_part && lower_part->type() != PT_TABLE)) {
      part->clear_table_type();
    }
  }
}

// Set the type of a column segment based on the ratio of table to text cells
void TableFinder::SetColumnsType(ColSegment_LIST* column_blocks) {
  ColSegment_IT it(column_blocks);
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    ColSegment* seg = it.data();
    TBOX box = seg->bounding_box();
    int num_table_cells = 0;
    int num_text_cells = 0;
    GridSearch<ColPartition, ColPartition_CLIST, ColPartition_C_IT>
        rsearch(&clean_part_grid_);
    rsearch.SetUniqueMode(true);
    rsearch.StartRectSearch(box);
    ColPartition* part = NULL;
    while ((part = rsearch.NextRectSearch()) != NULL) {
      if (part->type() == PT_TABLE) {
        num_table_cells++;
      } else if (part->type() == PT_FLOWING_TEXT) {
        num_text_cells++;
      }
    }
    // If a column block has no text or table partition in it, it is not needed
    // for table detection.
    if (!num_table_cells && !num_text_cells) {
      delete it.extract();
    } else {
      seg->set_num_table_cells(num_table_cells);
      seg->set_num_text_cells(num_text_cells);
      // set column type based on the ratio of table to text cells
      seg->set_type();
    }
  }
}

// Move column blocks to grid
void TableFinder::MoveColSegmentsToGrid(ColSegment_LIST *segments,
                                         ColSegmentGrid *col_seg_grid) {
  ColSegment_IT it(segments);
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    ColSegment* seg = it.extract();
    col_seg_grid->InsertBBox(true, true, seg);
  }
}

// Merge column blocks if a split is detected due to the presence of a
// table. A text block is considered split if it has multiple
// neighboring blocks above/below it, and at least one of the
// neighboring blocks is of table type (has a high density of table
// partitions). In this case neighboring blocks in the direction
// (above/below) of the table block are merged with the text block.

// Comment: This method does not handle split due to a full page table
// since table columns in this case do not have a text column on which
// split decision can be based.
void TableFinder::GridMergeColumnBlocks() {
  int margin = gridsize();

  // Iterate the Column Blocks in the grid.
  GridSearch<ColSegment, ColSegment_CLIST, ColSegment_C_IT>
    gsearch(&col_seg_grid_);
  gsearch.StartFullSearch();
  ColSegment* seg;
  while ((seg = gsearch.NextFullSearch()) != NULL) {
    if (seg->type() != COL_TEXT)
      continue;  // only consider text blocks for split detection
    bool neighbor_found = false;
    bool modified = false;  // Modified at least once
    // keep expanding current box as long as neighboring table columns
    // are found above or below it.
    do {
      TBOX box = seg->bounding_box();
      // slightly expand the search region vertically
      int top_range = MIN(box.top() + margin, tright().y());
      int bottom_range = MAX(box.bottom() - margin, bleft().y());
      box.set_top(top_range);
      box.set_bottom(bottom_range);
      neighbor_found = false;
      GridSearch<ColSegment, ColSegment_CLIST, ColSegment_C_IT>
          rectsearch(&col_seg_grid_);
      rectsearch.StartRectSearch(box);
      ColSegment* neighbor = NULL;
      while ((neighbor = rectsearch.NextRectSearch()) != NULL) {
        if (neighbor == seg)
          continue;
        const TBOX& neighbor_box = neighbor->bounding_box();
        // If the neighbor box significantly overlaps with the current
        // box (due to the expansion of the current box in the
        // previous iteration of this loop), remove the neighbor box
        // and expand the current box to include it.
        if (neighbor_box.overlap_fraction(box) >= 0.9) {
          seg->InsertBox(neighbor_box);
          modified = true;
          rectsearch.RemoveBBox();
          gsearch.RepositionIterator();
          delete neighbor;
          continue;
        }
        // Only expand if the neighbor box is of table type
        if (neighbor->type() != COL_TABLE)
          continue;
        // Insert the neighbor box into the current column block
        if (neighbor_box.major_x_overlap(box) &&
            !box.contains(neighbor_box)) {
          seg->InsertBox(neighbor_box);
          neighbor_found = true;
          modified = true;
          rectsearch.RemoveBBox();
          gsearch.RepositionIterator();
          delete neighbor;
        }
      }
    } while (neighbor_found);
    if (modified) {
      // Because the box has changed, it has to be removed first.
      gsearch.RemoveBBox();
      col_seg_grid_.InsertBBox(true, true, seg);
      gsearch.RepositionIterator();
    }
  }
}

// Group horizontally overlapping table partitions into table columns.
// TODO(faisal): This is too aggressive at the moment. The method should
// consider more attributes to group table partitions together. Some common
// errors are:
//  1- page number is merged with a table column above it even
//      if there is a large vertical gap between them.
//  2- column headers go on to catch one of the columns arbitrarily
//  3- an isolated noise blob near page top or bottom merges with the table
//     column below/above it
//  4- cells from two vertically adjacent tables merge together to make a
//     single column resulting in merging of the two tables
void TableFinder::GetTableColumns(ColSegment_LIST *table_columns) {
  ColSegment_IT it(table_columns);
  // Iterate the ColPartitions in the grid.
  GridSearch<ColPartition, ColPartition_CLIST, ColPartition_C_IT>
    gsearch(&clean_part_grid_);
  gsearch.StartFullSearch();
  ColPartition* part;
  while ((part = gsearch.NextFullSearch()) != NULL) {
    if (part->inside_table_column() || part->type() != PT_TABLE)
      continue;  // prevent a partition to be assigned to multiple columns
    const TBOX& box = part->bounding_box();
    ColSegment* col = new ColSegment();
    col->InsertBox(box);
    part->set_inside_table_column(true);
    // Start a search below the current cell to find bottom neighbours
    // Note: a full search will always process things above it first, so
    // this should be starting at the highest cell and working its way down.
    GridSearch<ColPartition, ColPartition_CLIST, ColPartition_C_IT>
        vsearch(&clean_part_grid_);
    vsearch.StartVerticalSearch(box.left(), box.right(), box.bottom());
    ColPartition* neighbor = NULL;
    bool found_neighbours = false;
    while ((neighbor = vsearch.NextVerticalSearch(true)) != NULL) {
      // only consider neighbors not assigned to any column yet
      if (neighbor->inside_table_column())
        continue;
      // Horizontal lines should not break the flow
      if (neighbor->IsHorizontalLine())
        continue;
      // presence of a non-table neighbor marks the end of current
      // table column
      if (neighbor->type() != PT_TABLE)
        break;
      // add the neighbor partition to the table column
      const TBOX& neighbor_box = neighbor->bounding_box();
      col->InsertBox(neighbor_box);
      neighbor->set_inside_table_column(true);
      found_neighbours = true;
    }
    if (found_neighbours) {
      it.add_after_then_move(col);
    } else {
      part->set_inside_table_column(false);
      delete col;
    }
  }
}

// Mark regions in a column that are x-bounded by the column boundaries and
// y-bounded by the table columns' projection on the y-axis as table regions
void TableFinder::GetTableRegions(ColSegment_LIST* table_columns,
                                  ColSegment_LIST* table_regions) {
  ColSegment_IT cit(table_columns);
  ColSegment_IT rit(table_regions);
  // Iterate through column blocks
  GridSearch<ColSegment, ColSegment_CLIST, ColSegment_C_IT>
      gsearch(&col_seg_grid_);
  gsearch.StartFullSearch();
  ColSegment* part;
  int page_height = tright().y() - bleft().y();
  ASSERT_HOST(page_height > 0);
  // create a bool array to hold projection on y-axis
  bool* table_region = new bool[page_height];
  while ((part = gsearch.NextFullSearch()) != NULL) {
    TBOX part_box = part->bounding_box();
    // reset the projection array
    for (int i = 0; i < page_height; i++) {
      table_region[i] = false;
    }
    // iterate through all table columns to find regions in the current
    // page column block
    cit.move_to_first();
    for (cit.mark_cycle_pt(); !cit.cycled_list(); cit.forward()) {
      TBOX col_box = cit.data()->bounding_box();
      // find intersection region of table column and page column
      TBOX intersection_box = col_box.intersection(part_box);
      // project table column on the y-axis
      for (int i = intersection_box.bottom(); i < intersection_box.top(); i++) {
        table_region[i - bleft().y()] = true;
      }
    }
    // set x-limits of table regions to page column width
    TBOX current_table_box;
    current_table_box.set_left(part_box.left());
    current_table_box.set_right(part_box.right());
    // go through the y-axis projection to find runs of table
    // regions. Each run makes one table region.
    for (int i = 1; i < page_height; i++) {
      // detect start of a table region
      if (!table_region[i - 1] && table_region[i]) {
        current_table_box.set_bottom(i + bleft().y());
      }
      // TODO(nbeato): Is it guaranteed that the last row is not a table region?
      // detect end of a table region
      if (table_region[i - 1] && !table_region[i]) {
        current_table_box.set_top(i + bleft().y());
        if (!current_table_box.null_box()) {
          ColSegment* seg = new ColSegment();
          seg->InsertBox(current_table_box);
          rit.add_after_then_move(seg);
        }
      }
    }
  }
  delete[] table_region;
}

// Merge table regions corresponding to tables spanning multiple columns if
// there is a colpartition (horizontal ruling line or normal text) that
// touches both regions.
// TODO(faisal): A rare error occurs if there are two horizontally adjacent
// tables with aligned ruling lines. In this case, line finder returns a
// single line and hence the tables get merged together
void TableFinder::GridMergeTableRegions() {
  // Iterate the table regions in the grid.
  GridSearch<ColSegment, ColSegment_CLIST, ColSegment_C_IT>
      gsearch(&table_grid_);
  gsearch.StartFullSearch();
  ColSegment* seg = NULL;
  while ((seg = gsearch.NextFullSearch()) != NULL) {
    bool neighbor_found = false;
    bool modified = false;  // Modified at least once
    do {
      // Start a rectangle search x-bounded by the image and y by the table
      const TBOX& box = seg->bounding_box();
      TBOX search_region(box);
      search_region.set_left(bleft().x());
      search_region.set_right(tright().x());
      neighbor_found = false;
      GridSearch<ColSegment, ColSegment_CLIST, ColSegment_C_IT>
          rectsearch(&table_grid_);
      rectsearch.StartRectSearch(search_region);
      ColSegment* neighbor = NULL;
      while ((neighbor = rectsearch.NextRectSearch()) != NULL) {
        if (neighbor == seg)
          continue;
        const TBOX& neighbor_box = neighbor->bounding_box();
        // Check if a neighbor box has a large overlap with the table
        // region.  This may happen as a result of merging two table
        // regions in the previous iteration.
        if (neighbor_box.overlap_fraction(box) >= 0.9) {
          seg->InsertBox(neighbor_box);
          rectsearch.RemoveBBox();
          gsearch.RepositionIterator();
          delete neighbor;
          modified = true;
          continue;
        }
        // Check if two table regions belong together based on a common
        // horizontal ruling line
        if (BelongToOneTable(box, neighbor_box)) {
          seg->InsertBox(neighbor_box);
          neighbor_found = true;
          modified = true;
          rectsearch.RemoveBBox();
          gsearch.RepositionIterator();
          delete neighbor;
        }
      }
    } while (neighbor_found);
    if (modified) {
      // Because the box has changed, it has to be removed first.
      gsearch.RemoveBBox();
      table_grid_.InsertBBox(true, true, seg);
      gsearch.RepositionIterator();
    }
  }
}

// Decide if two table regions belong to one table based on a common
// horizontal ruling line or another colpartition
bool TableFinder::BelongToOneTable(const TBOX &box1, const TBOX &box2) {
  // Check the obvious case. Most likely not true because overlapping boxes
  // should already be merged, but seems like a good thing to do in case things
  // change.
  if (box1.overlap(box2))
    return true;
  // Check for ColPartitions spanning both table regions
  TBOX bbox = box1.bounding_union(box2);
  // Start a rect search on bbox
  GridSearch<ColPartition, ColPartition_CLIST, ColPartition_C_IT>
      rectsearch(&clean_part_grid_);
  rectsearch.StartRectSearch(bbox);
  ColPartition* part = NULL;
  while ((part = rectsearch.NextRectSearch()) != NULL) {
    const TBOX& part_box = part->bounding_box();
    // return true if a colpartition spanning both table regions is found
    if (part_box.overlap(box1) && part_box.overlap(box2) &&
        !part->IsImageType())
      return true;
  }
  return false;
}

// Adjust table boundaries by:
//  - building a tight bounding box around all ColPartitions contained in it.
//  - expanding table boundaries to include all colpartitions that overlap the
//    table by more than half of their area
//  - expanding table boundaries to include nearby horizontal rule lines
//  - expanding table vertically to include left out column headers
// TODO(faisal): Expansion of table boundaries is quite aggressive. It usually
//               makes following errors:
//  1- horizontal lines consisting of underlines are included in the table if
//     they are close enough
//  2- horizontal lines originating from noise tend to get merged with a table
//     near the top of the page
//  3- the criteria for including horizontal lines is very generous. Many times
//     horizontal lines separating headers and footers get merged with a
//     single-column table in a multi-column page thereby including text
//     from the neighboring column inside the table
//  4- the criteria for including left out column headers also tends to
//     occasionally include text-lines above the tables, typically from
//     table caption
void TableFinder::AdjustTableBoundaries() {
  // Iterate the table regions in the grid
  ColSegment_CLIST adjusted_tables;
  ColSegment_C_IT it(&adjusted_tables);
  ColSegmentGridSearch gsearch(&table_grid_);
  gsearch.StartFullSearch();
  ColSegment* table = NULL;
  while ((table = gsearch.NextFullSearch()) != NULL) {
    const TBOX& table_box = table->bounding_box();
    TBOX grown_box = table_box;
    GrowTableBox(table_box, &grown_box);
    // To prevent a table from expanding again, do not insert the
    // modified box back to the grid. Instead move it to a list and
    // and remove it from the grid. The list is moved later back to the grid.
    if (!grown_box.null_box()) {
      ColSegment* col = new ColSegment();
      col->InsertBox(grown_box);
      it.add_after_then_move(col);
    }
    gsearch.RemoveBBox();
    delete table;
  }
  // clear table grid to move final tables in it
  // TODO(nbeato): table_grid_ should already be empty. The above loop
  // removed everything. Maybe just assert it is empty?
  table_grid_.Clear();
  it.move_to_first();
  // move back final tables to table_grid_
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    ColSegment* seg = it.extract();
    table_grid_.InsertBBox(true, true, seg);
  }
}

void TableFinder::GrowTableBox(const TBOX& table_box, TBOX* result_box) {
  // TODO(nbeato): The growing code is a bit excessive right now.
  // By removing these lines, the partitions considered need
  // to have some overlap or be special cases. These lines could
  // be added again once a check is put in place to make sure that
  // growing tables don't stomp on a lot of non-table partitions.

  // search for horizontal ruling lines within the vertical margin
  // int vertical_margin = kRulingVerticalMargin * gridsize();
  TBOX search_box = table_box;
  // int top = MIN(search_box.top() + vertical_margin, tright().y());
  // int bottom = MAX(search_box.bottom() - vertical_margin, bleft().y());
  // search_box.set_top(top);
  // search_box.set_bottom(bottom);

  GrowTableToIncludePartials(table_box, search_box, result_box);
  GrowTableToIncludeLines(table_box, search_box, result_box);
  IncludeLeftOutColumnHeaders(result_box);
}

// Grow a table by increasing the size of the box to include
// partitions with significant overlap with the table.
void TableFinder::GrowTableToIncludePartials(const TBOX& table_box,
                                             const TBOX& search_range,
                                             TBOX* result_box) {
  // Rulings are in a different grid, so search 2 grids for rulings, text,
  // and table partitions that are not entirely within the new box.
  for (int i = 0; i < 2; ++i) {
    ColPartitionGrid* grid = (i == 0) ? &fragmented_text_grid_ :
                                        &leader_and_ruling_grid_;
    ColPartitionGridSearch rectsearch(grid);
    rectsearch.StartRectSearch(search_range);
    ColPartition* part = NULL;
    while ((part = rectsearch.NextRectSearch()) != NULL) {
     // Only include text and table types.
      if (part->IsImageType())
        continue;
      const TBOX& part_box = part->bounding_box();
      // Include partition in the table if more than half of it
      // is covered by the table
      if (part_box.overlap_fraction(table_box) > kMinOverlapWithTable) {
        *result_box = result_box->bounding_union(part_box);
        continue;
      }
    }
  }
}

// Grow a table by expanding to the extents of significantly
// overlapping lines.
void TableFinder::GrowTableToIncludeLines(const TBOX& table_box,
                                          const TBOX& search_range,
                                          TBOX* result_box) {
  ColPartitionGridSearch rsearch(&leader_and_ruling_grid_);
  rsearch.SetUniqueMode(true);
  rsearch.StartRectSearch(search_range);
  ColPartition* part = NULL;
  while ((part = rsearch.NextRectSearch()) != NULL) {
    // TODO(nbeato) This should also do vertical, but column
    // boundaries are breaking things. This function needs to be
    // updated to allow vertical lines as well.
    if (!part->IsLineType())
      continue;
    // Avoid the following function call if the result of the
    // function is irrelevant.
    const TBOX& part_box = part->bounding_box();
    if (result_box->contains(part_box))
      continue;
    // Include a partially overlapping horizontal line only if the
    // extra ColPartitions that will be included due to expansion
    // have large side spacing w.r.t. columns containing them.
    if (HLineBelongsToTable(*part, table_box))
      *result_box = result_box->bounding_union(part_box);
    // TODO(nbeato): Vertical
  }
}

// Checks whether the horizontal line belong to the table by looking at the
// side spacing of extra ColParitions that will be included in the table
// due to expansion
bool TableFinder::HLineBelongsToTable(const ColPartition& part,
                                      const TBOX& table_box) {
  if (!part.IsHorizontalLine())
    return false;
  const TBOX& part_box = part.bounding_box();
  if (!part_box.major_x_overlap(table_box))
    return false;
  // Do not consider top-most horizontal line since it usually
  // originates from noise.
  // TODO(nbeato): I had to comment this out because the ruling grid doesn't
  // have neighbors solved.
  // if (!part.nearest_neighbor_above())
  //   return false;
  const TBOX bbox = part_box.bounding_union(table_box);
  // In the "unioned table" box (the table extents expanded by the line),
  // keep track of how many partitions have significant padding to the left
  // and right. If more than half of the partitions covered by the new table
  // have significant spacing, the line belongs to the table and the table
  // grows to include all of the partitions.
  int num_extra_partitions = 0;
  int extra_space_to_right = 0;
  int extra_space_to_left = 0;
  // Rulings are in a different grid, so search 2 grids for rulings, text,
  // and table partitions that are introduced by the new box.
  for (int i = 0; i < 2; ++i) {
    ColPartitionGrid* grid = (i == 0) ? &clean_part_grid_ :
                                        &leader_and_ruling_grid_;
    // Start a rect search on bbox
    ColPartitionGridSearch rectsearch(grid);
    rectsearch.SetUniqueMode(true);
    rectsearch.StartRectSearch(bbox);
    ColPartition* extra_part = NULL;
    while ((extra_part = rectsearch.NextRectSearch()) != NULL) {
      // ColPartition already in table
      const TBOX& extra_part_box = extra_part->bounding_box();
      if (extra_part_box.overlap_fraction(table_box) > kMinOverlapWithTable)
        continue;
      // Non-text ColPartitions do not contribute
      if (extra_part->IsImageType())
        continue;
      // Consider this partition.
      num_extra_partitions++;
      // presence of a table cell is a strong hint, so just increment the scores
      // without looking at the spacing.
      if (extra_part->type() == PT_TABLE || extra_part->IsLineType()) {
        extra_space_to_right++;
        extra_space_to_left++;
        continue;
      }
      int space_threshold = kSideSpaceMargin * part.median_size();
      if (extra_part->space_to_right() > space_threshold)
        extra_space_to_right++;
      if (extra_part->space_to_left() > space_threshold)
        extra_space_to_left++;
    }
  }
  // tprintf("%d %d %d\n",
  // num_extra_partitions,extra_space_to_right,extra_space_to_left);
  return (extra_space_to_right > num_extra_partitions / 2) ||
      (extra_space_to_left > num_extra_partitions / 2);
}

// Look for isolated column headers above the given table box and
// include them in the table
void TableFinder::IncludeLeftOutColumnHeaders(TBOX* table_box) {
  // Start a search above the current table to look for column headers
  ColPartitionGridSearch vsearch(&clean_part_grid_);
  vsearch.StartVerticalSearch(table_box->left(), table_box->right(),
                              table_box->top());
  ColPartition* neighbor = NULL;
  ColPartition* previous_neighbor = NULL;
  while ((neighbor = vsearch.NextVerticalSearch(false)) != NULL) {
    // Max distance to find a table heading.
    const int max_distance = kMaxColumnHeaderDistance *
                             neighbor->median_size();
    int table_top = table_box->top();
    const TBOX& box = neighbor->bounding_box();
    // Do not continue if the next box is way above
    if (box.bottom() - table_top > max_distance)
      break;
    // Unconditionally include partitions of type TABLE or LINE
    // TODO(faisal): add some reasonable conditions here
    if (neighbor->type() == PT_TABLE || neighbor->IsLineType()) {
      table_box->set_top(box.top());
      previous_neighbor = NULL;
      continue;
    }
    // If there are two text partitions, one above the other, without a table
    // cell on their left or right side, consider them a barrier and quit
    if (previous_neighbor == NULL) {
      previous_neighbor = neighbor;
    } else {
      const TBOX& previous_box = previous_neighbor->bounding_box();
      if (!box.major_y_overlap(previous_box))
        break;
    }
  }
}

// Remove false alarms consiting of a single column based on their
// projection on the x-axis. Projection of a real table on the x-axis
// should have at least one zero-valley larger than the global median
// x-height of the page.
void TableFinder::DeleteSingleColumnTables() {
  int page_width = tright().x() - bleft().x();
  ASSERT_HOST(page_width > 0);
  // create an integer array to hold projection on x-axis
  int* table_xprojection = new int[page_width];
  // Iterate through all tables in the table grid
  GridSearch<ColSegment, ColSegment_CLIST, ColSegment_C_IT>
      table_search(&table_grid_);
  table_search.StartFullSearch();
  ColSegment* table;
  while ((table = table_search.NextFullSearch()) != NULL) {
    TBOX table_box = table->bounding_box();
    // reset the projection array
    for (int i = 0; i < page_width; i++) {
      table_xprojection[i] = 0;
    }
    // Start a rect search on table_box
    GridSearch<ColPartition, ColPartition_CLIST, ColPartition_C_IT>
        rectsearch(&clean_part_grid_);
    rectsearch.SetUniqueMode(true);
    rectsearch.StartRectSearch(table_box);
    ColPartition* part;
    while ((part = rectsearch.NextRectSearch()) != NULL) {
      if (!part->IsTextType())
        continue;  // Do not consider non-text partitions
      if (part->flow() == BTFT_LEADER)
        continue;  // Assume leaders are in tables
      TBOX part_box = part->bounding_box();
      // Do not consider partitions partially covered by the table
      if (part_box.overlap_fraction(table_box) < kMinOverlapWithTable)
        continue;
      BLOBNBOX_CLIST* part_boxes = part->boxes();
      BLOBNBOX_C_IT pit(part_boxes);

      // Make sure overlapping blobs don't artificially inflate the number
      // of rows in the table. This happens frequently with things such as
      // decimals and split characters. Do this by assuming the column
      // partition is sorted mostly left to right and just clip
      // bounding boxes by the previous box's extent.
      int next_position_to_write = 0;

      for (pit.mark_cycle_pt(); !pit.cycled_list(); pit.forward()) {
        BLOBNBOX *pblob = pit.data();
        // ignore blob height for the purpose of projection since we
        // are only interested in finding valleys
        int xstart = pblob->bounding_box().left();
        int xend = pblob->bounding_box().right();

        xstart = MAX(xstart, next_position_to_write);
        for (int i = xstart; i < xend; i++)
          table_xprojection[i - bleft().x()]++;
        next_position_to_write = xend;
      }
    }
    // Find largest valley between two reasonable peaks in the table
    if (!GapInXProjection(table_xprojection, page_width)) {
      table_search.RemoveBBox();
      delete table;
    }
  }
  delete[] table_xprojection;
}

// Return true if at least one gap larger than the global x-height
// exists in the horizontal projection
bool TableFinder::GapInXProjection(int* xprojection, int length) {
  // Find peak value of the histogram
  int peak_value = 0;
  for (int i = 0; i < length; i++) {
    if (xprojection[i] > peak_value) {
      peak_value = xprojection[i];
    }
  }
  // Peak value represents the maximum number of horizontally
  // overlapping colpartitions, so this can be considered as the
  // number of rows in the table
  if (peak_value < kMinRowsInTable)
    return false;
  double projection_threshold = kSmallTableProjectionThreshold * peak_value;
  if (peak_value >= kLargeTableRowCount)
    projection_threshold = kLargeTableProjectionThreshold * peak_value;
  // Threshold the histogram
  for (int i = 0; i < length; i++) {
    xprojection[i] = (xprojection[i] >= projection_threshold) ? 1 : 0;
  }
  // Find the largest run of zeros between two ones
  int largest_gap = 0;
  int run_start = -1;
  for (int i = 1; i < length; i++) {
    // detect start of a run of zeros
    if (xprojection[i - 1] && !xprojection[i]) {
      run_start = i;
    }
    // detect end of a run of zeros and update the value of largest gap
    if (run_start != -1 && !xprojection[i - 1] && xprojection[i]) {
      int gap = i - run_start;
      if (gap > largest_gap)
        largest_gap = gap;
      run_start = -1;
    }
  }
  return largest_gap > kMaxXProjectionGapFactor * global_median_xheight_;
}

// Given the location of a table "guess", try to overlay a cellular
// grid in the location, adjusting the boundaries.
// TODO(nbeato): Falsely introduces:
//   -headers/footers (not any worse, too much overlap destroys cells)
//   -page numbers (not worse, included because maximize margins)
//   -equations (nicely fit into a celluar grid, but more sparsely)
//   -figures (random text box, also sparse)
//   -small left-aligned text areas with overlapping positioned whitespace
//       (rejected before)
// Overall, this just needs some more work.
void TableFinder::RecognizeTables() {
  ScrollView* table_win = NULL;
  if (textord_show_tables) {
    table_win = MakeWindow(0, 0, "Table Structure");
    DisplayColPartitions(table_win, &fragmented_text_grid_,
                         ScrollView::BLUE, ScrollView::LIGHT_BLUE);
    // table_grid_.DisplayBoxes(table_win);
  }


  TableRecognizer recognizer;
  recognizer.Init();
  recognizer.set_line_grid(&leader_and_ruling_grid_);
  recognizer.set_text_grid(&fragmented_text_grid_);
  recognizer.set_max_text_height(global_median_xheight_ * 2.0);
  recognizer.set_min_height(1.5 * gridheight());
  // Loop over all of the tables and try to fit them.
  // Store the good tables here.
  ColSegment_CLIST good_tables;
  ColSegment_C_IT good_it(&good_tables);

  ColSegmentGridSearch gsearch(&table_grid_);
  gsearch.StartFullSearch();
  ColSegment* found_table = NULL;
  while ((found_table = gsearch.NextFullSearch()) != NULL) {
    gsearch.RemoveBBox();

    // The goal is to make the tables persistent in a list.
    // When that happens, this will move into the search loop.
    const TBOX& found_box = found_table->bounding_box();
    StructuredTable* table_structure = recognizer.RecognizeTable(found_box);

    // Process a table. Good tables are inserted into the grid again later on
    // We can't change boxes in the grid while it is running a search.
    if (table_structure != NULL) {
      if (textord_show_tables) {
        table_structure->Display(table_win, ScrollView::LIME_GREEN);
      }
      found_table->set_bounding_box(table_structure->bounding_box());
      delete table_structure;
      good_it.add_after_then_move(found_table);
    } else {
      delete found_table;
    }
  }
  // TODO(nbeato): MERGE!! There is awesome info now available for merging.

  // At this point, the grid is empty. We can safely insert the good tables
  // back into grid.
  for (good_it.mark_cycle_pt(); !good_it.cycled_list(); good_it.forward())
    table_grid_.InsertBBox(true, true, good_it.extract());
}

// Displays the column segments in some window.
void TableFinder::DisplayColSegments(ScrollView* win,
                                     ColSegment_LIST *segments,
                                     ScrollView::Color color) {
#ifndef GRAPHICS_DISABLED
  win->Pen(color);
  win->Brush(ScrollView::NONE);
  ColSegment_IT it(segments);
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    ColSegment* col = it.data();
    const TBOX& box = col->bounding_box();
    int left_x = box.left();
    int right_x = box.right();
    int top_y = box.top();
    int bottom_y = box.bottom();
    win->Rectangle(left_x, bottom_y, right_x, top_y);
  }
  win->UpdateWindow();
#endif
}

void TableFinder::DisplayColSegmentGrid(ScrollView* win, ColSegmentGrid* grid,
                                         ScrollView::Color color) {
#ifndef GRAPHICS_DISABLED
  // Iterate the ColPartitions in the grid.
  GridSearch<ColSegment, ColSegment_CLIST, ColSegment_C_IT>
    gsearch(grid);
  gsearch.StartFullSearch();
  ColSegment* seg = NULL;
  while ((seg = gsearch.NextFullSearch()) != NULL) {
    const TBOX& box = seg->bounding_box();
    int left_x = box.left();
    int right_x = box.right();
    int top_y = box.top();
    int bottom_y = box.bottom();
    win->Brush(ScrollView::NONE);
    win->Pen(color);
    win->Rectangle(left_x, bottom_y, right_x, top_y);
  }
  win->UpdateWindow();
#endif
}

// Displays the colpartitions using a new coloring on an existing window.
// Note: This method is only for debug purpose during development and
// would not be part of checked in code
void TableFinder::DisplayColPartitions(ScrollView* win,
                                       ColPartitionGrid* grid,
                                       ScrollView::Color default_color,
                                       ScrollView::Color table_color) {
#ifndef GRAPHICS_DISABLED
  ScrollView::Color color = default_color;
  // Iterate the ColPartitions in the grid.
  GridSearch<ColPartition, ColPartition_CLIST, ColPartition_C_IT>
    gsearch(grid);
  gsearch.StartFullSearch();
  ColPartition* part = NULL;
  while ((part = gsearch.NextFullSearch()) != NULL) {
    color = default_color;
    if (part->type() == PT_TABLE)
      color = table_color;

    const TBOX& box = part->bounding_box();
    int left_x = box.left();
    int right_x = box.right();
    int top_y = box.top();
    int bottom_y = box.bottom();
    win->Brush(ScrollView::NONE);
    win->Pen(color);
    win->Rectangle(left_x, bottom_y, right_x, top_y);
  }
  win->UpdateWindow();
#endif
}
void TableFinder::DisplayColPartitions(ScrollView* win,
                                       ColPartitionGrid* grid,
                                       ScrollView::Color default_color) {
  DisplayColPartitions(win, grid, default_color, ScrollView::YELLOW);
}

void TableFinder::DisplayColPartitionConnections(
                     ScrollView* win,
                     ColPartitionGrid* grid,
                     ScrollView::Color color) {
#ifndef GRAPHICS_DISABLED
  // Iterate the ColPartitions in the grid.
  GridSearch<ColPartition, ColPartition_CLIST, ColPartition_C_IT>
    gsearch(grid);
  gsearch.StartFullSearch();
  ColPartition* part = NULL;
  while ((part = gsearch.NextFullSearch()) != NULL) {
    const TBOX& box = part->bounding_box();
    int left_x = box.left();
    int right_x = box.right();
    int top_y = box.top();
    int bottom_y = box.bottom();

    ColPartition* upper_part = part->nearest_neighbor_above();
    if (upper_part) {
      TBOX upper_box = upper_part->bounding_box();
      int mid_x = (left_x + right_x) / 2;
      int mid_y = (top_y + bottom_y) / 2;
      int other_x = (upper_box.left() + upper_box.right()) / 2;
      int other_y = (upper_box.top() + upper_box.bottom()) / 2;
      win->Brush(ScrollView::NONE);
      win->Pen(color);
      win->Line(mid_x, mid_y, other_x, other_y);
    }
    ColPartition* lower_part = part->nearest_neighbor_below();
    if (lower_part) {
      TBOX lower_box = lower_part->bounding_box();
      int mid_x = (left_x + right_x) / 2;
      int mid_y = (top_y + bottom_y) / 2;
      int other_x = (lower_box.left() + lower_box.right()) / 2;
      int other_y = (lower_box.top() + lower_box.bottom()) / 2;
      win->Brush(ScrollView::NONE);
      win->Pen(color);
      win->Line(mid_x, mid_y, other_x, other_y);
    }
  }
  win->UpdateWindow();
#endif
}


// Write debug image and text file.
// Note: This method is only for debug purpose during development and
// would not be part of checked in code
void TableFinder::WriteToPix(const FCOORD& reskew) {
  // Input file must be named test1.tif
  PIX* pix = pixRead("test1.tif");
  if (!pix) {
    tprintf("Input file test1.tif not found.\n");
    return;
  }
  int img_height = pixGetHeight(pix);
  int img_width = pixGetWidth(pix);
  // Maximum number of text or table partitions
  int num_boxes = 10;
  BOXA* text_box_array = boxaCreate(num_boxes);
  BOXA* table_box_array = boxaCreate(num_boxes);
  GridSearch<ColPartition, ColPartition_CLIST, ColPartition_C_IT>
    gsearch(&clean_part_grid_);
  gsearch.StartFullSearch();
  ColPartition* part;
  // load colpartitions into text_box_array and table_box_array
  while ((part = gsearch.NextFullSearch()) != NULL) {
    TBOX box = part->bounding_box();
    box.rotate_large(reskew);
    BOX* lept_box = boxCreate(box.left(), img_height - box.top(),
                              box.right() - box.left(),
                              box.top() - box.bottom());
    if (part->type() == PT_TABLE)
      boxaAddBox(table_box_array, lept_box, L_INSERT);
    else
      boxaAddBox(text_box_array, lept_box, L_INSERT);
  }
  // draw colpartitions on the output image
  PIX* out = pixDrawBoxa(pix, text_box_array, 3, 0xff000000);
  out = pixDrawBoxa(out, table_box_array, 3, 0x0000ff00);

  BOXA* table_array = boxaCreate(num_boxes);
  // text file containing detected table bounding boxes
  FILE* fptr = fopen("tess-table.txt", "wb");
  GridSearch<ColSegment, ColSegment_CLIST, ColSegment_C_IT>
      table_search(&table_grid_);
  table_search.StartFullSearch();
  ColSegment* table;
  // load table boxes to table_array and write them to text file as well
  while ((table = table_search.NextFullSearch()) != NULL) {
    TBOX box = table->bounding_box();
    box.rotate_large(reskew);
    // Since deskewing introduces negative coordinates, reskewing
    // might not completely recover from that since both steps enlarge
    // the actual box. Hence a box that undergoes deskewing/reskewing
    // may go out of image boundaries. Crop a table box if needed to
    // contain it inside the image dimensions.
    box = box.intersection(TBOX(0, 0, img_width - 1, img_height - 1));
    BOX* lept_box = boxCreate(box.left(), img_height - box.top(),
                              box.right() - box.left(),
                              box.top() - box.bottom());
    boxaAddBox(table_array, lept_box, L_INSERT);
    fprintf(fptr, "%d %d %d %d TABLE\n", box.left(),
            img_height - box.top(), box.right(), img_height - box.bottom());
  }
  fclose(fptr);
  // paint table boxes on the debug image
  out = pixDrawBoxa(out, table_array, 5, 0x7fff0000);

  pixWrite("out.png", out, IFF_PNG);
  // memory cleanup
  boxaDestroy(&text_box_array);
  boxaDestroy(&table_box_array);
  boxaDestroy(&table_array);
  pixDestroy(&pix);
  pixDestroy(&out);
}

// Merge all colpartitions in table regions to make them a single
// colpartition and revert types of isolated table cells not
// assigned to any table to their original types.
void TableFinder::MakeTableBlocks(ColPartitionGrid* grid,
                                  ColPartitionSet** all_columns,
                                  WidthCallback* width_cb) {
  // Since we have table blocks already, remove table tags from all
  // colpartitions
  GridSearch<ColPartition, ColPartition_CLIST, ColPartition_C_IT>
    gsearch(grid);
  gsearch.StartFullSearch();
  ColPartition* part = NULL;

  while ((part = gsearch.NextFullSearch()) != NULL) {
    if (part->type() == PT_TABLE) {
      part->clear_table_type();
    }
  }
  // Now make a single colpartition out of each table block and remove
  // all colpartitions contained within a table
  GridSearch<ColSegment, ColSegment_CLIST, ColSegment_C_IT>
      table_search(&table_grid_);
  table_search.StartFullSearch();
  ColSegment* table;
  while ((table = table_search.NextFullSearch()) != NULL) {
    TBOX table_box = table->bounding_box();
    // Start a rect search on table_box
    GridSearch<ColPartition, ColPartition_CLIST, ColPartition_C_IT>
        rectsearch(grid);
    rectsearch.StartRectSearch(table_box);
    ColPartition* part;
    ColPartition* table_partition = NULL;
    while ((part = rectsearch.NextRectSearch()) != NULL) {
     // Do not consider image partitions
      if (!part->IsTextType())
        continue;
      TBOX part_box = part->bounding_box();
      // Include partition in the table if more than half of it
      // is covered by the table
      if (part_box.overlap_fraction(table_box) > kMinOverlapWithTable) {
        rectsearch.RemoveBBox();
        if (table_partition) {
          table_partition->Absorb(part, width_cb);
        } else {
          table_partition = part;
        }
      }
    }
    // Insert table colpartition back to part_grid_
    if (table_partition) {
      // To match the columns used when transforming to blocks, the new table
      // partition must have its first and last column set at the grid y that
      // corresponds to its bottom.
      const TBOX& table_box = table_partition->bounding_box();
      int grid_x, grid_y;
      grid->GridCoords(table_box.left(), table_box.bottom(), &grid_x, &grid_y);
      table_partition->SetPartitionType(resolution_, all_columns[grid_y]);
      table_partition->set_table_type();
      table_partition->set_blob_type(BRT_TEXT);
      table_partition->set_flow(BTFT_CHAIN);
      table_partition->SetBlobTypes();
      grid->InsertBBox(true, true, table_partition);
    }
  }
}

//////// ColSegment code
////////
ColSegment::ColSegment()
    : ELIST_LINK(),
      num_table_cells_(0),
      num_text_cells_(0),
      type_(COL_UNKNOWN) {
}
ColSegment::~ColSegment() {
}

// Provides a color for BBGrid to draw the rectangle.
ScrollView::Color  ColSegment::BoxColor() const {
  const ScrollView::Color kBoxColors[PT_COUNT] = {
    ScrollView::YELLOW,
    ScrollView::BLUE,
    ScrollView::YELLOW,
    ScrollView::MAGENTA,
  };
  return kBoxColors[type_];
}

// Insert a box into this column segment
void ColSegment::InsertBox(const TBOX& other) {
  bounding_box_ = bounding_box_.bounding_union(other);
}

// Set column segment type based on the ratio of text and table partitions
// in it.
void ColSegment::set_type() {
  if (num_table_cells_ > kTableColumnThreshold * num_text_cells_)
    type_ = COL_TABLE;
  else if (num_text_cells_ > num_table_cells_)
    type_ = COL_TEXT;
  else
    type_ = COL_MIXED;
}

}  // namespace tesseract.
