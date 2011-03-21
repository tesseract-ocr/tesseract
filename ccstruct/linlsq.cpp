/**********************************************************************
 * File:        linlsq.cpp  (Formerly llsq.c)
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

#include "mfcpch.h"  // Must be first include for windows.
#include          <stdio.h>
#include          <math.h>
#include          "errcode.h"
#include          "linlsq.h"

const ERRCODE EMPTY_LLSQ = "Can't delete from an empty LLSQ";

/**********************************************************************
 * LLSQ::clear
 *
 * Function to initialize a LLSQ.
 **********************************************************************/

void LLSQ::clear() {  // initialize
  total_weight = 0.0;                         // no elements
  sigx = 0.0;                      // update accumulators
  sigy = 0.0;
  sigxx = 0.0;
  sigxy = 0.0;
  sigyy = 0.0;
}


/**********************************************************************
 * LLSQ::add
 *
 * Add an element to the accumulator.
 **********************************************************************/

void LLSQ::add(double x, double y) {          // add an element
  total_weight++;                           // count elements
  sigx += x;                     // update accumulators
  sigy += y;
  sigxx += x * x;
  sigxy += x * y;
  sigyy += y * y;
}
// Adds an element with a specified weight.
void LLSQ::add(double x, double y, double weight) {
  total_weight += weight;
  sigx += x * weight;                     // update accumulators
  sigy += y * weight;
  sigxx += x * x * weight;
  sigxy += x * y * weight;
  sigyy += y * y * weight;
}
// Adds a whole LLSQ.
void LLSQ::add(const LLSQ& other) {
  total_weight += other.total_weight;
  sigx += other.sigx;                     // update accumulators
  sigy += other.sigy;
  sigxx += other.sigxx;
  sigxy += other.sigxy;
  sigyy += other.sigyy;
}


/**********************************************************************
 * LLSQ::remove
 *
 * Delete an element from the acculuator.
 **********************************************************************/

void LLSQ::remove(double x, double y) {          // delete an element
  if (total_weight <= 0.0)                       // illegal
    EMPTY_LLSQ.error("LLSQ::remove", ABORT, NULL);
  total_weight--;                           // count elements
  sigx -= x;                     // update accumulators
  sigy -= y;
  sigxx -= x * x;
  sigxy -= x * y;
  sigyy -= y * y;
}


/**********************************************************************
 * LLSQ::m
 *
 * Return the gradient of the line fit.
 **********************************************************************/

double LLSQ::m() const {  // get gradient
  double covar = covariance();
  double x_var = x_variance();
  if (x_var != 0.0)
    return covar / x_var;
  else
    return 0.0;                    // too little
}


/**********************************************************************
 * LLSQ::c
 *
 * Return the constant of the line fit.
 **********************************************************************/

double LLSQ::c(double m) const {          // get constant
  if (total_weight > 0.0)
    return (sigy - m * sigx) / total_weight;
  else
    return 0;                    // too little
}


/**********************************************************************
 * LLSQ::rms
 *
 * Return the rms error of the fit.
 **********************************************************************/

double LLSQ::rms(double m,  double c) const {          // get error
  double error;                  // total error

  if (total_weight > 0) {
    error = sigyy + m * (m * sigxx + 2 * (c * sigx - sigxy)) + c *
            (total_weight * c - 2 * sigy);
    if (error >= 0)
      error = sqrt(error / total_weight);  // sqrt of mean
    else
      error = 0;
  } else {
    error = 0;                   // too little
  }
  return error;
}


/**********************************************************************
 * LLSQ::pearson
 *
 * Return the pearson product moment correlation coefficient.
 **********************************************************************/

double LLSQ::pearson() const {  // get correlation
  double r = 0.0;                  // Correlation is 0 if insufficent data.

  double covar = covariance();
  if (covar != 0.0) {
    double var_product = x_variance()  * y_variance();
    if (var_product > 0.0)
      r = covar / sqrt(var_product);
  }
  return r;
}

// Returns the x,y means as an FCOORD.
FCOORD LLSQ::mean_point() const {
  if (total_weight > 0.0) {
    return FCOORD(sigx / total_weight, sigy / total_weight);
  } else {
    return FCOORD(0.0f, 0.0f);
  }
}

// Returns the direction of the fitted line as a unit vector, using the
// least mean squared perpendicular distance. The line runs through the
// mean_point, i.e. a point p on the line is given by:
// p = mean_point() + lambda * vector_fit() for some real number lambda.
// Note that the result (0<=x<=1, -1<=y<=-1) is directionally ambiguous
// and may be negated without changing its meaning.
FCOORD LLSQ::vector_fit() const {
  double x_var = x_variance();
  double y_var = y_variance();
  double covar = covariance();
  FCOORD result;
  if (x_var >= y_var) {
    if (x_var == 0.0)
      return FCOORD(0.0f, 0.0f);
    result.set_x(x_var / sqrt(x_var * x_var + covar * covar));
    result.set_y(sqrt(1.0 - result.x() * result.x()));
  } else {
    result.set_y(y_var / sqrt(y_var * y_var + covar * covar));
    result.set_x(sqrt(1.0 - result.y() * result.y()));
  }
  if (covar < 0.0)
    result.set_y(-result.y());
  return result;
}
