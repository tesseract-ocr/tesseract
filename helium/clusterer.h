// Copyright 2006 Google Inc.
// All Rights Reserved.
// Author: <renn@google.com> (Marius Renn)
//
// This file contains the Clusterer class, used for clustering Shapes to larger
// contiguous units. This is where the characters are clustered to words, bars
// to bar-codes, and so on. A ClusterValidator is passed to the Clusterer, 
// which is responsible for allowing only those Clusters that make sense for
// a certain scenario (such as detecting text, or detecting bar-codes). 
//
#ifndef HELIUM_CLUSTERER_H__
#define HELIUM_CLUSTERER_H__

#include "array.h"
#include "box.h"

namespace helium {

class Cluster;
class ClusterValidator;
class Image;
class Shape;

// We use Cluster Arrays often enough to justify a prettier name.
typedef Array<Cluster*> ClusterArray;

// The Clusterer class provides functionality for clustering a given set of
// Shapes using a given ClusterValidator. It stores the found Clusters in an 
// internal list of Clusters, that can be accessed using the clusters() method.
class Clusterer {
  public:
    Clusterer();
    
    // Destructor calls ClearClusters().
    virtual ~Clusterer();
    
    // Given the Array of Shapes and the ClusterValidator, this method clusters
    // the Shapes based on their distance to one another, and whether or not
    // the ClusterValidator accepts a proposed Cluster. Adds the found Clusters
    // to the internal Array. This method has O(n^2) complexity, where n is
    // the number of Shapes.
    virtual void ClusterShapes(const Array<Shape*>& shapes,
                               ClusterValidator& validator);

    // Returns the found clusters.
    inline const ClusterArray& clusters() const {
      return clusters_;
    }
    
    // Clears the internal Cluster Array and deallocates all contained 
    // Clusters. You may call ClusterShapes(...) after clearing.
    void ClearClusters();
    
    // Draw the bounding boxes of the found clusters onto the specified Image.
    // For debugging only.
    void DrawClusterBounds(Image& image) const;
    
  private:
    // Iterates through the given neighbors, and for each one, finds the 
    // closest Shape, that the ClusterValidator accepts as a valid Shape
    // (ValidateShape(...)), and valid neighbor (ValidatePair(...)).
    void SetNeighbors(const Array<Shape*>& shapes,
                      ClusterValidator& validator);
    
    // Once the neighbors are set, this method creates Cluster objects, one 
    // for each string of neighbors. Clusters are added only if the
    // ClusterValidator accepts a proposed Cluster 
    // (ValidateClusterStartingAt(...)).
    void AddClusters(const Array<Shape*>& shapes,
                     ClusterValidator& validator);
    
    ClusterArray clusters_;
};

} // namespace

#endif  // HELIUM_CLUSTERER_H__
