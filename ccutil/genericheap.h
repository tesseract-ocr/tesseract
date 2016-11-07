// Copyright 2012 Google Inc. All Rights Reserved.
// Author: rays@google.com (Ray Smith)
///////////////////////////////////////////////////////////////////////
// File:        genericheap.h
// Description: Template heap class.
// Author:      Ray Smith, based on Dan Johnson's original code.
// Created:     Wed Mar 14 08:13:00 PDT 2012
//
// (C) Copyright 2012, Google Inc.
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

#include "errcode.h"
#include "genericvector.h"

#ifndef TESSERACT_CCUTIL_GENERICHEAP_H_
#define TESSERACT_CCUTIL_GENERICHEAP_H_

namespace tesseract {

// GenericHeap requires 1 template argument:
// Pair will normally be either KDPairInc<Key, Data> or KDPairDec<Key, Data>
// for some arbitrary Key and scalar, smart pointer, or non-ownership pointer
// Data type, according to whether a MIN heap or a MAX heap is desired,
// respectively. Using KDPtrPairInc<Key, Data> or KDPtrPairDec<Key, Data>,
// GenericHeap can also handle simple Data pointers and own them.
// If no additional data is required, Pair can also be a scalar, since
// GenericHeap doesn't look inside it except for operator<.
//
// The heap is stored as a packed binary tree in an array hosted by a
// GenericVector<Pair>, with the invariant that the children of each node are
// both NOT Pair::operator< the parent node. KDPairInc defines Pair::operator<
// to use Key::operator< to generate a MIN heap and KDPairDec defines
// Pair::operator< to use Key::operator> to generate a MAX heap by reversing
// all the comparisons.
// See http://en.wikipedia.org/wiki/Heap_(data_structure) for more detail on
// the basic heap implementation.
//
// Insertion and removal are both O(log n) and, unlike the STL heap, an
// explicit Reshuffle function allows a node to be repositioned in time O(log n)
// after changing its value.
//
// Accessing the element for revaluation is a more complex matter, since the
// index and pointer can be changed arbitrarily by heap operations.
// Revaluation can be done by making the Data type in the Pair derived from or
// contain a DoublePtr as its first data element, making it possible to convert
// the pointer to a Pair using KDPairInc::RecastDataPointer.
template <typename Pair>
class GenericHeap {
 public:
  GenericHeap() {}
  // The initial size is only a GenericVector::reserve. It is not enforced as
  // the size limit of the heap. Caller must implement their own enforcement.
  explicit GenericHeap(int initial_size) {
    heap_.reserve(initial_size);
  }

  // Simple accessors.
  bool empty() const {
    return heap_.empty();
  }
  int size() const {
    return heap_.size();
  }
  int size_reserved() const {
    return heap_.size_reserved();
  }
  void clear() {
    // Clear truncates to 0 to keep the number reserved in tact.
    heap_.truncate(0);
  }
  // Provides access to the underlying vector.
  // Caution! any changes that modify the keys will invalidate the heap!
  GenericVector<Pair>* heap() {
    return &heap_;
  }
  // Provides read-only access to an element of the underlying vector.
  const Pair& get(int index) const {
    return heap_[index];
  }

  // Add entry to the heap, keeping the smallest item at the top, by operator<.
  // Note that *entry is used as the source of operator=, but it is non-const
  // to allow for a smart pointer to be contained within.
  // Time = O(log n).
  void Push(Pair* entry) {
    int hole_index = heap_.size();
    // Make a hole in the end of heap_ and sift it up to be the correct
    // location for the new *entry. To avoid needing a default constructor
    // for primitive types, and to allow for use of DoublePtr in the Pair
    // somewhere, we have to incur a double copy here.
    heap_.push_back(*entry);
    *entry = heap_.back();
    hole_index = SiftUp(hole_index, *entry);
    heap_[hole_index] = *entry;
  }

  // Get the value of the top (smallest, defined by operator< ) element.
  const Pair& PeekTop() const {
    return heap_[0];
  }
  // Get the value of the worst (largest, defined by operator< ) element.
  const Pair& PeekWorst() const { return heap_[IndexOfWorst()]; }

  // Removes the top element of the heap. If entry is not NULL, the element
  // is copied into *entry, otherwise it is discarded.
  // Returns false if the heap was already empty.
  // Time = O(log n).
  bool Pop(Pair* entry) {
    int new_size = heap_.size() - 1;
    if (new_size < 0)
      return false;  // Already empty.
    if (entry != NULL)
      *entry = heap_[0];
    if (new_size > 0) {
      // Sift the hole at the start of the heap_ downwards to match the last
      // element.
      Pair hole_pair = heap_[new_size];
      heap_.truncate(new_size);
      int hole_index = SiftDown(0, hole_pair);
      heap_[hole_index] = hole_pair;
    } else {
      heap_.truncate(new_size);
    }
    return true;
  }

  // Removes the MAXIMUM element of the heap. (MIN from a MAX heap.) If entry is
  // not NULL, the element is copied into *entry, otherwise it is discarded.
  // Time = O(n). Returns false if the heap was already empty.
  bool PopWorst(Pair* entry) {
    int worst_index = IndexOfWorst();
    if (worst_index < 0) return false;  // It cannot be empty!
    // Extract the worst element from the heap, leaving a hole at worst_index.
    if (entry != NULL)
      *entry = heap_[worst_index];
    int heap_size = heap_.size() - 1;
    if (heap_size > 0) {
      // Sift the hole upwards to match the last element of the heap_
      Pair hole_pair = heap_[heap_size];
      int hole_index = SiftUp(worst_index, hole_pair);
      heap_[hole_index] = hole_pair;
    }
    heap_.truncate(heap_size);
    return true;
  }

  // Returns the index of the worst element. Time = O(n/2).
  int IndexOfWorst() const {
    int heap_size = heap_.size();
    if (heap_size == 0) return -1;  // It cannot be empty!

    // Find the maximum element. Its index is guaranteed to be greater than
    // the index of the parent of the last element, since by the heap invariant
    // the parent must be less than or equal to the children.
    int worst_index = heap_size - 1;
    int end_parent = ParentNode(worst_index);
    for (int i = worst_index - 1; i > end_parent; --i) {
      if (heap_[worst_index] < heap_[i]) worst_index = i;
    }
    return worst_index;
  }

  // The pointed-to Pair has changed its key value, so the location of pair
  // is reshuffled to maintain the heap invariant.
  // Must be a valid pointer to an element of the heap_!
  // Caution! Since GenericHeap is based on GenericVector, reallocs may occur
  // whenever the vector is extended and elements may get shuffled by any
  // Push or Pop operation. Therefore use this function only if Data in Pair is
  // of type DoublePtr, derived (first) from DoublePtr, or has a DoublePtr as
  // its first element. Reshuffles the heap to maintain the invariant.
  // Time = O(log n).
  void Reshuffle(Pair* pair) {
    int index = pair - &heap_[0];
    Pair hole_pair = heap_[index];
    index = SiftDown(index, hole_pair);
    index = SiftUp(index, hole_pair);
    heap_[index] = hole_pair;
  }

 private:
  // A hole in the heap exists at hole_index, and we want to fill it with the
  // given pair. SiftUp sifts the hole upward to the correct position and
  // returns the destination index without actually putting pair there.
  int SiftUp(int hole_index, const Pair& pair) {
    int parent;
    while (hole_index > 0 && pair < heap_[parent = ParentNode(hole_index)]) {
      heap_[hole_index] = heap_[parent];
      hole_index = parent;
    }
    return hole_index;
  }

  // A hole in the heap exists at hole_index, and we want to fill it with the
  // given pair. SiftDown sifts the hole downward to the correct position and
  // returns the destination index without actually putting pair there.
  int SiftDown(int hole_index, const Pair& pair) {
    int heap_size = heap_.size();
    int child;
    while ((child = LeftChild(hole_index)) < heap_size) {
      if (child + 1 < heap_size && heap_[child + 1] < heap_[child])
        ++child;
      if (heap_[child] < pair) {
        heap_[hole_index] = heap_[child];
        hole_index = child;
      } else {
        break;
      }
    }
    return hole_index;
  }

  // Functions to navigate the tree. Unlike the original implementation, we
  // store the root at index 0.
  int ParentNode(int index) const {
    return (index + 1) / 2 - 1;
  }
  int LeftChild(int index) const {
    return index * 2 + 1;
  }

 private:
  GenericVector<Pair> heap_;
};

}  // namespace tesseract

#endif  // TESSERACT_CCUTIL_GENERICHEAP_H_
