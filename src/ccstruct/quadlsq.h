/**********************************************************************
 * File:        quadlsq.h  (Formerly qlsq.h)
 * Description: Code for least squares approximation of quadratics.
 * Author:      Ray Smith
 * Created:     Wed Oct  6 15:14:23 BST 1993
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

#ifndef QUADLSQ_H
#define QUADLSQ_H

#include "points.h"

class QLSQ
{
  public:
    QLSQ() {  //constructor
      clear();  //set to zeros
    }
    void clear();  //initialize

    void add(           //add element
             double x,  //coords to add
             double y);
    void remove(           //delete element
                double x,  //coords to delete
                double y);
    int32_t count() {  //no of elements
      return n;
    }

    void fit(              //fit the given
             int degree);  //return actual
    double get_a() {  //get x squard
      return a;
    }
    double get_b() {  //get x squard
      return b;
    }
    double get_c() {  //get x squard
      return c;
    }

  private:
    int32_t n;                     //no of elements
    double a, b, c;              //result
    double sigx;                 //sum of x
    double sigy;                 //sum of y
    double sigxx;                //sum x squared
    double sigxy;                //sum of xy
    double sigyy;                //sum y squared
    long double sigxxx;          //sum x cubed
    long double sigxxy;          //sum xsquared y
    long double sigxxxx;         //sum x fourth
};
#endif
