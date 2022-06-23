/**********************************************************************
 * File:        pithsync.cpp  (Formerly pitsync2.c)
 * Description: Code to find the optimum fixed pitch segmentation of some blobs.
 * Author:      Ray Smith
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

#include "pithsync.h"

#include "makerow.h"
#include "pitsync1.h"
#include "topitch.h"
#include "tprintf.h"

#include <cfloat> // for FLT_MAX
#include <cmath>
#include <vector> // for std::vector

namespace tesseract {

/**********************************************************************
 * FPCUTPT::setup
 *
 * Constructor to make a new FPCUTPT.
 **********************************************************************/

void FPCUTPT::setup(      // constructor
    FPCUTPT *cutpts,      // predecessors
    int16_t array_origin, // start coord
    STATS *projection,    // vertical occupation
    int16_t zero_count,   // official zero
    int16_t pitch,        // proposed pitch
    int16_t x,            // position
    int16_t offset        // dist to gap
) {
  // half of pitch
  int16_t half_pitch = pitch / 2 - 1;
  uint32_t lead_flag; // new flag
  int32_t ind;        // current position

  if (half_pitch > 31) {
    half_pitch = 31;
  } else if (half_pitch < 0) {
    half_pitch = 0;
  }
  lead_flag = 1 << half_pitch;

  pred = nullptr;
  mean_sum = 0;
  sq_sum = offset * offset;
  cost = sq_sum;
  faked = false;
  terminal = false;
  fake_count = 0;
  xpos = x;
  region_index = 0;
  mid_cuts = 0;
  if (x == array_origin) {
    back_balance = 0;
    fwd_balance = 0;
    for (ind = 0; ind <= half_pitch; ind++) {
      fwd_balance >>= 1;
      if (projection->pile_count(ind) > zero_count) {
        fwd_balance |= lead_flag;
      }
    }
  } else {
    back_balance = cutpts[x - 1 - array_origin].back_balance << 1;
    back_balance &= lead_flag + (lead_flag - 1);
    if (projection->pile_count(x) > zero_count) {
      back_balance |= 1;
    }
    fwd_balance = cutpts[x - 1 - array_origin].fwd_balance >> 1;
    if (projection->pile_count(x + half_pitch) > zero_count) {
      fwd_balance |= lead_flag;
    }
  }
}

/**********************************************************************
 * FPCUTPT::assign
 *
 * Constructor to make a new FPCUTPT.
 **********************************************************************/

void FPCUTPT::assign(       // constructor
    FPCUTPT *cutpts,        // predecessors
    int16_t array_origin,   // start coord
    int16_t x,              // position
    bool faking,            // faking this one
    bool mid_cut,           // cheap cut.
    int16_t offset,         // dist to gap
    STATS *projection,      // vertical occupation
    float projection_scale, // scaling
    int16_t zero_count,     // official zero
    int16_t pitch,          // proposed pitch
    int16_t pitch_error     // allowed tolerance
) {
  int index;             // test index
  int balance_index;     // for balance factor
  int16_t balance_count; // ding factor
  int16_t r_index;       // test cut number
  FPCUTPT *segpt;        // segment point
  int32_t dist;          // from prev segment
  double sq_dist;        // squared distance
  double mean;           // mean pitch
  double total;          // total dists
  double factor;         // cost function
                         // half of pitch
  int16_t half_pitch = pitch / 2 - 1;
  uint32_t lead_flag; // new flag

  if (half_pitch > 31) {
    half_pitch = 31;
  } else if (half_pitch < 0) {
    half_pitch = 0;
  }
  lead_flag = 1 << half_pitch;

  back_balance = cutpts[x - 1 - array_origin].back_balance << 1;
  back_balance &= lead_flag + (lead_flag - 1);
  if (projection->pile_count(x) > zero_count) {
    back_balance |= 1;
  }
  fwd_balance = cutpts[x - 1 - array_origin].fwd_balance >> 1;
  if (projection->pile_count(x + half_pitch) > zero_count) {
    fwd_balance |= lead_flag;
  }

  xpos = x;
  cost = FLT_MAX;
  pred = nullptr;
  faked = faking;
  terminal = false;
  region_index = 0;
  fake_count = INT16_MAX;
  for (index = x - pitch - pitch_error; index <= x - pitch + pitch_error; index++) {
    if (index >= array_origin) {
      segpt = &cutpts[index - array_origin];
      dist = x - segpt->xpos;
      if (!segpt->terminal && segpt->fake_count < INT16_MAX) {
        balance_count = 0;
        if (textord_balance_factor > 0) {
          if (textord_fast_pitch_test) {
            lead_flag = back_balance ^ segpt->fwd_balance;
            balance_count = 0;
            while (lead_flag != 0) {
              balance_count++;
              lead_flag &= lead_flag - 1;
            }
          } else {
            for (balance_index = 0; index + balance_index < x - balance_index; balance_index++) {
              balance_count += (projection->pile_count(index + balance_index) <= zero_count) ^
                               (projection->pile_count(x - balance_index) <= zero_count);
            }
          }
          balance_count =
              static_cast<int16_t>(balance_count * textord_balance_factor / projection_scale);
        }
        r_index = segpt->region_index + 1;
        total = segpt->mean_sum + dist;
        balance_count += offset;
        sq_dist = dist * dist + segpt->sq_sum + balance_count * balance_count;
        mean = total / r_index;
        factor = mean - pitch;
        factor *= factor;
        factor += sq_dist / (r_index)-mean * mean;
        if (factor < cost && segpt->fake_count + faked <= fake_count) {
          cost = factor; // find least cost
          pred = segpt;  // save path
          mean_sum = total;
          sq_sum = sq_dist;
          fake_count = segpt->fake_count + faked;
          mid_cuts = segpt->mid_cuts + mid_cut;
          region_index = r_index;
        }
      }
    }
  }
}

/**********************************************************************
 * FPCUTPT::assign_cheap
 *
 * Constructor to make a new FPCUTPT on the cheap.
 **********************************************************************/

void FPCUTPT::assign_cheap( // constructor
    FPCUTPT *cutpts,        // predecessors
    int16_t array_origin,   // start coord
    int16_t x,              // position
    bool faking,            // faking this one
    bool mid_cut,           // cheap cut.
    int16_t offset,         // dist to gap
    STATS *projection,      // vertical occupation
    float projection_scale, // scaling
    int16_t zero_count,     // official zero
    int16_t pitch,          // proposed pitch
    int16_t pitch_error     // allowed tolerance
) {
  int index;             // test index
  int16_t balance_count; // ding factor
  int16_t r_index;       // test cut number
  FPCUTPT *segpt;        // segment point
  int32_t dist;          // from prev segment
  double sq_dist;        // squared distance
  double mean;           // mean pitch
  double total;          // total dists
  double factor;         // cost function
                         // half of pitch
  int16_t half_pitch = pitch / 2 - 1;
  uint32_t lead_flag; // new flag

  if (half_pitch > 31) {
    half_pitch = 31;
  } else if (half_pitch < 0) {
    half_pitch = 0;
  }
  lead_flag = 1 << half_pitch;

  back_balance = cutpts[x - 1 - array_origin].back_balance << 1;
  back_balance &= lead_flag + (lead_flag - 1);
  if (projection->pile_count(x) > zero_count) {
    back_balance |= 1;
  }
  fwd_balance = cutpts[x - 1 - array_origin].fwd_balance >> 1;
  if (projection->pile_count(x + half_pitch) > zero_count) {
    fwd_balance |= lead_flag;
  }

  xpos = x;
  cost = FLT_MAX;
  pred = nullptr;
  faked = faking;
  terminal = false;
  region_index = 0;
  fake_count = INT16_MAX;
  index = x - pitch;
  if (index >= array_origin) {
    segpt = &cutpts[index - array_origin];
    dist = x - segpt->xpos;
    if (!segpt->terminal && segpt->fake_count < INT16_MAX) {
      balance_count = 0;
      if (textord_balance_factor > 0) {
        lead_flag = back_balance ^ segpt->fwd_balance;
        balance_count = 0;
        while (lead_flag != 0) {
          balance_count++;
          lead_flag &= lead_flag - 1;
        }
        balance_count =
            static_cast<int16_t>(balance_count * textord_balance_factor / projection_scale);
      }
      r_index = segpt->region_index + 1;
      total = segpt->mean_sum + dist;
      balance_count += offset;
      sq_dist = dist * dist + segpt->sq_sum + balance_count * balance_count;
      mean = total / r_index;
      factor = mean - pitch;
      factor *= factor;
      factor += sq_dist / (r_index)-mean * mean;
      cost = factor; // find least cost
      pred = segpt;  // save path
      mean_sum = total;
      sq_sum = sq_dist;
      fake_count = segpt->fake_count + faked;
      mid_cuts = segpt->mid_cuts + mid_cut;
      region_index = r_index;
    }
  }
}

/**********************************************************************
 * check_pitch_sync
 *
 * Construct the lattice of possible segmentation points and choose the
 * optimal path. Return the optimal path only.
 * The return value is a measure of goodness of the sync.
 **********************************************************************/

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
) {
  bool faking;                  // illegal cut pt
  bool mid_cut;                 // cheap cut pt.
  int16_t x;                    // current coord
  int16_t blob_index;           // blob number
  int16_t left_edge;            // of word
  int16_t right_edge;           // of word
  int16_t array_origin;         // x coord of array
  int16_t offset;               // dist to legal area
  int16_t zero_count;           // projection zero
  int16_t best_left_x = 0;      // for equals
  int16_t best_right_x = 0;     // right edge
  TBOX this_box;                // bounding box
  TBOX next_box;                // box of next blob
  FPSEGPT *segpt;               // segment point
  double best_cost;             // best path
  double mean_sum;              // computes result
  FPCUTPT *best_end;            // end of best path
  int16_t best_fake;            // best fake level
  int16_t best_count;           // no of cuts
  BLOBNBOX_IT this_it;          // copy iterator
  FPSEGPT_IT seg_it = seg_list; // output iterator

  //      tprintf("Computing sync on word of %d blobs with pitch %d\n",
  //              blob_count, pitch);
  //      if (blob_count==8 && pitch==27)
  //              projection->print(stdout,true);
  zero_count = 0;
  if (pitch < 3) {
    pitch = 3; // nothing ludicrous
  }
  if ((pitch - 3) / 2 < pitch_error) {
    pitch_error = (pitch - 3) / 2;
  }
  this_it = *blob_it;
  this_box = box_next(&this_it); // get box
  //      left_edge=this_box.left(); //left of word right_edge=this_box.right();
  //      for (blob_index=1;blob_index<blob_count;blob_index++)
  //      {
  //              this_box=box_next(&this_it);
  //              if (this_box.right()>right_edge)
  //                      right_edge=this_box.right();
  //      }
  for (left_edge = projection_left;
       projection->pile_count(left_edge) == 0 && left_edge < projection_right; left_edge++) {
    ;
  }
  for (right_edge = projection_right;
       projection->pile_count(right_edge) == 0 && right_edge > left_edge; right_edge--) {
    ;
  }
  ASSERT_HOST(right_edge >= left_edge);
  if (pitsync_linear_version >= 4) {
    return check_pitch_sync3(projection_left, projection_right, zero_count, pitch, pitch_error,
                             projection, projection_scale, occupation_count, seg_list, start, end);
  }
  array_origin = left_edge - pitch;
  // array of points
  std::vector<FPCUTPT> cutpts(right_edge - left_edge + pitch * 2 + 1);
  for (x = array_origin; x < left_edge; x++) {
    // free cuts
    cutpts[x - array_origin].setup(&cutpts[0], array_origin, projection, zero_count, pitch, x, 0);
  }
  for (offset = 0; offset <= pitch_error; offset++, x++) {
    // not quite free
    cutpts[x - array_origin].setup(&cutpts[0], array_origin, projection, zero_count, pitch, x,
                                   offset);
  }

  this_it = *blob_it;
  best_cost = FLT_MAX;
  best_end = nullptr;
  this_box = box_next(&this_it); // first box
  next_box = box_next(&this_it); // second box
  blob_index = 1;
  while (x < right_edge - pitch_error) {
    if (x > this_box.right() + pitch_error && blob_index < blob_count) {
      this_box = next_box;
      next_box = box_next(&this_it);
      blob_index++;
    }
    faking = false;
    mid_cut = false;
    if (x <= this_box.left()) {
      offset = 0;
    } else if (x <= this_box.left() + pitch_error) {
      offset = x - this_box.left();
    } else if (x >= this_box.right()) {
      offset = 0;
    } else if (x >= next_box.left() && blob_index < blob_count) {
      offset = x - next_box.left();
      if (this_box.right() - x < offset) {
        offset = this_box.right() - x;
      }
    } else if (x >= this_box.right() - pitch_error) {
      offset = this_box.right() - x;
    } else if (x - this_box.left() > pitch * pitsync_joined_edge &&
               this_box.right() - x > pitch * pitsync_joined_edge) {
      mid_cut = true;
      offset = 0;
    } else {
      faking = true;
      offset = projection->pile_count(x);
    }
    cutpts[x - array_origin].assign(&cutpts[0], array_origin, x, faking, mid_cut, offset,
                                    projection, projection_scale, zero_count, pitch, pitch_error);
    x++;
  }

  best_fake = INT16_MAX;
  best_cost = INT32_MAX;
  best_count = INT16_MAX;
  while (x < right_edge + pitch) {
    offset = x < right_edge ? right_edge - x : 0;
    cutpts[x - array_origin].assign(&cutpts[0], array_origin, x, false, false, offset, projection,
                                    projection_scale, zero_count, pitch, pitch_error);
    cutpts[x - array_origin].terminal = true;
    if (cutpts[x - array_origin].index() + cutpts[x - array_origin].fake_count <=
        best_count + best_fake) {
      if (cutpts[x - array_origin].fake_count < best_fake ||
          (cutpts[x - array_origin].fake_count == best_fake &&
           cutpts[x - array_origin].cost_function() < best_cost)) {
        best_fake = cutpts[x - array_origin].fake_count;
        best_cost = cutpts[x - array_origin].cost_function();
        best_left_x = x;
        best_right_x = x;
        best_count = cutpts[x - array_origin].index();
      } else if (cutpts[x - array_origin].fake_count == best_fake && x == best_right_x + 1 &&
                 cutpts[x - array_origin].cost_function() == best_cost) {
        // exactly equal
        best_right_x = x;
      }
    }
    x++;
  }
  ASSERT_HOST(best_fake < INT16_MAX);

  best_end = &cutpts[(best_left_x + best_right_x) / 2 - array_origin];
  if (this_box.right() == textord_test_x && this_box.top() == textord_test_y) {
    for (x = left_edge - pitch; x < right_edge + pitch; x++) {
      tprintf("x=%d, C=%g, s=%g, sq=%g, prev=%d\n", x, cutpts[x - array_origin].cost_function(),
              cutpts[x - array_origin].sum(), cutpts[x - array_origin].squares(),
              cutpts[x - array_origin].previous()->position());
    }
  }
  occupation_count = -1;
  do {
    for (x = best_end->position() - pitch + pitch_error;
         x < best_end->position() - pitch_error && projection->pile_count(x) == 0; x++) {
      ;
    }
    if (x < best_end->position() - pitch_error) {
      occupation_count++;
    }
    // copy it
    segpt = new FPSEGPT(best_end);
    seg_it.add_before_then_move(segpt);
    best_end = best_end->previous();
  } while (best_end != nullptr);
  seg_it.move_to_last();
  mean_sum = seg_it.data()->sum();
  mean_sum = mean_sum * mean_sum / best_count;
  if (seg_it.data()->squares() - mean_sum < 0) {
    tprintf("Impossible sqsum=%g, mean=%g, total=%d\n", seg_it.data()->squares(),
            seg_it.data()->sum(), best_count);
  }
  //      tprintf("blob_count=%d, pitch=%d, sync=%g, occ=%d\n",
  //              blob_count,pitch,seg_it.data()->squares()-mean_sum,
  //              occupation_count);
  return seg_it.data()->squares() - mean_sum;
}

/**********************************************************************
 * check_pitch_sync
 *
 * Construct the lattice of possible segmentation points and choose the
 * optimal path. Return the optimal path only.
 * The return value is a measure of goodness of the sync.
 **********************************************************************/

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
) {
  bool faking;                  // illegal cut pt
  bool mid_cut;                 // cheap cut pt.
  int16_t left_edge;            // of word
  int16_t right_edge;           // of word
  int16_t x;                    // current coord
  int16_t array_origin;         // x coord of array
  int16_t offset;               // dist to legal area
  int16_t projection_offset;    // from scaled projection
  int16_t prev_zero;            // previous zero dist
  int16_t next_zero;            // next zero dist
  int16_t zero_offset;          // scan window
  int16_t best_left_x = 0;      // for equals
  int16_t best_right_x = 0;     // right edge
  FPSEGPT *segpt;               // segment point
  int minindex;                 // next input position
  int test_index;               // index to mins
  double best_cost;             // best path
  double mean_sum;              // computes result
  FPCUTPT *best_end;            // end of best path
  int16_t best_fake;            // best fake level
  int16_t best_count;           // no of cuts
  FPSEGPT_IT seg_it = seg_list; // output iterator

  end = (end - start) % pitch;
  if (pitch < 3) {
    pitch = 3; // nothing ludicrous
  }
  if ((pitch - 3) / 2 < pitch_error) {
    pitch_error = (pitch - 3) / 2;
  }
  // min dist of zero
  zero_offset = static_cast<int16_t>(pitch * pitsync_joined_edge);
  for (left_edge = projection_left;
       projection->pile_count(left_edge) == 0 && left_edge < projection_right; left_edge++) {
    ;
  }
  for (right_edge = projection_right;
       projection->pile_count(right_edge) == 0 && right_edge > left_edge; right_edge--) {
    ;
  }
  array_origin = left_edge - pitch;
  // array of points
  std::vector<FPCUTPT> cutpts(right_edge - left_edge + pitch * 2 + 1);
  // local min results
  std::vector<bool> mins(pitch_error * 2 + 1);
  for (x = array_origin; x < left_edge; x++) {
    // free cuts
    cutpts[x - array_origin].setup(&cutpts[0], array_origin, projection, zero_count, pitch, x, 0);
  }
  prev_zero = left_edge - 1;
  for (offset = 0; offset <= pitch_error; offset++, x++) {
    // not quite free
    cutpts[x - array_origin].setup(&cutpts[0], array_origin, projection, zero_count, pitch, x,
                                   offset);
  }

  best_cost = FLT_MAX;
  best_end = nullptr;
  for (offset = -pitch_error, minindex = 0; offset < pitch_error; offset++, minindex++) {
    mins[minindex] = projection->local_min(x + offset);
  }
  next_zero = x + zero_offset + 1;
  for (offset = next_zero - 1; offset >= x; offset--) {
    if (projection->pile_count(offset) <= zero_count) {
      next_zero = offset;
      break;
    }
  }
  while (x < right_edge - pitch_error) {
    mins[minindex] = projection->local_min(x + pitch_error);
    minindex++;
    if (minindex > pitch_error * 2) {
      minindex = 0;
    }
    faking = false;
    mid_cut = false;
    offset = 0;
    if (projection->pile_count(x) <= zero_count) {
      prev_zero = x;
    } else {
      for (offset = 1; offset <= pitch_error; offset++) {
        if (projection->pile_count(x + offset) <= zero_count ||
            projection->pile_count(x - offset) <= zero_count) {
          break;
        }
      }
    }
    if (offset > pitch_error) {
      if (x - prev_zero > zero_offset && next_zero - x > zero_offset) {
        for (offset = 0; offset <= pitch_error; offset++) {
          test_index = minindex + pitch_error + offset;
          if (test_index > pitch_error * 2) {
            test_index -= pitch_error * 2 + 1;
          }
          if (mins[test_index]) {
            break;
          }
          test_index = minindex + pitch_error - offset;
          if (test_index > pitch_error * 2) {
            test_index -= pitch_error * 2 + 1;
          }
          if (mins[test_index]) {
            break;
          }
        }
      }
      if (offset > pitch_error) {
        offset = projection->pile_count(x);
        faking = true;
      } else {
        projection_offset = static_cast<int16_t>(projection->pile_count(x) / projection_scale);
        if (projection_offset > offset) {
          offset = projection_offset;
        }
        mid_cut = true;
      }
    }
    if ((start == 0 && end == 0) || !textord_fast_pitch_test ||
        (x - projection_left - start) % pitch <= end) {
      cutpts[x - array_origin].assign(&cutpts[0], array_origin, x, faking, mid_cut, offset,
                                      projection, projection_scale, zero_count, pitch, pitch_error);
    } else {
      cutpts[x - array_origin].assign_cheap(&cutpts[0], array_origin, x, faking, mid_cut, offset,
                                            projection, projection_scale, zero_count, pitch,
                                            pitch_error);
    }
    x++;
    if (next_zero < x || next_zero == x + zero_offset) {
      next_zero = x + zero_offset + 1;
    }
    if (projection->pile_count(x + zero_offset) <= zero_count) {
      next_zero = x + zero_offset;
    }
  }

  best_fake = INT16_MAX;
  best_cost = INT32_MAX;
  best_count = INT16_MAX;
  while (x < right_edge + pitch) {
    offset = x < right_edge ? right_edge - x : 0;
    cutpts[x - array_origin].assign(&cutpts[0], array_origin, x, false, false, offset, projection,
                                    projection_scale, zero_count, pitch, pitch_error);
    cutpts[x - array_origin].terminal = true;
    if (cutpts[x - array_origin].index() + cutpts[x - array_origin].fake_count <=
        best_count + best_fake) {
      if (cutpts[x - array_origin].fake_count < best_fake ||
          (cutpts[x - array_origin].fake_count == best_fake &&
           cutpts[x - array_origin].cost_function() < best_cost)) {
        best_fake = cutpts[x - array_origin].fake_count;
        best_cost = cutpts[x - array_origin].cost_function();
        best_left_x = x;
        best_right_x = x;
        best_count = cutpts[x - array_origin].index();
      } else if (cutpts[x - array_origin].fake_count == best_fake && x == best_right_x + 1 &&
                 cutpts[x - array_origin].cost_function() == best_cost) {
        // exactly equal
        best_right_x = x;
      }
    }
    x++;
  }
  ASSERT_HOST(best_fake < INT16_MAX);

  best_end = &cutpts[(best_left_x + best_right_x) / 2 - array_origin];
  //      for (x=left_edge-pitch;x<right_edge+pitch;x++)
  //      {
  //              tprintf("x=%d, C=%g, s=%g, sq=%g, prev=%d\n",
  //                      x,cutpts[x-array_origin].cost_function(),
  //                      cutpts[x-array_origin].sum(),
  //                      cutpts[x-array_origin].squares(),
  //                      cutpts[x-array_origin].previous()->position());
  //      }
  occupation_count = -1;
  do {
    for (x = best_end->position() - pitch + pitch_error;
         x < best_end->position() - pitch_error && projection->pile_count(x) == 0; x++) {
    }
    if (x < best_end->position() - pitch_error) {
      occupation_count++;
    }
    // copy it
    segpt = new FPSEGPT(best_end);
    seg_it.add_before_then_move(segpt);
    best_end = best_end->previous();
  } while (best_end != nullptr);
  seg_it.move_to_last();
  mean_sum = seg_it.data()->sum();
  mean_sum = mean_sum * mean_sum / best_count;
  if (seg_it.data()->squares() - mean_sum < 0) {
    tprintf("Impossible sqsum=%g, mean=%g, total=%d\n", seg_it.data()->squares(),
            seg_it.data()->sum(), best_count);
  }
  return seg_it.data()->squares() - mean_sum;
}

} // namespace tesseract
