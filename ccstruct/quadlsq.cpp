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

#include "mfcpch.h"
#include          <stdio.h>
#include          <math.h>
#include          "errcode.h"
#include          "quadlsq.h"

const ERRCODE EMPTY_QLSQ = "Can't delete from an empty QLSQ";

#define EXTERN

/**********************************************************************
 * QLSQ::clear
 *
 * Function to initialize a QLSQ.
 **********************************************************************/

void QLSQ::clear() {  //initialize
  a = 0;
  b = 0;
  c = 0;
  n = 0;                         //no elements
  sigx = 0;                      //update accumulators
  sigy = 0;
  sigxx = 0;
  sigxy = 0;
  sigyy = 0;
  sigxxx = 0;
  sigxxy = 0;
  sigxxxx = 0;
}


/**********************************************************************
 * QLSQ::add
 *
 * Add an element to the accumulator.
 **********************************************************************/

void QLSQ::add(           //add an element
               double x,  //xcoord
               double y   //ycoord
              ) {
  n++;                           //count elements
  sigx += x;                     //update accumulators
  sigy += y;
  sigxx += x * x;
  sigxy += x * y;
  sigyy += y * y;
  sigxxx += (long double) x *x * x;
  sigxxy += (long double) x *x * y;
  sigxxxx += (long double) x *x * x * x;
}


/**********************************************************************
 * QLSQ::remove
 *
 * Delete an element from the acculuator.
 **********************************************************************/

void QLSQ::remove(           //delete an element
                  double x,  //xcoord
                  double y   //ycoord
                 ) {
  if (n <= 0)
                                 //illegal
    EMPTY_QLSQ.error ("QLSQ::remove", ABORT, NULL);
  n--;                           //count elements
  sigx -= x;                     //update accumulators
  sigy -= y;
  sigxx -= x * x;
  sigxy -= x * y;
  sigyy -= y * y;
  sigxxx -= (long double) x *x * x;
  sigxxy -= (long double) x *x * y;
  sigxxxx -= (long double) x *x * x * x;
}


/**********************************************************************
 * QLSQ::fit
 *
 * Fit the given degree of polynomial and store the result.
 **********************************************************************/

void QLSQ::fit(            //fit polynomial
               int degree  //degree to fit
              ) {
  long double cubetemp;          //intermediates
  long double squaretemp;
  long double top96, bottom96;   /*accurate top & bottom */

  if (n >= 4 && degree >= 2) {
    cubetemp = sigxxx * n - (long double) sigxx *sigx;

    top96 =
      cubetemp * ((long double) sigxy * n - (long double) sigx * sigy);

    squaretemp = (long double) sigxx *n - (long double) sigx *sigx;

    top96 += squaretemp * ((long double) sigxx * sigy - sigxxy * n);

    bottom96 = cubetemp * cubetemp;

    bottom96 -= squaretemp * (sigxxxx * n - (long double) sigxx * sigxx);

    a = top96 / bottom96;

    top96 = ((long double) sigxx * sigx - sigxxx * n) * a
      + (long double) sigxy *n - (long double) sigx *sigy;
    bottom96 = (long double) sigxx *n - (long double) sigx *sigx;
    b = top96 / bottom96;

    c = (sigy - a * sigxx - b * sigx) / n;
  }
  else if (n == 0 || degree < 0) {
    a = b = c = 0;
  }
  else {
    a = 0;
    if (n > 1 && degree > 0) {
      b = (sigxy * n - sigx * sigy) / (sigxx * n - sigx * sigx);
    }
    else
      b = 0;
    c = (sigy - b * sigx) / n;
  }
}
