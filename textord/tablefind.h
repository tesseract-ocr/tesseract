///////////////////////////////////////////////////////////////////////
// File:        tablefind.h
// Description: Helper classes to find tables from ColPartitions.
// Author:      Faisal Shafait (faisal.shafait@dfki.de)
// Created:     Tue Jan 06 11:13:01 PST 2009
//
// (C) Copyright 2009, Google Inc.
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

#ifndef TESSERACT_TEXTORD_TABLEFIND_H__
#define TESSERACT_TEXTORD_TABLEFIND_H__

#include "rect.h"
#include "elst.h"

namespace tesseract {

// Possible types for a column segment.
enum ColSegType {
  COL_UNKNOWN,
  COL_TEXT,
  COL_TABLE,
  COL_MIXED,
  COL_COUNT
};

// ColSegment holds rectangular blocks that represent segmentation of a page
// into regions containing single column text/table.
class ColSegment;
ELISTIZEH(ColSegment)
CLISTIZEH(ColSegment)

class ColSegment : public ELIST_LINK {
 public:
  ColSegment() : num_table_cells_(0), num_text_cells_(0),
                 type_(COL_UNKNOWN) {
  }
  ~ColSegment() { }

  // Simple accessors and mutators
  const TBOX& bounding_box() const {
    return bounding_box_;
  }

  void set_top(int y) {
    bounding_box_.set_top(y);
  }

  void set_bottom(int y) {
    bounding_box_.set_bottom(y);
  }

  void set_left(int x) {
    bounding_box_.set_left(x);
  }

  void set_right(int x) {
    bounding_box_.set_right(x);
  }

  void set_bounding_box(const TBOX& other) {
    bounding_box_ = other;
  }

  int get_num_table_cells() {
    return num_table_cells_;
  }

  // set the number of table colpartitions covered by the bounding_box_
  void set_num_table_cells(int n) {
    num_table_cells_ = n;
  }

  int get_num_text_cells() {
    return num_text_cells_;
  }

  // set the number of text colpartitions covered by the bounding_box_
  void set_num_text_cells(int n) {
    num_text_cells_ = n;
  }

  ColSegType type() {
    return type_;
  }

  // set the type of the block based on the ratio of table to text
  // colpartitions covered by it.
  void set_type();

  // Provides a color for BBGrid to draw the rectangle.
  ScrollView::Color  BoxColor() const;

  // Insert a rectangle into bounding_box_
  void InsertBox(const TBOX& other);

 private:
  // Initializes the bulk of the members to default values.
  void Init() {
  }

  TBOX bounding_box_;                    // bounding box
  int num_table_cells_;
  int num_text_cells_;
  ColSegType type_;
};

// Typedef BBGrid of ColSegments
typedef BBGrid<ColSegment,
               ColSegment_CLIST,
               ColSegment_C_IT> ColSegmentGrid;


}  // namespace tesseract.

#endif  // TESSERACT_TEXTORD_TABLEFIND_H__
