/**********************************************************************
 * File:        ocrrow.h  (Formerly row.h)
 * Description: Code for the ROW class.
 * Author:		Ray Smith
 * Created:		Tue Oct 08 15:58:04 BST 1991
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

#ifndef           OCRROW_H
#define           OCRROW_H

#include          <stdio.h>
#include          "quspline.h"
#include          "werd.h"

class TO_ROW;

class ROW:public ELIST_LINK
{
  friend void tweak_row_baseline(ROW *, double, double);
  public:
    ROW() {
    }                            //empty constructor
    ROW(                    //constructor
        inT32 spline_size,  //no of segments
        inT32 *xstarts,     //segment boundaries
        double *coeffs,     //coefficients //ascender size
        float x_height,
        float ascenders,
        float descenders,   //descender size
        inT16 kern,         //char gap
        inT16 space);       //word gap
    ROW(               //constructor
        TO_ROW *row,   //textord row
        inT16 kern,    //char gap
        inT16 space);  //word gap

    WERD_LIST *word_list() {  //get words
      return &words;
    }

    float base_line(                     //compute baseline
                    float xpos) const {  //at the position
                                 //get spline value
      return (float) baseline.y (xpos);
    }
    float x_height() const {  //return x height
      return xheight;
    }
    void set_x_height(float new_xheight) {  // set x height
      xheight = new_xheight;
    }
    inT32 kern() const {  //return kerning
      return kerning;
    }
    inT32 space() const {  //return spacing
      return spacing;
    }
    float ascenders() const {  //return size
      return ascrise;
    }
    float descenders() const {  //return size
      return descdrop;
    }
    TBOX bounding_box() const {  //return bounding box
      return bound_box;
    }

    void recalc_bounding_box();  //recalculate BB

    void move(                    // reposition row
              const ICOORD vec);  // by vector

    void print(            //print
               FILE *fp);  //file to print on

    void plot(                 //draw one
              ScrollView* window,   //window to draw in
              ScrollView::Color colour);  //uniform colour
    void plot(                 //draw one
              ScrollView* window);  //in rainbow colours

#ifndef GRAPHICS_DISABLED
    void plot_baseline(                  //draw the baseline
                       ScrollView* window,    //window to draw in
                       ScrollView::Color colour) {  //colour to draw
                                 //draw it
      baseline.plot (window, colour);
    }
#endif
    ROW& operator= (const ROW & source);

  private:
    inT32 kerning;               //inter char gap
    inT32 spacing;               //inter word gap
    TBOX bound_box;               //bounding box
    float xheight;               //height of line
    float ascrise;               //size of ascenders
    float descdrop;              //-size of descenders
    WERD_LIST words;             //words
    QSPLINE baseline;            //baseline spline
};

ELISTIZEH (ROW)
#endif
