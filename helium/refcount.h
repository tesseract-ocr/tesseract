// Copyright 2006 Google Inc.
// All Rights Reserved.
// Author: <renn@google.com> (Marius Renn)
//
// Some objects, like Images and Strings, are passed around extensively during
// program flow, making it difficult to ascertain ownership. For these a
// reference counting base-class (ReferenceCounted) has been implemented, to
// automate the process of ownership transfer.
//
#ifndef HELIUM_REFCOUNT_H__
#define HELIUM_REFCOUNT_H__

namespace helium {

// ReferenceCounted is the base class for all classes that require automated
// member deallocation by reference counting. It should provide a simple way
// to manage objects with difficult ownership requirements, without the need
// to wrap them in smart pointers. The trade-off for this is, that subclasses
// must make sure to implement various methods in a specific way.
// Two methods provide control over object ownership: Release() and Retain(). 
// Release decrements the reference counter and calls DeleteData(),
// iff the counter has reached 0. Retain increments the counter. These methods
// are called during construction and deconstruction, so that objects
// on the stack are retained and released automatically.
// Since the destructor does not necessarily delete the contents of the object,
// subclassers must be sure to implement the destructor in a special way. More
// specifically, they must ask for permission (by calling ShouldDelete()) 
// whether or not to call DeleteData(). The method ShouldDelete() makes sure, 
// that the members are deallocated exactly one time.
class ReferenceCounted {
  public:
    // Constructor. Initializes reference counter to 1. 
    ReferenceCounted();
    
    // Copy Constructor. Both objects share the same reference counter, which
    // is retained to reflect the additional reference by the copy.
    ReferenceCounted(const ReferenceCounted& other);
    
    // Deconstructor. Releases the receiver. Subclasses MUST implement this
    // method by calling DeleteData() iff ShouldDelete() returns true 
    // (otherwise do nothing).
    // Note that when calling DeleteData() from the destructor, a non-virtual
    // call is made, even if the modifier states otherwise. This does not
    // inhibit us however, as the most specialized class will be deleted first
    // anyway, resulting in the most specialized method of DeleteData() to 
    // be called.
    virtual ~ReferenceCounted();
    
    // Assignment operator. First, the receiver is released. Once the reference
    // counter has been transferred, the receiver is retained again.
    void operator=(const ReferenceCounted& other);
    
    // Accessor to the reference counter.
    inline int ref_count() const {
      return *ref_count_;
    }
    
    // The Retain method is called to signal new ownership of an object 
    // (incrementing the reference counter). Objects on the stack are
    // retained automatically, and for these this method should not be called.
    // Objects that have been deallocated already, or are in the process of
    // becoming deallocated may not be retained. Doing so will result in an
    // assertion failure.
    void Retain();
    
    // The Release method is called to signal that an object is no longer in 
    // use, and thus no longer requires its referenced data. Objects on the
    // stack are released automatically. If you would like to pre-release an
    // object on the stack before it goes out of scope, it is safe to do so,
    // as it will only be released once (the going-out-of-scope release will
    // be ignored). However you must not access released objects in any way,
    // so be careful to release objects only, when you are sure they will no
    // longer be used.
    void Release();
    
    // Static method to return the number of reference counted allocations. 
    // This is NOT the number of objects, that reference these allocations!
    // Useful for memory debugging, and verifying that this method returns 0
    // on program termination.
    inline static int number_of_allocations() {
      return number_of_allocations_;
    }
    
  protected:
    // This method is called when the reference counter has reached 0, and all
    // referenced data should be deleted. Subclasses must deallocate any 
    // necessary data here, and then call the superclass's implementation of
    // DeleteData(). 
    // ReferenceCounted's implementation simply deallocates the reference
    // counter.
    virtual void DeleteData();
    
    // ShouldDelete specifies whether or not a subclasss should call 
    // DeleteData() in their destructors. 
    // It makes sure that the reference counter is indeed about
    // to reach 0, and that the data has not been deleted already.
    bool ShouldDelete();
    
    // Subclasses must call Realloc, whenever they reference new data, and
    // discard the old one. 
    void Realloc();
    
  private:
    // The pointer to the reference counter.
    int* ref_count_;
    
    // The number of allocations of reference counted data.
    static int number_of_allocations_;
    
};

} // namespace

#endif // HELIUM_REFCOUNT_H__
