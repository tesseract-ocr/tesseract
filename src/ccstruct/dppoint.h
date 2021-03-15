/**********************************************************************
 * File:        dppoint.h
 * Description: Simple generic dynamic programming class.
 * Author:      Ray Smith
 * Created:     Wed Mar 25 18:57:01 PDT 2009
 *
 * (C) Copyright 2009, Google Inc.
 ** Licensed under the Apache License, Version 2.0 (the "License");
 ** you may not use this file except in compliance with the License.
 ** You may obtain a copy of the License at
 ** http://www.apache.org/licenses/LICENSE-2.0
 ** Unless required by applicable law or agreed to in writing, software
 ** distributed under the License is distributed on an "AS IS" BASIS,
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 ** See the License for the specific language governing permissions and
 ** limitations under the License.
 *
 **********************************************************************/

#ifndef TESSERACT_CCSTRUCT_DPPOINT_H_
#define TESSERACT_CCSTRUCT_DPPOINT_H_

#include <cstdint>

namespace tesseract {

// A simple class to provide a dynamic programming solution to a class of
// 1st-order problems in which the cost is dependent only on the current
// step and the best cost to that step, with a possible special case
// of using the variance of the steps, and only the top choice is required.
// Useful for problems such as finding the optimal cut points in a fixed-pitch
// (vertical or horizontal) situation.
// Skeletal Example:
// DPPoint* array = new DPPoint[width];
// for (int i = 0; i < width; i++) {
//   array[i].AddLocalCost(cost_at_i)
// }
// DPPoint* best_end = DPPoint::Solve(..., array);
// while (best_end != nullptr) {
//   int cut_index = best_end - array;
//   best_end = best_end->best_prev();
// }
// delete [] array;
class DPPoint {
public:
  // The cost function evaluates the total cost at this (excluding this's
  // local_cost) and if it beats this's total_cost, then
  // replace the appropriate values in this.
  using CostFunc = int64_t (DPPoint::*)(const DPPoint *);

  DPPoint()
      : local_cost_(0)
      , total_cost_(INT32_MAX)
      , total_steps_(1)
      , best_prev_(nullptr)
      , n_(0)
      , sig_x_(0)
      , sig_xsq_(0) {}

  // Solve the dynamic programming problem for the given array of points, with
  // the given size and cost function.
  // Steps backwards are limited to being between min_step and max_step
  // inclusive.
  // The return value is the tail of the best path.
  static DPPoint *Solve(int min_step, int max_step, bool debug, CostFunc cost_func, int size,
                        DPPoint *points);

  // A CostFunc that takes the variance of step into account in the cost.
  int64_t CostWithVariance(const DPPoint *prev);

  // Accessors.
  int total_cost() const {
    return total_cost_;
  }
  int Pathlength() const {
    return total_steps_;
  }
  const DPPoint *best_prev() const {
    return best_prev_;
  }
  void AddLocalCost(int new_cost) {
    local_cost_ += new_cost;
  }

private:
  // Code common to different cost functions.

  // Update the other members if the cost is lower.
  void UpdateIfBetter(int64_t cost, int32_t steps, const DPPoint *prev, int32_t n, int32_t sig_x,
                      int64_t sig_xsq);

  int32_t local_cost_;       // Cost of this point on its own.
  int32_t total_cost_;       // Sum of all costs in best path to here.
                             // During cost calculations local_cost is excluded.
  int32_t total_steps_;      // Number of steps in best path to here.
  const DPPoint *best_prev_; // Pointer to prev point in best path from here.
  // Information for computing the variance part of the cost.
  int32_t n_;       // Number of steps in best path to here for variance.
  int32_t sig_x_;   // Sum of step sizes for computing variance.
  int64_t sig_xsq_; // Sum of squares of steps for computing variance.
};

} // namespace tesseract.

#endif // TESSERACT_CCSTRUCT_DPPOINT_H_
