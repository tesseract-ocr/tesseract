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

#include <string>
#include <utility>

#include "doubleptr.h"
#include "genericheap.h"
#include <tesseract/genericvector.h>
#include "kdpair.h"

#include "include_gunit.h"

namespace tesseract {

int test_data[] = {8, 1, 2, -4, 7, 9, 65536, 4, 9, 0};

// The fixture for testing GenericHeap and DoublePtr.
class HeapTest : public testing::Test {
 protected:
  void SetUp() {
    std::locale::global(std::locale(""));
  }

 public:
  virtual ~HeapTest();
  // Pushes the test data onto both the heap and the KDVector.
  void PushTestData(GenericHeap<IntKDPair>* heap, KDVector* v) {
    for (size_t i = 0; i < ARRAYSIZE(test_data); ++i) {
      IntKDPair pair(test_data[i], i);
      heap->Push(&pair);
      v->push_back(pair);
    }
  }
  // Verifies that the data in the heap matches the vector (after sorting) by
  // popping everything off the heap.
  void VerifyHeapVectorMatch(GenericHeap<IntKDPair>* heap, KDVector* v) {
    EXPECT_FALSE(heap->empty());
    EXPECT_EQ(heap->size(), v->size());
    // Sort the vector and check that the keys come out of the heap in the same
    // order as v.
    // Also check that the indices match, except for 9, which is duplicated.
    v->sort();
    // Check that we have increasing order.
    EXPECT_LT((*v)[0].key, v->back().key);
    for (int i = 0; i < v->size(); ++i) {
      EXPECT_EQ((*v)[i].key, heap->PeekTop().key);
      // Indices don't necessarily match for equal keys, so don't test them.
      if (i + 1 < v->size() && (*v)[i + 1].key == (*v)[i].key) {
        while (i + 1 < v->size() && (*v)[i + 1].key == (*v)[i].key) {
          heap->Pop(nullptr);
          ++i;
          EXPECT_FALSE(heap->empty());
          EXPECT_EQ((*v)[i].key, heap->PeekTop().key);
        }
      } else {
        // The indices must also match if the key is unique.
        EXPECT_EQ((*v)[i].data, heap->PeekTop().data);
      }
      EXPECT_FALSE(heap->empty());
      EXPECT_TRUE(heap->Pop(nullptr));
    }
    EXPECT_TRUE(heap->empty());
  }
};

// Destructor.
// It is defined here, so the compiler can create a single vtable
// instead of a weak vtable (fixes compiler warning).
HeapTest::~HeapTest() = default;

// Tests that a sort using a GenericHeap matches the result of a sort using
// a KDVector.
TEST_F(HeapTest, SortTest) {
  GenericHeap<IntKDPair> heap;
  EXPECT_TRUE(heap.empty());
  KDVector v;
  EXPECT_EQ(heap.size(), v.size());
  // Push the test data onto both the heap and the KDVector.
  PushTestData(&heap, &v);
  VerifyHeapVectorMatch(&heap, &v);
}

// Tests that pushing some stuff, popping some stuff, and then pushing more
// stuff results in output that matches the sort using a KDVector.
// a KDVector.
TEST_F(HeapTest, MixedTest) {
  GenericHeap<IntKDPair> heap;
  KDVector v;
  // Push the test data onto both the heap and the KDVector.
  PushTestData(&heap, &v);
  // Sort the vector and remove the first 5 values from both heap and v.
  v.sort();
  for (int i = 0; i < 5; ++i) {
    heap.Pop(nullptr);
    v.remove(0);
  }
  // Push the test data onto both the heap and the KDVector.
  PushTestData(&heap, &v);
  // Heap and vector should still match!
  VerifyHeapVectorMatch(&heap, &v);
}

// Tests that PopWorst still leaves the heap in a state such that it still
// matches a sorted KDVector.
TEST_F(HeapTest, PopWorstTest) {
  GenericHeap<IntKDPair> heap;
  KDVector v;
  // Push the test data onto both the heap and the KDVector.
  PushTestData(&heap, &v);
  // Get the worst element off the heap.
  IntKDPair pair;
  heap.PopWorst(&pair);
  EXPECT_EQ(pair.key, 65536);
  EXPECT_EQ(pair.data, 6);
  // Sort and remove the worst element from the vector.
  v.sort();
  v.truncate(v.size() - 1);
  // After that they should still match!
  VerifyHeapVectorMatch(&heap, &v);
}

// Tests that Reshuffle works and the heap still matches a KDVector with the
// same value changed. Doubles up as a test of DoublePtr.
TEST_F(HeapTest, RevalueTest) {
  // Here the data element of the pair is a DoublePtr, which links the entries
  // in the vector and heap, and we test a MAX heap.
  typedef KDPairDec<int, DoublePtr> PtrPair;
  GenericHeap<PtrPair> heap;
  GenericVector<PtrPair> v;
  // Push the test data onto both the heap and the vector.
  for (size_t i = 0; i < ARRAYSIZE(test_data); ++i) {
    PtrPair h_pair;
    h_pair.key = test_data[i];
    PtrPair v_pair;
    v_pair.key = test_data[i];
    h_pair.data.Connect(&v_pair.data);
    heap.Push(&h_pair);
    v.push_back(v_pair);
  }
  // Test changes both ways. Index 0 is 8, so change it to -1.
  v[0].key = -1;
  // v[0].data.OtherEnd() is a pointer to the data element in the appropriate
  // heap entry, wherever it may be. We can change its value via that pointer.
  // Without Reshuffle, that would be a terribly bad thing to do, as it violates
  // the heap invariant, making the heap corrupt.
  PtrPair* pair_ptr = PtrPair::RecastDataPointer(v[0].data.OtherEnd());
  pair_ptr->key = v[0].key;
  heap.Reshuffle(pair_ptr);
  // Index 1 is 1. Change to 32767.
  v[1].key = 32767;
  pair_ptr = PtrPair::RecastDataPointer(v[1].data.OtherEnd());
  pair_ptr->key = v[1].key;
  heap.Reshuffle(pair_ptr);
  // After the changes, popping the heap should still match the sorted order
  // of the vector.
  v.sort();
  EXPECT_GT(v[0].key, v.back().key);
  for (int i = 0; i < v.size(); ++i) {
    EXPECT_EQ(v[i].key, heap.PeekTop().key);
    EXPECT_FALSE(heap.empty());
    heap.Pop(nullptr);
  }
  EXPECT_TRUE(heap.empty());
}

#if 0
// Helper checks that the compiler rejects use of a copy constructor with
// a const argument and the default copy constructor is properly hidden by
// the non-const version.
static void ConstRefTest(const DoublePtr& ptr1) {
  DoublePtr ptr2(ptr1);  // Compiler error here.
  EXPECT_EQ(&ptr2, ptr2.OtherEnd()->OtherEnd());
  EXPECT_TRUE(ptr1.OtherEnd() == nullptr);
}
#endif

// Tests that DoublePtr works as expected.
TEST_F(HeapTest, DoublePtrTest) {
  DoublePtr ptr1;
  DoublePtr ptr2;
  ptr1.Connect(&ptr2);
  // Check that the correct copy constructor is used.
  DoublePtr ptr3(ptr1);
  EXPECT_EQ(&ptr3, ptr3.OtherEnd()->OtherEnd());
  EXPECT_TRUE(ptr1.OtherEnd() == nullptr);
  // Check that the correct operator= is used.
  ptr1 = ptr3;
  EXPECT_EQ(&ptr1, ptr1.OtherEnd()->OtherEnd());
  EXPECT_TRUE(ptr3.OtherEnd() == nullptr);
}

}  // namespace tesseract
