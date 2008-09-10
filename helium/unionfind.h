// Copyright 2006 Google Inc.
// All Rights Reserved.
// Author: <renn@google.com> (Marius Renn)
//
// This file contains the Unions class, used to partition elements into 
// non-overlapping sets. This is implemented using a Union-Find algorithm, 
// and a disjoint-set data structure.
#ifndef HELIUM_UNIONFIND_H__
#define HELIUM_UNIONFIND_H__

// Local includes
#include "array.h"
#include "types.h"

namespace helium {

struct UnionElement {
  uint32 parent;
  uint16 value;
  
  UnionElement() : parent(0), value(0) {
  }
};

// The Unions class implements a simple Union Find algorithm. 
class Unions {
  public:
    Unions();
    
    // This specifies that a and b belong to the same class.
    void Unify(uint32 a, uint32 b);
    
    // Find the class identifier for the given value.
    uint16 Find(uint32 value);
    
    // When no more elements are added, label the roots of the UnionFind tree
    // with this method.
    void LabelRoots();
    
  private:
    Array<UnionElement> parents_;
    unsigned max_root_;
    uint32 FindParentOrAdd(uint32 value, unsigned& steps);
};

} // namespace

#endif  // HELIUM_UNIONFIND_H__
