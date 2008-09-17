// Copyright 2006 Google Inc.
// All Rights Reserved.
// Author: <renn@google.com> (Marius Renn)

// Local includes
#include "debugging.h"
#include "unionfind.h"

using namespace helium;

Unions::Unions() 
  : parents_(16), max_root_(0) {
}

void Unions::Unify(uint32 a, uint32 b) {
  ASSERT((a != 0) && (b != 0));   // zero is reserved for no-value
  
  if (a == b) return;
  
  unsigned steps1, steps2;
  uint32 r1 = FindParentOrAdd(a, steps1);
  uint32 r2 = FindParentOrAdd(b, steps2);
  
  if (r1 == r2) return; // Already in the same class
  
  if(steps1 < steps2) 
    parents_.ValueAt(r1).parent = r2;
  else
    parents_.ValueAt(r2).parent = r1;
}

uint16 Unions::Find(uint32 value) {
  if (value == 0) return 0;
  if (value >= parents_.size()) {
    int max_id = max_root_;
    parents_.Resize(value + 1);
    parents_.ValueAt(value).value = max_id;
    max_root_++;
    return max_id;
  }
  uint32 next_index, cur_index = value;
  while ((next_index = parents_.ValueAt(cur_index).parent) != 0)
    cur_index = next_index;
  return parents_.ValueAt(cur_index).value;
}

uint32 Unions::FindParentOrAdd(uint32 value, unsigned& steps) {
  // Special case: Value not in tree yet
  if (value >= parents_.size()) {
    parents_.Resize(value + 1);
    steps = 0;
    return value;
  }
  uint32 next_node;
  for (steps = 0; (next_node = parents_.ValueAt(value).parent) != 0; steps++) 
    value = next_node;
  
  return value;
}

void Unions::LabelRoots() {
  max_root_ = 0;
  for (unsigned r = 0; r < parents_.size(); r++) {
    if (!parents_.ValueAt(r).parent) parents_.ValueAt(r).value = max_root_++;
  }
}
