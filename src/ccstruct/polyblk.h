/**********************************************************************
 * File:        polyblk.h  (Formerly poly_block.h)
 * Description: Polygonal blocks
 *
 * (C) Copyright 1993, Hewlett-Packard Ltd.
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

#ifndef POLYBLK_H
#define POLYBLK_H

#include <tesseract/publictypes.h>
#include "elst.h"
#include "points.h"
#include "rect.h"
#include "scrollview.h"

class DLLSYM POLY_BLOCK {
 public:
  POLY_BLOCK() = default;
  // Initialize from box coordinates.
  POLY_BLOCK(const TBOX& tbox, PolyBlockType type);
  POLY_BLOCK(ICOORDELT_LIST *points, PolyBlockType type);
  ~POLY_BLOCK () = default;

  TBOX *bounding_box() {  // access function
    return &box;
  }

  ICOORDELT_LIST *points() {  // access function
    return &vertices;
  }

  void compute_bb();

  PolyBlockType isA() const {
    return type;
  }

  bool IsText() const {
    return PTIsTextType(type);
  }

  // Rotate about the origin by the given rotation. (Analogous to
  // multiplying by a complex number.
  void rotate(FCOORD rotation);
  // Reflect the coords of the polygon in the y-axis. (Flip the sign of x.)
  void reflect_in_y_axis();
  // Move by adding shift to all coordinates.
  void move(ICOORD shift);

  void plot(ScrollView* window, int32_t num);

  #ifndef GRAPHICS_DISABLED
  void fill(ScrollView* window, ScrollView::Color colour);
  #endif // !GRAPHICS_DISABLED

  // Returns true if other is inside this.
  bool contains(POLY_BLOCK *other);

  // Returns true if the polygons of other and this overlap.
  bool overlap(POLY_BLOCK *other);

  // Returns the winding number of this around the test_pt.
  // Positive for anticlockwise, negative for clockwise, and zero for
  // test_pt outside this.
  int16_t winding_number(const ICOORD &test_pt);

  #ifndef GRAPHICS_DISABLED
  // Static utility functions to handle the PolyBlockType.
  // Returns a color to draw the given type.
  static ScrollView::Color ColorForPolyBlockType(PolyBlockType type);
  #endif // !GRAPHICS_DISABLED

 private:
  ICOORDELT_LIST vertices;     // vertices
  TBOX box;                     // bounding box
  PolyBlockType type;              // Type of this region.
};

// Class to iterate the scanlines of a polygon.
class DLLSYM PB_LINE_IT {
 public:
  PB_LINE_IT(POLY_BLOCK *blkptr) {
    block = blkptr;
  }

  void set_to_block(POLY_BLOCK * blkptr) {
    block = blkptr;
  }

  // Returns a list of runs of pixels for the given y coord.
  // Each element of the returned list is the start (x) and extent(y) of
  // a run inside the region.
  // Delete the returned list after use.
  ICOORDELT_LIST *get_line(int16_t y);

 private:
  POLY_BLOCK * block;
};
#endif
