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

#include <memory>

#include "colpartition.h"
#include "colpartitiongrid.h"
#include "tablefind.h"

#include "include_gunit.h"

using tesseract::ColPartition;
using tesseract::ColPartition_LIST;
using tesseract::ColPartitionGrid;

namespace {

class TestableTableFinder : public tesseract::TableFinder {
 public:
  using TableFinder::GapInXProjection;
  using TableFinder::HasLeaderAdjacent;
  using TableFinder::InsertLeaderPartition;
  using TableFinder::InsertTextPartition;
  using TableFinder::set_global_median_blob_width;
  using TableFinder::set_global_median_ledding;
  using TableFinder::set_global_median_xheight;
  using TableFinder::SplitAndInsertFragmentedTextPartition;

  void ExpectPartition(const TBOX& box) {
    tesseract::ColPartitionGridSearch gsearch(&fragmented_text_grid_);
    gsearch.SetUniqueMode(true);
    gsearch.StartFullSearch();
    ColPartition* part = nullptr;
    bool found = false;
    while ((part = gsearch.NextFullSearch()) != nullptr) {
      if (part->bounding_box().left() == box.left() &&
          part->bounding_box().bottom() == box.bottom() &&
          part->bounding_box().right() == box.right() &&
          part->bounding_box().top() == box.top()) {
        found = true;
      }
    }
    EXPECT_TRUE(found);
  }
  void ExpectPartitionCount(int expected_count) {
    tesseract::ColPartitionGridSearch gsearch(&fragmented_text_grid_);
    gsearch.SetUniqueMode(true);
    gsearch.StartFullSearch();
    ColPartition* part = nullptr;
    int count = 0;
    while ((part = gsearch.NextFullSearch()) != nullptr) {
      ++count;
    }
    EXPECT_EQ(expected_count, count);
  }
};

class TableFinderTest : public testing::Test {
 protected:
  void SetUp() {
    free_boxes_it_.set_to_list(&free_boxes_);
    finder_.reset(new TestableTableFinder());
    finder_->Init(1, ICOORD(0, 0), ICOORD(500, 500));
    // gap finding
    finder_->set_global_median_xheight(5);
    finder_->set_global_median_blob_width(5);
  }

  void TearDown() {
    if (partition_.get() != nullptr) partition_->DeleteBoxes();
    DeletePartitionListBoxes();
    finder_.reset(nullptr);
  }

  void MakePartition(int x_min, int y_min, int x_max, int y_max) {
    MakePartition(x_min, y_min, x_max, y_max, 0, 0);
  }

  void MakePartition(int x_min, int y_min, int x_max, int y_max,
                     int first_column, int last_column) {
    if (partition_.get() != nullptr) partition_->DeleteBoxes();
    TBOX box;
    box.set_to_given_coords(x_min, y_min, x_max, y_max);
    partition_.reset(
        ColPartition::FakePartition(box, PT_UNKNOWN, BRT_UNKNOWN, BTFT_NONE));
    partition_->set_first_column(first_column);
    partition_->set_last_column(last_column);
  }

  void InsertTextPartition(ColPartition* part) {
    finder_->InsertTextPartition(part);
    free_boxes_it_.add_after_then_move(part);
  }

  void InsertLeaderPartition(int x_min, int y_min, int x_max, int y_max) {
    InsertLeaderPartition(x_min, y_min, x_max, y_max, 0, 0);
  }

  void InsertLeaderPartition(int x_min, int y_min, int x_max, int y_max,
                             int first_column, int last_column) {
    TBOX box;
    box.set_to_given_coords(x_min, y_min, x_max, y_max);
    ColPartition* part = ColPartition::FakePartition(box, PT_FLOWING_TEXT,
                                                     BRT_UNKNOWN, BTFT_LEADER);
    part->set_first_column(first_column);
    part->set_last_column(last_column);
    finder_->InsertLeaderPartition(part);
    free_boxes_it_.add_after_then_move(part);
  }

  void DeletePartitionListBoxes() {
    for (free_boxes_it_.mark_cycle_pt(); !free_boxes_it_.cycled_list();
         free_boxes_it_.forward()) {
      ColPartition* part = free_boxes_it_.data();
      part->DeleteBoxes();
    }
  }

  std::unique_ptr<TestableTableFinder> finder_;
  std::unique_ptr<ColPartition> partition_;

 private:
  tesseract::ColPartition_CLIST free_boxes_;
  tesseract::ColPartition_C_IT free_boxes_it_;
};

TEST_F(TableFinderTest, GapInXProjectionNoGap) {
  int data[100];
  for (int i = 0; i < 100; ++i) data[i] = 10;
  EXPECT_FALSE(finder_->GapInXProjection(data, 100));
}

TEST_F(TableFinderTest, GapInXProjectionEdgeGap) {
  int data[100];
  for (int i = 0; i < 10; ++i) data[i] = 2;
  for (int i = 10; i < 90; ++i) data[i] = 10;
  for (int i = 90; i < 100; ++i) data[i] = 2;
  EXPECT_FALSE(finder_->GapInXProjection(data, 100));
}

TEST_F(TableFinderTest, GapInXProjectionExists) {
  int data[100];
  for (int i = 0; i < 10; ++i) data[i] = 10;
  for (int i = 10; i < 90; ++i) data[i] = 2;
  for (int i = 90; i < 100; ++i) data[i] = 10;
  EXPECT_TRUE(finder_->GapInXProjection(data, 100));
}

TEST_F(TableFinderTest, HasLeaderAdjacentOverlapping) {
  InsertLeaderPartition(90, 0, 150, 5);
  MakePartition(0, 0, 100, 10);
  EXPECT_TRUE(finder_->HasLeaderAdjacent(*partition_));
  MakePartition(0, 25, 100, 40);
  EXPECT_FALSE(finder_->HasLeaderAdjacent(*partition_));
  MakePartition(145, 0, 200, 20);
  EXPECT_TRUE(finder_->HasLeaderAdjacent(*partition_));
  MakePartition(40, 0, 50, 4);
  EXPECT_TRUE(finder_->HasLeaderAdjacent(*partition_));
}

TEST_F(TableFinderTest, HasLeaderAdjacentNoOverlap) {
  InsertLeaderPartition(90, 10, 150, 15);
  MakePartition(0, 10, 85, 20);
  EXPECT_TRUE(finder_->HasLeaderAdjacent(*partition_));
  MakePartition(0, 25, 100, 40);
  EXPECT_FALSE(finder_->HasLeaderAdjacent(*partition_));
  MakePartition(0, 0, 100, 10);
  EXPECT_FALSE(finder_->HasLeaderAdjacent(*partition_));
  // TODO(nbeato): is this a useful metric? case fails
  // MakePartition(160, 0, 200, 15);  // leader is primarily above it
  // EXPECT_FALSE(finder_->HasLeaderAdjacent(*partition_));
}

TEST_F(TableFinderTest, HasLeaderAdjacentPreservesColumns) {
  InsertLeaderPartition(90, 0, 150, 5, 1, 2);
  MakePartition(0, 0, 85, 10, 0, 0);
  EXPECT_FALSE(finder_->HasLeaderAdjacent(*partition_));
  MakePartition(0, 0, 100, 10, 0, 1);
  EXPECT_TRUE(finder_->HasLeaderAdjacent(*partition_));
  MakePartition(0, 0, 200, 10, 0, 5);
  EXPECT_TRUE(finder_->HasLeaderAdjacent(*partition_));
  MakePartition(155, 0, 200, 10, 5, 5);
  EXPECT_FALSE(finder_->HasLeaderAdjacent(*partition_));
}

// TODO(nbeato): Only testing a splitting case. Add more...
// Also test non-split cases.
TEST_F(TableFinderTest, SplitAndInsertFragmentedPartitionsBasicPass) {
  finder_->set_global_median_blob_width(3);
  finder_->set_global_median_xheight(10);

  TBOX part_box(10, 5, 100, 15);
  ColPartition* all = new ColPartition(BRT_UNKNOWN, ICOORD(0, 1));
  all->set_type(PT_FLOWING_TEXT);
  all->set_blob_type(BRT_TEXT);
  all->set_flow(BTFT_CHAIN);
  all->set_left_margin(10);
  all->set_right_margin(100);
  TBOX blob_box = part_box;
  for (int i = 10; i <= 20; i += 5) {
    blob_box.set_left(i + 1);
    blob_box.set_right(i + 4);
    all->AddBox(new BLOBNBOX(C_BLOB::FakeBlob(blob_box)));
  }
  for (int i = 35; i <= 55; i += 5) {
    blob_box.set_left(i + 1);
    blob_box.set_right(i + 4);
    all->AddBox(new BLOBNBOX(C_BLOB::FakeBlob(blob_box)));
  }
  for (int i = 80; i <= 95; i += 5) {
    blob_box.set_left(i + 1);
    blob_box.set_right(i + 4);
    all->AddBox(new BLOBNBOX(C_BLOB::FakeBlob(blob_box)));
  }
  // TODO(nbeato): Ray's newer code...
  // all->ClaimBoxes();
  all->ComputeLimits();      // This is to make sure median iinfo is set.
  InsertTextPartition(all);  // This is to delete blobs
  ColPartition* fragment_me = all->CopyButDontOwnBlobs();

  finder_->SplitAndInsertFragmentedTextPartition(fragment_me);
  finder_->ExpectPartition(TBOX(11, 5, 24, 15));
  finder_->ExpectPartition(TBOX(36, 5, 59, 15));
  finder_->ExpectPartition(TBOX(81, 5, 99, 15));
  finder_->ExpectPartitionCount(3);
}

TEST_F(TableFinderTest, SplitAndInsertFragmentedPartitionsBasicFail) {
  finder_->set_global_median_blob_width(3);
  finder_->set_global_median_xheight(10);

  TBOX part_box(10, 5, 100, 15);
  ColPartition* all = new ColPartition(BRT_UNKNOWN, ICOORD(0, 1));
  all->set_type(PT_FLOWING_TEXT);
  all->set_blob_type(BRT_TEXT);
  all->set_flow(BTFT_CHAIN);
  all->set_left_margin(10);
  all->set_right_margin(100);
  TBOX blob_box = part_box;
  for (int i = 10; i <= 95; i += 5) {
    blob_box.set_left(i + 1);
    blob_box.set_right(i + 4);
    all->AddBox(new BLOBNBOX(C_BLOB::FakeBlob(blob_box)));
  }
  // TODO(nbeato): Ray's newer code...
  // all->ClaimBoxes();
  all->ComputeLimits();      // This is to make sure median iinfo is set.
  InsertTextPartition(all);  // This is to delete blobs
  ColPartition* fragment_me = all->CopyButDontOwnBlobs();

  finder_->SplitAndInsertFragmentedTextPartition(fragment_me);
  finder_->ExpectPartition(TBOX(11, 5, 99, 15));
  finder_->ExpectPartitionCount(1);
}

}  // namespace
