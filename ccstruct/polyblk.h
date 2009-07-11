/**********************************************************************
 * File:        polyblk.h  (Formerly poly_block.h)
 * Description: Polygonal blocks
 * Author:					Sheelagh Lloyd?
 * Created:
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
#ifndef           POLYBLK_H
#define           POLYBLK_H

#include          "rect.h"
#include          "points.h"
#include          "scrollview.h"
#include          "elst.h"

#include          "hpddef.h"     // must be last (handpd.dll)

// Possible types for a POLY_BLOCK or ColPartition. Must be kept in sync with
// kPBColors. Used extensively by ColPartition, but polyblk is a lower-level
// file.
enum PolyBlockType {
  PT_UNKNOWN,        // Type is not yet known. Keep as the first element.
  PT_FLOWING_TEXT,   // Text that lives inside a column.
  PT_HEADING_TEXT,   // Text that spans more than one column.
  PT_PULLOUT_TEXT,   // Text that is in a cross-column pull-out region.
  PT_TABLE,          // Partition belonging to a table region.
  PT_VERTICAL_TEXT,  // Text-line runs vertically.
  PT_FLOWING_IMAGE,  // Image that lives inside a column.
  PT_HEADING_IMAGE,  // Image that spans more than one column.
  PT_PULLOUT_IMAGE,  // Image that is in a cross-column pull-out region.
  PT_FLOWING_LINE,   // H-Line that lives inside a column.
  PT_HEADING_LINE,   // H-Line that spans more than one column.
  PT_PULLOUT_LINE,   // H-Line that is in a cross-column pull-out region.
  PT_NOISE,          // Lies outside of any column.
  PT_COUNT
};

class DLLSYM POLY_BLOCK {
 public:
  POLY_BLOCK() {
  }
  POLY_BLOCK(ICOORDELT_LIST *points, PolyBlockType type);
  ~POLY_BLOCK () {
  }

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
    return IsTextType(type);
  }

  // Rotate about the origin by the given rotation. (Analogous to
  // multiplying by a complex number.
  void rotate(FCOORD rotation);
  // Move by adding shift to all coordinates.
  void move(ICOORD shift);

  void plot(ScrollView* window, inT32 num);

  void fill(ScrollView* window, ScrollView::Color colour);

  // Returns true if other is inside this.
  bool contains(POLY_BLOCK *other);

  // Returns true if the polygons of other and this overlap.
  bool overlap(POLY_BLOCK *other);

  // Returns the winding number of this around the test_pt.
  // Positive for anticlockwise, negative for clockwise, and zero for
  // test_pt outside this.
  inT16 winding_number(const ICOORD &test_pt);

  // Serialization.
  void prep_serialise() {
    vertices.prep_serialise();
  }
  void dump(FILE *f) {
    vertices.dump(f);
  }
  void de_dump(FILE *f) {
    vertices.de_dump(f);
  }
  make_serialise(POLY_BLOCK)
  void serialise_asc(FILE * f);
  void de_serialise_asc(FILE *f);

  // Static utility functions to handle the PolyBlockType.

  // Returns a color to draw the given type.
  static ScrollView::Color ColorForPolyBlockType(PolyBlockType type);

  // Returns true if PolyBlockType is of horizontal line type
  static bool IsLineType(PolyBlockType type) {
    return (type == PT_FLOWING_LINE) || (type == PT_HEADING_LINE) ||
        (type == PT_PULLOUT_LINE);
  }
  // Returns true if PolyBlockType is of image type
  static bool IsImageType(PolyBlockType type) {
    return (type == PT_FLOWING_IMAGE) || (type == PT_HEADING_IMAGE) ||
           (type == PT_PULLOUT_IMAGE);
  }
  // Returns true if PolyBlockType is of text type
  static bool IsTextType(PolyBlockType type) {
    return (type == PT_FLOWING_TEXT) || (type == PT_HEADING_TEXT) ||
           (type == PT_PULLOUT_TEXT) || (type == PT_TABLE) ||
           (type == PT_VERTICAL_TEXT);
  }

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

  NEWDELETE2(PB_LINE_IT)

  void set_to_block(POLY_BLOCK * blkptr) {
    block = blkptr;
  }

  // Returns a list of runs of pixels for the given y coord.
  // Each element of the returned list is the start (x) and extent(y) of
  // a run inside the region.
  // Delete the returned list after use.
  ICOORDELT_LIST *get_line(inT16 y);

 private:
  POLY_BLOCK * block;
};
#endif
