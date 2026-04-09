///////////////////////////////////////////////////////////////////////
// File:        workingpartset.h
// Description: Class to hold a working set of partitions of the page
//              during construction of text/image regions.
// Author:      Ray Smith
// Created:     Tue Ocr 28 17:21:01 PDT 2008
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

#ifndef TESSERACT_TEXTORD_WORKINGPARSET_H_
#define TESSERACT_TEXTORD_WORKINGPARSET_H_

#include "blobbox.h"      // For TO_BLOCK_LIST and BLOCK_LIST.
#include "colpartition.h" // For ColPartition_LIST.

namespace tesseract {

// WorkingPartSet holds a working set of ColPartitions during transformation
// from the grid-based storage to regions in logical reading order, and is
// therefore only used during construction of the regions.
class WorkingPartSet : public ELIST<WorkingPartSet>::LINK {
public:
  explicit WorkingPartSet(ColPartition *column)
      : column_(column), latest_part_(nullptr), part_it_(&part_set_) {}

  // Simple accessors.
  ColPartition *column() const {
    return column_;
  }
  void set_column(ColPartition *col) {
    column_ = col;
  }

  // Add the partition to this WorkingPartSet. Partitions are generally
  // stored in the order in which they are received, but if the partition
  // has a SingletonPartner, make sure that it stays with its partner.
  void AddPartition(ColPartition *part);

  // Make blocks out of any partitions in this WorkingPartSet, and append
  // them to the end of the blocks list. bleft, tright and resolution give
  // the bounds and resolution of the source image, so that blocks can be
  // made to fit in the bounds.
  // All ColPartitions go in the used_parts list, as they need to be kept
  // around, but are no longer needed.
  void ExtractCompletedBlocks(const ICOORD &bleft, const ICOORD &tright, int resolution,
                              ColPartition_LIST *used_parts, BLOCK_LIST *blocks,
                              TO_BLOCK_LIST *to_blocks);

  // Insert the given blocks at the front of the completed_blocks_ list so
  // they can be kept in the correct reading order.
  void InsertCompletedBlocks(BLOCK_LIST *blocks, TO_BLOCK_LIST *to_blocks);

private:
  // Convert the part_set_ into blocks, starting a new block at a break
  // in partnerships, or a change in linespacing (for text).
  void MakeBlocks(const ICOORD &bleft, const ICOORD &tright, int resolution,
                  ColPartition_LIST *used_parts);

  // The column that this working set applies to. Used by the caller.
  ColPartition *column_;
  // The most recently added partition.
  ColPartition *latest_part_;
  // All the partitions in the block that is currently under construction.
  ColPartition_LIST part_set_;
  // Iteratorn on part_set_ pointing to the most recent addition.
  ColPartition_IT part_it_;
  // The blocks that have been made so far and belong before the current block.
  BLOCK_LIST completed_blocks_;
  TO_BLOCK_LIST to_blocks_;
};

ELISTIZEH(WorkingPartSet)

} // namespace tesseract.

#endif // TESSERACT_TEXTORD_WORKINGPARSET_H_
