///////////////////////////////////////////////////////////////////////
// File:        colpartitionset.h
// Description: Class to hold a list of ColPartitions of the page that
//              correspond roughly to columns.
// Author:      Ray Smith
// Created:     Thu Aug 14 10:50:01 PDT 2008
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

#ifndef TESSERACT_TEXTORD_COLPARTITIONSET_H__
#define TESSERACT_TEXTORD_COLPARTITIONSET_H__

#include "colpartition.h"   // For ColPartition_LIST.
#include "genericvector.h"  // For GenericVector.
#include "rect.h"           // For TBOX.
#include "tabvector.h"      // For BLOBNBOX_CLIST.

namespace tesseract {

class WorkingPartSet_LIST;
class ColSegment_LIST;
class ColPartitionSet;
typedef GenericVector<ColPartitionSet*> PartSetVector;

// ColPartitionSet is a class that holds a list of ColPartitions.
// Its main use is in holding a candidate partitioning of the width of the
// image into columns, where each member ColPartition is a single column.
// ColPartitionSets are used in building the column layout of a page.
class ColPartitionSet : public ELIST_LINK {
 public:
  ColPartitionSet() {
  }
  explicit ColPartitionSet(ColPartition_LIST* partitions);
  explicit ColPartitionSet(ColPartition* partition);

  ~ColPartitionSet();

  // Simple accessors.
  const TBOX& bounding_box() const {
    return bounding_box_;
  }
  bool Empty() {
    return parts_.empty();
  }
  int ColumnCount() {
    return parts_.length();
  }

  // Return an element of the parts_ list from its index.
  ColPartition* GetColumnByIndex(int index);

  // Return the ColPartition that contains the given coords, if any, else NULL.
  ColPartition* ColumnContaining(int x, int y);

  // Return the bounding boxes of columns at the given y-range
  void GetColumnBoxes(int y_bottom, int y_top, ColSegment_LIST *segments);

  // Move the parts to the output list, giving up ownership.
  void ReturnParts(ColPartition_LIST* parts);

  // Merge any significantly overlapping partitions within the this and other,
  // and unique the boxes so that no two partitions use the same box.
  // Return true if any changes were made to either set.
  bool MergeOverlaps(ColPartitionSet* other, WidthCallback* cb);

  // Attempt to improve this by adding partitions or expanding partitions.
  void ImproveColumnCandidate(WidthCallback* cb, PartSetVector* src_sets);

  // If this set is good enough to represent a new partitioning into columns,
  // add it to the vector of sets, otherwise delete it.
  void AddToColumnSetsIfUnique(PartSetVector* column_sets, WidthCallback* cb);

  // Return true if the partitions in other are all compatible with the columns
  // in this.
  bool CompatibleColumns(bool debug, ColPartitionSet* other, WidthCallback* cb);

  // Returns the total width of all blobs in the part_set that do not lie
  // within an approved column. Used as a cost measure for using this
  // column set over another that might be compatible.
  int UnmatchedWidth(ColPartitionSet* part_set);

  // Return true if this ColPartitionSet makes a legal column candidate by
  // having legal individual partitions and non-overlapping adjacent pairs.
  bool LegalColumnCandidate();

  // Return a copy of this. If good_only will only copy the Good ColPartitions.
  ColPartitionSet* Copy(bool good_only);

  // Display the edges of the columns at the given y coords.
  void DisplayColumnEdges(int y_bottom, int y_top, ScrollView* win);

  // Return the ColumnSpanningType that best explains the columns overlapped
  // by the given coords(left,right,y), with the given margins.
  // Also return the first and last column index touched by the coords and
  // the leftmost spanned column.
  // Column indices are 2n + 1 for real colums (0 based) and even values
  // represent the gaps in between columns, with 0 being left of the leftmost.
  // resolution refers to the ppi resolution of the image. It may be 0 if only
  // the first_col and last_col are required.
  ColumnSpanningType SpanningType(int resolution,
                                  int left, int right, int y,
                                  int left_margin, int right_margin,
                                  int* first_col, int* last_col,
                                  int* first_spanned_col);

  // The column_set has changed. Close down all in-progress WorkingPartSets in
  // columns that do not match and start new ones for the new columns in this.
  // As ColPartitions are turned into BLOCKs, the used ones are put in
  // used_parts, as they still need to be referenced in the grid.
  void ChangeWorkColumns(const ICOORD& bleft, const ICOORD& tright,
                         int resolution, ColPartition_LIST* used_parts,
                         WorkingPartSet_LIST* working_set);

  // Accumulate the widths and gaps into the given variables.
  void AccumulateColumnWidthsAndGaps(int* total_width, int* width_samples,
                                     int* total_gap, int* gap_samples);

  // Provide debug output for this ColPartitionSet and all the ColPartitions.
  void Print();

 private:
  // Add the given partition to the list in the appropriate place.
  void AddPartition(ColPartition* new_part, ColPartition_IT* it);

  // Compute the coverage and good column count.
  void ComputeCoverage();

  // The partitions in this column candidate.
  ColPartition_LIST parts_;
  // The number of partitions that have a frequent column width.
  int good_column_count_;
  // Total width of all the ColPartitions.
  int total_coverage_;
  // Bounding box of all partitions in the set.
  TBOX bounding_box_;
};

ELISTIZEH(ColPartitionSet)

}  // namespace tesseract.

#endif  // TESSERACT_TEXTORD_COLPARTITION_H__
