///////////////////////////////////////////////////////////////////////
// File:        UnicityTable.h
// Description: a class to uniquify objects, manipulating them using integers
// ids.
// Author:      Samuel Charron
//
// (C) Copyright 2006, Google Inc.
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

#ifndef TESSERACT_CCUTIL_UNICITY_TABLE_H_
#define TESSERACT_CCUTIL_UNICITY_TABLE_H_

#include "tesscallback.h"
#include "errcode.h"
#include "genericvector.h"

// A class to uniquify objects, manipulating them using integers ids.
// T requirements:
//   operator= to add an element
//   default-constructible: allocating the internal table will call the default
//     constructor.
template <typename T>
class UnicityTable {
 public:
  UnicityTable();
  /// Clear the structures and deallocate internal structures.
  ~UnicityTable();

  /// Reserve some memory. If there is size or more elements, the table will
  /// then allocate size * 2 elements.
  void reserve(int size);

  /// Return the size used.
  int size() const;

  /// Return the object from an id.
  const T &get(int id) const;

  // Return the pointer to an object with the given id.
  T *get_mutable(int id);

  /// Return the id of the T object.
  /// This method NEEDS a compare_callback to be passed to
  /// set_compare_callback.
  int get_id(T object) const;

  /// Return true if T is in the table
  bool contains(T object) const;

  /// Return true if the id is valid
  T contains_id(int id) const;

  /// Add an element in the table
  int push_back(T object);

  /// Add a callback to be called to delete the elements when the table took
  /// their ownership.
  void set_clear_callback(TessCallback1<T>* cb);

  /// Add a callback to be called to compare the elements when needed (contains,
  /// get_id, ...)
  void set_compare_callback(TessResultCallback2<bool, T const &, T const &>* cb);

  /// Clear the table, calling the callback function if any.
  /// All the owned Callbacks are also deleted.
  /// If you don't want the Callbacks to be deleted, before calling clear, set
  /// the callback to NULL.
  void clear();

  /// This method clear the current object, then, does a shallow copy of
  /// its argument, and finally invalidate its argument.
  void move(UnicityTable<T>* from);

  /// Read/Write the table to a file. This does _NOT_ read/write the callbacks.
  /// The Callback given must be permanent since they will be called more than
  /// once. The given callback will be deleted at the end.
  /// Returns false on read/write error.
  bool write(FILE* f, TessResultCallback2<bool, FILE*, T const &>* cb);
  /// swap is used to switch the endianness.
  bool read(FILE* f, TessResultCallback3<bool, FILE*, T*, bool>* cb, bool swap);

 private:
  GenericVector<T> table_;
  // Mutable because Run method is not const
  mutable TessResultCallback2<bool, T const &, T const &>* compare_cb_;
};

template <typename T>
class UnicityTableEqEq : public UnicityTable<T> {
 public:
  UnicityTableEqEq() {
    UnicityTable<T>::set_compare_callback(
        NewPermanentTessCallback(tesseract::cmp_eq<T>));
  }
};

template <typename T>
UnicityTable<T>::UnicityTable() :
  compare_cb_(0) {
}


template <typename T>
UnicityTable<T>::~UnicityTable() {
  clear();
}

template <typename T>
int UnicityTable<T>::size() const{
  return table_.size();
}

// Reserve some memory. If there is size or more elements, the table will
// then allocate size * 2 elements.
template <typename T>
void UnicityTable<T>::reserve(int size) {
  table_.reserve(size);
}

// Return the object from an id.
template <typename T>
const T &UnicityTable<T>::get(int id) const {
  return table_.get(id);
}
// Returns the pointer to the object with the given id.
template <typename T>
T *UnicityTable<T>::get_mutable(int id) {
  return &(table_.get(id));
}
// Return true if the id is valid
template <typename T>
T UnicityTable<T>::contains_id(int id) const {
  return table_.contains_index(id);
}

// Return the id of the T object.
template <typename T>
int UnicityTable<T>::get_id(T object) const {
  return table_.get_index(object);
}

// Return true if T is in the table
template <typename T>
bool UnicityTable<T>::contains(T object) const {
  return get_id(object) != -1;
}

// Add an element in the table
template <typename T>
int UnicityTable<T>::push_back(T object) {
  int idx = get_id(object);
  if (idx == -1) {
    idx = table_.push_back(object);
  }
  return idx;
}

// Add a callback to be called to delete the elements when the table took
// their ownership.
template <typename T>
void UnicityTable<T>::set_clear_callback(TessCallback1<T>* cb) {
  table_.set_clear_callback(cb);
}

// Add a callback to be called to delete the elements when the table took
// their ownership.
template <typename T>
void UnicityTable<T>::set_compare_callback(TessResultCallback2<bool, T const &, T const &>* cb) {
  table_.set_compare_callback(cb);
  compare_cb_ = cb;
}

// Clear the table, calling the callback function if any.
template <typename T>
void UnicityTable<T>::clear() {
  table_.clear();
}

template <typename T>
bool UnicityTable<T>::write(FILE* f,
                            TessResultCallback2<bool, FILE*, T const &>* cb) {
  return table_.write(f, cb);
}

template <typename T>
bool UnicityTable<T>::read(
    FILE* f, TessResultCallback3<bool, FILE*, T*, bool>* cb, bool swap) {
  return table_.read(f, cb, swap);
}

// This method clear the current object, then, does a shallow copy of
// its argument, and finally invalidate its argument.
template <typename T>
void UnicityTable<T>::move(UnicityTable<T>* from) {
  table_.move(&from->table_);
}

#endif  // TESSERACT_CCUTIL_UNICITY_TABLE_H_
