///////////////////////////////////////////////////////////////////////
// File:        strokewidth.cpp
// Description: Subclass of BBGrid to find uniformity of strokewidth.
// Author:      Ray Smith
// Created:     Mon Mar 31 16:17:01 PST 2008
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

#ifdef _MSC_VER
#pragma warning(disable:4244)  // Conversion warnings
#endif

#include "strokewidth.h"
#include "blobbox.h"
#include "tabfind.h"
#include "tordmain.h"  // For SetBlobStrokeWidth.

// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif

namespace tesseract {

/** Allowed proportional change in stroke width to be the same font. */
const double kStrokeWidthFractionTolerance = 0.125;
/**
 * Allowed constant change in stroke width to be the same font. 
 * Really 1.5 pixels.
 */
const double kStrokeWidthTolerance = 1.5;
/** Maximum height in inches of the largest possible text. */
const double kMaxTextSize = 2.0;

StrokeWidth::StrokeWidth(int gridsize,
                         const ICOORD& bleft, const ICOORD& tright)
  : BBGrid<BLOBNBOX, BLOBNBOX_CLIST, BLOBNBOX_C_IT>(gridsize, bleft, tright) {
}

StrokeWidth::~StrokeWidth() {
}

/** Puts the block blobs (normal and large) into the grid. */
void StrokeWidth::InsertBlobs(TO_BLOCK* block, TabFind* line_grid) {
  // Insert the blobs into this grid using the separator lines in line_grid.
  line_grid->InsertBlobList(true, false, false, &block->blobs, false, this);
  line_grid->InsertBlobList(true, false, true, &block->large_blobs,
                            false, this);
}

/**
 * Moves the large blobs that have good stroke-width neighbours to the normal
 * blobs list.
 */
void StrokeWidth::MoveGoodLargeBlobs(int resolution, TO_BLOCK* block) {
  BLOBNBOX_IT large_it = &block->large_blobs;
  BLOBNBOX_IT blob_it = &block->blobs;
  int max_height = static_cast<int>(resolution * kMaxTextSize);
  int b_count = 0;
  for (large_it.mark_cycle_pt(); !large_it.cycled_list(); large_it.forward()) {
    BLOBNBOX* large_blob = large_it.data();
    if (large_blob->bounding_box().height() <= max_height &&
        GoodTextBlob(large_blob)) {
      blob_it.add_to_end(large_it.extract());
      ++b_count;
    }
  }
  if (textord_debug_tabfind) {
    tprintf("Moved %d large blobs to normal list\n",
            b_count);
  }
}

/** Displays the blobs green or red according to whether they are good or not. */
ScrollView* StrokeWidth::DisplayGoodBlobs(const char* window_name,
                                          ScrollView* window) {
#ifndef GRAPHICS_DISABLED
  if (window == NULL)
    window = MakeWindow(0, 0, window_name);
  // For every blob in the grid, display it.
  window->Brush(ScrollView::NONE);

  // For every bbox in the grid, display it.
  GridSearch<BLOBNBOX, BLOBNBOX_CLIST, BLOBNBOX_C_IT> gsearch(this);
  gsearch.StartFullSearch();
  BLOBNBOX* bbox;
  while ((bbox = gsearch.NextFullSearch()) != NULL) {
    TBOX box = bbox->bounding_box();
    int left_x = box.left();
    int right_x = box.right();
    int top_y = box.top();
    int bottom_y = box.bottom();
    if (textord_debug_printable || GoodTextBlob(bbox))
      window->Pen(ScrollView::GREEN);
    else
      window->Pen(ScrollView::RED);
    window->Rectangle(left_x, bottom_y, right_x, top_y);
  }
  window->Update();
#endif
  return window;
}

/** Handles a click event in a display window. */
void StrokeWidth::HandleClick(int x, int y) {
  BBGrid<BLOBNBOX, BLOBNBOX_CLIST, BLOBNBOX_C_IT>::HandleClick(x, y);
  // Run a radial search for blobs that overlap.
  GridSearch<BLOBNBOX, BLOBNBOX_CLIST, BLOBNBOX_C_IT> radsearch(this);
  radsearch.StartRadSearch(x, y, 1);
  BLOBNBOX* neighbour;
  FCOORD click(x, y);
  while ((neighbour = radsearch.NextRadSearch()) != NULL) {
    TBOX nbox = neighbour->bounding_box();
    if (nbox.contains(click) && neighbour->cblob() != NULL) {
      SetBlobStrokeWidth(true, neighbour);
      tprintf("Box (%d,%d)->(%d,%d): h-width=%.1f, v-width=%.1f p-width=%1.f\n",
              nbox.left(), nbox.bottom(), nbox.right(), nbox.top(),
              neighbour->horz_stroke_width(), neighbour->vert_stroke_width(),
              2.0 * neighbour->cblob()->area()/neighbour->cblob()->perimeter());
    }
  }
}

/**
 * Returns true if there is at least one side neighbour that has a similar
 * stroke width and is not on the other side of a rule line.
 */
bool StrokeWidth::GoodTextBlob(BLOBNBOX* blob) {
  double h_width = blob->horz_stroke_width();
  double v_width = blob->vert_stroke_width();
  // The perimeter-based width is used as a backup in case there is
  // no information in the blob.
  double p_width = 2.0f * blob->cblob()->area();
  p_width /= blob->cblob()->perimeter();
  double h_tolerance = h_width * kStrokeWidthFractionTolerance
                     + kStrokeWidthTolerance;
  double v_tolerance = v_width * kStrokeWidthFractionTolerance
                     + kStrokeWidthTolerance;
  double p_tolerance = p_width * kStrokeWidthFractionTolerance
                     + kStrokeWidthTolerance;

  // Run a radial search for neighbours that overlap.
  TBOX box = blob->bounding_box();
  int radius = box.height() / gridsize_ + 2;
  GridSearch<BLOBNBOX, BLOBNBOX_CLIST, BLOBNBOX_C_IT> radsearch(this);
  radsearch.StartRadSearch((box.left() + box.right()) / 2, box.bottom(),
                           radius);
  int top = box.top();
  int bottom = box.bottom();
  int min_overlap = (top - bottom) / 2;
  BLOBNBOX* neighbour;
  while ((neighbour = radsearch.NextRadSearch()) != NULL) {
    TBOX nbox = neighbour->bounding_box();
    if (neighbour == blob) {
      continue;
    }
    // In finding a suitable neighbour, do not cross rule lines.
    if (nbox.right() > blob->right_rule() || nbox.left() < blob->left_rule()) {
      continue;  // Can't use it.
    }
    int overlap = MIN(nbox.top(), top) - MAX(nbox.bottom(), bottom);
    if (overlap >= min_overlap &&
        !TabFind::DifferentSizes(box.height(), nbox.height())) {
      double n_h_width = neighbour->horz_stroke_width();
      double n_v_width = neighbour->vert_stroke_width();
      double n_p_width = 2.0f * neighbour->cblob()->area();
      n_p_width /= neighbour->cblob()->perimeter();
      bool h_zero = h_width == 0.0f || n_h_width == 0.0f;
      bool v_zero = v_width == 0.0f || n_v_width == 0.0f;
      bool h_ok = !h_zero && NearlyEqual(h_width, n_h_width, h_tolerance);
      bool v_ok = !v_zero && NearlyEqual(v_width, n_v_width, v_tolerance);
      bool p_ok = h_zero && v_zero &&
                  NearlyEqual(p_width, n_p_width, p_tolerance);
      // For a match, at least one of the horizontal and vertical widths
      // must match, and the other one must either match or be zero.
      // Only if both are zero will we look at the perimeter metric.
      if (p_ok || ((v_ok || h_ok) && (h_ok || h_zero) && (v_ok || v_zero))) {
        return true;
      }
    }
  }
  return false;
}

}  // namespace tesseract.

