// Copyright 2012 Google Inc. All Rights Reserved.
// Author: rays@google.com (Ray Smith)
///////////////////////////////////////////////////////////////////////
// File:        kdpair.h
// Description: Template pair class like STL pair but geared towards
//              the Key+Data design pattern in which some data needs
//              to be sorted or kept in a heap sorted on some separate key.
// Author:      Ray Smith.
// Created:     Thu Mar 15 14:48:05 PDT 2012
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

#ifndef TESSERACT_CCUTIL_KDPAIR_H_
#define TESSERACT_CCUTIL_KDPAIR_H_

#include <tesseract/genericvector.h>

#include <memory>
#include <utility>

namespace tesseract {

// A useful base struct to facilitate the common operation of sorting a vector
// of simple or smart-pointer data using a separate key. Similar to STL pair.
template <typename Key, typename Data>
class KDPair : public std::pair<Key, Data> {
  using base = std::pair<Key, Data>;
 public:
  using std::pair<Key, Data>::pair;

  using base::first;
  using base::second;

  int operator==(const KDPair<Key, Data>& other) const {
    return first == other.first;
  }

  Key& key() {
      return first;
  }
  Data& data() {
      return second;
  }
  const Key& key() const {
      return first;
  }
  const Data& data() const {
      return second;
  }
};

// Specialization of KDPair to provide operator< for sorting in increasing order
// and recasting of data pointers for use with DoublePtr.
template <typename Key, typename Data>
class KDPairInc : public KDPair<Key, Data> {
 public:
  using KDPair<Key, Data>::KDPair;

  // Operator< facilitates sorting in increasing order.
  int operator<(const KDPairInc<Key, Data>& other) const {
    return this->key() < other.key();
  }
};

// Specialization of KDPair to provide operator< for sorting in decreasing order
// and recasting of data pointers for use with DoublePtr.
template <typename Key, typename Data>
class KDPairDec : public KDPair<Key, Data> {
 public:
  using KDPair<Key, Data>::KDPair;

  // Operator< facilitates sorting in decreasing order by using operator> on
  // the key values.
  int operator<(const KDPairDec<Key, Data>& other) const {
    return this->key() > other.key();
  }
};

// A useful base class to facilitate the common operation of sorting a vector
// of owned pointer data using a separate key. This class owns its data pointer,
// deleting it when it has finished with it, and providing copy constructor and
// operator= that have move semantics so that the data does not get copied and
// only a single instance of KDPtrPair holds a specific data pointer.
template <typename Key, typename Data>
class KDPtrPair : public std::pair<Key, std::unique_ptr<Data>> {
  using base = std::pair<Key, std::unique_ptr<Data>>;
 public:
  using base::first;
  using base::second;

  KDPtrPair() = default;
  KDPtrPair(Key k, Data* d) : std::pair<Key, std::unique_ptr<Data>>(k, d) {}
  KDPtrPair(KDPtrPair &&src) = default;
  KDPtrPair &operator=(KDPtrPair &&src) = default;

  int operator==(const KDPtrPair<Key, Data>& other) const {
    return key() == other.key();
  }

  // Accessors.
  const Key& key() const {
    return first;
  }
  void set_key(const Key& new_key) {
    first = new_key;
  }
  const Data* data() const {
    return second.get();
  }
  // Sets the data pointer, taking ownership of the data.
  void set_data(Data* new_data) {
    second.reset(new_data);
  }
  // Relinquishes ownership of the data pointer (setting it to nullptr).
  Data* extract_data() {
    return second.release();
  }
};

// Specialization of KDPtrPair to provide operator< for sorting in increasing
// order.
template <typename Key, typename Data>
struct KDPtrPairInc : public KDPtrPair<Key, Data> {
  using KDPtrPair<Key, Data>::KDPtrPair;
  // Operator< facilitates sorting in increasing order.
  int operator<(const KDPtrPairInc<Key, Data>& other) const {
    return this->key() < other.key();
  }
};

// Specialization of KDPtrPair to provide operator< for sorting in decreasing
// order.
template <typename Key, typename Data>
struct KDPtrPairDec : public KDPtrPair<Key, Data> {
  using KDPtrPair<Key, Data>::KDPtrPair;
  // Operator< facilitates sorting in decreasing order by using operator> on
  // the key values.
  int operator<(const KDPtrPairDec<Key, Data>& other) const {
    return this->key() > other.key();
  }
};

// Specialization for a pair of ints in increasing order.
using IntKDPair = KDPairInc<int, int>;

// Vector of IntKDPair.
class KDVector : public GenericVector<IntKDPair> {
  // TODO(rays) Add some code to manipulate a KDVector. For now there
  // is nothing and this class is effectively a specialization typedef.
};

}  // namespace tesseract

#endif  // TESSERACT_CCUTIL_KDPAIR_H_
