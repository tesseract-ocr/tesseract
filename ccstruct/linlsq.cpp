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

// Returns the sqrt of the mean squared error measured perpendicular from the
// line through mean_point() in the direction dir.
//
// Derivation:
//   Lemma:  Let v and x_i (i=1..N) be a k-dimensional vectors (1xk matrices).
//     Let % be dot product and ' be transpose.  Note that:
//      Sum[i=1..N] (v % x_i)^2
//         = v * [x_1' x_2' ... x_N'] * [x_1' x_2' .. x_N']' * v'
//     If x_i have average 0 we have:
//       = v * (N * COVARIANCE_MATRIX(X)) * v'
//     Expanded for the case that k = 2, where we treat the dimensions
//     as x_i and y_i, this is:
//       = v * (N * [VAR(X), COV(X,Y); COV(X,Y) VAR(Y)]) * v'
//  Now, we are trying to calculate the mean squared error, where v is
//  perpendicular to our line of interest:
//    Mean squared error
//      = E [ (v % (x_i - x_avg))) ^2 ]
//      = Sum (v % (x_i - x_avg))^2 / N
//      = v * N * [VAR(X) COV(X,Y); COV(X,Y) VAR(Y)] / N * v'
//      = v * [VAR(X) COV(X,Y); COV(X,Y) VAR(Y)] * v'
//      = code below
double LLSQ::rms_orth(const FCOORD &dir) const {
  FCOORD v = !dir;
  v.normalise();
  return sqrt(v.x() * v.x() * x_variance() +
              2 * v.x() * v.y() * covariance() +
              v.y() * v.y() * y_variance());
}

// Returns the direction of the fitted line as a unit vector, using the
// least mean squared perpendicular distance. The line runs through the
// mean_point, i.e. a point p on the line is given by:
// p = mean_point() + lambda * vector_fit() for some real number lambda.
// Note that the result (0<=x<=1, -1<=y<=-1) is directionally ambiguous
// and may be negated without changing its meaning.
// Fitting a line m + 𝜆v to a set of N points Pi = (xi, yi), where
// m is the mean point (𝝁, 𝝂) and
// v is the direction vector (cos𝜃, sin𝜃)
// The perpendicular distance of each Pi from the line is:
// (Pi - m) x v, where x is the scalar cross product.
// Total squared error is thus:
// E = ∑((xi - 𝝁)sin𝜃 - (yi - 𝝂)cos𝜃)²
//   = ∑(xi - 𝝁)²sin²𝜃  - 2∑(xi - 𝝁)(yi - 𝝂)sin𝜃 cos𝜃 + ∑(yi - 𝝂)²cos²𝜃
//   = NVar(xi)sin²𝜃  - 2NCovar(xi, yi)sin𝜃 cos𝜃  + NVar(yi)cos²𝜃   (Eq 1)
// where Var(xi) is the variance of xi,
// and Covar(xi, yi) is the covariance of xi, yi.
// Taking the derivative wrt 𝜃 and setting to 0 to obtain the min/max:
// 0 = 2NVar(xi)sin𝜃 cos𝜃 -2NCovar(xi, yi)(cos²𝜃 - sin²𝜃) -2NVar(yi)sin𝜃 cos𝜃
// => Covar(xi, yi)(cos²𝜃 - sin²𝜃) = (Var(xi) - Var(yi))sin𝜃 cos𝜃
// Using double angles:
// 2Covar(xi, yi)cos2𝜃 = (Var(xi) - Var(yi))sin2𝜃   (Eq 2)
// So 𝜃 = 0.5 atan2(2Covar(xi, yi), Var(xi) - Var(yi)) (Eq 3)

// Because it involves 2𝜃 , Eq 2 has 2 solutions 90 degrees apart, but which
// is the min and which is the max? From Eq1:
// E/N = Var(xi)sin²𝜃  - 2Covar(xi, yi)sin𝜃 cos𝜃  + Var(yi)cos²𝜃
// and 90 degrees away, using sin/cos equivalences:
// E'/N = Var(xi)cos²𝜃  + 2Covar(xi, yi)sin𝜃 cos𝜃  + Var(yi)sin²𝜃
// The second error is smaller (making it the minimum) iff
// E'/N < E/N ie:
// (Var(xi) - Var(yi))(cos²𝜃 - sin²𝜃) < -4Covar(xi, yi)sin𝜃 cos𝜃
// Using double angles:
// (Var(xi) - Var(yi))cos2𝜃  < -2Covar(xi, yi)sin2𝜃  (InEq 1)
// But atan2(2Covar(xi, yi), Var(xi) - Var(yi)) picks 2𝜃  such that:
// sgn(cos2𝜃) = sgn(Var(xi) - Var(yi)) and sgn(sin2𝜃) = sgn(Covar(xi, yi))
// so InEq1 can *never* be true, making the atan2 result *always* the min!
// In the degenerate case, where Covar(xi, yi) = 0 AND Var(xi) = Var(yi),
// the 2 solutions have equal error and the inequality is still false.
// Therefore the solution really is as trivial as Eq 3.

// This is equivalent to returning the Principal Component in PCA, or the
// eigenvector corresponding to the largest eigenvalue in the covariance
// matrix.  However, atan2 is much simpler! The one reference I found that
// uses this formula is http://web.mit.edu/18.06/www/Essays/tlsfit.pdf but
// that is still a much more complex derivation. It seems Pearson had already
// found this simple solution in 1901.
// http://books.google.com/books?id=WXwvAQAAIAAJ&pg=PA559
FCOORD LLSQ::vector_fit() const {
  double x_var = x_variance();
  double y_var = y_variance();
  double covar = covariance();
  double theta = 0.5 * atan2(2.0 * covar, x_var - y_var);
  FCOORD result(cos(theta), sin(theta));
  return result;
}
