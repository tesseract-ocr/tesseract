// Copyright 2006 Google Inc.
// All Rights Reserved.
// Author: <renn@google.com> (Marius Renn)

#include "color.h"
#include "labeler.h"
#include "trace.h"
#include "unionfind.h"

using namespace helium;

Image Labeler::Label(const Mask& mask) {
  // Create new image
  Image labels(mask.width(), mask.height());
  
  // Clear image
  labels.Clear();
  
  // Label the regions
  LabelAllRegions(labels, mask);
  return labels;
}

void Labeler::LabelAllRegions(Image& labels, const Mask& edges) {
  // Make sure we have room for a border around the labels
  ASSERT(labels.width() == edges.width());
  ASSERT(labels.height() == edges.height());
  
  Color* labels_ptr = labels.Access(1, 1);
  bool* edge_ptr = edges.Access(1, 1);
  
  unsigned areaid = 1;
  //int label_vstep = labels.width() - edges.width();
  
  Unions blobs;
  
  // First pass
  Point p;
  for (p.y = 1; p.y < edges.height() - 1; p.y++) {
    for (p.x = 1; p.x < edges.width() - 1; p.x++) {
      if (!(*edge_ptr)) {
        Color fill_left = *(labels_ptr - 1);
        Color fill_up = *(labels_ptr - labels.width());
        
        // 0 values are don't care values
        if (fill_left == 0) 
          fill_left = fill_up;
        else if (fill_up == 0) 
          fill_up = fill_left;
        
        if (fill_left == fill_up) {
          if(fill_up == 0) fill_up = areaid++;  // New fill id
        } else {
          blobs.Unify(fill_left, fill_up);
        }
        *labels_ptr = fill_up;
      } else *labels_ptr = 0;
      labels_ptr++;
      edge_ptr++;
    }
    labels_ptr += 2;
    edge_ptr += 2;
  }
  
  blobs.LabelRoots();
  
  // Second pass
  labels_ptr = labels.Access(1, 1);
  for (p.y = 1; p.y < edges.height() - 1; p.y++) {
    for (p.x = 1; p.x < edges.width() - 1; p.x++) {
      *labels_ptr = blobs.Find(*labels_ptr);
      labels_ptr++;
    }
    labels_ptr += 2;
  }
}
