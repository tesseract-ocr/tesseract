// Copyright 2011 Google Inc. All Rights Reserved.
// Author: rays@google.com (Ray Smith)
///////////////////////////////////////////////////////////////////////
// File:        intfeaturedist.h
// Description: Fast set-difference-based feature distance calculator.
// Created:     Thu Sep 01 12:14:30 PDT 2011
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
///////////////////////////////////////////////////////////////////////

#ifndef TESSERACT_CLASSIFY_INTFEATUREDIST_H_
#define TESSERACT_CLASSIFY_INTFEATUREDIST_H_

#include <tesseract/genericvector.h>

namespace tesseract {

class IntFeatureMap;

// Feature distance calculator designed to provide a fast distance calculation
// based on set difference between a given feature set and many other feature
// sets in turn.
// Representation of a feature set as an array of bools that are sparsely
// true, and companion arrays that allow fast feature set distance
// calculations with allowance of offsets in position.
// Init is expensive, so for greatest efficiency, to re-initialize for a new
// feature set, use Set(..., false) on the SAME feature set as was used to
// setup with Set(..., true), to return to its initialized state before
// reuse with Set(..., true) on a new feature set.
class IntFeatureDist {
 public:
  IntFeatureDist();
  ~IntFeatureDist();

  // Initialize the bool array to the given size of feature space.
  // The feature_map is just borrowed, and must exist for the entire
  // lifetime of the IntFeatureDist.
  void Init(const IntFeatureMap* feature_map);

  // Setup the map for the given indexed_features that have been indexed by
  // feature_map. After use, use Set(..., false) to reset to the initial state
  // as this is faster than calling Init for sparse spaces.
  void Set(const GenericVector<int>& indexed_features,
           int canonical_count, bool value);

  // Compute the distance between the given feature vector and the last
  // Set feature vector.
  double FeatureDistance(const GenericVector<int>& features) const;
  double DebugFeatureDistance(const GenericVector<int>& features) const;

 private:
  // Clear all data.
  void Clear();

  // Size of the indexed feature space.
  int size_;
  // Total weight of features currently stored in the maps.
  double total_feature_weight_;
  // Pointer to IntFeatureMap given at Init to find offset features.
  const IntFeatureMap* feature_map_;
  // Array of bools indicating presence of a feature.
  bool* features_;
  // Array indicating the presence of a feature offset by one unit.
  bool* features_delta_one_;
  // Array indicating the presence of a feature offset by two units.
  bool* features_delta_two_;
};

}  // namespace tesseract

#endif  // TESSERACT_CLASSIFY_INTFEATUREDIST_H_
