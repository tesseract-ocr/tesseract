///////////////////////////////////////////////////////////////////////
// File:        detlinefit.h
// Description: Deterministic least upper-quartile squares line fitting.
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

#ifndef TESSERACT_CCSTRUCT_DETLINEFIT_H_
#define TESSERACT_CCSTRUCT_DETLINEFIT_H_

#include "kdpair.h"
#include "points.h"

namespace tesseract {

// This class fits a line to a set of ICOORD points.
// There is no restriction on the direction of the line, as it
// uses a vector method, ie no concern over infinite gradients.
// The fitted line has the least upper quartile of squares of perpendicular
// distances of all source points from the line, subject to the constraint
// that the line is made from one of the pairs of [{p1,p2,p3},{pn-2, pn-1, pn}]
// i.e. the 9 combinations of one of the first 3 and last 3 points.
// A fundamental assumption of this algorithm is that one of the first 3 and
// one of the last 3 points are near the best line fit.
// The points must be Added in line order for the algorithm to work properly.
// No floating point calculations are needed* to make an accurate fit,
// and no random numbers are needed** so the algorithm is deterministic,
// architecture-stable, and compiler-stable as well as stable to minor
// changes in the input.
// *A single floating point division is used to compute each line's distance.
// This is unlikely to result in choice of a different line, but if it does,
// it would be easy to replace with a 64 bit integer calculation.
// **Random numbers are used in the nth_item function, but the worst
// non-determinism that can result is picking a different result among equals,
// and that wouldn't make any difference to the end-result distance, so the
// randomness does not affect the determinism of the algorithm. The random
// numbers are only there to guarantee average linear time.
// Fitting time is linear, but with a high constant, as it tries 9 different
// lines and computes the distance of all points each time.
// This class is aimed at replacing the LLSQ (linear least squares) and
// LMS (least median of squares) classes that are currently used for most
// of the line fitting in Tesseract.
class DetLineFit {
public:
  DetLineFit();
  ~DetLineFit() = default;

  // Delete all Added points.
  void Clear();

  // Adds a new point. Takes a copy - the pt doesn't need to stay in scope.
  // Add must be called on points in sequence along the line.
  void Add(const ICOORD &pt);
  // Associates a half-width with the given point if a point overlaps the
  // previous point by more than half the width, and its distance is further
  // than the previous point, then the more distant point is ignored in the
  // distance calculation. Useful for ignoring i dots and other diacritics.
  void Add(const ICOORD &pt, int halfwidth);

  // Fits a line to the points, returning the fitted line as a pair of
  // points, and the upper quartile error.
  double Fit(ICOORD *pt1, ICOORD *pt2) {
    return Fit(0, 0, pt1, pt2);
  }
  // Fits a line to the points, ignoring the skip_first initial points and the
  // skip_last final points, returning the fitted line as a pair of points,
  // and the upper quartile error.
  double Fit(int skip_first, int skip_last, ICOORD *pt1, ICOORD *pt2);

  // Constrained fit with a supplied direction vector. Finds the best line_pt,
  // that is one of the supplied points having the median cross product with
  // direction, ignoring points that have a cross product outside of the range
  // [min_dist, max_dist]. Returns the resulting error metric using the same
  // reduced set of points.
  // *Makes use of floating point arithmetic*
  double ConstrainedFit(const FCOORD &direction, double min_dist, double max_dist, bool debug,
                        ICOORD *line_pt);

  // Returns true if there were enough points at the last call to Fit or
  // ConstrainedFit for the fitted points to be used on a badly fitted line.
  bool SufficientPointsForIndependentFit() const;

  // Backwards compatible fit returning a gradient and constant.
  // Deprecated. Prefer Fit(ICOORD*, ICOORD*) where possible, but use this
  // function in preference to the LMS class.
  double Fit(float *m, float *c);

  // Backwards compatible constrained fit with a supplied gradient.
  // Deprecated. Use ConstrainedFit(const FCOORD& direction) where possible
  // to avoid potential difficulties with infinite gradients.
  double ConstrainedFit(double m, float *c);

private:
  // Simple struct to hold an ICOORD point and a halfwidth representing half
  // the "width" (supposedly approximately parallel to the direction of the
  // line) of each point, such that distant points can be discarded when they
  // overlap nearer points. (Think i dot and other diacritics or noise.)
  struct PointWidth {
    PointWidth() : pt(ICOORD(0, 0)), halfwidth(0) {}
    PointWidth(const ICOORD &pt0, int halfwidth0) : pt(pt0), halfwidth(halfwidth0) {}

    ICOORD pt;
    int halfwidth;
  };
  // Type holds the distance of each point from the fitted line and the point
  // itself. Use of double allows integer distances from ICOORDs to be stored
  // exactly, and also the floating point results from ConstrainedFit.
  using DistPointPair = KDPairInc<double, ICOORD>;

  // Computes and returns the squared evaluation metric for a line fit.
  double EvaluateLineFit();

  // Computes the absolute values of the precomputed distances_,
  // and returns the squared upper-quartile error distance.
  double ComputeUpperQuartileError();

  // Returns the number of sample points that have an error more than threshold.
  int NumberOfMisfittedPoints(double threshold) const;

  // Computes all the cross product distances of the points from the line,
  // storing the actual (signed) cross products in distances_.
  // Ignores distances of points that are further away than the previous point,
  // and overlaps the previous point by at least half.
  void ComputeDistances(const ICOORD &start, const ICOORD &end);

  // Computes all the cross product distances of the points perpendicular to
  // the given direction, ignoring distances outside of the give distance range,
  // storing the actual (signed) cross products in distances_.
  void ComputeConstrainedDistances(const FCOORD &direction, double min_dist, double max_dist);

  // Stores all the source points in the order they were given and their
  // halfwidths, if any.
  std::vector<PointWidth> pts_;
  // Stores the computed perpendicular distances of (some of) the pts_ from a
  // given vector (assuming it goes through the origin, making it a line).
  // Since the distances may be a subset of the input points, and get
  // re-ordered by the nth_item function, the original point is stored
  // along side the distance.
  std::vector<DistPointPair> distances_; // Distances of points.
  // The squared length of the vector used to compute distances_.
  double square_length_;
};

} // namespace tesseract.

#endif // TESSERACT_CCSTRUCT_DETLINEFIT_H_
