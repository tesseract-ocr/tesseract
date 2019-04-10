/**********************************************************************
 * File:        pitsync1.cpp  (Formerly pitsync.c)
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

#include <cfloat>      // for FLT_MAX
#include <cmath>
#include "pitsync1.h"

ELISTIZE (FPSEGPT) CLISTIZE (FPSEGPT_LIST)

INT_VAR(pitsync_linear_version, 6, "Use new fast algorithm");
double_VAR(pitsync_joined_edge, 0.75, "Dist inside big blob for chopping");
double_VAR(pitsync_offset_freecut_fraction, 0.25,
  "Fraction of cut for free cuts");
INT_VAR(pitsync_fake_depth, 1, "Max advance fake generation");

/**********************************************************************
 * FPSEGPT::FPSEGPT
 *
 * Constructor to make a new FPSEGPT.
 * The existing FPCUTPT is duplicated.
 **********************************************************************/

FPSEGPT::FPSEGPT(                //constructor
                 FPCUTPT *cutpt  //create from new form
                ) {
  pred = nullptr;
  mean_sum = cutpt->sum ();
  sq_sum = cutpt->squares ();
  cost = cutpt->cost_function ();
  faked = cutpt->faked;
  terminal = cutpt->terminal;
  fake_count = cutpt->fake_count;
  xpos = cutpt->position ();
  mid_cuts = cutpt->cheap_cuts ();
}


/**********************************************************************
 * FPSEGPT::FPSEGPT
 *
 * Constructor to make a new FPSEGPT.
 **********************************************************************/

FPSEGPT::FPSEGPT (               //constructor
int16_t x                        //position
):xpos (x) {
  pred = nullptr;
  mean_sum = 0;
  sq_sum = 0;
  cost = 0;
  faked = false;
  terminal = false;
  fake_count = 0;
  mid_cuts = 0;
}


/**********************************************************************
 * FPSEGPT::FPSEGPT
 *
 * Constructor to make a new FPSEGPT.
 **********************************************************************/

FPSEGPT::FPSEGPT (               //constructor
int16_t x,                       //position
bool faking,                     //faking this one
int16_t offset,                  //dist to gap
int16_t region_index,            //segment number
int16_t pitch,                   //proposed pitch
int16_t pitch_error,             //allowed tolerance
FPSEGPT_LIST * prev_list         //previous segment
)
: fake_count(0),
  xpos(x),
  mean_sum(0.0),
  sq_sum(0.0)
{
  int16_t best_fake;             //on previous
  FPSEGPT *segpt;                //segment point
  int32_t dist;                  //from prev segment
  double sq_dist;                //squared distance
  double mean;                   //mean pitch
  double total;                  //total dists
  double factor;                 //cost function
  FPSEGPT_IT pred_it = prev_list;//for previuos segment

  cost = FLT_MAX;
  pred = nullptr;
  faked = faking;
  terminal = false;
  best_fake = INT16_MAX;
  mid_cuts = 0;
  for (pred_it.mark_cycle_pt (); !pred_it.cycled_list (); pred_it.forward ()) {
    segpt = pred_it.data ();
    if (segpt->fake_count < best_fake)
      best_fake = segpt->fake_count;
    dist = x - segpt->xpos;
    if (dist >= pitch - pitch_error && dist <= pitch + pitch_error
    && !segpt->terminal) {
      total = segpt->mean_sum + dist;
      sq_dist = dist * dist + segpt->sq_sum + offset * offset;
      //sum of squarees
      mean = total / region_index;
      factor = mean - pitch;
      factor *= factor;
      factor += sq_dist / (region_index) - mean * mean;
      if (factor < cost) {
        cost = factor;           //find least cost
        pred = segpt;            //save path
        mean_sum = total;
        sq_sum = sq_dist;
        fake_count = segpt->fake_count + faked;
      }
    }
  }
  if (fake_count > best_fake + 1)
    pred = nullptr;                 //fail it
}

/**********************************************************************
 * check_pitch_sync
 *
 * Construct the lattice of possible segmentation points and choose the
 * optimal path. Return the optimal path only.
 * The return value is a measure of goodness of the sync.
 **********************************************************************/

double check_pitch_sync(                        //find segmentation
                        BLOBNBOX_IT *blob_it,   //blobs to do
                        int16_t blob_count,     //no of blobs
                        int16_t pitch,          //pitch estimate
                        int16_t pitch_error,    //tolerance
                        STATS *projection,      //vertical
                        FPSEGPT_LIST *seg_list  //output list
                       ) {
  int16_t x;                     //current coord
  int16_t min_index;             //blob number
  int16_t max_index;             //blob number
  int16_t left_edge;             //of word
  int16_t right_edge;            //of word
  int16_t right_max;             //max allowed x
  int16_t min_x;                 //in this region
  int16_t max_x;
  int16_t region_index;
  int16_t best_region_index = 0; //for best result
  int16_t offset;                //dist to legal area
  int16_t left_best_x;           //edge of good region
  int16_t right_best_x;          //right edge
  TBOX min_box;                  //bounding box
  TBOX max_box;                  //bounding box
  TBOX next_box;                 //box of next blob
  FPSEGPT *segpt;                //segment point
  FPSEGPT_LIST *segpts;          //points in a segment
  double best_cost;              //best path
  double mean_sum;               //computes result
  FPSEGPT *best_end;             //end of best path
  BLOBNBOX_IT min_it;            //copy iterator
  BLOBNBOX_IT max_it;            //copy iterator
  FPSEGPT_IT segpt_it;           //iterator
                                 //output segments
  FPSEGPT_IT outseg_it = seg_list;
  FPSEGPT_LIST_CLIST lattice;    //list of lists
                                 //region iterator
  FPSEGPT_LIST_C_IT lattice_it = &lattice;

  //      tprintf("Computing sync on word of %d blobs with pitch %d\n",
  //              blob_count, pitch);
  //      if (blob_count==8 && pitch==27)
  //              projection->print(stdout,true);
  if (pitch < 3)
    pitch = 3;                   //nothing ludicrous
  if ((pitch - 3) / 2 < pitch_error)
    pitch_error = (pitch - 3) / 2;
  min_it = *blob_it;
  min_box = box_next (&min_it);  //get box
  //      if (blob_count==8 && pitch==27)
  //              tprintf("1st box at (%d,%d)->(%d,%d)\n",
  //                      min_box.left(),min_box.bottom(),
  //                      min_box.right(),min_box.top());
                                 //left of word
  left_edge = min_box.left () + pitch_error;
  for (min_index = 1; min_index < blob_count; min_index++) {
    min_box = box_next (&min_it);
    //              if (blob_count==8 && pitch==27)
    //                      tprintf("Box at (%d,%d)->(%d,%d)\n",
    //                              min_box.left(),min_box.bottom(),
    //                              min_box.right(),min_box.top());
  }
  right_edge = min_box.right (); //end of word
  max_x = left_edge;
                                 //min permissible
  min_x = max_x - pitch + pitch_error * 2 + 1;
  right_max = right_edge + pitch - pitch_error - 1;
  segpts = new FPSEGPT_LIST;     //list of points
  segpt_it.set_to_list (segpts);
  for (x = min_x; x <= max_x; x++) {
    segpt = new FPSEGPT (x);     //make a new one
                                 //put in list
    segpt_it.add_after_then_move (segpt);
  }
                                 //first segment
  lattice_it.add_before_then_move (segpts);
  min_index = 0;
  region_index = 1;
  best_cost = FLT_MAX;
  best_end = nullptr;
  min_it = *blob_it;
  min_box = box_next (&min_it);  //first box
  do {
    left_best_x = -1;
    right_best_x = -1;
    segpts = new FPSEGPT_LIST;   //list of points
    segpt_it.set_to_list (segpts);
    min_x += pitch - pitch_error;//next limits
    max_x += pitch + pitch_error;
    while (min_box.right () < min_x && min_index < blob_count) {
      min_index++;
      min_box = box_next (&min_it);
    }
    max_it = min_it;
    max_index = min_index;
    max_box = min_box;
    next_box = box_next (&max_it);
    for (x = min_x; x <= max_x && x <= right_max; x++) {
      while (x < right_edge && max_index < blob_count
      && x > max_box.right ()) {
        max_index++;
        max_box = next_box;
        next_box = box_next (&max_it);
      }
      if (x <= max_box.left () + pitch_error
        || x >= max_box.right () - pitch_error || x >= right_edge
        || (max_index < blob_count - 1 && x >= next_box.left ())
        || (x - max_box.left () > pitch * pitsync_joined_edge
      && max_box.right () - x > pitch * pitsync_joined_edge)) {
      //                      || projection->local_min(x))
        if (x - max_box.left () > 0
          && x - max_box.left () <= pitch_error)
                                 //dist to real break
          offset = x - max_box.left ();
        else if (max_box.right () - x > 0
          && max_box.right () - x <= pitch_error
          && (max_index >= blob_count - 1
          || x < next_box.left ()))
          offset = max_box.right () - x;
        else
          offset = 0;
        //                              offset=pitsync_offset_freecut_fraction*projection->pile_count(x);
        segpt = new FPSEGPT (x, false, offset, region_index,
          pitch, pitch_error, lattice_it.data ());
      }
      else {
        offset = projection->pile_count (x);
        segpt = new FPSEGPT (x, true, offset, region_index,
          pitch, pitch_error, lattice_it.data ());
      }
      if (segpt->previous () != nullptr) {
        segpt_it.add_after_then_move (segpt);
        if (x >= right_edge - pitch_error) {
          segpt->terminal = true;//no more wanted
          if (segpt->cost_function () < best_cost) {
            best_cost = segpt->cost_function ();
            //find least
            best_end = segpt;
            best_region_index = region_index;
            left_best_x = x;
            right_best_x = x;
          }
          else if (segpt->cost_function () == best_cost
            && right_best_x == x - 1)
            right_best_x = x;
        }
      }
      else {
        delete segpt;            //no good
      }
    }
    if (segpts->empty ()) {
      if (best_end != nullptr)
        break;                   //already found one
      make_illegal_segment (lattice_it.data (), min_box, min_it,
        region_index, pitch, pitch_error, segpts);
    }
    else {
      if (right_best_x > left_best_x + 1) {
        left_best_x = (left_best_x + right_best_x + 1) / 2;
        for (segpt_it.mark_cycle_pt (); !segpt_it.cycled_list ()
          && segpt_it.data ()->position () != left_best_x;
          segpt_it.forward ());
        if (segpt_it.data ()->position () == left_best_x)
                                 //middle of region
          best_end = segpt_it.data ();
      }
    }
                                 //new segment
    lattice_it.add_before_then_move (segpts);
    region_index++;
  }
  while (min_x < right_edge);
  ASSERT_HOST (best_end != nullptr);//must always find some

  for (lattice_it.mark_cycle_pt (); !lattice_it.cycled_list ();
  lattice_it.forward ()) {
    segpts = lattice_it.data ();
    segpt_it.set_to_list (segpts);
    //              if (blob_count==8 && pitch==27)
    //              {
    //                      for (segpt_it.mark_cycle_pt();!segpt_it.cycled_list();segpt_it.forward())
    //                      {
    //                              segpt=segpt_it.data();
    //                              tprintf("At %d, (%x) cost=%g, m=%g, sq=%g, pred=%x\n",
    //                                      segpt->position(),segpt,segpt->cost_function(),
    //                                      segpt->sum(),segpt->squares(),segpt->previous());
    //                      }
    //                      tprintf("\n");
    //              }
    for (segpt_it.mark_cycle_pt (); !segpt_it.cycled_list ()
      && segpt_it.data () != best_end; segpt_it.forward ());
    if (segpt_it.data () == best_end) {
                                 //save good one
      segpt = segpt_it.extract ();
      outseg_it.add_before_then_move (segpt);
      best_end = segpt->previous ();
    }
  }
  ASSERT_HOST (best_end == nullptr);
  ASSERT_HOST (!outseg_it.empty ());
  outseg_it.move_to_last ();
  mean_sum = outseg_it.data ()->sum ();
  mean_sum = mean_sum * mean_sum / best_region_index;
  if (outseg_it.data ()->squares () - mean_sum < 0)
    tprintf ("Impossible sqsum=%g, mean=%g, total=%d\n",
      outseg_it.data ()->squares (), outseg_it.data ()->sum (),
      best_region_index);
  lattice.deep_clear ();         //shift the lot
  return outseg_it.data ()->squares () - mean_sum;
}


/**********************************************************************
 * make_illegal_segment
 *
 * Make a fake set of chop points due to having no legal places.
 **********************************************************************/

void make_illegal_segment(                          //find segmentation
                          FPSEGPT_LIST *prev_list,  //previous segments
                          TBOX blob_box,            //bounding box
                          BLOBNBOX_IT blob_it,      //iterator
                          int16_t region_index,     //number of segment
                          int16_t pitch,            //pitch estimate
                          int16_t pitch_error,      //tolerance
                          FPSEGPT_LIST *seg_list    //output list
                         ) {
  int16_t x;                     //current coord
  int16_t min_x = 0;             //in this region
  int16_t max_x = 0;
  int16_t offset;                //dist to edge
  FPSEGPT *segpt;                //segment point
  FPSEGPT *prevpt;               //previous point
  float best_cost;               //best path
  FPSEGPT_IT segpt_it = seg_list;//iterator
                                 //previous points
  FPSEGPT_IT prevpt_it = prev_list;

  best_cost = FLT_MAX;
  for (prevpt_it.mark_cycle_pt (); !prevpt_it.cycled_list ();
  prevpt_it.forward ()) {
    prevpt = prevpt_it.data ();
    if (prevpt->cost_function () < best_cost) {
                                 //find least
      best_cost = prevpt->cost_function ();
      min_x = prevpt->position ();
      max_x = min_x;             //limits on coords
    }
    else if (prevpt->cost_function () == best_cost) {
      max_x = prevpt->position ();
    }
  }
  min_x += pitch - pitch_error;
  max_x += pitch + pitch_error;
  for (x = min_x; x <= max_x; x++) {
    while (x > blob_box.right ()) {
      blob_box = box_next (&blob_it);
    }
    offset = x - blob_box.left ();
    if (blob_box.right () - x < offset)
      offset = blob_box.right () - x;
    segpt = new FPSEGPT (x, false, offset,
      region_index, pitch, pitch_error, prev_list);
    if (segpt->previous () != nullptr) {
      ASSERT_HOST (offset >= 0);
      fprintf (stderr, "made fake at %d\n", x);
                                 //make one up
      segpt_it.add_after_then_move (segpt);
      segpt->faked = true;
      segpt->fake_count++;
    }
    else
      delete segpt;
  }
}
