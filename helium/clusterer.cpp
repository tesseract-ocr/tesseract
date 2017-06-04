// Copyright 2006 Google Inc.
// All Rights Reserved.
// Author: <renn@google.com> (Marius Renn)

#include "helium_cluster.h"
#include "clusterer.h"
#include "clustervalidator.h"
#include "debugging.h"
#include "helium_image.h"
#include "shape.h"

using namespace helium;

const int kMaxInt = ~(0x01 << (sizeof(int)*8 - 1));

Clusterer::Clusterer() : clusters_(4) {
}

Clusterer::~Clusterer() {
  ClearClusters();
}

void Clusterer::ClearClusters() {
  for (unsigned i = 0; i < clusters_.size(); i++) delete clusters_.ValueAt(i);
  clusters_.DeleteValues();
}

void Clusterer::ClusterShapes(const Array<Shape*>& shapes,
                              ClusterValidator& validator){
  SetNeighbors(shapes, validator);
  AddClusters(shapes, validator);
}

void Clusterer::SetNeighbors(const Array<Shape*>& shapes,
                             ClusterValidator& validator) {
  for (unsigned i = 0; i < shapes.size(); i++) {
    Shape* left = shapes.ValueAt(i);
    if (!validator.ValidateShape(*left)) continue;
    
    Box left_box = left->bounds();
               
    // Find best neighbor
    unsigned best_distance = kMaxInt;
    Shape* best_neighbor = NULL;
    for (unsigned j = 0; j < shapes.size(); j++) {
      if (j == i) continue;
      
      Shape* right = shapes.ValueAt(j);
      if (!validator.ValidateShape(*right)) continue;
      
      Box right_box = right->bounds();
      
      if (right_box.center().x <= left_box.center().x) continue;
      
      bool qualifies = validator.ValidatePair(*left, *right);
      if (qualifies) {   
        Point left_rb(left_box.right(), left_box.bottom());
        Point right_lb(right_box.left(), right_box.bottom());
        unsigned distance = Distance(left_rb, right_lb);
        
        if (distance < best_distance) {
          best_distance = distance;
          best_neighbor = right;
        }
      }
    }
    
    // Did we find a neighbor?
    if (best_neighbor) {
      if (best_neighbor->left_neighbor()) // Remove old link, if there
        best_neighbor->left_neighbor()->set_right_neighbor(NULL);
      left->set_right_neighbor(best_neighbor);
      best_neighbor->set_left_neighbor(left);
      
      left->set_needs_clustering(true);
      best_neighbor->set_needs_clustering(true);
    }
  }
}

void Clusterer::AddClusters(const Array<Shape*>& shapes,
                            ClusterValidator& validator) {
  for (unsigned i = 0; i < shapes.size(); i++) {
    Shape* cur_shape = shapes.ValueAt(i);
    
    if (!cur_shape->needs_clustering()) continue;

    while (cur_shape->left_neighbor()) 
      cur_shape = cur_shape->left_neighbor();
    
    Shape *first_shape = validator.ValidateClusterStartingAt(*cur_shape);
    
    if (first_shape) {
      Cluster* cluster = new Cluster(first_shape);
      clusters_.Add(cluster);
    }
  }
}

void Clusterer::DrawClusterBounds(Image& image) const {
  Color white = MakeColor(255, 0, 0);
  for (unsigned i = 0; i < clusters_.size(); ++i) {
    Cluster* cur_cluster = clusters_.ValueAt(i);
    const Box& bounds = cur_cluster->CalculateBounds();
    image.DrawBox(bounds, white);
  }
}
