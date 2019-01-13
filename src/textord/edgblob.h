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

#ifndef           EDGBLOB_H
#define           EDGBLOB_H

#include          "scrollview.h"
#include          "params.h"
#include          "ocrblock.h"
#include          "coutln.h"
#include          "crakedge.h"

#include <memory>

#define BUCKETSIZE      16

class OL_BUCKETS
{
  public:
    OL_BUCKETS(               //constructor
               ICOORD bleft,  //corners
               ICOORD tright);

    ~OL_BUCKETS () = default;

    C_OUTLINE_LIST *operator () (//array access
      int16_t x,                   //image coords
      int16_t y);
                                 //first non-empty bucket
    C_OUTLINE_LIST *start_scan() {
      for (index = 0; buckets[index].empty () && index < bxdim * bydim - 1;
        index++);
      return &buckets[index];
    }
                                 //next non-empty bucket
    C_OUTLINE_LIST *scan_next() {
      for (; buckets[index].empty () && index < bxdim * bydim - 1; index++);
      return &buckets[index];
    }
    int32_t count_children(                     //recursive sum
                         C_OUTLINE *outline,  //parent outline
                         int32_t max_count);    // max output
    int32_t outline_complexity(                 // new version of count_children
                         C_OUTLINE *outline,  // parent outline
                         int32_t max_count,     // max output
                         int16_t depth);        // level of recursion
    void extract_children(                     //single level get
                          C_OUTLINE *outline,  //parent outline
                          C_OUTLINE_IT *it);   //destination iterator

  private:
    std::unique_ptr<C_OUTLINE_LIST[]> buckets;    //array of buckets
    int16_t bxdim;                 //size of array
    int16_t bydim;
    ICOORD bl;                   //corners
    ICOORD tr;
    int32_t index;                 //for extraction scan
};

void extract_edges(Pix* pix,        // thresholded image
                   BLOCK* block);   // block to scan
void outlines_to_blobs(               //find blobs
                       BLOCK *block,  //block to scan
                       ICOORD bleft,  //block box //outlines in block
                       ICOORD tright,
                       C_OUTLINE_LIST *outlines);
void fill_buckets(                           //find blobs
                  C_OUTLINE_LIST *outlines,  //outlines in block
                  OL_BUCKETS *buckets        //output buckets
                 );
void empty_buckets(                     //find blobs
                   BLOCK *block,        //block to scan
                   OL_BUCKETS *buckets  //output buckets
                  );
bool capture_children(                       //find children
        OL_BUCKETS* buckets,   //bucket sort clanss
        C_BLOB_IT* reject_it,  //dead grandchildren
        C_OUTLINE_IT* blob_it  //output outlines
);
#endif
