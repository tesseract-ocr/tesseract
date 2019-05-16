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
#include "tablerecog.h"

#include "include_gunit.h"

using tesseract::ColPartition;
using tesseract::ColPartition_LIST;
using tesseract::ColPartitionGrid;

namespace {

class TestableTableRecognizer : public tesseract::TableRecognizer {
 public:
  using TableRecognizer::FindLinesBoundingBox;
  using TableRecognizer::HasSignificantLines;
  using TableRecognizer::RecognizeLinedTable;
  using TableRecognizer::RecognizeTable;
  using TableRecognizer::RecognizeWhitespacedTable;
};

class TestableStructuredTable : public tesseract::StructuredTable {
 public:
  using StructuredTable::CountHorizontalIntersections;
  using StructuredTable::CountVerticalIntersections;
  using StructuredTable::FindLinedStructure;
  using StructuredTable::FindWhitespacedColumns;
  using StructuredTable::FindWhitespacedStructure;
  using StructuredTable::VerifyLinedTableCells;

  void InjectCellY(int y) {
    cell_y_.push_back(y);
    cell_y_.sort();
  }
  void InjectCellX(int x) {
    cell_x_.push_back(x);
    cell_x_.sort();
  }

  void ExpectCellX(int x_min, int second, int add, int almost_done, int x_max) {
    ASSERT_EQ(0, (almost_done - second) % add);
    EXPECT_EQ(3 + (almost_done - second) / add, cell_x_.length());
    EXPECT_EQ(x_min, cell_x_.get(0));
    EXPECT_EQ(x_max, cell_x_.get(cell_x_.length() - 1));
    for (int i = 1; i < cell_x_.length() - 1; ++i) {
      EXPECT_EQ(second + add * (i - 1), cell_x_.get(i));
    }
  }

  void ExpectSortedX() {
    EXPECT_GT(cell_x_.length(), 0);
    for (int i = 1; i < cell_x_.length(); ++i) {
      EXPECT_LT(cell_x_.get(i - 1), cell_x_.get(i));
    }
  }
};

class SharedTest : public testing::Test {
 protected:
  void SetUp() {
    std::locale::global(std::locale(""));
    ICOORD bleft(0, 0);
    ICOORD tright(1000, 1000);
    text_grid_.reset(new ColPartitionGrid(5, bleft, tright));
    line_grid_.reset(new ColPartitionGrid(5, bleft, tright));
  }

  void TearDown() {
    tesseract::ColPartition_IT memory(&allocated_parts_);
    for (memory.mark_cycle_pt(); !memory.cycled_list(); memory.forward()) {
      memory.data()->DeleteBoxes();
    }
  }

  void InsertPartitions() {
    for (int row = 0; row < 800; row += 20)
      for (int col = 0; col < 500; col += 25)
        InsertPartition(col + 1, row + 1, col + 24, row + 19);
  }

  void InsertPartition(int left, int bottom, int right, int top) {
    TBOX box(left, bottom, right, top);
    ColPartition* part =
        ColPartition::FakePartition(box, PT_FLOWING_TEXT, BRT_TEXT, BTFT_NONE);
    part->set_median_width(3);
    part->set_median_height(3);
    text_grid_->InsertBBox(true, true, part);

    tesseract::ColPartition_IT add_it(&allocated_parts_);
    add_it.add_after_stay_put(part);
  }

  void InsertLines() {
    line_box_.set_to_given_coords(
        100 - line_grid_->gridsize(), 10 - line_grid_->gridsize(),
        450 + line_grid_->gridsize(), 50 + line_grid_->gridsize());
    for (int i = 10; i <= 50; i += 10) InsertHorizontalLine(100, 450, i);
    for (int i = 100; i <= 450; i += 50) InsertVerticalLine(i, 10, 50);

    for (int i = 100; i <= 200; i += 20) InsertHorizontalLine(0, 100, i);
  }

  void InsertHorizontalLine(int left, int right, int y) {
    TBOX box(left, y - line_grid_->gridsize(), right,
             y + line_grid_->gridsize());
    ColPartition* part =
        ColPartition::FakePartition(box, PT_HORZ_LINE, BRT_HLINE, BTFT_NONE);
    line_grid_->InsertBBox(true, true, part);

    tesseract::ColPartition_IT add_it(&allocated_parts_);
    add_it.add_after_stay_put(part);
  }
  void InsertVerticalLine(int x, int bottom, int top) {
    TBOX box(x - line_grid_->gridsize(), bottom, x + line_grid_->gridsize(),
             top);
    ColPartition* part =
        ColPartition::FakePartition(box, PT_VERT_LINE, BRT_VLINE, BTFT_NONE);
    line_grid_->InsertBBox(true, true, part);

    tesseract::ColPartition_IT add_it(&allocated_parts_);
    add_it.add_after_stay_put(part);
  }

  void InsertCellsInLines() {
    for (int y = 10; y <= 50; y += 10)
      for (int x = 100; x <= 450; x += 50)
        InsertPartition(x + 1, y + 1, x + 49, y + 9);
  }

  TBOX line_box_;
  std::unique_ptr<ColPartitionGrid> text_grid_;
  std::unique_ptr<ColPartitionGrid> line_grid_;
  ColPartition_LIST allocated_parts_;
};

class TableRecognizerTest : public SharedTest {
 protected:
  void SetUp() {
    SharedTest::SetUp();
    recognizer_.reset(new TestableTableRecognizer());
    recognizer_->Init();
    recognizer_->set_text_grid(text_grid_.get());
    recognizer_->set_line_grid(line_grid_.get());
  }

  std::unique_ptr<TestableTableRecognizer> recognizer_;
};

class StructuredTableTest : public SharedTest {
 protected:
  void SetUp() {
    SharedTest::SetUp();
    table_.reset(new TestableStructuredTable());
    table_->Init();
    table_->set_text_grid(text_grid_.get());
    table_->set_line_grid(line_grid_.get());
  }

  std::unique_ptr<TestableStructuredTable> table_;
};

TEST_F(TableRecognizerTest, HasSignificantLinesBasicPass) {
  InsertLines();
  TBOX smaller_guess(120, 15, 370, 45);
  TBOX larger_guess(90, 5, 490, 70);
  EXPECT_TRUE(recognizer_->HasSignificantLines(line_box_));
  EXPECT_TRUE(recognizer_->HasSignificantLines(larger_guess));
  EXPECT_TRUE(recognizer_->HasSignificantLines(smaller_guess));
}

TEST_F(TableRecognizerTest, HasSignificantLinesBasicFail) {
  InsertLines();
  TBOX box(370, 35, 500, 45);
  EXPECT_FALSE(recognizer_->HasSignificantLines(box));
}

TEST_F(TableRecognizerTest, HasSignificantLinesHorizontalOnlyFails) {
  InsertLines();
  TBOX box(0, 100, 200, 200);
  EXPECT_FALSE(recognizer_->HasSignificantLines(box));
}

TEST_F(TableRecognizerTest, FindLinesBoundingBoxBasic) {
  InsertLines();
  TBOX box(0, 0, 200, 50);
  bool result = recognizer_->FindLinesBoundingBox(&box);
  EXPECT_TRUE(result);
  EXPECT_EQ(line_box_.left(), box.left());
  EXPECT_EQ(line_box_.right(), box.right());
  EXPECT_EQ(line_box_.bottom(), box.bottom());
  EXPECT_EQ(line_box_.top(), box.top());
}

TEST_F(TableRecognizerTest, RecognizeLinedTableBasic) {
  InsertLines();
  TBOX guess(120, 15, 370, 45);
  tesseract::StructuredTable table;
  table.set_text_grid(text_grid_.get());
  table.set_line_grid(line_grid_.get());

  EXPECT_TRUE(recognizer_->RecognizeLinedTable(guess, &table));
  EXPECT_EQ(line_box_.bottom(), table.bounding_box().bottom());
  EXPECT_EQ(line_box_.top(), table.bounding_box().top());
  EXPECT_EQ(line_box_.left(), table.bounding_box().left());
  EXPECT_EQ(line_box_.right(), table.bounding_box().right());
  EXPECT_EQ(line_box_.area(), table.bounding_box().area());
  EXPECT_EQ(7, table.column_count());
  EXPECT_EQ(4, table.row_count());
  EXPECT_EQ(28, table.cell_count());
  EXPECT_TRUE(table.is_lined());
}

TEST_F(TableRecognizerTest, RecognizeWhitespacedTableBasic) {
  InsertPartitions();
  TBOX guess(0, 0, 500, 800);

  tesseract::StructuredTable table;
  table.set_text_grid(text_grid_.get());
  table.set_line_grid(line_grid_.get());
  EXPECT_TRUE(recognizer_->RecognizeWhitespacedTable(guess, &table));
  EXPECT_EQ(1, table.bounding_box().bottom());
  EXPECT_EQ(799, table.bounding_box().top());
  EXPECT_EQ(1, table.bounding_box().left());
  EXPECT_EQ(499, table.bounding_box().right());
  EXPECT_EQ(798 * 498, table.bounding_box().area());
  EXPECT_EQ(500 / 25, table.column_count());
  EXPECT_EQ(800 / 20, table.row_count());
  EXPECT_EQ(500 * 800 / 20 / 25, table.cell_count());
  EXPECT_FALSE(table.is_lined());
}

TEST_F(StructuredTableTest, CountVerticalIntersectionsAll) {
  table_->set_bounding_box(TBOX(0, 0, 1000, 1000));
  InsertPartition(0, 0, 100, 10);
  InsertPartition(1, 12, 43, 21);
  EXPECT_EQ(2, table_->CountVerticalIntersections(4));
  EXPECT_EQ(2, table_->CountVerticalIntersections(20));
  EXPECT_EQ(2, table_->CountVerticalIntersections(40));
  EXPECT_EQ(1, table_->CountVerticalIntersections(50));
  EXPECT_EQ(1, table_->CountVerticalIntersections(60));
  EXPECT_EQ(1, table_->CountVerticalIntersections(80));
  EXPECT_EQ(1, table_->CountVerticalIntersections(95));
  EXPECT_EQ(0, table_->CountVerticalIntersections(104));
  EXPECT_EQ(0, table_->CountVerticalIntersections(150));
}

TEST_F(StructuredTableTest, CountHorizontalIntersectionsAll) {
  table_->set_bounding_box(TBOX(0, 0, 1000, 1000));
  InsertPartition(0, 3, 100, 10);
  InsertPartition(110, 5, 200, 16);

  EXPECT_EQ(0, table_->CountHorizontalIntersections(0));
  EXPECT_EQ(1, table_->CountHorizontalIntersections(4));
  EXPECT_EQ(2, table_->CountHorizontalIntersections(8));
  EXPECT_EQ(1, table_->CountHorizontalIntersections(12));
  EXPECT_EQ(0, table_->CountHorizontalIntersections(20));
}

TEST_F(StructuredTableTest, VerifyLinedTableBasicPass) {
  for (int y = 10; y <= 50; y += 10) table_->InjectCellY(y);
  for (int x = 100; x <= 450; x += 50) table_->InjectCellX(x);
  InsertLines();
  InsertCellsInLines();
  table_->set_bounding_box(line_box_);
  EXPECT_TRUE(table_->VerifyLinedTableCells());
}

TEST_F(StructuredTableTest, VerifyLinedTableHorizontalFail) {
  for (int y = 10; y <= 50; y += 10) table_->InjectCellY(y);
  for (int x = 100; x <= 450; x += 50) table_->InjectCellX(x);
  InsertLines();
  InsertCellsInLines();
  InsertPartition(101, 11, 299, 19);
  table_->set_bounding_box(line_box_);
  EXPECT_FALSE(table_->VerifyLinedTableCells());
}

TEST_F(StructuredTableTest, VerifyLinedTableVerticalFail) {
  for (int y = 10; y <= 50; y += 10) table_->InjectCellY(y);
  for (int x = 100; x <= 450; x += 50) table_->InjectCellX(x);
  InsertLines();
  InsertCellsInLines();
  InsertPartition(151, 21, 199, 39);
  table_->set_bounding_box(line_box_);
  EXPECT_FALSE(table_->VerifyLinedTableCells());
}

TEST_F(StructuredTableTest, FindWhitespacedColumnsBasic) {
  InsertPartitions();
  TBOX guess(0, 0, 500, 800);
  table_->set_bounding_box(guess);
  table_->FindWhitespacedColumns();
  table_->ExpectCellX(1, 25, 25, 475, 499);
}

TEST_F(StructuredTableTest, FindWhitespacedColumnsSorted) {
  InsertPartitions();
  TBOX guess(0, 0, 500, 800);
  table_->set_bounding_box(guess);
  table_->FindWhitespacedColumns();
  table_->ExpectSortedX();
}

// TODO(nbeato): check failure cases
// TODO(nbeato): check Recognize processes correctly on trivial real examples.

}  // namespace
