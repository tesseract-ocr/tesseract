/**********************************************************************
 * File:        normalis.h  (Formerly denorm.h)
 * Description: Code for the DENORM class.
 * Author:      Ray Smith
 * Created:     Thu Apr 23 09:22:43 BST 1992
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
class BLOCK;

class DENORM_SEG
{
  public:
    DENORM_SEG() {}

    inT32 xstart;         // start of segment
    inT32 ycoord;         // y at segment
    float scale_factor;   // normalized_x/scale_factor + x_center == original_x
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
      block_ = NULL;
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
      block_ = NULL;
    }
    DENORM(                      // constructor
           float x,              // from same pieces
           float scaling,
           double line_m,        // default line: y = mx + c
           double line_c,
           inT16 seg_count,      // no of segments
           DENORM_SEG *seg_pts,  // actual segments
           BOOL8 using_row,      // as baseline
           ROW *src);
    DENORM(const DENORM &);
    DENORM & operator= (const DENORM &);
    ~DENORM() {
      if (segments > 0)
        delete[]segs;
    }

    // Return the original x coordinate of the middle of the word
    // (mapped to 0 in normalized coordinates).
    float origin() const { return x_centre; }

    float scale() const {  //get scale
      return scale_factor;
    }
    ROW *row() const {  //get row
      return source_row;
    }
    const BLOCK* block() const {
      return block_;
    }
    void set_block(const BLOCK* block) {
      block_ = block;
    }

    // normalized x -> original x
    float x(float src_x) const;

    // Given a (y coordinate, x center of segment) in normalized coordinates,
    // return the original y coordinate.
    float y(float src_y, float src_x_centre) const;

    float scale_at_x(  // Return scaling at this coord.
            float src_x) const;
    float yshift_at_x(  // Return yshift at this coord.
            float src_x) const;

  private:
    const DENORM_SEG *binary_search_segment(float src_x) const;

    BOOL8 base_is_row;    // using row baseline?
    inT16 segments;       // no of segments
    double c, m;          // baseline: y = mx + c
    float x_centre;       // middle of word in original coordinates
    float scale_factor;   // normalized_x/scale_factor + x_center == original_x
    ROW *source_row;      // row it came from
    DENORM_SEG *segs;     // array of segments
    const BLOCK* block_;  // Block the word came from.
};
#endif
