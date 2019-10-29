// Copyright 2010 Google Inc. All Rights Reserved.
// Author: rays@google.com (Ray Smith)
///////////////////////////////////////////////////////////////////////
// File:        shapetable.h
// Description: Class to map a classifier shape index to unicharset
//              indices and font indices.
// Author:      Ray Smith
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

#include "bitvector.h"
#include "fontinfo.h"
#include "genericheap.h"
#include <tesseract/genericvector.h>
#include "intmatcher.h"

class STRING;
class UNICHARSET;

namespace tesseract {

class ShapeTable;

// Simple struct to hold a single classifier unichar selection, a corresponding
// rating, and a list of appropriate fonts.
struct UnicharRating {
  UnicharRating()
    : unichar_id(0), rating(0.0f), adapted(false), config(0),
      feature_misses(0) {}
  UnicharRating(int u, float r)
    : unichar_id(u), rating(r), adapted(false), config(0), feature_misses(0) {}

  // Print debug info.
  void Print() const {
    tprintf("Unichar-id=%d, rating=%g, adapted=%d, config=%d, misses=%d,"
            " %d fonts\n", unichar_id, rating, adapted, config, feature_misses,
            fonts.size());
  }

  // Sort function to sort ratings appropriately by descending rating.
  static int SortDescendingRating(const void* t1, const void* t2) {
    const auto* a = static_cast<const UnicharRating*>(t1);
    const auto* b = static_cast<const UnicharRating*>(t2);
    if (a->rating > b->rating) {
      return -1;
    } else if (a->rating < b->rating) {
      return 1;
    } else {
      return a->unichar_id - b->unichar_id;
    }
  }
  // Helper function to get the index of the first result with the required
  // unichar_id. If the results are sorted by rating, this will also be the
  // best result with the required unichar_id.
  // Returns -1 if the unichar_id is not found
  static int FirstResultWithUnichar(const GenericVector<UnicharRating>& results,
                                    UNICHAR_ID unichar_id);

  // Index into some UNICHARSET table indicates the class of the answer.
  UNICHAR_ID unichar_id;
  // Rating from classifier with 1.0 perfect and 0.0 impossible.
  // Call it a probability if you must.
  float rating;
  // True if this result is from the adaptive classifier.
  bool adapted;
  // Index of best matching font configuration of result.
  uint8_t config;
  // Number of features that were total misses - were liked by no classes.
  uint16_t feature_misses;
  // Unsorted collection of fontinfo ids and scores. Note that a raw result
  // from the IntegerMatch will contain config ids, that require transforming
  // to fontinfo ids via fontsets and (possibly) shapetable.
  GenericVector<ScoredFont> fonts;
};

// Classifier result from a low-level classification is an index into some
// ShapeTable and a rating.
struct ShapeRating {
  ShapeRating()
    : shape_id(0), rating(0.0f), raw(0.0f), font(0.0f),
      joined(false), broken(false) {}
  ShapeRating(int s, float r)
    : shape_id(s), rating(r), raw(1.0f), font(0.0f),
      joined(false), broken(false) {}

  // Sort function to sort ratings appropriately by descending rating.
  static int SortDescendingRating(const void* t1, const void* t2) {
    const auto* a = static_cast<const ShapeRating*>(t1);
    const auto* b = static_cast<const ShapeRating*>(t2);
    if (a->rating > b->rating) {
      return -1;
    } else if (a->rating < b->rating) {
      return 1;
    } else {
      return a->shape_id - b->shape_id;
    }
  }
  // Helper function to get the index of the first result with the required
  // unichar_id. If the results are sorted by rating, this will also be the
  // best result with the required unichar_id.
  // Returns -1 if the unichar_id is not found
  static int FirstResultWithUnichar(const GenericVector<ShapeRating>& results,
                                    const ShapeTable& shape_table,
                                    UNICHAR_ID unichar_id);

  // Index into some shape table indicates the class of the answer.
  int shape_id;
  // Rating from classifier with 1.0 perfect and 0.0 impossible.
  // Call it a probability if you must.
  float rating;
  // Subsidiary rating that a classifier may use internally.
  float raw;
  // Subsidiary rating that a classifier may use internally.
  float font;
  // Flag indicating that the input may be joined.
  bool joined;
  // Flag indicating that the input may be broken (a fragment).
  bool broken;
};

// Simple struct to hold an entry for a heap-based priority queue of
// ShapeRating.
struct ShapeQueueEntry {
  ShapeQueueEntry() : result(ShapeRating(0, 0.0f)), level(0) {}
  ShapeQueueEntry(const ShapeRating& rating, int level0)
    : result(rating), level(level0) {}

  // Sort by decreasing rating and decreasing level for equal rating.
  bool operator<(const ShapeQueueEntry& other) const {
    if (result.rating > other.result.rating) return true;
    if (result.rating == other.result.rating)
      return level > other.level;
    return false;
  }

  // Output from classifier.
  ShapeRating result;
  // Which level in the tree did this come from?
  int level;
};
using ShapeQueue = GenericHeap<ShapeQueueEntry>;

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
  bool DeSerialize(TFile* fp);

  // Sort function to sort a pair of UnicharAndFonts by unichar_id.
  static int SortByUnicharId(const void* v1, const void* v2);

  GenericVector<int32_t> font_ids;
  int32_t unichar_id;
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
  bool DeSerialize(TFile* fp);

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
  // Sets the unichar_id of the given index to the new unichar_id.
  void SetUnicharId(int index, int unichar_id) {
    unichars_[index].unichar_id = unichar_id;
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
  // Returns true if the shape contains the given font properties, ignoring
  // unichar_id.
  bool ContainsFontProperties(const FontInfoTable& font_table,
                              uint32_t properties) const;
  // Returns true if the shape contains multiple different font properties,
  // ignoring unichar_id.
  bool ContainsMultipleFontProperties(const FontInfoTable& font_table) const;
  // Returns true if this shape is equal to other (ignoring order of unichars
  // and fonts).
  bool operator==(const Shape& other) const;
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
  bool unichars_sorted_ = false;
  // If this Shape is part of a ShapeTable the destiation_index_ is the index
  // of some other shape in the ShapeTable with which this shape is merged.
  int destination_index_ = 0;
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
  bool DeSerialize(TFile* fp);

  // Accessors.
  int NumShapes() const {
    return shape_table_.size();
  }
  const UNICHARSET& unicharset() const {
    return *unicharset_;
  }
  // Returns the number of fonts used in this ShapeTable, computing it if
  // necessary.
  int NumFonts() const;
  // Shapetable takes a pointer to the UNICHARSET, so it must persist for the
  // entire life of the ShapeTable.
  void set_unicharset(const UNICHARSET& unicharset) {
    unicharset_ = &unicharset;
  }
  // Re-indexes the class_ids in the shapetable according to the given map.
  // Useful in conjunction with set_unicharset.
  void ReMapClassIds(const GenericVector<int>& unicharset_map);
  // Returns a string listing the classes/fonts in a shape.
  STRING DebugStr(int shape_id) const;
  // Returns a debug string summarizing the table.
  STRING SummaryStr() const;

  // Adds a new shape starting with the given unichar_id and font_id.
  // Returns the assigned index.
  int AddShape(int unichar_id, int font_id);
  // Adds a copy of the given shape unless it is already present.
  // Returns the assigned index or index of existing shape if already present.
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
  // Swaps two shape_ids.
  void SwapShapes(int shape_id1, int shape_id2);
  // Appends the master shapes from other to this.
  // Used to create a clean ShapeTable from a merged one, or to create a
  // copy of a ShapeTable.
  // If not nullptr, shape_map is set to map other shape_ids to this's shape_ids.
  void AppendMasterShapes(const ShapeTable& other,
                          GenericVector<int>* shape_map);
  // Returns the number of master shapes remaining after merging.
  int NumMasterShapes() const;
  // Returns the destination of this shape, (if merged), taking into account
  // the fact that the destination may itself have been merged.
  // For a non-merged shape, returns the input shape_id.
  int MasterDestinationIndex(int shape_id) const;

  // Returns false if the unichars in neither shape is a subset of the other..
  bool SubsetUnichar(int shape_id1, int shape_id2) const;
  // Returns false if the unichars in neither shape is a subset of the other..
  bool MergeSubsetUnichar(int merge_id1, int merge_id2, int shape_id) const;
  // Returns true if the unichar sets are equal between the shapes.
  bool EqualUnichars(int shape_id1, int shape_id2) const;
  bool MergeEqualUnichars(int merge_id1, int merge_id2, int shape_id) const;
  // Returns true if there is a common unichar between the shapes.
  bool CommonUnichars(int shape_id1, int shape_id2) const;
  // Returns true if there is a common font id between the shapes.
  bool CommonFont(int shape_id1, int shape_id2) const;

  // Adds the unichars of the given shape_id to the vector of results. Any
  // unichar_id that is already present just has the fonts added to the
  // font set for that result without adding a new entry in the vector.
  // NOTE: it is assumed that the results are given to this function in order
  // of decreasing rating.
  // The unichar_map vector indicates the index of the results entry containing
  // each unichar, or -1 if the unichar is not yet included in results.
  void AddShapeToResults(const ShapeRating& shape_rating,
                         GenericVector<int>* unichar_map,
                         GenericVector<UnicharRating>* results) const;

 private:
  // Adds the given unichar_id to the results if needed, updating unichar_map
  // and returning the index of unichar in results.
  int AddUnicharToResults(int unichar_id, float rating,
                          GenericVector<int>* unichar_map,
                          GenericVector<UnicharRating>* results) const;

  // Pointer to a provided unicharset used only by the Debugstr member.
  const UNICHARSET* unicharset_;
  // Vector of pointers to the Shapes in this ShapeTable.
  PointerVector<Shape> shape_table_;

  // Cached data calculated on demand.
  mutable int num_fonts_;
};

}  // namespace tesseract.

#endif  // TESSERACT_CLASSIFY_SHAPETABLE_H_
