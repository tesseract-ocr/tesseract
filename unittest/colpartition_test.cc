// (C) Copyright 2017, Google Inc.
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "blobbox.h"
#include "colpartition.h"
#include "colpartitiongrid.h"
#include "ocrblock.h"

#include "include_gunit.h"

namespace tesseract {

namespace {

int ExtractedBlockCount(BlobRegionType type, BlobTextFlowType flow, int box_count) {
  ColPartitionGrid grid(10, ICOORD(0, 0), ICOORD(100, 100));
  ColPartition *part =
      ColPartition::FakePartition(TBOX(10, 10, 20, 20), PT_FLOWING_TEXT, type, flow);
  for (int i = 1; i < box_count; ++i) {
    const TBOX box(10 + i * 12, 10, 20 + i * 12, 20);
    part->AddBox(new BLOBNBOX(C_BLOB::FakeBlob(box)));
  }
  // Make each BLOBNBOX own its C_BLOB so the fake blobs aren't leaked.
  BLOBNBOX_C_IT owns_it(part->boxes());
  for (owns_it.mark_cycle_pt(); !owns_it.cycled_list(); owns_it.forward()) {
    owns_it.data()->set_owns_cblob(true);
  }
  part->ComputeLimits();
  part->ClaimBoxes();
  part->SetBlobTypes();
  grid.InsertBBox(true, true, part);

  BLOCK_LIST blocks;
  TO_BLOCK_LIST to_blocks;
  grid.ExtractPartitionsAsBlocks(&blocks, &to_blocks);
  return blocks.length();
}

} // namespace

class TestableColPartition : public ColPartition {
public:
  void SetColumnRange(int first, int last) {
    set_first_column(first);
    set_last_column(last);
  }
};

class ColPartitionTest : public testing::Test {
protected:
  void SetUp() override {
    std::locale::global(std::locale(""));
  }

  void TearDown() override {}
};

TEST_F(ColPartitionTest, IsInSameColumnAsReflexive) {
  TestableColPartition a, b;
  a.SetColumnRange(1, 2);
  b.SetColumnRange(3, 3);

  EXPECT_TRUE(a.IsInSameColumnAs(a));
  EXPECT_TRUE(b.IsInSameColumnAs(b));
}

TEST_F(ColPartitionTest, IsInSameColumnAsBorders) {
  TestableColPartition a, b, c, d;
  a.SetColumnRange(0, 1);
  b.SetColumnRange(1, 2);
  c.SetColumnRange(2, 3);
  d.SetColumnRange(4, 5);

  EXPECT_TRUE(a.IsInSameColumnAs(b));
  EXPECT_TRUE(b.IsInSameColumnAs(a));
  EXPECT_FALSE(c.IsInSameColumnAs(d));
  EXPECT_FALSE(d.IsInSameColumnAs(c));
  EXPECT_FALSE(a.IsInSameColumnAs(d));
}

TEST_F(ColPartitionTest, IsInSameColumnAsSuperset) {
  TestableColPartition a, b;
  a.SetColumnRange(4, 7);
  b.SetColumnRange(2, 8);

  EXPECT_TRUE(a.IsInSameColumnAs(b));
  EXPECT_TRUE(b.IsInSameColumnAs(a));
}

TEST_F(ColPartitionTest, IsInSameColumnAsPartialOverlap) {
  TestableColPartition a, b;
  a.SetColumnRange(3, 8);
  b.SetColumnRange(6, 10);

  EXPECT_TRUE(a.IsInSameColumnAs(b));
  EXPECT_TRUE(b.IsInSameColumnAs(a));
}

TEST(ColPartitionGridTest, RejectsMultiBlobUnknownNonTextPartition) {
  EXPECT_EQ(0, ExtractedBlockCount(BRT_UNKNOWN, BTFT_NONTEXT, 2));
}

TEST(ColPartitionGridTest, RetainsMultiBlobUnknownPartitionWithTextFlow) {
  EXPECT_EQ(1, ExtractedBlockCount(BRT_UNKNOWN, BTFT_NEIGHBOURS, 2));
}

TEST(ColPartitionGridTest, RejectsSingleBlobUnknownPartition) {
  EXPECT_EQ(0, ExtractedBlockCount(BRT_UNKNOWN, BTFT_NONTEXT, 1));
}

TEST(ColPartitionGridTest, RetainsTextPartitionWithNonTextFlow) {
  EXPECT_EQ(1, ExtractedBlockCount(BRT_TEXT, BTFT_NONTEXT, 2));
}

} // namespace tesseract
