// Copyright 2010 Google Inc. All Rights Reserved.
// Author: rays@google.com (Ray Smith)
///////////////////////////////////////////////////////////////////////
// File:        intfeaturespace.h
// Description: Indexed feature space based on INT_FEATURE_STRUCT.
// Created:     Wed Mar 24 10:55:30 PDT 2010
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

#ifndef TESSERACT_CLASSIFY_INTFEATURESPACE_H__
#define TESSERACT_CLASSIFY_INTFEATURESPACE_H__

#include "genericvector.h"
#include "intproto.h"

// Extent of x,y,theta in the input feature space. [0,255].
const int kIntFeatureExtent = 256;
// Extent of x,y,theta dimensions in the quantized feature space.
const int kBoostXYBuckets = 16;
const int kBoostDirBuckets = 16;

namespace tesseract {

class IndexMap;

// Down-sampling quantization of the INT_FEATURE_STRUCT feature space and
// conversion to a single scalar index value, used as a binary feature space.
class IntFeatureSpace {
 public:
  IntFeatureSpace();
  // Default copy constructors and assignment OK!

  // Setup the feature space with the given dimensions.
  void Init(uinT8 xbuckets, uinT8 ybuckets, uinT8 thetabuckets);

  // Serializes the feature space definition to the given file.
  // Returns false on error.
  bool Serialize(FILE* fp) const;

  // DeSerializes the feature space definition from the given file.
  // If swap is true, the data is big/little-endian swapped.
  // Returns false on error.
  bool DeSerialize(bool swap, FILE* fp);

  // Returns the total size of the feature space.
  int Size() const {
    return static_cast<int>(x_buckets_) * y_buckets_ * theta_buckets_;
  }
  // Returns an INT_FEATURE_STRUCT corresponding to the given index.
  // This is the inverse of the Index member.
  INT_FEATURE_STRUCT PositionFromIndex(int index) const;

  // Returns a 1-dimensional index corresponding to the given feature value.
  // Range is [0, Size()-1]. Inverse of PositionFromIndex member.
  int Index(const INT_FEATURE_STRUCT& f) const {
    return (XBucket(f.X) * y_buckets_ + YBucket(f.Y)) * theta_buckets_ +
        ThetaBucket(f.Theta);
  }
  // Bulk calls to Index. Maps the given array of features to a vector of
  // inT32 indices in the same order as the input.
  void IndexFeatures(const INT_FEATURE_STRUCT* features, int num_features,
                     GenericVector<int>* mapped_features) const;
  // Bulk calls to Index. Maps the given array of features to a vector of
  // sorted inT32 indices.
  void IndexAndSortFeatures(const INT_FEATURE_STRUCT* features,
                            int num_features,
                            GenericVector<int>* sorted_features) const;
  // Returns a feature space index for the given x,y position in a display
  // window, or -1 if the feature is a miss.
  int XYToFeatureIndex(int x, int y) const;

 protected:
  // Converters to generate indices for individual feature dimensions.
  int XBucket(int x) const {
    int bucket = x * x_buckets_ / kIntFeatureExtent;
    return ClipToRange(bucket, 0, static_cast<int>(x_buckets_) - 1);
  }
  int YBucket(int y) const {
    int bucket = y * y_buckets_ / kIntFeatureExtent;
    return ClipToRange(bucket, 0, static_cast<int>(y_buckets_) - 1);
  }
  // Use DivRounded for theta so that exactly vertical and horizontal are in
  // the middle of a bucket. The Modulo takes care of the wrap-around.
  int ThetaBucket(int theta) const {
    int bucket = DivRounded(theta * theta_buckets_, kIntFeatureExtent);
    return Modulo(bucket, theta_buckets_);
  }
  // Returns an INT_FEATURE_STRUCT corresponding to the given buckets.
  INT_FEATURE_STRUCT PositionFromBuckets(int x, int y, int theta) const;

  // Feature space definition - serialized.
  uinT8 x_buckets_;
  uinT8 y_buckets_;
  uinT8 theta_buckets_;
};

}  // namespace tesseract.


#endif  // TESSERACT_CLASSIFY_INTFEATURESPACE_H__
