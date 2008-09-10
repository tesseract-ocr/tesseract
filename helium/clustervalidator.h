// Copyright 2006 Google Inc.
// All Rights Reserved.
// Author: <renn@google.com> (Marius Renn)
//
// This file contains the ClusterValidator class, that is used to assert
// whether proposed Shapes, neighbors and Clusters qualify as valid Clusters.
// The idea is that for each class of objects, such as text or bar-codes, a 
// separate validator is used, that analyzes a given Cluster for the 
// likelihood of belonging to the class it is testing for. On the Shape level
// this can be as simple as looking at the Shape's type. On a pair or
// Cluster level however, more sophisticated measures are required to validate
// the proposed relation.
//
#ifndef HELIUM_CLUSTERVALIDATOR_H__
#define HELIUM_CLUSTERVALIDATOR_H__

namespace helium {

class Shape;

// ClusterValidator subclasses must implement three methods to assert the
// validity of a potential cluster.
class ClusterValidator {
  public:
    // Destructor.
    virtual ~ClusterValidator() {}

    // Validate that the given Shape is a potential member of a cluster.
    virtual bool ValidateShape(const Shape& shape) = 0;
    
    // Validate that the two given Shapes are potential neighbors in a cluster.
    virtual bool ValidatePair(const Shape& left, const Shape& right) = 0;
    
    // Validate that the Cluster starting with the given Shape, is a valid
    // Cluster. This method may alter the Cluster, if it detects clustering
    // errors. Return a pointer to the first Shape (which may have changed
    // during error correction), or NULL if the Cluster is considered invalid.
    virtual Shape* ValidateClusterStartingAt(Shape& shape) = 0;
};

} // namespace

#endif // HELIUM_CLUSTERVALIDATOR_H__
