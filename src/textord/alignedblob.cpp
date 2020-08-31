///////////////////////////////////////////////////////////////////////
// File:        alignedblob.cpp
// Description: Subclass of BBGrid to find vertically aligned blobs.
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
#include "config_auto.h"
#endif

#include "alignedblob.h"

#include <algorithm>

INT_VAR(textord_debug_tabfind, 0, "Debug tab finding");
INT_VAR(textord_debug_bugs, 0, "Turn on output related to bugs in tab finding");
static INT_VAR(textord_testregion_left, -1, "Left edge of debug reporting rectangle in Leptonica coords (bottom=0/top=height), with horizontal lines x/y-flipped");
static INT_VAR(textord_testregion_top, INT32_MAX, "Top edge of debug reporting rectangle in Leptonica coords (bottom=0/top=height), with horizontal lines x/y-flipped");
static INT_VAR(textord_testregion_right, INT32_MAX, "Right edge of debug rectangle in Leptonica coords (bottom=0/top=height), with horizontal lines x/y-flipped");
static INT_VAR(textord_testregion_bottom, -1, "Bottom edge of debug rectangle in Leptonica coords (bottom=0/top=height), with horizontal lines x/y-flipped");
BOOL_VAR(textord_debug_printable, false, "Make debug windows printable");

namespace tesseract {

// Fraction of resolution used as alignment tolerance for aligned tabs.
const double kAlignedFraction = 0.03125;
// Fraction of resolution used as alignment tolerance for ragged tabs.
const double kRaggedFraction = 2.5;
// Fraction of height used as a minimum gutter gap for aligned blobs.
const double kAlignedGapFraction = 0.75;
// Fraction of height used as a minimum gutter gap for ragged tabs.
const double kRaggedGapFraction = 1.0;
// Constant number of pixels used as alignment tolerance for line finding.
const int kVLineAlignment = 3;
// Constant number of pixels used as gutter gap tolerance for line finding.
const int kVLineGutter = 1;
// Constant number of pixels used as the search size for line finding.
const int kVLineSearchSize = 150;
// Min number of points to accept for a ragged tab stop.
const int kMinRaggedTabs = 5;
// Min number of points to accept for an aligned tab stop.
const int kMinAlignedTabs = 4;
// Constant number of pixels minimum height of a vertical line.
const int kVLineMinLength = 300;
// Minimum gradient for a vertical tab vector. Used to prune away junk
// tab vectors with what would be a ridiculously large skew angle.
// Value corresponds to tan(90 - max allowed skew angle)
const double kMinTabGradient = 4.0;
// Tolerance to skew on top of current estimate of skew. Divide x or y length
// by kMaxSkewFactor to get the y or x skew distance.
// If the angle is small, the angle in degrees is roughly 60/kMaxSkewFactor.
const int kMaxSkewFactor = 15;

// Constructor to set the parameters for finding aligned and ragged tabs.
// Vertical_x and vertical_y are the current estimates of the true vertical
// direction (up) in the image. Height is the height of the starter blob.
// v_gap_multiple is the multiple of height that will be used as a limit
// on vertical gap before giving up and calling the line ended.
// resolution is the original image resolution, and align0 indicates the
// type of tab stop to be found.
AlignedBlobParams::AlignedBlobParams(int vertical_x, int vertical_y,
                                     int height, int v_gap_multiple,
                                     int min_gutter_width,
                                     int resolution, TabAlignment align0)
  : right_tab(align0 == TA_RIGHT_RAGGED || align0 == TA_RIGHT_ALIGNED),
    ragged(align0 == TA_LEFT_RAGGED || align0 == TA_RIGHT_RAGGED),
    alignment(align0),
    confirmed_type(TT_CONFIRMED),
    min_length(0) {
  // Set the tolerances according to the type of line sought.
  // For tab search, these are based on the image resolution for most, or
  // the height of the starting blob for the maximum vertical gap.
  max_v_gap = height * v_gap_multiple;
  if (ragged) {
    // In the case of a ragged edge, we are much more generous with the
    // inside alignment fraction, but also require a much bigger gutter.
    gutter_fraction = kRaggedGapFraction;
    if (alignment == TA_RIGHT_RAGGED) {
      l_align_tolerance = static_cast<int>(resolution * kRaggedFraction + 0.5);
      r_align_tolerance = static_cast<int>(resolution * kAlignedFraction + 0.5);
    } else {
      l_align_tolerance = static_cast<int>(resolution * kAlignedFraction + 0.5);
      r_align_tolerance = static_cast<int>(resolution * kRaggedFraction + 0.5);
    }
    min_points = kMinRaggedTabs;
  } else {
    gutter_fraction = kAlignedGapFraction;
    l_align_tolerance = static_cast<int>(resolution * kAlignedFraction + 0.5);
    r_align_tolerance = static_cast<int>(resolution * kAlignedFraction + 0.5);
    min_points = kMinAlignedTabs;
  }
  min_gutter = static_cast<int>(height * gutter_fraction + 0.5);
  if (min_gutter < min_gutter_width)
    min_gutter = min_gutter_width;
  // Fit the vertical vector into an ICOORD, which is 16 bit.
  set_vertical(vertical_x, vertical_y);
}

// Constructor to set the parameters for finding vertical lines.
// Vertical_x and vertical_y are the current estimates of the true vertical
// direction (up) in the image. Width is the width of the starter blob.
AlignedBlobParams::AlignedBlobParams(int vertical_x, int vertical_y,
                                     int width)
  : gutter_fraction(0.0),
    right_tab(false),
    ragged(false),
    alignment(TA_SEPARATOR),
    confirmed_type(TT_VLINE),
    max_v_gap(kVLineSearchSize),
    min_gutter(kVLineGutter),
    min_points(1),
    min_length(kVLineMinLength) {
  // Compute threshold for left and right alignment.
  l_align_tolerance = std::max(kVLineAlignment, width);
  r_align_tolerance = std::max(kVLineAlignment, width);

  // Fit the vertical vector into an ICOORD, which is 16 bit.
  set_vertical(vertical_x, vertical_y);
}

// Fit the vertical vector into an ICOORD, which is 16 bit.
void AlignedBlobParams::set_vertical(int vertical_x, int vertical_y) {
  int factor = 1;
  if (vertical_y > INT16_MAX)
    factor = vertical_y / INT16_MAX + 1;
  vertical.set_x(vertical_x / factor);
  vertical.set_y(vertical_y / factor);
}


AlignedBlob::AlignedBlob(int gridsize,
                         const ICOORD& bleft, const ICOORD& tright)
  : BlobGrid(gridsize, bleft, tright) {
}

// Return true if the given coordinates are within the test rectangle
// and the debug level is at least the given detail level.
bool AlignedBlob::WithinTestRegion(int detail_level, int x, int y) {
  if (textord_debug_tabfind < detail_level)
    return false;
  return x >= textord_testregion_left && x <= textord_testregion_right &&
         y <= textord_testregion_top && y >= textord_testregion_bottom;
}

#ifndef GRAPHICS_DISABLED

// Display the tab codes of the BLOBNBOXes in this grid.
ScrollView* AlignedBlob::DisplayTabs(const char* window_name,
                                     ScrollView* tab_win) {
  if (tab_win == nullptr)
    tab_win = MakeWindow(0, 50, window_name);
  // For every tab in the grid, display it.
  GridSearch<BLOBNBOX, BLOBNBOX_CLIST, BLOBNBOX_C_IT> gsearch(this);
  gsearch.StartFullSearch();
  BLOBNBOX* bbox;
  while ((bbox = gsearch.NextFullSearch()) != nullptr) {
    const TBOX& box = bbox->bounding_box();
    int left_x = box.left();
    int right_x = box.right();
    int top_y = box.top();
    int bottom_y = box.bottom();
    TabType tabtype = bbox->left_tab_type();
    if (tabtype != TT_NONE) {
      if (tabtype == TT_MAYBE_ALIGNED)
        tab_win->Pen(ScrollView::BLUE);
      else if (tabtype == TT_MAYBE_RAGGED)
        tab_win->Pen(ScrollView::YELLOW);
      else if (tabtype == TT_CONFIRMED)
        tab_win->Pen(ScrollView::GREEN);
      else
        tab_win->Pen(ScrollView::GREY);
      tab_win->Line(left_x, top_y, left_x, bottom_y);
    }
    tabtype = bbox->right_tab_type();
    if (tabtype != TT_NONE) {
      if (tabtype == TT_MAYBE_ALIGNED)
        tab_win->Pen(ScrollView::MAGENTA);
      else if (tabtype == TT_MAYBE_RAGGED)
        tab_win->Pen(ScrollView::ORANGE);
      else if (tabtype == TT_CONFIRMED)
        tab_win->Pen(ScrollView::RED);
      else
        tab_win->Pen(ScrollView::GREY);
      tab_win->Line(right_x, top_y, right_x, bottom_y);
    }
  }
  tab_win->Update();
  return tab_win;
}

#endif // !GRAPHICS_DISABLED

// Helper returns true if the total number of line_crossings of all the blobs
// in the list is at least 2.
static bool AtLeast2LineCrossings(BLOBNBOX_CLIST* blobs) {
  BLOBNBOX_C_IT it(blobs);
  int total_crossings = 0;
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    total_crossings += it.data()->line_crossings();
  }
  return total_crossings >= 2;
}

// Destructor.
// It is defined here, so the compiler can create a single vtable
// instead of weak vtables in every compilation unit.
AlignedBlob::~AlignedBlob() = default;

// Finds a vector corresponding to a set of vertically aligned blob edges
// running through the given box. The type of vector returned and the
// search parameters are determined by the AlignedBlobParams.
// vertical_x and y are updated with an estimate of the real
// vertical direction. (skew finding.)
// Returns nullptr if no decent vector can be found.
TabVector* AlignedBlob::FindVerticalAlignment(AlignedBlobParams align_params,
                                              BLOBNBOX* bbox,
                                              int* vertical_x,
                                              int* vertical_y) {
  int ext_start_y, ext_end_y;
  BLOBNBOX_CLIST good_points;
  // Search up and then down from the starting bbox.
  TBOX box = bbox->bounding_box();
  bool debug = WithinTestRegion(2, box.left(), box.bottom());
  int pt_count = AlignTabs(align_params, false, bbox, &good_points, &ext_end_y);
  pt_count += AlignTabs(align_params, true, bbox, &good_points, &ext_start_y);
  BLOBNBOX_C_IT it(&good_points);
  it.move_to_last();
  box = it.data()->bounding_box();
  int end_y = box.top();
  int end_x = align_params.right_tab ? box.right() : box.left();
  it.move_to_first();
  box = it.data()->bounding_box();
  int start_x = align_params.right_tab ? box.right() : box.left();
  int start_y = box.bottom();
  // Acceptable tab vectors must have a minimum number of points,
  // have a minimum acceptable length, and have a minimum gradient.
  // The gradient corresponds to the skew angle.
  // Ragged tabs don't need to satisfy the gradient condition, as they
  // will always end up parallel to the vertical direction.
  bool at_least_2_crossings = AtLeast2LineCrossings(&good_points);
  if ((pt_count >= align_params.min_points &&
      end_y - start_y >= align_params.min_length &&
      (align_params.ragged ||
          end_y - start_y >= abs(end_x - start_x) * kMinTabGradient)) ||
      at_least_2_crossings) {
    int confirmed_points = 0;
    // Count existing confirmed points to see if vector is acceptable.
    for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
      bbox = it.data();
      if (align_params.right_tab) {
        if (bbox->right_tab_type() == align_params.confirmed_type)
          ++confirmed_points;
      } else {
        if (bbox->left_tab_type() == align_params.confirmed_type)
          ++confirmed_points;
      }
    }
    // Ragged vectors are not allowed to use too many already used points.
    if (!align_params.ragged ||
        confirmed_points + confirmed_points < pt_count) {
      const TBOX& box = bbox->bounding_box();
      if (debug) {
        tprintf("Confirming tab vector of %d pts starting at %d,%d\n",
                pt_count, box.left(), box.bottom());
      }
      // Flag all the aligned neighbours as confirmed .
      for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
        bbox = it.data();
        if (align_params.right_tab) {
          bbox->set_right_tab_type(align_params.confirmed_type);
        } else {
          bbox->set_left_tab_type(align_params.confirmed_type);
        }
        if (debug) {
          bbox->bounding_box().print();
        }
      }
      // Now make the vector and return it.
      TabVector* result = TabVector::FitVector(align_params.alignment,
                                               align_params.vertical,
                                               ext_start_y, ext_end_y,
                                               &good_points,
                                               vertical_x, vertical_y);
      result->set_intersects_other_lines(at_least_2_crossings);
      if (debug) {
        tprintf("Box was %d, %d\n", box.left(), box.bottom());
        result->Print("After fitting");
      }
      return result;
    } else if (debug) {
      tprintf("Ragged tab used too many used points: %d out of %d\n",
              confirmed_points, pt_count);
    }
  } else if (debug) {
    tprintf("Tab vector failed basic tests: pt count %d vs min %d, "
            "length %d vs min %d, min grad %g\n",
            pt_count, align_params.min_points, end_y - start_y,
            align_params.min_length, abs(end_x - start_x) * kMinTabGradient);
  }
  return nullptr;
}

// Find a set of blobs that are aligned in the given vertical
// direction with the given blob. Returns a list of aligned
// blobs and the number in the list.
// For other parameters see FindAlignedBlob below.
int AlignedBlob::AlignTabs(const AlignedBlobParams& params,
                           bool top_to_bottom, BLOBNBOX* bbox,
                           BLOBNBOX_CLIST* good_points, int* end_y) {
  int ptcount = 0;
  BLOBNBOX_C_IT it(good_points);

  TBOX box = bbox->bounding_box();
  bool debug = WithinTestRegion(2, box.left(), box.bottom());
  if (debug) {
    tprintf("Starting alignment run at blob:");
    box.print();
  }
  int x_start = params.right_tab ? box.right() : box.left();
  while (bbox != nullptr) {
    // Add the blob to the list if the appropriate side is a tab candidate,
    // or if we are working on a ragged tab.
    TabType type = params.right_tab ? bbox->right_tab_type()
                                    : bbox->left_tab_type();
    if (((type != TT_NONE && type != TT_MAYBE_RAGGED) || params.ragged) &&
        (it.empty() || it.data() != bbox)) {
      if (top_to_bottom)
        it.add_before_then_move(bbox);
      else
        it.add_after_then_move(bbox);
      ++ptcount;
    }
    // Find the next blob that is aligned with the current one.
    // FindAlignedBlob guarantees that forward progress will be made in the
    // top_to_bottom direction, and therefore eventually it will return nullptr,
    // making this while (bbox != nullptr) loop safe.
    bbox = FindAlignedBlob(params, top_to_bottom, bbox, x_start, end_y);
    if (bbox != nullptr) {
      box = bbox->bounding_box();
      if (!params.ragged)
        x_start = params.right_tab ? box.right() : box.left();
    }
  }
  if (debug) {
    tprintf("Alignment run ended with %d pts at blob:", ptcount);
    box.print();
  }
  return ptcount;
}

// Search vertically for a blob that is aligned with the input bbox.
// The search parameters are determined by AlignedBlobParams.
// top_to_bottom tells whether to search down or up.
// The return value is nullptr if nothing was found in the search box
// or if a blob was found in the gutter. On a nullptr return, end_y
// is set to the edge of the search box or the leading edge of the
// gutter blob if one was found.
BLOBNBOX* AlignedBlob::FindAlignedBlob(const AlignedBlobParams& p,
                                       bool top_to_bottom, BLOBNBOX* bbox,
                                       int x_start, int* end_y) {
  TBOX box = bbox->bounding_box();
  // If there are separator lines, get the column edges.
  int left_column_edge = bbox->left_rule();
  int right_column_edge = bbox->right_rule();
  // start_y is used to guarantee that forward progress is made and the
  // search does not go into an infinite loop. New blobs must extend the
  // line beyond start_y.
  int start_y = top_to_bottom ? box.bottom() : box.top();
  if (WithinTestRegion(2, x_start, start_y)) {
    tprintf("Column edges for blob at (%d,%d)->(%d,%d) are [%d, %d]\n",
            box.left(), box.top(), box.right(), box.bottom(),
            left_column_edge, right_column_edge);
  }
  // Compute skew tolerance.
  int skew_tolerance = p.max_v_gap / kMaxSkewFactor;
  // Calculate xmin and xmax of the search box so that it contains
  // all possibly relevant boxes up to p.max_v_gap above or below according
  // to top_to_bottom.
  // Start with a notion of vertical with the current estimate.
  int x2 = (p.max_v_gap * p.vertical.x() + p.vertical.y()/2) / p.vertical.y();
  if (top_to_bottom) {
    x2 = x_start - x2;
    *end_y = start_y - p.max_v_gap;
  } else {
    x2 = x_start + x2;
    *end_y = start_y + p.max_v_gap;
  }
  // Expand the box by an additional skew tolerance
  int xmin = std::min(x_start, x2) - skew_tolerance;
  int xmax = std::max(x_start, x2) + skew_tolerance;
  // Now add direction-specific tolerances.
  if (p.right_tab) {
    xmax += p.min_gutter;
    xmin -= p.l_align_tolerance;
  } else {
    xmax += p.r_align_tolerance;
    xmin -= p.min_gutter;
  }
  // Setup a vertical search for an aligned blob.
  GridSearch<BLOBNBOX, BLOBNBOX_CLIST, BLOBNBOX_C_IT> vsearch(this);
  if (WithinTestRegion(2, x_start, start_y))
    tprintf("Starting %s %s search at %d-%d,%d, search_size=%d, gutter=%d\n",
            p.ragged ? "Ragged" : "Aligned", p.right_tab ? "Right" : "Left",
            xmin, xmax, start_y, p.max_v_gap, p.min_gutter);
  vsearch.StartVerticalSearch(xmin, xmax, start_y);
  // result stores the best real return value.
  BLOBNBOX* result = nullptr;
  // The backup_result is not a tab candidate and can be used if no
  // real tab candidate result is found.
  BLOBNBOX* backup_result = nullptr;
  // neighbour is the blob that is currently being investigated.
  BLOBNBOX* neighbour = nullptr;
  while ((neighbour = vsearch.NextVerticalSearch(top_to_bottom)) != nullptr) {
    if (neighbour == bbox)
      continue;
    TBOX nbox = neighbour->bounding_box();
    int n_y = (nbox.top() + nbox.bottom()) / 2;
    if ((!top_to_bottom && n_y > start_y + p.max_v_gap) ||
        (top_to_bottom && n_y < start_y - p.max_v_gap)) {
      if (WithinTestRegion(2, x_start, start_y))
        tprintf("Neighbour too far at (%d,%d)->(%d,%d)\n",
                nbox.left(), nbox.bottom(), nbox.right(), nbox.top());
      break;  // Gone far enough.
    }
    // It is CRITICAL to ensure that forward progress is made, (strictly
    // in/decreasing n_y) or the caller could loop infinitely, while
    // waiting for a sequence of blobs in a line to end.
    // NextVerticalSearch alone does not guarantee this, as there may be
    // more than one blob in a grid cell. See comment in AlignTabs.
    if ((n_y < start_y) != top_to_bottom || nbox.y_overlap(box))
      continue;  // Only look in the required direction.
    if (result != nullptr && result->bounding_box().y_gap(nbox) > gridsize())
      return result;  // This result is clear.
    if (backup_result != nullptr && p.ragged && result == nullptr &&
        backup_result->bounding_box().y_gap(nbox) > gridsize())
      return backup_result;  // This result is clear.

    // If the neighbouring blob is the wrong side of a separator line, then it
    // "doesn't exist" as far as we are concerned.
    int x_at_n_y = x_start + (n_y - start_y) * p.vertical.x() / p.vertical.y();
    if (x_at_n_y < neighbour->left_crossing_rule() ||
        x_at_n_y > neighbour->right_crossing_rule())
      continue;  // Separator line in the way.
    int n_left = nbox.left();
    int n_right = nbox.right();
    int n_x = p.right_tab ? n_right : n_left;
    if (WithinTestRegion(2, x_start, start_y))
      tprintf("neighbour at (%d,%d)->(%d,%d), n_x=%d, n_y=%d, xatn=%d\n",
              nbox.left(), nbox.bottom(), nbox.right(), nbox.top(),
              n_x, n_y, x_at_n_y);
    if (p.right_tab &&
        n_left < x_at_n_y + p.min_gutter &&
        n_right > x_at_n_y + p.r_align_tolerance &&
        (p.ragged || n_left < x_at_n_y + p.gutter_fraction * nbox.height())) {
      // In the gutter so end of line.
      if (bbox->right_tab_type() >= TT_MAYBE_ALIGNED)
        bbox->set_right_tab_type(TT_DELETED);
      *end_y = top_to_bottom ? nbox.top() : nbox.bottom();
      if (WithinTestRegion(2, x_start, start_y))
        tprintf("gutter\n");
      return nullptr;
    }
    if (!p.right_tab &&
        n_left < x_at_n_y - p.l_align_tolerance &&
        n_right > x_at_n_y - p.min_gutter &&
        (p.ragged || n_right > x_at_n_y - p.gutter_fraction * nbox.height())) {
      // In the gutter so end of line.
      if (bbox->left_tab_type() >= TT_MAYBE_ALIGNED)
        bbox->set_left_tab_type(TT_DELETED);
      *end_y = top_to_bottom ? nbox.top() : nbox.bottom();
      if (WithinTestRegion(2, x_start, start_y))
        tprintf("gutter\n");
      return nullptr;
    }
    if ((p.right_tab && neighbour->leader_on_right()) ||
        (!p.right_tab && neighbour->leader_on_left()))
      continue;  // Neighbours of leaders are not allowed to be used.
    if (n_x <= x_at_n_y + p.r_align_tolerance &&
        n_x >= x_at_n_y - p.l_align_tolerance) {
      // Aligned so keep it. If it is a marked tab save it as result,
      // otherwise keep it as backup_result to return in case of later failure.
      if (WithinTestRegion(2, x_start, start_y))
        tprintf("aligned, seeking%d, l=%d, r=%d\n",
                p.right_tab, neighbour->left_tab_type(),
                neighbour->right_tab_type());
      TabType n_type = p.right_tab ? neighbour->right_tab_type()
                                   : neighbour->left_tab_type();
      if (n_type != TT_NONE && (p.ragged || n_type != TT_MAYBE_RAGGED)) {
        if (result == nullptr) {
          result = neighbour;
        } else {
          // Keep the closest neighbour by Euclidean distance.
          // This prevents it from picking a tab blob in another column.
          const TBOX& old_box = result->bounding_box();
          int x_diff = p.right_tab ? old_box.right() : old_box.left();
          x_diff -= x_at_n_y;
          int y_diff = (old_box.top() + old_box.bottom()) / 2 - start_y;
          int old_dist = x_diff * x_diff + y_diff * y_diff;
          x_diff = n_x - x_at_n_y;
          y_diff = n_y - start_y;
          int new_dist = x_diff * x_diff + y_diff * y_diff;
          if (new_dist < old_dist)
            result = neighbour;
        }
      } else if (backup_result == nullptr) {
        if (WithinTestRegion(2, x_start, start_y))
          tprintf("Backup\n");
        backup_result = neighbour;
      } else {
        TBOX backup_box = backup_result->bounding_box();
        if ((p.right_tab && backup_box.right() < nbox.right()) ||
            (!p.right_tab && backup_box.left() > nbox.left())) {
          if (WithinTestRegion(2, x_start, start_y))
            tprintf("Better backup\n");
          backup_result = neighbour;
        }
      }
    }
  }
  return result != nullptr ? result : backup_result;
}

}  // namespace tesseract.
