///////////////////////////////////////////////////////////////////////
// File:        bbgrid.cpp
// Description: Class to hold BLOBNBOXs in a grid for fast access
//              to neighbours.
// Author:      Ray Smith
// Created:     Wed Jun 06 17:22:01 PDT 2007
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

#include "bbgrid.h"
#include "helpers.h"
#include "ocrblock.h"

namespace tesseract {

///////////////////////////////////////////////////////////////////////
// BBGrid IMPLEMENTATION.
///////////////////////////////////////////////////////////////////////
GridBase::GridBase() {
}

GridBase::GridBase(int gridsize, const ICOORD& bleft, const ICOORD& tright) {
  Init(gridsize, bleft, tright);
}

GridBase::~GridBase() {
}

// (Re)Initialize the grid. The gridsize is the size in pixels of each cell,
// and bleft, tright are the bounding box of everything to go in it.
void GridBase::Init(int gridsize, const ICOORD& bleft, const ICOORD& tright) {
  gridsize_ = gridsize;
  bleft_ = bleft;
  tright_ = tright;
  if (gridsize_ == 0)
    gridsize_ = 1;
  gridwidth_ = (tright.x() - bleft.x() + gridsize_ - 1) / gridsize_;
  gridheight_ = (tright.y() - bleft.y() + gridsize_ - 1) / gridsize_;
  gridbuckets_ = gridwidth_ * gridheight_;
}

// Compute the given grid coordinates from image coords.
void GridBase::GridCoords(int x, int y, int* grid_x, int* grid_y) const {
  *grid_x = (x - bleft_.x()) / gridsize_;
  *grid_y = (y - bleft_.y()) / gridsize_;
  ClipGridCoords(grid_x, grid_y);
}

// Clip the given grid coordinates to fit within the grid.
void GridBase::ClipGridCoords(int* x, int* y) const {
  *x = ClipToRange(*x, 0, gridwidth_ - 1);
  *y = ClipToRange(*y, 0, gridheight_ - 1);
}

IntGrid::IntGrid() {
  grid_ = NULL;
}

IntGrid::IntGrid(int gridsize, const ICOORD& bleft, const ICOORD& tright)
  : grid_(NULL) {
  Init(gridsize, bleft, tright);
}

IntGrid::~IntGrid() {
  if (grid_ != NULL)
    delete [] grid_;
}

// (Re)Initialize the grid. The gridsize is the size in pixels of each cell,
// and bleft, tright are the bounding box of everything to go in it.
void IntGrid::Init(int gridsize, const ICOORD& bleft, const ICOORD& tright) {
  GridBase::Init(gridsize, bleft, tright);
  if (grid_ != NULL)
    delete [] grid_;
  grid_ = new int[gridbuckets_];
  Clear();
}

// Clear all the ints in the grid to zero.
void IntGrid::Clear() {
  for (int i = 0; i < gridbuckets_; ++i) {
    grid_[i] = 0;
  }
}

// Rotate the grid by rotation, keeping cell contents.
// rotation must be a multiple of 90 degrees.
// NOTE: due to partial cells, cell coverage in the rotated grid will be
// inexact. This is why there is no Rotate for the generic BBGrid.
// TODO(rays) investigate fixing this inaccuracy by moving the origin after
// rotation.
void IntGrid::Rotate(const FCOORD& rotation) {
  ASSERT_HOST(rotation.x() == 0.0f || rotation.y() == 0.0f);
  ICOORD old_bleft(bleft());
  ICOORD old_tright(tright());
  int old_width = gridwidth();
  int old_height = gridheight();
  TBOX box(bleft(), tright());
  box.rotate(rotation);
  int* old_grid = grid_;
  grid_ = NULL;
  Init(gridsize(), box.botleft(), box.topright());
  // Iterate over the old grid, copying data to the rotated position in the new.
  int oldi = 0;
  FCOORD x_step(rotation);
  x_step *= gridsize();
  for (int oldy = 0; oldy < old_height; ++oldy) {
    FCOORD line_pos(old_bleft.x(), old_bleft.y() + gridsize() * oldy);
    line_pos.rotate(rotation);
    for (int oldx = 0; oldx < old_width; ++oldx, line_pos += x_step, ++oldi) {
      int grid_x, grid_y;
      GridCoords(static_cast<int>(line_pos.x() + 0.5),
                 static_cast<int>(line_pos.y() + 0.5),
                 &grid_x, &grid_y);
      grid_[grid_y * gridwidth() + grid_x] = old_grid[oldi];
    }
  }
  delete [] old_grid;
}

// Returns a new IntGrid containing values equal to the sum of all the
// neighbouring cells. The returned grid must be deleted after use.
// For ease of implementation, edge cells are double counted, to make them
// have the same range as the non-edge cells.
IntGrid* IntGrid::NeighbourhoodSum() const {
  IntGrid* sumgrid = new IntGrid(gridsize(), bleft(), tright());
  for (int y = 0; y < gridheight(); ++y) {
    for (int x = 0; x < gridwidth(); ++x) {
      int cell_count = 0;
      for (int yoffset = -1; yoffset <= 1; ++yoffset) {
        for (int xoffset = -1; xoffset <= 1; ++xoffset) {
          int grid_x = x + xoffset;
          int grid_y = y + yoffset;
          ClipGridCoords(&grid_x, &grid_y);
          cell_count += GridCellValue(grid_x, grid_y);
        }
      }
      if (GridCellValue(x, y) > 1)
        sumgrid->SetGridCell(x, y, cell_count);
    }
  }
  return sumgrid;
}


#ifdef HAVE_LIBLEPT
// Make a Pix of the correct scaled size for the TraceOutline functions.
Pix* GridReducedPix(const TBOX& box, int gridsize,
                    ICOORD bleft, int* left, int* bottom) {
  // Compute grid bounds of the outline and pad all round by 1.
  int grid_left = (box.left() - bleft.x()) / gridsize - 1;
  int grid_bottom = (box.bottom() - bleft.y()) / gridsize - 1;
  int grid_right = (box.right() - bleft.x()) / gridsize + 1;
  int grid_top = (box.top() - bleft.y()) / gridsize + 1;
  *left = grid_left;
  *bottom = grid_bottom;
  return pixCreate(grid_right - grid_left + 1,
                   grid_top - grid_bottom + 1,
                   1);
}

// Helper function to return a scaled Pix with one pixel per grid cell,
// set (black) where the given outline enters the corresponding grid cell,
// and clear where the outline does not touch the grid cell.
// Also returns the grid coords of the bottom-left of the Pix, in *left
// and *bottom, which corresponds to (0, 0) on the Pix.
// Note that the Pix is used upside-down, with (0, 0) being the bottom-left.
Pix* TraceOutlineOnReducedPix(C_OUTLINE* outline, int gridsize,
                              ICOORD bleft, int* left, int* bottom) {
  TBOX box = outline->bounding_box();
  Pix* pix = GridReducedPix(box, gridsize, bleft, left, bottom);
  int wpl = pixGetWpl(pix);
  l_uint32* data = pixGetData(pix);
  int length = outline->pathlength();
  ICOORD pos = outline->start_pos();
  for (int i = 0; i < length; ++i) {
    int grid_x = (pos.x() - bleft.x()) / gridsize - *left;
    int grid_y = (pos.y() - bleft.y()) / gridsize - *bottom;
    SET_DATA_BIT(data + grid_y * wpl, grid_x);
    pos += outline->step(i);
  }
  return pix;
}
#if 0  // Example code shows how to use TraceOutlineOnReducedPix.
  C_OUTLINE_IT ol_it(blob->cblob()->out_list());
  int grid_left, grid_bottom;
  Pix* pix = TraceOutlineOnReducedPix(ol_it.data(), gridsize_, bleft_,
                                      &grid_left, &grid_bottom);
  grid->InsertPixPtBBox(grid_left, grid_bottom, pix, blob);
  pixDestroy(&pix);
#endif

// As TraceOutlineOnReducedPix above, but on a BLOCK instead of a C_OUTLINE.
Pix* TraceBlockOnReducedPix(BLOCK* block, int gridsize,
                            ICOORD bleft, int* left, int* bottom) {
  TBOX box = block->bounding_box();
  Pix* pix = GridReducedPix(box, gridsize, bleft, left, bottom);
  int wpl = pixGetWpl(pix);
  l_uint32* data = pixGetData(pix);
  ICOORDELT_IT it(block->poly_block()->points());
  for (it.mark_cycle_pt(); !it.cycled_list();) {
    ICOORD pos = *it.data();
    it.forward();
    ICOORD next_pos = *it.data();
    ICOORD line_vector = next_pos - pos;
    int major, minor;
    ICOORD major_step, minor_step;
    line_vector.setup_render(&major_step, &minor_step, &major, &minor);
    int accumulator = major / 2;
    while (pos != next_pos) {
      int grid_x = (pos.x() - bleft.x()) / gridsize - *left;
      int grid_y = (pos.y() - bleft.y()) / gridsize - *bottom;
      SET_DATA_BIT(data + grid_y * wpl, grid_x);
      pos += major_step;
      accumulator += minor;
      if (accumulator >= major) {
        accumulator -= major;
        pos += minor_step;
      }
    }
  }
  return pix;
}
#endif

}  // namespace tesseract.

