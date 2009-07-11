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

#include "mfcpch.h"
#include          <stdio.h>
#include          <math.h>
#include          "errcode.h"
#include          "linlsq.h"

#ifndef __UNIX__
#define M_PI        3.14159265359
#endif

const ERRCODE EMPTY_LLSQ = "Can't delete from an empty LLSQ";

#define EXTERN

EXTERN double_VAR (pdlsq_posdir_ratio, 4e-6, "Mult of dir to cf pos");
EXTERN double_VAR (pdlsq_threshold_angleavg, 0.1666666,
"Frac of pi for simple fit");

/**********************************************************************
 * LLSQ::clear
 *
 * Function to initialize a LLSQ.
 **********************************************************************/

void LLSQ::clear() {  //initialize
  n = 0;                         //no elements
  sigx = 0;                      //update accumulators
  sigy = 0;
  sigxx = 0;
  sigxy = 0;
  sigyy = 0;
}


/**********************************************************************
 * LLSQ::add
 *
 * Add an element to the accumulator.
 **********************************************************************/

void LLSQ::add(           //add an element
               double x,  //xcoord
               double y   //ycoord
              ) {
  n++;                           //count elements
  sigx += x;                     //update accumulators
  sigy += y;
  sigxx += x * x;
  sigxy += x * y;
  sigyy += y * y;
}


/**********************************************************************
 * LLSQ::remove
 *
 * Delete an element from the acculuator.
 **********************************************************************/

void LLSQ::remove(           //delete an element
                  double x,  //xcoord
                  double y   //ycoord
                 ) {
  if (n <= 0)
                                 //illegal
    EMPTY_LLSQ.error ("LLSQ::remove", ABORT, NULL);
  n--;                           //count elements
  sigx -= x;                     //update accumulators
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

double LLSQ::m() {  //get gradient
  if (n > 1)
    return (sigxy - sigx * sigy / n) / (sigxx - sigx * sigx / n);
  else
    return 0;                    //too little
}


/**********************************************************************
 * LLSQ::c
 *
 * Return the constant of the line fit.
 **********************************************************************/

double LLSQ::c(          //get constant
               double m  //gradient to fit with
              ) {
  if (n > 0)
    return (sigy - m * sigx) / n;
  else
    return 0;                    //too little
}


/**********************************************************************
 * LLSQ::rms
 *
 * Return the rms error of the fit.
 **********************************************************************/

double LLSQ::rms(           //get error
                 double m,  //gradient to fit with
                 double c   //constant to fit with
                ) {
  double error;                  //total error

  if (n > 0) {
    error =
      sigyy + m * (m * sigxx + 2 * (c * sigx - sigxy)) + c * (n * c -
      2 * sigy);
    if (error >= 0)
      error = sqrt (error / n);  //sqrt of mean
    else
      error = 0;
  }
  else
    error = 0;                   //too little
  return error;
}


/**********************************************************************
 * LLSQ::spearman
 *
 * Return the spearman correlation coefficient.
 **********************************************************************/

double LLSQ::spearman() {  //get error
  double error;                  //total error

  if (n > 1) {
    error = (sigxx - sigx * sigx / n) * (sigyy - sigy * sigy / n);
    if (error > 0) {
      error = (sigxy - sigx * sigy / n) / sqrt (error);
    }
    else
      error = 1;
  }
  else
    error = 1;                   //too little
  return error;
}


/**********************************************************************
 * PDLSQ::fit
 *
 * Return all the parameters of the fit to pos/dir.
 * The return value is the rms error.
 **********************************************************************/

float PDLSQ::fit(                 //get fit
                 DIR128 &ang,     //output angle
                 float &sin_ang,  //r,theta parameterisation
                 float &cos_ang,
                 float &r) {
  double a, b;                   //itermediates
  double angle;                  //resulting angle
  double avg_angle;              //simple average
  double error;                  //total error
  double sinx, cosx;             //return values

  if (pos.n > 0) {
    a = pos.sigxy - pos.sigx * pos.sigy / pos.n
      + pdlsq_posdir_ratio * dir.sigxy;
    b =
      pos.sigxx - pos.sigyy + (pos.sigy * pos.sigy -
      pos.sigx * pos.sigx) / pos.n +
      pdlsq_posdir_ratio * (dir.sigxx - dir.sigyy);
    if (dir.sigy != 0 || dir.sigx != 0)
      avg_angle = atan2 (dir.sigy, dir.sigx);
    else
      avg_angle = 0;
    if ((a != 0 || b != 0) && pos.n > 1)
      angle = atan2 (2 * a, b) / 2;
    else
      angle = avg_angle;
    error = avg_angle - angle;
    if (error > M_PI / 2) {
      error -= M_PI;
      angle += M_PI;
    }
    if (error < -M_PI / 2) {
      error += M_PI;
      angle -= M_PI;
    }
    if (error > M_PI * pdlsq_threshold_angleavg
      || error < -M_PI * pdlsq_threshold_angleavg)
      angle = avg_angle;         //go simple
                                 //convert direction
    ang = (inT16) (angle * MODULUS / (2 * M_PI));
    sinx = sin (angle);
    cosx = cos (angle);
    r = (sinx * pos.sigx - cosx * pos.sigy) / pos.n;
    //              tprintf("x=%g, y=%g, xx=%g, xy=%g, yy=%g, a=%g, b=%g, ang=%g, r=%g\n",
    //                      pos.sigx,pos.sigy,pos.sigxx,pos.sigxy,pos.sigyy,
    //                      a,b,angle,r);
    error = dir.sigxx * sinx * sinx + dir.sigyy * cosx * cosx
      - 2 * dir.sigxy * sinx * cosx;
    error *= pdlsq_posdir_ratio;
    error += sinx * sinx * pos.sigxx + cosx * cosx * pos.sigyy
      - 2 * sinx * cosx * pos.sigxy
      - 2 * r * (sinx * pos.sigx - cosx * pos.sigy) + r * r * pos.n;
    if (error >= 0)
                                 //rms value
        error = sqrt (error / pos.n);
    else
      error = 0;                 //-0
    sin_ang = sinx;
    cos_ang = cosx;
  }
  else {
    sin_ang = 0.0f;
    cos_ang = 0.0f;
    ang = 0;
    error = 0;                   //too little
  }
  return error;
}
