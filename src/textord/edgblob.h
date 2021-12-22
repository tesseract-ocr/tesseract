/**********************************************************************
 * File:        edgblob.h  (Formerly edgeloop.h)
 * Description: Functions to clean up an outline before approximation.
 * Author:      Ray Smith
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

#ifndef EDGBLOB_H
#define EDGBLOB_H

#include "coutln.h"   // for C_OUTLINE
#include "ocrblock.h" // for BLOCK
#include "points.h"   // for ICOORD

#include <vector>

namespace tesseract {

class OL_BUCKETS {
public:
  OL_BUCKETS(       // constructor
      ICOORD bleft, // corners
      ICOORD tright);

  C_OUTLINE_LIST *operator()( // array access
      TDimension x,           // image coords
      TDimension y);
  // first non-empty bucket
  C_OUTLINE_LIST *start_scan();
  // next non-empty bucket
  C_OUTLINE_LIST *scan_next();
  int32_t count_children(     // recursive sum
      C_OUTLINE *outline,     // parent outline
      int32_t max_count);     // max output
  int32_t outline_complexity( // new version of count_children
      C_OUTLINE *outline,     // parent outline
      int32_t max_count,      // max output
      int16_t depth);         // level of recursion
  void extract_children(      // single level get
      C_OUTLINE *outline,     // parent outline
      C_OUTLINE_IT *it);      // destination iterator

private:
  int16_t bxdim; // size of array
  int16_t bydim;
  std::vector<C_OUTLINE_LIST> buckets; // array of buckets
  ICOORD bl;                           // corners
  ICOORD tr;
  decltype(buckets)::iterator it; // for extraction scan

  C_OUTLINE_LIST *scan_next(decltype(buckets)::iterator it);
};

void extract_edges(Image pix,     // thresholded image
                   BLOCK *block); // block to scan
void outlines_to_blobs(           // find blobs
    BLOCK *block,                 // block to scan
    ICOORD bleft,                 // block box //outlines in block
    ICOORD tright, C_OUTLINE_LIST *outlines);

} // namespace tesseract

#endif
