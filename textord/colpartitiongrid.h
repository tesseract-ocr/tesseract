///////////////////////////////////////////////////////////////////////
// File:        colpartitionrid.h
// Description: Class collecting code that acts on a BBGrid of ColPartitions.
// Author:      Ray Smith
// Created:     Mon Oct 05 08:42:01 PDT 2009
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

#ifndef TESSERACT_TEXTORD_COLPARTITIONGRID_H__
#define TESSERACT_TEXTORD_COLPARTITIONGRID_H__

#include "bbgrid.h"
#include "colpartition.h"

namespace tesseract {

class TabFind;

// ColPartitionGrid is a BBGrid of ColPartition.
// It collects functions that work on the grid.
class ColPartitionGrid : public BBGrid<ColPartition,
                                       ColPartition_CLIST,
                                       ColPartition_C_IT> {
 public:
  ColPartitionGrid();
  ColPartitionGrid(int gridsize, const ICOORD& bleft, const ICOORD& tright);

  ~ColPartitionGrid();

  // Handles a click event in a display window.
  void HandleClick(int x, int y);

  // Finds all the ColPartitions in the grid that overlap with the given
  // box and returns them SortByBoxLeft(ed) and uniqued in the given list.
  // Any partition equal to not_this (may be NULL) is excluded.
  void FindOverlappingPartitions(const TBOX& box, const ColPartition* not_this,
                                 ColPartition_CLIST* parts);

  // Finds and returns the best candidate ColPartition to merge with part,
  // selected from the candidates list, based on the minimum increase in
  // pairwise overlap among all the partitions overlapped by the combined box.
  // If overlap_increase is not NULL then it returns the increase in overlap
  // that would result from the merge.
  // See colpartitiongrid.cpp for a diagram.
  ColPartition* BestMergeCandidate(
      const ColPartition* part, ColPartition_CLIST* candidates, bool debug,
      TessResultCallback2<bool, const ColPartition*,
                          const ColPartition*>* confirm_cb,
      int* overlap_increase);

  // Improves the margins of the ColPartitions in the grid by calling
  // FindPartitionMargins on each.
  void GridFindMargins(ColPartitionSet** best_columns);

  // Improves the margins of the ColPartitions in the list by calling
  // FindPartitionMargins on each.
  void ListFindMargins(ColPartitionSet** best_columns,
                       ColPartition_LIST* parts);

  // Finds and marks text partitions that represent figure captions.
  void FindFigureCaptions();

  //////// Functions that manipulate ColPartitions in the grid     ///////
  //////// to find chains of partner partitions of the same type.  ///////
  // For every ColPartition in the grid, finds its upper and lower neighbours.
  void FindPartitionPartners();
  // Finds the best partner in the given direction for the given partition.
  // Stores the result with AddPartner.
  void FindPartitionPartners(bool upper, ColPartition* part);
  // For every ColPartition with multiple partners in the grid, reduces the
  // number of partners to 0 or 1. If get_desperate is true, goes to more
  // desperate merge methods to merge flowing text before breaking partnerships.
  void RefinePartitionPartners(bool get_desperate);

 private:
  // Improves the margins of the ColPartition by searching for
  // neighbours that vertically overlap significantly.
  void FindPartitionMargins(ColPartitionSet* columns, ColPartition* part);

  // Starting at x, and going in the specified direction, upto x_limit, finds
  // the margin for the given y range by searching sideways,
  // and ignoring not_this.
  int FindMargin(int x, bool right_to_left, int x_limit,
                 int y_bottom, int y_top, const ColPartition* not_this);
};

}  // namespace tesseract.

#endif  // TESSERACT_TEXTORD_COLPARTITIONGRID_H__
