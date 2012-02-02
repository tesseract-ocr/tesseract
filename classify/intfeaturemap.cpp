// Copyright 2010 Google Inc. All Rights Reserved.
// Author: rays@google.com (Ray Smith)
///////////////////////////////////////////////////////////////////////
// File:        intfeaturemap.cpp
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

#include "intfeaturemap.h"

#include "intfeaturespace.h"
#include "intfx.h"
// These includes do not exist yet, but will be coming soon.
//#include "sampleiterator.h"
//#include "trainingsample.h"
//#include "trainingsampleset.h"

namespace tesseract {

const int kMaxOffsetDist = 32;
const double kMinPCLengthIncrease = 1.0 / 1024;

IntFeatureMap::IntFeatureMap()
  : mapping_changed_(true), compact_size_(0) {
  for (int dir = 0; dir < kNumOffsetMaps; ++dir) {
    offset_plus_[dir] = NULL;
    offset_minus_[dir] = NULL;
  }
}

IntFeatureMap::~IntFeatureMap() {
  Clear();
}

// Pseudo-accessors.
int IntFeatureMap::IndexFeature(const INT_FEATURE_STRUCT& f) const {
  return feature_space_.Index(f);
}
int IntFeatureMap::MapFeature(const INT_FEATURE_STRUCT& f) const {
  return feature_map_.SparseToCompact(feature_space_.Index(f));
}
int IntFeatureMap::MapIndexFeature(int index_feature) const {
  return feature_map_.SparseToCompact(index_feature);
}
INT_FEATURE_STRUCT IntFeatureMap::InverseIndexFeature(int index_feature) const {
  return feature_space_.PositionFromIndex(index_feature);
}
INT_FEATURE_STRUCT IntFeatureMap::InverseMapFeature(int map_feature) const {
  int index = feature_map_.CompactToSparse(map_feature);
  return feature_space_.PositionFromIndex(index);
}
void IntFeatureMap::DeleteMapFeature(int map_feature) {
  feature_map_.Merge(-1, map_feature);
  mapping_changed_ = true;
}
bool IntFeatureMap::IsMapFeatureDeleted(int map_feature) const {
  return feature_map_.IsCompactDeleted(map_feature);
}

// Copies the given feature_space and uses it as the index feature map
// from INT_FEATURE_STRUCT.
void IntFeatureMap::Init(const IntFeatureSpace& feature_space) {
  feature_space_ = feature_space;
  mapping_changed_ = false;
  int sparse_size = feature_space_.Size();
  feature_map_.Init(sparse_size, true);
  feature_map_.Setup();
  compact_size_ = feature_map_.CompactSize();
  // Initialize look-up tables if needed.
  FCOORD dir = FeatureDirection(0);
  if (dir.x() == 0.0f && dir.y() == 0.0f)
    InitIntegerFX();
  // Compute look-up tables to generate offset features.
  for (int dir = 0; dir < kNumOffsetMaps; ++dir) {
    delete [] offset_plus_[dir];
    delete [] offset_minus_[dir];
    offset_plus_[dir] = new int[sparse_size];
    offset_minus_[dir] = new int[sparse_size];
  }
  for (int dir = 1; dir <= kNumOffsetMaps; ++dir) {
    for (int i = 0; i < sparse_size; ++i) {
      int offset_index = ComputeOffsetFeature(i, dir);
      offset_plus_[dir - 1][i] = offset_index;
      offset_index = ComputeOffsetFeature(i, -dir);
      offset_minus_[dir - 1][i] = offset_index;
    }
  }
}

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
int IntFeatureMap::OffsetFeature(int index_feature, int dir) const {
  if (dir > 0 && dir <= kNumOffsetMaps)
    return offset_plus_[dir - 1][index_feature];
  else if (dir < 0 && -dir <= kNumOffsetMaps)
    return offset_minus_[-dir - 1][index_feature];
  else if (dir == 0)
    return index_feature;
  else
    return -1;
}


//#define EXPERIMENT_ON
#ifdef EXPERIMENT_ON  // This code is commented out as SampleIterator and
// TrainingSample are not reviewed/checked in yet, but these functions are a
// useful indicator of how an IntFeatureMap is setup.

// Computes the features used by the subset of samples defined by
// the iterator and sets up the feature mapping.
// Returns the size of the compacted feature space.
int IntFeatureMap::FindNZFeatureMapping(SampleIterator* it) {
  feature_map_.Init(feature_space_.Size(), false);
  int total_samples = 0;
  for (it->Begin(); !it->AtEnd(); it->Next()) {
    const TrainingSample& sample = it->GetSample();
    GenericVector<int> features;
    feature_space_.IndexAndSortFeatures(sample.features(),
                                        sample.num_features(),
                                        &features);
    int num_features = features.size();
    for (int f = 0; f < num_features; ++f)
      feature_map_.SetMap(features[f], true);
    ++total_samples;
  }
  feature_map_.Setup();
  compact_size_ = feature_map_.CompactSize();
  mapping_changed_ = true;
  FinalizeMapping(it);
  tprintf("%d non-zero features found in %d samples\n",
          compact_size_, total_samples);
  return compact_size_;
}
#endif

// After deleting some features, finish setting up the mapping, and map
// all the samples. Returns the size of the compacted feature space.
int IntFeatureMap::FinalizeMapping(SampleIterator* it) {
  if (mapping_changed_) {
    feature_map_.CompleteMerges();
    compact_size_ = feature_map_.CompactSize();
#ifdef EXPERIMENT_ON
    it->MapSampleFeatures(*this);
#endif
    mapping_changed_ = false;
  }
  return compact_size_;
}

// Prints the map features from the set in human-readable form.
void IntFeatureMap::DebugMapFeatures(
    const GenericVector<int>& map_features) const {
  for (int i = 0; i < map_features.size(); ++i) {
    INT_FEATURE_STRUCT f = InverseMapFeature(map_features[i]);
    f.print();
  }
}

void IntFeatureMap::Clear() {
  for (int dir = 0; dir < kNumOffsetMaps; ++dir) {
    delete [] offset_plus_[dir];
    delete [] offset_minus_[dir];
    offset_plus_[dir] = NULL;
    offset_minus_[dir] = NULL;
  }
}

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
int IntFeatureMap::ComputeOffsetFeature(int index_feature, int dir) const {
  INT_FEATURE_STRUCT f = InverseIndexFeature(index_feature);
  ASSERT_HOST(IndexFeature(f) == index_feature);
  if (dir == 0) {
    return index_feature;
  } else if (dir == 1 || dir == -1) {
    FCOORD feature_dir = FeatureDirection(f.Theta);
    FCOORD rotation90(0.0f, 1.0f);
    feature_dir.rotate(rotation90);
    // Find the nearest existing feature.
    for (int m = 1; m < kMaxOffsetDist; ++m) {
      double x_pos = f.X + feature_dir.x() * (m * dir);
      double y_pos = f.Y + feature_dir.y() * (m * dir);
      int x = IntCastRounded(x_pos);
      int y = IntCastRounded(y_pos);
      if (x >= 0 && x <= MAX_UINT8 && y >= 0 && y <= MAX_UINT8) {
        INT_FEATURE_STRUCT offset_f;
        offset_f.X = x;
        offset_f.Y = y;
        offset_f.Theta = f.Theta;
        int offset_index = IndexFeature(offset_f);
        if (offset_index != index_feature && offset_index >= 0)
          return offset_index;  // Found one.
      } else {
        return -1;  // Hit the edge of feature space.
      }
    }
  } else if (dir == 2 || dir == -2) {
    // Find the nearest existing index_feature.
    for (int m = 1; m < kMaxOffsetDist; ++m) {
      int theta = f.Theta + m * dir / 2;
      INT_FEATURE_STRUCT offset_f;
      offset_f.X = f.X;
      offset_f.Y = f.Y;
      offset_f.Theta = Modulo(theta, 256);
      int offset_index = IndexFeature(offset_f);
      if (offset_index != index_feature && offset_index >= 0)
        return offset_index;  // Found one.
    }
  }
  return -1;  // Nothing within the max distance.
}

}  // namespace tesseract.
