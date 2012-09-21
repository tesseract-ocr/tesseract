// Copyright 2010 Google Inc. All Rights Reserved.
// Author: rays@google.com (Ray Smith)
///////////////////////////////////////////////////////////////////////
// File:        shapetable.h
// Description: Class to map a classifier shape index to unicharset
//              indices and font indices.
// Author:      Ray Smith
// Created:     Thu Oct 28 17:46:32 PDT 2010
//
// (C) Copyright 2010, Google Inc.
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

#ifndef TESSERACT_CLASSIFY_SHAPETABLE_H_
#define TESSERACT_CLASSIFY_SHAPETABLE_H_

#include "genericvector.h"
#include "intmatcher.h"

class STRING;
class UNICHARSET;

namespace tesseract {

// Simple struct to hold a set of fonts associated with a single unichar-id.
// A vector of UnicharAndFonts makes a shape.
struct UnicharAndFonts {
  UnicharAndFonts() : unichar_id(0) {
  }
  UnicharAndFonts(int uni_id, int font_id) : unichar_id(uni_id) {
    font_ids.push_back(font_id);
  }

  // Writes to the given file. Returns false in case of error.
  bool Serialize(FILE* fp) const;
  // Reads from the given file. Returns false in case of error.
  // If swap is true, assumes a big/little-endian swap is needed.
  bool DeSerialize(bool swap, FILE* fp);

  // Sort function to sort a pair of UnicharAndFonts by unichar_id.
  static int SortByUnicharId(const void* v1, const void* v2);

  GenericVector<inT32> font_ids;
  inT32 unichar_id;
};

// A Shape is a collection of unichar-ids and a list of fonts associated with
// each, organized as a vector of UnicharAndFonts. Conceptually a Shape is
// a classifiable unit, and represents a group of characters or parts of
// characters that have a similar or identical shape. Shapes/ShapeTables may
// be organized hierarchically from identical shapes at the leaves to vaguely
// similar shapes near the root.
class Shape {
 public:
  Shape() : destination_index_(-1) {}

  // Writes to the given file. Returns false in case of error.
  bool Serialize(FILE* fp) const;
  // Reads from the given file. Returns false in case of error.
  // If swap is true, assumes a big/little-endian swap is needed.
  bool DeSerialize(bool swap, FILE* fp);

  int destination_index() const {
    return destination_index_;
  }
  void set_destination_index(int index) {
    destination_index_ = index;
  }
  int size() const {
    return unichars_.size();
  }
  // Returns a UnicharAndFonts entry for the given index, which must be
  // in the range [0, size()).
  const UnicharAndFonts& operator[](int index) const {
    return unichars_[index];
  }
  // Adds a font_id for the given unichar_id. If the unichar_id is not
  // in the shape, it is added.
  void AddToShape(int unichar_id, int font_id);
  // Adds everything in other to this.
  void AddShape(const Shape& other);
  // Returns true if the shape contains the given unichar_id, font_id pair.
  bool ContainsUnicharAndFont(int unichar_id, int font_id) const;
  // Returns true if the shape contains the given unichar_id, ignoring font.
  bool ContainsUnichar(int unichar_id) const;
  // Returns true if the shape contains the given font, ignoring unichar_id.
  bool ContainsFont(int font_id) const;
  // Returns true if this is a subset (including equal) of other.
  bool IsSubsetOf(const Shape& other) const;
  // Returns true if the lists of unichar ids are the same in this and other,
  // ignoring fonts.
  // NOT const, as it will sort the unichars on demand.
  bool IsEqualUnichars(Shape* other);

 private:
  // Sorts the unichars_ vector by unichar.
  void SortUnichars();

  // Flag indicates that the unichars are sorted, allowing faster set
  // operations with another shape.
  bool unichars_sorted_;
  // If this Shape is part of a ShapeTable the destiation_index_ is the index
  // of some other shape in the ShapeTable with which this shape is merged.
  int destination_index_;
  // Array of unichars, each with a set of fonts. Each unichar has at most
  // one entry in the vector.
  GenericVector<UnicharAndFonts> unichars_;
};

// ShapeTable is a class to encapsulate the triple indirection that is
// used here.
// ShapeTable is a vector of shapes.
// Each shape is a vector of UnicharAndFonts representing the set of unichars
// that the shape represents.
// Each UnicharAndFonts also lists the fonts of the unichar_id that were
// mapped to the shape during training.
class ShapeTable {
 public:
  ShapeTable();
  // The UNICHARSET reference supplied here, or in set_unicharset below must
  // exist for the entire life of the ShapeTable. It is used only by DebugStr.
  explicit ShapeTable(const UNICHARSET& unicharset);

  // Writes to the given file. Returns false in case of error.
  bool Serialize(FILE* fp) const;
  // Reads from the given file. Returns false in case of error.
  // If swap is true, assumes a big/little-endian swap is needed.
  bool DeSerialize(bool swap, FILE* fp);

  // Accessors.
  int NumShapes() const {
    return shape_table_.size();
  }
  const UNICHARSET& unicharset() const {
    return *unicharset_;
  }
  // Shapetable takes a pointer to the UNICHARSET, so it must persist for the
  // entire life of the ShapeTable.
  void set_unicharset(const UNICHARSET& unicharset) {
    unicharset_ = &unicharset;
  }
  // Returns a string listing the classes/fonts in a shape.
  STRING DebugStr(int shape_id) const;
  // Returns a debug string summarizing the table.
  STRING SummaryStr() const;

  // Adds a new shape starting with the given unichar_id and font_id.
  // Returns the assigned index.
  int AddShape(int unichar_id, int font_id);
  // Adds a copy of the given shape.
  // Returns the assigned index.
  int AddShape(const Shape& other);
  // Removes the shape given by the shape index. All indices above are changed!
  void DeleteShape(int shape_id);
  // Adds a font_id to the given existing shape index for the given
  // unichar_id. If the unichar_id is not in the shape, it is added.
  void AddToShape(int shape_id, int unichar_id, int font_id);
  // Adds the given shape to the existing shape with the given index.
  void AddShapeToShape(int shape_id, const Shape& other);
  // Returns the id of the shape that contains the given unichar and font.
  // If not found, returns -1.
  // If font_id < 0, the font_id is ignored and the first shape that matches
  // the unichar_id is returned.
  int FindShape(int unichar_id, int font_id) const;
  // Returns the first unichar_id and font_id in the given shape.
  void GetFirstUnicharAndFont(int shape_id,
                              int* unichar_id, int* font_id) const;

  // Accessors for the Shape with the given shape_id.
  const Shape& GetShape(int shape_id) const {
    return *shape_table_[shape_id];
  }
  Shape* MutableShape(int shape_id) {
    return shape_table_[shape_id];
  }

  // Expands all the classes/fonts in the shape individually to build
  // a ShapeTable.
  int BuildFromShape(const Shape& shape, const ShapeTable& master_shapes);

  // Returns true if the shapes are already merged.
  bool AlreadyMerged(int shape_id1, int shape_id2) const;
  // Returns true if any shape contains multiple unichars.
  bool AnyMultipleUnichars() const;
  // Returns the maximum number of unichars over all shapes.
  int MaxNumUnichars() const;
  // Merges shapes with a common unichar over the [start, end) interval.
  // Assumes single unichar per shape.
  void ForceFontMerges(int start, int end);
  // Returns the number of unichars in the master shape.
  int MasterUnicharCount(int shape_id) const;
  // Returns the sum of the font counts in the master shape.
  int MasterFontCount(int shape_id) const;
  // Returns the number of unichars that would result from merging the shapes.
  int MergedUnicharCount(int shape_id1, int shape_id2) const;
  // Merges two shape_ids, leaving shape_id2 marked as merged.
  void MergeShapes(int shape_id1, int shape_id2);
  // Appends the master shapes from other to this.
  // Used to create a clean ShapeTable from a merged one, or to create a
  // copy of a ShapeTable.
  void AppendMasterShapes(const ShapeTable& other);
  // Returns the number of master shapes remaining after merging.
  int NumMasterShapes() const;
  // Returns the destination of this shape, (if merged), taking into account
  // the fact that the destination may itself have been merged.
  // For a non-merged shape, returns the input shape_id.
  int MasterDestinationIndex(int shape_id) const;

 private:
  // Pointer to a provided unicharset used only by the Debugstr member.
  const UNICHARSET* unicharset_;
  // Vector of pointers to the Shapes in this ShapeTable.
  PointerVector<Shape> shape_table_;
};

}  // namespace tesseract.

#endif  // TESSERACT_CLASSIFY_SHAPETABLE_H_
