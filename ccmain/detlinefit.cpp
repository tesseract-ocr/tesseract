///////////////////////////////////////////////////////////////////////
// File:        detlinefit.cpp
// Description: Deterministic least median squares line fitting.
// Author:      Ray Smith
// Created:     Thu Feb 28 14:45:01 PDT 2008
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
#include "statistc.h"
#include "ndminx.h"

namespace tesseract {

// The number of points to consider at each end.
const int kNumEndPoints = 3;

DetLineFit::DetLineFit() {
}

DetLineFit::~DetLineFit() {
}

// Delete all Added points.
void DetLineFit::Clear() {
  pt_list_.clear();
}

// Add a new point. Takes a copy - the pt doesn't need to stay in scope.
void DetLineFit::Add(const ICOORD& pt) {
  ICOORDELT_IT it = &pt_list_;
  ICOORDELT* new_pt = new ICOORDELT(pt);
  it.add_to_end(new_pt);
}

// Fit a line to the points, returning the fitted line as a pair of
// points, and the upper quartile error.
double DetLineFit::Fit(ICOORD* pt1, ICOORD* pt2) {
  ICOORDELT_IT it(&pt_list_);
  // Do something sensible with no points.
  if (pt_list_.empty()) {
    pt1->set_x(0);
    pt1->set_y(0);
    *pt2 = *pt1;
    return 0.0;
  }
  // Count the points and find the first and last kNumEndPoints.
  ICOORD* starts[kNumEndPoints];
  ICOORD* ends[kNumEndPoints];
  int pt_count = 0;
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    if (pt_count < kNumEndPoints) {
      starts[pt_count] = it.data();
      ends[pt_count] = starts[pt_count];
    } else {
      for (int i = 1; i < kNumEndPoints; ++i)
        ends[i - 1] = ends[i];
      ends[kNumEndPoints - 1] = it.data();
    }
    ++pt_count;
  }
  // 1 or 2 points need special treatment.
  if (pt_count <= 2) {
    *pt1 = *starts[0];
    if (pt_count > 1)
      *pt2 = *starts[1];
    else
      *pt2 = *pt1;
    return 0.0;
  }
  int end_count = MIN(pt_count, kNumEndPoints);
  int* distances = new int[pt_count];
  double best_uq = -1.0;
  // Iterate each pair of points and find the best fitting line.
  for (int i = 0; i < end_count; ++i) {
    ICOORD* start = starts[i];
    for (int j = 0; j < end_count; ++j) {
      ICOORD* end = ends[j];
      if (start != end) {
        // Compute the upper quartile error from the line.
        double dist = ComputeErrors(*start, *end, distances);
        if (dist < best_uq || best_uq < 0.0) {
          best_uq = dist;
          *pt1 = *start;
          *pt2 = *end;
        }
      }
    }
  }
  delete [] distances;
  // Finally compute the square root to return the true distance.
  return best_uq > 0.0 ? sqrt(best_uq) : best_uq;
}

// Comparator function used by the nth_item funtion.
static int CompareInts(const void *p1, const void *p2) {
  const int* i1 = reinterpret_cast<const int*>(p1);
  const int* i2 = reinterpret_cast<const int*>(p2);

  return *i1 - *i2;
}

// Compute all the cross product distances of the points from the line
// and return the true squared upper quartile distance.
double DetLineFit::ComputeErrors(const ICOORD start, const ICOORD end,
                                 int* distances) {
  ICOORDELT_IT it(&pt_list_);
  ICOORD line_vector = end;
  line_vector -= start;
  // Compute the distance of each point from the line.
  int pt_index = 0;
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    ICOORD pt_vector = *it.data();
    pt_vector -= start;
    // Compute |line_vector||pt_vector|sin(angle between)
    int dist = line_vector * pt_vector;
    if (dist < 0)
      dist = -dist;
    distances[pt_index++] = dist;
  }
  // Now get the upper quartile distance.
  int index = choose_nth_item(3 * pt_index / 4, distances, pt_index,
                              sizeof(distances[0]), CompareInts);
  double dist = distances[index];
  // The true distance is the square root of the dist squared / the
  // squared length of line_vector (which is the dot product with itself)
  // Don't bother with the square root. Just return the square distance.
  return dist * dist / (line_vector % line_vector);
}

}  // namespace tesseract.


