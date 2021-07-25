/**********************************************************************
 * File:        pitsync1.h  (Formerly pitsync.h)
 * Description: Code to find the optimum fixed pitch segmentation of some blobs.
 * Author:    Ray Smith
 * Created:   Thu Nov 19 11:48:05 GMT 1992
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

#ifndef PITSYNC1_H
#define PITSYNC1_H

#include "blobbox.h"
#include "clst.h"
#include "elst.h"
#include "params.h"
#include "pithsync.h"
#include "statistc.h"

namespace tesseract {

class FPSEGPT_LIST;

class FPSEGPT : public ELIST_LINK {
public:
  FPSEGPT() = default;
  FPSEGPT(                      // constructor
      int16_t x);               // position
  FPSEGPT(                      // constructor
      int16_t x,                // position
      bool faking,              // faking this one
      int16_t offset,           // extra cost dist
      int16_t region_index,     // segment number
      int16_t pitch,            // proposed pitch
      int16_t pitch_error,      // allowed tolerance
      FPSEGPT_LIST *prev_list); // previous segment
  FPSEGPT(FPCUTPT *cutpt);      // build from new type

  int32_t position() { // access func
    return xpos;
  }
  double cost_function() {
    return cost;
  }
  double squares() {
    return sq_sum;
  }
  double sum() {
    return mean_sum;
  }
  FPSEGPT *previous() {
    return pred;
  }
  int16_t cheap_cuts() const { // no of cheap cuts
    return mid_cuts;
  }

  bool faked;         // faked split point
  bool terminal;      // successful end
  int16_t fake_count; // total fakes to here

private:
  int16_t mid_cuts; // no of cheap cuts
  int32_t xpos;     // location
  FPSEGPT *pred;    // optimal previous
  double mean_sum;  // mean so far
  double sq_sum;    // summed distsances
  double cost;      // cost function
};

ELISTIZEH(FPSEGPT)
CLISTIZEH(FPSEGPT_LIST)
extern INT_VAR_H(pitsync_linear_version);
extern double_VAR_H(pitsync_joined_edge);
extern double_VAR_H(pitsync_offset_freecut_fraction);
double check_pitch_sync(   // find segmentation
    BLOBNBOX_IT *blob_it,  // blobs to do
    int16_t blob_count,    // no of blobs
    int16_t pitch,         // pitch estimate
    int16_t pitch_error,   // tolerance
    STATS *projection,     // vertical
    FPSEGPT_LIST *seg_list // output list
);
void make_illegal_segment(   // find segmentation
    FPSEGPT_LIST *prev_list, // previous segments
    TBOX blob_box,           // bounding box
    BLOBNBOX_IT blob_it,     // iterator
    int16_t region_index,    // number of segment
    int16_t pitch,           // pitch estimate
    int16_t pitch_error,     // tolerance
    FPSEGPT_LIST *seg_list   // output list
);
int16_t vertical_torow_projection( // project whole row
    TO_ROW *row,                   // row to do
    STATS *projection              // output
);
void vertical_cblob_projection( // project outlines
    C_BLOB *blob,               // blob to project
    STATS *stats                // output
);
void vertical_coutline_projection( // project outlines
    C_OUTLINE *outline,            // outline to project
    STATS *stats                   // output
);

} // namespace tesseract

#endif
