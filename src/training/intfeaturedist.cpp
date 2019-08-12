// Copyright 2011 Google Inc. All Rights Reserved.
// Author: rays@google.com (Ray Smith)
///////////////////////////////////////////////////////////////////////
// File:        intfeaturedist.cpp
// Description: Fast set-difference-based feature distance calculator.
// Created:     Thu Sep 01 13:07:30 PDT 2011
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

#include "intfeaturedist.h"
#include "intfeaturemap.h"

namespace tesseract {

IntFeatureDist::IntFeatureDist()
  : size_(0), total_feature_weight_(0.0),
    feature_map_(nullptr), features_(nullptr),
    features_delta_one_(nullptr), features_delta_two_(nullptr) {
}

IntFeatureDist::~IntFeatureDist() {
  Clear();
}

// Initialize the table to the given size of feature space.
void IntFeatureDist::Init(const IntFeatureMap* feature_map) {
  size_ = feature_map->sparse_size();
  Clear();
  feature_map_ = feature_map;
  features_ = new bool[size_];
  features_delta_one_ = new bool[size_];
  features_delta_two_ = new bool[size_];
  memset(features_, false, size_ * sizeof(features_[0]));
  memset(features_delta_one_, false, size_ * sizeof(features_delta_one_[0]));
  memset(features_delta_two_, false, size_ * sizeof(features_delta_two_[0]));
  total_feature_weight_ = 0.0;
}

// Setup the map for the given indexed_features that have been indexed by
// feature_map.
void IntFeatureDist::Set(const GenericVector<int>& indexed_features,
                          int canonical_count, bool value) {
  total_feature_weight_ = canonical_count;
  for (int i = 0; i < indexed_features.size(); ++i) {
    const int f = indexed_features[i];
    features_[f] = value;
    for (int dir = -kNumOffsetMaps; dir <= kNumOffsetMaps; ++dir) {
      if (dir == 0) continue;
      const int mapped_f = feature_map_->OffsetFeature(f, dir);
      if (mapped_f >= 0) {
        features_delta_one_[mapped_f] = value;
        for (int dir2 = -kNumOffsetMaps; dir2 <= kNumOffsetMaps; ++dir2) {
          if (dir2 == 0) continue;
          const int mapped_f2 = feature_map_->OffsetFeature(mapped_f, dir2);
          if (mapped_f2 >= 0)
            features_delta_two_[mapped_f2] = value;
        }
      }
    }
  }
}

// Compute the distance between the given feature vector and the last
// Set feature vector.
double IntFeatureDist::FeatureDistance(
    const GenericVector<int>& features) const {
  const int num_test_features = features.size();
  const double denominator = total_feature_weight_ + num_test_features;
  double misses = denominator;
  for (int i = 0; i < num_test_features; ++i) {
    const int index = features[i];
    const double weight = 1.0;
    if (features_[index]) {
      // A perfect match.
      misses -= 2.0 * weight;
    } else if (features_delta_one_[index]) {
      misses -= 1.5 * weight;
    } else if (features_delta_two_[index]) {
      // A near miss.
      misses -= 1.0 * weight;
    }
  }
  return misses / denominator;
}

// Compute the distance between the given feature vector and the last
// Set feature vector.
double IntFeatureDist::DebugFeatureDistance(
    const GenericVector<int>& features) const {
  const int num_test_features = features.size();
  const double denominator = total_feature_weight_ + num_test_features;
  double misses = denominator;
  for (int i = 0; i < num_test_features; ++i) {
    const int index = features[i];
    const double weight = 1.0;
    INT_FEATURE_STRUCT f = feature_map_->InverseMapFeature(features[i]);
    tprintf("Testing feature weight %g:", weight);
    f.print();
    if (features_[index]) {
      // A perfect match.
      misses -= 2.0 * weight;
      tprintf("Perfect hit\n");
    } else if (features_delta_one_[index]) {
      misses -= 1.5 * weight;
      tprintf("-1 hit\n");
    } else if (features_delta_two_[index]) {
      // A near miss.
      misses -= 1.0 * weight;
      tprintf("-2 hit\n");
    } else {
      tprintf("Total miss\n");
    }
  }
  tprintf("Features present:");
  for (int i = 0; i < size_; ++i) {
    if (features_[i]) {
      INT_FEATURE_STRUCT f = feature_map_->InverseMapFeature(i);
      f.print();
    }
  }
  tprintf("\nMinus one features:");
  for (int i = 0; i < size_; ++i) {
    if (features_delta_one_[i]) {
      INT_FEATURE_STRUCT f = feature_map_->InverseMapFeature(i);
      f.print();
    }
  }
  tprintf("\nMinus two features:");
  for (int i = 0; i < size_; ++i) {
    if (features_delta_two_[i]) {
      INT_FEATURE_STRUCT f = feature_map_->InverseMapFeature(i);
      f.print();
    }
  }
  tprintf("\n");
  return misses / denominator;
}

// Clear all data.
void IntFeatureDist::Clear() {
  delete [] features_;
  features_ = nullptr;
  delete [] features_delta_one_;
  features_delta_one_ = nullptr;
  delete [] features_delta_two_;
  features_delta_two_ = nullptr;
}

}  // namespace tesseract
