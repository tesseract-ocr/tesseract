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
#include "tesscallback.h"
#include "tabvector.h"
#include "linefind.h"

extern BOOL_VAR_H(textord_tabfind_force_vertical_text, false,
       "Force using vertical text page mode");
extern BOOL_VAR_H(textord_tabfind_vertical_horizontal_mix, true,
       "find horizontal lines such as headers in vertical page mode");
extern double_VAR_H(textord_tabfind_vertical_text_ratio, 0.5,
       "Fraction of textlines deemed vertical to use vertical page mode");
extern double_VAR_H(textord_tabfind_aligned_gap_fraction, 0.75,
       "Fraction of height used as a minimum gap for aligned blobs.");

class BLOBNBOX;
class BLOBNBOX_LIST;
class TO_BLOCK;
class ScrollView;
struct Pix;

namespace tesseract {

typedef TessResultCallback1<bool, int> WidthCallback;

struct AlignedBlobParams;
class ColPartitionGrid;

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
          TabVector_LIST* vlines, int vertical_x, int vertical_y,
          int resolution);
  virtual ~TabFind();

  /**
   * Insert a list of blobs into the given grid (not necessarily this).
   * See InsertBlob for the other arguments.
   * It would seem to make more sense to swap this and grid, but this way
   * around allows grid to not be derived from TabFind, eg a ColPartitionGrid,
   * while the grid that provides the tab stops(this) has to be derived from
   * TabFind.
   */
  void InsertBlobsToGrid(bool h_spread, bool v_spread,
                         BLOBNBOX_LIST* blobs,
                         BBGrid<BLOBNBOX, BLOBNBOX_CLIST, BLOBNBOX_C_IT>* grid);

  /**
   * Insert a single blob into the given grid (not necessarily this).
   * If h_spread, then all cells covered horizontally by the box are
   * used, otherwise, just the bottom-left. Similarly for v_spread.
   * A side effect is that the left and right rule edges of the blob are
   * set according to the tab vectors in this (not grid).
   */
  bool InsertBlob(bool h_spread, bool v_spread, BLOBNBOX* blob,
                  BBGrid<BLOBNBOX, BLOBNBOX_CLIST, BLOBNBOX_C_IT>* grid);
  // Calls SetBlobRuleEdges for all the blobs in the given block.
  void SetBlockRuleEdges(TO_BLOCK* block);
  // Sets the left and right rule and crossing_rules for the blobs in the given
  // list by finding the next outermost tabvectors for each blob.
  void SetBlobRuleEdges(BLOBNBOX_LIST* blobs);

  // Returns the gutter width of the given TabVector between the given y limits.
  // Also returns x-shift to be added to the vector to clear any intersecting
  // blobs. The shift is deducted from the returned gutter.
  // If ignore_unmergeables is true, then blobs of UnMergeableType are
  // ignored as if they don't exist. (Used for text on image.)
  // max_gutter_width is used as the maximum width worth searching for in case
  // there is nothing near the TabVector.
  int GutterWidth(int bottom_y, int top_y, const TabVector& v,
                  bool ignore_unmergeables, int max_gutter_width,
                  int* required_shift);
  /**
   * Find the gutter width and distance to inner neighbour for the given blob.
   */
  void GutterWidthAndNeighbourGap(int tab_x, int mean_height,
                                  int max_gutter, bool left,
                                  BLOBNBOX* bbox, int* gutter_width,
                                  int* neighbour_gap);

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
   * Return true if the sizes are more than a
   * factor of 5 different.
   */
  static bool VeryDifferentSizes(int size1, int size2);

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
  TabVector_LIST* vectors() {
    return &vectors_;
  }
  TabVector_LIST* dead_vectors() {
    return &dead_vectors_;
  }

  /**
   * Top-level function to find TabVectors in an input page block.
   * Returns false if the detected skew angle is impossible.
   * Applies the detected skew angle to deskew the tabs, blobs and part_grid.
   */
  bool FindTabVectors(TabVector_LIST* hlines,
                      BLOBNBOX_LIST* image_blobs, TO_BLOCK* block,
                      int min_gutter_width,
                      ColPartitionGrid* part_grid,
                      FCOORD* deskew, FCOORD* reskew);

  // Top-level function to not find TabVectors in an input page block,
  // but setup for single column mode.
  void DontFindTabVectors(BLOBNBOX_LIST* image_blobs,
                          TO_BLOCK* block, FCOORD* deskew, FCOORD* reskew);

  // Cleans up the lists of blobs in the block ready for use by TabFind.
  // Large blobs that look like text are moved to the main blobs list.
  // Main blobs that are superseded by the image blobs are deleted.
  void TidyBlobs(TO_BLOCK* block);

  // Helper function to setup search limits for *TabForBox.
  void SetupTabSearch(int x, int y, int* min_key, int* max_key);

  /**
   * Display the tab vectors found in this grid.
   */
  ScrollView* DisplayTabVectors(ScrollView* tab_win);

  // First part of FindTabVectors, which may be used twice if the text
  // is mostly of vertical alignment.  If find_vertical_text flag is
  // true, this finds vertical textlines in possibly rotated blob space.
  // In other words, when the page has mostly vertical lines and is rotated,
  // setting this to true will find horizontal lines on the page.
  ScrollView* FindInitialTabVectors(BLOBNBOX_LIST* image_blobs,
                                    int min_gutter_width, TO_BLOCK* block);

  // Apply the given rotation to the given list of blobs.
  static void RotateBlobList(const FCOORD& rotation, BLOBNBOX_LIST* blobs);

  // Flip the vertical and horizontal lines and rotate the grid ready
  // for working on the rotated image.
  // The min_gutter_width will be adjusted to the median gutter width between
  // vertical tabs to set a better threshold for tabboxes in the 2nd pass.
  void ResetForVerticalText(const FCOORD& rotate, const FCOORD& rerotate,
                            TabVector_LIST* horizontal_lines,
                            int* min_gutter_width);

  // Clear the grid and get rid of the tab vectors, but not separators,
  // ready to start again.
  void Reset();

  // Reflect the separator tab vectors and the grids in the y-axis.
  // Can only be called after Reset!
  void ReflectInYAxis();

 private:
  // For each box in the grid, decide whether it is a candidate tab-stop,
  // and if so add it to the left and right tab boxes.
  ScrollView* FindTabBoxes(int min_gutter_width);

  // Return true if this box looks like a candidate tab stop, and set
  // the appropriate tab type(s) to TT_UNCONFIRMED.
  bool TestBoxForTabs(BLOBNBOX* bbox, int min_gutter_width);

  // Returns true if there is nothing in the rectangle of width min_gutter to
  // the left of bbox.
  bool ConfirmRaggedLeft(BLOBNBOX* bbox, int min_gutter);
  // Returns true if there is nothing in the rectangle of width min_gutter to
  // the right of bbox.
  bool ConfirmRaggedRight(BLOBNBOX* bbox, int min_gutter);
  // Returns true if there is nothing in the given search_box that vertically
  // overlaps target_box other than target_box itself.
  bool NothingYOverlapsInBox(const TBOX& search_box, const TBOX& target_box);

  // Fills the list of TabVector with the tabstops found in the grid,
  // and estimates the logical vertical direction.
  void FindAllTabVectors(int min_gutter_width);
  // Helper for FindAllTabVectors finds the vectors of a particular type.
  int FindTabVectors(int search_size_multiple,
                     TabAlignment alignment,
                     int min_gutter_width,
                     TabVector_LIST* vectors,
                     int* vertical_x, int* vertical_y);
  // Finds a vector corresponding to a tabstop running through the
  // given box of the given alignment type.
  // search_size_multiple is a multiple of height used to control
  // the size of the search.
  // vertical_x and y are updated with an estimate of the real
  // vertical direction. (skew finding.)
  // Returns NULL if no decent tabstop can be found.
  TabVector* FindTabVector(int search_size_multiple, int min_gutter_width,
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
  void ComputeColumnWidths(ScrollView* tab_win,
                           ColPartitionGrid* part_grid);

  // Find column width and pair-up tab vectors with existing ColPartitions.
  void ApplyPartitionsToColumnWidths(ColPartitionGrid* part_grid,
                                     STATS* col_widths);

  // Helper makes the list of common column widths in column_widths_ from the
  // input col_widths. Destroys the content of col_widths by repeatedly
  // finding the mode and erasing the peak.
  void MakeColumnWidths(int col_widths_size, STATS* col_widths);

  // Mark blobs as being in a vertical text line where that is the case.
  void MarkVerticalText();

  // Returns the median gutter width between pairs of matching tab vectors
  // assuming they are sorted left-to-right.  If there are too few data
  // points (< kMinLinesInColumn), then 0 is returned.
  int FindMedianGutterWidth(TabVector_LIST* tab_vectors);

  // Find the next adjacent (to left or right) blob on this text line,
  // with the constraint that it must vertically significantly overlap
  // the [top_y, bottom_y] range.
  // If ignore_images is true, then blobs with aligned_text() < 0 are treated
  // as if they do not exist.
  BLOBNBOX* AdjacentBlob(const BLOBNBOX* bbox,
                         bool look_left, bool ignore_images,
                         double min_overlap_fraction,
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
   * Returns false if the detected skew angle is impossible.
   */
  bool Deskew(TabVector_LIST* hlines, BLOBNBOX_LIST* image_blobs,
              TO_BLOCK* block, FCOORD* deskew, FCOORD* reskew);

  // Compute the rotation required to deskew, and its inverse rotation.
  void ComputeDeskewVectors(FCOORD* deskew, FCOORD* reskew);

  /**
   * Compute and apply constraints to the end positions of TabVectors so
   * that where possible partners end at the same y coordinate.
   */
  void ApplyTabConstraints();

 protected:
  ICOORD vertical_skew_;          //< Estimate of true vertical in this image.
  int resolution_;                //< Of source image in pixels per inch.
 private:
  ICOORD image_origin_;           //< Top-left of image in deskewed coords
  TabVector_LIST vectors_;        //< List of rule line and tabstops.
  TabVector_IT v_it_;             //< Iterator for searching vectors_.
  TabVector_LIST dead_vectors_;   //< Separators and unpartnered tab vectors.
  ICOORDELT_LIST column_widths_;  //< List of commonly occurring widths.
  /** Callback to test an int for being a common width. */
  WidthCallback* width_cb_;
  // Sets of bounding boxes that are candidate tab stops.
  GenericVector<BLOBNBOX*> left_tab_boxes_;
  GenericVector<BLOBNBOX*> right_tab_boxes_;
};

}  // namespace tesseract.

#endif  // TESSERACT_TEXTORD_TABFIND_H__
