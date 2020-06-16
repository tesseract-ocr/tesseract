/******************************************************************************
 *
 * File:         split.cpp  (Formerly split.c)
 * Author:       Mark Seaman, OCR Technology
 *
 * (c) Copyright 1987, Hewlett-Packard Company.
 ** Licensed under the Apache License, Version 2.0 (the "License");
 ** you may not use this file except in compliance with the License.
 ** You may obtain a copy of the License at
 ** http://www.apache.org/licenses/LICENSE-2.0
 ** Unless required by applicable law or agreed to in writing, software
 ** distributed under the License is distributed on an "AS IS" BASIS,
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 ** See the License for the specific language governing permissions and
 ** limitations under the License.
 *
 *************************************************************************/
/*----------------------------------------------------------------------
              I n c l u d e s
----------------------------------------------------------------------*/
// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif

#include "split.h"
#include "coutln.h"
#include "tprintf.h"

#include <algorithm>

/*----------------------------------------------------------------------
              V a r i a b l e s
----------------------------------------------------------------------*/
// Limit on the amount of penalty for the chop being off-center.
const int kCenterGradeCap = 25;
// Ridiculously large priority for splits that are no use.
const double kBadPriority = 999.0;

BOOL_VAR(wordrec_display_splits, 0, "Display splits");

// Hides the SPLIT so the outlines appear not to be cut by it.
void SPLIT::Hide() const {
  EDGEPT* edgept = point1;
  do {
    edgept->Hide();
    edgept = edgept->next;
  } while (!edgept->EqualPos(*point2) && edgept != point1);
  edgept = point2;
  do {
    edgept->Hide();
    edgept = edgept->next;
  } while (!edgept->EqualPos(*point1) && edgept != point2);
}

// Undoes hide, so the outlines are cut by the SPLIT.
void SPLIT::Reveal() const {
  EDGEPT* edgept = point1;
  do {
    edgept->Reveal();
    edgept = edgept->next;
  } while (!edgept->EqualPos(*point2) && edgept != point1);
  edgept = point2;
  do {
    edgept->Reveal();
    edgept = edgept->next;
  } while (!edgept->EqualPos(*point1) && edgept != point2);
}

// Compute a split priority based on the bounding boxes of the parts.
// The arguments here are config parameters defined in Wordrec. Add chop_
// to the beginning of the name.
float SPLIT::FullPriority(int xmin, int xmax, double overlap_knob,
                          int centered_maxwidth, double center_knob,
                          double width_change_knob) const {
  TBOX box1 = Box12();
  TBOX box2 = Box21();
  int min_left = std::min(box1.left(), box2.left());
  int max_right = std::max(box1.right(), box2.right());
  if (xmin < min_left && xmax > max_right) return kBadPriority;

  float grade = 0.0f;
  // grade_overlap.
  int width1 = box1.width();
  int width2 = box2.width();
  int min_width = std::min(width1, width2);
  int overlap = -box1.x_gap(box2);
  if (overlap == min_width) {
    grade += 100.0f;  // Total overlap.
  } else {
    if (2 * overlap > min_width) overlap += 2 * overlap - min_width;
    if (overlap > 0) grade += overlap_knob * overlap;
  }
  // grade_center_of_blob.
  if (width1 <= centered_maxwidth || width2 <= centered_maxwidth) {
    grade += std::min(static_cast<double>(kCenterGradeCap), center_knob * abs(width1 - width2));
  }
  // grade_width_change.
  float width_change_grade = 20 - (max_right - min_left - std::max(width1, width2));
  if (width_change_grade > 0.0f)
    grade += width_change_grade * width_change_knob;
  return grade;
}

// Returns true if *this SPLIT appears OK in the sense that it does not cross
// any outlines and does not chop off any ridiculously small pieces.
bool SPLIT::IsHealthy(const TBLOB& blob, int min_points, int min_area) const {
  return !IsLittleChunk(min_points, min_area) &&
         !blob.SegmentCrossesOutline(point1->pos, point2->pos);
}

// Returns true if the split generates a small chunk in terms of either area
// or number of points.
bool SPLIT::IsLittleChunk(int min_points, int min_area) const {
  if (point1->ShortNonCircularSegment(min_points, point2) &&
      point1->SegmentArea(point2) < min_area) {
    return true;
  }
  if (point2->ShortNonCircularSegment(min_points, point1) &&
      point2->SegmentArea(point1) < min_area) {
    return true;
  }
  return false;
}

/**********************************************************************
 * make_edgept
 *
 * Create an EDGEPT and hook it into an existing list of edge points.
 **********************************************************************/
EDGEPT *make_edgept(int x, int y, EDGEPT *next, EDGEPT *prev) {
  EDGEPT *this_edgept;
  /* Create point */
  this_edgept = new EDGEPT;
  this_edgept->pos.x = x;
  this_edgept->pos.y = y;
  // Now deal with the src_outline steps.
  C_OUTLINE* prev_ol = prev->src_outline;
  if (prev_ol != nullptr && prev->next == next) {
    // Compute the fraction of the segment that is being cut.
    FCOORD segment_vec(next->pos.x - prev->pos.x, next->pos.y - prev->pos.y);
    FCOORD target_vec(x - prev->pos.x, y - prev->pos.y);
    double cut_fraction = target_vec.length() / segment_vec.length();
    // Get the start and end at the step level.
    ICOORD step_start = prev_ol->position_at_index(prev->start_step);
    int end_step = prev->start_step + prev->step_count;
    int step_length = prev_ol->pathlength();
    ICOORD step_end = prev_ol->position_at_index(end_step % step_length);
    ICOORD step_vec = step_end - step_start;
    double target_length = step_vec.length() * cut_fraction;
    // Find the point on the segment that gives the length nearest to target.
    int best_step = prev->start_step;
    ICOORD total_step(0, 0);
    double best_dist = target_length;
    for (int s = prev->start_step; s < end_step; ++s) {
      total_step += prev_ol->step(s % step_length);
      double dist = fabs(target_length - total_step.length());
      if (dist < best_dist) {
        best_dist = dist;
        best_step = s + 1;
      }
    }
    // The new point is an intermediate point.
    this_edgept->src_outline = prev_ol;
    this_edgept->step_count = end_step - best_step;
    this_edgept->start_step = best_step % step_length;
    prev->step_count = best_step - prev->start_step;
  } else {
    // The new point is poly only.
    this_edgept->src_outline = nullptr;
    this_edgept->step_count = 0;
    this_edgept->start_step = 0;
  }
  /* Hook it up */
  this_edgept->next = next;
  this_edgept->prev = prev;
  prev->next = this_edgept;
  next->prev = this_edgept;
  /* Set up vec entries */
  this_edgept->vec.x = this_edgept->next->pos.x - x;
  this_edgept->vec.y = this_edgept->next->pos.y - y;
  this_edgept->prev->vec.x = x - this_edgept->prev->pos.x;
  this_edgept->prev->vec.y = y - this_edgept->prev->pos.y;
  return this_edgept;
}

/**********************************************************************
 * remove_edgept
 *
 * Remove a given EDGEPT from its list and delete it.
 **********************************************************************/
void remove_edgept(EDGEPT *point) {
  EDGEPT *prev = point->prev;
  EDGEPT *next = point->next;
  // Add point's steps onto prev's steps if they are from the same outline.
  if (prev->src_outline == point->src_outline && prev->src_outline != nullptr) {
    prev->step_count += point->step_count;
  }
  prev->next = next;
  next->prev = prev;
  prev->vec.x = next->pos.x - prev->pos.x;
  prev->vec.y = next->pos.y - prev->pos.y;
  delete point;
}

/**********************************************************************
 * Print
 *
 * Shows the coordinates of both points in a split.
 **********************************************************************/
void SPLIT::Print() const {
  tprintf("(%d,%d)--(%d,%d)", point1->pos.x, point1->pos.y, point2->pos.x,
          point2->pos.y);
}

#ifndef GRAPHICS_DISABLED
// Draws the split in the given window.
void SPLIT::Mark(ScrollView* window) const {
  window->Pen(ScrollView::GREEN);
  window->Line(point1->pos.x, point1->pos.y, point2->pos.x, point2->pos.y);
  window->UpdateWindow();
}
#endif

// Creates two outlines out of one by splitting the original one in half.
// Inserts the resulting outlines into the given list.
void SPLIT::SplitOutlineList(TESSLINE* outlines) const {
  SplitOutline();
  while (outlines->next != nullptr) outlines = outlines->next;

  outlines->next = new TESSLINE;
  outlines->next->loop = point1;
  outlines->next->ComputeBoundingBox();

  outlines = outlines->next;

  outlines->next = new TESSLINE;
  outlines->next->loop = point2;
  outlines->next->ComputeBoundingBox();

  outlines->next->next = nullptr;
}

// Makes a split between these two edge points, but does not affect the
// outlines to which they belong.
void SPLIT::SplitOutline() const {
  EDGEPT* temp2 = point2->next;
  EDGEPT* temp1 = point1->next;
  /* Create two new points */
  EDGEPT* new_point1 = make_edgept(point1->pos.x, point1->pos.y, temp1, point2);
  EDGEPT* new_point2 = make_edgept(point2->pos.x, point2->pos.y, temp2, point1);
  // point1 and 2 are now cross-over points, so they must have nullptr
  // src_outlines and give their src_outline information their new
  // replacements.
  new_point1->src_outline = point1->src_outline;
  new_point1->start_step = point1->start_step;
  new_point1->step_count = point1->step_count;
  new_point2->src_outline = point2->src_outline;
  new_point2->start_step = point2->start_step;
  new_point2->step_count = point2->step_count;
  point1->src_outline = nullptr;
  point1->start_step = 0;
  point1->step_count = 0;
  point2->src_outline = nullptr;
  point2->start_step = 0;
  point2->step_count = 0;
}

// Undoes the effect of SplitOutlineList, correcting the outlines for undoing
// the split, but possibly leaving some duplicate outlines.
void SPLIT::UnsplitOutlineList(TBLOB* blob) const {
  /* Modify edge points */
  UnsplitOutlines();

  auto* outline1 = new TESSLINE;
  outline1->next = blob->outlines;
  blob->outlines = outline1;
  outline1->loop = point1;

  auto* outline2 = new TESSLINE;
  outline2->next = blob->outlines;
  blob->outlines = outline2;
  outline2->loop = point2;
}

// Removes the split that was put between these two points.
void SPLIT::UnsplitOutlines() const {
  EDGEPT* tmp1 = point1->next;
  EDGEPT* tmp2 = point2->next;

  tmp1->next->prev = point2;
  tmp2->next->prev = point1;

  // tmp2 is coincident with point1. point1 takes tmp2's place as tmp2 is
  // deleted.
  point1->next = tmp2->next;
  point1->src_outline = tmp2->src_outline;
  point1->start_step = tmp2->start_step;
  point1->step_count = tmp2->step_count;
  // Likewise point2 takes tmp1's place.
  point2->next = tmp1->next;
  point2->src_outline = tmp1->src_outline;
  point2->start_step = tmp1->start_step;
  point2->step_count = tmp1->step_count;

  delete tmp1;
  delete tmp2;

  point1->vec.x = point1->next->pos.x - point1->pos.x;
  point1->vec.y = point1->next->pos.y - point1->pos.y;

  point2->vec.x = point2->next->pos.x - point2->pos.x;
  point2->vec.y = point2->next->pos.y - point2->pos.y;
}
