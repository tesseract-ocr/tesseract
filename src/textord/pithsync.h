/**********************************************************************
 * File:        pithsync.h  (Formerly pitsync2.h)
 * Description: Code to find the optimum fixed pitch segmentation of some blobs.
 * Author:    Ray Smith
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

#ifndef PITHSYNC_H
#define PITHSYNC_H

#include "blobbox.h"
#include "params.h"
#include "statistc.h"

namespace tesseract {

class FPSEGPT_LIST;

class FPCUTPT {
public:
  FPCUTPT() = default;
  void setup(               // start of cut
      FPCUTPT cutpts[],     // predecessors
      int16_t array_origin, // start coord
      STATS *projection,    // occupation
      int16_t zero_count,   // official zero
      int16_t pitch,        // proposed pitch
      int16_t x,            // position
      int16_t offset);      // dist to gap

  void assign(                // evaluate cut
      FPCUTPT cutpts[],       // predecessors
      int16_t array_origin,   // start coord
      int16_t x,              // position
      bool faking,            // faking this one
      bool mid_cut,           // doing free cut
      int16_t offset,         // extra cost dist
      STATS *projection,      // occupation
      float projection_scale, // scaling
      int16_t zero_count,     // official zero
      int16_t pitch,          // proposed pitch
      int16_t pitch_error);   // allowed tolerance

  void assign_cheap(          // evaluate cut
      FPCUTPT cutpts[],       // predecessors
      int16_t array_origin,   // start coord
      int16_t x,              // position
      bool faking,            // faking this one
      bool mid_cut,           // doing free cut
      int16_t offset,         // extra cost dist
      STATS *projection,      // occupation
      float projection_scale, // scaling
      int16_t zero_count,     // official zero
      int16_t pitch,          // proposed pitch
      int16_t pitch_error);   // allowed tolerance

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
  FPCUTPT *previous() {
    return pred;
  }
  int16_t cheap_cuts() const { // no of mi cuts
    return mid_cuts;
  }
  int16_t index() const {
    return region_index;
  }

  bool faked;         // faked split point
  bool terminal;      // successful end
  int16_t fake_count; // total fakes to here

private:
  int16_t region_index;  // cut serial number
  int16_t mid_cuts;      // no of cheap cuts
  int32_t xpos;          // location
  uint32_t back_balance; // proj backwards
  uint32_t fwd_balance;  // proj forwards
  FPCUTPT *pred;         // optimal previous
  double mean_sum;       // mean so far
  double sq_sum;         // summed distsances
  double cost;           // cost function
};
double check_pitch_sync2(    // find segmentation
    BLOBNBOX_IT *blob_it,    // blobs to do
    int16_t blob_count,      // no of blobs
    int16_t pitch,           // pitch estimate
    int16_t pitch_error,     // tolerance
    STATS *projection,       // vertical
    int16_t projection_left, // edges //scale factor
    int16_t projection_right, float projection_scale,
    int16_t &occupation_count, // no of occupied cells
    FPSEGPT_LIST *seg_list,    // output list
    int16_t start,             // start of good range
    int16_t end                // end of good range
);
double check_pitch_sync3(    // find segmentation
    int16_t projection_left, // edges //to be considered 0
    int16_t projection_right, int16_t zero_count,
    int16_t pitch,             // pitch estimate
    int16_t pitch_error,       // tolerance
    STATS *projection,         // vertical
    float projection_scale,    // scale factor
    int16_t &occupation_count, // no of occupied cells
    FPSEGPT_LIST *seg_list,    // output list
    int16_t start,             // start of good range
    int16_t end                // end of good range
);

} // namespace tesseract

#endif
