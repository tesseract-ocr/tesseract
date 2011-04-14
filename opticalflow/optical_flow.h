// Copyright 2009 Google Inc. All Rights Reserved.
// Author: andrewharp@google.com (Andrew Harp)

#ifndef JAVA_COM_GOOGLE_ANDROID_APPS_UNVEIL_JNI_OPTICALFLOW_OPTICAL_FLOW_H_
#define JAVA_COM_GOOGLE_ANDROID_APPS_UNVEIL_JNI_OPTICALFLOW_OPTICAL_FLOW_H_

#include "types.h"
#include "utils.h"

// Feature detection.
#define MAX_TEMP_FEATURES 4096
#define MAX_FEATURES 128

// Number of floats each feature takes up when exporting to an array.
#define FEATURE_STEP 7

// Number of frame deltas to keep around in the circular queue.
#define NUM_FRAMES 128

// Redetect if we ever have less than this number of features.
#define MIN_FEATURES 6

// How long to wait between forcing complete feature regeneration.
#define REGEN_FEATURES_MS 400

// Number of iterations to do tracking on each feature at each pyramid level.
#define NUM_ITERATIONS 3

// Number of pyramid levels used for tracking.
#define NUM_LEVELS 4

// Window size to integrate over to find local image derivative.
#define WINDOW_SIZE 3

// Total area of integration windows.
#define ARRAY_SIZE (2 * WINDOW_SIZE + 1) * (2 * WINDOW_SIZE + 1)

// Error that's considered good enough to early abort tracking.
#define THRESHOLD 0.03f

// Maximum number of deviations a feature delta can be from the weighted
// average before being thrown out for region-based queries.
#define NUM_DEVIATIONS 2.0f

// Resolution of feature grid to seed features with.
#define FEATURE_GRID_WIDTH 4
#define FEATURE_GRID_HEIGHT 3

// Whether to normalize feature windows for intensity.
#define NORMALIZE

namespace flow {

template <typename T>
class Image;

// Class that encapsulates all bulky processed data for a frame.
class ImageData {
 public:
  explicit ImageData(Size size) {
    timestamp_ = 0;

    image_ = new Image<uint8>(size);

    for (int32 i = 0; i < NUM_LEVELS; ++i) {
      pyramid_[i] = (i == 0) ? image_ : new Image<uint8>(size);

      spatial_x_[i] = new Image<int32>(size);
      spatial_y_[i] = new Image<int32>(size);

      size.width /= 2;
      size.height /= 2;
    }
  }

  ~ImageData() {
    // image_ will be deleted along with the rest of the pyramids.

    for (int32 i = 0; i < NUM_LEVELS; ++i) {
      SAFE_DELETE(pyramid_[i]);
      SAFE_DELETE(spatial_x_[i]);
      SAFE_DELETE(spatial_y_[i]);
    }
  }

  void init(const uint8* const new_frame, const int32 stride,
            const clock_t timestamp, const int32 downsample_factor_) {
    timestamp_ = timestamp;

    image_->fromArray(new_frame, stride, downsample_factor_);
    timeLog("Downsampled image");

    // Create the smoothed pyramids.
    computeSmoothedPyramid(*image_, NUM_LEVELS, pyramid_);
    timeLog("Created smoothed pyramids");

    // Create the spatial derivatives for frame 1.
    computeSpatialPyramid((const Image<uint8>**)pyramid_,
                          NUM_LEVELS, spatial_x_, spatial_y_);
    timeLog("Created spatial derivatives");
  }

  clock_t timestamp_;
  Image<uint8>* image_;
  Image<uint8>* pyramid_[NUM_LEVELS];
  Image<int32>* spatial_x_[NUM_LEVELS];
  Image<int32>* spatial_y_[NUM_LEVELS];
};

// A class that records a timestamped frame features
// translation delta for optical flow.
class FramePair {
 public:
  // Cleans up the FramePair so that they can be reused.
  void init(const clock_t end_time);

  // Throws out outliers based on the input weighting.
  Point2D getWeightedDelta(const float32* const weights) const;

  // Weights points based on the query_point and cutoff_dist, then
  // returns getWeightedDelta.  Essentially tells you where a point at the
  // beginning of a frame ends up.
  Point2D queryFlow(const Point2D& query_point,
                    const float32 cutoff_dist) const;

  // Just count up and return the number of features from the first frame that
  // were found in the second frame.
  inline int32 countFoundFeatures() const {
    int32 num_found_features = 0;
    for (int32 i = 0; i < number_of_features_; ++i) {
      if (optical_flow_found_feature_[i]) {
        ++num_found_features;
      }
    }
    return num_found_features;
  }

  // The time at frame2.
  clock_t end_time;

  // This array will contain the features found in frame 1.
  Point2D frame1_features_[MAX_FEATURES];

  // Contain the locations of the points from frame 1 in frame 2.
  Point2D frame2_features_[MAX_FEATURES];

  // The number of features in frame 1.
  int32 number_of_features_;

  // Keeps track of which features were actually found from one frame
  // another.
  // The i-th element of this array will be non-zero if and only if the i-th
  // feature of frame 1 was found in frame 2.
  bool optical_flow_found_feature_[MAX_FEATURES];
};

// Class encapsulating all the data and logic necessary for performing optical
// flow.  The general order of operations on a per frame basis is:
//
// // Notify optical flow that a new frame is available.
// nextFrame(...);
//
// // Tell it any regions we want it to pay special attention to.
// addInterestRegion(...);
//
// // Have it compute the flow.
// computeFlow();
//
// // Look up the delta from a given point at a given time to the current time.
// getAccumulatedDelta(...);
class OpticalFlow {
 public:
  OpticalFlow(const int32 frame_width, const int32 frame_height,
              const int32 downsample_factor);
  ~OpticalFlow();

  // Add a new frame to the optical flow.  Will update all the non-feature
  // related member variables.
  //
  // new_frame should be a buffer of grayscale values, one byte per pixel,
  // at the original frame_width and frame_height used to initialize the
  // OpticalFlow object.  Downsampling will be handled internally.
  //
  // time_stamp should be a time in milliseconds that later calls to this and
  // other methods will be relative to.
  void nextFrame(const uint8* const new_frame, const clock_t timestamp);

  // Find the features in the frame before the current frame.
  // If only one frame exists, features will be found in that frame.
  void computeFeatures(const bool cached_ok = false);

  // Process the most recent two frames, and fill in the feature arrays.
  void computeFlow();

  // Copy the feature arrays after computeFlow is called.
  // out_data should be at least MAX_FEATURES * FEATURE_STEP long.
  // Currently, its format is [x1 y1 found x2 y2 score] repeated N times,
  // where N is the number of features tracked.  N is returned as the result.
  int32 getFeatures(const bool only_found, float32* const out_data) const;

  // Tells you the overall flow for region of a given radius at a given time to
  // the present.
  Point2D getAccumulatedDelta(const Point2D& position,
                              const float radius,
                              const clock_t timestamp) const;

  // Pay special attention to the area inside this box on the next
  // optical flow pass.
  void addInterestRegion(const int32 num_x, const int32 num_y,
                         float32 left, float32 top,
                         float32 right, float32 bottom);

  // Finds the correspondences for all the points in the current pair of frames.
  // Stores the results in the given FramePair.
  void findCorrespondences(FramePair* const curr_change) const;

  // An implementation of the Pyramidal Lucas-Kanade Optical Flow algorithm.
  bool findFlowAtPoint(const float32 u_x, const float32 u_y,
                       float32* final_x, float32* final_y) const;

  void printInfo() const {
#ifdef VERBOSE_LOGGING
    const int32 first_frame_index = geNthIndexFromStart(0);
    const FramePair& first_frame_pair = frame_pairs_[first_frame_index];

    const int32 last_frame_index = geNthIndexFromEnd(0);
    const FramePair& last_frame_pair = frame_pairs_[last_frame_index];

    LOGV  ("Queue size: %d, last/first: %4d %4d: %8ld - %8ld = %8ld",
         num_frames_, last_frame_index, first_frame_index,
         last_frame_pair.end_time, first_frame_pair.end_time,
         last_frame_pair.end_time - first_frame_pair.end_time);
#endif
  }

 private:
  inline int32 geNthIndexFromStart(const int32 offset) const {
    CHECK(offset >= 0 && offset < num_frames_,
          "Offset out of range!  %d out of %d.", offset, num_frames_);
    return (first_frame_index_ + offset) % NUM_FRAMES;
  }

  inline int32 geNthIndexFromEnd(const int32 offset) const {
    return geNthIndexFromStart(num_frames_ - 1 - offset);
  }

  // Finds features in the previous frame and adds them to curr_change.
  void findFeatures(const FramePair& prev_change,
                    FramePair* const curr_change);

  // Copies and compacts the found features in the second frame of prev_change
  // into the array at new_features.
  static int32 copyFeatures(const FramePair& prev_change,
                            Point2D* const new_features);

  const int32 downsample_factor_;

  // Size of the original images.
  const Size original_size_;

  // Size of the internally allocated images (after original is downsampled).
  const Size working_size_;

  float32 avg_g_x_;
  float32 avg_g_y_;

  int32 first_frame_index_;
  int32 num_frames_;

  clock_t last_time_fresh_features_;

  Point2D tmp_features_[MAX_TEMP_FEATURES];

  FramePair frame_pairs_[NUM_FRAMES];

  // Scratch memory for feature candidacy detection and non-max suppression.
  Image<uint8>* feature_scratch_;

  // Regions of the image to pay special attention to.
  Image<bool>* interest_map_;

  ImageData* frame1_;
  ImageData* frame2_;

  bool frame_added_;
  bool features_computed_;
  bool flow_computed_;
};

}  // namespace flow
#endif  // JAVA_COM_GOOGLE_ANDROID_APPS_UNVEIL_JNI_OPTICALFLOW_OPTICAL_FLOW_H_
