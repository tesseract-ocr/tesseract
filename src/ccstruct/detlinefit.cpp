///////////////////////////////////////////////////////////////////////
// File:        detlinefit.cpp
// Description: Deterministic least median squares line fitting.
// Author:      Ray Smith
//
// (C) Copyright 2008, Google Inc.
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

#include "detlinefit.h"
#include "helpers.h"        // for IntCastRounded
#include "statistc.h"
#include "tesserrstream.h"  // for tesserr

#include <algorithm>
#include <cfloat> // for FLT_MAX

namespace tesseract {

// The number of points to consider at each end.
const int kNumEndPoints = 3;
// The minimum number of points at which to switch to number of points
// for badly fitted lines.
// To ensure a sensible error metric, kMinPointsForErrorCount should be at
// least kMaxRealDistance / (1 - %ile) where %ile is the fractile used in
// ComputeUpperQuartileError.
const int kMinPointsForErrorCount = 16;
// The maximum real distance to use before switching to number of
// mis-fitted points, which will get square-rooted for true distance.
const int kMaxRealDistance = 2.0;

DetLineFit::DetLineFit() : square_length_(0.0) {}

// Delete all Added points.
void DetLineFit::Clear() {
  pts_.clear();
  distances_.clear();
}

// Add a new point. Takes a copy - the pt doesn't need to stay in scope.
void DetLineFit::Add(const ICOORD &pt) {
  pts_.emplace_back(pt, 0);
}
// Associates a half-width with the given point if a point overlaps the
// previous point by more than half the width, and its distance is further
// than the previous point, then the more distant point is ignored in the
// distance calculation. Useful for ignoring i dots and other diacritics.
void DetLineFit::Add(const ICOORD &pt, int halfwidth) {
  pts_.emplace_back(pt, halfwidth);
}

// Fits a line to the points, ignoring the skip_first initial points and the
// skip_last final points, returning the fitted line as a pair of points,
// and the upper quartile error.
double DetLineFit::Fit(int skip_first, int skip_last, ICOORD *pt1, ICOORD *pt2) {
  // Do something sensible with no points.
  if (pts_.empty()) {
    pt1->set_x(0);
    pt1->set_y(0);
    *pt2 = *pt1;
    return 0.0;
  }
  // Count the points and find the first and last kNumEndPoints.
  int pt_count = pts_.size();
  ICOORD *starts[kNumEndPoints];
  if (skip_first >= pt_count) {
    skip_first = pt_count - 1;
  }
  int start_count = 0;
  int end_i = std::min(skip_first + kNumEndPoints, pt_count);
  for (int i = skip_first; i < end_i; ++i) {
    starts[start_count++] = &pts_[i].pt;
  }
  ICOORD *ends[kNumEndPoints];
  if (skip_last >= pt_count) {
    skip_last = pt_count - 1;
  }
  int end_count = 0;
  end_i = std::max(0, pt_count - kNumEndPoints - skip_last);
  for (int i = pt_count - 1 - skip_last; i >= end_i; --i) {
    ends[end_count++] = &pts_[i].pt;
  }
  // 1 or 2 points need special treatment.
  if (pt_count <= 2) {
    *pt1 = *starts[0];
    if (pt_count > 1) {
      *pt2 = *ends[0];
    } else {
      *pt2 = *pt1;
    }
    return 0.0;
  }
  // Although with between 2 and 2*kNumEndPoints-1 points, there will be
  // overlap in the starts, ends sets, this is OK and taken care of by the
  // if (*start != *end) test below, which also tests for equal input points.
  double best_uq = -1.0;
  // Iterate each pair of points and find the best fitting line.
  for (int i = 0; i < start_count; ++i) {
    ICOORD *start = starts[i];
    for (int j = 0; j < end_count; ++j) {
      ICOORD *end = ends[j];
      if (*start != *end) {
        ComputeDistances(*start, *end);
        // Compute the upper quartile error from the line.
        double dist = EvaluateLineFit();
        if (dist < best_uq || best_uq < 0.0) {
          best_uq = dist;
          *pt1 = *start;
          *pt2 = *end;
        }
      }
    }
  }
  // Finally compute the square root to return the true distance.
  return best_uq > 0.0 ? sqrt(best_uq) : best_uq;
}

// Constrained fit with a supplied direction vector. Finds the best line_pt,
// that is one of the supplied points having the median cross product with
// direction, ignoring points that have a cross product outside of the range
// [min_dist, max_dist]. Returns the resulting error metric using the same
// reduced set of points.
// *Makes use of floating point arithmetic*
double DetLineFit::ConstrainedFit(const FCOORD &direction, double min_dist, double max_dist,
                                  bool debug, ICOORD *line_pt) {
  ComputeConstrainedDistances(direction, min_dist, max_dist);
  // Do something sensible with no points or computed distances.
  if (pts_.empty() || distances_.empty()) {
    line_pt->set_x(0);
    line_pt->set_y(0);
    return 0.0;
  }
  auto median_index = distances_.size() / 2;
  std::nth_element(distances_.begin(), distances_.begin() + median_index, distances_.end());
  *line_pt = distances_[median_index].data();
  if (debug) {
    tesserr << "Constrained fit to dir " << direction.x() << ", "
            << direction.y() << " = "
            << line_pt->x() << ", " << line_pt->y()
            << " :" << distances_.size() << " distances:\n";
    for (unsigned i = 0; i < distances_.size(); ++i) {
      tesserr << i << ": "
              << distances_[i].data().x() << ", "
              << distances_[i].data().y() << " -> "
              << distances_[i].key() << '\n';
    }
    tesserr << "Result = " << median_index << '\n';
  }
  // Center distances on the fitted point.
  double dist_origin = direction * *line_pt;
  for (auto &distance : distances_) {
    distance.key() -= dist_origin;
  }
  return sqrt(EvaluateLineFit());
}

// Returns true if there were enough points at the last call to Fit or
// ConstrainedFit for the fitted points to be used on a badly fitted line.
bool DetLineFit::SufficientPointsForIndependentFit() const {
  return distances_.size() >= kMinPointsForErrorCount;
}

// Backwards compatible fit returning a gradient and constant.
// Deprecated. Prefer Fit(ICOORD*, ICOORD*) where possible, but use this
// function in preference to the LMS class.
double DetLineFit::Fit(float *m, float *c) {
  ICOORD start, end;
  double error = Fit(&start, &end);
  if (end.x() != start.x()) {
    *m = static_cast<float>(end.y() - start.y()) / (end.x() - start.x());
    *c = start.y() - *m * start.x();
  } else {
    *m = 0.0f;
    *c = 0.0f;
  }
  return error;
}

// Backwards compatible constrained fit with a supplied gradient.
// Deprecated. Use ConstrainedFit(const FCOORD& direction) where possible
// to avoid potential difficulties with infinite gradients.
double DetLineFit::ConstrainedFit(double m, float *c) {
  // Do something sensible with no points.
  if (pts_.empty()) {
    *c = 0.0f;
    return 0.0;
  }
  double cos = 1.0 / sqrt(1.0 + m * m);
  FCOORD direction(cos, m * cos);
  ICOORD line_pt;
  double error = ConstrainedFit(direction, -FLT_MAX, FLT_MAX, false, &line_pt);
  *c = line_pt.y() - line_pt.x() * m;
  return error;
}

// Computes and returns the squared evaluation metric for a line fit.
double DetLineFit::EvaluateLineFit() {
  // Compute the upper quartile error from the line.
  double dist = ComputeUpperQuartileError();
  if (distances_.size() >= kMinPointsForErrorCount && dist > kMaxRealDistance * kMaxRealDistance) {
    // Use the number of mis-fitted points as the error metric, as this
    // gives a better measure of fit for badly fitted lines where more
    // than a quarter are badly fitted.
    double threshold = kMaxRealDistance * sqrt(square_length_);
    dist = NumberOfMisfittedPoints(threshold);
  }
  return dist;
}

// Computes the absolute error distances of the points from the line,
// and returns the squared upper-quartile error distance.
double DetLineFit::ComputeUpperQuartileError() {
  int num_errors = distances_.size();
  if (num_errors == 0) {
    return 0.0;
  }
  // Get the absolute values of the errors.
  for (int i = 0; i < num_errors; ++i) {
    if (distances_[i].key() < 0) {
      distances_[i].key() = -distances_[i].key();
    }
  }
  // Now get the upper quartile distance.
  auto index = 3 * num_errors / 4;
  std::nth_element(distances_.begin(), distances_.begin() + index, distances_.end());
  double dist = distances_[index].key();
  // The true distance is the square root of the dist squared / square_length.
  // Don't bother with the square root. Just return the square distance.
  return square_length_ > 0.0 ? dist * dist / square_length_ : 0.0;
}

// Returns the number of sample points that have an error more than threshold.
int DetLineFit::NumberOfMisfittedPoints(double threshold) const {
  int num_misfits = 0;
  int num_dists = distances_.size();
  // Get the absolute values of the errors.
  for (int i = 0; i < num_dists; ++i) {
    if (distances_[i].key() > threshold) {
      ++num_misfits;
    }
  }
  return num_misfits;
}

// Computes all the cross product distances of the points from the line,
// storing the actual (signed) cross products in distances.
// Ignores distances of points that are further away than the previous point,
// and overlaps the previous point by at least half.
void DetLineFit::ComputeDistances(const ICOORD &start, const ICOORD &end) {
  distances_.clear();
  ICOORD line_vector = end;
  line_vector -= start;
  square_length_ = line_vector.sqlength();
  int line_length = IntCastRounded(sqrt(square_length_));
  // Compute the distance of each point from the line.
  int prev_abs_dist = 0;
  int prev_dot = 0;
  for (unsigned i = 0; i < pts_.size(); ++i) {
    ICOORD pt_vector = pts_[i].pt;
    pt_vector -= start;
    int dot = line_vector % pt_vector;
    // Compute |line_vector||pt_vector|sin(angle between)
    int dist = line_vector * pt_vector;
    int abs_dist = dist < 0 ? -dist : dist;
    if (abs_dist > prev_abs_dist && i > 0) {
      // Ignore this point if it overlaps the previous one.
      int separation = abs(dot - prev_dot);
      if (separation < line_length * pts_[i].halfwidth ||
          separation < line_length * pts_[i - 1].halfwidth) {
        continue;
      }
    }
    distances_.emplace_back(dist, pts_[i].pt);
    prev_abs_dist = abs_dist;
    prev_dot = dot;
  }
}

// Computes all the cross product distances of the points perpendicular to
// the given direction, ignoring distances outside of the give distance range,
// storing the actual (signed) cross products in distances_.
void DetLineFit::ComputeConstrainedDistances(const FCOORD &direction, double min_dist,
                                             double max_dist) {
  distances_.clear();
  square_length_ = direction.sqlength();
  // Compute the distance of each point from the line.
  for (auto &pt : pts_) {
    FCOORD pt_vector = pt.pt;
    // Compute |line_vector||pt_vector|sin(angle between)
    double dist = direction * pt_vector;
    if (min_dist <= dist && dist <= max_dist) {
      distances_.emplace_back(dist, pt.pt);
    }
  }
}

} // namespace tesseract.
