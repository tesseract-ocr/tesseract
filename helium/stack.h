// Copyright 2006 Google Inc.
// All Rights Reserved.
// Author: <renn@google.com> (Marius Renn)
//
// This file contains the Stack class, which is used to provide a more 
// stack-like interface to Arrays. This is mainly used by Tracers to store
// alternate routes.
//
#ifndef HELIUM_STACK_H__
#define HELIUM_STACK_H__

#include "array.h"

namespace helium {

// The Stack class simply provides a different interface to an Array, that more
// resembles stack functionality.
template<typename T>
class Stack {
  public:
    Stack() : values_(4) {
    }
    
    // Clears the Stack.
    inline void Clear() {
      values_.Clear();
    }
    
    // Push an element onto the Stack.
    inline void Push(T loc) {
      values_.Add(loc);
    }
    
    // Pop an element off of the top of the Stack.
    inline T Pop() {
      T value = values_.ValueAt(values_.size() - 1);
      values_.RemoveLast(1);
      return value;
    }
  
    // Return the top element on the Stack.
    inline const T& Top() const {
      return values_.ValueAt(values_.size() - 1);
    }
    
    // Returns true if there is no element on the Stack.
    inline bool Empty() const {
      return (values_.size() == 0);
    }
    
    // Returns the size of the Stack.
    inline unsigned Size() const {
      return values_.size();
    }
    
    // Returns the amount of memory the Stack uses in bytes. This is good for
    // debugging.
    inline unsigned MemoryUsage() const {
      return values_.MemoryUsage();
    }
    
  private:
    Array<T> values_;
    
    // Disallow copying
    Stack(const Stack&);
    void operator=(const Stack&);
};

} // namespace

#endif  // HELIUM_STACK_H__
