// Copyright 2006 Google Inc.
// All Rights Reserved.
// Author: <renn@google.com> (Marius Renn)
//
// This file contains the Array class, used to store a dynamic number of values
// of any type.
//
#ifndef HELIUM_ARRAY_H__
#define HELIUM_ARRAY_H__

// Local includes
#include "debugging.h"

// C includes
#include <string.h>

namespace helium {

// The template class Array provides a safe and dynamic way to store numerous
// values. Internally it uses a C-Array, that is reallocated once the number
// of values exceeds its size. Copying does not occur via memcpy(...), but
// by assigning the values one by one. This has the advantage that C++ objects
// behave correctly in an Array, making it possible to properly store reference
// counted objects in Arrays.
template<typename T>
class Array {
  public:
    Array();

    // Initialize an Array with the given capacity.
    explicit Array(unsigned initial_capacity);

    // Initialize an Array with the specified data and size. Note that Array
    // owns the data!
    Array(T* data, unsigned size);

    // Deconstructor deletes the internal array.
    ~Array();

    // Add a value to the array. If the increased size exceeds the capacity,
    // a new array will be allocated with twice the capacity of the old one,
    // and the values copied to the new array.
    void Add(T value);

    // Get const access to the value at the specified index. Bounds are
    // checked in debug mode.
    inline const T& ValueAt(unsigned index) const {
      ASSERT_IN_DEBUG_MODE(index < size_);
      return values_[index];
    }

    // Get non-const access to the value at the specified index. Bounds are
    // checked in debug mode.
    inline T& ValueAt(unsigned index) {
      ASSERT_IN_DEBUG_MODE(index < size_);
      return values_[index];
    }

    // Returns the size of the array.
    inline unsigned size() const {
      return size_;
    }

    // Returns a pointer to the values. Keep in mind that the data is owned
    // by the Array.
    inline T* values() const {
      return values_;
    }

    // Returns the amount of memory the internal array is using, in bytes.
    // Used for debugging and memory management purposes.
    inline unsigned MemoryUsage() const {
      return capacity_ * sizeof(T);
    }

    // Returns an Array object that has a copy of the receiver's internal
    // array. This method is expensive, and should be called only when
    // absolutely necessary.
    Array* Copy() const;

    // Resizes the array to the new size. This will always copy the elements
    // to a new array of the given size, so use this method wisely!
    void Resize(unsigned new_size);

    // Remove the specified number of elements from the end of the Array. This
    // method does not actually shrink the allocated array, so this is not a
    // way to save memory. Bounds are checked in debug mode.
    inline void RemoveLast(unsigned elements) {
      ASSERT_IN_DEBUG_MODE(elements <= size_);
      size_ -= elements;
    }

    // Clears the array by setting the size to 0. This method does not perform
    // any sort of deallocation. Use DeleteValues() for this.
    inline void Clear() {
      size_ = 0;
    }

    // Delete the internal array of values. The size of the Array will be 0
    // after this call. Adding elements to an Array that has been deallocated
    // with DeleteValues() is allowed, but will allocate a new internal array.
    void DeleteValues();

  protected:
    unsigned size_;
    unsigned capacity_;
    T* values_;

    // This protected copy constructor is available for subclasses that want
    // to implement their own Copy() function.
    explicit Array(const Array& other);

  private:
    // Assignment of Arrays is not allowed!
    void operator=(const Array& other);

    // This function is called internally when adding an element to the Array.
    // It checks whether the Array needs resizing, and if yes, allocates a new
    // internal array, and copies the values from the old one.
    void ResizeIfNecessary();
};

// Template implementations ----------------------------------------------------
template<typename T>
Array<T>::Array()
  : size_(0), capacity_(0), values_(NULL) {
}

template<typename T>
Array<T>::Array(unsigned initial_capacity)
  : size_(0), capacity_(initial_capacity), values_(NULL) {
  if (initial_capacity > 0) values_ = new T[initial_capacity];
}

template<typename T>
Array<T>::Array(T* data, unsigned size)
  : size_(size), capacity_(size), values_(data) {
  ASSERT(values_);
}

template<typename T>
void Array<T>::Add(T value) {
  ResizeIfNecessary();
  values_[size_] = value;
  ++size_;
}

template<typename T>
Array<T>::~Array() {
  DeleteValues();
}

template<typename T>
void Array<T>::DeleteValues() {
  delete[] values_;
  values_ = NULL;
  size_ = 0;
  capacity_ = 0;
}

template<typename T>
Array<T>* Array<T>::Copy() const {
  return new Array<T>(*this);
}

template<typename T>
void Array<T>::Resize(unsigned new_size) {
  T* new_values = new T[new_size + 1];
  unsigned min_size = (size_ < new_size) ? size_ : new_size;
  for (unsigned i = 0; i < min_size; i++) new_values[i] = values_[i];
  delete[] values_;
  values_ = new_values;
  capacity_ = size_ = new_size;
}

template<typename T>
Array<T>::Array(const Array<T>& other)
  : size_(other.size_), capacity_(other.size_), values_(new T[other.size_]) {
  for (unsigned i = 0; i < size_; i++) values_[i] = other.values_[i];
}

template<typename T>
void Array<T>::ResizeIfNecessary() {
  if (size_ >= capacity_) {
    if (capacity_ == 0) capacity_ = 1;
    T* new_values = new T[capacity_ * 2];
    for (unsigned i = 0; i < size_; i++) new_values[i] = values_[i];
    delete[] values_;
    values_ = new_values;
    capacity_ *= 2;
  }
}

} // namespace

#endif  // HELIUM_ARRAY_H__
