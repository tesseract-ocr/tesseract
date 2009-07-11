/**********************************************************************
 * File:        lmedsq.h  (Formerly lms.h)
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

#ifndef           LMEDSQ_H
#define           LMEDSQ_H

#include          "points.h"
#include          "varable.h"
#include          "scrollview.h"
#include          "notdll.h"

class LMS
{
  public:
    LMS(              //constructor
        inT32 size);  //no of samples
    ~LMS ();                     //destructor
    void clear();  //clear samples
    void add(                 //add sample
             FCOORD sample);  //sample coords
    void fit(           //generate fit
             float &m,  //output line
             float &c);
    void constrained_fit(                //fixed gradient
                         float fixed_m,  //forced gradient
                         float &out_c);  //output line
    void fit_quadratic(                          //easy quadratic
                       float outlier_threshold,  //min outlier
                       double &a,                //x squared
                       float &b,                 //x
                       float &c);                //constant
    void plot(                 //plot fit
              ScrollView* win,      //window
              ScrollView::Color colour);  //colour to draw in
    float error() {  //get error
      return fitted ? line_error : -1;
    }

  private:

    void pick_line(           //random choice
                   float &m,  //output line
                   float &c);
    void pick_quadratic(            //random choice
                        double &a,  //output curve
                        float &b,
                        float &c);
    void compute_errors(          //find errors
                        float m,  //from line
                        float c);
                                 //find errors
    float compute_quadratic_errors(float outlier_threshold,  //min outlier
                                   double a,                 //from curve
                                   float m,
                                   float c);

    BOOL8 fitted;                //line parts valid
    inT32 samplesize;            //max samples
    inT32 samplecount;           //current sample size
    FCOORD *samples;             //array of samples
    float *errors;               //error distances
    double a;                    //x squared
    float m;                     //line gradient
    float c;
    float line_error;            //error of fit
};
extern INT_VAR_H (lms_line_trials, 12, "Number of linew fits to do");
#endif
