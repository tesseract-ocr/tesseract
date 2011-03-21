/**********************************************************************
 * File:        linlsq.h  (Formerly llsq.h)
 * Description: Linear Least squares fitting code.
 * Author:		Ray Smith
 * Created:		Thu Sep 12 08:44:51 BST 1991
 *
 * (C) Copyright 1991, Hewlett-Packard Ltd.
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

#ifndef TESSERACT_CCSTRUCT_LINLSQ_H_
#define TESSERACT_CCSTRUCT_LINLSQ_H_

#include "points.h"
#include "params.h"

class LLSQ {
 public:
  LLSQ() {  // constructor
    clear();  // set to zeros
  }
  void clear();  // initialize

  // Adds an element with a weight of 1.
  void add(double x, double y);
  // Adds an element with a specified weight.
  void add(double x, double y, double weight);
  // Adds a whole LLSQ.
  void add(const LLSQ& other);
  // Deletes an element with a weight of 1.
  void remove(double x, double y);
  inT32 count() const {  // no of elements
    return static_cast<int>(total_weight + 0.5);
  }

  double m() const;  // get gradient
  double c(double m) const;            // get constant
  double rms(double m, double c) const;            // get error
  double pearson() const;  // get correlation coefficient.

  // Returns the x,y means as an FCOORD.
  FCOORD mean_point() const;
  // Returns the direction of the fitted line as a unit vector, using the
  // least mean squared perpendicular distance. The line runs through the
  // mean_point, i.e. a point p on the line is given by:
  // p = mean_point() + lambda * vector_fit() for some real number lambda.
  // Note that the result (0<=x<=1, -1<=y<=-1) is directionally ambiguous
  // and may be negated without changing its meaning.
  FCOORD vector_fit() const;
  // Returns the covariance.
  double covariance() const {
    if (total_weight > 0.0)
      return (sigxy - sigx * sigy / total_weight) / total_weight;
    else
      return 0.0;
  }
  double x_variance() const {
    if (total_weight > 0.0)
      return (sigxx - sigx * sigx / total_weight) / total_weight;
    else
      return 0.0;
  }
  double y_variance() const {
    if (total_weight > 0.0)
      return (sigyy - sigy * sigy / total_weight) / total_weight;
    else
      return 0.0;
  }

 private:
  double total_weight;         // no of elements or sum of weights.
  double sigx;                 // sum of x
  double sigy;                 // sum of y
  double sigxx;                // sum x squared
  double sigxy;                // sum of xy
  double sigyy;                // sum y squared
};

#endif  // TESSERACT_CCSTRUCT_LINLSQ_H_
