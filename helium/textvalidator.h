// Copyright 2006 Google Inc.
// All Rights Reserved.
// Author: <renn@google.com> (Marius Renn)
//
// This file contains the TextValidator class, which checks whether a proposed
// Cluster is a text line or not.
//
#ifndef HELIUM_TEXTVALIDATOR_H__
#define HELIUM_TEXTVALIDATOR_H__

#include "clusterer.h"
#include "clustervalidator.h"

namespace helium {

// The TextValidator class implements the validation methods of the abstract
// base class ClusterValidator. It is fairly complex, analyzing a variety of
// attributes linked to the proposed relations.
class TextValidator : public ClusterValidator {
  public:
    // Validates that a proposed shape is indeed a text character, by checking
    // the Shape's trace class, and analyzing the Shape's color variance.
    bool ValidateShape(const Shape& shape);
    
    // Validates that a proposed Shape pair, qualifies as two neighboring
    // text characters, by analyzing features of both Shapes, such as size,
    // distance, overlap, enclosure and colors.
    bool ValidatePair(const Shape& left, const Shape& right);
    
    // Validates that the proposed Cluster, starting at the given Shape, 
    // qualifies as a text line, by analyzing the bounds of the cluster, and
    // asserting whether the characters roughly lie on a single straight line. 
    // This method may alter the Cluster by removing leading and trailing
    // characters that do not lie on the text line.
    Shape* ValidateClusterStartingAt(Shape& shape);
  
  private:
    // Removes the given Shape from the Cluster, assuming that this is the 
    // left-most Shape of the Cluster.
    void RemoveLeftShape(Shape& shape);
    
    // Remove the given Shape from the Cluster, and all the Shapes that follow
    // it to the right. 
    void RemoveRightShapesFrom(Shape& shape);
    
    // Validates that the proposed Cluster, starting at the given Shape, 
    // qualifies as a text line, by analyzing if the elements of the Cluster
    // lie on the same line. It will remove leading and trailing characters,
    // that are not on the text line, assuming that these are clustering
    // errors. If the line shrinks down too far, this method will return
    // NULL.
    Shape* ValidateSlopesOfCluster(Shape& shape, unsigned cluster_size);
};

} // namespace

#endif  // HELIUM_TEXTVALIDATOR_H__
