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
#include "ocrblock.h"

namespace tesseract {

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

