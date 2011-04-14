/*
 * Copyright 2011, Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// Author: Andrew Harp

#include "utils.h"
#include "time_log.h"

#include "image.h"

#include "math.h"

#include "optical_flow.h"
#include "feature_detector.h"

namespace flow {

OpticalFlow::OpticalFlow(const int32 frame_width,
                         const int32 frame_height,
                         const int32 downsample_factor) :
    downsample_factor_(downsample_factor),
    original_size_(frame_width, frame_height),
    working_size_(frame_width / downsample_factor_,
                  frame_height / downsample_factor_),
    avg_g_x_(0.0f),
    avg_g_y_(0.0f),
    first_frame_index_(0),
    num_frames_(0),
    last_time_fresh_features_(0),
    frame_added_(false),
    features_computed_(false),
    flow_computed_(false) {
  for (int i = 0; i < NUM_FRAMES; ++i) {
    frame_pairs_[i].init(0);
  }

  interest_map_ = new Image<bool>(working_size_);
  feature_scratch_ = new Image<uint8>(working_size_);

  frame1_ = new ImageData(working_size_);
  frame2_ = new ImageData(working_size_);
}


OpticalFlow::~OpticalFlow() {
  // Delete all image storage.
  SAFE_DELETE(feature_scratch_);
  SAFE_DELETE(interest_map_);

  SAFE_DELETE(frame1_);
  SAFE_DELETE(frame2_);
}


void OpticalFlow::nextFrame(const uint8* const new_frame,
                            const clock_t timestamp) {
  frame_added_ = false;
  features_computed_ = false;
  flow_computed_ = false;

  // Move the current framechange index up.
  ++num_frames_;

  // If we've got too many, push up the start of the queue.
  if (num_frames_ > NUM_FRAMES) {
    first_frame_index_ = geNthIndexFromStart(1);
    --num_frames_;
  }

  FramePair* const curr_change = frame_pairs_ + geNthIndexFromEnd(0);
  curr_change->init(timestamp);

  interest_map_->clear(false);

  // Cache out data from last frame.
  // Don't run on frame 0 (no point) or frame 1 (because the old frame will
  // already be in frame1_).
  if (num_frames_ > 2) {
    swap(&frame1_, &frame2_);
    timeLog("Copied data from last run");
  }

  frame2_->init(new_frame, original_size_.width, timestamp, downsample_factor_);

  // Special case for the first frame: make sure the image ends up in
  // frame1_ so that feature detection can be done on it if desired.
  // TODO(andrewharp): Make it so that feature detection is always done
  // on the last frame added.
  if (num_frames_ == 1) {
    swap(&frame1_, &frame2_);
  }

  frame_added_ = true;
}


void OpticalFlow::computeFlow() {
  CHECK(frame_added_ && features_computed_ && !flow_computed_,
        "Optical Flow function called out of order!");

  if (num_frames_ < 2) {
    LOGV("Frame index was %d, skipping computation.", num_frames_);
    return;
  }

  FramePair* const curr_change = &frame_pairs_[geNthIndexFromEnd(0)];

  findCorrespondences(curr_change);

  flow_computed_ = true;
}


void OpticalFlow::findFeatures(const FramePair& prev_change,
                               FramePair* const curr_change) {
  int32 number_of_tmp_features = 0;

  // Copy features from second frame of last pass to temp features of this
  // pass.
  number_of_tmp_features = copyFeatures(prev_change, tmp_features_);

  const float32 buffer = 30.0f;
  number_of_tmp_features += seedFeatures(*frame1_->image_,
      FEATURE_GRID_WIDTH, FEATURE_GRID_HEIGHT,
      buffer, buffer,
      (float32) working_size_.width - buffer,
      (float32) working_size_.height - buffer, FEATURE_DEFAULT,
      tmp_features_ + number_of_tmp_features);
  timeLog("Seeded features.");

  const int32 max_num_fast = MAX_TEMP_FEATURES - number_of_tmp_features;
  number_of_tmp_features +=
      findFastFeatures(*frame1_->image_, max_num_fast,
                       tmp_features_ + number_of_tmp_features,
                       feature_scratch_);

  if (number_of_tmp_features >= MAX_TEMP_FEATURES) {
    LOGW("Hit cap of %d for temporary features!", number_of_tmp_features);
  }

  // Score them...
  scoreFeatures(*frame1_->spatial_x_[0], *frame1_->spatial_y_[0],
                number_of_tmp_features, tmp_features_);

  timeLog("Scored features");

  // Now pare it down a bit.
  curr_change->number_of_features_ = sortAndSelect(
      number_of_tmp_features,
      MAX_FEATURES,
      *interest_map_,
      tmp_features_,
      curr_change->frame1_features_,
      feature_scratch_);

  timeLog("Sorted and selected features");

  LOGV("Picked %d (%d max) final features out of %d potential.",
       curr_change->number_of_features_, MAX_FEATURES, number_of_tmp_features);

  last_time_fresh_features_ = curr_change->end_time;
}


int32 OpticalFlow::copyFeatures(const FramePair& prev_change,
                                Point2D* const new_features) {
  int32 number_of_features = 0;

  // Caching values from last pass, just copy and compact.
  for (int32 i = 0; i < prev_change.number_of_features_; ++i) {
    if (prev_change.optical_flow_found_feature_[i]) {
      new_features[number_of_features] =
          prev_change.frame2_features_[i];

      new_features[number_of_features].score =
          prev_change.frame1_features_[i].score;

      ++number_of_features;
    }
  }

  timeLog("Copied features");

  return number_of_features;
}


void OpticalFlow::computeFeatures(const bool cached_ok) {
  CHECK(frame_added_ && !features_computed_ && !flow_computed_,
        "Optical Flow function called out of order!");

  const FramePair& prev_change = frame_pairs_[geNthIndexFromEnd(1)];
  FramePair* const curr_change = &frame_pairs_[geNthIndexFromEnd(0)];

  const int32 num_found_features = prev_change.countFoundFeatures();
  const clock_t ms_since_last_refresh =
      (curr_change->end_time - last_time_fresh_features_);

  if (cached_ok &&
      num_found_features >= MIN_FEATURES &&
      ms_since_last_refresh <= REGEN_FEATURES_MS) {
    // Reuse the found features from the last frame if we can.
    curr_change->number_of_features_ =
        copyFeatures(prev_change, curr_change->frame1_features_);
  } else {
    // Only find new features to track if we've lost too many since the last
    // time, or it's time to regenerate anyway.
    LOGV("Not enough features (%d/%d), or it's been too long (%ld), "
         "finding more.",
         num_found_features, MIN_FEATURES, ms_since_last_refresh);
    findFeatures(prev_change, curr_change);
  }

  features_computed_ = true;
}


int32 OpticalFlow::getFeatures(const bool only_found,
                               float32* const out_data) const {
  CHECK(frame_added_ && features_computed_,
        "Optical Flow function called out of order!");

  int32 curr_feature = 0;
  const FramePair& change = frame_pairs_[geNthIndexFromEnd(0)];

  for (int32 i = 0; i < change.number_of_features_; ++i) {
    if (!only_found || change.optical_flow_found_feature_[i]) {
      const int base = curr_feature * FEATURE_STEP;
      out_data[base + 0] = change.frame1_features_[i].x * downsample_factor_;
      out_data[base + 1] = change.frame1_features_[i].y * downsample_factor_;
      out_data[base + 2] = change.optical_flow_found_feature_[i];
      out_data[base + 3] = change.frame2_features_[i].x * downsample_factor_;
      out_data[base + 4] = change.frame2_features_[i].y * downsample_factor_;
      out_data[base + 5] = change.frame1_features_[i].score;
      out_data[base + 6] = change.frame1_features_[i].type;
      ++curr_feature;
    }
  }

  LOGV("Got %d features.", curr_feature);

  return curr_feature;
}


// Finds the correspondences for all the points in the current pair of frames.
// Stores the results in the given FramePair.
void OpticalFlow::findCorrespondences(FramePair* const frame_pair) const {
  // Features aren't found until they're found.
  memset(frame_pair->optical_flow_found_feature_, false,
         sizeof(*frame_pair->optical_flow_found_feature_) * MAX_FEATURES);
  timeLog("Cleared old found features");

  int32 num_features_found = 0;

  // For every feature...
  for (int32 i_feat = 0; i_feat < frame_pair->number_of_features_; ++i_feat) {
    Point2D* feature1 = frame_pair->frame1_features_ + i_feat;
    Point2D* feature2 = frame_pair->frame2_features_ + i_feat;

    if (findFlowAtPoint(feature1->x, feature1->y,
                        &feature2->x, &feature2->y)) {
      frame_pair->optical_flow_found_feature_[i_feat] = true;
      ++num_features_found;
    }
  }

  timeLog("Found correspondences");

  LOGV("Found %d of %d feature correspondences",
       num_features_found, frame_pair->number_of_features_);
}


// An implementation of the Pyramidal Lucas-Kanade Optical Flow algorithm.
// See http://robots.stanford.edu/cs223b04/algo_tracking.pdf for details.
bool OpticalFlow::findFlowAtPoint(const float32 u_x, const float32 u_y,
                                  float32* final_x, float32* final_y) const {
  const float32 threshold_squared = square(THRESHOLD);

  // Initial guess.
  float32 g_x = 0.0f;
  float32 g_y = 0.0f;

  // For every level in the pyramid, update the coordinates of the best match.
  for (int32 l = NUM_LEVELS - 1; l >= 0; --l) {
    // Shrink factor from original.
    const int32 shrink_factor = (1 << l);

    // Images I (prev) and J (next).
    const Image<uint8>& img_I = *frame1_->pyramid_[l];
    const Image<uint8>& img_J = *frame2_->pyramid_[l];

    // Computed gradients.
    const Image<int32>& I_x = *frame1_->spatial_x_[l];
    const Image<int32>& I_y = *frame1_->spatial_y_[l];

    // Image position vector (p := u^l), scaled for this level.
    const float32 p_x = u_x / static_cast<float32>(shrink_factor);
    const float32 p_y = u_y / static_cast<float32>(shrink_factor);

    // LOGV("Level %d: (%d, %d) / %d -> (%d, %d)",
    //      l, u_x, u_y, shrink_factor, p_x, p_y);

    // Get values for frame 1.  They remain constant through the inner
    // iteration loop.
    float32 vals_I[ARRAY_SIZE];
    float32 vals_I_x[ARRAY_SIZE];
    float32 vals_I_y[ARRAY_SIZE];

    int32 val_idx = 0;
    for (int32 win_x = -WINDOW_SIZE; win_x <= WINDOW_SIZE; ++win_x) {
      for (int32 win_y = -WINDOW_SIZE; win_y <= WINDOW_SIZE; ++win_y) {
        const float32 x_pos = p_x + win_x;
        const float32 y_pos = p_y + win_y;

        if (!img_I.validInterpPixel(x_pos, y_pos)) {
          return false;
        }

        vals_I[val_idx] = img_I.getPixelInterp(x_pos, y_pos);

        vals_I_x[val_idx] = I_x.getPixelInterp(x_pos, y_pos);
        vals_I_y[val_idx] = I_y.getPixelInterp(x_pos, y_pos);

        ++val_idx;
      }
    }

    // Compute the spatial gradient matrix about point p.
    float32 G[] = { 0, 0, 0, 0 };
    calculateG(vals_I_x, vals_I_y, ARRAY_SIZE, G);

    // Find the inverse of G.
    float32 G_inv[4];
    if (!invert2x2(G, G_inv)) {
      // If we can't invert, hope that the next level will have better luck.
      continue;
    }

#ifdef NORMALIZE
    const float32 mean_I = computeMean(vals_I, ARRAY_SIZE);
    const float32 std_dev_I = computeStdDev(vals_I, ARRAY_SIZE, mean_I);
#endif

    // Iterate NUM_ITERATIONS times or until we converge.
    for (int32 iteration = 0; iteration < NUM_ITERATIONS; ++iteration) {
      // Get values for frame 2.
      float32 vals_J[ARRAY_SIZE];
      int32 val_idx = 0;
      for (int32 win_x = -WINDOW_SIZE; win_x <= WINDOW_SIZE; ++win_x) {
        for (int32 win_y = -WINDOW_SIZE; win_y <= WINDOW_SIZE; ++win_y) {
          const float32 x_pos = p_x + win_x + g_x;
          const float32 y_pos = p_y + win_y + g_y;

          if (!img_I.validInterpPixel(x_pos, y_pos)) {
            return false;
          }

          vals_J[val_idx] = img_J.getPixelInterp(x_pos, y_pos);

          ++val_idx;
        }
      }

#ifdef NORMALIZE
      const float32 mean_J = computeMean(vals_J, ARRAY_SIZE);
      const float32 std_dev_J = computeStdDev(vals_J, ARRAY_SIZE, mean_J);

      const float32 std_dev_ratio = std_dev_I / std_dev_J;
#endif

      // Compute image mismatch vector.
      float32 b_x = 0.0f;
      float32 b_y = 0.0f;
      val_idx = 0;
      for (int32 win_x = -WINDOW_SIZE; win_x <= WINDOW_SIZE; ++win_x) {
        for (int32 win_y = -WINDOW_SIZE; win_y <= WINDOW_SIZE; ++win_y) {
          // Normalized Image difference.

#ifdef NORMALIZE
          const float32 dI = (vals_I[val_idx] - mean_I) -
                             (vals_J[val_idx] - mean_J) * std_dev_ratio;
#else
          const float32 dI = vals_I[val_idx] - vals_J[val_idx];
#endif

          b_x += dI * vals_I_x[val_idx];
          b_y += dI * vals_I_y[val_idx];

          ++val_idx;
        }
      }

      // Optical flow... solve n = G^-1 * b
      const float32 n_x = (G_inv[0] * b_x) + (G_inv[1] * b_y);
      const float32 n_y = (G_inv[2] * b_x) + (G_inv[3] * b_y);

      // Update best guess with residual displacement from this level and
      // iteration.
      g_x += n_x;
      g_y += n_y;

      // LOGV("Iteration %d: delta (%.3f, %.3f)", iteration, n_x, n_y);

      // Abort early if we're already below the threshold.
      if (square(n_x) + square(n_y) < threshold_squared) {
        break;
      }
    }  // Iteration.

    if (l > 0) {
      // Every lower level of the pyramid is 2x as large dimensionally.
      g_x = 2.0f * g_x;
      g_y = 2.0f * g_y;
    }
  }  // Level.

  // LOGV("Final displacement for feature %d was (%.2f, %.2f)",
  //      iFeat, g_x, g_y);

  *final_x = u_x + g_x;
  *final_y = u_y + g_y;

  // Assign the best guess, if we're still in the image.
  if (frame1_->pyramid_[0]->validInterpPixel(*final_x, *final_y)) {
    return true;
  } else {
    return false;
  }
}


void OpticalFlow::addInterestRegion(const int32 num_x, const int32 num_y,
                                    float32 left, float32 top,
                                    float32 right, float32 bottom) {
  left = max(left / downsample_factor_, 0);
  top = max(top / downsample_factor_, 0);
  right = min(right / downsample_factor_, working_size_.width - 1);
  bottom = min(bottom / downsample_factor_, working_size_.height - 1);

  if (left > right || top > bottom) {
    return;
  }

  // This is inclusive of the border pixels, hence the +1.
  const int32 width = right - left + 1;

  // Also inclusive, so it uses a LTE.
  for (int32 y = top; y <= bottom; ++y) {
    bool* row_start = interest_map_->getPixelPtr(left, y);
    memset(row_start, true, width * sizeof(*row_start));
  }
}


Point2D OpticalFlow::getAccumulatedDelta(const Point2D& position,
                                         const float32 radius,
                                         const clock_t timestamp) const {
  Point2D curr_pos(position);

  // Scale down to downsampled size.
  curr_pos.x /= downsample_factor_;
  curr_pos.y /= downsample_factor_;

  LOGV("Tracking accumulated delta from %.2f, %.2f", curr_pos.x, curr_pos.y);

  const float32 cutoff_dist = radius / downsample_factor_;

  // Anything that ended before the requested timestamp is of no concern to us.
  bool found_it = false;
  int32 num_frames_back = -1;
  for (int32 i = 0; i < num_frames_; ++i) {
    const FramePair& frame_pair =
        frame_pairs_[geNthIndexFromEnd(i)];

    if (frame_pair.end_time <= timestamp) {
      num_frames_back = i - 1;

      if (num_frames_back > 0) {
        LOGV("Went %d out of %d frames before finding frame. (index: %d)",
             num_frames_back, num_frames_, geNthIndexFromEnd(i));
      }

      found_it = true;
      break;
    }
  }

  if (!found_it) {
    const FramePair& frame_pair = frame_pairs_[geNthIndexFromStart(0)];
    const FramePair& latest_frame_pair = frame_pairs_[geNthIndexFromEnd(0)];

    clock_t latest_time = latest_frame_pair.end_time;

    LOGW("History did not go back far enough! %ld vs %ld",
         latest_time - frame_pair.end_time,
         latest_time - timestamp);
  }

  // Loop over all the frames in the queue, tracking the accumulated delta
  // of the point from frame to frame.  It's possible the point could
  // go out of frame, but keep tracking as best we can, using points near
  // the edge of the screen where it went out of bounds.
  for (int32 i = num_frames_back; i >= 0; --i) {
    const FramePair& frame_pair = frame_pairs_[geNthIndexFromEnd(i)];
    CHECK(frame_pair.end_time >= timestamp, "Frame timestamp was too early!");

    const Point2D delta = frame_pair.queryFlow(curr_pos, cutoff_dist);
    curr_pos.x += delta.x;
    curr_pos.y += delta.y;
  }

  // Scale back to original size.
  curr_pos.x *= downsample_factor_;
  curr_pos.y *= downsample_factor_;

  // Return the delta only.
  return curr_pos - position;
}


void FramePair::init(const clock_t end_time) {
  this->end_time = end_time;
  memset(optical_flow_found_feature_, false,
         sizeof(*optical_flow_found_feature_) * MAX_FEATURES);
  number_of_features_ = 0;
}


Point2D FramePair::queryFlow(
    const Point2D& initial, const float32 cutoff_dist) const {
  float32 weights[MAX_FEATURES];
  memset(weights, 0, sizeof(float32) * MAX_FEATURES);

  // Compute the max score.
  float32 max_score = 0.0f;
  for (int32 i = 0; i < MAX_FEATURES; ++i) {
    if (optical_flow_found_feature_[i]) {
      max_score = max(max_score, frame1_features_[i].score);
    }
  }

  const float32 cutoff_dist_squared = cutoff_dist * cutoff_dist;
  for (int32 i = 0; i < MAX_FEATURES; ++i) {
    if (optical_flow_found_feature_[i]) {
      const float32 sq_x_dist = square(initial.x - frame1_features_[i].x);
      const float32 sq_y_dist = square(initial.y - frame1_features_[i].y);

      const float32 dist_squared = sq_x_dist + sq_y_dist;

      // The weighting based off distance.  Anything within the cuttoff
      // distance has a weight of 1, and everything outside of that is within
      // the range [0, 1).
      const float32 distance_score =
          min(cutoff_dist_squared / dist_squared, 1.0f);

      // The weighting based on score strength. 0.5f - 1.0f.
      float32 intrinsic_score = 1.0f;
      if (max_score > 0) {
        intrinsic_score = (frame1_features_[i].score / max_score) / 2.0f;
      }

      // The final score will be in the range [0, 1].
      weights[i] = distance_score * intrinsic_score;
    }
  }

  return getWeightedDelta(weights);
}


Point2D FramePair::getWeightedDelta(const float32* const weights) const {
  float32 total_weight = 0.0f;
  float32 weighted_sum_x = 0.0f;
  float32 weighted_sum_y = 0.0f;

  Point2D deltas[MAX_FEATURES];

  // Compute weighted mean and deltas.
  for (int32 i = 0; i < MAX_FEATURES; ++i) {
    const float32 weight = weights[i];
    if (weight > 0.0f) {
      deltas[i] = frame2_features_[i] - frame1_features_[i];
      weighted_sum_x += deltas[i].x * weight;
      weighted_sum_y += deltas[i].y * weight;
      total_weight += weight;
    }
  }
  const float32 weighted_mean_x = weighted_sum_x / total_weight;
  const float32 weighted_mean_y = weighted_sum_y / total_weight;

  // Compute weighted squared standard deviation from weighted mean.
  float32 weighted_dev_squared_sum = 0.0f;
  for (int32 i = 0; i < MAX_FEATURES; ++i) {
    const float32 weight = weights[i];

    if (weight > 0.0f) {
      const float32 devX = deltas[i].x - weighted_mean_x;
      const float32 devY = deltas[i].y - weighted_mean_y;

      const float32 squared_deviation = (devX * devX) + (devY * devY);
      weighted_dev_squared_sum += squared_deviation * weight;
    }
  }
  const float32 weighted_std_dev_squared =
      weighted_dev_squared_sum / total_weight;

  // Recompute weighted mean change without outliers.
  float32 good_weight = 0.0f;
  float32 good_sum_x = 0.0f;
  float32 good_sum_y = 0.0f;

  for (int32 i = 0; i < MAX_FEATURES; ++i) {
    const float32 weight = weights[i];

    if (weight > 0.0f) {
      const float32 dev_x = deltas[i].x - weighted_mean_x;
      const float32 dev_y = deltas[i].y - weighted_mean_y;

      const float32 sqrd_deviation = (dev_x * dev_x) + (dev_y * dev_y);

      // Throw out anything beyond NUM_DEVIATIONS.
      if (sqrd_deviation <= NUM_DEVIATIONS * weighted_std_dev_squared) {
        good_sum_x += deltas[i].x * weight;
        good_sum_y += deltas[i].y * weight;
        good_weight += weight;
      }
    }
  }

  if (good_weight > 0.0f) {
    return Point2D(good_sum_x / good_weight, good_sum_y / good_weight);
  } else {
    return Point2D(0.0f, 0.0f);
  }
}

}  // namespace flow
