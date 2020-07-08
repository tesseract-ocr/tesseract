///////////////////////////////////////////////////////////////////////
// File:        bbgrid.h
// Description: Class to hold BLOBNBOXs in a grid for fast access
//              to neighbours.
// Author:      Ray Smith
//
// (C) Copyright 2007, Google Inc.
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

#ifndef TESSERACT_TEXTORD_BBGRID_H_
#define TESSERACT_TEXTORD_BBGRID_H_

#include <unordered_set>

#include "clst.h"
#include "coutln.h"
#include "rect.h"
#include "scrollview.h"

#include "allheaders.h"

class BLOCK;

namespace tesseract {

// Helper function to return a scaled Pix with one pixel per grid cell,
// set (black) where the given outline enters the corresponding grid cell,
// and clear where the outline does not touch the grid cell.
// Also returns the grid coords of the bottom-left of the Pix, in *left
// and *bottom, which corresponds to (0, 0) on the Pix.
// Note that the Pix is used upside-down, with (0, 0) being the bottom-left.
Pix* TraceOutlineOnReducedPix(C_OUTLINE* outline, int gridsize,
                              ICOORD bleft, int* left, int* bottom);
// As TraceOutlineOnReducedPix above, but on a BLOCK instead of a C_OUTLINE.
Pix* TraceBlockOnReducedPix(BLOCK* block, int gridsize,
                            ICOORD bleft, int* left, int* bottom);

template<class BBC, class BBC_CLIST, class BBC_C_IT> class GridSearch;

// The GridBase class is the base class for BBGrid and IntGrid.
// It holds the geometry and scale of the grid.
class GridBase {
 public:
  GridBase() = default;
  GridBase(int gridsize, const ICOORD& bleft, const ICOORD& tright);
  virtual ~GridBase();

  // (Re)Initialize the grid. The gridsize is the size in pixels of each cell,
  // and bleft, tright are the bounding box of everything to go in it.
  void Init(int gridsize, const ICOORD& bleft, const ICOORD& tright);

  // Simple accessors.
  int gridsize() const {
    return gridsize_;
  }
  int gridwidth() const {
    return gridwidth_;
  }
  int gridheight() const {
    return gridheight_;
  }
  const ICOORD& bleft() const {
    return bleft_;
  }
  const ICOORD& tright() const {
    return tright_;
  }
  // Compute the given grid coordinates from image coords.
  void GridCoords(int x, int y, int* grid_x, int* grid_y) const;

  // Clip the given grid coordinates to fit within the grid.
  void ClipGridCoords(int* x, int* y) const;

 protected:
  // TODO(rays) Make these private and migrate to the accessors in subclasses.
  int gridsize_;     // Pixel size of each grid cell.
  int gridwidth_;    // Size of the grid in cells.
  int gridheight_;
  int gridbuckets_;  // Total cells in grid.
  ICOORD bleft_;     // Pixel coords of bottom-left of grid.
  ICOORD tright_;    // Pixel coords of top-right of grid.

 private:
};

// The IntGrid maintains a single int for each cell in a grid.
class IntGrid : public GridBase {
 public:
  IntGrid();
  IntGrid(int gridsize, const ICOORD& bleft, const ICOORD& tright);
  ~IntGrid() override;

  // (Re)Initialize the grid. The gridsize is the size in pixels of each cell,
  // and bleft, tright are the bounding box of everything to go in it.
  void Init(int gridsize, const ICOORD& bleft, const ICOORD& tright);

  // Clear all the ints in the grid to zero.
  void Clear();

  // Rotate the grid by rotation, keeping cell contents.
  // rotation must be a multiple of 90 degrees.
  // NOTE: due to partial cells, cell coverage in the rotated grid will be
  // inexact. This is why there is no Rotate for the generic BBGrid.
  void Rotate(const FCOORD& rotation);

  // Returns a new IntGrid containing values equal to the sum of all the
  // neighbouring cells. The returned grid must be deleted after use.
  IntGrid* NeighbourhoodSum() const;

  int GridCellValue(int grid_x, int grid_y) const {
    ClipGridCoords(&grid_x, &grid_y);
    return grid_[grid_y * gridwidth_ + grid_x];
  }
  void SetGridCell(int grid_x, int grid_y, int value) {
    ASSERT_HOST(grid_x >= 0 && grid_x < gridwidth());
    ASSERT_HOST(grid_y >= 0 && grid_y < gridheight());
    grid_[grid_y * gridwidth_ + grid_x] = value;
  }
  // Returns true if more than half the area of the rect is covered by grid
  // cells that are over the threshold.
  bool RectMostlyOverThreshold(const TBOX& rect, int threshold) const;

  // Returns true if any cell value in the given rectangle is zero.
  bool AnyZeroInRect(const TBOX& rect) const;

  // Returns a full-resolution binary pix in which each cell over the given
  // threshold is filled as a black square. pixDestroy after use.
  Pix* ThresholdToPix(int threshold) const;

 private:
  int* grid_;  // 2-d array of ints.
};

// The BBGrid class holds C_LISTs of template classes BBC (bounding box class)
// in a grid for fast neighbour access.
// The BBC class must have a member const TBOX& bounding_box() const.
// The BBC class must have been CLISTIZEH'ed elsewhere to make the
// list class BBC_CLIST and the iterator BBC_C_IT.
// Use of C_LISTs enables BBCs to exist in multiple cells simultaneously.
// As a consequence, ownership of BBCs is assumed to be elsewhere and
// persistent for at least the life of the BBGrid, or at least until Clear is
// called which removes all references to inserted objects without actually
// deleting them.
// Most uses derive a class from a specific instantiation of BBGrid,
// thereby making most of the ugly template notation go away.
// The friend class GridSearch, with the same template arguments, is
// used to search a grid efficiently in one of several search patterns.
template<class BBC, class BBC_CLIST, class BBC_C_IT> class BBGrid
  : public GridBase {
  friend class GridSearch<BBC, BBC_CLIST, BBC_C_IT>;
 public:
  BBGrid();
  BBGrid(int gridsize, const ICOORD& bleft, const ICOORD& tright);
  ~BBGrid() override;

  // (Re)Initialize the grid. The gridsize is the size in pixels of each cell,
  // and bleft, tright are the bounding box of everything to go in it.
  void Init(int gridsize, const ICOORD& bleft, const ICOORD& tright);

  // Empty all the lists but leave the grid itself intact.
  void Clear();
  // Deallocate the data in the lists but otherwise leave the lists and the grid
  // intact.
  void ClearGridData(void (*free_method)(BBC*));

  // Insert a bbox into the appropriate place in the grid.
  // If h_spread, then all cells covered horizontally by the box are
  // used, otherwise, just the bottom-left. Similarly for v_spread.
  // WARNING: InsertBBox may invalidate an active GridSearch. Call
  // RepositionIterator() on any GridSearches that are active on this grid.
  void InsertBBox(bool h_spread, bool v_spread, BBC* bbox);

  // Using a pix from TraceOutlineOnReducedPix or TraceBlockOnReducedPix, in
  // which each pixel corresponds to a grid cell, insert a bbox into every
  // place in the grid where the corresponding pixel is 1. The Pix is handled
  // upside-down to match the Tesseract coordinate system. (As created by
  // TraceOutlineOnReducedPix or TraceBlockOnReducedPix.)
  // (0, 0) in the pix corresponds to (left, bottom) in the
  // grid (in grid coords), and the pix works up the grid from there.
  // WARNING: InsertPixPtBBox may invalidate an active GridSearch. Call
  // RepositionIterator() on any GridSearches that are active on this grid.
  void InsertPixPtBBox(int left, int bottom, Pix* pix, BBC* bbox);

  // Remove the bbox from the grid.
  // WARNING: Any GridSearch operating on this grid could be invalidated!
  // If a GridSearch is operating, call GridSearch::RemoveBBox() instead.
  void RemoveBBox(BBC* bbox);

  // Returns true if the given rectangle has no overlapping elements.
  bool RectangleEmpty(const TBOX& rect);

  // Returns an IntGrid showing the number of elements in each cell.
  // Returned IntGrid must be deleted after use.
  IntGrid* CountCellElements();

  // Make a window of an appropriate size to display things in the grid.
  ScrollView* MakeWindow(int x, int y, const char* window_name);

  // Display the bounding boxes of the BLOBNBOXes in this grid.
  // Use of this function requires an additional member of the BBC class:
  // ScrollView::Color BBC::BoxColor() const.
  void DisplayBoxes(ScrollView* window);

  // ASSERT_HOST that every cell contains no more than one copy of each entry.
  void AssertNoDuplicates();

  // Handle a click event in a display window.
  virtual void HandleClick(int x, int y);

 protected:
  BBC_CLIST* grid_;  // 2-d array of CLISTS of BBC elements.

 private:
};

// Hash functor for generic pointers.
template<typename T> struct PtrHash {
  size_t operator()(const T* ptr) const {
    return reinterpret_cast<uintptr_t>(ptr) / sizeof(T);
  }
};


// The GridSearch class enables neighbourhood searching on a BBGrid.
template<class BBC, class BBC_CLIST, class BBC_C_IT> class GridSearch {
 public:
  GridSearch(BBGrid<BBC, BBC_CLIST, BBC_C_IT>* grid)
      : grid_(grid) {
  }

  // Get the grid x, y coords of the most recently returned BBC.
  int GridX() const {
    return x_;
  }
  int GridY() const {
    return y_;
  }

  // Sets the search mode to return a box only once.
  // Efficiency warning: Implementation currently uses a squared-order
  // search in the number of returned elements. Use only where a small
  // number of elements are spread over a wide area, eg ColPartitions.
  void SetUniqueMode(bool mode) {
    unique_mode_ = mode;
  }
  // TODO(rays) Replace calls to ReturnedSeedElement with SetUniqueMode.
  // It only works if the search includes the bottom-left corner.
  // Apart from full search, all other searches return a box several
  // times if the box is inserted with h_spread or v_spread.
  // This method will return true for only one occurrence of each box
  // that was inserted with both h_spread and v_spread as true.
  // It will usually return false for boxes that were not inserted with
  // both h_spread=true and v_spread=true
  bool ReturnedSeedElement() const {
    TBOX box = previous_return_->bounding_box();
    int x_center = (box.left()+box.right())/2;
    int y_center = (box.top()+box.bottom())/2;
    int grid_x, grid_y;
    grid_->GridCoords(x_center, y_center, &grid_x, &grid_y);
    return (x_ == grid_x) && (y_ == grid_y);
  }

  // Various searching iterations... Note that these iterations
  // all share data members, so you can't run more than one iteration
  // in parallel in a single GridSearch instance, but multiple instances
  // can search the same BBGrid in parallel.
  // Note that all the searches can return blobs that may not exactly
  // match the search conditions, since they return everything in the
  // covered grid cells. It is up to the caller to check for
  // appropriateness.
  // TODO(rays) NextRectSearch only returns valid elements. Make the other
  // searches test before return also and remove the tests from code
  // that uses GridSearch.

  // Start a new full search. Will iterate all stored blobs, from the top.
  // If the blobs have been inserted using InsertBBox, (not InsertPixPtBBox)
  // then the full search guarantees to return each blob in the grid once.
  // Other searches may return a blob more than once if they have been
  // inserted using h_spread or v_spread.
  void StartFullSearch();
  // Return the next bbox in the search or nullptr if done.
  BBC* NextFullSearch();

  // Start a new radius search. Will search in a spiral up to a
  // given maximum radius in grid cells from the given center in pixels.
  void StartRadSearch(int x, int y, int max_radius);
  // Return the next bbox in the radius search or nullptr if the
  // maximum radius has been reached.
  BBC* NextRadSearch();

  // Start a new left or right-looking search. Will search to the side
  // for a box that vertically overlaps the given vertical line segment.
  // CAVEAT: This search returns all blobs from the cells to the side
  // of the start, and somewhat below, since there is no guarantee
  // that there may not be a taller object in a lower cell. The
  // blobs returned will include all those that vertically overlap and
  // are no more than twice as high, but may also include some that do
  // not overlap and some that are more than twice as high.
  void StartSideSearch(int x, int ymin, int ymax);
  // Return the next bbox in the side search or nullptr if the
  // edge has been reached. Searches left to right or right to left
  // according to the flag.
  BBC* NextSideSearch(bool right_to_left);

  // Start a vertical-looking search. Will search up or down
  // for a box that horizontally overlaps the given line segment.
  void StartVerticalSearch(int xmin, int xmax, int y);
  // Return the next bbox in the vertical search or nullptr if the
  // edge has been reached. Searches top to bottom or bottom to top
  // according to the flag.
  BBC* NextVerticalSearch(bool top_to_bottom);

  // Start a rectangular search. Will search for a box that overlaps the
  // given rectangle.
  void StartRectSearch(const TBOX& rect);
  // Return the next bbox in the rectangular search or nullptr if complete.
  BBC* NextRectSearch();

  // Remove the last returned BBC. Will not invalidate this. May invalidate
  // any other concurrent GridSearch on the same grid. If any others are
  // in use, call RepositionIterator on those, to continue without harm.
  void RemoveBBox();
  void RepositionIterator();

 private:
  // Factored out helper to start a search.
  void CommonStart(int x, int y);
  // Factored out helper to complete a next search.
  BBC* CommonNext();
  // Factored out final return when search is exhausted.
  BBC* CommonEnd();
  // Factored out function to set the iterator to the current x_, y_
  // grid coords and mark the cycle pt.
  void SetIterator();

 private:
  // The grid we are searching.
  BBGrid<BBC, BBC_CLIST, BBC_C_IT>* grid_ = nullptr;
  // For executing a search. The different search algorithms use these in
  // different ways, but most use x_origin_ and y_origin_ as the start position.
  int x_origin_ = 0;
  int y_origin_ = 0;
  int max_radius_ = 0;
  int radius_ = 0;
  int rad_index_ = 0;
  int rad_dir_ = 0;
  TBOX rect_;
  int x_ = 0; // The current location in grid coords, of the current search.
  int y_ = 0;
  bool unique_mode_ = false;
  BBC* previous_return_ = nullptr; // Previous return from Next*.
  BBC* next_return_ = nullptr; // Current value of it_.data() used for repositioning.
  // An iterator over the list at (x_, y_) in the grid_.
  BBC_C_IT it_;
  // Set of unique returned elements used when unique_mode_ is true.
  std::unordered_set<BBC*, PtrHash<BBC> > returns_;
};

// Sort function to sort a BBC by bounding_box().left().
template<class BBC>
int SortByBoxLeft(const void* void1, const void* void2) {
  // The void*s are actually doubly indirected, so get rid of one level.
  const BBC* p1 = *static_cast<const BBC* const*>(void1);
  const BBC* p2 = *static_cast<const BBC* const*>(void2);
  int result = p1->bounding_box().left() - p2->bounding_box().left();
  if (result != 0)
    return result;
  result = p1->bounding_box().right() - p2->bounding_box().right();
  if (result != 0)
    return result;
  result = p1->bounding_box().bottom() - p2->bounding_box().bottom();
  if (result != 0)
    return result;
  return p1->bounding_box().top() - p2->bounding_box().top();
}

// Sort function to sort a BBC by bounding_box().right() in right-to-left order.
template<class BBC>
int SortRightToLeft(const void* void1, const void* void2) {
  // The void*s are actually doubly indirected, so get rid of one level.
  const BBC* p1 = *static_cast<const BBC* const*>(void1);
  const BBC* p2 = *static_cast<const BBC* const*>(void2);
  int result = p2->bounding_box().right() - p1->bounding_box().right();
  if (result != 0)
    return result;
  result = p2->bounding_box().left() - p1->bounding_box().left();
  if (result != 0)
    return result;
  result = p1->bounding_box().bottom() - p2->bounding_box().bottom();
  if (result != 0)
    return result;
  return p1->bounding_box().top() - p2->bounding_box().top();
}

// Sort function to sort a BBC by bounding_box().bottom().
template<class BBC>
int SortByBoxBottom(const void* void1, const void* void2) {
  // The void*s are actually doubly indirected, so get rid of one level.
  const BBC* p1 = *static_cast<const BBC* const*>(void1);
  const BBC* p2 = *static_cast<const BBC* const*>(void2);
  int result = p1->bounding_box().bottom() - p2->bounding_box().bottom();
  if (result != 0)
    return result;
  result =  p1->bounding_box().top() - p2->bounding_box().top();
  if (result != 0)
    return result;
  result = p1->bounding_box().left() - p2->bounding_box().left();
  if (result != 0)
    return result;
  return p1->bounding_box().right() - p2->bounding_box().right();
}

///////////////////////////////////////////////////////////////////////
// BBGrid IMPLEMENTATION.
///////////////////////////////////////////////////////////////////////
template<class BBC, class BBC_CLIST, class BBC_C_IT>
BBGrid<BBC, BBC_CLIST, BBC_C_IT>::BBGrid() : grid_(nullptr) {
}

template<class BBC, class BBC_CLIST, class BBC_C_IT>
BBGrid<BBC, BBC_CLIST, BBC_C_IT>::BBGrid(
  int gridsize, const ICOORD& bleft, const ICOORD& tright)
    : grid_(nullptr) {
  Init(gridsize, bleft, tright);
}

template<class BBC, class BBC_CLIST, class BBC_C_IT>
BBGrid<BBC, BBC_CLIST, BBC_C_IT>::~BBGrid() {
  delete [] grid_;
}

// (Re)Initialize the grid. The gridsize is the size in pixels of each cell,
// and bleft, tright are the bounding box of everything to go in it.
template<class BBC, class BBC_CLIST, class BBC_C_IT>
void BBGrid<BBC, BBC_CLIST, BBC_C_IT>::Init(int gridsize,
                                            const ICOORD& bleft,
                                            const ICOORD& tright) {
  GridBase::Init(gridsize, bleft, tright);
  delete [] grid_;
  grid_ = new BBC_CLIST[gridbuckets_];
}

// Clear all lists, but leave the array of lists present.
template<class BBC, class BBC_CLIST, class BBC_C_IT>
void BBGrid<BBC, BBC_CLIST, BBC_C_IT>::Clear() {
  for (int i = 0; i < gridbuckets_; ++i) {
    grid_[i].shallow_clear();
  }
}

// Deallocate the data in the lists but otherwise leave the lists and the grid
// intact.
template<class BBC, class BBC_CLIST, class BBC_C_IT>
void BBGrid<BBC, BBC_CLIST, BBC_C_IT>::ClearGridData(
    void (*free_method)(BBC*)) {
  if (grid_ == nullptr) return;
  GridSearch<BBC, BBC_CLIST, BBC_C_IT> search(this);
  search.StartFullSearch();
  BBC* bb;
  BBC_CLIST bb_list;
  BBC_C_IT it(&bb_list);
  while ((bb = search.NextFullSearch()) != nullptr) {
    it.add_after_then_move(bb);
  }
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    free_method(it.data());
  }
}

// Insert a bbox into the appropriate place in the grid.
// If h_spread, then all cells covered horizontally by the box are
// used, otherwise, just the bottom-left. Similarly for v_spread.
// WARNING: InsertBBox may invalidate an active GridSearch. Call
// RepositionIterator() on any GridSearches that are active on this grid.
template<class BBC, class BBC_CLIST, class BBC_C_IT>
void BBGrid<BBC, BBC_CLIST, BBC_C_IT>::InsertBBox(bool h_spread, bool v_spread,
                                                  BBC* bbox) {
  TBOX box = bbox->bounding_box();
  int start_x, start_y, end_x, end_y;
  GridCoords(box.left(), box.bottom(), &start_x, &start_y);
  GridCoords(box.right(), box.top(), &end_x, &end_y);
  if (!h_spread)
    end_x = start_x;
  if (!v_spread)
    end_y = start_y;
  int grid_index = start_y * gridwidth_;
  for (int y = start_y; y <= end_y; ++y, grid_index += gridwidth_) {
    for (int x = start_x; x <= end_x; ++x) {
      grid_[grid_index + x].add_sorted(SortByBoxLeft<BBC>, true, bbox);
    }
  }
}

// Using a pix from TraceOutlineOnReducedPix or TraceBlockOnReducedPix, in
// which each pixel corresponds to a grid cell, insert a bbox into every
// place in the grid where the corresponding pixel is 1. The Pix is handled
// upside-down to match the Tesseract coordinate system. (As created by
// TraceOutlineOnReducedPix or TraceBlockOnReducedPix.)
// (0, 0) in the pix corresponds to (left, bottom) in the
// grid (in grid coords), and the pix works up the grid from there.
// WARNING: InsertPixPtBBox may invalidate an active GridSearch. Call
// RepositionIterator() on any GridSearches that are active on this grid.
template<class BBC, class BBC_CLIST, class BBC_C_IT>
void BBGrid<BBC, BBC_CLIST, BBC_C_IT>::InsertPixPtBBox(int left, int bottom,
                                                       Pix* pix, BBC* bbox) {
  int width = pixGetWidth(pix);
  int height = pixGetHeight(pix);
  for (int y = 0; y < height; ++y) {
    l_uint32* data = pixGetData(pix) + y * pixGetWpl(pix);
    for (int x = 0; x < width; ++x) {
      if (GET_DATA_BIT(data, x)) {
        grid_[(bottom + y) * gridwidth_ + x + left].
          add_sorted(SortByBoxLeft<BBC>, true, bbox);
      }
    }
  }
}

// Remove the bbox from the grid.
// WARNING: Any GridSearch operating on this grid could be invalidated!
// If a GridSearch is operating, call GridSearch::RemoveBBox() instead.
template<class BBC, class BBC_CLIST, class BBC_C_IT>
void BBGrid<BBC, BBC_CLIST, BBC_C_IT>::RemoveBBox(BBC* bbox) {
  TBOX box = bbox->bounding_box();
  int start_x, start_y, end_x, end_y;
  GridCoords(box.left(), box.bottom(), &start_x, &start_y);
  GridCoords(box.right(), box.top(), &end_x, &end_y);
  int grid_index = start_y * gridwidth_;
  for (int y = start_y; y <= end_y; ++y, grid_index += gridwidth_) {
    for (int x = start_x; x <= end_x; ++x) {
      BBC_C_IT it(&grid_[grid_index + x]);
      for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
        if (it.data() == bbox)
          it.extract();
      }
    }
  }
}

// Returns true if the given rectangle has no overlapping elements.
template<class BBC, class BBC_CLIST, class BBC_C_IT>
bool BBGrid<BBC, BBC_CLIST, BBC_C_IT>::RectangleEmpty(const TBOX& rect) {
  GridSearch<BBC, BBC_CLIST, BBC_C_IT> rsearch(this);
  rsearch.StartRectSearch(rect);
  return rsearch.NextRectSearch() == nullptr;
}

// Returns an IntGrid showing the number of elements in each cell.
// Returned IntGrid must be deleted after use.
template<class BBC, class BBC_CLIST, class BBC_C_IT>
IntGrid* BBGrid<BBC, BBC_CLIST, BBC_C_IT>::CountCellElements() {
  auto* intgrid = new IntGrid(gridsize(), bleft(), tright());
  for (int y = 0; y < gridheight(); ++y) {
    for (int x = 0; x < gridwidth(); ++x) {
      int cell_count = grid_[y * gridwidth() + x].length();
      intgrid->SetGridCell(x, y, cell_count);
    }
  }
  return intgrid;
}


template<class G> class TabEventHandler : public SVEventHandler {
 public:
  explicit TabEventHandler(G* grid) : grid_(grid) {
  }
  void Notify(const SVEvent* sv_event) override {
    if (sv_event->type == SVET_CLICK) {
      grid_->HandleClick(sv_event->x, sv_event->y);
    }
  }
 private:
  G* grid_;
};

#ifndef GRAPHICS_DISABLED

// Make a window of an appropriate size to display things in the grid.
// Position the window at the given x,y.
template<class BBC, class BBC_CLIST, class BBC_C_IT>
ScrollView* BBGrid<BBC, BBC_CLIST, BBC_C_IT>::MakeWindow(
    int x, int y, const char* window_name) {
  auto tab_win = new ScrollView(window_name, x, y,
                                tright_.x() - bleft_.x(),
                                tright_.y() - bleft_.y(),
                                tright_.x() - bleft_.x(),
                                tright_.y() - bleft_.y(),
                                true);
  auto* handler =
    new TabEventHandler<BBGrid<BBC, BBC_CLIST, BBC_C_IT> >(this);
  tab_win->AddEventHandler(handler);
  tab_win->Pen(ScrollView::GREY);
  tab_win->Rectangle(0, 0, tright_.x() - bleft_.x(), tright_.y() - bleft_.y());
  return tab_win;
}

// Create a window at (x,y) and display the bounding boxes of the
// BLOBNBOXes in this grid.
// Use of this function requires an additional member of the BBC class:
// ScrollView::Color BBC::BoxColor() const.
template<class BBC, class BBC_CLIST, class BBC_C_IT>
void BBGrid<BBC, BBC_CLIST, BBC_C_IT>::DisplayBoxes(ScrollView* tab_win) {
  tab_win->Pen(ScrollView::BLUE);
  tab_win->Brush(ScrollView::NONE);

  // For every bbox in the grid, display it.
  GridSearch<BBC, BBC_CLIST, BBC_C_IT> gsearch(this);
  gsearch.StartFullSearch();
  BBC* bbox;
  while ((bbox = gsearch.NextFullSearch()) != nullptr) {
    const TBOX& box = bbox->bounding_box();
    int left_x = box.left();
    int right_x = box.right();
    int top_y = box.top();
    int bottom_y = box.bottom();
    ScrollView::Color box_color = bbox->BoxColor();
    tab_win->Pen(box_color);
    tab_win->Rectangle(left_x, bottom_y, right_x, top_y);
  }
  tab_win->Update();
}

#endif // !GRAPHICS_DISABLED

// ASSERT_HOST that every cell contains no more than one copy of each entry.
template<class BBC, class BBC_CLIST, class BBC_C_IT>
void BBGrid<BBC, BBC_CLIST, BBC_C_IT>::AssertNoDuplicates() {
  // Process all grid cells.
  for (int i = gridwidth_ * gridheight_ - 1; i >= 0; --i) {
    // Iterate over all elements excent the last.
    for (BBC_C_IT it(&grid_[i]); !it.at_last(); it.forward()) {
      BBC* ptr = it.data();
      BBC_C_IT it2(it);
      // None of the rest of the elements in the list should equal ptr.
      for (it2.forward(); !it2.at_first(); it2.forward()) {
        ASSERT_HOST(it2.data() != ptr);
      }
    }
  }
}

// Handle a click event in a display window.
template<class BBC, class BBC_CLIST, class BBC_C_IT>
void BBGrid<BBC, BBC_CLIST, BBC_C_IT>::HandleClick(int x, int y) {
  tprintf("Click at (%d, %d)\n", x, y);
}

///////////////////////////////////////////////////////////////////////
// GridSearch IMPLEMENTATION.
///////////////////////////////////////////////////////////////////////

// Start a new full search. Will iterate all stored blobs.
template<class BBC, class BBC_CLIST, class BBC_C_IT>
void GridSearch<BBC, BBC_CLIST, BBC_C_IT>::StartFullSearch() {
  // Full search uses x_ and y_ as the current grid
  // cell being searched.
  CommonStart(grid_->bleft_.x(), grid_->tright_.y());
}

// Return the next bbox in the search or nullptr if done.
// The other searches will return a box that overlaps the grid cell
// thereby duplicating boxes, but NextFullSearch only returns each box once.
template<class BBC, class BBC_CLIST, class BBC_C_IT>
BBC* GridSearch<BBC, BBC_CLIST, BBC_C_IT>::NextFullSearch() {
  int x;
  int y;
  do {
    while (it_.cycled_list()) {
      ++x_;
      if (x_ >= grid_->gridwidth_) {
        --y_;
        if (y_ < 0)
          return CommonEnd();
        x_ = 0;
      }
      SetIterator();
    }
    CommonNext();
    TBOX box = previous_return_->bounding_box();
    grid_->GridCoords(box.left(), box.bottom(), &x, &y);
  } while (x != x_ || y != y_);
  return previous_return_;
}

// Start a new radius search.
template<class BBC, class BBC_CLIST, class BBC_C_IT>
void GridSearch<BBC, BBC_CLIST, BBC_C_IT>::StartRadSearch(int x, int y,
                                                          int max_radius) {
  // Rad search uses x_origin_ and y_origin_ as the center of the circle.
  // The radius_ is the radius of the (diamond-shaped) circle and
  // rad_index/rad_dir_ combine to determine the position around it.
  max_radius_ = max_radius;
  radius_ = 0;
  rad_index_ = 0;
  rad_dir_ = 3;
  CommonStart(x, y);
}

// Return the next bbox in the radius search or nullptr if the
// maximum radius has been reached.
template<class BBC, class BBC_CLIST, class BBC_C_IT>
BBC* GridSearch<BBC, BBC_CLIST, BBC_C_IT>::NextRadSearch() {
  do {
    while (it_.cycled_list()) {
      ++rad_index_;
      if (rad_index_ >= radius_) {
        ++rad_dir_;
        rad_index_ = 0;
        if (rad_dir_ >= 4) {
          ++radius_;
          if (radius_ > max_radius_)
            return CommonEnd();
          rad_dir_ = 0;
        }
      }
      ICOORD offset = C_OUTLINE::chain_step(rad_dir_);
      offset *= radius_ - rad_index_;
      offset += C_OUTLINE::chain_step(rad_dir_ + 1) * rad_index_;
      x_ = x_origin_ + offset.x();
      y_ = y_origin_ + offset.y();
      if (x_ >= 0 && x_ < grid_->gridwidth_ &&
          y_ >= 0 && y_ < grid_->gridheight_)
        SetIterator();
    }
    CommonNext();
  } while (unique_mode_ && returns_.find(previous_return_) != returns_.end());
  if (unique_mode_)
    returns_.insert(previous_return_);
  return previous_return_;
}

// Start a new left or right-looking search. Will search to the side
// for a box that vertically overlaps the given vertical line segment.
template<class BBC, class BBC_CLIST, class BBC_C_IT>
void GridSearch<BBC, BBC_CLIST, BBC_C_IT>::StartSideSearch(int x,
                                                           int ymin, int ymax) {
  // Right search records the x in x_origin_, the ymax in y_origin_
  // and the size of the vertical strip to search in radius_.
  // To guarantee finding overlapping objects of up to twice the
  // given size, double the height.
  radius_ = ((ymax - ymin) * 2 + grid_->gridsize_ - 1) / grid_->gridsize_;
  rad_index_ = 0;
  CommonStart(x, ymax);
}

// Return the next bbox in the side search or nullptr if the
// edge has been reached. Searches left to right or right to left
// according to the flag.
template<class BBC, class BBC_CLIST, class BBC_C_IT>
BBC* GridSearch<BBC, BBC_CLIST, BBC_C_IT>::NextSideSearch(bool right_to_left) {
  do {
    while (it_.cycled_list()) {
      ++rad_index_;
      if (rad_index_ > radius_) {
        if (right_to_left)
          --x_;
        else
          ++x_;
        rad_index_ = 0;
        if (x_ < 0 || x_ >= grid_->gridwidth_)
          return CommonEnd();
      }
      y_ = y_origin_ - rad_index_;
      if (y_ >= 0 && y_ < grid_->gridheight_)
        SetIterator();
    }
    CommonNext();
  } while (unique_mode_ && returns_.find(previous_return_) != returns_.end());
  if (unique_mode_)
    returns_.insert(previous_return_);
  return previous_return_;
}

// Start a vertical-looking search. Will search up or down
// for a box that horizontally overlaps the given line segment.
template<class BBC, class BBC_CLIST, class BBC_C_IT>
void GridSearch<BBC, BBC_CLIST, BBC_C_IT>::StartVerticalSearch(int xmin,
                                                               int xmax,
                                                               int y) {
  // Right search records the xmin in x_origin_, the y in y_origin_
  // and the size of the horizontal strip to search in radius_.
  radius_ = (xmax - xmin + grid_->gridsize_ - 1) / grid_->gridsize_;
  rad_index_ = 0;
  CommonStart(xmin, y);
}

// Return the next bbox in the vertical search or nullptr if the
// edge has been reached. Searches top to bottom or bottom to top
// according to the flag.
template<class BBC, class BBC_CLIST, class BBC_C_IT>
BBC* GridSearch<BBC, BBC_CLIST, BBC_C_IT>::NextVerticalSearch(
    bool top_to_bottom) {
  do {
    while (it_.cycled_list()) {
      ++rad_index_;
      if (rad_index_ > radius_) {
        if (top_to_bottom)
          --y_;
        else
          ++y_;
        rad_index_ = 0;
        if (y_ < 0 || y_ >= grid_->gridheight_)
          return CommonEnd();
      }
      x_ = x_origin_ + rad_index_;
      if (x_ >= 0 && x_ < grid_->gridwidth_)
        SetIterator();
    }
    CommonNext();
  } while (unique_mode_ && returns_.find(previous_return_) != returns_.end());
  if (unique_mode_)
    returns_.insert(previous_return_);
  return previous_return_;
}

// Start a rectangular search. Will search for a box that overlaps the
// given rectangle.
template<class BBC, class BBC_CLIST, class BBC_C_IT>
void GridSearch<BBC, BBC_CLIST, BBC_C_IT>::StartRectSearch(const TBOX& rect) {
  // Rect search records the xmin in x_origin_, the ymin in y_origin_
  // and the xmax in max_radius_.
  // The search proceeds left to right, top to bottom.
  rect_ = rect;
  CommonStart(rect.left(), rect.top());
  grid_->GridCoords(rect.right(), rect.bottom(),  // - rect.height(),
                    &max_radius_, &y_origin_);
}

// Return the next bbox in the rectangular search or nullptr if complete.
template<class BBC, class BBC_CLIST, class BBC_C_IT>
BBC* GridSearch<BBC, BBC_CLIST, BBC_C_IT>::NextRectSearch() {
  do {
    while (it_.cycled_list()) {
      ++x_;
      if (x_ > max_radius_) {
        --y_;
        x_ = x_origin_;
        if (y_ < y_origin_)
          return CommonEnd();
      }
      SetIterator();
    }
    CommonNext();
  } while (!rect_.overlap(previous_return_->bounding_box()) ||
           (unique_mode_ && returns_.find(previous_return_) != returns_.end()));
  if (unique_mode_)
    returns_.insert(previous_return_);
  return previous_return_;
}

// Remove the last returned BBC. Will not invalidate this. May invalidate
// any other concurrent GridSearch on the same grid. If any others are
// in use, call RepositionIterator on those, to continue without harm.
template<class BBC, class BBC_CLIST, class BBC_C_IT>
void GridSearch<BBC, BBC_CLIST, BBC_C_IT>::RemoveBBox() {
  if (previous_return_ != nullptr) {
    // Remove all instances of previous_return_ from the list, so the iterator
    // remains valid after removal from the rest of the grid cells.
    // if previous_return_ is not on the list, then it has been removed already.
    BBC* prev_data = nullptr;
    BBC* new_previous_return = nullptr;
    it_.move_to_first();
    for (it_.mark_cycle_pt(); !it_.cycled_list();) {
      if (it_.data() ==  previous_return_) {
        new_previous_return = prev_data;
        it_.extract();
        it_.forward();
        next_return_ = it_.cycled_list() ? nullptr : it_.data();
      } else {
        prev_data = it_.data();
        it_.forward();
      }
    }
    grid_->RemoveBBox(previous_return_);
    previous_return_ = new_previous_return;
    RepositionIterator();
  }
}

template<class BBC, class BBC_CLIST, class BBC_C_IT>
void GridSearch<BBC, BBC_CLIST, BBC_C_IT>::RepositionIterator() {
  // Something was deleted, so we have little choice but to clear the
  // returns list.
  returns_.clear();
  // Reset the iterator back to one past the previous return.
  // If the previous_return_ is no longer in the list, then
  // next_return_ serves as a backup.
  it_.move_to_first();
  // Special case, the first element was removed and reposition
  // iterator was called. In this case, the data is fine, but the
  // cycle point is not. Detect it and return.
  if (!it_.empty() && it_.data() == next_return_) {
    it_.mark_cycle_pt();
    return;
  }
  for (it_.mark_cycle_pt(); !it_.cycled_list(); it_.forward()) {
    if (it_.data() == previous_return_ ||
        it_.data_relative(1) == next_return_) {
      CommonNext();
      return;
    }
  }
  // We ran off the end of the list. Move to a new cell next time.
  previous_return_ = nullptr;
  next_return_ = nullptr;
}

// Factored out helper to start a search.
template<class BBC, class BBC_CLIST, class BBC_C_IT>
void GridSearch<BBC, BBC_CLIST, BBC_C_IT>::CommonStart(int x, int y) {
  grid_->GridCoords(x, y, &x_origin_, &y_origin_);
  x_ = x_origin_;
  y_ = y_origin_;
  SetIterator();
  previous_return_ = nullptr;
  next_return_ = it_.empty() ? nullptr : it_.data();
  returns_.clear();
}

// Factored out helper to complete a next search.
template<class BBC, class BBC_CLIST, class BBC_C_IT>
BBC* GridSearch<BBC, BBC_CLIST, BBC_C_IT>::CommonNext() {
  previous_return_ = it_.data();
  it_.forward();
  next_return_ = it_.cycled_list() ? nullptr : it_.data();
  return previous_return_;
}

// Factored out final return when search is exhausted.
template<class BBC, class BBC_CLIST, class BBC_C_IT>
BBC* GridSearch<BBC, BBC_CLIST, BBC_C_IT>::CommonEnd() {
  previous_return_ = nullptr;
  next_return_ = nullptr;
  return nullptr;
}

// Factored out function to set the iterator to the current x_, y_
// grid coords and mark the cycle pt.
template<class BBC, class BBC_CLIST, class BBC_C_IT>
void GridSearch<BBC, BBC_CLIST, BBC_C_IT>::SetIterator() {
  it_= &(grid_->grid_[y_ * grid_->gridwidth_ + x_]);
  it_.mark_cycle_pt();
}

}  // namespace tesseract.

#endif  // TESSERACT_TEXTORD_BBGRID_H_
