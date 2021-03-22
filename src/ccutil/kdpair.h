// Copyright 2012 Google Inc. All Rights Reserved.
// Author: rays@google.com (Ray Smith)
///////////////////////////////////////////////////////////////////////
// File:        kdpair.h
// Description: Template pair class like STL pair but geared towards
//              the Key+Data design pattern in which some data needs
//              to be sorted or kept in a heap sorted on some separate key.
// Author:      Ray Smith.
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

#include <vector>

namespace tesseract {

// A useful base struct to facilitate the common operation of sorting a vector
// of simple or smart-pointer data using a separate key. Similar to STL pair.
template <typename Key, typename Data>
struct KDPair {
  KDPair() = default;
  KDPair(Key k, Data d) : data_(d), key_(k) {}

  int operator==(const KDPair<Key, Data> &other) const {
    return key_ == other.key_;
  }

  Data &data() {
    return data_;
  }
  const Data &data() const {
    return data_;
  }
  Key &key() {
    return key_;
  }
  const Key &key() const {
    return key_;
  }

  // WARNING! Keep data as the first element! KDPairInc and KDPairDec depend
  // on the order of these elements so they can downcast pointers appropriately
  // for use by GenericHeap::Reshuffle.
  Data data_;
  Key key_;
};
// Specialization of KDPair to provide operator< for sorting in increasing order
// and recasting of data pointers for use with DoublePtr.
template <typename Key, typename Data>
struct KDPairInc : public KDPair<Key, Data> {
  KDPairInc() = default;
  KDPairInc(Key k, Data d) : KDPair<Key, Data>(k, d) {}
  // Operator< facilitates sorting in increasing order.
  int operator<(const KDPairInc<Key, Data> &other) const {
    return this->key() < other.key();
  }
  // Returns the input Data pointer recast to a KDPairInc pointer.
  // Just casts a pointer to the first element to a pointer to the whole struct.
  static KDPairInc *RecastDataPointer(Data *data_ptr) {
    return reinterpret_cast<KDPairInc *>(data_ptr);
  }
};
// Specialization of KDPair to provide operator< for sorting in decreasing order
// and recasting of data pointers for use with DoublePtr.
template <typename Key, typename Data>
struct KDPairDec : public KDPair<Key, Data> {
  KDPairDec() = default;
  KDPairDec(Key k, Data d) : KDPair<Key, Data>(k, d) {}
  // Operator< facilitates sorting in decreasing order by using operator> on
  // the key values.
  int operator<(const KDPairDec<Key, Data> &other) const {
    return this->key() > other.key();
  }
  // Returns the input Data pointer recast to a KDPairDec pointer.
  // Just casts a pointer to the first element to a pointer to the whole struct.
  static KDPairDec *RecastDataPointer(Data *data_ptr) {
    return reinterpret_cast<KDPairDec *>(data_ptr);
  }
};

// A useful base class to facilitate the common operation of sorting a vector
// of owned pointer data using a separate key. This class owns its data pointer,
// deleting it when it has finished with it, and providing copy constructor and
// operator= that have move semantics so that the data does not get copied and
// only a single instance of KDPtrPair holds a specific data pointer.
template <typename Key, typename Data>
class KDPtrPair {
public:
  KDPtrPair() : data_(nullptr) {}
  KDPtrPair(Key k, Data *d) : data_(d), key_(k) {}
  // Copy constructor steals the pointer from src and nulls it in src, thereby
  // moving the (single) ownership of the data.
  KDPtrPair(const KDPtrPair &src) : data_(src.data_), key_(src.key_) {
    ((KDPtrPair &)src).data_ = nullptr;
  }
  // Destructor deletes data, assuming it is the sole owner.
  ~KDPtrPair() {
    delete this->data_;
    this->data_ = nullptr;
  }
  // Operator= steals the pointer from src and nulls it in src, thereby
  // moving the (single) ownership of the data.
  void operator=(const KDPtrPair &src) {
    delete this->data_;
    this->data_ = src.data_;
    ((KDPtrPair &)src).data_ = nullptr;
    this->key_ = src.key_;
  }

  int operator==(const KDPtrPair<Key, Data> &other) const {
    return key_ == other.key_;
  }

  // Accessors.
  const Key &key() const {
    return key_;
  }
  void set_key(const Key &new_key) {
    key_ = new_key;
  }
  const Data *data() const {
    return data_;
  }
  // Sets the data pointer, taking ownership of the data.
  void set_data(Data *new_data) {
    delete data_;
    data_ = new_data;
  }
  // Relinquishes ownership of the data pointer (setting it to nullptr).
  Data *extract_data() {
    Data *result = data_;
    data_ = nullptr;
    return result;
  }

private:
  // Data members are private to keep deletion of data_ encapsulated.
  Data *data_;
  Key key_;
};
// Specialization of KDPtrPair to provide operator< for sorting in increasing
// order.
template <typename Key, typename Data>
struct KDPtrPairInc : public KDPtrPair<Key, Data> {
  // Since we are doing non-standard stuff we have to duplicate *all* the
  // constructors and operator=.
  KDPtrPairInc() : KDPtrPair<Key, Data>() {}
  KDPtrPairInc(Key k, Data *d) : KDPtrPair<Key, Data>(k, d) {}
  KDPtrPairInc(const KDPtrPairInc &src) : KDPtrPair<Key, Data>(src) {}
  void operator=(const KDPtrPairInc &src) {
    KDPtrPair<Key, Data>::operator=(src);
  }
  // Operator< facilitates sorting in increasing order.
  int operator<(const KDPtrPairInc<Key, Data> &other) const {
    return this->key() < other.key();
  }
};
// Specialization of KDPtrPair to provide operator< for sorting in decreasing
// order.
template <typename Key, typename Data>
struct KDPtrPairDec : public KDPtrPair<Key, Data> {
  // Since we are doing non-standard stuff we have to duplicate *all* the
  // constructors and operator=.
  KDPtrPairDec() : KDPtrPair<Key, Data>() {}
  KDPtrPairDec(Key k, Data *d) : KDPtrPair<Key, Data>(k, d) {}
  KDPtrPairDec(const KDPtrPairDec &src) : KDPtrPair<Key, Data>(src) {}
  void operator=(const KDPtrPairDec &src) {
    KDPtrPair<Key, Data>::operator=(src);
  }
  // Operator< facilitates sorting in decreasing order by using operator> on
  // the key values.
  int operator<(const KDPtrPairDec<Key, Data> &other) const {
    return this->key() > other.key();
  }
};

// Specialization for a pair of ints in increasing order.
using IntKDPair = KDPairInc<int, int>;

// Vector of IntKDPair.
class KDVector : public std::vector<IntKDPair> {
  // TODO(rays) Add some code to manipulate a KDVector. For now there
  // is nothing and this class is effectively a specialization typedef.
};

} // namespace tesseract

#endif // TESSERACT_CCUTIL_KDPAIR_H_
