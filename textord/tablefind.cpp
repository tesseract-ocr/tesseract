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

#include "colfind.h"
#include <math.h>
#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif
#ifdef HAVE_LIBLEPT
#include "allheaders.h"
#endif

namespace tesseract {

// Maximum vertical spacing between neighbor partitions
const int kMaxVerticalSpacing = 500;

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

// Maximum x-height a table partition can have as a multiple of global
// median x-height
const double kMaxTableCellXheight = 2.0;

// Maximum line spacing between a table column header and column contents
// for merging the two
const int kMaxColumnHeaderDistance = 100;

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
const double kProjectionThreshold = 0.35;

// Minmimum number of rows in a table
const int kMinRowsInTable = 3;

BOOL_VAR(textord_dump_table_images, false, "Paint table detection output");
BOOL_VAR(textord_show_tables, false, "Show table regions");

ELISTIZE(ColSegment)
CLISTIZE(ColSegment)

// Copy cleaned partitions from part_grid_ to clean_part_grid_ and
// insert dot-like noise into period_grid_
void ColumnFinder::GetCleanPartitions(TO_BLOCK* block) {
  double min_dim = block->line_size/3.0;
  // Initialize clean partitions list and grid
  clean_part_grid_.Init(gridsize(), bleft(), tright());
  period_grid_.Init(gridsize(), bleft(), tright());
  // Iterate the ColPartitions in the grid.
  GridSearch<ColPartition, ColPartition_CLIST, ColPartition_C_IT>
    gsearch(&part_grid_);
  gsearch.StartFullSearch();
  ColPartition* part;
  while ((part = gsearch.NextFullSearch()) != NULL) {
    ColPartition* clean_part = part->ShallowCopy();
    // Insert all non-text partitions to clean_parts
    if (!part->IsTextType()) {
      clean_part_grid_.InsertBBox(true, true, clean_part);
      continue;
    }
    // Insert text colpartitions after removing noisy components from them
    BLOBNBOX_CLIST* part_boxes = part->boxes();
    BLOBNBOX_C_IT pit(part_boxes);
    for (pit.mark_cycle_pt(); !pit.cycled_list(); pit.forward()) {
      BLOBNBOX *pblob = pit.data();
      if (!pblob->noise_flag()) {
        clean_part->AddBox(pblob);
      } else {
        TBOX blob_box = pblob->bounding_box();
        if (blob_box.height() < min_dim && blob_box.width() < 2*min_dim) {
          period_grid_.InsertBBox(false, false, pblob);
        }
      }
    }
    if (!clean_part->IsEmpty())
      clean_part_grid_.InsertBBox(true, true, clean_part);
    else
      delete clean_part;
  }

// TODO(rays) This is the previous period blob code. Neither is completely
// satisfactory, as a more disciplined approach to noise removal would be
// better, so revisit this choice and decide what to keep when the earlier
// stages do a better job of noise removal.
#if 0
  BLOBNBOX_IT sit(&block->small_blobs);
  BLOBNBOX_IT nit(&block->noise_blobs);
  BLOBNBOX_IT it(&period_blobs_);
  // Insert dot sized boxes from small_blobs into period_blobs_
  for (sit.mark_cycle_pt(); !sit.cycled_list(); sit.forward()) {
    BLOBNBOX * blob = sit.data();
    TBOX blob_box = blob->bounding_box();
    if (blob_box.height() < min_dim && blob_box.width() < 2*min_dim) {
      it.add_after_then_move(sit.extract());
    }
  }
  // Insert dot sized boxes from noise_blobs into period_blobs_
  for (nit.mark_cycle_pt(); !nit.cycled_list(); nit.forward()) {
    BLOBNBOX * blob = nit.data();
    TBOX blob_box = blob->bounding_box();
    if (blob_box.height() < min_dim && blob_box.width() < 2*min_dim) {
      it.add_after_then_move(nit.extract());
    }
  }
  InsertBlobList(false, false, false, &period_blobs_, false, &period_grid_);
#endif
}

// High level function to perform table detection
void ColumnFinder::LocateTables() {
  // Make single-column blocks from good_columns_ partitions. col_segments are
  // moved to a grid later which takes the ownership
  ColSegment_LIST column_blocks;
  GetColumnBlocks(&column_blocks);

  SetPartitionSpacings();

  // Mark ColPartitions as being candidate table partition depending on
  // the inter-word spacing
  GridMarkTablePartitions();
  FilterFalseAlarms();
  SmoothTablePartitionRuns();

  // Set the ratio of candidate table partitions in each column
  SetColumnsType(&column_blocks);

  // Move column segments to col_seg_grid_
  MoveColSegmentsToGrid(&column_blocks, &col_seg_grid_);

  // Detect split in column layout that might have occured due to the
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

  // Merge table regions across columns for tables spanning multiple
  // columns
  MoveColSegmentsToGrid(&table_regions, &table_grid_);
  GridMergeTableRegions();

  // Adjust table boundaries by including nearby horizontal lines and left
  // out column headers
  AdjustTableBoundaries();
  GridMergeTableRegions();

  // Remove false alarms consiting of a single column
  DeleteSingleColumnTables();

  if (textord_show_tables) {
    ScrollView* table_win = MakeWindow(1500, 300, "Detected Tables");
    DisplayColPartitions(table_win, ScrollView::BLUE);
    DisplayColSegments(&table_columns, table_win, ScrollView::GREEN);
    table_grid_.DisplayBoxes(table_win);
  }

  if (textord_dump_table_images)
    WriteToPix();

  // Merge all colpartitions in table regions to make them a single
  // colpartition and revert types of isolated table cells not
  // assigned to any table to their original types.
  MakeTableBlocks();
}

// Make single-column blocks from good_columns_ partitions.
void ColumnFinder::GetColumnBlocks(ColSegment_LIST* column_blocks) {
  for (int i = 0; i < gridheight_; ++i) {
    ColPartitionSet* columns = best_columns_[i];
    if (columns != NULL) {
      ColSegment_LIST new_blocks;
      // Get boxes from the current vertical position on the grid
      columns->GetColumnBoxes(i*gridsize_, (i + 1) * gridsize_, &new_blocks);
      // Merge the new_blocks boxes into column_blocks if they are well-aligned
      GroupColumnBlocks(&new_blocks, column_blocks);
    }
  }
}

// Merge column segments into the current list if they are well aligned.
void ColumnFinder::GroupColumnBlocks(ColSegment_LIST* new_blocks,
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
bool ColumnFinder::ConsecutiveBoxes(const TBOX &b1, const TBOX &b2) {
  int x_margin = 20;
  int y_margin = 5;
  return (abs(b1.left() - b2.left()) < x_margin) &&
      (abs(b1.right() - b2.right()) < x_margin) &&
      (abs(b1.top()-b2.bottom()) < y_margin ||
       abs(b2.top()-b1.bottom()) < y_margin);
}

// Set left, right and top, bottom spacings of each colpartition.
void ColumnFinder::SetPartitionSpacings() {
  // Iterate the ColPartitions in the grid.
  GridSearch<ColPartition, ColPartition_CLIST, ColPartition_C_IT>
    gsearch(&clean_part_grid_);
  gsearch.StartFullSearch();
  ColPartition* part;
  while ((part = gsearch.NextFullSearch()) != NULL) {
    ColPartitionSet* columns = best_columns_[gsearch.GridY()];
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
    SetVerticalSpacing(part);
  }
  SetGlobalSpacings();
}

// Set spacing and closest neighbors above and below a given colpartition.
void ColumnFinder::SetVerticalSpacing(ColPartition* part) {
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
void ColumnFinder::SetGlobalSpacings() {
  STATS xheight_stats(0, kMaxVerticalSpacing + 1);
  STATS ledding_stats(0, kMaxVerticalSpacing + 1);
  // Iterate the ColPartitions in the grid.
  GridSearch<ColPartition, ColPartition_CLIST, ColPartition_C_IT>
    gsearch(&clean_part_grid_);
  gsearch.StartFullSearch();
  ColPartition* part;
  while ((part = gsearch.NextFullSearch()) != NULL) {
    if (part->IsTextType()) {
      xheight_stats.add(part->median_size(), 1);
      ledding_stats.add(part->space_above(), 1);
      ledding_stats.add(part->space_below(), 1);
    }
  }
  // Set estimates based on median of statistics obtained
  global_median_xheight_ = static_cast<int>(xheight_stats.median() + 0.5);
  global_median_ledding_ = static_cast<int>(ledding_stats.median() + 0.5);
  if (textord_show_tables) {
    ScrollView* stats_win = MakeWindow(500, 10,
                                       "X-height and ledding histograms");
    xheight_stats.plot(stats_win, 10, 200, 2, 15, ScrollView::GREEN);
    ledding_stats.plot(stats_win, 10, 200, 2, 15, ScrollView::RED);
  }
}

// Three types of partitions are maked as table partitions:
//  1- Partitions that have at lease one large gap between words
//  2- Partitions that consist of only one word (no significant gap
//     between components)
//  3- Partitions that vertically overlap with other partitions within the
//     same column.
void ColumnFinder::GridMarkTablePartitions() {
  // Iterate the ColPartitions in the grid.
  GridSearch<ColPartition, ColPartition_CLIST, ColPartition_C_IT>
    gsearch(&clean_part_grid_);
  gsearch.StartFullSearch();
  ColPartition* part;
  while ((part = gsearch.NextFullSearch()) != NULL) {
    if (!part->IsTextType())  // Only consider text partitions
      continue;
    // Only consider partitions in dominant font size or smaller
    if (part->median_size() > kMaxTableCellXheight * global_median_xheight_)
      continue;
    // Mark partitions with a large gap, or no significant gap as
    // table partitions.
    // Comments: It produces several false alarms at:
    //  - last line of a paragraph
    //  - single word section headings
    //  - page headers and footers
    //  - numbered equations
    //  - line drawing regions
    // TODO(faisal): detect and fix above-mentioned cases
    if (HasWideOrNoInterWordGap(part)) {
      part->set_table_type();
    }
  }
}

// Check if the partition has at lease one large gap between words or no
// significant gap at all
bool ColumnFinder::HasWideOrNoInterWordGap(ColPartition* part) {
  BLOBNBOX_CLIST* part_boxes = part->boxes();
  BLOBNBOX_C_IT pit(part_boxes);

  if (part->bounding_box().width() <
      kMinBoxesInTextPartition * part->median_size() &&
      pit.length() < kMinBoxesInTextPartition)
    return true;

  // Make a copy of the components in the current partition and insert periods
  // into it to compute gaps while taking periods into account.
  BLOBNBOX_CLIST boxes;
  BLOBNBOX_C_IT it(&boxes);
  for (pit.mark_cycle_pt(); !pit.cycled_list(); pit.forward()) {
    BLOBNBOX *pblob = pit.data();
    it.add_after_then_move(pblob);
  }
  // Start rectangular search to find periods in this partition
  GridSearch<BLOBNBOX, BLOBNBOX_CLIST, BLOBNBOX_C_IT> rectsearch(&period_grid_);
  TBOX part_box = part->bounding_box();
  rectsearch.StartRectSearch(part_box);
  BLOBNBOX* period;
  while ((period = rectsearch.NextRectSearch()) != NULL) {
    // Insert a period only if it lies in a gap between two consecutive boxes
    if (LiesInGap(period, &boxes))
      boxes.add_sorted(SortByBoxLeft<BLOBNBOX>, true, period);
  }

  int current_x0;
  int current_x1;
  int previous_x1 = -1;
  int max_partition_gap = -1;
  double max_gap = kMaxGapInTextPartition * part->median_size();
  double min_gap = kMinMaxGapInTextPartition * part->median_size();

  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    BLOBNBOX *blob = it.data();
    current_x0 = blob->bounding_box().left();
    current_x1 = blob->bounding_box().right();
    if (previous_x1 != -1) {
      int gap = current_x0 - previous_x1;
      // If a large enough gap is found, mark it as a table cell (return true)
      if (gap > max_gap)
        return true;
      if (gap > max_partition_gap)
        max_partition_gap = gap;
    }
    previous_x1 = current_x1;
  }
  // Since no large gap was found, return false if the partition is too
  // long to be a data cell
  if (part->bounding_box().width() >
      kMaxBoxesInDataPartition * part->median_size() ||
      pit.length() > kMaxBoxesInDataPartition)
    return false;

  // return true if the maximum gap found is smaller than the minimum allowed
  // max_gap in a text partition
  return (max_partition_gap < min_gap);
}

// Check if the period lies in a gap between consecutive boxes
bool ColumnFinder::LiesInGap(BLOBNBOX* period, BLOBNBOX_CLIST* boxes) {
  BLOBNBOX_C_IT it(boxes);
  TBOX period_box = period->bounding_box();
  int num_boxes = it.length();
  // skip the first element since it has no gap to its left.
  it.forward();
  for (int i = 1; i < num_boxes; i++) {
    TBOX box = it.data()->bounding_box();
    TBOX previous_blob = it.data_relative(-1)->bounding_box();
    TBOX gap_box = TBOX(previous_blob.botright(), box.topleft());
    if (gap_box.major_overlap(period_box)) {
      return true;
    }
    it.forward();
  }
  return false;
}

// Filter individual text partitions marked as table partitions
// consisting of paragraph endings, small section headings, and
// headers and footers.
void ColumnFinder::FilterFalseAlarms() {
  // Detect last line of paragraph
  // Iterate the ColPartitions in the grid.
  GridSearch<ColPartition, ColPartition_CLIST, ColPartition_C_IT>
    gsearch(&clean_part_grid_);
  gsearch.StartFullSearch();
  ColPartition* part;
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
    // Paragraph ending should be left-aligned to text line above it.
    if (abs(part->bounding_box().left() - upper_part->bounding_box().left())
        > global_median_xheight_)
      continue;
    // Ledding above the line should be less than ledding below
    if (part->space_above() < part->space_below() &&
        part->space_above() <= 2 * global_median_ledding_)
      part->clear_table_type();
  }
  // Consider top-most text colpartition as header and bottom most as footer
  ColPartition* header = NULL;
  ColPartition* footer = NULL;
  int max_top = -MAX_INT32;
  int min_bottom = MAX_INT32;
  gsearch.StartFullSearch();
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
void ColumnFinder::SmoothTablePartitionRuns() {
  // Iterate the ColPartitions in the grid.
  GridSearch<ColPartition, ColPartition_CLIST, ColPartition_C_IT>
    gsearch(&clean_part_grid_);
  gsearch.StartFullSearch();
  ColPartition* part;
  while ((part = gsearch.NextFullSearch()) != NULL) {
    if (part->type() >= PT_TABLE)
      continue;  // Consider only text partitions
    ColPartition* upper_part = part->nearest_neighbor_above();
    ColPartition* lower_part = part->nearest_neighbor_below();
    if (!upper_part || !lower_part)
      continue;
    if (upper_part->type() == PT_TABLE && lower_part->type() == PT_TABLE)
      part->set_table_type();
  }
}

// Set the type of a column segment based on the ratio of table to text cells
void ColumnFinder::SetColumnsType(ColSegment_LIST* column_blocks) {
  ColSegment_IT it(column_blocks);
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    ColSegment* seg = it.data();
    TBOX box = seg->bounding_box();
    int num_table_cells = 0;
    int num_text_cells = 0;
    GridSearch<ColPartition, ColPartition_CLIST, ColPartition_C_IT>
        rsearch(&clean_part_grid_);
    rsearch.StartRectSearch(box);
    ColPartition* part;
    while ((part = rsearch.NextRectSearch()) != NULL) {
      if (!rsearch.ReturnedSeedElement())
        continue;  // Consider each partition only once
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
void ColumnFinder::MoveColSegmentsToGrid(ColSegment_LIST *segments,
                                         ColSegmentGrid *col_seg_grid) {
  col_seg_grid->Init(gridsize(), bleft(), tright());
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
void ColumnFinder::GridMergeColumnBlocks() {
  int margin = gridsize_;

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
      ColSegment* neighbor;
      while ((neighbor = rectsearch.NextRectSearch()) != NULL) {
        if (neighbor == seg)
          continue;
        TBOX neighbor_box = neighbor->bounding_box();
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
void ColumnFinder::GetTableColumns(ColSegment_LIST *table_columns) {
  ColSegment_IT it(table_columns);
  // Iterate the ColPartitions in the grid.
  GridSearch<ColPartition, ColPartition_CLIST, ColPartition_C_IT>
    gsearch(&clean_part_grid_);
  gsearch.StartFullSearch();
  ColPartition* part;
  while ((part = gsearch.NextFullSearch()) != NULL) {
    if (part->inside_table_column() || part->type() != PT_TABLE)
      continue;  // prevent a partition to be assigned to multiple columns
    TBOX box = part->bounding_box();
    ColSegment* col = new ColSegment();
    col->InsertBox(box);
    part->set_inside_table_column(true);
    // Start a search below the current cell to find bottom neighbours
    GridSearch<ColPartition, ColPartition_CLIST, ColPartition_C_IT>
        vsearch(&clean_part_grid_);
    vsearch.StartVerticalSearch(box.left(), box.right(), box.bottom());
    ColPartition* neighbor;
    bool found_neighbours = false;
    while ((neighbor = vsearch.NextVerticalSearch(true)) != NULL) {
      // only consider neighbors not assigned to any column yet
      if (neighbor->inside_table_column())
        continue;

      // presence of a non-table neighbor marks the end of current
      // table column
      if (neighbor->type() != PT_TABLE) {
        // Horizontal lines should not break the flow
        if (neighbor->IsLineType())
          continue;
        else
          break;
      }
      TBOX neighbor_box = neighbor->bounding_box();
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
void ColumnFinder::GetTableRegions(ColSegment_LIST* table_columns,
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
void ColumnFinder::GridMergeTableRegions() {
  // Iterate the table regions in the grid.
  GridSearch<ColSegment, ColSegment_CLIST, ColSegment_C_IT>
      gsearch(&table_grid_);
  gsearch.StartFullSearch();
  ColSegment* seg;
  while ((seg = gsearch.NextFullSearch()) != NULL) {
    bool neighbor_found = false;
    bool modified = false;  // Modified at least once
    do {
      // Start a rectangle search x-bounded by the image and y by the table
      TBOX box = seg->bounding_box();
      TBOX search_region(box);
      search_region.set_left(bleft().x());
      search_region.set_right(tright().x());
      neighbor_found = false;
      GridSearch<ColSegment, ColSegment_CLIST, ColSegment_C_IT>
          rectsearch(&table_grid_);
      rectsearch.StartRectSearch(search_region);
      ColSegment* neighbor;
      while ((neighbor = rectsearch.NextRectSearch()) != NULL) {
        if (neighbor == seg)
          continue;
        TBOX neighbor_box = neighbor->bounding_box();
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
bool ColumnFinder::BelongToOneTable(const TBOX &box1, const TBOX &box2) {
  // Check for ColPartitions spanning both table regions
  TBOX bbox = box1.bounding_union(box2);
  // Start a rect search on bbox
  GridSearch<ColPartition, ColPartition_CLIST, ColPartition_C_IT>
      rectsearch(&clean_part_grid_);
  rectsearch.StartRectSearch(bbox);
  ColPartition* part;
  while ((part = rectsearch.NextRectSearch()) != NULL) {
    TBOX part_box = part->bounding_box();
    // return true if a colpartition spanning both table regions is found
    if (part_box.overlap(box1) && part_box.overlap(box2))
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
void ColumnFinder::AdjustTableBoundaries() {
  // Iterate the table regions in the grid
  ColSegment_CLIST adjusted_tables;
  ColSegment_C_IT it(&adjusted_tables);
  GridSearch<ColSegment, ColSegment_CLIST, ColSegment_C_IT>
      gsearch(&table_grid_);
  gsearch.StartFullSearch();
  ColSegment* table;
  // search for horizontal ruling lines within the vertical margin
  int vertical_margin = kRulingVerticalMargin * gridsize_;
  while ((table = gsearch.NextFullSearch()) != NULL) {
    TBOX table_box = table->bounding_box();
    TBOX search_box = table_box;
    int top = MIN(search_box.top() + vertical_margin, tright().y());
    int bottom = MAX(search_box.bottom() - vertical_margin, bleft().y());
    search_box.set_top(top);
    search_box.set_bottom(bottom);
    TBOX box;
    // Start a rect search on table_box
    GridSearch<ColPartition, ColPartition_CLIST, ColPartition_C_IT>
        rectsearch(&clean_part_grid_);
    rectsearch.StartRectSearch(search_box);
    ColPartition* part;
    while ((part = rectsearch.NextRectSearch()) != NULL) {
     // Do not consider image partitions
      if (part->IsImageType())
        continue;
      TBOX part_box = part->bounding_box();
      // Include partition in the table if more than half of it
      // is covered by the table
      if (part_box.overlap_fraction(table_box) > kMinOverlapWithTable) {
        box = box.bounding_union(part_box);
        continue;
      }
      // Include a partially overlapping horizontal line only if the
      // extra ColPartitions that will be included due to expansion
      // have large side spacing w.r.t. columns containing them.
      if (HLineBelongsToTable(part, table_box)) {
        box = box.bounding_union(part_box);
      }
    }
    IncludeLeftOutColumnHeaders(box);
    // To prevent a table from expanding again, do not insert the
    // modified box back to the grid. Instead move it to a list and
    // and remove it from the grid. The list is moved later back to the grid.
    if (!box.null_box()) {
      ColSegment* col = new ColSegment();
      col->InsertBox(box);
      it.add_after_then_move(col);
    }
    gsearch.RemoveBBox();
    delete table;
  }
  // clear table grid to move final tables in it
  table_grid_.Clear();
  it.move_to_first();
  // move back final tables to table_grid_
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    ColSegment* seg = it.extract();
    table_grid_.InsertBBox(true, true, seg);
  }
}

// Checks whether the horizontal line belong to the table by looking at the
// side spacing of extra ColParitions that will be included in the table
// due to expansion
bool ColumnFinder::HLineBelongsToTable(ColPartition* part,
                                       const TBOX& table_box) {
  TBOX part_box = part->bounding_box();
  if (!part->IsLineType() || !part_box.major_x_overlap(table_box))
    return false;
  // Do not consider top-most horizontal line since it usually
  // originates from noise
  if (!part->nearest_neighbor_above())
    return false;
  TBOX bbox = part_box.bounding_union(table_box);
  // Start a rect search on bbox
  GridSearch<ColPartition, ColPartition_CLIST, ColPartition_C_IT>
      rectsearch(&clean_part_grid_);
  rectsearch.StartRectSearch(bbox);
  ColPartition* extra_part;
  int num_extra_partitions = 0;
  int extra_space_to_right = 0;
  int extra_space_to_left = 0;
  while ((extra_part = rectsearch.NextRectSearch()) != NULL) {
    if (!rectsearch.ReturnedSeedElement())
      continue;
    TBOX extra_part_box = extra_part->bounding_box();
    if (extra_part_box.overlap_fraction(table_box) > 0.6)
      continue;  // ColPartition already in table
    if (extra_part->IsImageType())  // Non-text ColPartitions do not contribute
      continue;
    num_extra_partitions++;
    // presense of a table cell is a strong hint, so just increment the scores
    // without looking at the spacing.
    if (extra_part->type() == PT_TABLE || extra_part->IsLineType()) {
      extra_space_to_right++;
      extra_space_to_left++;
      continue;
    }
    int space_threshold = kSideSpaceMargin * part->median_size();
    if (extra_part->space_to_right() > space_threshold)
      extra_space_to_right++;
    if (extra_part->space_to_left() > space_threshold)
      extra_space_to_left++;
  }
  // tprintf("%d %d %d\n",
  // num_extra_partitions,extra_space_to_right,extra_space_to_left);
  return (extra_space_to_right > num_extra_partitions / 2) ||
      (extra_space_to_left > num_extra_partitions / 2);
}

// Look for isolated column headers above the given table box and
// include them in the table
void ColumnFinder::IncludeLeftOutColumnHeaders(TBOX& table_box) {
  // Start a search above the current table to look for column headers
  GridSearch<ColPartition, ColPartition_CLIST, ColPartition_C_IT>
      vsearch(&clean_part_grid_);
  vsearch.StartVerticalSearch(table_box.left(), table_box.right(),
                              table_box.top());
  ColPartition* neighbor;
  ColPartition* previous_neighbor = NULL;
  while ((neighbor = vsearch.NextVerticalSearch(false)) != NULL) {
    int table_top = table_box.top();
    TBOX box = neighbor->bounding_box();
    // Do not continue if the next box is way above
    // TODO(faisal): make the threshold some factor of line spacing
    if (box.bottom() - table_top > kMaxColumnHeaderDistance)
      break;
    // Unconditionally include partitions of type TABLE or LINE
    // TODO(faisal): add some reasonable conditions here
    if (neighbor->type() == PT_TABLE || neighbor->IsLineType()) {
      table_box.set_top(box.top());
      previous_neighbor = NULL;
      continue;
    }
    // If there are two text partitions, one above the other, without a table
    // cell on their left or right side, consider them a barrier and quit
    if (previous_neighbor == NULL) {
      previous_neighbor = neighbor;
    } else {
      TBOX previous_box = previous_neighbor->bounding_box();
      if (!box.major_y_overlap(previous_box))
        break;
    }
  }
}

// Remove false alarms consiting of a single column based on their
// projection on the x-axis. Projection of a real table on the x-axis
// should have at least one zero-valley larger than the global median
// x-height of the page.
void ColumnFinder::DeleteSingleColumnTables() {
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
    rectsearch.StartRectSearch(table_box);
    ColPartition* part;
    while ((part = rectsearch.NextRectSearch()) != NULL) {
      if (!rectsearch.ReturnedSeedElement())
        continue;  // Consider each partition only once
      if (!part->IsTextType())
        continue;  // Do not consider non-text partitions
      TBOX part_box = part->bounding_box();
      // Do not consider partitions partially covered by the table
      if (part_box.overlap_fraction(table_box) < kMinOverlapWithTable)
        continue;
      BLOBNBOX_CLIST* part_boxes = part->boxes();
      BLOBNBOX_C_IT pit(part_boxes);
      for (pit.mark_cycle_pt(); !pit.cycled_list(); pit.forward()) {
        BLOBNBOX *pblob = pit.data();
        // ignore blob height for the purpose of projection since we
        // are only interested in finding valleys
        int xstart = pblob->bounding_box().left();
        int xend = pblob->bounding_box().right();
        for (int i = xstart; i < xend; i++)
          table_xprojection[i - bleft().x()]++;
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
bool ColumnFinder::GapInXProjection(int* xprojection, int length) {
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
  double projection_threshold = kProjectionThreshold * peak_value;
  // Threshold the histogram
  for (int i = 0; i < length; i++) {
    xprojection[i] = (xprojection[i] > projection_threshold) ? 1 : 0;
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
  return (largest_gap > global_median_xheight_);
}

// Displays the column segments in some window.
void ColumnFinder::DisplayColSegments(ColSegment_LIST *segments,
                                      ScrollView* win,
                                      ScrollView::Color color) {
#ifndef GRAPHICS_DISABLED
  win->Pen(color);
  win->Brush(ScrollView::NONE);
  ColSegment_IT it(segments);
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    ColSegment *col = it.data();
    TBOX box = col->bounding_box();
    int left_x = box.left();
    int right_x = box.right();
    int top_y = box.top();
    int bottom_y = box.bottom();
    win->Rectangle(left_x, bottom_y, right_x, top_y);
  }
  win->Update();
#endif
}

// Displays the colpartitions using a new coloring on an existing window.
// Note: This method is only for debug purpose during development and
// would not be part of checked in code
void ColumnFinder::DisplayColPartitions(ScrollView* win,
                                        ScrollView::Color default_color) {
#ifndef GRAPHICS_DISABLED
  // Iterate the ColPartitions in the grid.
  GridSearch<ColPartition, ColPartition_CLIST, ColPartition_C_IT>
    gsearch(&clean_part_grid_);
  gsearch.StartFullSearch();
  ColPartition* part;
  win->Brush(ScrollView::NONE);
  ScrollView::Color color;
  while ((part = gsearch.NextFullSearch()) != NULL) {
    color = default_color;
    TBOX box = part->bounding_box();
//     ColPartition* upper_part = part->nearest_neighbor_above();
//     ColPartition* lower_part = part->nearest_neighbor_below();
//     if (!upper_part && !lower_part)
//       color = ScrollView::ORANGE;
//     else if (upper_part && !lower_part)
//       color = ScrollView::CYAN;
//     else if (!upper_part && lower_part)
//       color = ScrollView::MAGENTA;
    if (part->type() == PT_TABLE)
      color = ScrollView::YELLOW;

    int left_x = box.left();
    int right_x = box.right();
    int top_y = box.top();
    int bottom_y = box.bottom();
    win->Pen(color);
    win->Rectangle(left_x, bottom_y, right_x, top_y);
  }
  win->Update();
#endif
}

// Write debug image and text file.
// Note: This method is only for debug purpose during development and
// would not be part of checked in code
void ColumnFinder::WriteToPix() {
#ifdef HAVE_LIBLEPT
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
    box.rotate_large(reskew_);
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
  FILE* fptr = fopen("tess-table.txt", "w");
  GridSearch<ColSegment, ColSegment_CLIST, ColSegment_C_IT>
      table_search(&table_grid_);
  table_search.StartFullSearch();
  ColSegment* table;
  // load table boxes to table_array and write them to text file as well
  while ((table = table_search.NextFullSearch()) != NULL) {
    TBOX box = table->bounding_box();
    box.rotate_large(reskew_);
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
#endif
}

// Merge all colpartitions in table regions to make them a single
// colpartition and revert types of isolated table cells not
// assigned to any table to their original types.
void ColumnFinder::MakeTableBlocks() {
  // Since we have table blocks already, remove table tags from all
  // colpartitions
  GridSearch<ColPartition, ColPartition_CLIST, ColPartition_C_IT>
    gsearch(&part_grid_);
  gsearch.StartFullSearch();
  ColPartition* part;
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
        rectsearch(&part_grid_);
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
          table_partition->Absorb(part, WidthCB());
        } else {
          table_partition = part;
        }
      }
    }
    // Insert table colpartition back to part_grid_
    if (table_partition) {
      table_partition->SetPartitionType(best_columns_[table_search.GridY()]);
      table_partition->set_table_type();
      part_grid_.InsertBBox(true, true, table_partition);
    }
  }
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
