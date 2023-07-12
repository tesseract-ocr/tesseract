///////////////////////////////////////////////////////////////////////
// File:        unicity_table.h
// Description: a class to uniquify objects, manipulating them using integers
//              ids.
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

#include "errcode.h"

#include "genericvector.h"

#include <functional> // for std::function

namespace tesseract {

// A class to uniquify objects, manipulating them using integers ids.
// T requirements:
//   operator= to add an element
//   default-constructible: allocating the internal table will call the default
//     constructor.
template <typename T>
class UnicityTable {
public:
  /// Clear the structures and deallocate internal structures.
  ~UnicityTable() {
    clear();
  }

  /// Reserve some memory. If there is size or more elements, the table will
  /// then allocate size * 2 elements.
  void reserve(int size) {
    table_.reserve(size);
  }

  /// Return the size used.
  int size() const  {
    return table_.size();
  }

  /// Return the object from an id.
  const T &at(int id) const {
    return table_.at(id);
  }

  // Return the pointer to an object with the given id.
  T &at(int id) {
    return table_.at(id);
  }

  T &operator[](size_t id) {
    return table_[id];
  }
  const T &operator[](size_t id) const {
    return table_[id];
  }

  /// Return the id of the T object.
  /// This method NEEDS a compare_callback to be passed to
  /// set_compare_callback.
  int get_index(T object) const {
    return table_.get_index(object);
  }

  /// Add an element in the table
  int push_back(T object)  {
    auto idx = get_index(object);
    if (idx == -1) {
      idx = table_.push_back(object);
    }
    return idx;
  }

  /// Add a callback to be called to delete the elements when the table took
  /// their ownership.
  void set_clear_callback(const std::function<void(T)> &cb) {
    table_.set_clear_callback(cb);
  }

  /// Clear the table, calling the callback function if any.
  /// All the owned Callbacks are also deleted.
  /// If you don't want the Callbacks to be deleted, before calling clear, set
  /// the callback to nullptr.
  void clear()  {
    table_.clear();
  }

  /// This method clear the current object, then, does a shallow copy of
  /// its argument, and finally invalidate its argument.
  void move(UnicityTable<T> *from) {
    table_.move(&from->table_);
  }

  /// Read/Write the table to a file. This does _NOT_ read/write the callbacks.
  /// The Callback given must be permanent since they will be called more than
  /// once. The given callback will be deleted at the end.
  /// Returns false on read/write error.
  bool write(FILE *f, const std::function<bool(FILE *, const T &)> &cb) const {
    return table_.write(f, cb);
  }
  bool read(tesseract::TFile *f, const std::function<bool(tesseract::TFile *, T *)> &cb) {
    return table_.read(f, cb);
  }

private:
  GenericVector<T> table_;
};

} // namespace tesseract

#endif // TESSERACT_CCUTIL_UNICITY_TABLE_H_
