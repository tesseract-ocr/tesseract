// Copyright 2006 Google Inc.
// All Rights Reserved.
// Author: <renn@google.com> (Marius Renn)
//

// Local includes
#include "debugging.h"
#include "refcount.h"

// C includes
#include <stddef.h>

using namespace helium;

int ReferenceCounted::number_of_allocations_ = 0;

ReferenceCounted::ReferenceCounted() : ref_count_(new int(1)) {
  number_of_allocations_++;
}

ReferenceCounted::ReferenceCounted(const ReferenceCounted& other) 
  : ref_count_(other.ref_count_) {
  (*ref_count_)++; 
}

ReferenceCounted::~ReferenceCounted() {
  if (ShouldDelete())
    DeleteData();
  else if (ref_count_)
    (*ref_count_)--;
}

void ReferenceCounted::Retain() {
  ASSERT(ref_count_ && (*ref_count_) > 0);
  (*ref_count_)++;
}

void ReferenceCounted::Release() {
  if (!ref_count_) return;
  (*ref_count_)--;
  if ((*ref_count_) == 0) DeleteData();
}

bool ReferenceCounted::ShouldDelete() {
  // Assert that we still have a reference counter, and that it will reach
  // 0 due to this object being deconstructed.
  if (ref_count_ && (*ref_count_ == 1)) {
    *ref_count_ = 0;  // Mark data as deleted
    return true;
  }
  return false;
}

void ReferenceCounted::Realloc() {
  Release();                  // Release old data
  ref_count_ = new int(1);    // Re-init reference counter
  number_of_allocations_++;   // Count the new allocation
}

void ReferenceCounted::DeleteData() {
  delete ref_count_;          // Delete and invalidate reference counter
  ref_count_ = NULL;
  number_of_allocations_--;  
}

void ReferenceCounted::operator=(const ReferenceCounted& other) {
  Release();  // Release old object
  ref_count_ = other.ref_count_;
  Retain();   // Retain new object
}
