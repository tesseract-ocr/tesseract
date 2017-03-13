// Copyright 2010 Google Inc. All Rights Reserved.
// Author: andrewharp@google.com (Andrew Harp)

#ifndef JAVA_COM_GOOGLE_ANDROID_APPS_UNVEIL_JNI_OPTICALFLOW_OPTICAL_FLOW_UTILS_H_
#define JAVA_COM_GOOGLE_ANDROID_APPS_UNVEIL_JNI_OPTICALFLOW_OPTICAL_FLOW_UTILS_H_

#include "utils.h"

namespace flow {

// Arbitrary feature type ids for labeling the origin of tracked features.
#define FEATURE_DEFAULT 0
#define FEATURE_FAST 1
#define FEATURE_INTEREST 2

#define EPSILON 0.0000001f

class Size {
 public:
  int32 width;
  int32 height;

  Size(const int32 width, const int32 height) : width(width), height(height) {}
};

// For keeping track of features.
class Point2D {
 public:
  float32 x;
  float32 y;
  float32 score;
  uint8 type;

  Point2D() : x(0.0f), y(0.0f), score(0.0f), type(0) {}
  Point2D(const float32 x, const float32 y) :
    x(x), y(y), score(0.0f), type(0) {}

  // The following operators all compare the computed score of the feature for
  // sorting purposes.
  // TODO(andrewharp): Use the qsort function in stdlib.
  inline bool operator< (const Point2D& that) const {
    return this->score < that.score;
  }

  inline bool operator<= (const Point2D& that) const {
    return this->score <= that.score;
  }

  inline bool operator> (const Point2D& that) const {
    return this->score > that.score;
  }

  inline Point2D operator- (const Point2D& that) const {
    return Point2D(this->x - that.x, this->y - that.y);
  }
};

// Returns the determinant of a 2x2 matrix.
inline float findDeterminant2x2(const float32* const a) {
  // Determinant: (ad - bc)
  return a[0] * a[3] - a[1] * a[2];
}

// Finds the inverse of a 2x2 matrix.
// Returns true upon success, false if the matrix is not invertible.
inline bool invert2x2(const float32* const a, float32* const a_inv) {
  const float det = findDeterminant2x2(a);
  if (abs(det) < EPSILON) {
    return false;
  }
  const float32 inv_det = 1.0f / static_cast<float32>(det);

  a_inv[0] = inv_det * a[3];  // d
  a_inv[1] = inv_det * -a[1];  // -b
  a_inv[2] = inv_det * -a[2];  // -c
  a_inv[3] = inv_det * a[0];  // a

  return true;
}

// TODO(andrewharp): Accelerate with NEON.
inline float32 computeMean(const float32* const values,
                           const int32 num_vals) {
  // Get mean.
  float32 sum = 0.0f;
  for (int32 i = 0; i < num_vals; ++i) {
    sum += values[i];
  }
  return sum / static_cast<float32>(num_vals);
}

// TODO(andrewharp): Accelerate with NEON.
inline float32 computeStdDev(const float32* const values,
                             const int32 num_vals,
                             const float32 mean) {
  // Get Std dev.
  float32 squared_sum = 0.0f;
  for (int32 i = 0; i < num_vals; ++i) {
    squared_sum += square(values[i] - mean);
  }
  return sqrt(squared_sum / static_cast<float32>(num_vals));
}

// TODO(andrewharp): Accelerate with NEON.
inline float32 computeWeightedMean(const float32* const values,
                                   const float32* const weights,
                                   const int32 num_vals) {
  float32 sum = 0.0f;
  float32 total_weight = 0.0f;
  for (int32 i = 0; i < num_vals; ++i) {
    sum += values[i] * weights[i];
    total_weight += weights[i];
  }
  return sum / num_vals;
}

// Partitioning phase of quicksort.
template<typename T>
inline int32 partition(T* const arr_start,
                       const int32 num_elems, const T pivot) {
  int32 i = 0;
  int32 j = num_elems - 1;

  // Put everything <= pivot on the left, and everything > pivot to the right.
  while (true) {
    while (arr_start[i] <= pivot && (i < j)) {
      ++i;
    }

    while (arr_start[j] > pivot && (i < j)) {
      --j;
    }

    // Termination condition.
    if (i >= j) {
      break;
    }

    swap(arr_start + i, arr_start + j);
    ++i;
    --j;
  }

  int32 part_size;
  for (part_size = 0; part_size < num_elems; ++part_size) {
    if (arr_start[part_size] > pivot) {
      break;
    }
  }
  return part_size;
}

// Just your basic quicksort implementation.
// Sorts an array of size num_elems inplace by the score field of the elements.
template<typename T>
void qsort(T* const arr_start, const int32 num_elems) {
  // No point in sorting a list of size 1 or 0!
  if (num_elems <= 1) {
    return;
  }

  // Check to see if we're already sorted...
  bool sorted = true;
  T last_score = arr_start[0];
  for (int32 i = 1; i < num_elems; ++i) {
    const T curr_score = arr_start[i];
    if (last_score > curr_score) {
      sorted = false;
      break;
    }
    last_score = curr_score;
  }
  if (sorted) {
    return;
  }

  // Select partition element randomly.
  const int32 sort_ind = rand() % num_elems;
  const T pivot = arr_start[sort_ind];

  const int32 first_part_size = partition(arr_start, num_elems, pivot);
  const int32 second_part_size = num_elems - first_part_size;

  qsort(arr_start, first_part_size);
  qsort(arr_start + first_part_size, second_part_size);
}

}  // namespace flow

#endif // JAVA_COM_GOOGLE_ANDROID_APPS_UNVEIL_JNI_OPTICALFLOW_OPTICAL_FLOW_UTILS_H_
