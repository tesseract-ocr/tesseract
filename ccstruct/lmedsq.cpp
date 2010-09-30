/**********************************************************************
 * File:        lmedsq.cpp  (Formerly lms.c)
 * Description: Code for the LMS class.
 * Author:		Ray Smith
 * Created:		Fri Aug  7 09:30:53 BST 1992
 *
 * (C) Copyright 1992, Hewlett-Packard Ltd.
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
#include          <stdlib.h>
#include          "statistc.h"
#include          "memry.h"
#include          "statistc.h"
#include          "lmedsq.h"

// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif

#define EXTERN

EXTERN INT_VAR (lms_line_trials, 12, "Number of linew fits to do");
#define SEED1       0x1234       //default seeds
#define SEED2       0x5678
#define SEED3       0x9abc
#define LMS_MAX_FAILURES  3

#ifndef __UNIX__
uinT32 nrand48(               //get random number
               uinT16 *seeds  //seeds to use
              ) {
  static uinT32 seed = 0;        //only seed

  if (seed == 0) {
    seed = seeds[0] ^ (seeds[1] << 8) ^ (seeds[2] << 16);
    srand(seed);
  }
                                 //make 32 bit one
  return rand () | (rand () << 16);
}
#endif

/**********************************************************************
 * LMS::LMS
 *
 * Construct a LMS class, given the max no of samples to be given
 **********************************************************************/

LMS::LMS (                       //constructor
inT32 size                       //samplesize
):samplesize (size) {
  samplecount = 0;
  a = 0;
  m = 0.0f;
  c = 0.0f;
  samples = (FCOORD *) alloc_mem (size * sizeof (FCOORD));
  errors = (float *) alloc_mem (size * sizeof (float));
  line_error = 0.0f;
  fitted = FALSE;
}


/**********************************************************************
 * LMS::~LMS
 *
 * Destruct a LMS class.
 **********************************************************************/

LMS::~LMS (                      //constructor
) {
  free_mem(samples);
  free_mem(errors);
}


/**********************************************************************
 * LMS::clear
 *
 * Clear samples from array.
 **********************************************************************/

void LMS::clear() {  //clear sample
  samplecount = 0;
  fitted = FALSE;
}


/**********************************************************************
 * LMS::add
 *
 * Add another sample. More than the constructed number will be ignored.
 **********************************************************************/

void LMS::add(               //add sample
              FCOORD sample  //sample coords
             ) {
  if (samplecount < samplesize)
                                 //save it
    samples[samplecount++] = sample;
  fitted = FALSE;
}


/**********************************************************************
 * LMS::fit
 *
 * Fit a line to the given sample points.
 **********************************************************************/

void LMS::fit(               //fit sample
              float &out_m,  //output line
              float &out_c) {
  inT32 index;                   //of median
  inT32 trials;                  //no of medians
  float test_m, test_c;          //candidate line
  float test_error;              //error of test line

  switch (samplecount) {
    case 0:
      m = 0.0f;                  //no info
      c = 0.0f;
      line_error = 0.0f;
      break;

    case 1:
      m = 0.0f;
      c = samples[0].y ();       //horiz thru pt
      line_error = 0.0f;
      break;

    case 2:
      if (samples[0].x () != samples[1].x ()) {
        m = (samples[1].y () - samples[0].y ())
          / (samples[1].x () - samples[0].x ());
        c = samples[0].y () - m * samples[0].x ();
      }
      else {
        m = 0.0f;
        c = (samples[0].y () + samples[1].y ()) / 2;
      }
      line_error = 0.0f;
      break;

    default:
      pick_line(m, c);  //use pts at random
      compute_errors(m, c);  //from given line
      index = choose_nth_item (samplecount / 2, errors, samplecount);
      line_error = errors[index];
      for (trials = 1; trials < lms_line_trials; trials++) {
                                 //random again
        pick_line(test_m, test_c);
        compute_errors(test_m, test_c);
        index = choose_nth_item (samplecount / 2, errors, samplecount);
        test_error = errors[index];
        if (test_error < line_error) {
                                 //find least median
          line_error = test_error;
          m = test_m;
          c = test_c;
        }
      }
  }
  fitted = TRUE;
  out_m = m;
  out_c = c;
  a = 0;
}


/**********************************************************************
 * LMS::fit_quadratic
 *
 * Fit a quadratic to the given sample points.
 **********************************************************************/

void LMS::fit_quadratic(                          //fit sample
                        float outlier_threshold,  //min outlier size
                        double &out_a,            //x squared
                        float &out_b,             //output line
                        float &out_c) {
  inT32 trials;                  //no of medians
  double test_a;
  float test_b, test_c;          //candidate line
  float test_error;              //error of test line

  if (samplecount < 3) {
    out_a = 0;
    fit(out_b, out_c);
    return;
  }
  pick_quadratic(a, m, c);
  line_error = compute_quadratic_errors (outlier_threshold, a, m, c);
  for (trials = 1; trials < lms_line_trials * 2; trials++) {
    pick_quadratic(test_a, test_b, test_c);
    test_error = compute_quadratic_errors (outlier_threshold,
      test_a, test_b, test_c);
    if (test_error < line_error) {
      line_error = test_error;   //find least median
      a = test_a;
      m = test_b;
      c = test_c;
    }
  }
  fitted = TRUE;
  out_a = a;
  out_b = m;
  out_c = c;
}


/**********************************************************************
 * LMS::constrained_fit
 *
 * Fit a line to the given sample points.
 * The line must have the given gradient.
 **********************************************************************/

void LMS::constrained_fit(                //fit sample
                          float fixed_m,  //forced gradient
                          float &out_c) {
  inT32 index;                   //of median
  inT32 trials;                  //no of medians
  float test_c;                  //candidate line
  static uinT16 seeds[3] = { SEED1, SEED2, SEED3 };
  //for nrand
  float test_error;              //error of test line

  m = fixed_m;
  switch (samplecount) {
    case 0:
      c = 0.0f;
      line_error = 0.0f;
      break;

    case 1:
                                 //horiz thru pt
      c = samples[0].y () - m * samples[0].x ();
      line_error = 0.0f;
      break;

    case 2:
      c = (samples[0].y () + samples[1].y ()
        - m * (samples[0].x () + samples[1].x ())) / 2;
      line_error = m * samples[0].x () + c - samples[0].y ();
      line_error *= line_error;
      break;

    default:
      index = (inT32) nrand48 (seeds) % samplecount;
                                 //compute line
      c = samples[index].y () - m * samples[index].x ();
      compute_errors(m, c);  //from given line
      index = choose_nth_item (samplecount / 2, errors, samplecount);
      line_error = errors[index];
      for (trials = 1; trials < lms_line_trials; trials++) {
        index = (inT32) nrand48 (seeds) % samplecount;
        test_c = samples[index].y () - m * samples[index].x ();
        //compute line
        compute_errors(m, test_c);
        index = choose_nth_item (samplecount / 2, errors, samplecount);
        test_error = errors[index];
        if (test_error < line_error) {
                                 //find least median
          line_error = test_error;
          c = test_c;
        }
      }
  }
  fitted = TRUE;
  out_c = c;
  a = 0;
}


/**********************************************************************
 * LMS::pick_line
 *
 * Fit a line to a random pair of sample points.
 **********************************************************************/

void LMS::pick_line(                //fit sample
                    float &line_m,  //output gradient
                    float &line_c) {
  inT16 trial_count;             //no of attempts
  static uinT16 seeds[3] = { SEED1, SEED2, SEED3 };
  //for nrand
  inT32 index1;                  //picked point
  inT32 index2;                  //picked point

  trial_count = 0;
  do {
    index1 = (inT32) nrand48 (seeds) % samplecount;
    index2 = (inT32) nrand48 (seeds) % samplecount;
    line_m = samples[index2].x () - samples[index1].x ();
    trial_count++;
  }
  while (line_m == 0 && trial_count < LMS_MAX_FAILURES);
  if (line_m == 0) {
    line_c = (samples[index2].y () + samples[index1].y ()) / 2;
  }
  else {
    line_m = (samples[index2].y () - samples[index1].y ()) / line_m;
    line_c = samples[index1].y () - samples[index1].x () * line_m;
  }
}


/**********************************************************************
 * LMS::pick_quadratic
 *
 * Fit a quadratic to a random triplet of sample points.
 **********************************************************************/

void LMS::pick_quadratic(                 //fit sample
                         double &line_a,  //x suaread
                         float &line_m,   //output gradient
                         float &line_c) {
  inT16 trial_count;             //no of attempts
  static uinT16 seeds[3] = { SEED1, SEED2, SEED3 };
  //for nrand
  inT32 index1;                  //picked point
  inT32 index2;                  //picked point
  inT32 index3;
  FCOORD x1x2;                   //vector
  FCOORD x1x3;
  FCOORD x3x2;
  double bottom;                 //of a

  trial_count = 0;
  do {
    if (trial_count >= LMS_MAX_FAILURES - 1) {
      index1 = 0;
      index2 = samplecount / 2;
      index3 = samplecount - 1;
    }
    else {
      index1 = (inT32) nrand48 (seeds) % samplecount;
      index2 = (inT32) nrand48 (seeds) % samplecount;
      index3 = (inT32) nrand48 (seeds) % samplecount;
    }
    x1x2 = samples[index2] - samples[index1];
    x1x3 = samples[index3] - samples[index1];
    x3x2 = samples[index2] - samples[index3];
    bottom = x1x2.x () * x1x3.x () * x3x2.x ();
    trial_count++;
  }
  while (bottom == 0 && trial_count < LMS_MAX_FAILURES);
  if (bottom == 0) {
    line_a = 0;
    pick_line(line_m, line_c);
  }
  else {
    line_a = x1x3 * x1x2 / bottom;
    line_m = x1x2.y () - line_a * x1x2.x ()
      * (samples[index2].x () + samples[index1].x ());
    line_m /= x1x2.x ();
    line_c = samples[index1].y () - samples[index1].x ()
      * (samples[index1].x () * line_a + line_m);
  }
}


/**********************************************************************
 * LMS::compute_errors
 *
 * Compute the squared error from all the points.
 **********************************************************************/

void LMS::compute_errors(               //fit sample
                         float line_m,  //input gradient
                         float line_c) {
  inT32 index;                   //picked point

  for (index = 0; index < samplecount; index++) {
    errors[index] =
      line_m * samples[index].x () + line_c - samples[index].y ();
    errors[index] *= errors[index];
  }
}


/**********************************************************************
 * LMS::compute_quadratic_errors
 *
 * Compute the squared error from all the points.
 **********************************************************************/

float LMS::compute_quadratic_errors(                          //fit sample
                                    float outlier_threshold,  //min outlier
                                    double line_a,
                                    float line_m,             //input gradient
                                    float line_c) {
  inT32 outlier_count;           //total outliers
  inT32 index;                   //picked point
  inT32 error_count;             //no in total
  double total_error;            //summed squares

  total_error = 0;
  outlier_count = 0;
  error_count = 0;
  for (index = 0; index < samplecount; index++) {
    errors[error_count] = line_c + samples[index].x ()
      * (line_m + samples[index].x () * line_a) - samples[index].y ();
    errors[error_count] *= errors[error_count];
    if (errors[error_count] > outlier_threshold) {
      outlier_count++;
      errors[samplecount - outlier_count] = errors[error_count];
    }
    else {
      total_error += errors[error_count++];
    }
  }
  if (outlier_count * 3 < error_count)
    return total_error / error_count;
  else {
    index = choose_nth_item (outlier_count / 2,
      errors + samplecount - outlier_count,
      outlier_count);
    //median outlier
    return errors[samplecount - outlier_count + index];
  }
}


/**********************************************************************
 * LMS::plot
 *
 * Plot the fitted line of a LMS.
 **********************************************************************/

#ifndef GRAPHICS_DISABLED
void LMS::plot(               //plot fit
               ScrollView* win,    //window
               ScrollView::Color colour  //colour to draw in
              ) {
  if (fitted) {
    win->Pen(colour);
    win->SetCursor(samples[0].x (),
      c + samples[0].x () * (m + samples[0].x () * a));
    win->DrawTo(samples[samplecount - 1].x (),
      c + samples[samplecount - 1].x () * (m +
      samples[samplecount -
      1].x () * a));
  }
}
#endif
