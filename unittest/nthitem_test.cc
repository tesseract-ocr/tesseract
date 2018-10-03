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

#include "genericvector.h"
#include "kdpair.h"

#include "include_gunit.h"

namespace tesseract {

int test_data[] = {8, 1, 2, -4, 7, 9, 65536, 4, 9, 0, -32767, 6, 7};

// The fixture for testing GenericHeap and DoublePtr.
class NthItemTest : public testing::Test {
 public:
  virtual ~NthItemTest();
  // Pushes the test data onto the KDVector.
  void PushTestData(KDVector* v) {
    for (int i = 0; i < ARRAYSIZE(test_data); ++i) {
      IntKDPair pair(test_data[i], i);
      v->push_back(pair);
    }
  }
};

// Destructor.
// It is defined here, so the compiler can create a single vtable
// instead of a weak vtable (fixes compiler warning).
NthItemTest::~NthItemTest() = default;

// Tests basic results.
TEST_F(NthItemTest, GeneralTest) {
  KDVector v;
  // Push the test data onto the KDVector.
  PushTestData(&v);
  // Get the min item.
  int index = v.choose_nth_item(0);
  // The result is -32767.
  EXPECT_EQ(-32767, v[index].key);
  // Get the max item.
  index = v.choose_nth_item(v.size() - 1);
  // The result is 65536.
  EXPECT_EQ(65536, v[index].key);
  // Invalid items are silently truncated to valid.
  // Get the min item.
  index = v.choose_nth_item(-1);
  // The result is -32767.
  EXPECT_EQ(-32767, v[index].key);
  // Get the max item.
  index = v.choose_nth_item(v.size());
  // The result is 65536.
  EXPECT_EQ(65536, v[index].key);
}

// Tests results on boring data with lots of duplication.
TEST_F(NthItemTest, BoringTest) {
  KDVector v;
  // Push the test data onto the KDVector.
  int test_data[] = {8, 8, 8, 8, 8, 7, 7, 7, 7};
  for (int i = 0; i < ARRAYSIZE(test_data); ++i) {
    IntKDPair pair(test_data[i], i);
    v.push_back(pair);
  }
  // The 3rd item is 7 but the 4th is 8..
  int index = v.choose_nth_item(3);
  // The result is 7.
  EXPECT_EQ(7, v[index].key);
  index = v.choose_nth_item(4);
  // The result is 8.
  EXPECT_EQ(8, v[index].key);
  // Get the min item.
  index = v.choose_nth_item(0);
  // The result is 7.
  EXPECT_EQ(7, v[index].key);
  // Get the max item.
  index = v.choose_nth_item(v.size() - 1);
  // The result is 8.
  EXPECT_EQ(8, v[index].key);
}

// Tests that a unique median in an odd-size array is found correctly.
TEST_F(NthItemTest, UniqueTest) {
  KDVector v;
  // Push the test data onto the KDVector.
  PushTestData(&v);
  // Get the median item.
  int index = v.choose_nth_item(v.size() / 2);
  // The result is 6, it started out at index 11.
  EXPECT_EQ(6, v[index].key);
  EXPECT_EQ(11, v[index].data);
}

// Tests that an equal median is found correctly.
TEST_F(NthItemTest, EqualTest) {
  KDVector v;
  // Push the test data onto the KDVector.
  PushTestData(&v);
  // Add an extra 8. This makes the median 7.
  IntKDPair pair(8, 13);
  v.push_back(pair);
  // Get the median item.
  int index = v.choose_nth_item(v.size() / 2);
  // The result is 7, it started out at index 4 or 12.
  EXPECT_EQ(7, v[index].key);
  EXPECT_TRUE(v[index].data == 4 || v[index].data == 12);
}

}  // namespace tesseract
