/**********************************************************************
 * File:        stepblob.cpp  (Formerly cblob.c)
 * Description: Code for C_BLOB class.
 * Author:      Ray Smith
 * Created:     Tue Oct 08 10:41:13 BST 1991
 *
 * (C) Copyright 1991, Hewlett-Packard Ltd.
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
 **********************************************************************/

// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif

#include "stepblob.h"
#include "allheaders.h"     // for pixCreate, pixGetDepth
#include "genericvector.h"  // for GenericVector
#include "host.h"           // for TRUE, FALSE
#include "points.h"         // for operator+=, FCOORD, ICOORD

class DENORM;

// Max perimeter to width ratio for a baseline position above box bottom.
const double kMaxPerimeterWidthRatio = 8.0;

ELISTIZE (C_BLOB)
/**********************************************************************
 * position_outline
 *
 * Position the outline in the given list at the relevant place
 * according to its nesting.
 **********************************************************************/
static void position_outline(                          //put in place
                             C_OUTLINE *outline,       //thing to place
                             C_OUTLINE_LIST *destlist  //desstination list
                            ) {
  C_OUTLINE *dest_outline;       //outline from dest list
  C_OUTLINE_IT it = destlist;    //iterator
                                 //iterator on children
  C_OUTLINE_IT child_it = outline->child ();

  if (!it.empty ()) {
    do {
      dest_outline = it.data (); //get destination
                                 //encloses dest
      if (*dest_outline < *outline) {
                                 //take off list
        dest_outline = it.extract ();
                                 //put this in place
        it.add_after_then_move (outline);
                                 //make it a child
        child_it.add_to_end (dest_outline);
        while (!it.at_last ()) {
          it.forward ();         //do rest of list
                                 //check for other children
          dest_outline = it.data ();
          if (*dest_outline < *outline) {
                                 //take off list
            dest_outline = it.extract ();
            child_it.add_to_end (dest_outline);
            //make it a child
            if (it.empty ())
              break;
          }
        }
        return;                  //finished
      }
                                 //enclosed by dest
      else if (*outline < *dest_outline) {
        position_outline (outline, dest_outline->child ());
        //place in child list
        return;                  //finished
      }
      it.forward ();
    }
    while (!it.at_first ());
  }
  it.add_to_end (outline);       //at outer level
}


/**********************************************************************
 * plot_outline_list
 *
 * Draw a list of outlines in the given colour and their children
 * in the child colour.
 **********************************************************************/

#ifndef GRAPHICS_DISABLED
static void plot_outline_list(                       //draw outlines
                              C_OUTLINE_LIST *list,  //outline to draw
                              ScrollView* window,         //window to draw in
                              ScrollView::Color colour,         //colour to use
                              ScrollView::Color child_colour    //colour of children
                             ) {
  C_OUTLINE *outline;            //current outline
  C_OUTLINE_IT it = list;        //iterator

  for (it.mark_cycle_pt (); !it.cycled_list (); it.forward ()) {
    outline = it.data ();
                                 //draw it
    outline->plot (window, colour);
    if (!outline->child ()->empty ())
      plot_outline_list (outline->child (), window,
        child_colour, child_colour);
  }
}
// Draws the outlines in the given colour, and child_colour, normalized
// using the given denorm, making use of sub-pixel accurate information
// if available.
static void plot_normed_outline_list(const DENORM& denorm,
                                     C_OUTLINE_LIST *list,
                                     ScrollView::Color colour,
                                     ScrollView::Color child_colour,
                                     ScrollView* window) {
  C_OUTLINE_IT it(list);
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    C_OUTLINE* outline = it.data();
    outline->plot_normed(denorm, colour, window);
    if (!outline->child()->empty())
      plot_normed_outline_list(denorm, outline->child(), child_colour,
                               child_colour, window);
  }
}
#endif


/**********************************************************************
 * reverse_outline_list
 *
 * Reverse a list of outlines and their children.
 **********************************************************************/

static void reverse_outline_list(C_OUTLINE_LIST *list) {
  C_OUTLINE_IT it = list;        // iterator

  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    C_OUTLINE* outline = it.data();
    outline->reverse();         // reverse it
    outline->set_flag(COUT_INVERSE, TRUE);
    if (!outline->child()->empty())
      reverse_outline_list(outline->child());
  }
}


/**********************************************************************
 * C_BLOB::C_BLOB
 *
 * Constructor to build a C_BLOB from a list of C_OUTLINEs.
 * The C_OUTLINEs are not copied so the source list is emptied.
 * The C_OUTLINEs are nested correctly in the blob.
 **********************************************************************/

C_BLOB::C_BLOB(C_OUTLINE_LIST *outline_list) {
  for (C_OUTLINE_IT ol_it(outline_list); !ol_it.empty(); ol_it.forward()) {
    C_OUTLINE* outline = ol_it.extract();
    // Position this outline in appropriate position in the hierarchy.
    position_outline(outline, &outlines);
  }
  CheckInverseFlagAndDirection();
}

// Simpler constructor to build a blob from a single outline that has
// already been fully initialized.
C_BLOB::C_BLOB(C_OUTLINE* outline) {
  C_OUTLINE_IT it(&outlines);
  it.add_to_end(outline);
}

// Builds a set of one or more blobs from a list of outlines.
// Input: one outline on outline_list contains all the others, but the
// nesting and order are undefined.
// If good_blob is true, the blob is added to good_blobs_it, unless
// an illegal (generation-skipping) parent-child relationship is found.
// If so, the parent blob goes to bad_blobs_it, and the immediate children
// are promoted to the top level, recursively being sent to good_blobs_it.
// If good_blob is false, all created blobs will go to the bad_blobs_it.
// Output: outline_list is empty. One or more blobs are added to
// good_blobs_it and/or bad_blobs_it.
void C_BLOB::ConstructBlobsFromOutlines(bool good_blob,
                                        C_OUTLINE_LIST* outline_list,
                                        C_BLOB_IT* good_blobs_it,
                                        C_BLOB_IT* bad_blobs_it) {
  // List of top-level outlines with correctly nested children.
  C_OUTLINE_LIST nested_outlines;
  for (C_OUTLINE_IT ol_it(outline_list); !ol_it.empty(); ol_it.forward()) {
    C_OUTLINE* outline = ol_it.extract();
    // Position this outline in appropriate position in the hierarchy.
    position_outline(outline, &nested_outlines);
  }
  // Check for legal nesting and reassign as required.
  for (C_OUTLINE_IT ol_it(&nested_outlines); !ol_it.empty(); ol_it.forward()) {
    C_OUTLINE* outline = ol_it.extract();
    bool blob_is_good = good_blob;
    if (!outline->IsLegallyNested()) {
      // The blob is illegally nested.
      // Mark it bad, and add all its children to the top-level list.
      blob_is_good = false;
      ol_it.add_list_after(outline->child());
    }
    C_BLOB* blob = new C_BLOB(outline);
    // Set inverse flag and reverse if needed.
    blob->CheckInverseFlagAndDirection();
    // Put on appropriate list.
    if (!blob_is_good && bad_blobs_it != nullptr)
      bad_blobs_it->add_after_then_move(blob);
    else
      good_blobs_it->add_after_then_move(blob);
  }
}

// Sets the COUT_INVERSE flag appropriately on the outlines and their
// children recursively, reversing the outlines if needed so that
// everything has an anticlockwise top-level.
void C_BLOB::CheckInverseFlagAndDirection() {
  C_OUTLINE_IT ol_it(&outlines);
  for (ol_it.mark_cycle_pt(); !ol_it.cycled_list(); ol_it.forward()) {
    C_OUTLINE* outline = ol_it.data();
    if (outline->turn_direction() < 0) {
      outline->reverse();
      reverse_outline_list(outline->child());
      outline->set_flag(COUT_INVERSE, TRUE);
    } else {
      outline->set_flag(COUT_INVERSE, FALSE);
    }
  }
}


// Build and return a fake blob containing a single fake outline with no
// steps.
C_BLOB* C_BLOB::FakeBlob(const TBOX& box) {
  C_OUTLINE_LIST outlines;
  C_OUTLINE::FakeOutline(box, &outlines);
  return new C_BLOB(&outlines);
}

/**********************************************************************
 * C_BLOB::bounding_box
 *
 * Return the bounding box of the blob.
 **********************************************************************/

TBOX C_BLOB::bounding_box() const {  // bounding box
  C_OUTLINE *outline;                // current outline
  // This is a read-only iteration of the outlines.
  C_OUTLINE_IT it = const_cast<C_OUTLINE_LIST*>(&outlines);
  TBOX box;                          // bounding box

  for (it.mark_cycle_pt (); !it.cycled_list (); it.forward ()) {
    outline = it.data ();
    box += outline->bounding_box ();
  }
  return box;
}


/**********************************************************************
 * C_BLOB::area
 *
 * Return the area of the blob.
 **********************************************************************/

int32_t C_BLOB::area() {  //area
  C_OUTLINE *outline;            //current outline
  C_OUTLINE_IT it = &outlines;   //outlines of blob
  int32_t total;                   //total area

  total = 0;
  for (it.mark_cycle_pt (); !it.cycled_list (); it.forward ()) {
    outline = it.data ();
    total += outline->area ();
  }
  return total;
}

/**********************************************************************
 * C_BLOB::perimeter
 *
 * Return the perimeter of the top and 2nd level outlines.
 **********************************************************************/

int32_t C_BLOB::perimeter() {
  C_OUTLINE *outline;            // current outline
  C_OUTLINE_IT it = &outlines;   // outlines of blob
  int32_t total;                   // total perimeter

  total = 0;
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    outline = it.data();
    total += outline->perimeter();
  }
  return total;
}


/**********************************************************************
 * C_BLOB::outer_area
 *
 * Return the area of the blob.
 **********************************************************************/

int32_t C_BLOB::outer_area() {  //area
  C_OUTLINE *outline;            //current outline
  C_OUTLINE_IT it = &outlines;   //outlines of blob
  int32_t total;                   //total area

  total = 0;
  for (it.mark_cycle_pt (); !it.cycled_list (); it.forward ()) {
    outline = it.data ();
    total += outline->outer_area ();
  }
  return total;
}


/**********************************************************************
 * C_BLOB::count_transitions
 *
 * Return the total x and y maxes and mins in the blob.
 * Chlid outlines are not counted.
 **********************************************************************/

int32_t C_BLOB::count_transitions(                 //area
                                int32_t threshold  //on size
                               ) {
  C_OUTLINE *outline;            //current outline
  C_OUTLINE_IT it = &outlines;   //outlines of blob
  int32_t total;                   //total area

  total = 0;
  for (it.mark_cycle_pt (); !it.cycled_list (); it.forward ()) {
    outline = it.data ();
    total += outline->count_transitions (threshold);
  }
  return total;
}


/**********************************************************************
 * C_BLOB::move
 *
 * Move C_BLOB by vector
 **********************************************************************/

void C_BLOB::move(                  // reposition blob
                  const ICOORD vec  // by vector
                 ) {
  C_OUTLINE_IT it(&outlines);  // iterator

  for (it.mark_cycle_pt (); !it.cycled_list (); it.forward ())
    it.data ()->move (vec);      // move each outline
}

// Static helper for C_BLOB::rotate to allow recursion of child outlines.
static void RotateOutlineList(const FCOORD& rotation,
                              C_OUTLINE_LIST* outlines) {
  C_OUTLINE_LIST new_outlines;
  C_OUTLINE_IT src_it(outlines);
  C_OUTLINE_IT dest_it(&new_outlines);
  while (!src_it.empty()) {
    C_OUTLINE* old_outline = src_it.extract();
    src_it.forward();
    C_OUTLINE* new_outline = new C_OUTLINE(old_outline, rotation);
    if (!old_outline->child()->empty()) {
      RotateOutlineList(rotation, old_outline->child());
      C_OUTLINE_IT child_it(new_outline->child());
      child_it.add_list_after(old_outline->child());
    }
    delete old_outline;
    dest_it.add_to_end(new_outline);
  }
  src_it.add_list_after(&new_outlines);
}

/**********************************************************************
 * C_BLOB::rotate
 *
 * Rotate C_BLOB by rotation.
 * Warning! has to rebuild all the C_OUTLINEs.
 **********************************************************************/
void C_BLOB::rotate(const FCOORD& rotation) {
  RotateOutlineList(rotation, &outlines);
}

// Helper calls ComputeEdgeOffsets or ComputeBinaryOffsets recursively on the
// outline list and its children.
static void ComputeEdgeOffsetsOutlineList(int threshold, Pix* pix,
                                          C_OUTLINE_LIST *list) {
  C_OUTLINE_IT it(list);
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    C_OUTLINE* outline = it.data();
    if (pix != nullptr && pixGetDepth(pix) == 8)
      outline->ComputeEdgeOffsets(threshold, pix);
    else
      outline->ComputeBinaryOffsets();
    if (!outline->child()->empty())
      ComputeEdgeOffsetsOutlineList(threshold, pix, outline->child());
  }
}

// Adds sub-pixel resolution EdgeOffsets for the outlines using greyscale
// if the supplied pix is 8-bit or the binary edges if nullptr.
void C_BLOB::ComputeEdgeOffsets(int threshold, Pix* pix) {
  ComputeEdgeOffsetsOutlineList(threshold, pix, &outlines);
}

// Estimates and returns the baseline position based on the shape of the
// outlines.
// We first find the minimum y-coord (y_mins) at each x-coord within the blob.
// If there is a run of some y or y+1 in y_mins that is longer than the total
// number of positions at bottom or bottom+1, subject to the additional
// condition that at least one side of the y/y+1 run is higher than y+1, so it
// is not a local minimum, then y, not the bottom, makes a good candidate
// baseline position for this blob. Eg
//   |                  ---|
//   |                  |
//   |-      -----------|        <=  Good candidate baseline position.
//    |-    -|
//     |   -|
//     |---|                     <=  Bottom of blob
int16_t C_BLOB::EstimateBaselinePosition() {
  TBOX box = bounding_box();
  int left = box.left();
  int width = box.width();
  int bottom = box.bottom();
  if (outlines.empty() || perimeter() > width * kMaxPerimeterWidthRatio)
    return bottom;  // This is only for non-CJK blobs.
  // Get the minimum y coordinate at each x-coordinate.
  GenericVector<int> y_mins;
  y_mins.init_to_size(width + 1, box.top());
  C_OUTLINE_IT it(&outlines);
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    C_OUTLINE* outline = it.data();
    ICOORD pos = outline->start_pos();
    for (int s = 0; s < outline->pathlength(); ++s) {
      if (pos.y() < y_mins[pos.x() - left])
        y_mins[pos.x() - left] = pos.y();
      pos += outline->step(s);
    }
  }
  // Find the total extent of the bottom or bottom + 1.
  int bottom_extent = 0;
  for (int x = 0; x <= width; ++x) {
    if (y_mins[x] == bottom || y_mins[x] == bottom + 1)
      ++bottom_extent;
  }
  // Find the lowest run longer than the bottom extent that is not the bottom.
  int best_min = box.top();
  int prev_run = 0;
  int prev_y = box.top();
  int prev_prev_y = box.top();
  for (int x = 0; x < width; x += prev_run) {
    // Find the length of the current run.
    int y_at_x = y_mins[x];
    int run = 1;
    while (x + run <= width && y_mins[x + run] == y_at_x) ++run;
    if (y_at_x > bottom + 1) {
      // Possible contender.
      int total_run = run;
      // Find extent of current value or +1 to the right of x.
      while (x + total_run <= width &&
          (y_mins[x + total_run] == y_at_x ||
              y_mins[x + total_run] == y_at_x + 1)) ++total_run;
      // At least one end has to be higher so it is not a local max.
      if (prev_prev_y > y_at_x + 1 || x + total_run > width ||
          y_mins[x + total_run] > y_at_x + 1) {
        // If the prev_run is at y + 1, then we can add that too. There cannot
        // be a suitable run at y before that or we would have found it already.
        if (prev_run > 0 && prev_y == y_at_x + 1) total_run += prev_run;
        if (total_run > bottom_extent && y_at_x < best_min) {
          best_min = y_at_x;
        }
      }
    }
    prev_run = run;
    prev_prev_y = prev_y;
    prev_y = y_at_x;
  }
  return best_min == box.top() ? bottom : best_min;
}

static void render_outline_list(C_OUTLINE_LIST *list,
                                int left, int top, Pix* pix) {
  C_OUTLINE_IT it(list);
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    C_OUTLINE* outline = it.data();
    outline->render(left, top, pix);
    if (!outline->child()->empty())
      render_outline_list(outline->child(), left, top, pix);
  }
}

static void render_outline_list_outline(C_OUTLINE_LIST *list,
                                        int left, int top, Pix* pix) {
  C_OUTLINE_IT it(list);
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    C_OUTLINE* outline = it.data();
    outline->render_outline(left, top, pix);
  }
}

// Returns a Pix rendering of the blob. pixDestroy after use.
Pix* C_BLOB::render() {
  TBOX box = bounding_box();
  Pix* pix = pixCreate(box.width(), box.height(), 1);
  render_outline_list(&outlines, box.left(), box.top(), pix);
  return pix;
}

// Returns a Pix rendering of the outline of the blob. (no fill).
// pixDestroy after use.
Pix* C_BLOB::render_outline() {
  TBOX box = bounding_box();
  Pix* pix = pixCreate(box.width(), box.height(), 1);
  render_outline_list_outline(&outlines, box.left(), box.top(), pix);
  return pix;
}

/**********************************************************************
 * C_BLOB::plot
 *
 * Draw the C_BLOB in the given colour.
 **********************************************************************/

#ifndef GRAPHICS_DISABLED
void C_BLOB::plot(ScrollView* window,                // window to draw in
                  ScrollView::Color blob_colour,     // main colour
                  ScrollView::Color child_colour) {  // for holes
  plot_outline_list(&outlines, window, blob_colour, child_colour);
}
// Draws the blob in the given colour, and child_colour, normalized
// using the given denorm, making use of sub-pixel accurate information
// if available.
void C_BLOB::plot_normed(const DENORM& denorm,
                         ScrollView::Color blob_colour,
                         ScrollView::Color child_colour,
                         ScrollView* window) {
  plot_normed_outline_list(denorm, &outlines, blob_colour, child_colour,
                           window);
}
#endif
