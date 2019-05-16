// (C) Copyright 2017, Google Inc.
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "intfeaturemap.h"
#include "intfeaturespace.h"

#include "include_gunit.h"

using tesseract::IntFeatureMap;
using tesseract::IntFeatureSpace;

// Random re-quantization to test that they don't have to be easy.
// WARNING! Change these and change the expected_misses calculation below.
const int kXBuckets = 16;
const int kYBuckets = 24;
const int kThetaBuckets = 13;

namespace {

class IntFeatureMapTest : public testing::Test {
 protected:
  void SetUp() {
    std::locale::global(std::locale(""));
  }

 public:
  // Expects that the given vector has contiguous integer values in the
  // range [start, end).
  void ExpectContiguous(const GenericVector<int>& v, int start, int end) {
    for (int i = start; i < end; ++i) {
      EXPECT_EQ(i, v[i - start]);
    }
  }
};

// Tests the IntFeatureMap and implicitly the IntFeatureSpace underneath.
TEST_F(IntFeatureMapTest, Exhaustive) {
#ifdef DISABLED_LEGACY_ENGINE
  // Skip test because IntFeatureSpace is missing.
  GTEST_SKIP();
#else
  IntFeatureSpace space;
  space.Init(kXBuckets, kYBuckets, kThetaBuckets);
  IntFeatureMap map;
  map.Init(space);
  int total_size = kIntFeatureExtent * kIntFeatureExtent * kIntFeatureExtent;
  std::unique_ptr<INT_FEATURE_STRUCT[]> features(
      new INT_FEATURE_STRUCT[total_size]);
  // Fill the features with every value.
  for (int y = 0; y < kIntFeatureExtent; ++y) {
    for (int x = 0; x < kIntFeatureExtent; ++x) {
      for (int theta = 0; theta < kIntFeatureExtent; ++theta) {
        int f_index = (y * kIntFeatureExtent + x) * kIntFeatureExtent + theta;
        features[f_index].X = x;
        features[f_index].Y = y;
        features[f_index].Theta = theta;
      }
    }
  }
  GenericVector<int> index_features;
  map.IndexAndSortFeatures(features.get(), total_size, &index_features);
  EXPECT_EQ(total_size, index_features.size());
  int total_buckets = kXBuckets * kYBuckets * kThetaBuckets;
  GenericVector<int> map_features;
  int misses = map.MapIndexedFeatures(index_features, &map_features);
  EXPECT_EQ(0, misses);
  EXPECT_EQ(total_buckets, map_features.size());
  ExpectContiguous(map_features, 0, total_buckets);
  EXPECT_EQ(total_buckets, map.compact_size());
  EXPECT_EQ(total_buckets, map.sparse_size());

  // Every offset should be within dx, dy, dtheta of the start point.
  int dx = kIntFeatureExtent / kXBuckets + 1;
  int dy = kIntFeatureExtent / kYBuckets + 1;
  int dtheta = kIntFeatureExtent / kThetaBuckets + 1;
  int bad_offsets = 0;
  for (int index = 0; index < total_buckets; ++index) {
    for (int dir = -tesseract::kNumOffsetMaps; dir <= tesseract::kNumOffsetMaps;
         ++dir) {
      int offset_index = map.OffsetFeature(index, dir);
      if (dir == 0) {
        EXPECT_EQ(index, offset_index);
      } else if (offset_index >= 0) {
        INT_FEATURE_STRUCT f = map.InverseIndexFeature(index);
        INT_FEATURE_STRUCT f2 = map.InverseIndexFeature(offset_index);
        EXPECT_TRUE(f.X != f2.X || f.Y != f2.Y || f.Theta != f2.Theta);
        EXPECT_LE(abs(f.X - f2.X), dx);
        EXPECT_LE(abs(f.Y - f2.Y), dy);
        int theta_delta = abs(f.Theta - f2.Theta);
        if (theta_delta > kIntFeatureExtent / 2)
          theta_delta = kIntFeatureExtent - theta_delta;
        EXPECT_LE(theta_delta, dtheta);
      } else {
        ++bad_offsets;
        INT_FEATURE_STRUCT f = map.InverseIndexFeature(index);
      }
    }
  }
  EXPECT_LE(bad_offsets, (kXBuckets + kYBuckets) * kThetaBuckets);

  // To test the mapping further, delete the 1st and last map feature, and
  // test again.
  map.DeleteMapFeature(0);
  map.DeleteMapFeature(total_buckets - 1);
  map.FinalizeMapping(nullptr);
  map.IndexAndSortFeatures(features.get(), total_size, &index_features);
  // Has no effect on index features.
  EXPECT_EQ(total_size, index_features.size());
  misses = map.MapIndexedFeatures(index_features, &map_features);
  int expected_misses = (kIntFeatureExtent / kXBuckets) *
                        (kIntFeatureExtent / kYBuckets) *
                        (kIntFeatureExtent / kThetaBuckets + 1);
  expected_misses += (kIntFeatureExtent / kXBuckets) *
                     (kIntFeatureExtent / kYBuckets + 1) *
                     (kIntFeatureExtent / kThetaBuckets);
  EXPECT_EQ(expected_misses, misses);
  EXPECT_EQ(total_buckets - 2, map_features.size());
  ExpectContiguous(map_features, 0, total_buckets - 2);
  EXPECT_EQ(total_buckets - 2, map.compact_size());
  EXPECT_EQ(total_buckets, map.sparse_size());
#endif
}

}  // namespace.
