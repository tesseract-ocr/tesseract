/**********************************************************************
 * File:        normalis.h  (Formerly denorm.h)
 * Description: Code for the DENORM class.
 * Author:		Ray Smith
 * Created:		Thu Apr 23 09:22:43 BST 1992
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

#ifndef           NORMALIS_H
#define           NORMALIS_H

#include          <stdio.h>

class ROW;                       //forward decl

class DENORM_SEG
{
  public:
    DENORM_SEG() {
    }                            //empty

    inT32 xstart;                //start of segment
    inT32 ycoord;                //y at segment
    float scale_factor;          //for this segment
};

class DENORM
{
  public:
    DENORM() {  //constructor
      source_row = NULL;
      x_centre = 0.0f;
      scale_factor = 1.0f;
      segments = 0;
      segs = NULL;
      base_is_row = TRUE;
      m = c = 0;
    }
    DENORM(          //constructor
           float x,  //from same pieces
           float scaling,
           ROW *src) {
      x_centre = x;              //just copy
      scale_factor = scaling;
      source_row = src;
      segments = 0;
      segs = NULL;
      base_is_row = TRUE;
      m = c = 0;
    }
    DENORM(                      //constructor
           float x,              //from same pieces
           float scaling,
           double line_m,        //default line //no of segments
           double line_c,
           inT16 seg_count,
           DENORM_SEG *seg_pts,  //actual segments
           BOOL8 using_row,      //as baseline
           ROW *src);
    DENORM(const DENORM &);
    DENORM & operator= (const DENORM &);
    ~DENORM () {
      if (segments > 0)
        delete[]segs;
    }

    float origin() const {  //get x centre
      return x_centre;
    }
    float scale() const {  //get scale
      return scale_factor;
    }
    ROW *row() const {  //get row
      return source_row;
    }
    float x(  //convert an xcoord
            float src_x) const;
    float y(                    //convert a ycoord
            float src_y,        //coord to convert
            float src_centre) const;  //normed x centre
    float scale_at_x(  // Return scaling at this coord.
            float src_x) const;
    float yshift_at_x(  // Return yshift at this coord.
            float src_x) const;

  private:
    const DENORM_SEG *binary_search_segment(float src_x) const;

    BOOL8 base_is_row;           //using row baseline?
    inT16 segments;              //no of segments
    double c, m;                 //baseline
    float x_centre;              //middle of word
    float scale_factor;          //scaling
    ROW *source_row;             //row it came from
    DENORM_SEG *segs;            //array of segments
};
#endif
