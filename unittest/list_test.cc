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
#if 0 // TODO: add tests for CLIST
#include "clst.h"
#endif
#include "elst.h"
#if 0 // TODO: add tests for ELIST2
#include "elst2.h"
#endif

namespace tesseract {

class ListTest : public ::testing::Test {
 protected:
  void SetUp() override {
    static std::locale system_locale("");
    std::locale::global(system_locale);
  }
};

class Elst : public ELIST_LINK {
 public:
  Elst(unsigned n) : value(n) {
  }
  unsigned value;
};

ELISTIZEH(Elst)
ELISTIZE(Elst)

TEST_F(ListTest, TestELIST) {
  Elst_LIST list;
  auto it = ELIST_ITERATOR(&list);
  for (unsigned i = 0; i < 10; i++) {
    auto* elst = new Elst(i);
    //EXPECT_TRUE(elst->empty());
    //EXPECT_EQ(elst->length(), 0);
    it.add_to_end(elst);
  }
  it.move_to_first();
  unsigned n = 0;
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    auto* elst = reinterpret_cast<Elst*>(it.data());
    EXPECT_EQ(elst->value, n);
    n++;
  }
  it.forward();
  n++;
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    auto* elst = reinterpret_cast<Elst*>(it.extract());
    EXPECT_EQ(elst->value, n % 10);
    n++;
    delete elst;
  }
  // TODO: add more tests for ELIST
}

}  // namespace tesseract.
