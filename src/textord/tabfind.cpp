///////////////////////////////////////////////////////////////////////
// File:        tabfind.cpp
// Description: Subclass of BBGrid to find vertically aligned blobs.
// Author:      Ray Smith
// Created:     Fri Mar 21 15:03:01 PST 2008
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
#include "config_auto.h"
#endif

#include "tabfind.h"
#include "alignedblob.h"
#include "blobbox.h"
#include "colpartitiongrid.h"
#include "detlinefit.h"
#include "linefind.h"

#include <algorithm>

namespace tesseract {

// Multiple of box size to search for initial gaps.
const int kTabRadiusFactor = 5;
// Min and Max multiple of height to search vertically when extrapolating.
const int kMinVerticalSearch = 3;
const int kMaxVerticalSearch = 12;
const int kMaxRaggedSearch = 25;
// Minimum number of lines in a column width to make it interesting.
const int kMinLinesInColumn = 10;
// Minimum width of a column to be interesting.
const int kMinColumnWidth = 200;
// Minimum fraction of total column lines for a column to be interesting.
const double kMinFractionalLinesInColumn = 0.125;
// Fraction of height used as alignment tolerance for aligned tabs.
const double kAlignedFraction = 0.03125;
// Maximum gutter width (in absolute inch) that we care about
const double kMaxGutterWidthAbsolute = 2.00;
// Multiplier of gridsize for min gutter width of TT_MAYBE_RAGGED blobs.
const int kRaggedGutterMultiple = 5;
// Min aspect ratio of tall objects to be considered a separator line.
// (These will be ignored in searching the gutter for obstructions.)
const double kLineFragmentAspectRatio = 10.0;
// Min number of points to accept after evaluation.
const int kMinEvaluatedTabs = 3;
// Up to 30 degrees is allowed for rotations of diacritic blobs.
// Keep this value slightly larger than kCosSmallAngle in blobbox.cpp
// so that the assert there never fails.
const double kCosMaxSkewAngle = 0.866025;

BOOL_VAR(textord_tabfind_show_initialtabs, false, "Show tab candidates");
BOOL_VAR(textord_tabfind_show_finaltabs, false, "Show tab vectors");

TabFind::TabFind(int gridsize, const ICOORD& bleft, const ICOORD& tright,
                 TabVector_LIST* vlines, int vertical_x, int vertical_y,
                 int resolution)
  : AlignedBlob(gridsize, bleft, tright),
    resolution_(resolution),
    image_origin_(0, tright.y() - 1),
    v_it_(&vectors_) {
  width_cb_ = nullptr;
  v_it_.add_list_after(vlines);
  SetVerticalSkewAndParallelize(vertical_x, vertical_y);
  width_cb_ = NewPermanentTessCallback(this, &TabFind::CommonWidth);
}

TabFind::~TabFind() {
  delete width_cb_;
}

///////////////// PUBLIC functions (mostly used by TabVector). //////////////

// Insert a list of blobs into the given grid (not necessarily this).
// If take_ownership is true, then the blobs are removed from the source list.
// See InsertBlob for the other arguments.
// It would seem to make more sense to swap this and grid, but this way
// around allows grid to not be derived from TabFind, eg a ColPartitionGrid,
// while the grid that provides the tab stops(this) has to be derived from
// TabFind.
void TabFind::InsertBlobsToGrid(bool h_spread, bool v_spread,
                                BLOBNBOX_LIST* blobs,
                                BBGrid<BLOBNBOX, BLOBNBOX_CLIST,
                                       BLOBNBOX_C_IT>* grid) {
  BLOBNBOX_IT blob_it(blobs);
  int b_count = 0;
  int reject_count = 0;
  for (blob_it.mark_cycle_pt(); !blob_it.cycled_list(); blob_it.forward()) {
    BLOBNBOX* blob = blob_it.data();
//    if (InsertBlob(true, true, blob, grid)) {
    if (InsertBlob(h_spread, v_spread, blob, grid)) {
      ++b_count;
    } else {
      ++reject_count;
    }
  }
  if (textord_debug_tabfind) {
    tprintf("Inserted %d blobs into grid, %d rejected.\n",
            b_count, reject_count);
  }
}

// Insert a single blob into the given grid (not necessarily this).
// If h_spread, then all cells covered horizontally by the box are
// used, otherwise, just the bottom-left. Similarly for v_spread.
// A side effect is that the left and right rule edges of the blob are
// set according to the tab vectors in this (not grid).
bool TabFind::InsertBlob(bool h_spread, bool v_spread, BLOBNBOX* blob,
                         BBGrid<BLOBNBOX, BLOBNBOX_CLIST,
                                BLOBNBOX_C_IT>* grid) {
  TBOX box = blob->bounding_box();
  blob->set_left_rule(LeftEdgeForBox(box, false, false));
  blob->set_right_rule(RightEdgeForBox(box, false, false));
  blob->set_left_crossing_rule(LeftEdgeForBox(box, true, false));
  blob->set_right_crossing_rule(RightEdgeForBox(box, true, false));
  if (blob->joined_to_prev())
    return false;
  grid->InsertBBox(h_spread, v_spread, blob);
  return true;
}

// Calls SetBlobRuleEdges for all the blobs in the given block.
void TabFind::SetBlockRuleEdges(TO_BLOCK* block) {
  SetBlobRuleEdges(&block->blobs);
  SetBlobRuleEdges(&block->small_blobs);
  SetBlobRuleEdges(&block->noise_blobs);
  SetBlobRuleEdges(&block->large_blobs);
}

// Sets the left and right rule and crossing_rules for the blobs in the given
// list by fiding the next outermost tabvectors for each blob.
void TabFind::SetBlobRuleEdges(BLOBNBOX_LIST* blobs) {
  BLOBNBOX_IT blob_it(blobs);
  for (blob_it.mark_cycle_pt(); !blob_it.cycled_list(); blob_it.forward()) {
    BLOBNBOX* blob = blob_it.data();
    TBOX box = blob->bounding_box();
    blob->set_left_rule(LeftEdgeForBox(box, false, false));
    blob->set_right_rule(RightEdgeForBox(box, false, false));
    blob->set_left_crossing_rule(LeftEdgeForBox(box, true, false));
    blob->set_right_crossing_rule(RightEdgeForBox(box, true, false));
  }
}

// Returns the gutter width of the given TabVector between the given y limits.
// Also returns x-shift to be added to the vector to clear any intersecting
// blobs. The shift is deducted from the returned gutter.
// If ignore_unmergeables is true, then blobs of UnMergeableType are
// ignored as if they don't exist. (Used for text on image.)
// max_gutter_width is used as the maximum width worth searching for in case
// there is nothing near the TabVector.
int TabFind::GutterWidth(int bottom_y, int top_y, const TabVector& v,
                         bool ignore_unmergeables, int max_gutter_width,
                         int* required_shift) {
  bool right_to_left = v.IsLeftTab();
  int bottom_x = v.XAtY(bottom_y);
  int top_x = v.XAtY(top_y);
  int start_x = right_to_left ? std::max(top_x, bottom_x) : std::min(top_x, bottom_x);
  BlobGridSearch sidesearch(this);
  sidesearch.StartSideSearch(start_x, bottom_y, top_y);
  int min_gap = max_gutter_width;
  *required_shift = 0;
  BLOBNBOX* blob = nullptr;
  while ((blob = sidesearch.NextSideSearch(right_to_left)) != nullptr) {
    const TBOX& box = blob->bounding_box();
    if (box.bottom() >= top_y || box.top() <= bottom_y)
      continue;  // Doesn't overlap enough.
    if (box.height() >= gridsize() * 2 &&
        box.height() > box.width() * kLineFragmentAspectRatio) {
      // Skip likely separator line residue.
      continue;
    }
    if (ignore_unmergeables && BLOBNBOX::UnMergeableType(blob->region_type()))
      continue;  // Skip non-text if required.
    int mid_y = (box.bottom() + box.top()) / 2;
    // We use the x at the mid-y so that the required_shift guarantees
    // to clear all the blobs on the tab-stop. If we use the min/max
    // of x at top/bottom of the blob, then exactness would be required,
    // which is not a good thing.
    int tab_x = v.XAtY(mid_y);
    int gap;
    if (right_to_left) {
      gap = tab_x - box.right();
      if (gap < 0 && box.left() - tab_x < *required_shift)
        *required_shift = box.left() - tab_x;
    } else {
      gap = box.left() - tab_x;
      if (gap < 0 && box.right() - tab_x > *required_shift)
        *required_shift = box.right() - tab_x;
    }
    if (gap > 0 && gap < min_gap)
      min_gap = gap;
  }
  // Result may be negative, in which case,  this is a really bad tabstop.
  return min_gap - abs(*required_shift);
}

// Find the gutter width and distance to inner neighbour for the given blob.
void TabFind::GutterWidthAndNeighbourGap(int tab_x, int mean_height,
                                         int max_gutter, bool left,
                                         BLOBNBOX* bbox, int* gutter_width,
                                         int* neighbour_gap) {
  const TBOX& box = bbox->bounding_box();
  // The gutter and internal sides of the box.
  int gutter_x = left ? box.left() : box.right();
  int internal_x = left ? box.right() : box.left();
  // On ragged edges, the gutter side of the box is away from the tabstop.
  int tab_gap = left ? gutter_x - tab_x : tab_x - gutter_x;
  *gutter_width = max_gutter;
  // If the box is away from the tabstop, we need to increase
  // the allowed gutter width.
  if (tab_gap > 0)
    *gutter_width += tab_gap;
  bool debug = WithinTestRegion(2, box.left(), box.bottom());
  if (debug)
    tprintf("Looking in gutter\n");
  // Find the nearest blob on the outside of the column.
  BLOBNBOX* gutter_bbox = AdjacentBlob(bbox, left,
                                       bbox->flow() == BTFT_TEXT_ON_IMAGE, 0.0,
                                       *gutter_width, box.top(), box.bottom());
  if (gutter_bbox != nullptr) {
    const TBOX& gutter_box = gutter_bbox->bounding_box();
    *gutter_width = left ? tab_x - gutter_box.right()
                        : gutter_box.left() - tab_x;
  }
  if (*gutter_width >= max_gutter) {
    // If there is no box because a tab was in the way, get the tab coord.
    TBOX gutter_box(box);
    if (left) {
      gutter_box.set_left(tab_x - max_gutter - 1);
      gutter_box.set_right(tab_x - max_gutter);
      int tab_gutter = RightEdgeForBox(gutter_box, true, false);
      if (tab_gutter < tab_x - 1)
        *gutter_width = tab_x - tab_gutter;
    } else {
      gutter_box.set_left(tab_x + max_gutter);
      gutter_box.set_right(tab_x + max_gutter + 1);
      int tab_gutter = LeftEdgeForBox(gutter_box, true, false);
      if (tab_gutter > tab_x + 1)
        *gutter_width = tab_gutter - tab_x;
    }
  }
  if (*gutter_width > max_gutter)
    *gutter_width = max_gutter;
  // Now look for a neighbour on the inside.
  if (debug)
    tprintf("Looking for neighbour\n");
  BLOBNBOX* neighbour = AdjacentBlob(bbox, !left,
                                     bbox->flow() == BTFT_TEXT_ON_IMAGE, 0.0,
                                     *gutter_width, box.top(), box.bottom());
  int neighbour_edge = left ? RightEdgeForBox(box, true, false)
                            : LeftEdgeForBox(box, true, false);
  if (neighbour != nullptr) {
    const TBOX& n_box = neighbour->bounding_box();
    if (debug) {
      tprintf("Found neighbour:");
      n_box.print();
    }
    if (left && n_box.left() < neighbour_edge)
      neighbour_edge = n_box.left();
    else if (!left && n_box.right() > neighbour_edge)
      neighbour_edge = n_box.right();
  }
  *neighbour_gap = left ? neighbour_edge - internal_x
                        : internal_x - neighbour_edge;
}

// Return the x-coord that corresponds to the right edge for the given
// box. If there is a rule line to the right that vertically overlaps it,
// then return the x-coord of the rule line, otherwise return the right
// edge of the page. For details see RightTabForBox below.
int TabFind::RightEdgeForBox(const TBOX& box, bool crossing, bool extended) {
  TabVector* v = RightTabForBox(box, crossing, extended);
  return v == nullptr ? tright_.x() : v->XAtY((box.top() + box.bottom()) / 2);
}
// As RightEdgeForBox, but finds the left Edge instead.
int TabFind::LeftEdgeForBox(const TBOX& box, bool crossing, bool extended) {
  TabVector* v = LeftTabForBox(box, crossing, extended);
  return v == nullptr ? bleft_.x() : v->XAtY((box.top() + box.bottom()) / 2);
}

// This comment documents how this function works.
// For its purpose and arguments, see the comment in tabfind.h.
// TabVectors are stored sorted by perpendicular distance of middle from
// the global mean vertical vector. Since the individual vectors can have
// differing directions, their XAtY for a given y is not necessarily in the
// right order. Therefore the search has to be run with a margin.
// The middle of a vector that passes through (x,y) cannot be higher than
// halfway from y to the top, or lower than halfway from y to the bottom
// of the coordinate range; therefore, the search margin is the range of
// sort keys between these halfway points. Any vector with a sort key greater
// than the upper margin must be to the right of x at y, and likewise any
// vector with a sort key less than the lower margin must pass to the left
// of x at y.
TabVector* TabFind::RightTabForBox(const TBOX& box, bool crossing,
                                   bool extended) {
  if (v_it_.empty())
    return nullptr;
  int top_y = box.top();
  int bottom_y = box.bottom();
  int mid_y = (top_y + bottom_y) / 2;
  int right = crossing ? (box.left() + box.right()) / 2 : box.right();
  int min_key, max_key;
  SetupTabSearch(right, mid_y, &min_key, &max_key);
  // Position the iterator at the first TabVector with sort_key >= min_key.
  while (!v_it_.at_first() && v_it_.data()->sort_key() >= min_key)
    v_it_.backward();
  while (!v_it_.at_last() && v_it_.data()->sort_key() < min_key)
    v_it_.forward();
  // Find the leftmost tab vector that overlaps and has XAtY(mid_y) >= right.
  TabVector* best_v = nullptr;
  int best_x = -1;
  int key_limit = -1;
  do {
    TabVector* v = v_it_.data();
    int x = v->XAtY(mid_y);
    if (x >= right &&
        (v->VOverlap(top_y, bottom_y) > 0 ||
         (extended && v->ExtendedOverlap(top_y, bottom_y) > 0))) {
      if (best_v == nullptr || x < best_x) {
        best_v = v;
        best_x = x;
        // We can guarantee that no better vector can be found if the
        // sort key exceeds that of the best by max_key - min_key.
        key_limit = v->sort_key() + max_key - min_key;
      }
    }
    // Break when the search is done to avoid wrapping the iterator and
    // thereby potentially slowing the next search.
    if (v_it_.at_last() ||
        (best_v != nullptr && v->sort_key() > key_limit))
      break;  // Prevent restarting list for next call.
    v_it_.forward();
  } while (!v_it_.at_first());
  return best_v;
}

// As RightTabForBox, but finds the left TabVector instead.
TabVector* TabFind::LeftTabForBox(const TBOX& box, bool crossing,
                                  bool extended) {
  if (v_it_.empty())
    return nullptr;
  int top_y = box.top();
  int bottom_y = box.bottom();
  int mid_y = (top_y + bottom_y) / 2;
  int left = crossing ? (box.left() + box.right()) / 2 : box.left();
  int min_key, max_key;
  SetupTabSearch(left, mid_y, &min_key, &max_key);
  // Position the iterator at the last TabVector with sort_key <= max_key.
  while (!v_it_.at_last() && v_it_.data()->sort_key() <= max_key)
    v_it_.forward();
  while (!v_it_.at_first() && v_it_.data()->sort_key() > max_key) {
    v_it_.backward();
  }
  // Find the rightmost tab vector that overlaps and has XAtY(mid_y) <= left.
  TabVector* best_v = nullptr;
  int best_x = -1;
  int key_limit = -1;
  do {
    TabVector* v = v_it_.data();
    int x = v->XAtY(mid_y);
    if (x <= left &&
        (v->VOverlap(top_y, bottom_y) > 0 ||
         (extended && v->ExtendedOverlap(top_y, bottom_y) > 0))) {
      if (best_v == nullptr || x > best_x) {
        best_v = v;
        best_x = x;
        // We can guarantee that no better vector can be found if the
        // sort key is less than that of the best by max_key - min_key.
        key_limit = v->sort_key() - (max_key - min_key);
      }
    }
    // Break when the search is done to avoid wrapping the iterator and
    // thereby potentially slowing the next search.
    if (v_it_.at_first() ||
        (best_v != nullptr && v->sort_key() < key_limit))
      break;  // Prevent restarting list for next call.
    v_it_.backward();
  } while (!v_it_.at_last());
  return best_v;
}

// Return true if the given width is close to one of the common
// widths in column_widths_.
bool TabFind::CommonWidth(int width) {
  width /= kColumnWidthFactor;
  ICOORDELT_IT it(&column_widths_);
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    ICOORDELT* w = it.data();
    if (w->x() - 1 <= width && width <= w->y() + 1)
      return true;
  }
  return false;
}

// Return true if the sizes are more than a
// factor of 2 different.
bool TabFind::DifferentSizes(int size1, int size2) {
  return size1 > size2 * 2 || size2 > size1 * 2;
}

// Return true if the sizes are more than a
// factor of 5 different.
bool TabFind::VeryDifferentSizes(int size1, int size2) {
  return size1 > size2 * 5 || size2 > size1 * 5;
}

///////////////// PROTECTED functions (used by ColumnFinder). //////////////

// Top-level function to find TabVectors in an input page block.
// Returns false if the detected skew angle is impossible.
// Applies the detected skew angle to deskew the tabs, blobs and part_grid.
bool TabFind::FindTabVectors(TabVector_LIST* hlines,
                             BLOBNBOX_LIST* image_blobs, TO_BLOCK* block,
                             int min_gutter_width,
                             double tabfind_aligned_gap_fraction,
                             ColPartitionGrid* part_grid,
                             FCOORD* deskew, FCOORD* reskew) {
  ScrollView* tab_win = FindInitialTabVectors(image_blobs, min_gutter_width,
                                              tabfind_aligned_gap_fraction,
                                              block);
  ComputeColumnWidths(tab_win, part_grid);
  TabVector::MergeSimilarTabVectors(vertical_skew_, &vectors_, this);
  SortVectors();
  CleanupTabs();
  if (!Deskew(hlines, image_blobs, block, deskew, reskew))
    return false;  // Skew angle is too large.
  part_grid->Deskew(*deskew);
  ApplyTabConstraints();
  #ifndef GRAPHICS_DISABLED
  if (textord_tabfind_show_finaltabs) {
    tab_win = MakeWindow(640, 50, "FinalTabs");
    DisplayBoxes(tab_win);
    DisplayTabs("FinalTabs", tab_win);
    tab_win = DisplayTabVectors(tab_win);
  }
  #endif  // GRAPHICS_DISABLED
  return true;
}

// Top-level function to not find TabVectors in an input page block,
// but setup for single column mode.
void TabFind::DontFindTabVectors(BLOBNBOX_LIST* image_blobs, TO_BLOCK* block,
                                 FCOORD* deskew, FCOORD* reskew) {
  InsertBlobsToGrid(false, false, image_blobs, this);
  InsertBlobsToGrid(true, false, &block->blobs, this);
  deskew->set_x(1.0f);
  deskew->set_y(0.0f);
  reskew->set_x(1.0f);
  reskew->set_y(0.0f);
}

// Cleans up the lists of blobs in the block ready for use by TabFind.
// Large blobs that look like text are moved to the main blobs list.
// Main blobs that are superseded by the image blobs are deleted.
void TabFind::TidyBlobs(TO_BLOCK* block) {
  BLOBNBOX_IT large_it = &block->large_blobs;
  BLOBNBOX_IT blob_it = &block->blobs;
  int b_count = 0;
  for (large_it.mark_cycle_pt(); !large_it.cycled_list(); large_it.forward()) {
    BLOBNBOX* large_blob = large_it.data();
    if (large_blob->owner() != nullptr) {
      blob_it.add_to_end(large_it.extract());
      ++b_count;
    }
  }
  if (textord_debug_tabfind) {
    tprintf("Moved %d large blobs to normal list\n",
            b_count);
    #ifndef GRAPHICS_DISABLED
    ScrollView* rej_win = MakeWindow(500, 300, "Image blobs");
    block->plot_graded_blobs(rej_win);
    block->plot_noise_blobs(rej_win);
    rej_win->Update();
    #endif  // GRAPHICS_DISABLED
  }
  block->DeleteUnownedNoise();
}

// Helper function to setup search limits for *TabForBox.
void TabFind::SetupTabSearch(int x, int y, int* min_key, int* max_key) {
  int key1 = TabVector::SortKey(vertical_skew_, x, (y + tright_.y()) / 2);
  int key2 = TabVector::SortKey(vertical_skew_, x, (y + bleft_.y()) / 2);
  *min_key = std::min(key1, key2);
  *max_key = std::max(key1, key2);
}

ScrollView* TabFind::DisplayTabVectors(ScrollView* tab_win) {
#ifndef GRAPHICS_DISABLED
  // For every vector, display it.
  TabVector_IT it(&vectors_);
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    TabVector* vector = it.data();
    vector->Display(tab_win);
  }
  tab_win->Update();
#endif
  return tab_win;
}

// PRIVATE CODE.
//
// First part of FindTabVectors, which may be used twice if the text
// is mostly of vertical alignment.
ScrollView* TabFind::FindInitialTabVectors(BLOBNBOX_LIST* image_blobs,
                                           int min_gutter_width,
                                           double tabfind_aligned_gap_fraction,
                                           TO_BLOCK* block) {
  if (textord_tabfind_show_initialtabs) {
    ScrollView* line_win = MakeWindow(0, 0, "VerticalLines");
    line_win = DisplayTabVectors(line_win);
  }
  // Prepare the grid.
  if (image_blobs != nullptr)
    InsertBlobsToGrid(true, false, image_blobs, this);
  InsertBlobsToGrid(true, false, &block->blobs, this);
  ScrollView* initial_win = FindTabBoxes(min_gutter_width,
                                         tabfind_aligned_gap_fraction);
  FindAllTabVectors(min_gutter_width);

  TabVector::MergeSimilarTabVectors(vertical_skew_, &vectors_, this);
  SortVectors();
  EvaluateTabs();
  if (textord_tabfind_show_initialtabs && initial_win != nullptr)
    initial_win = DisplayTabVectors(initial_win);
  MarkVerticalText();
  return initial_win;
}

// Helper displays all the boxes in the given vector on the given window.
static void DisplayBoxVector(const GenericVector<BLOBNBOX*>& boxes,
                             ScrollView* win) {
  #ifndef GRAPHICS_DISABLED
  for (int i = 0; i < boxes.size(); ++i) {
    TBOX box = boxes[i]->bounding_box();
    int left_x = box.left();
    int right_x = box.right();
    int top_y = box.top();
    int bottom_y = box.bottom();
    ScrollView::Color box_color = boxes[i]->BoxColor();
    win->Pen(box_color);
    win->Rectangle(left_x, bottom_y, right_x, top_y);
  }
  win->Update();
  #endif  // GRAPHICS_DISABLED
}

// For each box in the grid, decide whether it is a candidate tab-stop,
// and if so add it to the left/right tab boxes.
ScrollView* TabFind::FindTabBoxes(int min_gutter_width,
                                  double tabfind_aligned_gap_fraction) {
  left_tab_boxes_.clear();
  right_tab_boxes_.clear();
  // For every bbox in the grid, determine whether it uses a tab on an edge.
  GridSearch<BLOBNBOX, BLOBNBOX_CLIST, BLOBNBOX_C_IT> gsearch(this);
  gsearch.StartFullSearch();
  BLOBNBOX* bbox;
  while ((bbox = gsearch.NextFullSearch()) != nullptr) {
    if (TestBoxForTabs(bbox, min_gutter_width, tabfind_aligned_gap_fraction)) {
      // If it is any kind of tab, insert it into the vectors.
      if (bbox->left_tab_type() != TT_NONE)
        left_tab_boxes_.push_back(bbox);
      if (bbox->right_tab_type() != TT_NONE)
        right_tab_boxes_.push_back(bbox);
    }
  }
  // Sort left tabs by left and right by right to see the outermost one first
  // on a ragged tab.
  left_tab_boxes_.sort(SortByBoxLeft<BLOBNBOX>);
  right_tab_boxes_.sort(SortRightToLeft<BLOBNBOX>);
  ScrollView* tab_win = nullptr;
  #ifndef GRAPHICS_DISABLED
  if (textord_tabfind_show_initialtabs) {
    tab_win = MakeWindow(0, 100, "InitialTabs");
    tab_win->Pen(ScrollView::BLUE);
    tab_win->Brush(ScrollView::NONE);
    // Display the left and right tab boxes.
    DisplayBoxVector(left_tab_boxes_, tab_win);
    DisplayBoxVector(right_tab_boxes_, tab_win);
    tab_win = DisplayTabs("Tabs", tab_win);
  }
  #endif  // GRAPHICS_DISABLED
  return tab_win;
}

bool TabFind::TestBoxForTabs(BLOBNBOX* bbox, int min_gutter_width,
                             double tabfind_aligned_gap_fraction) {
  GridSearch<BLOBNBOX, BLOBNBOX_CLIST, BLOBNBOX_C_IT> radsearch(this);
  TBOX box = bbox->bounding_box();
  // If there are separator lines, get the column edges.
  int left_column_edge = bbox->left_rule();
  int right_column_edge = bbox->right_rule();
  // The edges of the bounding box of the blob being processed.
  int left_x = box.left();
  int right_x = box.right();
  int top_y = box.top();
  int bottom_y = box.bottom();
  int height = box.height();
  bool debug = WithinTestRegion(3, left_x, top_y);
  if (debug) {
    tprintf("Column edges for blob at (%d,%d)->(%d,%d) are [%d, %d]\n",
            left_x, top_y, right_x, bottom_y,
            left_column_edge, right_column_edge);
  }
  // Compute a search radius based on a multiple of the height.
  int radius = (height * kTabRadiusFactor + gridsize_ - 1) / gridsize_;
  radsearch.StartRadSearch((left_x + right_x)/2, (top_y + bottom_y)/2, radius);
  // In Vertical Page mode, once we have an estimate of the vertical line
  // spacing, the minimum amount of gutter space before a possible tab is
  // increased under the assumption that column partition is always larger
  // than line spacing.
  int min_spacing =
      static_cast<int>(height * tabfind_aligned_gap_fraction);
  if (min_gutter_width > min_spacing)
    min_spacing = min_gutter_width;
  int min_ragged_gutter = kRaggedGutterMultiple * gridsize();
  if (min_gutter_width > min_ragged_gutter)
    min_ragged_gutter = min_gutter_width;
  int target_right = left_x - min_spacing;
  int target_left = right_x + min_spacing;
  // We will be evaluating whether the left edge could be a left tab, and
  // whether the right edge could be a right tab.
  // A box can be a tab if its bool is_(left/right)_tab remains true, meaning
  // that no blobs have been found in the gutter during the radial search.
  // A box can also be a tab if there are objects in the gutter only above
  // or only below, and there are aligned objects on the opposite side, but
  // not too many unaligned objects. The maybe_(left/right)_tab_up counts
  // aligned objects above and negatively counts unaligned objects above,
  // and is set to -INT32_MAX if a gutter object is found above.
  // The other 3 maybe ints work similarly for the other sides.
  // These conditions are very strict, to minimize false positives, and really
  // only aligned tabs and outermost ragged tab blobs will qualify, so we
  // also have maybe_ragged_left/right with less stringent rules.
  // A blob that is maybe_ragged_left/right will be further qualified later,
  // using the min_ragged_gutter.
  bool is_left_tab = true;
  bool is_right_tab = true;
  bool maybe_ragged_left = true;
  bool maybe_ragged_right = true;
  int maybe_left_tab_up = 0;
  int maybe_right_tab_up = 0;
  int maybe_left_tab_down = 0;
  int maybe_right_tab_down = 0;
  if (bbox->leader_on_left()) {
    is_left_tab = false;
    maybe_ragged_left = false;
    maybe_left_tab_up = -INT32_MAX;
    maybe_left_tab_down = -INT32_MAX;
  }
  if (bbox->leader_on_right()) {
    is_right_tab = false;
    maybe_ragged_right = false;
    maybe_right_tab_up = -INT32_MAX;
    maybe_right_tab_down = -INT32_MAX;
  }
  int alignment_tolerance = static_cast<int>(resolution_ * kAlignedFraction);
  BLOBNBOX* neighbour = nullptr;
  while ((neighbour = radsearch.NextRadSearch()) != nullptr) {
    if (neighbour == bbox)
      continue;
    TBOX nbox = neighbour->bounding_box();
    int n_left = nbox.left();
    int n_right = nbox.right();
    if (debug)
      tprintf("Neighbour at (%d,%d)->(%d,%d)\n",
              n_left, nbox.bottom(), n_right, nbox.top());
    // If the neighbouring blob is the wrong side of a separator line, then it
    // "doesn't exist" as far as we are concerned.
    if (n_right > right_column_edge || n_left < left_column_edge ||
        left_x < neighbour->left_rule() || right_x > neighbour->right_rule())
      continue;  // Separator line in the way.
    int n_mid_x = (n_left + n_right) / 2;
    int n_mid_y = (nbox.top() + nbox.bottom()) / 2;
    if (n_mid_x <= left_x && n_right >= target_right) {
      if (debug)
        tprintf("Not a left tab\n");
      is_left_tab = false;
      if (n_mid_y < top_y)
        maybe_left_tab_down = -INT32_MAX;
      if (n_mid_y > bottom_y)
        maybe_left_tab_up = -INT32_MAX;
    } else if (NearlyEqual(left_x, n_left, alignment_tolerance)) {
      if (debug)
        tprintf("Maybe a left tab\n");
      if (n_mid_y > top_y && maybe_left_tab_up > -INT32_MAX)
        ++maybe_left_tab_up;
      if (n_mid_y < bottom_y && maybe_left_tab_down > -INT32_MAX)
        ++maybe_left_tab_down;
    } else if (n_left < left_x && n_right >= left_x) {
      // Overlaps but not aligned so negative points on a maybe.
      if (debug)
        tprintf("Maybe Not a left tab\n");
      if (n_mid_y > top_y && maybe_left_tab_up > -INT32_MAX)
        --maybe_left_tab_up;
      if (n_mid_y < bottom_y && maybe_left_tab_down > -INT32_MAX)
        --maybe_left_tab_down;
    }
    if (n_left < left_x && nbox.y_overlap(box) && n_right >= target_right) {
      maybe_ragged_left = false;
      if (debug)
        tprintf("Not a ragged left\n");
    }
    if (n_mid_x >= right_x && n_left <= target_left) {
      if (debug)
        tprintf("Not a right tab\n");
      is_right_tab = false;
      if (n_mid_y < top_y)
        maybe_right_tab_down = -INT32_MAX;
      if (n_mid_y > bottom_y)
        maybe_right_tab_up = -INT32_MAX;
    } else if (NearlyEqual(right_x, n_right, alignment_tolerance)) {
      if (debug)
        tprintf("Maybe a right tab\n");
      if (n_mid_y > top_y && maybe_right_tab_up > -INT32_MAX)
        ++maybe_right_tab_up;
      if (n_mid_y < bottom_y && maybe_right_tab_down > -INT32_MAX)
        ++maybe_right_tab_down;
    } else if (n_right > right_x && n_left <= right_x) {
      // Overlaps but not aligned so negative points on a maybe.
      if (debug)
        tprintf("Maybe Not a right tab\n");
      if (n_mid_y > top_y && maybe_right_tab_up > -INT32_MAX)
        --maybe_right_tab_up;
      if (n_mid_y < bottom_y && maybe_right_tab_down > -INT32_MAX)
        --maybe_right_tab_down;
    }
    if (n_right > right_x && nbox.y_overlap(box) && n_left <= target_left) {
      maybe_ragged_right = false;
      if (debug)
        tprintf("Not a ragged right\n");
    }
    if (maybe_left_tab_down == -INT32_MAX && maybe_left_tab_up == -INT32_MAX &&
        maybe_right_tab_down == -INT32_MAX && maybe_right_tab_up == -INT32_MAX)
      break;
  }
  if (is_left_tab || maybe_left_tab_up > 1 || maybe_left_tab_down > 1) {
    bbox->set_left_tab_type(TT_MAYBE_ALIGNED);
  } else if (maybe_ragged_left && ConfirmRaggedLeft(bbox, min_ragged_gutter)) {
    bbox->set_left_tab_type(TT_MAYBE_RAGGED);
  } else {
    bbox->set_left_tab_type(TT_NONE);
  }
  if (is_right_tab || maybe_right_tab_up > 1 || maybe_right_tab_down > 1) {
    bbox->set_right_tab_type(TT_MAYBE_ALIGNED);
  } else if (maybe_ragged_right &&
             ConfirmRaggedRight(bbox, min_ragged_gutter)) {
    bbox->set_right_tab_type(TT_MAYBE_RAGGED);
  } else {
    bbox->set_right_tab_type(TT_NONE);
  }
  if (debug) {
    tprintf("Left result = %s, Right result=%s\n",
            bbox->left_tab_type() == TT_MAYBE_ALIGNED ? "Aligned" :
            (bbox->left_tab_type() == TT_MAYBE_RAGGED ? "Ragged" : "None"),
            bbox->right_tab_type() == TT_MAYBE_ALIGNED ? "Aligned" :
            (bbox->right_tab_type() == TT_MAYBE_RAGGED ? "Ragged" : "None"));
  }
  return bbox->left_tab_type() != TT_NONE || bbox->right_tab_type() != TT_NONE;
}

// Returns true if there is nothing in the rectangle of width min_gutter to
// the left of bbox.
bool TabFind::ConfirmRaggedLeft(BLOBNBOX* bbox, int min_gutter) {
  TBOX search_box(bbox->bounding_box());
  search_box.set_right(search_box.left());
  search_box.set_left(search_box.left() - min_gutter);
  return NothingYOverlapsInBox(search_box, bbox->bounding_box());
}

// Returns true if there is nothing in the rectangle of width min_gutter to
// the right of bbox.
bool TabFind::ConfirmRaggedRight(BLOBNBOX* bbox, int min_gutter) {
  TBOX search_box(bbox->bounding_box());
  search_box.set_left(search_box.right());
  search_box.set_right(search_box.right() + min_gutter);
  return NothingYOverlapsInBox(search_box, bbox->bounding_box());
}

// Returns true if there is nothing in the given search_box that vertically
// overlaps target_box other than target_box itself.
bool TabFind::NothingYOverlapsInBox(const TBOX& search_box,
                                    const TBOX& target_box) {
  BlobGridSearch rsearch(this);
  rsearch.StartRectSearch(search_box);
  BLOBNBOX* blob;
  while ((blob = rsearch.NextRectSearch()) != nullptr) {
    const TBOX& box = blob->bounding_box();
    if (box.y_overlap(target_box) && !(box == target_box))
      return false;
  }
  return true;
}

void TabFind::FindAllTabVectors(int min_gutter_width) {
  // A list of vectors that will be created in estimating the skew.
  TabVector_LIST dummy_vectors;
  // An estimate of the vertical direction, revised as more lines are added.
  int vertical_x = 0;
  int vertical_y = 1;
  // Find an estimate of the vertical direction by finding some tab vectors.
  // Slowly up the search size until we get some vectors.
  for (int search_size = kMinVerticalSearch; search_size < kMaxVerticalSearch;
       search_size += kMinVerticalSearch) {
    int vector_count = FindTabVectors(search_size, TA_LEFT_ALIGNED,
                                      min_gutter_width,
                                      &dummy_vectors,
                                      &vertical_x, &vertical_y);
    vector_count += FindTabVectors(search_size, TA_RIGHT_ALIGNED,
                                   min_gutter_width,
                                   &dummy_vectors,
                                   &vertical_x, &vertical_y);
    if (vector_count > 0)
      break;
  }
  // Get rid of the test vectors and reset the types of the tabs.
  dummy_vectors.clear();
  for (int i = 0; i < left_tab_boxes_.size(); ++i) {
    BLOBNBOX* bbox = left_tab_boxes_[i];
    if (bbox->left_tab_type() == TT_CONFIRMED)
      bbox->set_left_tab_type(TT_MAYBE_ALIGNED);
  }
  for (int i = 0; i < right_tab_boxes_.size(); ++i) {
    BLOBNBOX* bbox = right_tab_boxes_[i];
    if (bbox->right_tab_type() == TT_CONFIRMED)
      bbox->set_right_tab_type(TT_MAYBE_ALIGNED);
  }
  if (textord_debug_tabfind) {
    tprintf("Beginning real tab search with vertical = %d,%d...\n",
            vertical_x, vertical_y);
  }
  // Now do the real thing ,but keep the vectors in the dummy_vectors list
  // until they are all done, so we don't get the tab vectors confused with
  // the rule line vectors.
  FindTabVectors(kMaxVerticalSearch, TA_LEFT_ALIGNED, min_gutter_width,
                 &dummy_vectors, &vertical_x, &vertical_y);
  FindTabVectors(kMaxVerticalSearch, TA_RIGHT_ALIGNED, min_gutter_width,
                 &dummy_vectors, &vertical_x, &vertical_y);
  FindTabVectors(kMaxRaggedSearch, TA_LEFT_RAGGED, min_gutter_width,
                 &dummy_vectors, &vertical_x, &vertical_y);
  FindTabVectors(kMaxRaggedSearch, TA_RIGHT_RAGGED, min_gutter_width,
                 &dummy_vectors, &vertical_x, &vertical_y);
  // Now add the vectors to the vectors_ list.
  TabVector_IT v_it(&vectors_);
  v_it.add_list_after(&dummy_vectors);
  // Now use the summed (mean) vertical vector as the direction for everything.
  SetVerticalSkewAndParallelize(vertical_x, vertical_y);
}

// Helper for FindAllTabVectors finds the vectors of a particular type.
int TabFind::FindTabVectors(int search_size_multiple, TabAlignment alignment,
                            int min_gutter_width, TabVector_LIST* vectors,
                            int* vertical_x, int* vertical_y) {
  TabVector_IT vector_it(vectors);
  int vector_count = 0;
  // Search the right or left tab boxes, looking for tab vectors.
  bool right = alignment == TA_RIGHT_ALIGNED || alignment == TA_RIGHT_RAGGED;
  const GenericVector<BLOBNBOX*>& boxes = right ? right_tab_boxes_
                                                : left_tab_boxes_;
  for (int i = 0; i < boxes.size(); ++i) {
    BLOBNBOX* bbox = boxes[i];
    if ((!right && bbox->left_tab_type() == TT_MAYBE_ALIGNED) ||
        (right && bbox->right_tab_type() == TT_MAYBE_ALIGNED)) {
      TabVector* vector = FindTabVector(search_size_multiple, min_gutter_width,
                                        alignment,
                                        bbox, vertical_x, vertical_y);
      if (vector != nullptr) {
        ++vector_count;
        vector_it.add_to_end(vector);
      }
    }
  }
  return vector_count;
}

// Finds a vector corresponding to a tabstop running through the
// given box of the given alignment type.
// search_size_multiple is a multiple of height used to control
// the size of the search.
// vertical_x and y are updated with an estimate of the real
// vertical direction. (skew finding.)
// Returns nullptr if no decent tabstop can be found.
TabVector* TabFind::FindTabVector(int search_size_multiple,
                                  int min_gutter_width,
                                  TabAlignment alignment,
                                  BLOBNBOX* bbox,
                                  int* vertical_x, int* vertical_y) {
  int height = std::max(static_cast<int>(bbox->bounding_box().height()), gridsize());
  AlignedBlobParams align_params(*vertical_x, *vertical_y,
                                 height,
                                 search_size_multiple, min_gutter_width,
                                 resolution_, alignment);
  // FindVerticalAlignment is in the parent (AlignedBlob) class.
  return FindVerticalAlignment(align_params, bbox, vertical_x, vertical_y);
}

// Set the vertical_skew_ member from the given vector and refit
// all vectors parallel to the skew vector.
void TabFind::SetVerticalSkewAndParallelize(int vertical_x, int vertical_y) {
  // Fit the vertical vector into an ICOORD, which is 16 bit.
  vertical_skew_.set_with_shrink(vertical_x, vertical_y);
  if (textord_debug_tabfind)
    tprintf("Vertical skew vector=(%d,%d)\n",
            vertical_skew_.x(), vertical_skew_.y());
  v_it_.set_to_list(&vectors_);
  for (v_it_.mark_cycle_pt(); !v_it_.cycled_list(); v_it_.forward()) {
    TabVector* v = v_it_.data();
    v->Fit(vertical_skew_, true);
  }
  // Now sort the vectors as their direction has potentially changed.
  SortVectors();
}

// Sort all the current vectors using the given vertical direction vector.
void TabFind::SortVectors() {
  vectors_.sort(TabVector::SortVectorsByKey);
  v_it_.set_to_list(&vectors_);
}

// Evaluate all the current tab vectors.
void TabFind::EvaluateTabs() {
  TabVector_IT rule_it(&vectors_);
  for (rule_it.mark_cycle_pt(); !rule_it.cycled_list(); rule_it.forward()) {
    TabVector* tab = rule_it.data();
    if (!tab->IsSeparator()) {
      tab->Evaluate(vertical_skew_, this);
      if (tab->BoxCount() < kMinEvaluatedTabs) {
        if (textord_debug_tabfind > 2)
          tab->Print("Too few boxes");
        delete rule_it.extract();
        v_it_.set_to_list(&vectors_);
      } else if (WithinTestRegion(3, tab->startpt().x(), tab->startpt().y())) {
        tab->Print("Evaluated tab");
      }
    }
  }
}

// Trace textlines from one side to the other of each tab vector, saving
// the most frequent column widths found in a list so that a given width
// can be tested for being a common width with a simple callback function.
void TabFind::ComputeColumnWidths(ScrollView* tab_win,
                                  ColPartitionGrid* part_grid) {
  #ifndef GRAPHICS_DISABLED
  if (tab_win != nullptr)
    tab_win->Pen(ScrollView::WHITE);
  #endif  // GRAPHICS_DISABLED
  // Accumulate column sections into a STATS
  int col_widths_size = (tright_.x() - bleft_.x()) / kColumnWidthFactor;
  STATS col_widths(0, col_widths_size + 1);
  ApplyPartitionsToColumnWidths(part_grid, &col_widths);
  #ifndef GRAPHICS_DISABLED
  if (tab_win != nullptr) {
    tab_win->Update();
  }
  #endif  // GRAPHICS_DISABLED
  if (textord_debug_tabfind > 1)
    col_widths.print();
  // Now make a list of column widths.
  MakeColumnWidths(col_widths_size, &col_widths);
  // Turn the column width into a range.
  ApplyPartitionsToColumnWidths(part_grid, nullptr);
}

// Finds column width and:
//   if col_widths is not null (pass1):
//     pair-up tab vectors with existing ColPartitions and accumulate widths.
//   else (pass2):
//     find the largest real partition width for each recorded column width,
//     to be used as the minimum acceptable width.
void TabFind::ApplyPartitionsToColumnWidths(ColPartitionGrid* part_grid,
                                            STATS* col_widths) {
  // For every ColPartition in the part_grid, add partners to the tabvectors
  // and accumulate the column widths.
  ColPartitionGridSearch gsearch(part_grid);
  gsearch.StartFullSearch();
  ColPartition* part;
  while ((part = gsearch.NextFullSearch()) != nullptr) {
    BLOBNBOX_C_IT blob_it(part->boxes());
    if (blob_it.empty())
      continue;
    BLOBNBOX* left_blob = blob_it.data();
    blob_it.move_to_last();
    BLOBNBOX* right_blob = blob_it.data();
    TabVector* left_vector = LeftTabForBox(left_blob->bounding_box(),
                                           true, false);
    if (left_vector == nullptr || left_vector->IsRightTab())
      continue;
    TabVector* right_vector = RightTabForBox(right_blob->bounding_box(),
                                             true, false);
    if (right_vector == nullptr || right_vector->IsLeftTab())
      continue;

    int line_left = left_vector->XAtY(left_blob->bounding_box().bottom());
    int line_right = right_vector->XAtY(right_blob->bounding_box().bottom());
    // Add to STATS of measurements if the width is significant.
    int width = line_right - line_left;
    if (col_widths != nullptr) {
      AddPartnerVector(left_blob, right_blob, left_vector, right_vector);
      if (width >= kMinColumnWidth)
        col_widths->add(width / kColumnWidthFactor, 1);
    } else {
      width /= kColumnWidthFactor;
      ICOORDELT_IT it(&column_widths_);
      for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
        ICOORDELT* w = it.data();
        if (NearlyEqual<int>(width, w->y(), 1)) {
          int true_width = part->bounding_box().width() / kColumnWidthFactor;
          if (true_width <= w->y() && true_width > w->x())
            w->set_x(true_width);
          break;
        }
      }
    }
  }
}

// Helper makes the list of common column widths in column_widths_ from the
// input col_widths. Destroys the content of col_widths by repeatedly
// finding the mode and erasing the peak.
void TabFind::MakeColumnWidths(int col_widths_size, STATS* col_widths) {
  ICOORDELT_IT w_it(&column_widths_);
  int total_col_count = col_widths->get_total();
  while (col_widths->get_total() > 0) {
    int width = col_widths->mode();
    int col_count = col_widths->pile_count(width);
    col_widths->add(width, -col_count);
    // Get the entire peak.
    for (int left = width - 1; left > 0 &&
         col_widths->pile_count(left) > 0;
         --left) {
      int new_count = col_widths->pile_count(left);
      col_count += new_count;
      col_widths->add(left, -new_count);
    }
    for (int right = width + 1; right < col_widths_size &&
         col_widths->pile_count(right) > 0;
         ++right) {
      int new_count = col_widths->pile_count(right);
      col_count += new_count;
      col_widths->add(right, -new_count);
    }
    if (col_count > kMinLinesInColumn &&
        col_count > kMinFractionalLinesInColumn * total_col_count) {
      ICOORDELT* w = new ICOORDELT(0, width);
      w_it.add_after_then_move(w);
      if (textord_debug_tabfind)
        tprintf("Column of width %d has %d = %.2f%% lines\n",
              width * kColumnWidthFactor, col_count,
              100.0 * col_count / total_col_count);
    }
  }
}

// Mark blobs as being in a vertical text line where that is the case.
// Returns true if the majority of the image is vertical text lines.
void TabFind::MarkVerticalText() {
  if (textord_debug_tabfind)
    tprintf("Checking for vertical lines\n");
  BlobGridSearch gsearch(this);
  gsearch.StartFullSearch();
  BLOBNBOX* blob = nullptr;
  while ((blob = gsearch.NextFullSearch()) != nullptr) {
    if (blob->region_type() < BRT_UNKNOWN)
      continue;
    if (blob->UniquelyVertical()) {
      blob->set_region_type(BRT_VERT_TEXT);
    }
  }
}

int TabFind::FindMedianGutterWidth(TabVector_LIST *lines) {
  TabVector_IT it(lines);
  int prev_right = -1;
  int max_gap = static_cast<int>(kMaxGutterWidthAbsolute * resolution_);
  STATS gaps(0, max_gap);
  STATS heights(0, max_gap);
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    TabVector* v = it.data();
    TabVector* partner = v->GetSinglePartner();
    if (!v->IsLeftTab() || v->IsSeparator() || !partner) continue;
    heights.add(partner->startpt().x() - v->startpt().x(), 1);
    if (prev_right > 0 && v->startpt().x() > prev_right) {
      gaps.add(v->startpt().x() - prev_right, 1);
    }
    prev_right = partner->startpt().x();
  }
  if (textord_debug_tabfind)
    tprintf("TabGutter total %d  median_gap %.2f  median_hgt %.2f\n",
            gaps.get_total(), gaps.median(), heights.median());
  if (gaps.get_total() < kMinLinesInColumn) return 0;
  return static_cast<int>(gaps.median());
}

// Find the next adjacent (looking to the left or right) blob on this text
// line, with the constraint that it must vertically significantly overlap
// the [top_y, bottom_y] range.
// If ignore_images is true, then blobs with aligned_text() < 0 are treated
// as if they do not exist.
BLOBNBOX* TabFind::AdjacentBlob(const BLOBNBOX* bbox,
                                bool look_left, bool ignore_images,
                                double min_overlap_fraction,
                                int gap_limit, int top_y, int bottom_y) {
  GridSearch<BLOBNBOX, BLOBNBOX_CLIST, BLOBNBOX_C_IT> sidesearch(this);
  const TBOX& box = bbox->bounding_box();
  int left = box.left();
  int right = box.right();
  int mid_x = (left + right) / 2;
  sidesearch.StartSideSearch(mid_x, bottom_y, top_y);
  int best_gap = 0;
  bool debug = WithinTestRegion(3, left, bottom_y);
  BLOBNBOX* result = nullptr;
  BLOBNBOX* neighbour = nullptr;
  while ((neighbour = sidesearch.NextSideSearch(look_left)) != nullptr) {
    if (debug) {
      tprintf("Adjacent blob: considering box:");
      neighbour->bounding_box().print();
    }
    if (neighbour == bbox ||
        (ignore_images && neighbour->region_type() < BRT_UNKNOWN))
      continue;
    const TBOX& nbox = neighbour->bounding_box();
    int n_top_y = nbox.top();
    int n_bottom_y = nbox.bottom();
    int v_overlap = std::min(n_top_y, top_y) - std::max(n_bottom_y, bottom_y);
    int height = top_y - bottom_y;
    int n_height = n_top_y - n_bottom_y;
    if (v_overlap > min_overlap_fraction * std::min(height, n_height) &&
        (min_overlap_fraction == 0.0 || !DifferentSizes(height, n_height))) {
      int n_left = nbox.left();
      int n_right = nbox.right();
      int h_gap = std::max(n_left, left) - std::min(n_right, right);
      int n_mid_x = (n_left + n_right) / 2;
      if (look_left == (n_mid_x < mid_x) && n_mid_x != mid_x) {
        if (h_gap > gap_limit) {
          // Hit a big gap before next tab so don't return anything.
          if (debug)
            tprintf("Giving up due to big gap = %d vs %d\n",
                    h_gap, gap_limit);
          return result;
        }
        if (h_gap > 0 && (look_left ? neighbour->right_tab_type()
                          : neighbour->left_tab_type()) >= TT_CONFIRMED) {
          // Hit a tab facing the wrong way. Stop in case we are crossing
          // the column boundary.
          if (debug)
            tprintf("Collision with like tab of type %d at %d,%d\n",
                    look_left ? neighbour->right_tab_type()
                                  : neighbour->left_tab_type(),
                    n_left, nbox.bottom());
          return result;
        }
        // This is a good fit to the line. Continue with this
        // neighbour as the bbox if the best gap.
        if (result == nullptr || h_gap < best_gap) {
          if (debug)
            tprintf("Good result\n");
          result = neighbour;
          best_gap = h_gap;
        } else {
          // The new one is worse, so we probably already have the best result.
          return result;
        }
      } else if (debug) {
        tprintf("Wrong way\n");
      }
    } else if (debug) {
      tprintf("Insufficient overlap\n");
    }
  }
  if (WithinTestRegion(3, left, box.top()))
    tprintf("Giving up due to end of search\n");
  return result;  // Hit the edge and found nothing.
}

// Add a bi-directional partner relationship between the left
// and the right. If one (or both) of the vectors is a separator,
// extend a nearby extendable vector or create a new one of the
// correct type, using the given left or right blob as a guide.
void TabFind::AddPartnerVector(BLOBNBOX* left_blob, BLOBNBOX* right_blob,
                               TabVector* left, TabVector* right) {
  const TBOX& left_box = left_blob->bounding_box();
  const TBOX& right_box = right_blob->bounding_box();
  if (left->IsSeparator()) {
    // Try to find a nearby left edge to extend.
    TabVector* v = LeftTabForBox(left_box, true, true);
    if (v != nullptr && v != left && v->IsLeftTab() &&
        v->XAtY(left_box.top()) > left->XAtY(left_box.top())) {
      left = v;  // Found a good replacement.
      left->ExtendToBox(left_blob);
    } else {
      // Fake a vector.
      left = new TabVector(*left, TA_LEFT_RAGGED, vertical_skew_, left_blob);
      vectors_.add_sorted(TabVector::SortVectorsByKey, left);
      v_it_.move_to_first();
    }
  }
  if (right->IsSeparator()) {
    // Try to find a nearby left edge to extend.
    if (WithinTestRegion(3, right_box.right(), right_box.bottom())) {
      tprintf("Box edge (%d,%d-%d)",
              right_box.right(), right_box.bottom(), right_box.top());
      right->Print(" looking for improvement for");
    }
    TabVector* v = RightTabForBox(right_box, true, true);
    if (v != nullptr && v != right && v->IsRightTab() &&
        v->XAtY(right_box.top()) < right->XAtY(right_box.top())) {
      right = v;  // Found a good replacement.
      right->ExtendToBox(right_blob);
      if (WithinTestRegion(3, right_box.right(), right_box.bottom())) {
        right->Print("Extended vector");
      }
    } else {
      // Fake a vector.
      right = new TabVector(*right, TA_RIGHT_RAGGED, vertical_skew_,
                            right_blob);
      vectors_.add_sorted(TabVector::SortVectorsByKey, right);
      v_it_.move_to_first();
      if (WithinTestRegion(3, right_box.right(), right_box.bottom())) {
        right->Print("Created new vector");
      }
    }
  }
  left->AddPartner(right);
  right->AddPartner(left);
}

// Remove separators and unused tabs from the main vectors_ list
// to the dead_vectors_ list.
void TabFind::CleanupTabs() {
  // TODO(rays) Before getting rid of separators and unused vectors, it
  // would be useful to try moving ragged vectors outwards to see if this
  // allows useful extension. Could be combined with checking ends of partners.
  TabVector_IT it(&vectors_);
  TabVector_IT dead_it(&dead_vectors_);
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    TabVector* v = it.data();
    if (v->IsSeparator() || v->Partnerless()) {
      dead_it.add_after_then_move(it.extract());
      v_it_.set_to_list(&vectors_);
    } else {
      v->FitAndEvaluateIfNeeded(vertical_skew_, this);
    }
  }
}

// Apply the given rotation to the given list of blobs.
void TabFind::RotateBlobList(const FCOORD& rotation, BLOBNBOX_LIST* blobs) {
  BLOBNBOX_IT it(blobs);
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    it.data()->rotate_box(rotation);
  }
}

// Recreate the grid with deskewed BLOBNBOXes.
// Returns false if the detected skew angle is impossible.
bool TabFind::Deskew(TabVector_LIST* hlines, BLOBNBOX_LIST* image_blobs,
                     TO_BLOCK* block, FCOORD* deskew, FCOORD* reskew) {
  ComputeDeskewVectors(deskew, reskew);
  if (deskew->x() < kCosMaxSkewAngle)
    return false;
  RotateBlobList(*deskew, image_blobs);
  RotateBlobList(*deskew, &block->blobs);
  RotateBlobList(*deskew, &block->small_blobs);
  RotateBlobList(*deskew, &block->noise_blobs);

  // Rotate the horizontal vectors. The vertical vectors don't need
  // rotating as they can just be refitted.
  TabVector_IT h_it(hlines);
  for (h_it.mark_cycle_pt(); !h_it.cycled_list(); h_it.forward()) {
    TabVector* h = h_it.data();
    h->Rotate(*deskew);
  }
  TabVector_IT d_it(&dead_vectors_);
  for (d_it.mark_cycle_pt(); !d_it.cycled_list(); d_it.forward()) {
    TabVector* d = d_it.data();
    d->Rotate(*deskew);
  }
  SetVerticalSkewAndParallelize(0, 1);
  // Rebuild the grid to the new size.
  TBOX grid_box(bleft_, tright_);
  grid_box.rotate_large(*deskew);
  Init(gridsize(), grid_box.botleft(), grid_box.topright());
  InsertBlobsToGrid(false, false, image_blobs, this);
  InsertBlobsToGrid(true, false, &block->blobs, this);
  return true;
}

// Flip the vertical and horizontal lines and rotate the grid ready
// for working on the rotated image.
// This also makes parameter adjustments for FindInitialTabVectors().
void TabFind::ResetForVerticalText(const FCOORD& rotate, const FCOORD& rerotate,
                                   TabVector_LIST* horizontal_lines,
                                   int* min_gutter_width) {
  // Rotate the horizontal and vertical vectors and swap them over.
  // Only the separators are kept and rotated; other tabs are used
  // to estimate the gutter width then thrown away.
  TabVector_LIST ex_verticals;
  TabVector_IT ex_v_it(&ex_verticals);
  TabVector_LIST vlines;
  TabVector_IT v_it(&vlines);
  while (!v_it_.empty()) {
    TabVector* v = v_it_.extract();
    if (v->IsSeparator()) {
      v->Rotate(rotate);
      ex_v_it.add_after_then_move(v);
    } else {
      v_it.add_after_then_move(v);
    }
    v_it_.forward();
  }

  // Adjust the min gutter width for better tabbox selection
  // in 2nd call to FindInitialTabVectors().
  int median_gutter = FindMedianGutterWidth(&vlines);
  if (median_gutter > *min_gutter_width)
    *min_gutter_width = median_gutter;

  TabVector_IT h_it(horizontal_lines);
  for (h_it.mark_cycle_pt(); !h_it.cycled_list(); h_it.forward()) {
    TabVector* h = h_it.data();
    h->Rotate(rotate);
  }
  v_it_.add_list_after(horizontal_lines);
  v_it_.move_to_first();
  h_it.set_to_list(horizontal_lines);
  h_it.add_list_after(&ex_verticals);

  // Rebuild the grid to the new size.
  TBOX grid_box(bleft(), tright());
  grid_box.rotate_large(rotate);
  Init(gridsize(), grid_box.botleft(), grid_box.topright());
}

// Clear the grid and get rid of the tab vectors, but not separators,
// ready to start again.
void TabFind::Reset() {
  v_it_.move_to_first();
  for (v_it_.mark_cycle_pt(); !v_it_.cycled_list(); v_it_.forward()) {
    if (!v_it_.data()->IsSeparator())
      delete v_it_.extract();
  }
  Clear();
}

// Reflect the separator tab vectors and the grids in the y-axis.
// Can only be called after Reset!
void TabFind::ReflectInYAxis() {
  TabVector_LIST temp_list;
  TabVector_IT temp_it(&temp_list);
  v_it_.move_to_first();
  // The TabVector list only contains vertical lines, but they need to be
  // reflected and the list needs to be reversed, so they are still in
  // sort_key order.
  while (!v_it_.empty()) {
    TabVector* v = v_it_.extract();
    v_it_.forward();
    v->ReflectInYAxis();
    temp_it.add_before_then_move(v);
  }
  v_it_.add_list_after(&temp_list);
  v_it_.move_to_first();
  // Reset this grid with reflected bounding boxes.
  TBOX grid_box(bleft(), tright());
  int tmp = grid_box.left();
  grid_box.set_left(-grid_box.right());
  grid_box.set_right(-tmp);
  Init(gridsize(), grid_box.botleft(), grid_box.topright());
}

// Compute the rotation required to deskew, and its inverse rotation.
void TabFind::ComputeDeskewVectors(FCOORD* deskew, FCOORD* reskew) {
  double length = vertical_skew_ % vertical_skew_;
  length = sqrt(length);
  deskew->set_x(static_cast<float>(vertical_skew_.y() / length));
  deskew->set_y(static_cast<float>(vertical_skew_.x() / length));
  reskew->set_x(deskew->x());
  reskew->set_y(-deskew->y());
}

// Compute and apply constraints to the end positions of TabVectors so
// that where possible partners end at the same y coordinate.
void TabFind::ApplyTabConstraints() {
  TabVector_IT it(&vectors_);
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    TabVector* v = it.data();
    v->SetupConstraints();
  }
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    TabVector* v = it.data();
    // With the first and last partner, we want a common bottom and top,
    // respectively, and for each change of partner, we want a common
    // top of first with bottom of next.
    v->SetupPartnerConstraints();
  }
  // TODO(rays) The back-to-back pairs should really be done like the
  // front-to-front pairs, but there is no convenient way of producing the
  // list of partners like there is with the front-to-front.
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    TabVector* v = it.data();
    if (!v->IsRightTab())
      continue;
    // For each back-to-back pair of vectors, try for common top and bottom.
    TabVector_IT partner_it(it);
    for (partner_it.forward(); !partner_it.at_first(); partner_it.forward()) {
      TabVector* partner = partner_it.data();
      if (!partner->IsLeftTab() || !v->VOverlap(*partner))
        continue;
      v->SetupPartnerConstraints(partner);
    }
  }
  // Now actually apply the constraints to get common start/end points.
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    TabVector* v = it.data();
    if (!v->IsSeparator())
      v->ApplyConstraints();
  }
  // TODO(rays) Where constraint application fails, it would be good to try
  // checking the ends to see if they really should be moved.
}

}  // namespace tesseract.
