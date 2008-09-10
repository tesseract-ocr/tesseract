// Copyright 2006 Google Inc.
// All Rights Reserved.
// Author: <renn@google.com> (Marius Renn)
//
// This file contains the Cluster class, which combines a series of Shapes to 
// a larger unit. 
// In a text detection scenario, Shapes would represent the characters, and
// Clusters would represent the words.
//
#ifndef HELIUM_CLUSTER_H__
#define HELIUM_CLUSTER_H__

#include "box.h"

namespace helium {

class Shape;

// The Cluster class is very simple, as it contains only a pointer to the 
// first Shape of the cluster it represents. Further Shapes are linked to 
// by the Shape itself (left_neighbor and right_neighbor members), making the
// Cluster class similar to a doubly-linked list.
class Cluster {
  public:
    // Constructor for a Cluster that begins at the specified Shape (the
    // specified Shape is the left-most Shape of the cluster). Sets the 
    // needs_clustering flags of all the linked Shapes to false.
    Cluster(Shape* shape);

    // Returns the first Shape of the cluster.
    inline Shape* first() const {
      return first_;
    }
    
    // Calculates the bounds of the Cluster by going through all the Shapes
    // and returning the minimum enclosing box.
    Box CalculateBounds() const;
    
  private:
    Shape* first_;  // Pointer to the first (left-most) Shape of the Cluster.
};

} // namespace

#endif  // HELIUM_CLUSTER_H__
