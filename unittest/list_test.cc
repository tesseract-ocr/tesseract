// (C) Copyright 2020, Google Inc.
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "include_gunit.h"
#include "clst.h"
#include "elst.h"
#include "elst2.h"

namespace tesseract {

class ListTest : public ::testing::Test {
protected:
  void SetUp() override {
    static std::locale system_locale("");
    std::locale::global(system_locale);
  }
  const size_t ListSize = 5;
};

class Clst : public CLIST_LINK {
public:
  Clst(unsigned n) : value(n) {}
  unsigned value;
};

class Elst : public ELIST_LINK {
public:
  Elst(unsigned n) : value(n) {}
  unsigned value;
};

class Elst2 : public ELIST2_LINK {
public:
  Elst2(unsigned n) : value(n) {}
  unsigned value;
};

CLISTIZEH(Clst)
ELISTIZEH(Elst)
ELIST2IZEH(Elst2)

TEST_F(ListTest, TestCLIST) {
  Clst_CLIST list;
  EXPECT_TRUE(list.empty());
  EXPECT_EQ(list.length(), 0);
  auto it = CLIST_ITERATOR(&list);
  for (unsigned i = 0; i < ListSize; i++) {
    auto *lst = new Clst(i);
    it.add_to_end(lst);
  }
  EXPECT_TRUE(!list.empty());
  EXPECT_EQ(list.length(), ListSize);
  it.move_to_first();
  unsigned n = 0;
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    EXPECT_TRUE(n == 0 || !it.at_first());
    auto *lst = reinterpret_cast<Clst *>(it.data());
    EXPECT_EQ(lst->value, n);
    n++;
    EXPECT_TRUE(n != ListSize || it.at_last());
  }
  it.forward();
  n++;
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    auto *lst = reinterpret_cast<Clst *>(it.extract());
    EXPECT_EQ(lst->value, n % ListSize);
    n++;
    delete lst;
  }
  // TODO: add more tests for CLIST
}

TEST_F(ListTest, TestELIST) {
  Elst_LIST list;
  EXPECT_TRUE(list.empty());
  EXPECT_EQ(list.length(), 0);
  auto it = ELIST_ITERATOR(&list);
  for (unsigned i = 0; i < ListSize; i++) {
    auto *elst = new Elst(i);
    it.add_to_end(elst);
  }
  EXPECT_TRUE(!list.empty());
  EXPECT_EQ(list.length(), ListSize);
  it.move_to_first();
  unsigned n = 0;
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    EXPECT_TRUE(n == 0 || !it.at_first());
    auto *elst = reinterpret_cast<Elst *>(it.data());
    EXPECT_EQ(elst->value, n);
    n++;
    EXPECT_TRUE(n != ListSize || it.at_last());
  }
  it.forward();
  n++;
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    auto *elst = reinterpret_cast<Elst *>(it.extract());
    EXPECT_EQ(elst->value, n % ListSize);
    n++;
    delete elst;
  }
  // TODO: add more tests for ELIST
}

TEST_F(ListTest, TestELIST2) {
  Elst2_LIST list;
  EXPECT_TRUE(list.empty());
  EXPECT_EQ(list.length(), 0);
  auto it = ELIST2_ITERATOR(&list);
  for (unsigned i = 0; i < ListSize; i++) {
    auto *lst = new Elst2(i);
    it.add_to_end(lst);
  }
  EXPECT_TRUE(!list.empty());
  EXPECT_EQ(list.length(), ListSize);
  it.move_to_first();
  unsigned n = 0;
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    EXPECT_TRUE(n == 0 || !it.at_first());
    auto *lst = reinterpret_cast<Elst2 *>(it.data());
    EXPECT_EQ(lst->value, n);
    n++;
    EXPECT_TRUE(n != ListSize || it.at_last());
  }
  it.backward();
  n--;
  for (it.mark_cycle_pt(); !it.cycled_list(); it.backward()) {
    auto *lst = reinterpret_cast<Elst2 *>(it.data());
    EXPECT_EQ(lst->value, n);
    n--;
  }
  it.forward();
  n++;
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    auto *lst = reinterpret_cast<Elst2 *>(it.extract());
    EXPECT_EQ(lst->value, n % ListSize);
    n++;
    delete lst;
  }
  // TODO: add more tests for ELIST2
}

} // namespace tesseract.
