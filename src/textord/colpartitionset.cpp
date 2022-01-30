///////////////////////////////////////////////////////////////////////
// File:        colpartitionset.cpp
// Description: Class to hold a list of ColPartitions of the page that
//              correspond roughly to columns.
// Author:      Ray Smith
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

#ifdef HAVE_CONFIG_H
#  include "config_auto.h"
#endif

#include "colpartitionset.h"
#include "tablefind.h"
#include "workingpartset.h"

namespace tesseract {

// Minimum width of a column to be interesting as a multiple of resolution.
const double kMinColumnWidth = 2.0 / 3;

ColPartitionSet::ColPartitionSet(ColPartition_LIST *partitions) {
  ColPartition_IT it(&parts_);
  it.add_list_after(partitions);
  ComputeCoverage();
}

ColPartitionSet::ColPartitionSet(ColPartition *part) {
  ColPartition_IT it(&parts_);
  it.add_after_then_move(part);
  ComputeCoverage();
}

// Returns the number of columns of good width.
int ColPartitionSet::GoodColumnCount() const {
  int num_good_cols = 0;
  // This is a read-only iteration of the list.
  ColPartition_IT it(const_cast<ColPartition_LIST *>(&parts_));
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    if (it.data()->good_width()) {
      ++num_good_cols;
    }
  }
  return num_good_cols;
}

// Return an element of the parts_ list from its index.
ColPartition *ColPartitionSet::GetColumnByIndex(int index) {
  ColPartition_IT it(&parts_);
  it.mark_cycle_pt();
  for (int i = 0; i < index && !it.cycled_list(); ++i, it.forward()) {
    ;
  }
  if (it.cycled_list()) {
    return nullptr;
  }
  return it.data();
}

// Return the ColPartition that contains the given coords, if any, else nullptr.
ColPartition *ColPartitionSet::ColumnContaining(int x, int y) {
  ColPartition_IT it(&parts_);
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    ColPartition *part = it.data();
    if (part->ColumnContains(x, y)) {
      return part;
    }
  }
  return nullptr;
}

// Extract all the parts from the list, relinquishing ownership.
void ColPartitionSet::RelinquishParts() {
  ColPartition_IT it(&parts_);
  while (!it.empty()) {
    it.extract();
    it.forward();
  }
}

// Attempt to improve this by adding partitions or expanding partitions.
void ColPartitionSet::ImproveColumnCandidate(const WidthCallback &cb,
                                             PartSetVector *src_sets) {
  int set_size = src_sets->size();
  // Iterate over the provided column sets, as each one may have something
  // to improve this.
  for (int i = 0; i < set_size; ++i) {
    ColPartitionSet *column_set = src_sets->at(i);
    if (column_set == nullptr) {
      continue;
    }
    // Iterate over the parts in this and column_set, adding bigger or
    // new parts in column_set to this.
    ColPartition_IT part_it(&parts_);
    ASSERT_HOST(!part_it.empty());
    int prev_right = INT32_MIN;
    part_it.mark_cycle_pt();
    ColPartition_IT col_it(&column_set->parts_);
    for (col_it.mark_cycle_pt(); !col_it.cycled_list(); col_it.forward()) {
      ColPartition *col_part = col_it.data();
      if (col_part->blob_type() < BRT_UNKNOWN) {
        continue; // Ignore image partitions.
      }
      int col_left = col_part->left_key();
      int col_right = col_part->right_key();
      // Sync-up part_it (in this) so it matches the col_part in column_set.
      ColPartition *part = part_it.data();
      while (!part_it.at_last() && part->right_key() < col_left) {
        prev_right = part->right_key();
        part_it.forward();
        part = part_it.data();
      }
      int part_left = part->left_key();
      int part_right = part->right_key();
      if (part_right < col_left || col_right < part_left) {
        // There is no overlap so this is a new partition.
        AddPartition(col_part->ShallowCopy(), &part_it);
        continue;
      }
      // Check the edges of col_part to see if they can improve part.
      bool part_width_ok = cb(part->KeyWidth(part_left, part_right));
      if (col_left < part_left && col_left > prev_right) {
        // The left edge of the column is better and it doesn't overlap,
        // so we can potentially expand it.
        int col_box_left = col_part->BoxLeftKey();
        bool tab_width_ok = cb(part->KeyWidth(col_left, part_right));
        bool box_width_ok = cb(part->KeyWidth(col_box_left, part_right));
        if (tab_width_ok || (!part_width_ok)) {
          // The tab is leaving the good column metric at least as good as
          // it was before, so use the tab.
          part->CopyLeftTab(*col_part, false);
          part->SetColumnGoodness(cb);
        } else if (col_box_left < part_left &&
                   (box_width_ok || !part_width_ok)) {
          // The box is leaving the good column metric at least as good as
          // it was before, so use the box.
          part->CopyLeftTab(*col_part, true);
          part->SetColumnGoodness(cb);
        }
        part_left = part->left_key();
      }
      if (col_right > part_right &&
          (part_it.at_last() ||
           part_it.data_relative(1)->left_key() > col_right)) {
        // The right edge is better, so we can possibly expand it.
        int col_box_right = col_part->BoxRightKey();
        bool tab_width_ok = cb(part->KeyWidth(part_left, col_right));
        bool box_width_ok = cb(part->KeyWidth(part_left, col_box_right));
        if (tab_width_ok || (!part_width_ok)) {
          // The tab is leaving the good column metric at least as good as
          // it was before, so use the tab.
          part->CopyRightTab(*col_part, false);
          part->SetColumnGoodness(cb);
        } else if (col_box_right > part_right &&
                   (box_width_ok || !part_width_ok)) {
          // The box is leaving the good column metric at least as good as
          // it was before, so use the box.
          part->CopyRightTab(*col_part, true);
          part->SetColumnGoodness(cb);
        }
      }
    }
  }
  ComputeCoverage();
}

// If this set is good enough to represent a new partitioning into columns,
// add it to the vector of sets, otherwise delete it.
void ColPartitionSet::AddToColumnSetsIfUnique(PartSetVector *column_sets,
                                              const WidthCallback &cb) {
  bool debug = TabFind::WithinTestRegion(2, bounding_box_.left(),
                                         bounding_box_.bottom());
  if (debug) {
    tprintf("Considering new column candidate:\n");
    Print();
  }
  if (!LegalColumnCandidate()) {
    if (debug) {
      tprintf("Not a legal column candidate:\n");
      Print();
    }
    delete this;
    return;
  }
  for (unsigned i = 0; i < column_sets->size(); ++i) {
    ColPartitionSet *columns = column_sets->at(i);
    // In ordering the column set candidates, good_coverage_ is king,
    // followed by good_column_count_ and then bad_coverage_.
    bool better = good_coverage_ > columns->good_coverage_;
    if (good_coverage_ == columns->good_coverage_) {
      better = good_column_count_ > columns->good_column_count_;
      if (good_column_count_ == columns->good_column_count_) {
        better = bad_coverage_ > columns->bad_coverage_;
      }
    }
    if (better) {
      // The new one is better so add it.
      if (debug) {
        tprintf("Good one\n");
      }
      column_sets->insert(column_sets->begin() + i, this);
      return;
    }
    if (columns->CompatibleColumns(false, this, cb)) {
      if (debug) {
        tprintf("Duplicate\n");
      }
      delete this;
      return; // It is not unique.
    }
  }
  if (debug) {
    tprintf("Added to end\n");
  }
  column_sets->push_back(this);
}

// Return true if the partitions in other are all compatible with the columns
// in this.
bool ColPartitionSet::CompatibleColumns(bool debug, ColPartitionSet *other,
                                        const WidthCallback &cb) {
  if (debug) {
    tprintf("CompatibleColumns testing compatibility\n");
    Print();
    other->Print();
  }
  if (other->parts_.empty()) {
    if (debug) {
      tprintf("CompatibleColumns true due to empty other\n");
    }
    return true;
  }
  ColPartition_IT it(&other->parts_);
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    ColPartition *part = it.data();
    if (part->blob_type() < BRT_UNKNOWN) {
      if (debug) {
        tprintf("CompatibleColumns ignoring image partition\n");
        part->Print();
      }
      continue; // Image partitions are irrelevant to column compatibility.
    }
    int y = part->MidY();
    int left = part->bounding_box().left();
    int right = part->bounding_box().right();
    ColPartition *left_col = ColumnContaining(left, y);
    ColPartition *right_col = ColumnContaining(right, y);
    if (right_col == nullptr || left_col == nullptr) {
      if (debug) {
        tprintf("CompatibleColumns false due to partition edge outside\n");
        part->Print();
      }
      return false; // A partition edge lies outside of all columns
    }
    if (right_col != left_col && cb(right - left)) {
      if (debug) {
        tprintf("CompatibleColumns false due to good width in multiple cols\n");
        part->Print();
      }
      return false; // Partition with a good width must be in a single column.
    }

    ColPartition_IT it2 = it;
    while (!it2.at_last()) {
      it2.forward();
      ColPartition *next_part = it2.data();
      if (!BLOBNBOX::IsTextType(next_part->blob_type())) {
        continue; // Non-text partitions are irrelevant.
      }
      int next_left = next_part->bounding_box().left();
      if (next_left == right) {
        break; // They share the same edge, so one must be a pull-out.
      }
      // Search to see if right and next_left fall within a single column.
      ColPartition *next_left_col = ColumnContaining(next_left, y);
      if (right_col == next_left_col) {
        // There is a column break in this column.
        // This can be due to a figure caption within a column, a pull-out
        // block, or a simple broken textline that remains to be merged:
        // all allowed, or a change in column layout: not allowed.
        // If both partitions are of good width, then it is likely
        // a change in column layout, otherwise probably an allowed situation.
        if (part->good_width() && next_part->good_width()) {
          if (debug) {
            int next_right = next_part->bounding_box().right();
            tprintf("CompatibleColumns false due to 2 parts of good width\n");
            tprintf("part1 %d-%d, part2 %d-%d\n", left, right, next_left,
                    next_right);
            right_col->Print();
          }
          return false;
        }
      }
      break;
    }
  }
  if (debug) {
    tprintf("CompatibleColumns true!\n");
  }
  return true;
}

// Returns the total width of all blobs in the part_set that do not lie
// within an approved column. Used as a cost measure for using this
// column set over another that might be compatible.
int ColPartitionSet::UnmatchedWidth(ColPartitionSet *part_set) {
  int total_width = 0;
  ColPartition_IT it(&part_set->parts_);
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    ColPartition *part = it.data();
    if (!BLOBNBOX::IsTextType(part->blob_type())) {
      continue; // Non-text partitions are irrelevant to column compatibility.
    }
    int y = part->MidY();
    BLOBNBOX_C_IT box_it(part->boxes());
    for (box_it.mark_cycle_pt(); !box_it.cycled_list(); box_it.forward()) {
      const TBOX &box = it.data()->bounding_box();
      // Assume that the whole blob is outside any column iff its x-middle
      // is outside.
      int x = (box.left() + box.right()) / 2;
      ColPartition *col = ColumnContaining(x, y);
      if (col == nullptr) {
        total_width += box.width();
      }
    }
  }
  return total_width;
}

// Return true if this ColPartitionSet makes a legal column candidate by
// having legal individual partitions and non-overlapping adjacent pairs.
bool ColPartitionSet::LegalColumnCandidate() {
  ColPartition_IT it(&parts_);
  if (it.empty()) {
    return false;
  }
  bool any_text_parts = false;
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    ColPartition *part = it.data();
    if (BLOBNBOX::IsTextType(part->blob_type())) {
      if (!part->IsLegal()) {
        return false; // Individual partition is illegal.
      }
      any_text_parts = true;
    }
    if (!it.at_last()) {
      ColPartition *next_part = it.data_relative(1);
      if (next_part->left_key() < part->right_key()) {
        return false;
      }
    }
  }
  return any_text_parts;
}

// Return a copy of this. If good_only will only copy the Good ColPartitions.
ColPartitionSet *ColPartitionSet::Copy(bool good_only) {
  ColPartition_LIST copy_parts;
  ColPartition_IT src_it(&parts_);
  ColPartition_IT dest_it(&copy_parts);
  for (src_it.mark_cycle_pt(); !src_it.cycled_list(); src_it.forward()) {
    ColPartition *part = src_it.data();
    if (BLOBNBOX::IsTextType(part->blob_type()) &&
        (!good_only || part->good_width() || part->good_column())) {
      dest_it.add_after_then_move(part->ShallowCopy());
    }
  }
  if (dest_it.empty()) {
    return nullptr;
  }
  return new ColPartitionSet(&copy_parts);
}

// Return the bounding boxes of columns at the given y-range
void ColPartitionSet::GetColumnBoxes(int y_bottom, int y_top,
                                     ColSegment_LIST *segments) {
  ColPartition_IT it(&parts_);
  ColSegment_IT col_it(segments);
  col_it.move_to_last();
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    ColPartition *part = it.data();
    ICOORD bot_left(part->LeftAtY(y_top), y_bottom);
    ICOORD top_right(part->RightAtY(y_bottom), y_top);
    auto *col_seg = new ColSegment();
    col_seg->InsertBox(TBOX(bot_left, top_right));
    col_it.add_after_then_move(col_seg);
  }
}

#ifndef GRAPHICS_DISABLED

// Display the edges of the columns at the given y coords.
void ColPartitionSet::DisplayColumnEdges(int y_bottom, int y_top,
                                         ScrollView *win) {
  ColPartition_IT it(&parts_);
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    ColPartition *part = it.data();
    win->Line(part->LeftAtY(y_top), y_top, part->LeftAtY(y_bottom), y_bottom);
    win->Line(part->RightAtY(y_top), y_top, part->RightAtY(y_bottom), y_bottom);
  }
}

#endif // !GRAPHICS_DISABLED

// Return the ColumnSpanningType that best explains the columns overlapped
// by the given coords(left,right,y), with the given margins.
// Also return the first and last column index touched by the coords and
// the leftmost spanned column.
// Column indices are 2n + 1 for real columns (0 based) and even values
// represent the gaps in between columns, with 0 being left of the leftmost.
// resolution refers to the ppi resolution of the image.
ColumnSpanningType ColPartitionSet::SpanningType(
    int resolution, int left, int right, int height, int y, int left_margin,
    int right_margin, int *first_col, int *last_col, int *first_spanned_col) {
  *first_col = -1;
  *last_col = -1;
  *first_spanned_col = -1;
  int margin_columns = 0;
  ColPartition_IT it(&parts_);
  int col_index = 1;
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward(), col_index += 2) {
    ColPartition *part = it.data();
    if (part->ColumnContains(left, y) ||
        (it.at_first() && part->ColumnContains(left + height, y))) {
      // In the default case, first_col is set, but columns_spanned remains
      // zero, so first_col will get reset in the first column genuinely
      // spanned, but we can tell the difference from a noise partition
      // that touches no column.
      *first_col = col_index;
      if (part->ColumnContains(right, y) ||
          (it.at_last() && part->ColumnContains(right - height, y))) {
        // Both within a single column.
        *last_col = col_index;
        return CST_FLOWING;
      }
      if (left_margin <= part->LeftAtY(y)) {
        // It completely spans this column.
        *first_spanned_col = col_index;
        margin_columns = 1;
      }
    } else if (part->ColumnContains(right, y) ||
               (it.at_last() && part->ColumnContains(right - height, y))) {
      if (*first_col < 0) {
        // It started in-between.
        *first_col = col_index - 1;
      }
      if (right_margin >= part->RightAtY(y)) {
        // It completely spans this column.
        if (margin_columns == 0) {
          *first_spanned_col = col_index;
        }
        ++margin_columns;
      }
      *last_col = col_index;
      break;
    } else if (left < part->LeftAtY(y) && right > part->RightAtY(y)) {
      // Neither left nor right are contained within, so it spans this
      // column.
      if (*first_col < 0) {
        // It started in between the previous column and the current column.
        *first_col = col_index - 1;
      }
      if (margin_columns == 0) {
        *first_spanned_col = col_index;
      }
      *last_col = col_index;
    } else if (right < part->LeftAtY(y)) {
      // We have gone past the end.
      *last_col = col_index - 1;
      if (*first_col < 0) {
        // It must lie completely between columns =>noise.
        *first_col = col_index - 1;
      }
      break;
    }
  }
  if (*first_col < 0) {
    *first_col = col_index - 1; // The last in-between.
  }
  if (*last_col < 0) {
    *last_col = col_index - 1; // The last in-between.
  }
  ASSERT_HOST(*first_col >= 0 && *last_col >= 0);
  ASSERT_HOST(*first_col <= *last_col);
  if (*first_col == *last_col && right - left < kMinColumnWidth * resolution) {
    // Neither end was in a column, and it didn't span any, so it lies
    // entirely between columns, therefore noise.
    return CST_NOISE;
  } else if (margin_columns <= 1) {
    // An exception for headings that stick outside of single-column text.
    if (margin_columns == 1 && parts_.singleton()) {
      return CST_HEADING;
    }
    // It is a pullout, as left and right were not in the same column, but
    // it doesn't go to the edge of its start and end.
    return CST_PULLOUT;
  }
  // Its margins went to the edges of first and last columns => heading.
  return CST_HEADING;
}

// The column_set has changed. Close down all in-progress WorkingPartSets in
// columns that do not match and start new ones for the new columns in this.
// As ColPartitions are turned into BLOCKs, the used ones are put in
// used_parts, as they still need to be referenced in the grid.
void ColPartitionSet::ChangeWorkColumns(const ICOORD &bleft,
                                        const ICOORD &tright, int resolution,
                                        ColPartition_LIST *used_parts,
                                        WorkingPartSet_LIST *working_set_list) {
  // Move the input list to a temporary location so we can delete its elements
  // as we add them to the output working_set.
  WorkingPartSet_LIST work_src;
  WorkingPartSet_IT src_it(&work_src);
  src_it.add_list_after(working_set_list);
  src_it.move_to_first();
  WorkingPartSet_IT dest_it(working_set_list);
  // Completed blocks and to_blocks are accumulated and given to the first new
  // one  whenever we keep a column, or at the end.
  BLOCK_LIST completed_blocks;
  TO_BLOCK_LIST to_blocks;
  WorkingPartSet *first_new_set = nullptr;
  WorkingPartSet *working_set = nullptr;
  ColPartition_IT col_it(&parts_);
  for (col_it.mark_cycle_pt(); !col_it.cycled_list(); col_it.forward()) {
    ColPartition *column = col_it.data();
    // Any existing column to the left of column is completed.
    while (!src_it.empty() &&
           ((working_set = src_it.data())->column() == nullptr ||
            working_set->column()->right_key() <= column->left_key())) {
      src_it.extract();
      working_set->ExtractCompletedBlocks(bleft, tright, resolution, used_parts,
                                          &completed_blocks, &to_blocks);
      delete working_set;
      src_it.forward();
    }
    // Make a new between-column WorkingSet for before the current column.
    working_set = new WorkingPartSet(nullptr);
    dest_it.add_after_then_move(working_set);
    if (first_new_set == nullptr) {
      first_new_set = working_set;
    }
    // A matching column gets to stay, and first_new_set gets all the
    // completed_sets.
    working_set = src_it.empty() ? nullptr : src_it.data();
    if (working_set != nullptr &&
        working_set->column()->MatchingColumns(*column)) {
      working_set->set_column(column);
      dest_it.add_after_then_move(src_it.extract());
      src_it.forward();
      first_new_set->InsertCompletedBlocks(&completed_blocks, &to_blocks);
      first_new_set = nullptr;
    } else {
      // Just make a new working set for the current column.
      working_set = new WorkingPartSet(column);
      dest_it.add_after_then_move(working_set);
    }
  }
  // Complete any remaining src working sets.
  while (!src_it.empty()) {
    working_set = src_it.extract();
    working_set->ExtractCompletedBlocks(bleft, tright, resolution, used_parts,
                                        &completed_blocks, &to_blocks);
    delete working_set;
    src_it.forward();
  }
  // Make a new between-column WorkingSet for after the last column.
  working_set = new WorkingPartSet(nullptr);
  dest_it.add_after_then_move(working_set);
  if (first_new_set == nullptr) {
    first_new_set = working_set;
  }
  // The first_new_set now gets any accumulated completed_parts/blocks.
  first_new_set->InsertCompletedBlocks(&completed_blocks, &to_blocks);
}

// Accumulate the widths and gaps into the given variables.
void ColPartitionSet::AccumulateColumnWidthsAndGaps(int *total_width,
                                                    int *width_samples,
                                                    int *total_gap,
                                                    int *gap_samples) {
  ColPartition_IT it(&parts_);
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    ColPartition *part = it.data();
    *total_width += part->ColumnWidth();
    ++*width_samples;
    if (!it.at_last()) {
      ColPartition *next_part = it.data_relative(1);
      int part_left = part->right_key();
      int part_right = next_part->left_key();
      int gap = part->KeyWidth(part_left, part_right);
      *total_gap += gap;
      ++*gap_samples;
    }
  }
}

// Provide debug output for this ColPartitionSet and all the ColPartitions.
void ColPartitionSet::Print() {
  ColPartition_IT it(&parts_);
  tprintf(
      "Partition set of %d parts, %d good, coverage=%d+%d"
      " (%d,%d)->(%d,%d)\n",
      it.length(), good_column_count_, good_coverage_, bad_coverage_,
      bounding_box_.left(), bounding_box_.bottom(), bounding_box_.right(),
      bounding_box_.top());
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    ColPartition *part = it.data();
    part->Print();
  }
}

// PRIVATE CODE.

// Add the given partition to the list in the appropriate place.
void ColPartitionSet::AddPartition(ColPartition *new_part,
                                   ColPartition_IT *it) {
  AddPartitionCoverageAndBox(*new_part);
  int new_right = new_part->right_key();
  if (it->data()->left_key() >= new_right) {
    it->add_before_stay_put(new_part);
  } else {
    it->add_after_stay_put(new_part);
  }
}

// Compute the coverage and good column count. Coverage is the amount of the
// width of the page (in pixels) that is covered by ColPartitions, which are
// used to provide candidate column layouts.
// Coverage is split into good and bad. Good coverage is provided by
// ColPartitions of a frequent width (according to the callback function
// provided by TabFinder::WidthCB, which accesses stored statistics on the
// widths of ColPartitions) and bad coverage is provided by all other
// ColPartitions, even if they have tab vectors at both sides. Thus:
// |-----------------------------------------------------------------|
// |        Double     width    heading                              |
// |-----------------------------------------------------------------|
// |-------------------------------| |-------------------------------|
// |   Common width ColParition    | |  Common width ColPartition    |
// |-------------------------------| |-------------------------------|
// the layout with two common-width columns has better coverage than the
// double width heading, because the coverage is "good," even though less in
// total coverage than the heading, because the heading coverage is "bad."
void ColPartitionSet::ComputeCoverage() {
  // Count the number of good columns and sum their width.
  ColPartition_IT it(&parts_);
  good_column_count_ = 0;
  good_coverage_ = 0;
  bad_coverage_ = 0;
  bounding_box_ = TBOX();
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    ColPartition *part = it.data();
    AddPartitionCoverageAndBox(*part);
  }
}

// Adds the coverage, column count and box for a single partition,
// without adding it to the list. (Helper factored from ComputeCoverage.)
void ColPartitionSet::AddPartitionCoverageAndBox(const ColPartition &part) {
  bounding_box_ += part.bounding_box();
  int coverage = part.ColumnWidth();
  if (part.good_width()) {
    good_coverage_ += coverage;
    good_column_count_ += 2;
  } else {
    if (part.blob_type() < BRT_UNKNOWN) {
      coverage /= 2;
    }
    if (part.good_column()) {
      ++good_column_count_;
    }
    bad_coverage_ += coverage;
  }
}

} // namespace tesseract.
