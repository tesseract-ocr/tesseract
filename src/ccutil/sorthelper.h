///////////////////////////////////////////////////////////////////////
// File:        sorthelper.h
// Description: Generic sort and maxfinding class.
// Author:      Ray Smith
// Created:     Thu May 20 17:48:21 PDT 2010
//
// (C) Copyright 2010, Google Inc.
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

#ifndef TESSERACT_CCUTIL_SORTHELPER_H_
#define TESSERACT_CCUTIL_SORTHELPER_H_

#include <cstdlib>
#include "genericvector.h"

// Generic class to provide functions based on a <value,count> pair.
// T is the value type.
// The class keeps a count of each value and can return the most frequent
// value or a sorted array of the values with counts.
// Note that this class uses linear search for adding. It is better
// to use the STATS class to get the mode of a large number of values
// in a small space. SortHelper is better to get the mode of a small number
// of values from a large space.
// T must have a copy constructor.
template <typename T>
class SortHelper {
 public:
  // Simple pair class to hold the values and counts.
  template<typename PairT> struct SortPair {
    PairT value;
    int count;
  };
  // qsort function to sort by decreasing count.
  static int SortPairsByCount(const void* v1, const void* v2) {
    const SortPair<T>* p1 = static_cast<const SortPair<T>*>(v1);
    const SortPair<T>* p2 = static_cast<const SortPair<T>*>(v2);
    return p2->count - p1->count;
  }
  // qsort function to sort by decreasing value.
  static int SortPairsByValue(const void* v1, const void* v2) {
    const SortPair<T>* p1 = static_cast<const SortPair<T>*>(v1);
    const SortPair<T>* p2 = static_cast<const SortPair<T>*>(v2);
    if (p2->value - p1->value < 0) return -1;
    if (p2->value - p1->value > 0) return 1;
    return 0;
  }

  // Constructor takes a hint of the array size, but it need not be accurate.
  explicit SortHelper(int sizehint) {
    counts_.reserve(sizehint);
  }

  // Add a value that may be a duplicate of an existing value.
  // Uses a linear search.
  void Add(T value, int count) {
    // Linear search for value.
    for (int i = 0; i < counts_.size(); ++i) {
      if (counts_[i].value == value) {
        counts_[i].count += count;
        return;
      }
    }
    SortPair<T> new_pair = {value, count};
    counts_.push_back(SortPair<T>(new_pair));
  }

  // Returns the frequency of the most frequent value.
  // If max_value is not nullptr, returns the most frequent value.
  // If the array is empty, returns -INT32_MAX and max_value is unchanged.
  int MaxCount(T* max_value) const {
    int best_count = -INT32_MAX;
    for (int i = 0; i < counts_.size(); ++i) {
      if (counts_[i].count > best_count) {
        best_count = counts_[i].count;
        if (max_value != nullptr)
          *max_value = counts_[i].value;
      }
    }
    return best_count;
  }

  // Returns the data array sorted by decreasing frequency.
  const GenericVector<SortPair<T> >& SortByCount() {
    counts_.sort(&SortPairsByCount);
    return counts_;
  }
  // Returns the data array sorted by decreasing value.
  const GenericVector<SortPair<T> >& SortByValue() {
    counts_.sort(&SortPairsByValue);
    return counts_;
  }

 private:
  GenericVector<SortPair<T> > counts_;
};


#endif  // TESSERACT_CCUTIL_SORTHELPER_H_.
