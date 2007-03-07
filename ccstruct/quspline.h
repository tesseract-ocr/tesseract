/**********************************************************************
 * File:        quspline.h  (Formerly qspline.h)
 * Description: Code for the QSPLINE class.
 * Author:		Ray Smith
 * Created:		Tue Oct 08 17:16:12 BST 1991
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

#ifndef           QUSPLINE_H
#define           QUSPLINE_H

#include          "grphics.h"
#include          "quadratc.h"
#include          "serialis.h"
#include          "memry.h"
#include          "rect.h"

class ROW;

class QSPLINE
{
  friend void make_first_baseline(BOX *,
                                  int,
                                  int *,
                                  int *,
                                  QSPLINE *,
                                  QSPLINE *,
                                  float);
  friend void make_holed_baseline(BOX *, int, QSPLINE *, QSPLINE *, float); 
  friend void tweak_row_baseline(ROW *); 
  public:
    QSPLINE() {  //empty constructor
      segments = 0;
      xcoords = NULL;            //everything empty
      quadratics = NULL;
    }
    QSPLINE(  //copy constructor
            const QSPLINE &src);
    QSPLINE(                  //constructor
            INT32 count,      //number of segments
            INT32 *xstarts,   //segment starts
            double *coeffs);  //coefficients
    ~QSPLINE ();                 //destructor
    QSPLINE (                    //least squares fit
      int xstarts[],             //spline boundaries
      int segcount,              //no of segments
      int xcoords[],             //points to fit
      int ycoords[], int blobcount,//no of coords
      int degree);               //function

    double step(            //step change
                double x1,  //between coords
                double x2);
    double y(                  //evaluate
             double x) const;  //at x

    void move(              // reposition spline
              ICOORD vec);  // by vector
    BOOL8 overlap(                   //test overlap
                  QSPLINE *spline2,  //2 cannot be smaller
                  double fraction);  //by more than this
    void extrapolate(                  //linear extrapolation
                     double gradient,  //gradient to use
                     int left,         //new left edge
                     int right);       //new right edge

#ifndef GRAPHICS_DISABLED
    void plot(                       //draw it
              WINDOW window,         //in window
              COLOUR colour) const;  //in colour
#endif

    void prep_serialise() {  //set ptrs to counts
    }                            //not required

    void dump(  //write external bits
              FILE *f) {
      serialise_bytes (f, (void *) xcoords, (segments + 1) * sizeof (INT32));
      serialise_bytes (f, (void *) quadratics, segments * sizeof (QUAD_COEFFS));
    }

    void de_dump(  //read external bits
                 FILE *f) {
      xcoords = (INT32 *) de_serialise_bytes (f,
        (segments + 1) * sizeof (INT32));
      quadratics = (QUAD_COEFFS *) de_serialise_bytes (f,
        segments *
        sizeof (QUAD_COEFFS));
    }

                                 //assign copy
    make_serialise (QSPLINE) QSPLINE & operator= (
      const QSPLINE & source);   //from this

  private:

    INT32 spline_index(                  //binary search
                       double x) const;  //for x
    INT32 segments;              //no of segments
    INT32 *xcoords;              //no of coords
    QUAD_COEFFS *quadratics;     //spline pieces
};
#endif
