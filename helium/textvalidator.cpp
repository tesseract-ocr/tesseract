// Copyright 2006 Google Inc.
// All Rights Reserved.
// Author: <renn@google.com> (Marius Renn)

// Local includes
#include "debugging.h"
#include "box.h"
#include "color.h"
#include "mathfunctions.h"
#include "shape.h"
#include "textvalidator.h"
#include "traceclassifier.h"

// C includes
#include <math.h>
#include <stdlib.h>

using namespace helium;

const unsigned kMaxHueDiff = 15;
const unsigned kMaxLightnessDiff = 64;
const unsigned kMaxVariance = 96;
const float kMaxSlopeDiff = 0.48;

bool TextValidator::ValidateShape(const Shape& shape) {
  return (shape.trace_class() == TRACECLASS_TEXT) 
      && (Lightness(shape.histogram().Variance()) <= kMaxVariance);
}

bool TextValidator::ValidatePair(const Shape& left, const Shape& right) {
  // Size compatibility
  const Box& left_box = left.bounds();
  const Box& right_box = right.bounds();
  float left_height = static_cast<float>(left_box.height());
  float right_height = static_cast<float>(right_box.height());
  
  float height_ratio = left_height / right_height; 
  if ((height_ratio < 0.5) || (height_ratio > 2.0)) return false;
  
  // Distance compatibility
  float distance = Distance(left_box.mid_right(), right_box.mid_left());
  float rel_dist_left = distance / left_height;
  float rel_dist_right = distance / right_height;
  
  if ((rel_dist_left > 2.0) || (rel_dist_right > 2.0)) return false;
  
  // Y Overlap
  if ((right_box.top() > left_box.bottom())
  || (left_box.top() > right_box.bottom()))
    return false;
  
  // Enclosure (Letter may not enclose the other!)
  if (Encloses(left_box, right_box) || Encloses(right_box, left_box)) 
    return false;
  
  // Color compatibility
  Color left_color = left.histogram().Expected();
  Color right_color = right.histogram().Expected();
  
  if ((Saturation(left_color) >= 72) || (Saturation(right_color) >= 72)) {
    // Saturated enough to compare hue
    if (HueDistance(left_color, right_color) > kMaxHueDiff) return false;
  }
  
  // Compare lightness
  if  (abs(Lightness(left_color) - Lightness(right_color)) > kMaxLightnessDiff) 
    return false;
  
  // Competition
  if (right.left_neighbor()) {
    float old_dist = Distance(right.left_neighbor()->bounds().mid_right(), 
                              right_box.mid_left());
    if (old_dist < distance) return false;
  }
  
  return true;
}

void TextValidator::RemoveLeftShape(Shape& shape) {
  ASSERT(!shape.left_neighbor());
  
  shape.right_neighbor()->set_left_neighbor(NULL);
  shape.set_right_neighbor(NULL);
  
  shape.set_needs_clustering(false);
}

void TextValidator::RemoveRightShapesFrom(Shape& shape) {
  for(Shape* cur_shape = &shape;
      cur_shape;
      cur_shape = cur_shape->right_neighbor()) {
    // Remove all links to shapes to the right
    if (cur_shape->left_neighbor()) {
      cur_shape->left_neighbor()->set_right_neighbor(NULL);
      cur_shape->set_left_neighbor(NULL);
    }
    cur_shape->set_needs_clustering(false);
  }
}

Shape* TextValidator::ValidateSlopesOfCluster(Shape& shape, 
                                              unsigned cluster_size) {
  Shape* first_shape = &shape;
  Shape* cur_shape = NULL;
  Shape* next_shape = NULL;
  
  // Get all slopes and compute the median
  float slopes[cluster_size - 1];
  cur_shape = first_shape;
  for (unsigned i = 0; cur_shape->right_neighbor(); ++i) {
    slopes[i] = cur_shape->SlopeTo(*cur_shape->right_neighbor());
    cur_shape = cur_shape->right_neighbor();
  }
  float median_slope = Median(slopes, cluster_size - 1);
  
  // Filter all shapes from the left, that are not on the slope
  for (cur_shape = first_shape; 
       cur_shape->right_neighbor(); 
       cur_shape = next_shape) {
    next_shape = cur_shape->right_neighbor();
    float cur_slope = cur_shape->SlopeTo(*next_shape);
    if (fabs(cur_slope - median_slope) <= kMaxSlopeDiff) break;
    RemoveLeftShape(*cur_shape);
    first_shape = next_shape;
  }
  
  // Now filter all shapes from right, starting at first invalid slope
  for (cur_shape = first_shape; 
       cur_shape->right_neighbor(); 
       cur_shape = next_shape) {
    next_shape = cur_shape->right_neighbor();
    float cur_slope = cur_shape->SlopeTo(*next_shape);
    if (fabs(cur_slope - median_slope) > kMaxSlopeDiff) {
      RemoveRightShapesFrom(*next_shape);
      break;
    }
  }
  
  // Has this cluster shrunk down too far?
  if (!first_shape || !first_shape->right_neighbor()) return NULL;
  
  return first_shape;
}
Shape* TextValidator::ValidateClusterStartingAt(Shape& shape) {
  if (!shape.right_neighbor()) return NULL; 
  
  // Gather some info about the cluster like size and bounds
  unsigned cluster_size = 1;
  Box bounds = shape.bounds();
  unsigned max_width = 0;
  unsigned max_height = 0;
  for (Shape* cur_shape = shape.right_neighbor(); 
       cur_shape; 
       cur_shape = cur_shape->right_neighbor()) {
    cluster_size++;
    const Box& shape_bounds = cur_shape->bounds();
    bounds = MinEnclosingBox(bounds, shape_bounds);
    if (shape_bounds.width() > max_width) 
      max_width = shape_bounds.width();
    if (shape_bounds.height() > max_height) 
      max_height = shape_bounds.height();
  }
  
  // Validate the bounds of this cluster
  // NOTE: This invalidates strongly distorted / rotated text!
  if (bounds.width() < bounds.height()) return NULL; 
  
  // Validate bounds of shapes
  if (max_width < 10) return NULL;
  if (max_height < 10) return NULL;
  
  // Validate that cluster lies on a single line. Filter any shapes that do
  // not lie on this line.
  Shape* first = ValidateSlopesOfCluster(shape, cluster_size);
  
  return first;
}
