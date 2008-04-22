/**********************************************************************
 * File:        normalis.cpp  (Formerly denorm.c)
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

#include "mfcpch.h"
#include          "werd.h"
#include          "normalis.h"

/**********************************************************************
 * DENORM::binary_search_segment
 *
 * Find the segment to use for the given x.
 **********************************************************************/

const DENORM_SEG *DENORM::binary_search_segment(float src_x) const {
  int bottom, top, middle;       //binary search

  bottom = 0;
  top = segments;
  do {
    middle = (bottom + top) / 2;
    if (segs[middle].xstart > src_x)
      top = middle;
    else
      bottom = middle;
  }
  while (top - bottom > 1);
  return &segs[bottom];
}

/**********************************************************************
 * DENORM::scale_at_x
 *
 * Return scaling at a given (normalized) x coord.
 **********************************************************************/

float DENORM::scale_at_x(float src_x) const {  // In normalized coords.
  if (segments != 0) {
    const DENORM_SEG* seg = binary_search_segment(src_x);
    if (seg->scale_factor > 0.0)
      return seg->scale_factor;
  }
  return scale_factor;
}

/**********************************************************************
 * DENORM::yshift_at_x
 *
 * Return yshift at a given (normalized) x coord.
 **********************************************************************/

float DENORM::yshift_at_x(float src_x) const {  // In normalized coords.
  if (segments != 0) {
    const DENORM_SEG* seg = binary_search_segment(src_x);
    if (seg->ycoord == -MAX_INT32) {
      if (base_is_row)
        return source_row->base_line(x(src_x)/scale_at_x(src_x) + x_centre);
      else
        return m * x(src_x) + c;
    } else {
      return seg->ycoord;
    }
  }
  return source_row->base_line (x(src_x)/scale_at_x(src_x) + x_centre);
}

/**********************************************************************
 * DENORM::x
 *
 * Denormalise an x coordinate.
 **********************************************************************/

float DENORM::x(             //convert x coord
                float src_x  //coord to convert
               ) const {
  return src_x / scale_at_x(src_x) + x_centre;
}


/**********************************************************************
 * DENORM::y
 *
 * Denormalise a y coordinate.
 **********************************************************************/

float DENORM::y(                  //convert y coord
                float src_y,      //coord to convert
                float src_centre  //x location for base
               ) const {
  return (src_y - bln_baseline_offset) / scale_at_x(src_centre)
    + yshift_at_x(src_centre);
}


DENORM::DENORM(float x,              //from same pieces
               float scaling,
               double line_m,        //default line
               double line_c,
               inT16 seg_count,      //no of segments
               DENORM_SEG *seg_pts,  //actual segments
               BOOL8 using_row,      //as baseline
               ROW *src) {
  x_centre = x;                  //just copy
  scale_factor = scaling;
  source_row = src;
  if (seg_count > 0) {
    segs = new DENORM_SEG[seg_count];
    for (segments = 0; segments < seg_count; segments++) {
      // It is possible, if infrequent that the segments may be out of order.
      // since we are searching with a binary search, keep them in order.
      if (segments == 0 || segs[segments - 1].xstart <=
          seg_pts[segments].xstart) {
        segs[segments] = seg_pts[segments];
      } else {
        int i;
        for (i = 0; i < segments
             && segs[segments - 1 - i].xstart > seg_pts[segments].xstart;
             ++i) {
          segs[segments - i ] = segs[segments - 1 - i];
        }
        segs[segments - i] = seg_pts[segments];
      }
    }
  }
  else {
    segments = 0;
    segs = NULL;
  }
  base_is_row = using_row;
  m = line_m;
  c = line_c;
}


DENORM::DENORM(const DENORM &src) {
  segments = 0;
  segs = NULL;
  *this = src;
}


DENORM & DENORM::operator= (const DENORM & src) {
  x_centre = src.x_centre;
  scale_factor = src.scale_factor;
  source_row = src.source_row;
  if (segments > 0)
    delete[]segs;
  if (src.segments > 0) {
    segs = new DENORM_SEG[src.segments];
    for (segments = 0; segments < src.segments; segments++)
      segs[segments] = src.segs[segments];
  }
  else {
    segments = 0;
    segs = NULL;
  }
  base_is_row = src.base_is_row;
  m = src.m;
  c = src.c;
  return *this;
}
