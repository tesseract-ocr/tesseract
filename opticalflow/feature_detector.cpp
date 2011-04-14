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
//
// Various feature detecting functions.

#include <float.h>

#include "utils.h"
#include "time_log.h"

#include "image.h"
#include "feature_detector.h"

// Threshold for pixels to be considered different.
#define FAST_DIFF_AMOUNT 10

// How far from edge of frame to stop looking for FAST features.
#define FAST_BORDER_BUFFER 20

// Minimum enforced distance between detected features.
// Default
#define MIN_FEATURE_DIST_NORMAL 24

// Regions selected as "interesting" (aka have annotations) can have denser
// feature coverage.
#define MIN_FEATURE_DIST_INTEREST 12

// How many FAST qualifying pixels must be connected to a pixel for it to be
// considered a candidate feature for Harris filtering.
#define MIN_NUM_CONNECTED 8

// Size of the window to integrate over for Harris filtering.
// Compare to WINDOW_SIZE in optical_flow.h.
#define HARRIS_WINDOW_SIZE 2

// Arbitrary parameter for how picky Harris filter is, the higher the more
// discriminating.
#define SENSITIVITY 0.2f

namespace flow {

void scoreFeatures(const Image<int32>& I_x, const Image<int32>& I_y,
                   const int32 num_candidates,
                   Point2D* const candidate_features) {
  // Score all the features
  for (int32 i = 0; i < num_candidates; ++i) {
    Point2D* const feature = candidate_features + i;
    feature->score = harrisFilter(I_x, I_y, feature->x, feature->y);
  }
}

// Quicksorts detected features by score and then selects them such that
// they are separated by a minimum distance.
int32 sortAndSelect(const int32 num_candidates, const int32 max_features,
                    const Image<bool>& interest_map,
                    Point2D* const candidate_features,
                    Point2D* const final_features,
                    Image<uint8>* const best_feature_map) {
  qsort(candidate_features, num_candidates);

#ifdef SANITY_CHECKS
  // Verify that the array got sorted.
  float32 last_score = -FLT_MAX;
  for (int32 i = 0; i < num_candidates; ++i) {
    const float32 curr_score = (candidate_features + i)->score;

    // Scores should be monotonically increasing.
    CHECK(last_score <= curr_score,
          "Quicksort failure! %d: %.5f > %d: %.5f",
          i - 1, last_score, i, curr_score);

    last_score = curr_score;
  }
#endif

  best_feature_map->clear(false);

  int32 num_features = 0;

  for (int32 i = num_candidates - 1; i >= 0; --i) {
    const Point2D& candidate = candidate_features[i];

    // Since features are sorted, the first 0 or less value means we can stop
    // looking.
    if (candidate.score <= 0.0f) {
      break;
    }

    // Lookup whether this feature is in an interest region.  If so, other
    // features may appear closer to it than normal.
    const int32 distance = interest_map.getPixel(candidate.x, candidate.y) ?
        MIN_FEATURE_DIST_INTEREST : MIN_FEATURE_DIST_NORMAL;

    if (markImage(candidate.x, candidate.y, distance, best_feature_map)) {
      final_features[num_features] = candidate;
      num_features++;

      if (num_features >= max_features) {
        break;
      }
    }
  }

  return num_features;
}

// Walks along the given circle checking for pixels above or below the center.
// Returns a score, or 0 if the feature did not pass the criteria.
//
// Parameters:
//  circle_perimeter: the circumference in pixels of the circle.
//  threshold: the minimum number of contiguous pixels that must be above or
//             below the center value.
//  center_ptr: the location of the center pixel in memory
//  offsets: the relative offsets from the center pixel of the edge pixels.
inline int32 testCircle(const int32 circle_perimeter, const int32 threshold,
                        const uint8* const center_ptr,
                        const int32* offsets) {
  // Get the actual value of the center pixel for easier reference later on.
  const int32 center_value = static_cast<int32>(*center_ptr);

  // Number of total pixels to check.  Have to wrap around some in case
  // the contiguous section is split by the array edges.
  const int32 num_total = circle_perimeter + threshold - 1;

  int32 num_above = 0;
  int32 above_diff = 0;

  int32 num_below = 0;
  int32 below_diff = 0;

  // Used to tell when this is definitely not going to meet the threshold so we
  // can early abort.
  int32 minimum_by_now = threshold - num_total + 1;

  // Go through every pixel along the perimeter of the circle, and then around
  // again a little bit.
  for (int32 i = 0; i < num_total; ++i) {
    // This should be faster than mod.
    const int32 perim_index = i < circle_perimeter ? i : i - circle_perimeter;

    // This gets the value of the current pixel along the perimeter by using
    // a precomputed offset.
    const int32 curr_value =
        static_cast<int32>(center_ptr[offsets[perim_index]]);

    const int32 difference = curr_value - center_value;

    if (difference > FAST_DIFF_AMOUNT) {
      above_diff += difference;
      ++num_above;

      num_below = 0;
      below_diff = 0;

      if (num_above >= threshold) {
        return above_diff;
      }
    } else if (difference < -FAST_DIFF_AMOUNT) {
      below_diff += difference;
      ++num_below;

      num_above = 0;
      above_diff = 0;

      if (num_below >= threshold) {
        return below_diff;
      }
    } else {
      num_above = 0;
      num_below = 0;
      above_diff = 0;
      below_diff = 0;
    }

    // See if there's any chance of making the threshold.
    if (max(num_above, num_below) < minimum_by_now) {
      // Didn't pass.
      return 0;
    }
    ++minimum_by_now;
  }

  // Didn't pass.
  return 0;
}

// Creates features in a regular grid, regardless of image contents.
int32 seedFeatures(const Image<uint8>& frame,
                   const int32 num_x, const int32 num_y,
                   const float32 left, const float32 top,
                   const float32 right, const float32 bottom,
                   const int32 type, Point2D* const features) {
  int32 num_features = 0;

  const float32 step_x = ((right - left) / (num_x - 1));
  const float32 step_y = ((bottom - top) / (num_y - 1));

  for (int32 x = 0; x < num_x; ++x) {
    for (int32 y = 0; y < num_y; ++y) {
      const int32 x_pos = x * step_x + left;
      const int32 y_pos = y * step_y + top;
      if (inRange(x_pos, 0, frame.width_less_one_) &&
          inRange(y_pos, 0, frame.height_less_one_)) {
        Point2D* const feature = features + num_features;
        feature->x = x_pos;
        feature->y = y_pos;
        feature->type = type;

        ++num_features;
      }
    }
  }

  return num_features;
}


// Returns how likely a point in the image is to be a corner.
float32 harrisFilter(const Image<int32>& I_x, const Image<int32>& I_y,
                     const int32 x, const int32 y) {
  // Image gradient matrix.
  float32 G[] = { 0, 0, 0, 0 };
  calculateG(HARRIS_WINDOW_SIZE, x, y, I_x, I_y, G);

  const float32 g_sum = G[0] + G[1] + G[2] + G[3];

  const float32 a = G[0] / g_sum;
  const float32 b = G[1] / g_sum;
  const float32 c = G[2] / g_sum;
  const float32 d = G[3] / g_sum;

  const float32 det = a * d - b * c;
  const float32 trace = a + d;

  const float32 inner = square(trace) - 4 * det;

  if (inner >= 0.0f) {
    const float32 square_root_inner = sqrtf(inner);
    const float32 eig1 = (trace + square_root_inner) / 2.0f;
    const float32 eig2 = (trace - square_root_inner) / 2.0f;
    return eig1 * eig2 - SENSITIVITY * square(eig1 + eig2);
  }

  // Way negative.
  return -100.0f;
}

// FAST feature detector.
int32 findFastFeatures(const Image<uint8>& frame, const int32 max_num_features,
                       Point2D* const features,
                       Image<uint8>* const best_feature_map) {
  /*
   // Reference for a circle of diameter 7.
   const int32 circle[] = {0, 0, 1, 1, 1, 0, 0,
                           0, 1, 0, 0, 0, 1, 0,
                           1, 0, 0, 0, 0, 0, 1,
                           1, 0, 0, 0, 0, 0, 1,
                           1, 0, 0, 0, 0, 0, 1,
                           0, 1, 0, 0, 0, 1, 0,
                           0, 0, 1, 1, 1, 0, 0};
   const int32 circle_offset[] =
       {2, 3, 4, 8, 12, 14, 20, 21, 27, 28, 34, 36, 40, 44, 45, 46};
   */

  // Quick test of compass directions.  Any length 16 circle with a break of up
  // to 4 pixels will have at least 3 of these 4 pixels active.
  static const int32 short_circle_perimeter = 4;
  static const int32 short_threshold = 3;
  static const int32 short_circle_x[] = { -3,  0, +3,  0 };
  static const int32 short_circle_y[] = {  0, -3,  0, +3 };

  // Precompute image offsets.
  int32 short_offsets[short_circle_perimeter];
  for (int i = 0; i < short_circle_perimeter; ++i) {
    short_offsets[i] = short_circle_x[i] + short_circle_y[i] * frame.getWidth();
  }

  // Large circle values.
  static const int32 full_circle_perimeter = 16;
  static const int32 full_threshold = 12;
  static const int32 full_circle_x[] =
      { -1,  0, +1, +2, +3, +3, +3, +2, +1, +0, -1, -2, -3, -3, -3, -2 };
  static const int32 full_circle_y[] =
      { -3, -3, -3, -2, -1,  0, +1, +2, +3, +3, +3, +2, +1, +0, -1, -2 };

  // Precompute image offsets.
  int32 full_offsets[full_circle_perimeter];
  for (int i = 0; i < full_circle_perimeter; ++i) {
    full_offsets[i] = full_circle_x[i] + full_circle_y[i] * frame.getWidth();
  }

  const int frame_width = frame.getWidth();

  const int end_y = frame.getHeight() - FAST_BORDER_BUFFER;
  const int end_x = frame.getWidth() - FAST_BORDER_BUFFER;

  best_feature_map->clear(0);

  // Loop through once to find FAST feature clumps.
  for (int32 img_y = FAST_BORDER_BUFFER; img_y < end_y; ++img_y) {
    const uint8* curr_pixel_ptr =
        frame.getPixelPtrConst(FAST_BORDER_BUFFER, img_y);

    for (int32 img_x = FAST_BORDER_BUFFER; img_x < end_x; ++img_x) {
      // Only insert it if it meets the quick minimum requirements test.
      if (testCircle(short_circle_perimeter, short_threshold,
                     curr_pixel_ptr, short_offsets) != 0) {

        // Longer test for actual feature score..
        const int32 fast_score = testCircle(full_circle_perimeter,
                                            full_threshold,
                                            curr_pixel_ptr,
                                            full_offsets);

        // Non-zero score means the feature was found.
        if (fast_score != 0) {
          uint8* const center_ptr = best_feature_map->getPixelPtr(img_x, img_y);

          // Increase the feature count on this pixel and the pixels in all
          // 4 cardinal directions.
          *center_ptr += 5;
          *(center_ptr - 1) += 1;
          *(center_ptr + 1) += 1;
          *(center_ptr - frame_width) += 1;
          *(center_ptr + frame_width) += 1;
        }
      }

      ++curr_pixel_ptr;
    }  // x
  }  // y

  timeLog("Found FAST features");

  int32 num_features = 0;
  // Loop through again and Harris filter pixels in the center of clumps.
  // We can shrink the window by 1 pixel on every side.
  for (int32 img_y = FAST_BORDER_BUFFER + 1; img_y < end_y - 1; ++img_y) {
    const int32 start_x = FAST_BORDER_BUFFER + 1;

    const uint8* curr_pixel_ptr =
        best_feature_map->getPixelPtrConst(start_x, img_y);

    for (int32 img_x = start_x; img_x < end_x - 1; ++img_x) {
      if (*curr_pixel_ptr >= MIN_NUM_CONNECTED) {
        Point2D* const feature = features + num_features;
        feature->x = img_x;
        feature->y = img_y;
        feature->score = 0;
        feature->type = FEATURE_FAST;

        ++num_features;
        if (num_features >= max_num_features) {
          return num_features;
        }
      }

      ++curr_pixel_ptr;
    }  // x
  }  // y

  timeLog("Filtered FAST features");
  return num_features;
}

}  // namespace flow
