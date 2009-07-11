///////////////////////////////////////////////////////////////////////
// File:        workingpartset.cpp
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

#include "workingpartset.h"
#include "colpartition.h"

namespace tesseract {

ELISTIZE(WorkingPartSet)

// Add the partition to this WorkingPartSet. Unrelated partitions are
// stored in the order in which they are received, but if the partition
// has a SingletonPartner, make sure that it stays with its partner.
void WorkingPartSet::AddPartition(ColPartition* part) {
  ColPartition* partner = part->SingletonPartner(true);
  if (partner != NULL) {
    ASSERT_HOST(partner->SingletonPartner(false) == part);
  }
  if (latest_part_ == NULL || partner == NULL) {
    // This partition goes at the end of the list
    part_it_.move_to_last();
  } else if (latest_part_->SingletonPartner(false) != part) {
    // Reposition the iterator to the correct partner, or at the end.
    for (part_it_.move_to_first(); !part_it_.at_last() &&
         part_it_.data() != partner;
         part_it_.forward());
  }
  part_it_.add_after_then_move(part);
  latest_part_ = part;
}

// Make blocks out of any partitions in this WorkingPartSet, and append
// them to the end of the blocks list. bleft, tright and resolution give
// the bounds and resolution of the source image, so that blocks can be
// made to fit in the bounds.
// All ColPartitions go in the used_parts list, as they need to be kept
// around, but are no longer needed.
void WorkingPartSet::ExtractCompletedBlocks(const ICOORD& bleft,
                                            const ICOORD& tright,
                                            int resolution,
                                            ColPartition_LIST* used_parts,
                                            BLOCK_LIST* blocks,
                                            TO_BLOCK_LIST* to_blocks) {
  MakeBlocks(bleft, tright, resolution, used_parts);
  BLOCK_IT block_it(blocks);
  block_it.move_to_last();
  block_it.add_list_after(&completed_blocks_);
  TO_BLOCK_IT to_block_it(to_blocks);
  to_block_it.move_to_last();
  to_block_it.add_list_after(&to_blocks_);
}

// Insert the given blocks at the front of the completed_blocks_ list so
// they can be kept in the correct reading order.
void WorkingPartSet::InsertCompletedBlocks(BLOCK_LIST* blocks,
                                           TO_BLOCK_LIST* to_blocks) {
  BLOCK_IT block_it(&completed_blocks_);
  block_it.add_list_before(blocks);
  TO_BLOCK_IT to_block_it(&to_blocks_);
  to_block_it.add_list_before(to_blocks);
}

// Make a block using lines parallel to the given vector that fit between
// the min and max coordinates specified by the ColPartitions.
// Construct a block from the given list of partitions.
void WorkingPartSet::MakeBlocks(const ICOORD& bleft, const ICOORD& tright,
                                int resolution, ColPartition_LIST* used_parts) {
  part_it_.move_to_first();
  while (!part_it_.empty()) {
    // Gather a list of ColPartitions in block_parts that will be split
    // by linespacing into smaller blocks.
    ColPartition_LIST block_parts;
    ColPartition_IT block_it(&block_parts);
    ColPartition* next_part = NULL;
    bool text_block = false;
    do {
      ColPartition* part = part_it_.extract();
      if (part->blob_type() == BRT_UNKNOWN || part->blob_type() == BRT_TEXT)
        text_block = true;
      part->set_working_set(NULL);
      part_it_.forward();
      block_it.add_after_then_move(part);
      next_part = part->SingletonPartner(false);
      if (part_it_.empty() || next_part != part_it_.data()) {
        // Sequences of partitions can get split by titles.
        next_part = NULL;
      }
      // Merge adjacent blocks that are of the same type and let the
      // linespacing determine the real boundaries.
      if (next_part == NULL && !part_it_.empty()) {
        ColPartition* next_block_part = part_it_.data();
        const TBOX& part_box = part->bounding_box();
        const TBOX& next_box = next_block_part->bounding_box();
        // In addition to the same type, the next box must not be above the
        // current box, nor (if image) too far below.
        if (next_block_part->type() == part->type() &&
            next_box.bottom() <= part_box.top() &&
            (text_block ||
             part_box.bottom() - next_box.top() < part_box.height()))
          next_part = next_block_part;
      }
    } while (!part_it_.empty() && next_part != NULL);
    if (!text_block) {
      TO_BLOCK* to_block = ColPartition::MakeBlock(bleft, tright,
                                                   &block_parts, used_parts);
      if (to_block != NULL) {
        TO_BLOCK_IT to_block_it(&to_blocks_);
        to_block_it.add_to_end(to_block);
        BLOCK_IT block_it(&completed_blocks_);
        block_it.add_to_end(to_block->block);
      }
    } else {
      // Further sub-divide text blocks where linespacing changes.
      ColPartition::LineSpacingBlocks(bleft, tright, resolution, &block_parts,
                                      used_parts,
                                      &completed_blocks_, &to_blocks_);
    }
  }
  part_it_.set_to_list(&part_set_);
  latest_part_ = NULL;
  ASSERT_HOST(completed_blocks_.length() == to_blocks_.length());
}

}  // namespace tesseract.

