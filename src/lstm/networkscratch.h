///////////////////////////////////////////////////////////////////////
// File:        networkscratch.h
// Description: Scratch space for Network layers that hides distinction
//              between float/int implementations.
// Author:      Ray Smith
//
// (C) Copyright 2014, Google Inc.
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
///////////////////////////////////////////////////////////////////////

#ifndef TESSERACT_LSTM_NETWORKSCRATCH_H_
#define TESSERACT_LSTM_NETWORKSCRATCH_H_

#include <mutex>
#include <tesseract/genericvector.h>
#include "matrix.h"
#include "networkio.h"

namespace tesseract {

// Generic scratch space for network layers. Provides NetworkIO that can store
// a complete set (over time) of intermediates, and GenericVector<float>
// scratch space that auto-frees after use. The aim here is to provide a set
// of temporary buffers to network layers that can be reused between layers
// and don't have to be reallocated on each call.
class NetworkScratch {
 public:
  NetworkScratch() : int_mode_(false) {}
  ~NetworkScratch() = default;

  // Sets the network representation. If the representation is integer, then
  // default (integer) NetworkIOs are separated from the always-float variety.
  // This saves memory by having separate int-specific and float-specific
  // stacks. If the network representation is float, then all NetworkIOs go
  // to the float stack.
  void set_int_mode(bool int_mode) {
    int_mode_ = int_mode;
  }

  // Class that acts like a NetworkIO (by having an implicit cast operator),
  // yet actually holds a pointer to NetworkIOs in the source NetworkScratch,
  // and knows how to unstack the borrowed pointers on destruction.
  class IO {
   public:
    // The NetworkIO should be sized after construction.
    IO(const NetworkIO& src, NetworkScratch* scratch)
      : int_mode_(scratch->int_mode_ && src.int_mode()),
        scratch_space_(scratch) {
      network_io_ = int_mode_ ? scratch_space_->int_stack_.Borrow()
                              : scratch_space_->float_stack_.Borrow();
    }
    // Default constructor for arrays. Use one of the Resize functions
    // below to initialize and size.
    IO() : int_mode_(false), network_io_(nullptr), scratch_space_(nullptr) {}

    ~IO() {
      if (scratch_space_ == nullptr) {
        ASSERT_HOST(network_io_ == nullptr);
      } else if (int_mode_) {
        scratch_space_->int_stack_.Return(network_io_);
      } else {
        scratch_space_->float_stack_.Return(network_io_);
      }
    }
    // Resizes the array (and stride), avoiding realloc if possible, to the
    // size from various size specs:
    // Same time size, given number of features.
    void Resize(const NetworkIO& src, int num_features,
                NetworkScratch* scratch) {
      if (scratch_space_ == nullptr) {
        int_mode_ = scratch->int_mode_ && src.int_mode();
        scratch_space_ = scratch;
        network_io_ = int_mode_ ? scratch_space_->int_stack_.Borrow()
                                : scratch_space_->float_stack_.Borrow();
      }
      network_io_->Resize(src, num_features);
    }
    // Resizes to a specific size as a temp buffer. No batches, no y-dim.
    void Resize2d(bool int_mode, int width, int num_features,
                  NetworkScratch* scratch) {
      if (scratch_space_ == nullptr) {
        int_mode_ = scratch->int_mode_ && int_mode;
        scratch_space_ = scratch;
        network_io_ = int_mode_ ? scratch_space_->int_stack_.Borrow()
                                : scratch_space_->float_stack_.Borrow();
      }
      network_io_->Resize2d(int_mode, width, num_features);
    }
    // Resize forcing a float representation with the width of src and the given
    // number of features.
    void ResizeFloat(const NetworkIO& src, int num_features,
                     NetworkScratch* scratch) {
      if (scratch_space_ == nullptr) {
        int_mode_ = false;
        scratch_space_ = scratch;
        network_io_ = scratch_space_->float_stack_.Borrow();
      }
      network_io_->ResizeFloat(src, num_features);
    }

    // Returns a ref to a NetworkIO that enables *this to be treated as if
    // it were just a NetworkIO*.
    NetworkIO& operator*() {
      return *network_io_;
    }
    NetworkIO* operator->() {
      return network_io_;
    }
    operator NetworkIO*() {
      return network_io_;
    }

   private:
    // True if this is from the always-float stack, otherwise the default stack.
    bool int_mode_;
    // The NetworkIO that we have borrowed from the scratch_space_.
    NetworkIO* network_io_;
    // The source scratch_space_. Borrowed pointer, used to free the
    // NetworkIO. Don't delete!
    NetworkScratch* scratch_space_;
  };  // class IO.

  // Class that acts like a fixed array of float, yet actually uses space
  // from a GenericVector<float> in the source NetworkScratch, and knows how
  // to unstack the borrowed vector on destruction.
  class FloatVec {
   public:
    // The array will have size elements in it, uninitialized.
    FloatVec(int size, NetworkScratch* scratch)
      : vec_(nullptr), scratch_space_(scratch) {
      Init(size, scratch);
    }
    // Default constructor is for arrays. Use Init to setup.
    FloatVec() : vec_(nullptr), data_(nullptr), scratch_space_(nullptr) {}
    ~FloatVec() {
      if (scratch_space_ != nullptr) scratch_space_->vec_stack_.Return(vec_);
    }

    void Init(int size, NetworkScratch* scratch) {
      if (scratch_space_ != nullptr && vec_ != nullptr)
        scratch_space_->vec_stack_.Return(vec_);
      scratch_space_ = scratch;
      vec_ = scratch_space_->vec_stack_.Borrow();
      vec_->resize_no_init(size);
      data_ = &(*vec_)[0];
    }

    // Use the cast operator instead of operator[] so the FloatVec can be used
    // as a double* argument to a function call.
    operator double*() const { return data_; }
    double* get() { return data_; }

   private:
    // Vector borrowed from the scratch space. Use Return to free it.
    GenericVector<double>* vec_;
    // Short-cut pointer to the underlying array.
    double* data_;
    // The source scratch_space_. Borrowed pointer, used to free the
    // vector. Don't delete!
    NetworkScratch* scratch_space_;
  };  // class FloatVec

  // Class that acts like a 2-D array of double, yet actually uses space
  // from the source NetworkScratch, and knows how to unstack the borrowed
  // array on destruction.
  class GradientStore {
   public:
    // Default constructor is for arrays. Use Init to setup.
    GradientStore() : array_(nullptr), scratch_space_(nullptr) {}
    ~GradientStore() {
      if (scratch_space_ != nullptr) scratch_space_->array_stack_.Return(array_);
    }

    void Init(int size1, int size2, NetworkScratch* scratch) {
      if (scratch_space_ != nullptr && array_ != nullptr)
        scratch_space_->array_stack_.Return(array_);
      scratch_space_ = scratch;
      array_ = scratch_space_->array_stack_.Borrow();
      array_->Resize(size1, size2, 0.0);
    }

    // Accessors to get to the underlying TransposedArray.
    TransposedArray* get() const { return array_; }
    const TransposedArray& operator*() const { return *array_; }

   private:
    // Array borrowed from the scratch space. Use Return to free it.
    TransposedArray* array_;
    // The source scratch_space_. Borrowed pointer, used to free the
    // vector. Don't delete!
    NetworkScratch* scratch_space_;
  };  // class GradientStore

  // Class that does the work of holding a stack of objects, a stack pointer
  // and a vector of in-use flags, so objects can be returned out of order.
  // It is safe to attempt to Borrow/Return in multiple threads.
  template<typename T> class Stack {
   public:
    Stack() : stack_top_(0) {
    }

    // Lends out the next free item, creating one if none available, sets
    // the used flags and increments the stack top.
    T* Borrow() {
      std::lock_guard<std::mutex> lock(mutex_);
      if (stack_top_ == stack_.size()) {
        stack_.push_back(new T);
        flags_.push_back(false);
      }
      flags_[stack_top_] = true;
      return stack_[stack_top_++];
    }
    // Takes back the given item, and marks it free. Item does not have to be
    // the most recently lent out, but free slots don't get re-used until the
    // blocking item is returned. The assumption is that there will only be
    // small, temporary variations from true stack use. (Determined by the order
    // of destructors within a local scope.)
    void Return(T* item) {
      std::lock_guard<std::mutex> lock(mutex_);
      // Linear search will do.
      int index = stack_top_ - 1;
      while (index >= 0 && stack_[index] != item) --index;
      if (index >= 0) flags_[index] = false;
      while (stack_top_ > 0 && !flags_[stack_top_ - 1]) --stack_top_;
    }

   private:
    PointerVector<T> stack_;
    GenericVector<bool> flags_;
    int stack_top_;
    std::mutex mutex_;
  };  // class Stack.

 private:
  // If true, the network weights are int8_t, if false, float.
  bool int_mode_;
  // Stacks of NetworkIO and GenericVector<float>. Once allocated, they are not
  // deleted until the NetworkScratch is deleted.
  Stack<NetworkIO> int_stack_;
  Stack<NetworkIO> float_stack_;
  Stack<GenericVector<double> > vec_stack_;
  Stack<TransposedArray> array_stack_;
};

}  // namespace tesseract.

#endif  // TESSERACT_LSTM_NETWORKSCRATCH_H_
