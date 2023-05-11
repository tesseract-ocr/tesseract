///////////////////////////////////////////////////////////////////////
// File:        genericvector.h
// Description: Generic vector class
// Author:      Daria Antonova
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

#include "helpers.h"
#include "serialis.h"

#include <algorithm>
#include <cassert>
#include <climits> // for LONG_MAX
#include <cstdint> // for uint32_t
#include <cstdio>
#include <cstdlib>
#include <functional> // for std::function

namespace tesseract {

// Use PointerVector<T> below in preference to GenericVector<T*>, as that
// provides automatic deletion of pointers, [De]Serialize that works, and
// sort that works.
template <typename T>
class GenericVector {
public:
  GenericVector() {
    init(kDefaultVectorSize);
  }

  // Copy
  GenericVector(const GenericVector &other) {
    this->init(other.size());
    this->operator+=(other);
  }
  GenericVector<T> &operator+=(const GenericVector &other);
  GenericVector<T> &operator=(const GenericVector &other);

  ~GenericVector();

  // Reserve some memory.
  void reserve(int size);
  // Double the size of the internal array.
  void double_the_size();

  // Resizes to size and sets all values to t.
  void init_to_size(int size, const T &t);
  void resize(int size, const T &t);
  // Resizes to size without any initialization.
  void resize_no_init(int size) {
    reserve(size);
    size_used_ = size;
  }

  // Return the size used.
  unsigned size() const {
    return size_used_;
  }
  // Workaround to avoid g++ -Wsign-compare warnings.
  size_t unsigned_size() const {
    static_assert(sizeof(size_used_) <= sizeof(size_t), "Wow! sizeof(size_t) < sizeof(int32_t)!!");
    assert(0 <= size_used_);
    return static_cast<size_t>(size_used_);
  }
  int size_reserved() const {
    return size_reserved_;
  }

  // Return true if empty.
  bool empty() const {
    return size_used_ == 0;
  }

  // Return the object from an index.
  T &at(int index) const {
    assert(index >= 0 && index < size_used_);
    return data_[index];
  }

  T &back() const;
  T &operator[](int index) const;
  // Returns the last object and removes it.
  T pop_back();

  // Return the index of the T object.
  int get_index(const T &object) const;

  // Push an element in the end of the array
  int push_back(T object);
  void operator+=(const T &t);

  // Set the value at the given index
  void set(const T &t, int index);

  // Insert t at the given index, push other elements to the right.
  void insert(const T &t, int index);

  // Removes an element at the given index and
  // shifts the remaining elements to the left.
  void remove(int index);

  // Truncates the array to the given size by removing the end.
  // If the current size is less, the array is not expanded.
  void truncate(int size) {
    if (size < size_used_) {
      size_used_ = size;
    }
  }

  // Add a callback to be called to delete the elements when the array took
  // their ownership.
  void set_clear_callback(const std::function<void(T)> &cb) {
    clear_cb_ = cb;
  }

  // Clear the array, calling the clear callback function if any.
  // All the owned callbacks are also deleted.
  // If you don't want the callbacks to be deleted, before calling clear, set
  // the callback to nullptr.
  void clear();

  // Delete objects pointed to by data_[i]
  void delete_data_pointers();

  // This method clears the current object, then, does a shallow copy of
  // its argument, and finally invalidates its argument.
  // Callbacks are moved to the current object;
  void move(GenericVector<T> *from);

  // Read/Write the array to a file. This does _NOT_ read/write the callbacks.
  // The callback given must be permanent since they will be called more than
  // once. The given callback will be deleted at the end.
  // If the callbacks are nullptr, then the data is simply read/written using
  // fread (and swapping)/fwrite.
  // Returns false on error or if the callback returns false.
  // DEPRECATED. Use [De]Serialize[Classes] instead.
  bool write(FILE *f, const std::function<bool(FILE *, const T &)> &cb) const;
  bool read(TFile *f, const std::function<bool(TFile *, T *)> &cb);
  // Writes a vector of simple types to the given file. Assumes that bitwise
  // read/write of T will work. Returns false in case of error.
  // TODO(rays) Change all callers to use TFile and remove deprecated methods.
  bool Serialize(FILE *fp) const;
  bool Serialize(TFile *fp) const;
  // Reads a vector of simple types from the given file. Assumes that bitwise
  // read/write will work with ReverseN according to sizeof(T).
  // Returns false in case of error.
  // If swap is true, assumes a big/little-endian swap is needed.
  // TFile is assumed to know about swapping.
  bool DeSerialize(bool swap, FILE *fp);
  bool DeSerialize(TFile *fp);
  // Writes a vector of classes to the given file. Assumes the existence of
  // bool T::Serialize(FILE* fp) const that returns false in case of error.
  // Returns false in case of error.
  bool SerializeClasses(FILE *fp) const;
  // Reads a vector of classes from the given file. Assumes the existence of
  // bool T::Deserialize(bool swap, FILE* fp) that returns false in case of
  // error. Also needs T::T() and T::T(constT&), as init_to_size is used in
  // this function. Returns false in case of error.
  // If swap is true, assumes a big/little-endian swap is needed.
  bool DeSerializeClasses(TFile *fp);

  // Reverses the elements of the vector.
  void reverse() {
    for (int i = 0; i < size_used_ / 2; ++i) {
      std::swap(data_[i], data_[size_used_ - 1 - i]);
    }
  }

  // Sorts the members of this vector using the less than comparator (cmp_lt),
  // which compares the values. Useful for GenericVectors to primitive types.
  // Will not work so great for pointers (unless you just want to sort some
  // pointers). You need to provide a specialization to sort_cmp to use
  // your type.
  void sort();

  // Sort the array into the order defined by the qsort function comparator.
  // The comparator function is as defined by qsort, ie. it receives pointers
  // to two Ts and returns negative if the first element is to appear earlier
  // in the result and positive if it is to appear later, with 0 for equal.
  void sort(int (*comparator)(const void *, const void *)) {
    qsort(data_, size_used_, sizeof(*data_), comparator);
  }

  // Swaps the elements with the given indices.
  void swap(int index1, int index2) {
    if (index1 != index2) {
      T tmp = data_[index1];
      data_[index1] = data_[index2];
      data_[index2] = tmp;
    }
  }
  // Returns true if all elements of *this are within the given range.
  // Only uses operator<
  /*bool WithinBounds(const T& rangemin, const T& rangemax) const {
  for (int i = 0; i < size_used_; ++i) {
    if (data_[i] < rangemin || rangemax < data_[i]) {
      return false;
    }
  }
  return true;
}*/

protected:
  // Init the object, allocating size memory.
  void init(int size);

  // We are assuming that the object generally placed in the
  // vector are small enough that for efficiency it makes sense
  // to start with a larger initial size.
  static const int kDefaultVectorSize = 4;
  int32_t size_used_{};
  int32_t size_reserved_{};
  T *data_;
  std::function<void(T)> clear_cb_;
};

// The default FileReader loads the whole file into the vector of char,
// returning false on error.
inline bool LoadDataFromFile(const char *filename, GenericVector<char> *data) {
  bool result = false;
  FILE *fp = fopen(filename, "rb");
  if (fp != nullptr) {
    fseek(fp, 0, SEEK_END);
    auto size = std::ftell(fp);
    fseek(fp, 0, SEEK_SET);
    // Trying to open a directory on Linux sets size to LONG_MAX. Catch it here.
    if (size > 0 && size < LONG_MAX) {
      // reserve an extra byte in case caller wants to append a '\0' character
      data->reserve(size + 1);
      data->resize_no_init(size);
      result = static_cast<long>(fread(&(*data)[0], 1, size, fp)) == size;
    }
    fclose(fp);
  }
  return result;
}

// The default FileWriter writes the vector of char to the filename file,
// returning false on error.
inline bool SaveDataToFile(const GenericVector<char> &data, const char *filename) {
  FILE *fp = fopen(filename, "wb");
  if (fp == nullptr) {
    return false;
  }
  bool result = fwrite(&data[0], 1, data.size(), fp) == data.size();
  fclose(fp);
  return result;
}

// Used by sort()
// return < 0 if t1 < t2
// return 0 if t1 == t2
// return > 0 if t1 > t2
template <typename T>
int sort_cmp(const void *t1, const void *t2) {
  const T *a = static_cast<const T *>(t1);
  const T *b = static_cast<const T *>(t2);
  if (*a < *b) {
    return -1;
  }
  if (*b < *a) {
    return 1;
  }
  return 0;
}

// Used by PointerVector::sort()
// return < 0 if t1 < t2
// return 0 if t1 == t2
// return > 0 if t1 > t2
template <typename T>
int sort_ptr_cmp(const void *t1, const void *t2) {
  const T *a = *static_cast<T *const *>(t1);
  const T *b = *static_cast<T *const *>(t2);
  if (*a < *b) {
    return -1;
  }
  if (*b < *a) {
    return 1;
  }
  return 0;
}

// Subclass for a vector of pointers. Use in preference to GenericVector<T*>
// as it provides automatic deletion and correct serialization, with the
// corollary that all copy operations are deep copies of the pointed-to objects.
template <typename T>
class PointerVector : public GenericVector<T *> {
public:
  PointerVector() : GenericVector<T *>() {}
  explicit PointerVector(int size) : GenericVector<T *>(size) {}
  ~PointerVector() {
    // Clear must be called here, even though it is called again by the base,
    // as the base will call the wrong clear.
    clear();
  }
  // Copy must be deep, as the pointers will be automatically deleted on
  // destruction.
  PointerVector(const PointerVector &other) : GenericVector<T *>(other) {
    this->init(other.size());
    this->operator+=(other);
  }
  PointerVector<T> &operator+=(const PointerVector &other) {
    this->reserve(this->size_used_ + other.size_used_);
    for (unsigned i = 0; i < other.size(); ++i) {
      this->push_back(new T(*other.data_[i]));
    }
    return *this;
  }

  PointerVector<T> &operator=(const PointerVector &other) {
    if (&other != this) {
      this->truncate(0);
      this->operator+=(other);
    }
    return *this;
  }

  // Removes an element at the given index and
  // shifts the remaining elements to the left.
  void remove(int index) {
    delete GenericVector<T *>::data_[index];
    GenericVector<T *>::remove(index);
  }

  // Truncates the array to the given size by removing the end.
  // If the current size is less, the array is not expanded.
  void truncate(int size) {
    for (int i = size; i < GenericVector<T *>::size_used_; ++i) {
      delete GenericVector<T *>::data_[i];
    }
    GenericVector<T *>::truncate(size);
  }

  // Clear the array, calling the clear callback function if any.
  // All the owned callbacks are also deleted.
  // If you don't want the callbacks to be deleted, before calling clear, set
  // the callback to nullptr.
  void clear() {
    GenericVector<T *>::delete_data_pointers();
    GenericVector<T *>::clear();
  }

  // Writes a vector of (pointers to) classes to the given file. Assumes the
  // existence of bool T::Serialize(FILE*) const that returns false in case of
  // error. There is no Serialize for simple types, as you would have a
  // normal GenericVector of those.
  // Returns false in case of error.
  bool Serialize(FILE *fp) const {
    int32_t used = GenericVector<T *>::size_used_;
    if (fwrite(&used, sizeof(used), 1, fp) != 1) {
      return false;
    }
    for (int i = 0; i < used; ++i) {
      int8_t non_null = GenericVector<T *>::data_[i] != nullptr;
      if (fwrite(&non_null, sizeof(non_null), 1, fp) != 1) {
        return false;
      }
      if (non_null && !GenericVector<T *>::data_[i]->Serialize(fp)) {
        return false;
      }
    }
    return true;
  }
  bool Serialize(TFile *fp) const {
    int32_t used = GenericVector<T *>::size_used_;
    if (fp->FWrite(&used, sizeof(used), 1) != 1) {
      return false;
    }
    for (int i = 0; i < used; ++i) {
      int8_t non_null = GenericVector<T *>::data_[i] != nullptr;
      if (fp->FWrite(&non_null, sizeof(non_null), 1) != 1) {
        return false;
      }
      if (non_null && !GenericVector<T *>::data_[i]->Serialize(fp)) {
        return false;
      }
    }
    return true;
  }
  // Reads a vector of (pointers to) classes to the given file. Assumes the
  // existence of bool T::DeSerialize(bool, Tfile*) const that returns false in
  // case of error. There is no Serialize for simple types, as you would have a
  // normal GenericVector of those.
  // If swap is true, assumes a big/little-endian swap is needed.
  // Also needs T::T(), as new T is used in this function.
  // Returns false in case of error.
  bool DeSerialize(bool swap, FILE *fp) {
    uint32_t reserved;
    if (fread(&reserved, sizeof(reserved), 1, fp) != 1) {
      return false;
    }
    if (swap) {
      Reverse32(&reserved);
    }
    // Arbitrarily limit the number of elements to protect against bad data.
    assert(reserved <= UINT16_MAX);
    if (reserved > UINT16_MAX) {
      return false;
    }
    GenericVector<T *>::reserve(reserved);
    truncate(0);
    for (uint32_t i = 0; i < reserved; ++i) {
      int8_t non_null;
      if (fread(&non_null, sizeof(non_null), 1, fp) != 1) {
        return false;
      }
      T *item = nullptr;
      if (non_null != 0) {
        item = new T;
        if (!item->DeSerialize(swap, fp)) {
          delete item;
          return false;
        }
        this->push_back(item);
      } else {
        // Null elements should keep their place in the vector.
        this->push_back(nullptr);
      }
    }
    return true;
  }

  // Sorts the items pointed to by the members of this vector using
  // t::operator<().
  void sort() {
    this->GenericVector<T *>::sort(&sort_ptr_cmp<T>);
  }
};

template <typename T>
void GenericVector<T>::init(int size) {
  size_used_ = 0;
  if (size <= 0) {
    data_ = nullptr;
    size_reserved_ = 0;
  } else {
    if (size < kDefaultVectorSize) {
      size = kDefaultVectorSize;
    }
    data_ = new T[size];
    size_reserved_ = size;
  }
  clear_cb_ = nullptr;
}

template <typename T>
GenericVector<T>::~GenericVector() {
  clear();
}

// Reserve some memory. If the internal array contains elements, they are
// copied.
template <typename T>
void GenericVector<T>::reserve(int size) {
  if (size_reserved_ >= size || size <= 0) {
    return;
  }
  if (size < kDefaultVectorSize) {
    size = kDefaultVectorSize;
  }
  T *new_array = new T[size];
  for (int i = 0; i < size_used_; ++i) {
    new_array[i] = data_[i];
  }
  delete[] data_;
  data_ = new_array;
  size_reserved_ = size;
}

template <typename T>
void GenericVector<T>::double_the_size() {
  if (size_reserved_ == 0) {
    reserve(kDefaultVectorSize);
  } else {
    reserve(2 * size_reserved_);
  }
}

// Resizes to size and sets all values to t.
template <typename T>
void GenericVector<T>::init_to_size(int size, const T &t) {
  reserve(size);
  size_used_ = size;
  for (int i = 0; i < size; ++i) {
    data_[i] = t;
  }
}

template <typename T>
void GenericVector<T>::resize(int size, const T &t) {
  init_to_size(size, t);
}

template <typename T>
T &GenericVector<T>::operator[](int index) const {
  assert(index >= 0 && index < size_used_);
  return data_[index];
}

template <typename T>
T &GenericVector<T>::back() const {
  assert(size_used_ > 0);
  return data_[size_used_ - 1];
}
// Returns the last object and removes it.
template <typename T>
T GenericVector<T>::pop_back() {
  assert(size_used_ > 0);
  return data_[--size_used_];
}

// Return the object from an index.
template <typename T>
void GenericVector<T>::set(const T &t, int index) {
  assert(index >= 0 && index < size_used_);
  data_[index] = t;
}

// Shifts the rest of the elements to the right to make
// space for the new elements and inserts the given element
// at the specified index.
template <typename T>
void GenericVector<T>::insert(const T &t, int index) {
  assert(index >= 0 && index <= size_used_);
  if (size_reserved_ == size_used_) {
    double_the_size();
  }
  for (int i = size_used_; i > index; --i) {
    data_[i] = data_[i - 1];
  }
  data_[index] = t;
  size_used_++;
}

// Removes an element at the given index and
// shifts the remaining elements to the left.
template <typename T>
void GenericVector<T>::remove(int index) {
  assert(index >= 0 && index < size_used_);
  for (int i = index; i < size_used_ - 1; ++i) {
    data_[i] = data_[i + 1];
  }
  size_used_--;
}

// Return the index of the T object.
template <typename T>
int GenericVector<T>::get_index(const T &object) const {
  for (int i = 0; i < size_used_; ++i) {
    if (object == data_[i]) {
      return i;
    }
  }
  return -1;
}

// Add an element in the array
template <typename T>
int GenericVector<T>::push_back(T object) {
  int index = 0;
  if (size_used_ == size_reserved_) {
    double_the_size();
  }
  index = size_used_++;
  data_[index] = std::move(object);
  return index;
}

template <typename T>
void GenericVector<T>::operator+=(const T &t) {
  push_back(t);
}

template <typename T>
GenericVector<T> &GenericVector<T>::operator+=(const GenericVector &other) {
  this->reserve(size_used_ + other.size_used_);
  for (unsigned i = 0; i < other.size(); ++i) {
    this->operator+=(other.data_[i]);
  }
  return *this;
}

template <typename T>
GenericVector<T> &GenericVector<T>::operator=(const GenericVector &other) {
  if (&other != this) {
    this->truncate(0);
    this->operator+=(other);
  }
  return *this;
}

// Clear the array, calling the callback function if any.
template <typename T>
void GenericVector<T>::clear() {
  if (size_reserved_ > 0 && clear_cb_ != nullptr) {
    for (int i = 0; i < size_used_; ++i) {
      clear_cb_(data_[i]);
    }
  }
  delete[] data_;
  data_ = nullptr;
  size_used_ = 0;
  size_reserved_ = 0;
  clear_cb_ = nullptr;
}

template <typename T>
void GenericVector<T>::delete_data_pointers() {
  for (int i = 0; i < size_used_; ++i) {
    delete data_[i];
  }
}

template <typename T>
bool GenericVector<T>::write(FILE *f, const std::function<bool(FILE *, const T &)> &cb) const {
  if (fwrite(&size_reserved_, sizeof(size_reserved_), 1, f) != 1) {
    return false;
  }
  if (fwrite(&size_used_, sizeof(size_used_), 1, f) != 1) {
    return false;
  }
  if (cb != nullptr) {
    for (int i = 0; i < size_used_; ++i) {
      if (!cb(f, data_[i])) {
        return false;
      }
    }
  } else {
    if (fwrite(data_, sizeof(T), size_used_, f) != unsigned_size()) {
      return false;
    }
  }
  return true;
}

template <typename T>
bool GenericVector<T>::read(TFile *f, const std::function<bool(TFile *, T *)> &cb) {
  int32_t reserved;
  if (f->FReadEndian(&reserved, sizeof(reserved), 1) != 1) {
    return false;
  }
  reserve(reserved);
  if (f->FReadEndian(&size_used_, sizeof(size_used_), 1) != 1) {
    return false;
  }
  if (cb != nullptr) {
    for (int i = 0; i < size_used_; ++i) {
      if (!cb(f, data_ + i)) {
        return false;
      }
    }
  } else {
    if (f->FReadEndian(data_, sizeof(T), size_used_) != static_cast<unsigned>(size_used_)) {
      return false;
    }
  }
  return true;
}

// Writes a vector of simple types to the given file. Assumes that bitwise
// read/write of T will work. Returns false in case of error.
template <typename T>
bool GenericVector<T>::Serialize(FILE *fp) const {
  if (fwrite(&size_used_, sizeof(size_used_), 1, fp) != 1) {
    return false;
  }
  if (fwrite(data_, sizeof(*data_), size_used_, fp) != unsigned_size()) {
    return false;
  }
  return true;
}
template <typename T>
bool GenericVector<T>::Serialize(TFile *fp) const {
  if (fp->FWrite(&size_used_, sizeof(size_used_), 1) != 1) {
    return false;
  }
  if (fp->FWrite(data_, sizeof(*data_), size_used_) != size_used_) {
    return false;
  }
  return true;
}

// Reads a vector of simple types from the given file. Assumes that bitwise
// read/write will work with ReverseN according to sizeof(T).
// Returns false in case of error.
// If swap is true, assumes a big/little-endian swap is needed.
template <typename T>
bool GenericVector<T>::DeSerialize(bool swap, FILE *fp) {
  uint32_t reserved;
  if (fread(&reserved, sizeof(reserved), 1, fp) != 1) {
    return false;
  }
  if (swap) {
    Reverse32(&reserved);
  }
  // Arbitrarily limit the number of elements to protect against bad data.
  assert(reserved <= UINT16_MAX);
  if (reserved > UINT16_MAX) {
    return false;
  }
  reserve(reserved);
  size_used_ = reserved;
  if (fread(data_, sizeof(T), size_used_, fp) != unsigned_size()) {
    return false;
  }
  if (swap) {
    for (int i = 0; i < size_used_; ++i) {
      ReverseN(&data_[i], sizeof(data_[i]));
    }
  }
  return true;
}
template <typename T>
bool GenericVector<T>::DeSerialize(TFile *fp) {
  uint32_t reserved;
  if (fp->FReadEndian(&reserved, sizeof(reserved), 1) != 1) {
    return false;
  }
  // Arbitrarily limit the number of elements to protect against bad data.
  const uint32_t limit = 50000000;
  assert(reserved <= limit);
  if (reserved > limit) {
    return false;
  }
  reserve(reserved);
  size_used_ = reserved;
  return fp->FReadEndian(data_, sizeof(T), size_used_) == size_used_;
}

// Writes a vector of classes to the given file. Assumes the existence of
// bool T::Serialize(FILE* fp) const that returns false in case of error.
// Returns false in case of error.
template <typename T>
bool GenericVector<T>::SerializeClasses(FILE *fp) const {
  if (fwrite(&size_used_, sizeof(size_used_), 1, fp) != 1) {
    return false;
  }
  for (int i = 0; i < size_used_; ++i) {
    if (!data_[i].Serialize(fp)) {
      return false;
    }
  }
  return true;
}

// Reads a vector of classes from the given file. Assumes the existence of
// bool T::Deserialize(bool swap, FILE* fp) that returns false in case of
// error. Also needs T::T() and T::T(constT&), as init_to_size is used in
// this function. Returns false in case of error.
// If swap is true, assumes a big/little-endian swap is needed.
template <typename T>
bool GenericVector<T>::DeSerializeClasses(TFile *fp) {
  int32_t reserved;
  if (fp->FReadEndian(&reserved, sizeof(reserved), 1) != 1) {
    return false;
  }
  T empty;
  init_to_size(reserved, empty);
  for (int i = 0; i < reserved; ++i) {
    if (!data_[i].DeSerialize(fp)) {
      return false;
    }
  }
  return true;
}

// This method clear the current object, then, does a shallow copy of
// its argument, and finally invalidates its argument.
template <typename T>
void GenericVector<T>::move(GenericVector<T> *from) {
  this->clear();
  this->data_ = from->data_;
  this->size_reserved_ = from->size_reserved_;
  this->size_used_ = from->size_used_;
  this->clear_cb_ = from->clear_cb_;
  from->data_ = nullptr;
  from->clear_cb_ = nullptr;
  from->size_used_ = 0;
  from->size_reserved_ = 0;
}

template <typename T>
void GenericVector<T>::sort() {
  sort(&sort_cmp<T>);
}

} // namespace tesseract

#endif // TESSERACT_CCUTIL_GENERICVECTOR_H_
