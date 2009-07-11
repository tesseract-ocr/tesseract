/**********************************************************************
 * File:        linlsq.h  (Formerly llsq.h)
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

#ifndef           LINLSQ_H
#define           LINLSQ_H

#include          "points.h"
#include          "mod128.h"
#include          "varable.h"

class LLSQ
{
  friend class PDLSQ;            //pos & direction

  public:
    LLSQ() {  //constructor
      clear();  //set to zeros
    }
    void clear();  //initialize

    void add(           //add element
             double x,  //coords to add
             double y);
    void remove(           //delete element
                double x,  //coords to delete
                double y);
    inT32 count() {  //no of elements
      return n;
    }

    double m();  //get gradient
    double c(            //get constant
             double m);  //gradient
    double rms(            //get error
               double m,   //gradient
               double c);  //constant
    double spearman();  //get error

  private:
    inT32 n;                     //no of elements
    double sigx;                 //sum of x
    double sigy;                 //sum of y
    double sigxx;                //sum x squared
    double sigxy;                //sum of xy
    double sigyy;                //sum y squared
};

class PDLSQ
{
  public:
    PDLSQ() {  //constructor
      clear();  //set to zeros
    }
    void clear() {  //initialize
      pos.clear ();              //clear both
      dir.clear ();
    }

    void add(                         //add element
             const ICOORD &addpos,    //position of pt
             const ICOORD &adddir) {  //dir of pt
      pos.add (addpos.x (), addpos.y ());
      dir.add (adddir.x (), adddir.y ());
    }
    void remove(                            //remove element
                const ICOORD &removepos,    //position of pt
                const ICOORD &removedir) {  //dir of pt
      pos.remove (removepos.x (), removepos.y ());
      dir.remove (removedir.x (), removedir.y ());
    }
    inT32 count() {  //no of elements
      return pos.count ();
    }

    float fit(                 //get fit parameters
              DIR128 &ang,     //output angle
              float &sin_ang,  //output components
              float &cos_ang,
              float &r);

  private:
    LLSQ pos;                    //position
    LLSQ dir;                    //directions
};
extern double_VAR_H (pdlsq_posdir_ratio, 0.4e-6, "Mult of dir to cf pos");
#endif
