// Copyright 2006 Google Inc.
// All Rights Reserved.
// Author: <renn@google.com> (Marius Renn)

#include "helium_cluster.h"
#include "shape.h"

using namespace helium;

Cluster::Cluster(Shape* shape) : first_(shape) {
  ASSERT(!shape->left_neighbor());
  while (shape) {
    shape->set_needs_clustering(false);
    shape = shape->right_neighbor();
  }
}

Box Cluster::CalculateBounds() const {
  ASSERT(first_);
  
  // Unify all bounding boxes of contained shapes
  Box total_bounds = first_->bounds();
  Shape* current = first_->right_neighbor();
  while (current) {
    total_bounds = MinEnclosingBox(total_bounds, current->bounds());
    current = current->right_neighbor();
  }
  return total_bounds;
}
