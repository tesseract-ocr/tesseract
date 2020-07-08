///////////////////////////////////////////////////////////////////////
// File:        tablerecog.cpp
// Description: Helper class to help structure table areas. Given an bounding
//              box from TableFinder, the TableRecognizer should give a
//              StructuredTable (maybe a list in the future) of "good" tables
//              in that area.
// Author:      Nicholas Beato
// Created:     Friday, Aug. 20, 2010
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

#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif

#include "tablerecog.h"

#include <algorithm>

namespace tesseract {

// The amount of space required between the ColPartitions in 2 columns
// of a non-lined table as a multiple of the median width.
const double kHorizontalSpacing = 0.30;
// The amount of space required between the ColPartitions in 2 rows
// of a non-lined table as multiples of the median height.
const double kVerticalSpacing = -0.2;
// The number of cells that the grid lines may intersect.
// See FindCellSplitLocations for explanation.
const int kCellSplitRowThreshold = 0;
const int kCellSplitColumnThreshold = 0;
// For "lined tables", the number of required lines. Currently a guess.
const int kLinedTableMinVerticalLines = 3;
const int kLinedTableMinHorizontalLines = 3;
// Number of columns required, as a fraction of the most columns found.
// None of these are tweaked at all.
const double kRequiredColumns = 0.7;
// The tolerance for comparing margins of potential tables.
const double kMarginFactor = 1.1;
// The first and last row should be consistent cell height.
// This factor is the first and last row cell height max.
const double kMaxRowSize = 2.5;
// Number of filled columns required to form a strong table row.
// For small tables, this is an absolute number.
const double kGoodRowNumberOfColumnsSmall[] = { 2, 2, 2, 2, 2, 3, 3 };
const int kGoodRowNumberOfColumnsSmallSize =
    sizeof(kGoodRowNumberOfColumnsSmall) / sizeof(double) - 1;
// For large tables, it is a relative number
const double kGoodRowNumberOfColumnsLarge = 0.7;
// The amount of area that must be covered in a cell by ColPartitions to
// be considered "filled"
const double kMinFilledArea = 0.35;

////////
//////// StructuredTable Class
////////

StructuredTable::StructuredTable()
    : text_grid_(nullptr),
      line_grid_(nullptr),
      is_lined_(false),
      space_above_(0),
      space_below_(0),
      space_left_(0),
      space_right_(0),
      median_cell_height_(0),
      median_cell_width_(0),
      max_text_height_(INT32_MAX) {
}

void StructuredTable::Init() {
}

void StructuredTable::set_text_grid(ColPartitionGrid* text_grid) {
  text_grid_ = text_grid;
}
void StructuredTable::set_line_grid(ColPartitionGrid* line_grid) {
  line_grid_ = line_grid;
}
void StructuredTable::set_max_text_height(int height) {
  max_text_height_ = height;
}
bool StructuredTable::is_lined() const {
  return is_lined_;
}
int StructuredTable::row_count() const {
  return cell_y_.size() == 0 ? 0 : cell_y_.size() - 1;
}
int StructuredTable::column_count() const {
  return cell_x_.size() == 0 ? 0 : cell_x_.size() - 1;
}
int StructuredTable::cell_count() const {
  return row_count() * column_count();
}
void StructuredTable::set_bounding_box(const TBOX& box) {
  bounding_box_ = box;
}
const TBOX& StructuredTable::bounding_box() const {
  return bounding_box_;
}
int StructuredTable::median_cell_height() {
  return median_cell_height_;
}
int StructuredTable::median_cell_width() {
  return median_cell_width_;
}
int StructuredTable::row_height(int row) const {
  ASSERT_HOST(0 <= row && row < row_count());
  return cell_y_[row + 1] - cell_y_[row];
}
int StructuredTable::column_width(int column) const {
  ASSERT_HOST(0 <= column && column < column_count());
  return cell_x_[column + 1] - cell_x_[column];
}
int StructuredTable::space_above() const {
  return space_above_;
}
int StructuredTable::space_below() const {
  return space_below_;
}

// At this point, we know that the lines are contained
// by the box (by FindLinesBoundingBox).
// So try to find the cell structure and make sure it works out.
// The assumption is that all lines span the table. If this
// assumption fails, the VerifyLinedTable method will
// abort the lined table. The TableRecognizer will fall
// back on FindWhitespacedStructure.
bool StructuredTable::FindLinedStructure() {
  ClearStructure();

  // Search for all of the lines in the current box.
  // Update the cellular structure with the exact lines.
  ColPartitionGridSearch box_search(line_grid_);
  box_search.SetUniqueMode(true);
  box_search.StartRectSearch(bounding_box_);
  ColPartition* line = nullptr;

  while ((line = box_search.NextRectSearch()) != nullptr) {
    if (line->IsHorizontalLine())
      cell_y_.push_back(line->MidY());
    if (line->IsVerticalLine())
      cell_x_.push_back(line->MidX());
  }

  // HasSignificantLines should guarantee cells.
  // Because that code is a different class, just gracefully
  // return false. This could be an assert.
  if (cell_x_.size() < 3 || cell_y_.size() < 3)
    return false;

  cell_x_.sort();
  cell_y_.sort();

  // Remove duplicates that may have occurred due to split lines.
  cell_x_.compact_sorted();
  cell_y_.compact_sorted();

  // The border should be the extents of line boxes, not middle.
  cell_x_[0] = bounding_box_.left();
  cell_x_[cell_x_.size() - 1] = bounding_box_.right();
  cell_y_[0] = bounding_box_.bottom();
  cell_y_[cell_y_.size() - 1] = bounding_box_.top();

  // Remove duplicates that may have occurred due to moving the borders.
  cell_x_.compact_sorted();
  cell_y_.compact_sorted();

  CalculateMargins();
  CalculateStats();
  is_lined_ = VerifyLinedTableCells();
  return is_lined_;
}

// Finds the cellular structure given a particular box.
bool StructuredTable::FindWhitespacedStructure() {
  ClearStructure();
  FindWhitespacedColumns();
  FindWhitespacedRows();

  if (!VerifyWhitespacedTable()) {
    return false;
  } else {
    bounding_box_.set_left(cell_x_[0]);
    bounding_box_.set_right(cell_x_[cell_x_.size() - 1]);
    bounding_box_.set_bottom(cell_y_[0]);
    bounding_box_.set_top(cell_y_[cell_y_.size() - 1]);
    AbsorbNearbyLines();
    CalculateMargins();
    CalculateStats();
    return true;
  }
}

// Tests if a partition fits inside the table structure.
// Partitions must fully span a grid line in order to intersect it.
// This means that a partition does not intersect a line
// that it "just" touches. This is mainly because the assumption
// throughout the code is that "0" distance is a very very small space.
bool StructuredTable::DoesPartitionFit(const ColPartition& part) const {
  const TBOX& box = part.bounding_box();
  for (int i = 0; i < cell_x_.size(); ++i)
    if (box.left() < cell_x_[i] && cell_x_[i] < box.right())
      return false;
  for (int i = 0; i < cell_y_.size(); ++i)
    if (box.bottom() < cell_y_[i] && cell_y_[i] < box.top())
      return false;
  return true;
}

// Checks if a sub-table has multiple data cells filled.
int StructuredTable::CountFilledCells() {
  return CountFilledCells(0, row_count() - 1, 0, column_count() - 1);
}
int StructuredTable::CountFilledCellsInRow(int row) {
  return CountFilledCells(row, row, 0, column_count() - 1);
}
int StructuredTable::CountFilledCellsInColumn(int column) {
  return CountFilledCells(0, row_count() - 1, column, column);
}
int StructuredTable::CountFilledCells(int row_start, int row_end,
                            int column_start, int column_end) {
  ASSERT_HOST(0 <= row_start && row_start <= row_end && row_end < row_count());
  ASSERT_HOST(0 <= column_start && column_start <= column_end &&
              column_end < column_count());
  int cell_count = 0;
  TBOX cell_box;
  for (int row = row_start; row <= row_end; ++row) {
    cell_box.set_bottom(cell_y_[row]);
    cell_box.set_top(cell_y_[row + 1]);
    for (int col = column_start; col <= column_end; ++col) {
      cell_box.set_left(cell_x_[col]);
      cell_box.set_right(cell_x_[col + 1]);
      if (CountPartitions(cell_box) > 0)
        ++cell_count;
    }
  }
  return cell_count;
}

// Makes sure that at least one cell in a row has substantial area filled.
// This can filter out large whitespace caused by growing tables too far
// and page numbers.
bool StructuredTable::VerifyRowFilled(int row) {
  for (int i = 0; i < column_count(); ++i) {
    double area_filled = CalculateCellFilledPercentage(row, i);
    if (area_filled >= kMinFilledArea)
      return true;
  }
  return false;
}

// Finds the filled area in a cell.
// Assume ColPartitions do not overlap for simplicity (even though they do).
double StructuredTable::CalculateCellFilledPercentage(int row, int column) {
  ASSERT_HOST(0 <= row && row <= row_count());
  ASSERT_HOST(0 <= column && column <= column_count());
  const TBOX kCellBox(cell_x_[column], cell_y_[row],
                      cell_x_[column + 1], cell_y_[row + 1]);
  ASSERT_HOST(!kCellBox.null_box());

  ColPartitionGridSearch gsearch(text_grid_);
  gsearch.SetUniqueMode(true);
  gsearch.StartRectSearch(kCellBox);
  double area_covered = 0;
  ColPartition* text = nullptr;
  while ((text = gsearch.NextRectSearch()) != nullptr) {
    if (text->IsTextType())
      area_covered += text->bounding_box().intersection(kCellBox).area();
  }
  const int32_t current_area = kCellBox.area();
  if (current_area == 0) {
    return 1.0;
  }
  return std::min(1.0, area_covered / current_area);
}

#ifndef GRAPHICS_DISABLED

void StructuredTable::Display(ScrollView* window, ScrollView::Color color) {
  window->Brush(ScrollView::NONE);
  window->Pen(color);
  window->Rectangle(bounding_box_.left(), bounding_box_.bottom(),
                    bounding_box_.right(), bounding_box_.top());
  for (int i = 0; i < cell_x_.size(); i++) {
    window->Line(cell_x_[i], bounding_box_.bottom(),
                 cell_x_[i], bounding_box_.top());
  }
  for (int i = 0; i < cell_y_.size(); i++) {
    window->Line(bounding_box_.left(), cell_y_[i],
                 bounding_box_.right(), cell_y_[i]);
  }
  window->UpdateWindow();
}

#endif

// Clear structure information.
void StructuredTable::ClearStructure() {
  cell_x_.clear();
  cell_y_.clear();
  is_lined_ = false;
  space_above_ = 0;
  space_below_ = 0;
  space_left_ = 0;
  space_right_ = 0;
  median_cell_height_ = 0;
  median_cell_width_ = 0;
}

// When a table has lines, the lines should not intersect any partitions.
// The following function makes sure the previous assumption is met.
bool StructuredTable::VerifyLinedTableCells() {
  // Function only called when lines exist.
  ASSERT_HOST(cell_y_.size() >= 2 && cell_x_.size() >= 2);
  for (int i = 0; i < cell_y_.size(); ++i) {
    if (CountHorizontalIntersections(cell_y_[i]) > 0)
      return false;
  }
  for (int i = 0; i < cell_x_.size(); ++i) {
    if (CountVerticalIntersections(cell_x_[i]) > 0)
      return false;
  }
  return true;
}

// TODO(nbeato): Could be much better than this.
// Examples:
//   - Caclulate the percentage of filled cells.
//   - Calculate the average number of ColPartitions per cell.
//   - Calculate the number of cells per row with partitions.
//   - Check if ColPartitions in adjacent cells are similar.
//   - Check that all columns are at least a certain width.
//   - etc.
bool StructuredTable::VerifyWhitespacedTable() {
  // criteria for a table, must be at least 2x3 or 3x2
  return row_count() >= 2 && column_count() >= 2 && cell_count() >= 6;
}

// Finds vertical splits in the ColPartitions of text_grid_ by considering
// all possible "good" guesses. A good guess is just the left/right sides of
// the partitions, since these locations will uniquely define where the
// extremal values where the splits can occur. The split happens
// in the middle of the two nearest partitions.
void StructuredTable::FindWhitespacedColumns() {
  // Set of the extents of all partitions on the page.
  GenericVectorEqEq<int> left_sides;
  GenericVectorEqEq<int> right_sides;

  // Look at each text partition. We want to find the partitions
  // that have extremal left/right sides. These will give us a basis
  // for the table columns.
  ColPartitionGridSearch gsearch(text_grid_);
  gsearch.SetUniqueMode(true);
  gsearch.StartRectSearch(bounding_box_);
  ColPartition* text = nullptr;
  while ((text = gsearch.NextRectSearch()) != nullptr) {
    if (!text->IsTextType())
      continue;

    ASSERT_HOST(text->bounding_box().left() < text->bounding_box().right());
    int spacing = static_cast<int>(text->median_width() *
                                   kHorizontalSpacing / 2.0 + 0.5);
    left_sides.push_back(text->bounding_box().left() - spacing);
    right_sides.push_back(text->bounding_box().right() + spacing);
  }
  // It causes disaster below, so avoid it!
  if (left_sides.size() == 0 || right_sides.size() == 0)
    return;

  // Since data may be inserted in grid order, we sort the left/right sides.
  left_sides.sort();
  right_sides.sort();

  // At this point, in the "merged list", we expect to have a left side,
  // followed by either more left sides or a right side. The last number
  // should be a right side. We find places where the splits occur by looking
  // for "valleys". If we want to force gap sizes or allow overlap, change
  // the spacing above. If you want to let lines "slice" partitions as long
  // as it is infrequent, change the following function.
  FindCellSplitLocations(left_sides, right_sides, kCellSplitColumnThreshold,
                         &cell_x_);
}

// Finds horizontal splits in the ColPartitions of text_grid_ by considering
// all possible "good" guesses. A good guess is just the bottom/top sides of
// the partitions, since these locations will uniquely define where the
// extremal values where the splits can occur. The split happens
// in the middle of the two nearest partitions.
void StructuredTable::FindWhitespacedRows() {
  // Set of the extents of all partitions on the page.
  GenericVectorEqEq<int> bottom_sides;
  GenericVectorEqEq<int> top_sides;
  // We will be "shrinking" partitions, so keep the min/max around to
  // make sure the bottom/top lines do not intersect text.
  int min_bottom = INT32_MAX;
  int max_top = INT32_MIN;

  // Look at each text partition. We want to find the partitions
  // that have extremal bottom/top sides. These will give us a basis
  // for the table rows. Because the textlines can be skewed and close due
  // to warping, the height of the partitions is toned down a little bit.
  ColPartitionGridSearch gsearch(text_grid_);
  gsearch.SetUniqueMode(true);
  gsearch.StartRectSearch(bounding_box_);
  ColPartition* text = nullptr;
  while ((text = gsearch.NextRectSearch()) != nullptr) {
    if (!text->IsTextType())
      continue;

    ASSERT_HOST(text->bounding_box().bottom() < text->bounding_box().top());
    min_bottom = std::min(min_bottom, static_cast<int>(text->bounding_box().bottom()));
    max_top = std::max(max_top, static_cast<int>(text->bounding_box().top()));

    // Ignore "tall" text partitions, as these are usually false positive
    // vertical text or multiple lines pulled together.
    if (text->bounding_box().height() > max_text_height_)
      continue;

    int spacing = static_cast<int>(text->bounding_box().height() *
                                   kVerticalSpacing / 2.0 + 0.5);
    int bottom = text->bounding_box().bottom() - spacing;
    int top = text->bounding_box().top() + spacing;
    // For horizontal text, the factor can be negative. This should
    // probably cause a warning or failure. I haven't actually checked if
    // it happens.
    if (bottom >= top)
      continue;

    bottom_sides.push_back(bottom);
    top_sides.push_back(top);
  }
  // It causes disaster below, so avoid it!
  if (bottom_sides.size() == 0 || top_sides.size() == 0)
    return;

  // Since data may be inserted in grid order, we sort the bottom/top sides.
  bottom_sides.sort();
  top_sides.sort();

  // At this point, in the "merged list", we expect to have a bottom side,
  // followed by either more bottom sides or a top side. The last number
  // should be a top side. We find places where the splits occur by looking
  // for "valleys". If we want to force gap sizes or allow overlap, change
  // the spacing above. If you want to let lines "slice" partitions as long
  // as it is infrequent, change the following function.
  FindCellSplitLocations(bottom_sides, top_sides, kCellSplitRowThreshold,
                         &cell_y_);

  // Recover the min/max correctly since it was shifted.
  cell_y_[0] = min_bottom;
  cell_y_[cell_y_.size() - 1] = max_top;
}

void StructuredTable::CalculateMargins() {
  space_above_ = INT32_MAX;
  space_below_ = INT32_MAX;
  space_right_ = INT32_MAX;
  space_left_ = INT32_MAX;
  UpdateMargins(text_grid_);
  UpdateMargins(line_grid_);
}
// Finds the nearest partition in grid to the table
// boundaries and updates the margin.
void StructuredTable::UpdateMargins(ColPartitionGrid* grid) {
  int below = FindVerticalMargin(grid, bounding_box_.bottom(), true);
  space_below_ = std::min(space_below_, below);
  int above = FindVerticalMargin(grid, bounding_box_.top(), false);
  space_above_ = std::min(space_above_, above);
  int left = FindHorizontalMargin(grid, bounding_box_.left(), true);
  space_left_ = std::min(space_left_, left);
  int right = FindHorizontalMargin(grid, bounding_box_.right(), false);
  space_right_ = std::min(space_right_, right);
}
int StructuredTable::FindVerticalMargin(ColPartitionGrid* grid, int border,
                                        bool decrease) const {
  ColPartitionGridSearch gsearch(grid);
  gsearch.SetUniqueMode(true);
  gsearch.StartVerticalSearch(bounding_box_.left(), bounding_box_.right(),
                              border);
  ColPartition* part = nullptr;
  while ((part = gsearch.NextVerticalSearch(decrease)) != nullptr) {
    if (!part->IsTextType() && !part->IsHorizontalLine())
      continue;
    int distance = decrease ? border - part->bounding_box().top()
                            : part->bounding_box().bottom() - border;
    if (distance >= 0)
      return distance;
  }
  return INT32_MAX;
}
int StructuredTable::FindHorizontalMargin(ColPartitionGrid* grid, int border,
                                          bool decrease) const {
  ColPartitionGridSearch gsearch(grid);
  gsearch.SetUniqueMode(true);
  gsearch.StartSideSearch(border, bounding_box_.bottom(), bounding_box_.top());
  ColPartition* part = nullptr;
  while ((part = gsearch.NextSideSearch(decrease)) != nullptr) {
    if (!part->IsTextType() && !part->IsVerticalLine())
      continue;
    int distance = decrease ? border - part->bounding_box().right()
                            : part->bounding_box().left() - border;
    if (distance >= 0)
      return distance;
  }
  return INT32_MAX;
}

void StructuredTable::CalculateStats() {
  const int kMaxCellHeight = 1000;
  const int kMaxCellWidth = 1000;
  STATS height_stats(0, kMaxCellHeight + 1);
  STATS width_stats(0, kMaxCellWidth + 1);

  for (int i = 0; i < row_count(); ++i)
    height_stats.add(row_height(i), column_count());
  for (int i = 0; i < column_count(); ++i)
    width_stats.add(column_width(i), row_count());

  median_cell_height_ = static_cast<int>(height_stats.median() + 0.5);
  median_cell_width_ = static_cast<int>(width_stats.median() + 0.5);
}

// Looks for grid lines near the current bounding box and
// grows the bounding box to include them if no intersections
// will occur as a result. This is necessary because the margins
// are calculated relative to the closest line/text. If the
// line isn't absorbed, the margin will be the distance to the line.
void StructuredTable::AbsorbNearbyLines() {
  ColPartitionGridSearch gsearch(line_grid_);
  gsearch.SetUniqueMode(true);

  // Is the closest line above good? Loop multiple times for tables with
  // multi-line (sometimes 2) borders. Limit the number of lines by
  // making sure they stay within a table cell or so.
  ColPartition* line = nullptr;
  gsearch.StartVerticalSearch(bounding_box_.left(), bounding_box_.right(),
                              bounding_box_.top());
  while ((line = gsearch.NextVerticalSearch(false)) != nullptr) {
    if (!line->IsHorizontalLine())
      break;
    TBOX text_search(bounding_box_.left(), bounding_box_.top() + 1,
                     bounding_box_.right(), line->MidY());
    if (text_search.height() > median_cell_height_ * 2)
      break;
    if (CountPartitions(text_search) > 0)
      break;
    bounding_box_.set_top(line->MidY());
  }
  // As above, is the closest line below good?
  line = nullptr;
  gsearch.StartVerticalSearch(bounding_box_.left(), bounding_box_.right(),
                              bounding_box_.bottom());
  while ((line = gsearch.NextVerticalSearch(true)) != nullptr) {
    if (!line->IsHorizontalLine())
      break;
    TBOX text_search(bounding_box_.left(), line->MidY(),
                     bounding_box_.right(), bounding_box_.bottom() - 1);
    if (text_search.height() > median_cell_height_ * 2)
      break;
    if (CountPartitions(text_search) > 0)
      break;
    bounding_box_.set_bottom(line->MidY());
  }
  // TODO(nbeato): vertical lines
}


// This function will find all "0 valleys" (of any length) given two
// arrays. The arrays are the mins and maxes of partitions (either
// left and right or bottom and top). Since the min/max lists are generated
// with pairs of increasing integers, we can make some assumptions in
// the function about ordering of the overall list, which are shown in the
// asserts.
// The algorithm works as follows:
//   While there are numbers to process, take the smallest number.
//     If it is from the min_list, increment the "hill" counter.
//     Otherwise, decrement the "hill" counter.
//     In the process of doing this, keep track of "crossing" the
//     desired height.
// The first/last items are extremal values of the list and known.
// NOTE: This function assumes the lists are sorted!
void StructuredTable::FindCellSplitLocations(const GenericVector<int>& min_list,
                                             const GenericVector<int>& max_list,
                                             int max_merged,
                                             GenericVector<int>* locations) {
  locations->clear();
  ASSERT_HOST(min_list.size() == max_list.size());
  if (min_list.size() == 0)
    return;
  ASSERT_HOST(min_list.get(0) < max_list.get(0));
  ASSERT_HOST(min_list.get(min_list.size() - 1) <
              max_list.get(max_list.size() - 1));

  locations->push_back(min_list.get(0));
  int min_index = 0;
  int max_index = 0;
  int stacked_partitions = 0;
  int last_cross_position = INT32_MAX;
  // max_index will expire after min_index.
  // However, we can't "increase" the hill size if min_index expired.
  // So finish processing when min_index expires.
  while (min_index < min_list.size()) {
    // Increase the hill count.
    if (min_list[min_index] < max_list[max_index]) {
      ++stacked_partitions;
      if (last_cross_position != INT32_MAX &&
          stacked_partitions > max_merged) {
        int mid = (last_cross_position + min_list[min_index]) / 2;
        locations->push_back(mid);
        last_cross_position = INT32_MAX;
      }
      ++min_index;
    } else {
      // Decrease the hill count.
      --stacked_partitions;
      if (last_cross_position == INT32_MAX &&
          stacked_partitions <= max_merged) {
        last_cross_position = max_list[max_index];
      }
      ++max_index;
    }
  }
  locations->push_back(max_list.get(max_list.size() - 1));
}

// Counts the number of partitions in the table
// box that intersection the given x value.
int StructuredTable::CountVerticalIntersections(int x) {
  int count = 0;
  // Make a small box to keep the search time down.
  const int kGridSize = text_grid_->gridsize();
  TBOX vertical_box = bounding_box_;
  vertical_box.set_left(x - kGridSize);
  vertical_box.set_right(x + kGridSize);

  ColPartitionGridSearch gsearch(text_grid_);
  gsearch.SetUniqueMode(true);
  gsearch.StartRectSearch(vertical_box);
  ColPartition* text = nullptr;
  while ((text = gsearch.NextRectSearch()) != nullptr) {
    if (!text->IsTextType())
      continue;
    const TBOX& box = text->bounding_box();
    if (box.left() < x && x < box.right())
      ++count;
  }
  return count;
}

// Counts the number of partitions in the table
// box that intersection the given y value.
int StructuredTable::CountHorizontalIntersections(int y) {
  int count = 0;
  // Make a small box to keep the search time down.
  const int kGridSize = text_grid_->gridsize();
  TBOX horizontal_box = bounding_box_;
  horizontal_box.set_bottom(y - kGridSize);
  horizontal_box.set_top(y + kGridSize);

  ColPartitionGridSearch gsearch(text_grid_);
  gsearch.SetUniqueMode(true);
  gsearch.StartRectSearch(horizontal_box);
  ColPartition* text = nullptr;
  while ((text = gsearch.NextRectSearch()) != nullptr) {
    if (!text->IsTextType())
      continue;

    const TBOX& box = text->bounding_box();
    if (box.bottom() < y && y < box.top())
      ++count;
  }
  return count;
}

// Counts how many text partitions are in this box.
// This is used to count partitons in cells, as that can indicate
// how "strong" a potential table row/column (or even full table) actually is.
int StructuredTable::CountPartitions(const TBOX& box) {
  ColPartitionGridSearch gsearch(text_grid_);
  gsearch.SetUniqueMode(true);
  gsearch.StartRectSearch(box);
  int count = 0;
  ColPartition* text = nullptr;
  while ((text = gsearch.NextRectSearch()) != nullptr) {
    if (text->IsTextType())
      ++count;
  }
  return count;
}

////////
//////// TableRecognizer Class
////////

TableRecognizer::TableRecognizer()
    : text_grid_(nullptr),
      line_grid_(nullptr),
      min_height_(0),
      min_width_(0),
      max_text_height_(INT32_MAX) {
}

TableRecognizer::~TableRecognizer() {
}

void TableRecognizer::Init() {
}

void TableRecognizer::set_text_grid(ColPartitionGrid* text_grid) {
  text_grid_ = text_grid;
}
void TableRecognizer::set_line_grid(ColPartitionGrid* line_grid) {
  line_grid_ = line_grid;
}
void TableRecognizer::set_min_height(int height) {
  min_height_ = height;
}
void TableRecognizer::set_min_width(int width) {
  min_width_ = width;
}
void TableRecognizer::set_max_text_height(int height) {
  max_text_height_ = height;
}

StructuredTable* TableRecognizer::RecognizeTable(const TBOX& guess) {
  auto* table = new StructuredTable();
  table->Init();
  table->set_text_grid(text_grid_);
  table->set_line_grid(line_grid_);
  table->set_max_text_height(max_text_height_);

  // Try to solve this simple case, a table with *both*
  // vertical and horizontal lines.
  if (RecognizeLinedTable(guess, table))
    return table;

  // Fallback to whitespace if that failed.
  // TODO(nbeato): Break this apart to take advantage of horizontal
  // lines or vertical lines when present.
  if (RecognizeWhitespacedTable(guess, table))
    return table;

  // No table found...
  delete table;
  return nullptr;
}

bool TableRecognizer::RecognizeLinedTable(const TBOX& guess_box,
                                          StructuredTable* table) {
  if (!HasSignificantLines(guess_box))
    return false;
  TBOX line_bound = guess_box;
  if (!FindLinesBoundingBox(&line_bound))
    return false;
  table->set_bounding_box(line_bound);
  return table->FindLinedStructure();
}

// Quick implementation. Just count the number of lines in the box.
// A better implementation would counter intersections and look for connected
// components. It could even go as far as finding similar length lines.
// To account for these possible issues, the VerifyLinedTableCells function
// will reject lined tables that cause intersections with text on the page.
// TODO(nbeato): look for "better" lines
bool TableRecognizer::HasSignificantLines(const TBOX& guess) {
  ColPartitionGridSearch box_search(line_grid_);
  box_search.SetUniqueMode(true);
  box_search.StartRectSearch(guess);
  ColPartition* line = nullptr;
  int vertical_count = 0;
  int horizontal_count = 0;

  while ((line = box_search.NextRectSearch()) != nullptr) {
    if (line->IsHorizontalLine())
      ++horizontal_count;
    if (line->IsVerticalLine())
      ++vertical_count;
  }

  return vertical_count >= kLinedTableMinVerticalLines &&
         horizontal_count >= kLinedTableMinHorizontalLines;
}

// Given a bounding box with a bunch of horizontal / vertical lines,
// we just find the extents of all of these lines iteratively.
// The box will be at least as large as guess. This
// could possibly be a bad assumption.
// It is guaranteed to halt in at least O(n * gridarea) where n
// is the number of lines.
// The assumption is that growing the box iteratively will add lines
// several times, but eventually we'll find the extents.
//
// For tables, the approach is a bit aggressive, a single line (which could be
// noise or a column ruling) can destroy the table inside.
//
// TODO(nbeato): This is a quick first implementation.
// A better implementation would actually look for consistency
// in extents of the lines and find the extents using lines
// that clearly describe the table. This would allow the
// lines to "vote" for height/width. An approach like
// this would solve issues with page layout rulings.
// I haven't looked for these issues yet, so I can't even
// say they happen confidently.
bool TableRecognizer::FindLinesBoundingBox(TBOX* bounding_box) {
  // The first iteration will tell us if there are lines
  // present and shrink the box to a minimal iterative size.
  if (!FindLinesBoundingBoxIteration(bounding_box))
    return false;

  // Keep growing until the area of the table stabilizes.
  // The box can only get bigger, increasing area.
  bool changed = true;
  while (changed) {
    changed = false;
    int old_area = bounding_box->area();
    bool check = FindLinesBoundingBoxIteration(bounding_box);
    // At this point, the function will return true.
    ASSERT_HOST(check);
    ASSERT_HOST(bounding_box->area() >= old_area);
    changed = (bounding_box->area() > old_area);
  }

  return true;
}

bool TableRecognizer::FindLinesBoundingBoxIteration(TBOX* bounding_box) {
  // Search for all of the lines in the current box, keeping track of extents.
  ColPartitionGridSearch box_search(line_grid_);
  box_search.SetUniqueMode(true);
  box_search.StartRectSearch(*bounding_box);
  ColPartition* line = nullptr;
  bool first_line = true;

  while ((line = box_search.NextRectSearch()) != nullptr) {
    if (line->IsLineType()) {
      if (first_line) {
        // The first iteration can shrink the box.
        *bounding_box = line->bounding_box();
        first_line = false;
      } else {
        *bounding_box += line->bounding_box();
      }
    }
  }
  return !first_line;
}

// The goal of this function is to move the table boundaries around and find
// a table that maximizes the whitespace around the table while maximizing
// the cellular structure. As a result, it gets confused by headers, footers,
// and merged columns (text that crosses columns). There is a tolerance
// that allows a few partitions to count towards potential cell merges.
// It's the max_merged parameter to FindPartitionLocations.
// It can work, but it needs some false positive remove on boundaries.
// For now, the grid structure must not intersect any partitions.
// Also, small tolerance is added to the horizontal lines for tightly packed
// tables. The tolerance is added by adjusting the bounding boxes of the
// partitions (in FindHorizontalPartitions). The current implementation
// only adjusts the vertical extents of the table.
//
// Also note. This was hacked at a lot. It could probably use some
// more hacking at to find a good set of border conditions and then a
// nice clean up.
bool TableRecognizer::RecognizeWhitespacedTable(const TBOX& guess_box,
                                                StructuredTable* table) {
  TBOX best_box = guess_box;  // Best borders known.
  int best_below = 0;         // Margin size above best table.
  int best_above = 0;         // Margin size below best table.
  TBOX adjusted = guess_box;  // The search box.

  // We assume that the guess box is somewhat accurate, so we don't allow
  // the adjusted border to pass half of the guessed area. This prevents
  // "negative" tables from forming.
  const int kMidGuessY = (guess_box.bottom() + guess_box.top()) / 2;
  // Keeps track of the most columns in an accepted table. The resulting table
  // may be less than the max, but we don't want to stray too far.
  int best_cols = 0;
  // Make sure we find a good border.
  bool found_good_border = false;

  // Find the bottom of the table by trying a few different locations. For
  // each location, the top, left, and right are fixed. We start the search
  // in a smaller table to favor best_cols getting a good estimate sooner.
  int last_bottom = INT32_MAX;
  int bottom = NextHorizontalSplit(guess_box.left(), guess_box.right(),
                                   kMidGuessY - min_height_ / 2, true);
  int top = NextHorizontalSplit(guess_box.left(), guess_box.right(),
                                kMidGuessY + min_height_ / 2, false);
  adjusted.set_top(top);

  // Headers/footers can be spaced far from everything.
  // Make sure that the space below is greater than the space above
  // the lowest row.
  int previous_below = 0;
  const int kMaxChances = 10;
  int chances = kMaxChances;
  while (bottom != last_bottom) {
    adjusted.set_bottom(bottom);

    if (adjusted.height() >= min_height_) {
      // Try to fit the grid on the current box. We give it a chance
      // if the number of columns didn't significantly drop.
      table->set_bounding_box(adjusted);
      if (table->FindWhitespacedStructure() &&
          table->column_count() >= best_cols * kRequiredColumns) {
        if (false && IsWeakTableRow(table, 0)) {
          // Currently buggy, but was looking promising so disabled.
          --chances;
        } else {
          // We favor 2 things,
          //   1- Adding rows that have partitioned data.
          //   2- Better margins (to find header/footer).
          // For better tables, we just look for multiple cells in the
          // bottom row with data in them.
          // For margins, the space below the last row should
          // be better than a table with the last row removed.
          chances = kMaxChances;
          double max_row_height = kMaxRowSize * table->median_cell_height();
          if ((table->space_below() * kMarginFactor >= best_below &&
               table->space_below() >= previous_below) ||
              (table->CountFilledCellsInRow(0) > 1 &&
               table->row_height(0) < max_row_height)) {
            best_box.set_bottom(bottom);
            best_below = table->space_below();
            best_cols = std::max(table->column_count(), best_cols);
            found_good_border = true;
          }
        }
        previous_below = table->space_below();
      } else {
       --chances;
      }
    }
    if (chances <= 0)
      break;

    last_bottom = bottom;
    bottom = NextHorizontalSplit(guess_box.left(), guess_box.right(),
                                 last_bottom, true);
  }
  if (!found_good_border)
    return false;

  // TODO(nbeato) comments: follow modified code above... put it in a function!
  found_good_border = false;
  int last_top = INT32_MIN;
  top = NextHorizontalSplit(guess_box.left(), guess_box.right(),
                            kMidGuessY + min_height_ / 2, false);
  int previous_above = 0;
  chances = kMaxChances;

  adjusted.set_bottom(best_box.bottom());
  while (last_top != top) {
    adjusted.set_top(top);
    if (adjusted.height() >= min_height_) {
      table->set_bounding_box(adjusted);
      if (table->FindWhitespacedStructure() &&
          table->column_count() >= best_cols * kRequiredColumns) {
        int last_row = table->row_count() - 1;
        if (false && IsWeakTableRow(table, last_row)) {
          // Currently buggy, but was looking promising so disabled.
          --chances;
        } else {
          chances = kMaxChances;
          double max_row_height = kMaxRowSize * table->median_cell_height();
          if ((table->space_above() * kMarginFactor >= best_above &&
               table->space_above() >= previous_above) ||
              (table->CountFilledCellsInRow(last_row) > 1 &&
               table->row_height(last_row) < max_row_height)) {
            best_box.set_top(top);
            best_above = table->space_above();
            best_cols = std::max(table->column_count(), best_cols);
            found_good_border = true;
          }
        }
        previous_above = table->space_above();
      } else {
       --chances;
      }
    }
    if (chances <= 0)
      break;

    last_top = top;
    top = NextHorizontalSplit(guess_box.left(), guess_box.right(),
                              last_top, false);
  }

  if (!found_good_border)
    return false;

  // If we get here, this shouldn't happen. It can be an assert, but
  // I haven't tested it enough to make it crash things.
  if (best_box.null_box())
    return false;

  // Given the best locations, fit the box to those locations.
  table->set_bounding_box(best_box);
  return table->FindWhitespacedStructure();
}

// Finds the closest value to y that can safely cause a horizontal
// split in the partitions.
// This function has been buggy and not as reliable as I would've
// liked. I suggest finding all of the splits using the
// FindPartitionLocations once and then just keeping the results
// of that function cached somewhere.
int TableRecognizer::NextHorizontalSplit(int left, int right, int y,
                                         bool top_to_bottom) {
  ColPartitionGridSearch gsearch(text_grid_);
  gsearch.SetUniqueMode(true);
  gsearch.StartVerticalSearch(left, right, y);
  ColPartition* text = nullptr;
  int last_y = y;
  while ((text = gsearch.NextVerticalSearch(top_to_bottom)) != nullptr) {
    if (!text->IsTextType() || !text->IsHorizontalType())
      continue;
    if (text->bounding_box().height() > max_text_height_)
      continue;

    const TBOX& text_box = text->bounding_box();
    if (top_to_bottom && (last_y >= y || last_y <= text_box.top())) {
      last_y = std::min(last_y, static_cast<int>(text_box.bottom()));
      continue;
    }
    if (!top_to_bottom && (last_y <= y || last_y >= text_box.bottom())) {
      last_y = std::max(last_y, static_cast<int>(text_box.top()));
      continue;
    }

    return last_y;
  }
  // If none is found, we at least want to preserve the min/max,
  // which defines the overlap of y with the last partition in the grid.
  return last_y;
}

// Code is buggy right now. It is disabled in the calling function.
// It seems like sometimes the row that is passed in is not correct
// sometimes (like a phantom row is introduced). There's something going
// on in the cell_y_ data member before this is called... not certain.
bool TableRecognizer::IsWeakTableRow(StructuredTable* table, int row) {
  if (!table->VerifyRowFilled(row))
    return false;

  double threshold = 0.0;
  if (table->column_count() > kGoodRowNumberOfColumnsSmallSize)
    threshold = table->column_count() * kGoodRowNumberOfColumnsLarge;
  else
    threshold = kGoodRowNumberOfColumnsSmall[table->column_count()];

  return table->CountFilledCellsInRow(row) < threshold;
}

}  // namespace tesseract
