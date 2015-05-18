/**********************************************************************
 * File:        quadlsq.cpp  (Formerly qlsq.c)
 * Description: Code for least squares approximation of quadratics.
 * Author:		Ray Smith
 * Created:		Wed Oct  6 15:14:23 BST 1993
 *
 * (C) Copyright 1993, Hewlett-Packard Ltd.
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

#include <stdio.h>
#include <math.h>
#include "quadlsq.h"
#include "tprintf.h"

// Minimum variance in least squares before backing off to a lower degree.
const double kMinVariance = 1.0 / 1024;

/**********************************************************************
 * QLSQ::clear
 *
 * Function to initialize a QLSQ.
 **********************************************************************/

void QLSQ::clear() {  // initialize
  a = 0.0;
  b = 0.0;
  c = 0.0;
  n = 0;                           // No elements.
  sigx = 0.0;                      // Zero accumulators.
  sigy = 0.0;
  sigxx = 0.0;
  sigxy = 0.0;
  sigyy = 0.0;
  sigxxx = 0.0;
  sigxxy = 0.0;
  sigxxxx = 0.0;
}


/**********************************************************************
 * QLSQ::add
 *
 * Add an element to the accumulator.
 **********************************************************************/

void QLSQ::add(double x, double y) {
  n++;                           // Count elements.
  sigx += x;                     // Update accumulators.
  sigy += y;
  sigxx += x * x;
  sigxy += x * y;
  sigyy += y * y;
  sigxxx += static_cast<long double>(x) * x * x;
  sigxxy += static_cast<long double>(x) * x * y;
  sigxxxx += static_cast<long double>(x) * x * x * x;
}


/**********************************************************************
 * QLSQ::remove
 *
 * Delete an element from the accumulator.
 **********************************************************************/

void QLSQ::remove(double x, double y) {
  if (n <= 0) {
    tprintf("Can't remove an element from an empty QLSQ accumulator!\n");
    return;
  }
  n--;                           // Count elements.
  sigx -= x;                     // Update accumulators.
  sigy -= y;
  sigxx -= x * x;
  sigxy -= x * y;
  sigyy -= y * y;
  sigxxx -= static_cast<long double>(x) * x * x;
  sigxxy -= static_cast<long double>(x) * x * y;
  sigxxxx -= static_cast<long double>(x) * x * x * x;
}


/**********************************************************************
 * QLSQ::fit
 *
 * Fit the given degree of polynomial and store the result.
 * This creates a quadratic of the form axx + bx + c, but limited to
 * the given degree.
 **********************************************************************/

void QLSQ::fit(int degree) {
  long double x_variance = static_cast<long double>(sigxx) * n -
      static_cast<long double>(sigx) * sigx;

  // Note: for computational efficiency, we do not normalize the variance,
  // covariance and cube variance here as they are in the same order in both
  // nominators and denominators. However, we need be careful in value range
  // check.
  if (x_variance < kMinVariance * n * n || degree < 1 || n < 2) {
    // We cannot calculate b reliably so forget a and b, and just work on c.
    a = b = 0.0;
    if (n >= 1 && degree >= 0) {
      c = sigy / n;
    } else {
      c = 0.0;
    }
    return;
  }
  long double top96 = 0.0;       // Accurate top.
  long double bottom96 = 0.0;    // Accurate bottom.
  long double cubevar = sigxxx * n - static_cast<long double>(sigxx) * sigx;
  long double covariance = static_cast<long double>(sigxy) * n -
      static_cast<long double>(sigx) * sigy;

  if (n >= 4 && degree >= 2) {
    top96 = cubevar * covariance;
    top96 += x_variance * (static_cast<long double>(sigxx) * sigy - sigxxy * n);

    bottom96 = cubevar * cubevar;
    bottom96 -= x_variance *
        (sigxxxx * n - static_cast<long double>(sigxx) * sigxx);
  }
  if (bottom96 >= kMinVariance * n * n * n * n) {
    // Denominators looking good
    a = top96 / bottom96;
    top96 = covariance - cubevar * a;
    b = top96 / x_variance;
  } else {
    // Forget a, and concentrate on b.
    a = 0.0;
    b = covariance / x_variance;
  }
  c = (sigy - a * sigxx - b * sigx) / n;
}
