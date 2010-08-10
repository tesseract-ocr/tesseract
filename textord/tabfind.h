///////////////////////////////////////////////////////////////////////
// File:        tabfind.h
// Description: Subclass of BBGrid to find tabstops.
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

#ifndef TESSERACT_TEXTORD_TABFIND_H__
#define TESSERACT_TEXTORD_TABFIND_H__

#include "alignedblob.h"
#include "callback.h"
#include "tabvector.h"
#include "linefind.h"

class BLOBNBOX;
class BLOBNBOX_LIST;
class TO_BLOCK;
class ScrollView;
struct Pix;

namespace tesseract {

typedef ResultCallback1<bool, int> WidthCallback;

struct AlignedBlobParams;

/** Pixel resolution of column width estimates. */
const int kColumnWidthFactor = 20;

/**
 * The TabFind class contains code to find tab-stops and maintain the
 * vectors_ list of tab vectors.
 * Also provides an interface to find neighbouring blobs
 * in the grid of BLOBNBOXes that is used by multiple subclasses.
 * Searching is a complex operation because of the need to enforce
 * rule/separator lines, and tabstop boundaries, (when available), so
 * as the holder of the list of TabVectors this class provides the functions.
 */
class TabFind : public AlignedBlob {
 public:
  TabFind(int gridsize, const ICOORD& bleft, const ICOORD& tright,
          TabVector_LIST* vlines, int vertical_x, int vertical_y);
  virtual ~TabFind();

  /**
   * Insert a list of blobs into the given grid (not necessarily this).
   * If take_ownership is true, then the blobs are removed from the source list.
   * See InsertBlob for the other arguments.
   */
  void InsertBlobList(bool h_spread, bool v_spread, bool large,
                      BLOBNBOX_LIST* blobs, bool take_ownership,
                      BBGrid<BLOBNBOX, BLOBNBOX_CLIST, BLOBNBOX_C_IT>* grid);

  /**
   * Insert a single blob into the given grid (not necessarily this).
   * If h_spread, then all cells covered horizontally by the box are
   * used, otherwise, just the bottom-left. Similarly for v_spread.
   * If large, then insert only if the bounding box doesn't intersect
   * anything else already in the grid. Returns true if the blob was inserted.
   * A side effect is that the left and right rule edges of the blob are
   * set according to the tab vectors in this (not grid).
   */
  bool InsertBlob(bool h_spread, bool v_spread, bool large, BLOBNBOX* blob,
                  BBGrid<BLOBNBOX, BLOBNBOX_CLIST, BLOBNBOX_C_IT>* grid);

  /**
   * Find the gutter width and distance to inner neighbour for the given blob.
   */
  void GutterWidthAndNeighbourGap(int tab_x, int mean_height,
                                  int max_gutter, bool left,
                                  BLOBNBOX* bbox, int* gutter_width,
                                  int* neighbour_gap);

  /**
   * Find the next adjacent (to left or right) blob on this text line,
   * with the constraint that it must vertically significantly overlap
   * the input box.
   */
  BLOBNBOX* AdjacentBlob(const BLOBNBOX* bbox,
                         bool right_to_left, int gap_limit);

  /**
   * Compute and return, but do not set the type as being BRT_TEXT or
   * BRT_UNKNOWN according to how well it forms a text line.
   */
  BlobRegionType ComputeBlobType(BLOBNBOX* blob);

  /**
   * Return the x-coord that corresponds to the right edge for the given
   * box. If there is a rule line to the right that vertically overlaps it,
   * then return the x-coord of the rule line, otherwise return the right
   * edge of the page. For details see RightTabForBox below.
   */
  int RightEdgeForBox(const TBOX& box, bool crossing, bool extended);
  /**
   * As RightEdgeForBox, but finds the left Edge instead.
   */
  int LeftEdgeForBox(const TBOX& box, bool crossing, bool extended);

  /**
   * Compute the rotation required to deskew, and its inverse rotation.
   */
  void ComputeDeskewVectors(FCOORD* deskew, FCOORD* reskew);

  /**
   * Return true if the given width is close to one of the common
   * widths in column_widths_.
   */
  bool CommonWidth(int width);
  /**
   * Return true if the sizes are more than a
   * factor of 2 different.
   */
  static bool DifferentSizes(int size1, int size2);

  /**
   * Return a callback for testing CommonWidth.
   */
  WidthCallback* WidthCB() {
    return width_cb_;
  }

  /**
   * Return the coords at which to draw the image backdrop.
   */
  const ICOORD& image_origin() const {
    return image_origin_;
  }

 protected:
  /**
  // Accessors
   */
  TabVector_LIST* get_vectors() {
    return &vectors_;
  }

  /**
  // Top-level function to find TabVectors in an input page block.
   */
  void FindTabVectors(int resolution, TabVector_LIST* hlines,
                      BLOBNBOX_LIST* image_blobs, TO_BLOCK* block,
                      FCOORD* reskew, FCOORD* rerotate);

  /**
  // Top-level function to not find TabVectors in an input page block,
  // but setup for single column mode.
   */
  void DontFindTabVectors(int resolution, BLOBNBOX_LIST* image_blobs,
                          TO_BLOCK* block, FCOORD* reskew);

  /**
   * Return the TabVector that corresponds to the right edge for the given
   * box. If there is a TabVector to the right that vertically overlaps it,
   * then return it, otherwise return NULL. Note that Right and Left refer
   * to the position of the TabVector, not its type, ie RightTabForBox
   * returns the nearest TabVector to the right of the box, regardless of
   * its type.
   * If a TabVector crosses right through the box (as opposed to grazing one
   * edge or missing entirely), then crossing false will ignore such a line.
   * Crossing true will return the line for BOTH left and right edges.
   * If extended is true, then TabVectors are considered to extend to their
   * extended_start/end_y, otherwise, just the startpt_ and endpt_.
   * These functions make use of an internal iterator to the vectors_ list
   * for speed when used repeatedly on neighbouring boxes. The caveat is
   * that the iterator must be updated whenever the list is modified.
   */
  TabVector* RightTabForBox(const TBOX& box, bool crossing, bool extended);
  /**
   * As RightTabForBox, but finds the left TabVector instead.
   */
  TabVector* LeftTabForBox(const TBOX& box, bool crossing, bool extended);
  /**
   * Helper function to setup search limits for *TabForBox.
   */
  void SetupTabSearch(int x, int y, int* min_key, int* max_key);

  /**
   * Display the tab vectors found in this grid.
   */
  ScrollView* DisplayTabVectors(ScrollView* tab_win);

 private:
  // First part of FindTabVectors, which may be used twice if the text
  // is mostly of vertical alignment.
  void FindInitialTabVectors(BLOBNBOX_LIST* image_blobs, TO_BLOCK* block);

  // For each box in the grid, decide whether it is a candidate tab-stop,
  // and if so add it to the tab_grid_.
  ScrollView* FindTabBoxes();

  // Return true if this box looks like a candidate tab stop, and set
  // the appropriate tab type(s) to TT_UNCONFIRMED.
  bool TestBoxForTabs(BLOBNBOX* bbox);

  // Fills the list of TabVector with the tabstops found in the grid,
  // and estimates the logical vertical direction.
  void FindAllTabVectors();
  // Helper for FindAllTabVectors finds the vectors of a particular type.
  int FindTabVectors(int search_size_multiple,
                     TabAlignment alignment,
                     TabVector_LIST* vectors,
                     int* vertical_x, int* vertical_y);
  // Finds a vector corresponding to a tabstop running through the
  // given box of the given alignment type.
  // search_size_multiple is a multiple of height used to control
  // the size of the search.
  // vertical_x and y are updated with an estimate of the real
  // vertical direction. (skew finding.)
  // Returns NULL if no decent tabstop can be found.
  TabVector* FindTabVector(int search_size_multiple,
                           TabAlignment alignment,
                           BLOBNBOX* bbox,
                           int* vertical_x, int* vertical_y);

  // Set the vertical_skew_ member from the given vector and refit
  // all vectors parallel to the skew vector.
  void SetVerticalSkewAndParellelize(int vertical_x, int vertical_y);

  // Sort all the current vectors using the vertical_skew_ vector.
  void SortVectors();

  // Evaluate all the current tab vectors.
  void EvaluateTabs();

  // Trace textlines from one side to the other of each tab vector, saving
  // the most frequent column widths found in a list so that a given width
  // can be tested for being a common width with a simple callback function.
  void ComputeColumnWidths(ScrollView* tab_win);

  // Set the region_type_ member for all the blobs in the grid.
  void ComputeBlobGoodness();

  // Set the region_type_ member of the blob, if not already known.
  void SetBlobRegionType(BLOBNBOX* blob);

  // Mark blobs as being in a vertical text line where that is the case.
  void MarkVerticalText();

  // Returns true if the majority of the image is vertical text lines.
  bool TextMostlyVertical();

  // If this box looks like it is on a textline in the given direction,
  // return the width of the textline-like group of blobs, and the number
  // of blobs found.
  // For more detail see FindTextlineSegment below.
  int FindTextlineWidth(bool right_to_left, BLOBNBOX* bbox, int* blob_count);

  // Search from the given tabstop bbox to the next opposite
  // tabstop bbox on the same text line, which may be itself.
  // Returns true if the search is successful, and sets
  // start_pt, end_pt to the fitted baseline, width to the measured
  // width of the text line (column width estimate.)
  bool TraceTextline(BLOBNBOX* bbox, ICOORD* start_pt, ICOORD* end_pt,
                     int* left_edge, int* right_edge);

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
  int FindTextlineSegment(bool right_to_lefts, bool ignore_images,
                          BLOBNBOX* bbox, int* blob_count,
                          ICOORD* start_pt, ICOORD* end_pt,
                          TabVector** left_vector, TabVector** right_vector,
                          BLOBNBOX** left_blob, BLOBNBOX** right_blob);

  // Find the next adjacent (to left or right) blob on this text line,
  // with the constraint that it must vertically significantly overlap
  // the [top_y, bottom_y] range.
  // If ignore_images is true, then blobs with aligned_text() < 0 are treated
  // as if they do not exist.
  BLOBNBOX* AdjacentBlob(const BLOBNBOX* bbox,
                         bool right_to_left, bool ignore_images,
                         int gap_limit, int top_y, int bottom_y);

  // Add a bi-directional partner relationship between the left
  // and the right. If one (or both) of the vectors is a separator,
  // extend a nearby extendable vector or create a new one of the
  // correct type, using the given left or right blob as a guide.
  void AddPartnerVector(BLOBNBOX* left_blob, BLOBNBOX* right_blob,
                        TabVector* left, TabVector* right);

  /**
   * Remove separators and unused tabs from the main vectors_ list
   * to the dead_vectors_ list.
   */
  void CleanupTabs();

  /**
   * Deskew the tab vectors and blobs, computing the rotation and resetting
   * the storked vertical_skew_. The deskew inverse is returned in reskew.
   */
  void Deskew(TabVector_LIST* hlines, BLOBNBOX_LIST* image_blobs,
              TO_BLOCK* block, FCOORD* reskew);

  /**
   * Restart everything and rotate the input blobs ready for vertical text.
   */
  void ResetForVerticalText(TabVector_LIST* hlines, BLOBNBOX_LIST* image_blobs,
                            TO_BLOCK* block, FCOORD* rerotate);

  /**
   * Compute and apply constraints to the end positions of TabVectors so
   * that where possible partners end at the same y coordinate.
   */
  void ApplyTabConstraints();

 protected:
  ICOORD vertical_skew_;          //< Estimate of true vertical in this image.
  int resolution_;                //< Of source image in pixels per inch.
 private:
  ICOORD image_origin_;           // Top-left of image in deskewed coords
  TabVector_LIST vectors_;        //< List of rule line and tabstops.
  TabVector_IT v_it_;             //< Iterator for searching vectors_.
  TabVector_LIST dead_vectors_;   //< Separators and unpartnered tab vectors.
  ICOORDELT_LIST column_widths_;  //< List of commonly occurring widths.
  /** Callback to test an int for being a common width. */
  WidthCallback* width_cb_;
  /** Instance of the base class that contains only candidate tab stops. */
  BBGrid<BLOBNBOX, BLOBNBOX_CLIST, BLOBNBOX_C_IT>* tab_grid_;
};

}  // namespace tesseract.

#endif  // TESSERACT_TEXTORD_TABFIND_H__

