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

#include          "quadratc.h"
#include          "serialis.h"
#include          "memry.h"
#include          "rect.h"

class ROW;
class Pix;

class QSPLINE
{
  friend void make_first_baseline(TBOX *,
                                  int,
                                  int *,
                                  int *,
                                  QSPLINE *,
                                  QSPLINE *,
                                  float);
  friend void make_holed_baseline(TBOX *, int, QSPLINE *, QSPLINE *, float);
  friend void tweak_row_baseline(ROW *, double, double);
  public:
    QSPLINE() {  //empty constructor
      segments = 0;
      xcoords = NULL;            //everything empty
      quadratics = NULL;
    }
    QSPLINE(  //copy constructor
            const QSPLINE &src);
    QSPLINE(                  //constructor
            inT32 count,      //number of segments
            inT32 *xstarts,   //segment starts
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
              ScrollView* window,         //in window
              ScrollView::Color colour) const;  //in colour
#endif

    // Paint the baseline over pix. If pix has depth of 32, then the line will
    // be painted in red. Otherwise it will be painted in black.
    void plot(Pix* pix) const;

    QSPLINE & operator= (
      const QSPLINE & source);   //from this

  private:

    inT32 spline_index(                  //binary search
                       double x) const;  //for x
    inT32 segments;              //no of segments
    inT32 *xcoords;              //no of coords
    QUAD_COEFFS *quadratics;     //spline pieces
};
#endif
