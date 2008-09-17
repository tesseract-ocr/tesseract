// Copyright 2006 Google Inc.
// All Rights Reserved.
// Author: <renn@google.com> (Marius Renn)

// Local includes
#include "cluster.h"
#include "debugging.h"
#include "heliumbinarizer.h"
#include "image.h"
#include "mathfunctions.h"
#include "shape.h"

#include <math.h>

using namespace helium;

const int kMaxInt = ~(1 << (sizeof(int)*8 - 1));
const float kVarianceFactor = 1.5;

inline bool IsForeground(Color pixel, Color reference, Color variance) {
  //Color variation = ColorDifference(pixel, reference);
  unsigned distance = EuclideanDistance(pixel, reference);

  int max_diff = Lightness(variance)*4;
  if (max_diff > 128) max_diff = 128;
  
  return distance < max_diff;
}

HeliumBinarizer::HeliumBinarizer(const Image& image) 
  : Binarizer(image), 
    areas_(4),
    extension_clusters_(2),
    current_index_(0),
    number_of_clusters_(0) {
  ASSERT(image_.Valid());
}

bool HeliumBinarizer::CheckBounds(const Box& bounds) const {
  return (bounds.right() < image_.width()) 
    && (bounds.bottom() < image_.height())
    && (bounds.top() >= 0)
    && (bounds.left() >= 0)
    && (bounds.width() > 0)
    && (bounds.height() > 0);
}

void HeliumBinarizer::AddCluster(const Cluster& cluster) {
  number_of_clusters_++;
  
  // Add area left of first cluster
  ExtendEnd(*(cluster.first()), number_of_clusters_, false);
  
  for (const Shape* cur_shape = cluster.first(); 
       cur_shape; 
       cur_shape = cur_shape->right_neighbor()) {
    // Add area right of last cluster
    if (!cur_shape->right_neighbor()) 
      ExtendEnd(*cur_shape, number_of_clusters_, true);
    else 
      AddInBetweenArea(*cur_shape, 
                       *cur_shape->right_neighbor(), 
                       number_of_clusters_);
                       
    // Add cluster area
    AddShapeArea(*cur_shape, number_of_clusters_);
  }
}

void HeliumBinarizer::AddClusters(const ClusterArray& clusters) {
  for (unsigned i = 0; i < clusters.size(); i++)
    AddCluster(*(clusters.ValueAt(i)));
}

void HeliumBinarizer::AddShapeArea(const Shape& shape, uint8 id) {
  Area area(shape.bounds(), 
            id, 
            shape.histogram().Expected(),
            shape.histogram().Variance());
  
  areas_.Add(area);
}

void HeliumBinarizer::AddInBetweenArea(const Shape& left, 
                                 const Shape& right, 
                                 uint8 id) {
  int x_0 = left.bounds().right();
  int x_1 = right.bounds().left();
  int y_0 = (left.bounds().top() + right.bounds().top()) / 2;
  int y_1 = (left.bounds().bottom() + right.bounds().bottom()) / 2;
  if ((x_1 <= x_0) || (y_1 <= y_0)) return;
  
  Box bounds(x_0, y_0, x_1 - x_0, y_1 - y_0);
           
  Area area(bounds, 
            id, 
            left.histogram().Expected(),  // NOTE: Not correct, mix l and r!
            left.histogram().Variance()); // NOTE: Same here!
             
  areas_.Add(area);
}

bool HeliumBinarizer::ScanHorizontal(const Area& area, const Point& p) const {
  Color* image_ptr = image_.Access(p);
  
  for (unsigned x = 0; x < area.bounds.width(); x++) 
    if (IsForeground(*(image_ptr++), area.expected, area.variance))
      return true;
  return false;
}

bool HeliumBinarizer::ScanVertical(const Area& area, const Point& p) const {
  const int step = image_.width();
  Color* image_ptr = image_.Access(p);
  
  for (unsigned y = 0; y < area.bounds.height(); y++) 
    if (IsForeground(*(image_ptr += step), area.expected, area.variance))
      return true;
  return false;
}

bool HeliumBinarizer::ScanLeft(const Area& area) const {
  Point left(area.bounds.left() + 1, area.bounds.top());
  return ScanVertical(area, left);
}

bool HeliumBinarizer::ScanTop(const Area& area) const {
  Point top(area.bounds.left(), area.bounds.top() + 1);
  return ScanHorizontal(area, top);
}

bool HeliumBinarizer::ScanRight(const Area& area) const {
  Point right(area.bounds.right() - 1, area.bounds.top());
  return ScanVertical(area, right);
}

bool HeliumBinarizer::ScanBottom(const Area& area) const {
  Point bottom(area.bounds.left(), area.bounds.bottom() - 1);
  return ScanHorizontal(area, bottom);
}

bool HeliumBinarizer::ScanVMiddle(const Area& area) const {
  Point middle(area.bounds.left(), area.bounds.center().y);
  return ScanVertical(area, middle);
}

bool HeliumBinarizer::ScanHMiddle(const Area& area) const {
  Point middle(area.bounds.center().x, area.bounds.top());
  return ScanVertical(area, middle);
}

void HeliumBinarizer::ExtendTopEndRec(const Area& area, Array<Box>* extensions) {
  if (ScanBottom(area)) {
    const Box& box = area.bounds;
    extensions->Add(box);
    
    // Create new extension area below
    Box ext_bounds(box.left(), box.top() - box.height() / 2, 
                   box.width(), box.height() / 2);
    if(!CheckBounds(ext_bounds)) return;
    
    // Create new area
    Area extension(ext_bounds, area.id, area.expected, area.variance);
    
    // Recursively call extending algorithm
    ExtendTopEndRec(extension, extensions);
  }
}

void HeliumBinarizer::ExtendBottomEndRec(const Area& area, Array<Box>* extensions) {
  if (ScanTop(area)) {
    const Box& box = area.bounds;
    extensions->Add(box);
    
    // Create new extension area below
    Box ext_bounds(box.left(), box.bottom(), box.width(), box.height() / 2);
    if(!CheckBounds(ext_bounds)) return;
    
    // Create new area
    Area extension(ext_bounds, area.id, area.expected, area.variance);
    
    // Recursively call extending algorithm
    ExtendBottomEndRec(extension, extensions);
  }
}

void HeliumBinarizer::ExtendEndRec(const Area& area, 
                             Array<Box>* extensions, 
                             bool to_right) {
  const Box& box = area.bounds;
  if ((ScanLeft(area) && to_right) 
  || ScanHMiddle(area) 
  || (ScanRight(area) && !to_right)) {
    extensions->Add(box);
    
    // Create new extension area to the right or left
    Box ext_bounds(to_right ? box.right() : box.left() - box.width(), 
                   box.top(), box.width(), box.height());
    if(!CheckBounds(ext_bounds)) return;
    
    Area extension(ext_bounds, area.id, area.expected, area.variance);
    
    // Recursively call extending algorithm
    ExtendEndRec(extension, extensions, to_right);
    
    // Also try to extend up...
    Box up_bounds(box.left(), box.top() - box.height() / 2, 
                  box.width(), box.height() / 2);
    if(CheckBounds(up_bounds)) {
      Area up_area(up_bounds, area.id, area.expected, area.variance);
      ExtendTopEndRec(up_area, extensions);
    }
    
    // ...And down
    Box down_bounds(box.left(), box.bottom(), box.width(), box.height() / 2);
    if(CheckBounds(down_bounds)) {
      Area down_area(down_bounds, area.id, area.expected, area.variance);
      ExtendBottomEndRec(down_area, extensions);
    }
  }
}

void HeliumBinarizer::ExtendEnd(const Shape& shape, uint8 id, bool right_end) {
  Array<Box>* extensions = new Array<Box>(2);
  
  const Box& box = shape.bounds();
  const Color& expected = shape.histogram().Expected();
  const Color& variance = shape.histogram().Variance();
  
  Box ext_bounds(right_end ? box.right() : box.left() - box.width() / 2,
                  box.top(), box.width() / 2, box.height());
                  
  // Make sure we don't end up having thousands of very small extensions
  if (ext_bounds.width() < 8) ext_bounds.set_width(8);
    
  Area first(ext_bounds, id, expected, variance);
                   
  ExtendEndRec(first, extensions, right_end);
  
  if (extensions->size() > 0) {
    Box total_bounds = MinEnclosingBox(*extensions);
    
    if (total_bounds.height() / box.height() < 2) {
      AreaCluster new_cluster(extensions, total_bounds, id, expected, variance);
      extension_clusters_.Add(new_cluster);
      return;
    }
  }
  
  delete extensions;
}

void HeliumBinarizer::MergeOverlaps(const AreaCluster& cluster) {
  const Box& bounds = cluster.bounds;
  for (unsigned i = 0; i < areas_.size(); i++) {
    const Area& cur_area = areas_.ValueAt(i);
    if (cur_area.id == cluster.id) continue;
    
    const unsigned smaller_height = cur_area.bounds.height() > bounds.height()
                                  ? bounds.height()
                                  : cur_area.bounds.height();
                                  
    Box overlap = Intersection(bounds, cur_area.bounds);
    if (!CheckBounds(overlap)) continue;
    
    uint8 color_distance = EuclideanDistance(cur_area.expected,
                                             cluster.expected);
    
    // Overlapping height must be atleast half the size of the height of the
    // smaller area.
    if ((smaller_height / overlap.height() <= 1) && (color_distance < 32)) 
      cluster_classes_.Unify(cluster.id, cur_area.id);
  }
}

void HeliumBinarizer::FindAreasOfClass(int class_id, 
                                 Array<Area>& areas, 
                                 Array<Area>& found) {
  for (unsigned i = 0; i < areas.size(); i++) {
    if (cluster_classes_.Find(areas.ValueAt(i).id) == class_id) {
      found.Add(areas.ValueAt(i));
      areas.ValueAt(i).binarized = true;
    }
  }
}

void HeliumBinarizer::FindAreasOfClassInClusters(int class_id, 
                                           Array<AreaCluster>& clusters, 
                                           Array<Area>& found) {
  for (unsigned i = 0; i < extension_clusters_.size(); i++) {
    const AreaCluster& cur_cluster = extension_clusters_.ValueAt(i);
    if (cluster_classes_.Find(cur_cluster.id) == class_id) {
      for (unsigned j = 0; j < cur_cluster.boxes->size(); j++) {
        Area cluster_area(cur_cluster.boxes->ValueAt(j),
                          cur_cluster.id,
                          cur_cluster.expected,
                          cur_cluster.variance);
        found.Add(cluster_area);
      }
    }
  }
}

void HeliumBinarizer::BinarizeAreaToMask(const Area& area, 
                                         Mask& mask, 
                                         const Point& mask_origin) {
  // Assert that area lies within mask bounds
  ASSERT(area.bounds.origin().x >= mask_origin.x);
  ASSERT(area.bounds.origin().y >= mask_origin.y);
  
  // Calculate where we need to start in mask
  Point mask_start(area.bounds.origin().x - mask_origin.x + 1,
                   area.bounds.origin().y - mask_origin.y + 1);
  
  // Setup pointers
  Color* image_ptr = image_.Access(area.bounds.origin());
  bool* mask_ptr = mask.Access(mask_start);

  // Setup steps in vetrical direction
  int mask_vstep = mask.width() - area.bounds.width();
  int image_vstep = image_.width() - area.bounds.width();
  
  // Copy loop
  Point p;
  Color expected = area.expected;
  Color variance = area.variance;
  for (p.y = area.bounds.top(); p.y <= area.bounds.bottom(); p.y++) {
    for (p.x = area.bounds.left(); p.x <= area.bounds.right(); p.x++) {
      if (IsForeground(*image_ptr, expected, variance)) *mask_ptr = 1;
      mask_ptr++;
      image_ptr++;
    }
    mask_ptr += mask_vstep;
    image_ptr += image_vstep;
  }
}

// NOTE: Kind of ugly mask creation: Maybe return Mask instead
bool HeliumBinarizer::GetNextMask(Mask& out_mask, Box& out_bounds) {
  if (current_index_ == 0) {
    // First time here? Label roots in UnionFind tree
    cluster_classes_.LabelRoots();
    
    // And merge all extension areas with existing areas
    for (unsigned i = 0; i < extension_clusters_.size(); i++)
      MergeOverlaps(extension_clusters_.ValueAt(i));
  }
  
  // Loop to find all relevant masks for the current text line
  while (current_index_ < areas_.size()) {
    const Area& cur_area = areas_.ValueAt(current_index_);
    current_index_++;
    
    if (cur_area.binarized) continue;
    
    int cur_class = cluster_classes_.Find(cur_area.id);
    Array<Area> cur_areas(4);
    
    // Find all areas of this class id in main area list...
    FindAreasOfClass(cur_class, areas_, cur_areas);
    
    // ...and in the clusters
    FindAreasOfClassInClusters(cur_class, extension_clusters_, cur_areas);
    
    // Create mask to hold all areas of these classes
    Box cur_bounds = BoundsOfAreas(cur_areas);
    out_mask = Mask(cur_bounds.width() + 2, cur_bounds.height() + 2);
    out_mask.Clear();
    
    // Copy all binarized areas to output mask
    for (unsigned i = 0; i < cur_areas.size(); i++) {
      Area area = cur_areas.ValueAt(i);
      BinarizeAreaToMask(area, out_mask, cur_bounds.origin());
    }
    
    out_bounds = cur_bounds;
    return true;
  }
  return false;
}

Box HeliumBinarizer::BoundsOfAreas(const Array<Area>& areas) {
  ASSERT(areas.size() > 0);
  Box bounds = areas.ValueAt(0).bounds;
  for (unsigned i = 1; i < areas.size(); i++) 
    bounds = MinEnclosingBox(bounds, areas.ValueAt(i).bounds);
  return bounds;
}
