///////////////////////////////////////////////////////////////////////
// File:        genericvector.h
// Description: Generic vector class
// Author:      Daria Antonova
// Created:     Mon Jun 23 11:26:43 PDT 2008
//
// (C) Copyright 2007, Google Inc.
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
//
#ifndef TESSERACT_CCUTIL_GENERICVECTOR_H_
#define TESSERACT_CCUTIL_GENERICVECTOR_H_

#include <stdio.h>

#include "callback.h"
#include "errcode.h"
#include "helpers.h"

template <typename T>
class GenericVector {
 public:
  GenericVector() { this->init(kDefaultVectorSize); }
  GenericVector(int size) { this->init(size); }

  // Copy
  GenericVector(const GenericVector& other) {
    this->init(other.size());
    this->operator+=(other);
  }
  GenericVector<T> &operator+=(const GenericVector& other);
  GenericVector<T> &operator=(const GenericVector& other);

  virtual ~GenericVector();

  // Reserve some memory.
  void reserve(int size);
  // Double the size of the internal array.
  void double_the_size();

  // Return the size used.
  int size() const {
    return size_used_;
  }

  int length() const {
    return size_used_;
  }

  // Return true if empty.
  bool empty() const {
    return size_used_ == 0;
  }

  // Return the object from an index.
  T &get(int index) const;
  T &operator[](int index) const;

  // Return the index of the T object.
  // This method NEEDS a compare_callback to be passed to
  // set_compare_callback.
  int get_index(T object) const;

  // Return true if T is in the array
  bool contains(T object) const;

  // Return true if the index is valid
  T contains_index(int index) const;

  // Push an element in the end of the array
  int push_back(T object);
  void operator+=(T t);

  // Set the value at the given index
  void set(T t, int index);

  // Insert t at the given index, push other elements to the right.
  void insert(T t, int index);

  // Removes an element at the given index and
  // shifts the remaining elements to the left.
  void remove(int index);

  // Add a callback to be called to delete the elements when the array took
  // their ownership.
  void set_clear_callback(Callback1<T>* cb);

  // Add a callback to be called to compare the elements when needed (contains,
  // get_id, ...)
  void set_compare_callback(ResultCallback2<bool, T const &, T const &>* cb);

  // Clear the array, calling the clear callback function if any.
  // All the owned Callbacks are also deleted.
  // If you don't want the Callbacks to be deleted, before calling clear, set
  // the callback to NULL.
  virtual void clear();

  // Delete objects pointed to by data_[i]
  void delete_data_pointers();

  // This method clears the current object, then, does a shallow copy of
  // its argument, and finally invalidate its argument.
  // Callbacks are moved to the current object;
  void move(GenericVector<T>* from);

  // Read/Write the array to a file. This does _NOT_ read/write the callbacks.
  // The Callback given must be permanent since they will be called more than
  // once. The given callback will be deleted at the end.
  // If the callbacks are NULL, then the data is simply read/written using
  // fread (and swapping)/fwrite.
  // Returns false on error or if the callback returns false.
  bool write(FILE* f, ResultCallback2<bool, FILE*, T const &>* cb) const;
  bool read(FILE* f, ResultCallback3<bool, FILE*, T*, bool>* cb, bool swap);

  // Allocates a new array of double the current_size, copies over the
  // information from data to the new location, deletes data and returns
  // the pointed to the new larger array.
  // This function uses memcpy to copy the data, instead of invoking
  // operator=() for each element like double_the_size() does.
  static T *double_the_size_memcpy(int current_size, T *data) {
    T *data_new = new T[current_size * 2];
    memcpy(data_new, data, sizeof(T) * current_size);
    delete[] data;
    return data_new;
  }

 protected:

  // Init the object, allocating size memory.
  void init(int size);

  // We are assuming that the object generally placed in thie
  // vector are small enough that for efficiency it makes sence
  // to start with a larger initial size.
  static const int kDefaultVectorSize = 4;
  inT32   size_used_;
  inT32   size_reserved_;
  T*    data_;
  Callback1<T>* clear_cb_;
  // Mutable because Run method is not const
  mutable ResultCallback2<bool, T const &, T const &>* compare_cb_;
};

namespace tesseract {

template <typename T>
bool cmp_eq(T const & t1, T const & t2) {
  return t1 == t2;
}

}  // namespace tesseract

// A useful vector that uses operator== to do comparisons.
template <typename T>
class GenericVectorEqEq : public GenericVector<T> {
 public:
  GenericVectorEqEq() {
    GenericVector<T>::set_compare_callback(
        NewPermanentCallback(tesseract::cmp_eq<T>));
  }
  GenericVectorEqEq(int size) : GenericVector<T>(size) {
    GenericVector<T>::set_compare_callback(
        NewPermanentCallback(tesseract::cmp_eq<T>));
  }
};

template <typename T>
void GenericVector<T>::init(int size) {
  size_used_ = 0;
  size_reserved_ = 0;
  data_ = 0;
  clear_cb_ = 0;
  compare_cb_ = 0;
  reserve(size);
}

template <typename T>
GenericVector<T>::~GenericVector() {
  clear();
}

// Reserve some memory. If the internal array contains elements, they are
// copied.
template <typename T>
void GenericVector<T>::reserve(int size) {
  if (size_reserved_ > size || size <= 0)
    return;
  T* new_array = new T[size];
  for (int i = 0; i < size_used_; ++i)
    new_array[i] = data_[i];
  if (data_ != NULL) delete[] data_;
  data_ = new_array;
  size_reserved_ = size;
}

template <typename T>
void GenericVector<T>::double_the_size() {
  if (size_reserved_ == 0) {
    reserve(kDefaultVectorSize);
  }
  else {
    reserve(2 * size_reserved_);
  }
}



// Return the object from an index.
template <typename T>
T &GenericVector<T>::get(int index) const {
  ASSERT_HOST(index >= 0 && index < size_used_);
  return data_[index];
}

template <typename T>
T &GenericVector<T>::operator[](int index) const {
 return data_[index];
}

// Return the object from an index.
template <typename T>
void GenericVector<T>::set(T t, int index) {
  ASSERT_HOST(index >= 0 && index < size_used_);
  data_[index] = t;
}

// Shifts the rest of the elements to the right to make
// space for the new elements and inserts the given element
// at the specified index.
template <typename T>
void GenericVector<T>::insert(T t, int index) {
  ASSERT_HOST(index >= 0 && index < size_used_);
  if (size_reserved_ == size_used_)
    double_the_size();
  for (int i = size_used_; i > index; --i) {
    data_[i] = data_[i-1];
  }
  data_[index] = t;
  size_used_++;
}

// Removes an element at the given index and
// shifts the remaining elements to the left.
template <typename T>
void GenericVector<T>::remove(int index) {
  ASSERT_HOST(index >= 0 && index < size_used_);
  for (int i = index; i < size_used_ - 1; ++i) {
    data_[i] = data_[i+1];
  }
  size_used_--;
}

// Return true if the index is valindex
template <typename T>
T GenericVector<T>::contains_index(int index) const {
  return index >= 0 && index < size_used_;
}

// Return the index of the T object.
template <typename T>
int GenericVector<T>::get_index(T object) const {
  for (int i = 0; i < size_used_; ++i) {
    ASSERT_HOST(compare_cb_ != NULL);
    if (compare_cb_->Run(object, data_[i]))
      return i;
  }
  return -1;
}

// Return true if T is in the array
template <typename T>
bool GenericVector<T>::contains(T object) const {
  return get_index(object) != -1;
}

// Add an element in the array
template <typename T>
int GenericVector<T>::push_back(T object) {
  int index = 0;
  if (size_used_ == size_reserved_)
    double_the_size();
  index = size_used_++;
  data_[index] = object;
  return index;
}

template <typename T>
void GenericVector<T>::operator+=(T t) {
  push_back(t);
}

template <typename T>
GenericVector<T> &GenericVector<T>::operator+=(const GenericVector& other) {
  for (int i = 0; i < other.size(); ++i) {
    this->operator+=(other.data_[i]);
  }
  return *this;
}

template <typename T>
GenericVector<T> &GenericVector<T>::operator=(const GenericVector& other) {
  this->clear();
  this->operator+=(other);
  return *this;
}

// Add a callback to be called to delete the elements when the array took
// their ownership.
template <typename T>
void GenericVector<T>::set_clear_callback(Callback1<T>* cb) {
  clear_cb_ = cb;
}

// Add a callback to be called to delete the elements when the array took
// their ownership.
template <typename T>
void GenericVector<T>::set_compare_callback(ResultCallback2<bool, T const &, T const &>* cb) {
  compare_cb_ = cb;
}

// Clear the array, calling the callback function if any.
template <typename T>
void GenericVector<T>::clear() {
  if (size_reserved_ > 0) {
    if (clear_cb_ != NULL)
      for (int i = 0; i < size_used_; ++i)
        clear_cb_->Run(data_[i]);
    delete[] data_;
    data_ = NULL;
    size_used_ = 0;
    size_reserved_ = 0;
  }
  if (clear_cb_ != NULL) {
    delete clear_cb_;
    clear_cb_ = NULL;
  }
  if (compare_cb_ != NULL) {
    delete compare_cb_;
    compare_cb_ = NULL;
  }
}

template <typename T>
void GenericVector<T>::delete_data_pointers() {
  for (int i = 0; i < size_used_; ++i)
    if (data_[i]) {
      delete data_[i];
    }
}


template <typename T>
bool GenericVector<T>::write(
    FILE* f, ResultCallback2<bool, FILE*, T const &>* cb) const {
  if (fwrite(&size_reserved_, sizeof(size_reserved_), 1, f) != 1) return false;
  if (fwrite(&size_used_, sizeof(size_used_), 1, f) != 1) return false;
  if (cb != NULL) {
    for (int i = 0; i < size_used_; ++i) {
      if (!cb->Run(f, data_[i])) {
        delete cb;
        return false;
      }
    }
    delete cb;
  } else {
    if (fwrite(data_, sizeof(T), size_used_, f) != size_used_) return false;
  }
  return true;
}

template <typename T>
bool GenericVector<T>::read(FILE* f,
                            ResultCallback3<bool, FILE*, T*, bool>* cb,
                            bool swap) {
  uinT32 reserved;
  if (fread(&reserved, sizeof(reserved), 1, f) != 1) return false;
  if (swap) Reverse32(&reserved);
  reserve(reserved);
  if (fread(&size_used_, sizeof(size_used_), 1, f) != 1) return false;
  if (swap) Reverse32(&size_used_);
  if (cb != NULL) {
    for (int i = 0; i < size_used_; ++i) {
      if (!cb->Run(f, data_ + i, swap)) {
        delete cb;
        return false;
      }
    }
    delete cb;
  } else {
    if (fread(data_, sizeof(T), size_used_, f) != size_used_) return false;
    if (swap) {
      for (int i = 0; i < size_used_; ++i)
        ReverseN(&data_[i], sizeof(T));
    }
  }
  return true;
}

// This method clear the current object, then, does a shallow copy of
// its argument, and finally invalindate its argument.
template <typename T>
void GenericVector<T>::move(GenericVector<T>* from) {
  this->clear();
  this->data_ = from->data_;
  this->size_reserved_ = from->size_reserved_;
  this->size_used_ = from->size_used_;
  this->compare_cb_ = from->compare_cb_;
  this->clear_cb_ = from->clear_cb_;
  from->data_ = NULL;
  from->clear_cb_ = NULL;
  from->compare_cb_ = NULL;
  from->size_used_ = 0;
  from->size_reserved_ = 0;
}

#endif  // TESSERACT_CCUTIL_GENERICVECTOR_H_
