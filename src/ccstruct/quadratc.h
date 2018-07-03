/**********************************************************************
 * File:        quadratc.h  (Formerly quadrtic.h)
 * Description: Code for the QUAD_COEFFS class.
 * Author:      Ray Smith
 * Created:     Tue Oct 08 17:24:40 BST 1991
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

#ifndef QUADRATC_H
#define QUADRATC_H

#include "points.h"

class QUAD_COEFFS
{
  public:
    QUAD_COEFFS() = default;
    QUAD_COEFFS(             //constructor
                double xsq,  //coefficients
                float x,
                float constant) {
      a = xsq;
      b = x;
      c = constant;
    }

    float y(                  //evaluate
            float x) const {  //at x
      return (float) ((a * x + b) * x + c);
    }

    void move(               // reposition word
              ICOORD vec) {  // by vector
      /************************************************************
        y - q = a (x - p)^2 + b (x - p) + c
        y - q = ax^2 - 2apx + ap^2 + bx - bp + c
          y = ax^2 + (b - 2ap)x + (c - bp + ap^2 + q)
      ************************************************************/
      int16_t p = vec.x ();
      int16_t q = vec.y ();

      c = (float) (c - b * p + a * p * p + q);
      b = (float) (b - 2 * a * p);
    }

    double a;                    //x squared
    float b;                     //x
    float c;                     //constant
  private:
};
#endif
