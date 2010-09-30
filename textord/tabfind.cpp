///////////////////////////////////////////////////////////////////////
// File:        TabFind.cpp
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

#include "tabfind.h"
#include "alignedblob.h"
#include "blobbox.h"
#include "detlinefit.h"
#include "linefind.h"
#include "ndminx.h"

// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif

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
// Fraction of height used as a minimum gap for aligned blobs.
const double kAlignedGapFraction = 0.75;
// Multiplier of new y positions in running average for skew estimation.
const double kSmoothFactor = 0.25;
// Min coverage for a good baseline between vectors
const double kMinBaselineCoverage = 0.5;
// Minimum overlap fraction when scanning text lines for column widths.
const double kCharVerticalOverlapFraction = 0.375;
// Maximum horizontal gap allowed when scanning for column widths
const double kMaxHorizontalGap = 3.0;
// Maximum upper quartile error allowed on a baseline fit as a fraction
// of height.
const double kMaxBaselineError = 0.4375;
// Min number of points to accept after evaluation.
const int kMinEvaluatedTabs = 3;
// Minimum aspect ratio of a textline to make a good textline blob with a
// single blob.
const int kMaxTextLineBlobRatio = 5;
// Minimum aspect ratio of a textline to make a good textline blob with
// multiple blobs. Target ratio varies according to number of blobs.
const int kMinTextLineBlobRatio = 3;
// Fraction of box area covered by image to make a blob image.
const double kMinImageArea = 0.5;

BOOL_VAR(textord_tabfind_show_initialtabs, false, "Show tab candidates");
BOOL_VAR(textord_tabfind_show_finaltabs, false, "Show tab vectors");
BOOL_VAR(textord_tabfind_vertical_text, true, "Enable vertical detection");

TabFind::TabFind(int gridsize, const ICOORD& bleft, const ICOORD& tright,
                 TabVector_LIST* vlines, int vertical_x, int vertical_y)
  : AlignedBlob(gridsize, bleft, tright),
    image_origin_(0, tright.y() - 1),
    tab_grid_(new BBGrid<BLOBNBOX, BLOBNBOX_CLIST, BLOBNBOX_C_IT>(gridsize,
                                                                  bleft,
                                                                  tright)) {
  resolution_ = 0;
  width_cb_ = NULL;
  v_it_.set_to_list(&vectors_);
  v_it_.add_list_after(vlines);
  SetVerticalSkewAndParellelize(vertical_x, vertical_y);
  width_cb_ = NewPermanentCallback(this, &TabFind::CommonWidth);
}

TabFind::~TabFind() {
  delete tab_grid_;
  if (width_cb_ != NULL)
    delete width_cb_;
}

///////////////// PUBLIC functions (mostly used by TabVector). //////////////

// Insert a list of blobs into the given grid (not necessarily this).
// If take_ownership is true, then the blobs are removed from the source list.
// See InsertBlob for the other arguments.
void TabFind::InsertBlobList(bool h_spread, bool v_spread, bool large,
                             BLOBNBOX_LIST* blobs, bool take_ownership,
                             BBGrid<BLOBNBOX, BLOBNBOX_CLIST,
                                    BLOBNBOX_C_IT>* grid) {
  BLOBNBOX_IT blob_it(blobs);
  int b_count = 0;
  int reject_count = 0;
  for (blob_it.mark_cycle_pt(); !blob_it.cycled_list(); blob_it.forward()) {
    BLOBNBOX* blob = blob_it.data();
    if (InsertBlob(h_spread, v_spread, large, blob, grid)) {
      ++b_count;
    } else {
      ++reject_count;
    }
    if (take_ownership)
      blob_it.extract();
  }
  if (textord_debug_tabfind) {
    if (large)
      tprintf("Inserted %d large blobs into grid, %d rejected\n",
              b_count, reject_count);
    else
      tprintf("Inserted %d normal blobs into grid\n", b_count);
  }
}

// Insert a single blob into the given grid (not necessarily this).
// If h_spread, then all cells covered horizontally by the box are
// used, otherwise, just the bottom-left. Similarly for v_spread.
// If large, then insert only if the bounding box doesn't intersect
// anything else already in the grid. Returns true if the blob was inserted.
// A side effect is that the left and right rule edges of the blob are
// set according to the tab vectors in this (not grid).
bool TabFind::InsertBlob(bool h_spread, bool v_spread, bool large,
                         BLOBNBOX* blob,
                         BBGrid<BLOBNBOX, BLOBNBOX_CLIST,
                                BLOBNBOX_C_IT>* grid) {
  TBOX box = blob->bounding_box();
  blob->set_left_rule(LeftEdgeForBox(box, false, false));
  blob->set_right_rule(RightEdgeForBox(box, false, false));
  blob->set_left_crossing_rule(LeftEdgeForBox(box, true, false));
  blob->set_right_crossing_rule(RightEdgeForBox(box, true, false));
  if (large) {
    // Search the grid to see what intersects it.
    // Setup a Rectangle search for overlapping this blob.
    GridSearch<BLOBNBOX, BLOBNBOX_CLIST, BLOBNBOX_C_IT> rsearch(grid);
    rsearch.StartRectSearch(box);
    BLOBNBOX* neighbour = rsearch.NextRectSearch();
    if (neighbour != NULL && box.major_overlap(neighbour->bounding_box())) {
      if (textord_debug_tabfind) {
        TBOX n_box = neighbour->bounding_box();
        tprintf("Blob at (%d,%d)->(%d,%d) significantly overlaps blob"
                " at (%d,%d)->(%d,%d)\n",
                box.left(), box.top(), box.right(), box.bottom(),
                n_box.left(), n_box.top(), n_box.right(), n_box.bottom());
      }
      return false;
    }
  }
  grid->InsertBBox(h_spread, v_spread, blob);
  return true;
}

// Find the gutter width and distance to inner neighbour for the given blob.
void TabFind::GutterWidthAndNeighbourGap(int tab_x, int mean_height,
                                         int max_gutter, bool left,
                                         BLOBNBOX* bbox, int* gutter_width,
                                         int* neighbour_gap ) {
  const TBOX& box = bbox->bounding_box();
  int height = box.height();
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
  // Find the nearest blob on the outside of the column.
  BLOBNBOX* gutter_bbox = AdjacentBlob(bbox, left, *gutter_width);
  if (gutter_bbox != NULL) {
    TBOX gutter_box = gutter_bbox->bounding_box();
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
  BLOBNBOX* neighbour = AdjacentBlob(bbox, !left, *gutter_width);
  int neighbour_edge = left ? RightEdgeForBox(box, true, false)
                            : LeftEdgeForBox(box, true, false);
  if (neighbour != NULL) {
    TBOX n_box = neighbour->bounding_box();
    if (!DifferentSizes(height, n_box.height())) {
      if (left && n_box.left() < neighbour_edge)
        neighbour_edge = n_box.left();
      else if (!left && n_box.right() > neighbour_edge)
        neighbour_edge = n_box.right();
    }
  }
  *neighbour_gap = left ? neighbour_edge - internal_x
                        : internal_x - neighbour_edge;
}

// Find the next adjacent (to left or right) blob on this text line,
// with the constraint that it must vertically significantly overlap
// the input box.
BLOBNBOX* TabFind::AdjacentBlob(const BLOBNBOX* bbox,
                                bool right_to_left, int gap_limit) {
  const TBOX& box = bbox->bounding_box();
  return AdjacentBlob(bbox, right_to_left, false, gap_limit,
                      box.top(), box.bottom());
}

// Compute and return, but do not set the type as being BRT_TEXT or
// BRT_UNKNOWN according to how well it forms a text line.
BlobRegionType TabFind::ComputeBlobType(BLOBNBOX* blob) {
  // Check the text line width.
  TBOX box = blob->bounding_box();
  int blob_count;
  int total_blobs;
  int width = FindTextlineWidth(true, blob, &total_blobs);
  width += FindTextlineWidth(false, blob, &blob_count);
  total_blobs += blob_count - 1;
  int target_ratio = kMaxTextLineBlobRatio - (total_blobs - 1);
  if (target_ratio < kMinTextLineBlobRatio)
    target_ratio = kMinTextLineBlobRatio;
  BlobRegionType blob_type = (width >= box.height() * target_ratio)
                           ? BRT_TEXT : BRT_UNKNOWN;
  if (WithinTestRegion(3, box.left(), box.bottom()))
    tprintf("Line width = %d, target = %d, result = %d\n",
            width, box.height() * target_ratio, blob_type);
  return blob_type;
}

// Return the x-coord that corresponds to the right edge for the given
// box. If there is a rule line to the right that vertically overlaps it,
// then return the x-coord of the rule line, otherwise return the right
// edge of the page. For details see RightTabForBox below.
int TabFind::RightEdgeForBox(const TBOX& box, bool crossing, bool extended) {
  TabVector* v = RightTabForBox(box, crossing, extended);
  return v == NULL ? tright_.x() : v->XAtY((box.top() + box.bottom()) / 2);
}
// As RightEdgeForBox, but finds the left Edge instead.
int TabFind::LeftEdgeForBox(const TBOX& box, bool crossing, bool extended) {
  TabVector* v = LeftTabForBox(box, crossing, extended);
  return v == NULL ? bleft_.x() : v->XAtY((box.top() + box.bottom()) / 2);
}

// Return true if the given width is close to one of the common
// widths in column_widths_.
bool TabFind::CommonWidth(int width) {
  width /= kColumnWidthFactor;
  ICOORDELT_IT it(&column_widths_);
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    ICOORDELT* w = it.data();
    if (NearlyEqual<int>(width, w->x(), 1))
      return true;
  }
  return false;
}

// Return true if the sizes are more than a
// factor of 2 different.
bool TabFind::DifferentSizes(int size1, int size2) {
  return size1 > size2 * 2 || size2 > size1 * 2;
}

///////////////// PROTECTED functions (used by ColumnFinder). //////////////

// Top-level function to find TabVectors in an input page block.
void TabFind::FindTabVectors(int resolution, TabVector_LIST* hlines,
                             BLOBNBOX_LIST* image_blobs, TO_BLOCK* block,
                             FCOORD* reskew, FCOORD* rerotate) {
  resolution_ = resolution;
  *rerotate = FCOORD(1.0f, 0.0f);
  FindInitialTabVectors(image_blobs, block);
  if (textord_tabfind_vertical_text && TextMostlyVertical()) {
    ResetForVerticalText(hlines, image_blobs, block, rerotate);
    FindInitialTabVectors(image_blobs, block);
  }
  TabVector::MergeSimilarTabVectors(vertical_skew_, &vectors_, this);
  SortVectors();
  CleanupTabs();
  Deskew(hlines, image_blobs, block, reskew);
  ApplyTabConstraints();
  if (textord_tabfind_show_finaltabs) {
    ScrollView* tab_win = MakeWindow(640, 50, "FinalTabs");
    if (textord_debug_images) {
      tab_win->Image(AlignedBlob::textord_debug_pix().string(),
                     image_origin_.x(), image_origin_.y());
    } else {
      DisplayBoxes(tab_win);
      DisplayTabs("FinalTabs", tab_win);
    }
    tab_win = DisplayTabVectors(tab_win);
  }
}

// Top-level function to not find TabVectors in an input page block,
// but setup for single column mode.
void TabFind::DontFindTabVectors(int resolution, BLOBNBOX_LIST* image_blobs,
                                 TO_BLOCK* block, FCOORD* reskew) {
  resolution_ = resolution;
  InsertBlobList(false, false, false, image_blobs, false, this);
  InsertBlobList(true, false, false, &block->blobs, false, this);
  ComputeBlobGoodness();
  reskew->set_x(1);
  reskew->set_y(0);
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
    return NULL;
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
  TabVector* best_v = NULL;
  int best_x = -1;
  int key_limit = -1;
  do {
    TabVector* v = v_it_.data();
    int x = v->XAtY(mid_y);
    if (x >= right &&
        (v->VOverlap(top_y, bottom_y) > 0 ||
         (extended && v->ExtendedOverlap(top_y, bottom_y) > 0))) {
      if (best_v == NULL || x < best_x) {
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
        (best_v != NULL && v->sort_key() > key_limit))
      break;  // Prevent restarting list for next call.
    v_it_.forward();
  } while (!v_it_.at_first());
  return best_v;
}

// As RightTabForBox, but finds the left TabVector instead.
TabVector* TabFind::LeftTabForBox(const TBOX& box, bool crossing,
                                  bool extended) {
  if (v_it_.empty())
    return NULL;
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
  TabVector* best_v = NULL;
  int best_x = -1;
  int key_limit = -1;
  do {
    TabVector* v = v_it_.data();
    int x = v->XAtY(mid_y);
    if (x <= left &&
        (v->VOverlap(top_y, bottom_y) > 0 ||
         (extended && v->ExtendedOverlap(top_y, bottom_y) > 0))) {
      if (best_v == NULL || x > best_x) {
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
        (best_v != NULL && v->sort_key() < key_limit))
      break;  // Prevent restarting list for next call.
    v_it_.backward();
  } while (!v_it_.at_last());
  return best_v;
}

// Helper function to setup search limits for *TabForBox.
void TabFind::SetupTabSearch(int x, int y, int* min_key, int* max_key) {
  int key1 = TabVector::SortKey(vertical_skew_, x, (y + tright_.y()) / 2);
  int key2 = TabVector::SortKey(vertical_skew_, x, (y + bleft_.y()) / 2);
  *min_key = MIN(key1, key2);
  *max_key = MAX(key1, key2);
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
void TabFind::FindInitialTabVectors(BLOBNBOX_LIST* image_blobs,
                                    TO_BLOCK* block) {
  if (textord_tabfind_show_initialtabs) {
    ScrollView* line_win = MakeWindow(0, 0, "VerticalLines");
    line_win = DisplayTabVectors(line_win);
  }
  // Prepare the grid.
  InsertBlobList(false, false, false, image_blobs, false, this);
  InsertBlobList(true, false, false, &block->blobs, false, this);
  ScrollView* initial_win = FindTabBoxes();
  FindAllTabVectors();
  if (textord_tabfind_show_initialtabs)
    initial_win = DisplayTabVectors(initial_win);

  TabVector::MergeSimilarTabVectors(vertical_skew_, &vectors_, this);
  SortVectors();
  EvaluateTabs();
  ComputeColumnWidths(initial_win);
  if (textord_tabfind_vertical_text)
    MarkVerticalText();
}

// For each box in the grid, decide whether it is a candidate tab-stop,
// and if so add it to the tab_grid_.
ScrollView* TabFind::FindTabBoxes() {
  // For every bbox in the grid, determine whether it uses a tab on an edge.
  GridSearch<BLOBNBOX, BLOBNBOX_CLIST, BLOBNBOX_C_IT> gsearch(this);
  gsearch.StartFullSearch();
  BLOBNBOX* bbox;
  while ((bbox = gsearch.NextFullSearch()) != NULL) {
    if (TestBoxForTabs(bbox)) {
      // If it is any kind of tab, insert it into the tab grid.
      tab_grid_->InsertBBox(false, false, bbox);
    }
  }
  ScrollView* tab_win = NULL;
  if (textord_tabfind_show_initialtabs) {
    tab_win = tab_grid_->MakeWindow(0, 100, "InitialTabs");
    tab_grid_->DisplayBoxes(tab_win);
    tab_win = DisplayTabs("Tabs", tab_win);
  }
  return tab_win;
}

bool TabFind::TestBoxForTabs(BLOBNBOX* bbox) {
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
  int target_right = left_x - height * kAlignedGapFraction;
  int target_left = right_x + height * kAlignedGapFraction;
  // We will be evaluating whether the left edge could be a left tab, and
  // whether the right edge could be a right tab.
  // A box can be a tab if its bool is_(left/right)_tab remains true, meaning
  // that no blobs have been found in the gutter during the radial search.
  // A box can also be a tab if there are objects in the gutter only above
  // or only below, and there are aligned objects on the opposite side, but
  // not too many unaligned objects. The maybe_(left/right)_tab_up counts
  // aligned objects above and negatively counts unaligned objects above,
  // and is set to -MAX_INT32 if a gutter object is found above.
  // The other 3 maybe ints work similarly for the other sides.
  bool is_left_tab = true;
  bool is_right_tab = true;
  int maybe_left_tab_up = 0;
  int maybe_right_tab_up = 0;
  int maybe_left_tab_down = 0;
  int maybe_right_tab_down = 0;
  int alignment_tolerance = static_cast<int>(resolution_ * kAlignedFraction);
  BLOBNBOX* neighbour = NULL;
  while ((neighbour = radsearch.NextRadSearch()) != NULL) {
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
        maybe_left_tab_down = -MAX_INT32;
      if (n_mid_y > bottom_y)
        maybe_left_tab_up = -MAX_INT32;
    } else if (NearlyEqual(left_x, n_left, alignment_tolerance)) {
      if (debug)
        tprintf("Maybe a left tab\n");
      if (n_mid_y > top_y && maybe_left_tab_up > -MAX_INT32)
        ++maybe_left_tab_up;
      if (n_mid_y < bottom_y && maybe_left_tab_down > -MAX_INT32)
        ++maybe_left_tab_down;
    } else if (n_left < left_x && n_right >= left_x) {
      // Overlaps but not aligned so negative points on a maybe.
      if (debug)
        tprintf("Maybe Not a left tab\n");
      if (n_mid_y > top_y && maybe_left_tab_up > -MAX_INT32)
        --maybe_left_tab_up;
      if (n_mid_y < bottom_y && maybe_left_tab_down > -MAX_INT32)
        --maybe_left_tab_down;
    }
    if (n_mid_x >= right_x && n_left <= target_left) {
      if (debug)
        tprintf("Not a right tab\n");
      is_right_tab = false;
      if (n_mid_y < top_y)
        maybe_right_tab_down = -MAX_INT32;
      if (n_mid_y > bottom_y)
        maybe_right_tab_up = -MAX_INT32;
    } else if (NearlyEqual(right_x, n_right, alignment_tolerance)) {
      if (debug)
        tprintf("Maybe a right tab\n");
      if (n_mid_y > top_y && maybe_right_tab_up > -MAX_INT32)
        ++maybe_right_tab_up;
      if (n_mid_y < bottom_y && maybe_right_tab_down > -MAX_INT32)
        ++maybe_right_tab_down;
    } else if (n_right > right_x && n_left <= right_x) {
      // Overlaps but not aligned so negative points on a maybe.
      if (debug)
        tprintf("Maybe Not a right tab\n");
      if (n_mid_y > top_y && maybe_right_tab_up > -MAX_INT32)
        --maybe_right_tab_up;
      if (n_mid_y < bottom_y && maybe_right_tab_down > -MAX_INT32)
        --maybe_right_tab_down;
    }
    if (maybe_left_tab_down == -MAX_INT32 && maybe_left_tab_up == -MAX_INT32 &&
        maybe_right_tab_down == -MAX_INT32 && maybe_right_tab_up == -MAX_INT32)
      break;
  }
  if (is_left_tab || maybe_left_tab_up > 1 || maybe_left_tab_down > 1) {
    if (debug)
      tprintf("Setting left tab\n");
    bbox->set_left_tab_type(TT_UNCONFIRMED);
  }
  if (is_right_tab || maybe_right_tab_up > 1 || maybe_right_tab_down > 1) {
    if (debug)
      tprintf("Setting right tab\n");
    bbox->set_right_tab_type(TT_UNCONFIRMED);
  }
  return bbox->left_tab_type() != TT_NONE || bbox->right_tab_type() != TT_NONE;
}

void TabFind::FindAllTabVectors() {
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
                                      &dummy_vectors,
                                      &vertical_x, &vertical_y);
    vector_count += FindTabVectors(search_size, TA_RIGHT_ALIGNED,
                                   &dummy_vectors,
                                   &vertical_x, &vertical_y);
    if (vector_count > 0)
      break;
  }
  // Get rid of the test vectors and reset the types of the tabs.
  dummy_vectors.clear();
  GridSearch<BLOBNBOX, BLOBNBOX_CLIST, BLOBNBOX_C_IT> tsearch(tab_grid_);
  BLOBNBOX* bbox;
  tsearch.StartFullSearch();
  while ((bbox = tsearch.NextFullSearch()) != NULL) {
    if (bbox->left_tab_type() == TT_CONFIRMED)
      bbox->set_left_tab_type(TT_UNCONFIRMED);
    if (bbox->right_tab_type() == TT_CONFIRMED)
      bbox->set_right_tab_type(TT_UNCONFIRMED);
  }
  if (textord_debug_tabfind) {
    tprintf("Beginning real tab search with vertical = %d,%d...\n",
            vertical_x, vertical_y);
  }
  // Now do the real thing ,but keep the vectors in the dummy_vectors list
  // until they are all done, so we don't get the tab vectors confused with
  // the rule line vectors.
  FindTabVectors(kMaxVerticalSearch, TA_LEFT_ALIGNED,
                 &dummy_vectors, &vertical_x, &vertical_y);
  FindTabVectors(kMaxVerticalSearch, TA_RIGHT_ALIGNED,
                 &dummy_vectors, &vertical_x, &vertical_y);
  FindTabVectors(kMaxRaggedSearch, TA_LEFT_RAGGED,
                 &dummy_vectors, &vertical_x, &vertical_y);
  FindTabVectors(kMaxRaggedSearch, TA_RIGHT_RAGGED,
                 &dummy_vectors, &vertical_x, &vertical_y);
  // Now add the vectors to the vectors_ list.
  TabVector_IT v_it(&vectors_);
  v_it.add_list_after(&dummy_vectors);
  // Now use the summed (mean) vertical vector as the direction for everything.
  SetVerticalSkewAndParellelize(vertical_x, vertical_y);
}

// Helper for FindAllTabVectors finds the vectors of a particular type.
int TabFind::FindTabVectors(int search_size_multiple, TabAlignment alignment,
                            TabVector_LIST* vectors,
                            int* vertical_x, int* vertical_y) {
  TabVector_IT vector_it(vectors);
  int vector_count = 0;
  // Search the entire tab grid, looking for tab vectors.
  GridSearch<BLOBNBOX, BLOBNBOX_CLIST, BLOBNBOX_C_IT> tsearch(tab_grid_);
  BLOBNBOX* bbox;
  tsearch.StartFullSearch();
  bool right = alignment == TA_RIGHT_ALIGNED || alignment == TA_RIGHT_RAGGED;
  while ((bbox = tsearch.NextFullSearch()) != NULL) {
    if ((!right && bbox->left_tab_type() == TT_UNCONFIRMED) ||
        (right && bbox->right_tab_type() == TT_UNCONFIRMED)) {
      TabVector* vector = FindTabVector(search_size_multiple, alignment,
                                        bbox, vertical_x, vertical_y);
      if (vector != NULL) {
        ++vector_count;
        vector_it.add_to_end(vector);
        ICOORD merged_vector = vector->endpt();
        merged_vector -= vector->startpt();
        if (abs(merged_vector.x()) > 100) {
          vector->Debug("Garbage result of FindTabVector?");
        }
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
// Returns NULL if no decent tabstop can be found.
TabVector* TabFind::FindTabVector(int search_size_multiple,
                                  TabAlignment alignment,
                                  BLOBNBOX* bbox,
                                  int* vertical_x, int* vertical_y) {
  AlignedBlobParams align_params(*vertical_x, *vertical_y,
                                 bbox->bounding_box().height(),
                                 search_size_multiple, resolution_,
                                 alignment);
  // FindVerticalAlignment is in the parent (AlignedBlob) class.
  return FindVerticalAlignment(align_params, bbox, vertical_x, vertical_y);
}

// Set the vertical_skew_ member from the given vector and refit
// all vectors parallel to the skew vector.
void TabFind::SetVerticalSkewAndParellelize(int vertical_x, int vertical_y) {
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
void TabFind::ComputeColumnWidths(ScrollView* tab_win) {
  // Set the aligned_text_ member of each blob, so text lines traces
  // get terminated where there is a change from text to image.
  ComputeBlobGoodness();
  if (tab_win != NULL)
    tab_win->Pen(ScrollView::WHITE);
  // Accumulate column sections into a STATS
  int col_widths_size = (tright_.x() - bleft_.x()) /kColumnWidthFactor;
  STATS col_widths(0, col_widths_size + 1);
  // For every bbox in the tab grid, search for an opposite
  // to estimate column width and skew..
  GridSearch<BLOBNBOX, BLOBNBOX_CLIST, BLOBNBOX_C_IT> gsearch(tab_grid_);
  gsearch.StartFullSearch();
  BLOBNBOX* bbox;
  while ((bbox = gsearch.NextFullSearch()) != NULL) {
    ICOORD start_pt, end_pt;
    if (bbox->left_tab_type() != TT_CONFIRMED &&
        bbox->right_tab_type() != TT_CONFIRMED)
      continue;
    int line_left, line_right;
    if (TraceTextline(bbox, &start_pt, &end_pt, &line_left, &line_right)) {
      int left_y = (line_left - start_pt.x()) * (end_pt.y() - start_pt.y()) /
               (end_pt.x() - start_pt.x()) + start_pt.y();
      int right_y = (line_right - start_pt.x()) * (end_pt.y() - start_pt.y()) /
                (end_pt.x() - start_pt.x()) + start_pt.y();
      if (start_pt.x() != end_pt.x()) {
        if (WithinTestRegion(3, start_pt.x(), start_pt.y()))
          tprintf("Baseline from (%d,%d) to (%d,%d), started at (%d,%d)\n",
                  line_left, left_y, line_right, right_y,
                  bbox->bounding_box().left(), bbox->bounding_box().bottom());
        if (tab_win != NULL)
          tab_win->Line(line_left, left_y, line_right, right_y);
        // If line scan was successful, add to STATS of measurements.
        int width = line_right - line_left;
        if (width >= kMinColumnWidth) {
          col_widths.add(width / kColumnWidthFactor, 1);
        }
      }
    }
  }
  if (tab_win != NULL) {
    tab_win->Update();
  }
  // Now make a list of column widths.
  ICOORDELT_IT w_it(&column_widths_);
  int total_col_count = col_widths.get_total();
  while (col_widths.get_total() > 0) {
    int width = col_widths.mode();
    int col_count = col_widths.pile_count(width);
    col_widths.add(width, -col_count);
    // Get the entire peak.
    for (int left = width - 1; left > 0 &&
         col_widths.pile_count(left) > 0;
         --left) {
      int new_count = col_widths.pile_count(left);
      col_count += new_count;
      col_widths.add(left, -new_count);
    }
    for (int right = width + 1; right < col_widths_size &&
         col_widths.pile_count(right) > 0;
         ++right) {
      int new_count = col_widths.pile_count(right);
      col_count += new_count;
      col_widths.add(right, -new_count);
    }
    if (col_count > kMinLinesInColumn &&
        col_count > kMinFractionalLinesInColumn * total_col_count) {
      ICOORDELT* w = new ICOORDELT(width, col_count);
      w_it.add_after_then_move(w);
      if (textord_debug_tabfind)
        tprintf("Column of width %d has %d = %.2f%% lines\n",
              width * kColumnWidthFactor, col_count,
              100.0 * col_count / total_col_count);
    }
  }
}

// Set the region_type_ member for all the blobs in the grid.
void TabFind::ComputeBlobGoodness() {
  GridSearch<BLOBNBOX, BLOBNBOX_CLIST, BLOBNBOX_C_IT> gsearch(this);
  gsearch.StartFullSearch();
  BLOBNBOX* bbox;
  while ((bbox = gsearch.NextFullSearch()) != NULL) {
    SetBlobRegionType(bbox);
  }
}

// Set the region_type_ member of the blob, if not already known.
void TabFind::SetBlobRegionType(BLOBNBOX* blob) {
  // If already set, just return the result.
  BlobRegionType blob_type = blob->region_type();
  if (blob_type != BRT_UNKNOWN)
    return;

  // Check for overlapping image blob or other blob already set to image.
  TBOX box = blob->bounding_box();
  GridSearch<BLOBNBOX, BLOBNBOX_CLIST, BLOBNBOX_C_IT> rectsearch(this);
  rectsearch.StartRectSearch(box);
  int rect_overlap = 0;
  int poly_overlap = 0;
  int text_overlap = 0;
  BLOBNBOX* neighbour;
  while ((neighbour = rectsearch.NextRectSearch()) != NULL) {
    if (neighbour != blob &&
        (blob_type = neighbour->region_type()) != BRT_UNKNOWN) {
      TBOX nbox = neighbour->bounding_box();
      nbox -= box;  // This is box intersection, not subtraction.
      int area = nbox.area();
      if (blob_type == BRT_RECTIMAGE) {
        rect_overlap += area;
      } else if (blob_type == BRT_POLYIMAGE) {
        poly_overlap += area;
      } else if (blob_type == BRT_TEXT) {
        text_overlap += area;
      }
    }
  }
  int area = box.area();
  rect_overlap -= text_overlap;
  poly_overlap -= text_overlap;
  if (rect_overlap >= area || poly_overlap >= area) {
    blob->set_region_type(BRT_NOISE);  // Make it disappear
  } else if (rect_overlap > area * kMinImageArea) {
    blob->set_region_type(BRT_RECTIMAGE);
  } else if (poly_overlap > area * kMinImageArea) {
    blob->set_region_type(BRT_POLYIMAGE);
  } else {
    // Actually check the text line width.
    blob->set_region_type(ComputeBlobType(blob));
  }
}

// Mark blobs as being in a vertical text line where that is the case.
// Returns true if the majority of the image is vertical text lines.
void TabFind::MarkVerticalText() {
  TabVector_IT it(&vectors_);
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    TabVector* v = it.data();
    TabVector* partner = v->VerticalTextlinePartner();
    if (partner != NULL) {
      TabVector* left = v->IsLeftTab() ? v : partner;
      TabVector* right = v->IsLeftTab() ? partner : v;
      // Setup a rectangle search to mark the text as vertical.
      TBOX box;
      box.set_left(MIN(left->startpt().x(), left->endpt().x()));
      box.set_right(MAX(right->startpt().x(), right->endpt().x()));
      box.set_bottom(MIN(left->startpt().y(), right->startpt().y()));
      box.set_top(MAX(left->endpt().y(), right->endpt().y()));

      GridSearch<BLOBNBOX, BLOBNBOX_CLIST, BLOBNBOX_C_IT> rsearch(this);
      rsearch.StartRectSearch(box);
      BLOBNBOX* blob = NULL;
      while ((blob = rsearch.NextRectSearch()) != NULL) {
        if (blob->region_type() < BRT_UNKNOWN)
          continue;
        const TBOX& blob_box = blob->bounding_box();
        if ((LeftTabForBox(blob_box, false, false) == left ||
             LeftTabForBox(blob_box, true, false) == left) &&
            (RightTabForBox(blob_box, false, false) == right ||
             RightTabForBox(blob_box, true, false) == right)) {
          blob->set_region_type(BRT_VERT_TEXT);
        }
      }
    }
  }
}

// Returns true if the majority of the image is vertical text lines.
bool TabFind::TextMostlyVertical() {
  int vertical_boxes = 0;
  int horizontal_boxes = 0;
  // Count vertical bboxes in the grid.
  GridSearch<BLOBNBOX, BLOBNBOX_CLIST, BLOBNBOX_C_IT> gsearch(this);
  gsearch.StartFullSearch();
  BLOBNBOX* bbox;
  while ((bbox = gsearch.NextFullSearch()) != NULL) {
    if (bbox->region_type() == BRT_VERT_TEXT)
      ++vertical_boxes;
    else
      ++horizontal_boxes;
  }
  return vertical_boxes > horizontal_boxes;
}

// If this box looks like it is on a textline in the given direction,
// return the width of the textline-like group of blobs, and the number
// of blobs found.
// For more detail see FindTextlineSegment below.
int TabFind::FindTextlineWidth(bool right_to_left, BLOBNBOX* bbox,
                               int* blob_count) {
  ICOORD start_pt, end_pt;
  BLOBNBOX* left_blob;
  BLOBNBOX* right_blob;
  return FindTextlineSegment(right_to_left, true, bbox, blob_count,
                             &start_pt, &end_pt, NULL, NULL,
                             &left_blob, &right_blob);
}

// Search from the given tabstop bbox to the next opposite
// tabstop bbox on the same text line, which may be itself.
// Returns true if the search is successful, and sets
// start_pt, end_pt to the fitted baseline, width to the measured
// width of the text line (column width estimate.)
bool TabFind::TraceTextline(BLOBNBOX* bbox, ICOORD* start_pt, ICOORD* end_pt,
                            int* left_edge, int* right_edge) {
  bool right_to_left = bbox->left_tab_type() != TT_CONFIRMED;
  const TBOX& box = bbox->bounding_box();
  TabVector* left_vector = NULL;
  TabVector* right_vector = NULL;
  if (right_to_left) {
    right_vector = RightTabForBox(box, true, false);
    if (right_vector == NULL || right_vector->IsLeftTab())
      return false;
  } else {
    left_vector = LeftTabForBox(box, true, false);
    if (left_vector == NULL || left_vector->IsRightTab())
      return false;
  }

  BLOBNBOX* left_blob;
  BLOBNBOX* right_blob;
  int blob_count;
  if (FindTextlineSegment(right_to_left, false, bbox, &blob_count,
                          start_pt, end_pt, &left_vector, &right_vector,
                          &left_blob, &right_blob)) {
    AddPartnerVector(left_blob, right_blob, left_vector, right_vector);
    *left_edge = left_vector->XAtY(left_blob->bounding_box().bottom());
    *right_edge = right_vector->XAtY(right_blob->bounding_box().bottom());
    return true;
  }
  return false;
}

// Search from the given bbox in the given direction until the next tab
// vector is found or a significant horizontal gap is found.
// Returns the width of the line if the search is successful, (defined
// as good coverage of the width and a good fitting baseline) and sets
// start_pt, end_pt to the fitted baseline, left_blob, right_blob to
// the ends of the line. Returns zero otherwise.
// Sets blob_count to the number of blobs found on the line.
// On input, either both left_vector and right_vector should be NULL,
// indicating a basic search, or both left_vector and right_vector should
// be not NULL and one of *left_vector and *right_vector should be not NULL,
// in which case the search is strictly between tab vectors and will return
// zero if a gap is found before the opposite tab vector is reached, or a
// conflicting tab vector is found.
// If ignore_images is true, then blobs with aligned_text() < 0 are treated
// as if they do not exist.
int TabFind::FindTextlineSegment(bool right_to_left, bool ignore_images,
                                 BLOBNBOX* bbox, int* blob_count,
                                 ICOORD* start_pt, ICOORD* end_pt,
                                 TabVector** left_vector,
                                 TabVector** right_vector,
                                 BLOBNBOX** left_blob, BLOBNBOX** right_blob) {
  // Bounding box of the current blob.
  TBOX box = bbox->bounding_box();
  // The estimates of top and bottom of the current line move in an
  // alpha-smoothed manner, but in lock-step.
  int top_y = box.top();
  int bottom_y = box.bottom();
  // Left and right of the current blob.
  int left = box.left();
  int right = box.right();
  // Returning the leftmost and rightmost blob used.
  *left_blob = bbox;
  *right_blob = bbox;
  // Coverage measurement as goodness metric.
  int coverage = 0;
  // Approximation of the baseline.
  DetLineFit linepoints;
  // Calculation of the mean height on this line segment.
  double total_height = 0.0;
  int height_count = 0;
  // Starter point for the approximation.
  ICOORD first_pt(right_to_left ? right : left, box.bottom());
  linepoints.Add(first_pt);
  // Last point for the approximation.
  ICOORD last_pt = first_pt;
  // End coordinate that we must not pass.
  int end_coord = right_to_left ? bleft_.x() : tright_.x();
  *blob_count = 0;

  // Maximum gap allowed before abandoning the search for the other edge.
  int gap_limit = static_cast<int>(kMaxHorizontalGap * box.height());
  if (WithinTestRegion(3, first_pt.x(), first_pt.y())) {
    tprintf("Starting %s line search at (%d, %d-%d)\n",
            right_to_left ? "RTL" : "LTR",
            left, bottom_y, top_y);
  }
  while (bbox != NULL) {
    int mid_x = (left + right) / 2;
    if (right_to_left) {
      TabVector* v = LeftTabForBox(box, true, false);
      if ((right_vector != NULL && v == *right_vector) ||
          (v != NULL && bbox == *right_blob && v->IsRightTab()))
        v = LeftTabForBox(box, false, false);
      if (right <= end_coord) {
        if (WithinTestRegion(3, first_pt.x(), first_pt.y()))
          v->Print("Passed through end vector");
        break;
      }
      // No break, so this is a good box.
      linepoints.Add(ICOORD(mid_x, box.bottom()));
      coverage += box.width();
      total_height += box.height();
      ++height_count;
      // In case this is the last one...
      *left_blob = bbox;
      last_pt.set_x(left);
      last_pt.set_y(box.bottom());
      if (v != NULL && (right_vector == NULL || v != *right_vector) &&
          (bbox != *right_blob || v->IsLeftTab())) {
        // The vector is not the starting vector. See if it is within range.
        int x_at_y = v->XAtY(bottom_y);
        if (x_at_y > left - gap_limit) {
          // Once we cross the end_coord, the search will stop.
          if (x_at_y > end_coord)
            end_coord = x_at_y;
          // If this vector is not the correct polarity, then strict searches
          // will fail.
          if (v->IsLeftTab()) {
            if (WithinTestRegion(3, first_pt.x(), first_pt.y()))
              v->Print("Hit End Vector!");
            if (left_vector != NULL)
              *left_vector = v;
          } else {
            if (WithinTestRegion(3, first_pt.x(), first_pt.y()))
              v->Print("Collided with like tab vector");
          }
        }
      }
      if (bbox->left_tab_type() == TT_CONFIRMED) {
        if (WithinTestRegion(3, first_pt.x(), first_pt.y()))
          tprintf("Hit another tab pt\n");
        break;
      }
    } else {
      TabVector* v = RightTabForBox(box, true, false);
      if ((left_vector != NULL && v == *left_vector) ||
          (v != NULL && bbox == *left_blob && v->IsLeftTab()))
        v = RightTabForBox(box, false, false);
      if (left >= end_coord) {
        if (WithinTestRegion(3, first_pt.x(), first_pt.y())) {
          tprintf("left=%d, end_coord=%d\n", left, end_coord);
          v->Print("Passed through end vector");
        }
        break;
      }
      // No break, so this is a good box.
      linepoints.Add(ICOORD(mid_x, box.bottom()));
      coverage += box.width();
      total_height += box.height();
      ++height_count;
      // In case this is the last one...
      *right_blob = bbox;
      last_pt.set_x(right);
      last_pt.set_y(box.bottom());
      if (v != NULL && (left_vector == NULL || v != *left_vector) &&
          (bbox != *left_blob || v->IsRightTab())) {
        // The vector is not the starting vector. See if it is within range.
        int x_at_y = v->XAtY(bottom_y);
        if (x_at_y < right + gap_limit) {
          // Once we cross the end_coord, the search will stop.
          if (x_at_y < end_coord)
            end_coord = x_at_y;
          // If this vector is not the correct polarity, then strict searches
          // will fail.
          if (v->IsRightTab()) {
            if (WithinTestRegion(3, first_pt.x(), first_pt.y()))
              v->Print("Hit End Vector!");
            if (right_vector != NULL)
              *right_vector = v;
          } else {
            if (WithinTestRegion(3, first_pt.x(), first_pt.y()))
              v->Print("Collided with like tab vector");
          }
        }
      }
      if (bbox->right_tab_type() == TT_CONFIRMED) {
        if (WithinTestRegion(3, first_pt.x(), first_pt.y()))
          tprintf("Hit another tab pt\n");
        break;
      }
    }
    // Force the top and bottom to stay the same distance apart by
    // computing the mean alpha smoothing factor of the top and bottom.
    double top_delta = (box.top() - top_y) * kSmoothFactor;
    double bottom_delta = (box.bottom() - bottom_y) * kSmoothFactor;
    int mean_delta = static_cast<int>((top_delta + bottom_delta) / 2.0);
    top_y += mean_delta;
    bottom_y += mean_delta;
    bbox = AdjacentBlob(bbox, right_to_left, ignore_images,
                        gap_limit, top_y, bottom_y);
    if (bbox != NULL && bbox->region_type() < BRT_UNKNOWN) {
      if (WithinTestRegion(3, first_pt.x(), first_pt.y()))
        tprintf("Next box is image region\n");
      bbox = NULL;
    }
    if (bbox != NULL) {
      box = bbox->bounding_box();
      left = box.left();
      right = box.right();
      if (WithinTestRegion(3, first_pt.x(), first_pt.y()))
        tprintf("Next box is at %d, %d\n", left, box.bottom());
    }
  }
  // Either none or both vectors should be NULL.
  if (height_count > 0 &&
      (left_vector == NULL || *left_vector == NULL) ==
      (right_vector == NULL || *right_vector == NULL)) {
    linepoints.Add(last_pt);
    // Maximum median error allowed to be a good text line.
    double max_error = kMaxBaselineError * total_height / height_count;
    double error = linepoints.Fit(start_pt, end_pt);
    int width = (*right_blob)->bounding_box().right()
              - (*left_blob)->bounding_box().left();
    bool good_fit = error < max_error && end_pt->x() != start_pt->x() &&
                    coverage >= kMinBaselineCoverage * width;
    if (WithinTestRegion(3, first_pt.x(), first_pt.y())) {
      tprintf("Found end! good=%d, error=%g/%g, coverage=%g%%"
              " on line (%d, %d) to (%d,%d)\n",
              good_fit, error, max_error, 100.0 * coverage / width,
              start_pt->x(), start_pt->y(), end_pt->x(), end_pt->y());
      tprintf("width=%d, L/R blob=%d/%d, first/last=%d/%d, start/end=%d/%d\n",
              width, (*left_blob)->bounding_box().left(),
              (*right_blob)->bounding_box().right(),
              first_pt.x(), last_pt.x(), start_pt->x(), end_pt->x());
    }
    *blob_count = height_count;
    return good_fit ? width : 0;
  }
  return 0;
}

// Find the next adjacent (to left or right) blob on this text line,
// with the constraint that it must vertically significantly overlap
// the [top_y, bottom_y] range.
// If ignore_images is true, then blobs with aligned_text() < 0 are treated
// as if they do not exist.
BLOBNBOX* TabFind::AdjacentBlob(const BLOBNBOX* bbox,
                                bool right_to_left, bool ignore_images,
                                int gap_limit, int top_y, int bottom_y) {
  GridSearch<BLOBNBOX, BLOBNBOX_CLIST, BLOBNBOX_C_IT> sidesearch(this);
  const TBOX& box = bbox->bounding_box();
  int left = box.left();
  int right = box.right();
  int mid_x = (left + right) / 2;
  sidesearch.StartSideSearch(mid_x, bottom_y, top_y);
  int best_gap = 0;
  BLOBNBOX* result = NULL;
  BLOBNBOX* neighbour = NULL;
  while ((neighbour = sidesearch.NextSideSearch(right_to_left)) != NULL) {
    if (neighbour == bbox ||
        (ignore_images && neighbour->region_type() < BRT_UNKNOWN))
      continue;
    const TBOX& nbox = neighbour->bounding_box();
    int n_top_y = nbox.top();
    int n_bottom_y = nbox.bottom();
    int v_overlap = MIN(n_top_y, top_y) - MAX(n_bottom_y, bottom_y);
    int height = top_y - bottom_y;
    int n_height = n_top_y - n_bottom_y;
    if (v_overlap > kCharVerticalOverlapFraction * MIN(height, n_height) &&
        !DifferentSizes(height, n_height)) {
      int n_left = nbox.left();
      int n_right = nbox.right();
      int h_gap = MAX(n_left, left) - MIN(n_right, right);
      int n_mid_x = (n_left + n_right) / 2;
      if (right_to_left == (n_mid_x < mid_x) && n_mid_x != mid_x) {
        if (h_gap > gap_limit) {
          // Hit a big gap before next tab so don't return anything.
          if (WithinTestRegion(3, left, n_top_y))
            tprintf("Giving up due to big gap = %d vs %d\n",
                    h_gap, gap_limit);
          return result;
        }
        if ((right_to_left ? neighbour->right_tab_type()
                           : neighbour->left_tab_type()) >= TT_FAKE) {
          // Hit a tab facing the wrong way. Stop in case we are crossing
          // the column boundary.
          if (WithinTestRegion(3, left, n_top_y))
            tprintf("Collision with like tab of type %d at %d,%d\n",
                    right_to_left ? neighbour->right_tab_type()
                                  : neighbour->left_tab_type(),
                    n_left, nbox.bottom());
          return result;
        }
        // This is a good fit to the line. Continue with this
        // neighbour as the bbox if the best gap.
        if (result == NULL || h_gap < best_gap) {
          if (WithinTestRegion(3, left, n_top_y))
            tprintf("Good result\n");
          result = neighbour;
          best_gap = h_gap;
        } else {
          // The new one is worse, so we probably already have the best result.
          return result;
        }
      }
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
    if (v != NULL && v != left && v->IsLeftTab() &&
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
    if (v != NULL && v != right && v->IsRightTab() &&
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

static void RotateBlobList(const FCOORD& rotation, BLOBNBOX_LIST* blobs) {
  BLOBNBOX_IT it(blobs);
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    it.data()->rotate_box(rotation);
  }
}

// Recreate the grid with deskewed BLOBNBOXes.
void TabFind::Deskew(TabVector_LIST* hlines, BLOBNBOX_LIST* image_blobs,
                     TO_BLOCK* block, FCOORD* reskew) {
  FCOORD deskew;
  ComputeDeskewVectors(&deskew, reskew);
  RotateBlobList(deskew, image_blobs);
  RotateBlobList(deskew, &block->blobs);
  RotateBlobList(deskew, &block->small_blobs);
  RotateBlobList(deskew, &block->noise_blobs);
#ifdef HAVE_LIBLEPT
  if (textord_debug_images) {
    // Rotate the debug pix and arrange for it to be drawn at the correct
    // pixel offset.
    Pix* pix_grey = pixRead(AlignedBlob::textord_debug_pix().string());
    int width = pixGetWidth(pix_grey);
    int height = pixGetHeight(pix_grey);
    float angle = atan2(deskew.y(), deskew.x());
    // Positive angle is clockwise to pixRotate.
    Pix* pix_rot = pixRotate(pix_grey, -angle, L_ROTATE_AREA_MAP,
                             L_BRING_IN_WHITE, width, height);
    // The image must be translated by the rotation of its center, since it
    // has just been rotated about its center.
    ICOORD center_offset(width / 2, height / 2);
    ICOORD new_center_offset(center_offset);
    new_center_offset.rotate(deskew);
    image_origin_ += new_center_offset - center_offset;
    // The image grew as it was rotated, so offset the (top/left) origin
    // by half the change in size. y is opposite to x because it is drawn
    // at ist top/left, not bottom/left.
    ICOORD corner_offset((width - pixGetWidth(pix_rot)) / 2,
                         (pixGetHeight(pix_rot) - height) / 2);
    image_origin_ += corner_offset;
    pixWrite(AlignedBlob::textord_debug_pix().string(), pix_rot, IFF_PNG);
    pixDestroy(&pix_grey);
    pixDestroy(&pix_rot);
  }
#endif  // HAVE_LIBLEPT

  // Rotate the horizontal vectors. The vertical vectors don't need
  // rotating as they can just be refitted.
  TabVector_IT h_it(hlines);
  for (h_it.mark_cycle_pt(); !h_it.cycled_list(); h_it.forward()) {
    TabVector* h = h_it.data();
    h->Rotate(deskew);
  }
  SetVerticalSkewAndParellelize(0, 1);
  // Rebuild the grid to the new size.
  TBOX grid_box(bleft_, tright_);
  grid_box.rotate_large(deskew);
  Init(gridsize(), grid_box.botleft(), grid_box.topright());
  InsertBlobList(false, false, false, image_blobs, false, this);
  InsertBlobList(true, false, false, &block->blobs, false, this);
}

static void ResetBlobList(BLOBNBOX_LIST* blobs) {
  BLOBNBOX_IT it(blobs);
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    BLOBNBOX* blob = it.data();
    blob->set_left_tab_type(TT_NONE);
    blob->set_right_tab_type(TT_NONE);
    blob->set_region_type(BRT_UNKNOWN);
  }
}

// Restart everything and rotate the input blobs ready for vertical text.
void TabFind::ResetForVerticalText(TabVector_LIST* hlines,
                                   BLOBNBOX_LIST* image_blobs,
                                   TO_BLOCK* block, FCOORD* rerotate) {
  // Rotate anti-clockwise, so vertical CJK text is still in reading order.
  FCOORD derotate(0.0f, 1.0f);
  *rerotate = FCOORD(0.0f, -1.0f);
  RotateBlobList(derotate, image_blobs);
  RotateBlobList(derotate, &block->blobs);
  RotateBlobList(derotate, &block->small_blobs);
  RotateBlobList(derotate, &block->noise_blobs);
  ResetBlobList(&block->blobs);

  // Rotate the horizontal and vertical vectors and swap them over.
  // Only the separators are kept, and existing tabs are deleted.
  // Note that to retain correct relative orientation, vertical and
  // horizontal lines must be rotated in opposite directions!
  TabVector_LIST ex_verticals;
  TabVector_IT ex_v_it(&ex_verticals);
  while (!v_it_.empty()) {
    TabVector* v = v_it_.extract();
    if (v->IsSeparator()) {
      v->Rotate(*rerotate);
      ex_v_it.add_after_then_move(v);
    } else {
      delete v;
    }
    v_it_.forward();
  }
  TabVector_IT h_it(hlines);
  for (h_it.mark_cycle_pt(); !h_it.cycled_list(); h_it.forward()) {
    TabVector* h = h_it.data();
    h->Rotate(derotate);
  }
  v_it_.add_list_after(hlines);
  v_it_.move_to_first();
  h_it.set_to_list(hlines);
  h_it.add_list_after(&ex_verticals);

  // Rebuild the grid to the new size.
  TBOX grid_box(bleft_, tright_);
  grid_box.rotate_large(derotate);
  Init(gridsize(), grid_box.botleft(), grid_box.topright());
  column_widths_.clear();
}

// Compute the rotation required to deskew, and its inverse rotation.
void TabFind::ComputeDeskewVectors(FCOORD* deskew, FCOORD* reskew) {
  double length = vertical_skew_ % vertical_skew_;
  length = sqrt(length);
  deskew->set_x(vertical_skew_.y() / length);
  deskew->set_y(vertical_skew_.x() / length);
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

