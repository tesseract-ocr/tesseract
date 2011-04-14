// Copyright 2010 Google Inc. All Rights Reserved.
// Author: andrewharp@google.com (Andrew Harp)

#ifndef JAVA_COM_GOOGLE_ANDROID_APPS_UNVEIL_JNI_OPTICALFLOW_FEATURE_DETECTOR_H_
#define JAVA_COM_GOOGLE_ANDROID_APPS_UNVEIL_JNI_OPTICALFLOW_FEATURE_DETECTOR_H_

namespace flow {

// Add features along a regular grid.
int32 seedFeatures(const Image<uint8>& frame,
                   const int32 num_x, const int32 num_y,
                   const float32 left, const float32 top,
                   const float32 right, const float32 bottom,
                   const int32 type, Point2D* const features);

// Compute the corneriness of a point in the image.
float32 harrisFilter(const Image<int32>& I_x, const Image<int32>& I_y,
                     const int32 x, const int32 y);

// Scan the frame for potential features using the FAST feature detector.
int32 findFastFeatures(const Image<uint8>& frame,
                       const int32 max_num_features,
                       Point2D* const features,
                       Image<uint8>* const best_feature_map);

// Score a bunch of candidate features.  Assigns the scores to the input
// candidate_features array entries.
void scoreFeatures(const Image<int32>& I_x, const Image<int32>& I_y,
                   const int32 num_candidates,
                   Point2D* const candidate_features);

// Copy the best features (with local non-max suppression) from
// candidate_features to final_features.
// Returns the number of features copied.
int32 sortAndSelect(const int32 num_candidates,
                    const int32 max_features,
                    const Image<bool>& interest_map,
                    Point2D* const candidate_features,
                    Point2D* const final_features,
                    Image<uint8>* const best_feature_map);

}  // namespace flow

#endif  // JAVA_COM_GOOGLE_ANDROID_APPS_UNVEIL_JNI_OPTICALFLOW_FEATURE_DETECTOR_H_
