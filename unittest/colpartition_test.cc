
#include "tesseract/textord/colpartition.h"

using tesseract::ColPartition;

namespace {

class TestableColPartition : public ColPartition {
 public:
  void SetColumnRange(int first, int last) {
    set_first_column(first);
    set_last_column(last);
  }
};

class ColPartitionTest : public testing::Test {
 protected:
  void SetUp() {
  }

  void TearDown() {
  }
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


} // namespace
