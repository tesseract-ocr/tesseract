// Copyright 2010 Google Inc. All Rights Reserved.
// Author: rays@google.com (Ray Smith)
///////////////////////////////////////////////////////////////////////
// File:        intfeaturespace.cpp
// Description: Indexed feature space based on INT_FEATURE_STRUCT.
// Created:     Wed Mar 24 11:21:27 PDT 2010
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

#include "intfeaturespace.h"
#include "intfx.h"

namespace tesseract {

IntFeatureSpace::IntFeatureSpace()
  : x_buckets_(0), y_buckets_(0), theta_buckets_(0) {
}

void IntFeatureSpace::Init(uint8_t xbuckets, uint8_t ybuckets, uint8_t thetabuckets) {
  x_buckets_ = xbuckets;
  y_buckets_ = ybuckets;
  theta_buckets_ = thetabuckets;
}

// Serializes the feature space definition to the given file.
// Returns false on error.
bool IntFeatureSpace::Serialize(FILE* fp) const {
  if (fwrite(&x_buckets_, sizeof(x_buckets_), 1, fp) != 1)
    return false;
  if (fwrite(&y_buckets_, sizeof(y_buckets_), 1, fp) != 1)
    return false;
  if (fwrite(&theta_buckets_, sizeof(theta_buckets_), 1, fp) != 1)
    return false;
  return true;
}

// Returns an INT_FEATURE_STRUCT corresponding to the given index.
// This is the inverse of the Index member.
INT_FEATURE_STRUCT IntFeatureSpace::PositionFromIndex(int index) const {
  return PositionFromBuckets(index / (y_buckets_ * theta_buckets_),
                             index / theta_buckets_ % y_buckets_,
                             index % theta_buckets_);
}

// Bulk calls to Index. Maps the given array of features to a vector of
// int32_t indices in the same order as the input.
void IntFeatureSpace::IndexFeatures(const INT_FEATURE_STRUCT* features,
                                    int num_features,
                                    GenericVector<int>* mapped_features) const {
  mapped_features->truncate(0);
  for (int f = 0; f < num_features; ++f)
    mapped_features->push_back(Index(features[f]));
}

// Bulk calls to Index. Maps the given array of features to a vector of
// sorted int32_t indices.
void IntFeatureSpace::IndexAndSortFeatures(
    const INT_FEATURE_STRUCT* features, int num_features,
    GenericVector<int>* sorted_features) const {
  sorted_features->truncate(0);
  for (int f = 0; f < num_features; ++f)
    sorted_features->push_back(Index(features[f]));
  sorted_features->sort();
}

// Returns a feature space index for the given x,y position in a display
// window, or -1 if the feature is a miss.
int IntFeatureSpace::XYToFeatureIndex(int x, int y) const {
  // Round the x,y position to a feature. Search for a valid theta.
  INT_FEATURE_STRUCT feature(x, y, 0);
  int index = -1;
  for (int theta = 0; theta <= UINT8_MAX && index < 0; ++theta) {
    feature.Theta = theta;
    index = Index(feature);
  }
  if (index < 0) {
    tprintf("(%d,%d) does not exist in feature space!\n", x, y);
    return -1;
  }
  feature = PositionFromIndex(index);
  tprintf("Click at (%d, %d) ->(%d, %d), ->(%d, %d)\n",
          x, y, feature.X, feature.Y, x - feature.X, y - feature.Y);
  // Get the relative position of x,y from the rounded feature.
  x -= feature.X;
  y -= feature.Y;
  if (x != 0 || y != 0) {
    double angle = atan2(static_cast<double>(y), static_cast<double>(x)) + M_PI;
    angle *= kIntFeatureExtent / (2.0 * M_PI);
    feature.Theta = static_cast<uint8_t>(angle + 0.5);
    index = Index(feature);
    if (index < 0) {
      tprintf("Feature failed to map to a valid index:");
      feature.print();
      return -1;
    }
    feature = PositionFromIndex(index);
  }
  feature.print();
  return index;
}

// Returns an INT_FEATURE_STRUCT corresponding to the given bucket coords.
INT_FEATURE_STRUCT IntFeatureSpace::PositionFromBuckets(int x,
                                                        int y,
                                                        int theta) const {
  INT_FEATURE_STRUCT pos(
      (x * kIntFeatureExtent + kIntFeatureExtent / 2) / x_buckets_,
      (y * kIntFeatureExtent + kIntFeatureExtent / 2) / y_buckets_,
      DivRounded(theta * kIntFeatureExtent, theta_buckets_));
  return pos;
}

}  // namespace tesseract.
