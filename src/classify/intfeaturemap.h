// Copyright 2010 Google Inc. All Rights Reserved.
// Author: rays@google.com (Ray Smith)
///////////////////////////////////////////////////////////////////////
// File:        intfeaturemap.h
// Description: Encapsulation of IntFeatureSpace with IndexMapBiDi
//              to provide a subspace mapping and fast feature lookup.
// Created:     Tue Oct 26 08:58:30 PDT 2010
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

#ifndef TESSERACT_CLASSIFY_INTFEATUREMAP_H_
#define TESSERACT_CLASSIFY_INTFEATUREMAP_H_

#include "intfeaturespace.h"
#include "indexmapbidi.h"
#include "intproto.h"

namespace tesseract {

class SampleIterator;

// Number of positive and negative offset maps.
static const int kNumOffsetMaps = 2;

// Class to map a feature space defined by INT_FEATURE_STRUCT to a compact
// down-sampled subspace of actually used features.
// The IntFeatureMap copes with 2 stages of transformation:
// The first step is down-sampling (re-quantization) and converting to a
// single index value from the 3-D input:
//   INT_FEATURE_STRUCT <-> index feature (via IntFeatureSpace) and
// the second is a feature-space compaction to map only the feature indices
// that are actually used. This saves space in classifiers that are built
// using the mapped feature space.
//   index (sparse) feature <-> map (compact) feature via IndexMapBiDi.
// Although the transformations are reversible, the inverses are lossy and do
// not return the exact input INT_FEATURE_STRUCT, due to the many->one nature
// of both transformations.
class IntFeatureMap {
 public:
  IntFeatureMap();
  ~IntFeatureMap();

  // Accessors.
  int sparse_size() const {
    return feature_space_.Size();
  }
  int compact_size() const {
    return compact_size_;
  }
  const IntFeatureSpace& feature_space() const {
    return feature_space_;
  }
  const IndexMapBiDi& feature_map() const {
    return feature_map_;
  }

  // Pseudo-accessors.
  int IndexFeature(const INT_FEATURE_STRUCT& f) const;
  int MapFeature(const INT_FEATURE_STRUCT& f) const;
  int MapIndexFeature(int index_feature) const;
  INT_FEATURE_STRUCT InverseIndexFeature(int index_feature) const;
  INT_FEATURE_STRUCT InverseMapFeature(int map_feature) const;
  void DeleteMapFeature(int map_feature);
  bool IsMapFeatureDeleted(int map_feature) const;

  // Copies the given feature_space and uses it as the index feature map
  // from INT_FEATURE_STRUCT.
  void Init(const IntFeatureSpace& feature_space);

  // Helper to return an offset index feature. In this context an offset
  // feature with a dir of +/-1 is a feature of a similar direction,
  // but shifted perpendicular to the direction of the feature. An offset
  // feature with a dir of +/-2 is feature at the same position, but rotated
  // by +/- one [compact] quantum. Returns the index of the generated offset
  // feature, or -1 if it doesn't exist. Dir should be in
  // [-kNumOffsetMaps, kNumOffsetMaps] to indicate the relative direction.
  // A dir of 0 is an identity transformation.
  // Both input and output are from the index(sparse) feature space, not
  // the mapped/compact feature space, but the offset feature is the minimum
  // distance moved from the input to guarantee that it maps to the next
  // available quantum in the mapped/compact space.
  int OffsetFeature(int index_feature, int dir) const;

  // Computes the features used by the subset of samples defined by
  // the iterator and sets up the feature mapping.
  // Returns the size of the compacted feature space.
  int FindNZFeatureMapping(SampleIterator* it);

  // After deleting some features, finish setting up the mapping, and map
  // all the samples. Returns the size of the compacted feature space.
  int FinalizeMapping(SampleIterator* it);

  // Indexes the given array of features to a vector of sorted indices.
  void IndexAndSortFeatures(const INT_FEATURE_STRUCT* features,
                            int num_features,
                            GenericVector<int>* sorted_features) const {
    feature_space_.IndexAndSortFeatures(features, num_features,
                                        sorted_features);
  }
  // Maps the given array of index/sparse features to an array of map/compact
  // features.
  // Assumes the input is sorted. The output indices are sorted and uniqued.
  // Returns the number of "missed" features, being features that
  // don't map to the compact feature space.
  int MapIndexedFeatures(const GenericVector<int>& index_features,
                         GenericVector<int>* map_features) const {
    return feature_map_.MapFeatures(index_features, map_features);
  }

  // Prints the map features from the set in human-readable form.
  void DebugMapFeatures(const GenericVector<int>& map_features) const;

 private:
  void Clear();

  // Helper to compute an offset index feature. In this context an offset
  // feature with a dir of +/-1 is a feature of a similar direction,
  // but shifted perpendicular to the direction of the feature. An offset
  // feature with a dir of +/-2 is feature at the same position, but rotated
  // by +/- one [compact] quantum. Returns the index of the generated offset
  // feature, or -1 if it doesn't exist. Dir should be in
  // [-kNumOffsetMaps, kNumOffsetMaps] to indicate the relative direction.
  // A dir of 0 is an identity transformation.
  // Both input and output are from the index(sparse) feature space, not
  // the mapped/compact feature space, but the offset feature is the minimum
  // distance moved from the input to guarantee that it maps to the next
  // available quantum in the mapped/compact space.
  int ComputeOffsetFeature(int index_feature, int dir) const;

  // True if the mapping has changed since it was last finalized.
  bool mapping_changed_;
  // Size of the compacted feature space, after unused features are removed.
  int compact_size_;
  // Feature space quantization definition and indexing from INT_FEATURE_STRUCT.
  IntFeatureSpace feature_space_;
  // Mapping from indexed feature space to the compacted space with unused
  // features mapping to -1.
  IndexMapBiDi feature_map_;
  // Index tables to map a feature index to the corresponding feature after a
  // shift perpendicular to the feature direction, or a rotation in place.
  // An entry of -1 indicates that there is no corresponding feature.
  // Array of arrays of size feature_space_.Size() owned by this class.
  int* offset_plus_[kNumOffsetMaps];
  int* offset_minus_[kNumOffsetMaps];

  // Don't use default copy and assign!
  IntFeatureMap(const IntFeatureMap&);
  void operator=(const IntFeatureMap&);
};

}  // namespace tesseract.

#endif  // TESSERACT_CLASSIFY_INTFEATUREMAP_H_
