/**********************************************************************
 * File:        pithsync.h  (Formerly pitsync2.h)
 * Description: Code to find the optimum fixed pitch segmentation of some blobs.
 * Author:		Ray Smith
 * Created:		Thu Nov 19 11:48:05 GMT 1992
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

#ifndef           PITHSYNC_H
#define           PITHSYNC_H

#include          "blobbox.h"
#include          "params.h"
#include          "statistc.h"

class FPSEGPT_LIST;

class FPCUTPT
{
  public:
    FPCUTPT() {  //empty
    }
    void setup (                 //start of cut
      FPCUTPT cutpts[],          //predecessors
      inT16 array_origin,        //start coord
      STATS * projection,        //occupation
      inT16 zero_count,          //official zero
      inT16 pitch,               //proposed pitch
      inT16 x,                   //position
      inT16 offset);             //dist to gap

    void assign (                //evaluate cut
      FPCUTPT cutpts[],          //predecessors
      inT16 array_origin,        //start coord
      inT16 x,                   //position
      BOOL8 faking,              //faking this one
      BOOL8 mid_cut,             //doing free cut
      inT16 offset,              //extra cost dist
      STATS * projection,        //occupation
      float projection_scale,    //scaling
      inT16 zero_count,          //official zero
      inT16 pitch,               //proposed pitch
      inT16 pitch_error);        //allowed tolerance

    void assign_cheap (          //evaluate cut
      FPCUTPT cutpts[],          //predecessors
      inT16 array_origin,        //start coord
      inT16 x,                   //position
      BOOL8 faking,              //faking this one
      BOOL8 mid_cut,             //doing free cut
      inT16 offset,              //extra cost dist
      STATS * projection,        //occupation
      float projection_scale,    //scaling
      inT16 zero_count,          //official zero
      inT16 pitch,               //proposed pitch
      inT16 pitch_error);        //allowed tolerance

    inT32 position() {  //acces func
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
    inT16 cheap_cuts() const {  //no of mi cuts
      return mid_cuts;
    }
    inT16 index() const {
      return region_index;
    }

    BOOL8 faked;                 //faked split point
    BOOL8 terminal;              //successful end
    inT16 fake_count;            //total fakes to here

  private:
    inT16 region_index;          //cut serial number
    inT16 mid_cuts;              //no of cheap cuts
    inT32 xpos;                  //location
    uinT32 back_balance;         //proj backwards
    uinT32 fwd_balance;          //proj forwards
    FPCUTPT *pred;               //optimal previous
    double mean_sum;             //mean so far
    double sq_sum;               //summed distsances
    double cost;                 //cost function
};
double check_pitch_sync2(                          //find segmentation
                         BLOBNBOX_IT *blob_it,     //blobs to do
                         inT16 blob_count,         //no of blobs
                         inT16 pitch,              //pitch estimate
                         inT16 pitch_error,        //tolerance
                         STATS *projection,        //vertical
                         inT16 projection_left,    //edges //scale factor
                         inT16 projection_right,
                         float projection_scale,
                         inT16 &occupation_count,  //no of occupied cells
                         FPSEGPT_LIST *seg_list,   //output list
                         inT16 start,              //start of good range
                         inT16 end                 //end of good range
                        );
double check_pitch_sync3(                          //find segmentation
                         inT16 projection_left,    //edges //to be considered 0
                         inT16 projection_right,
                         inT16 zero_count,
                         inT16 pitch,              //pitch estimate
                         inT16 pitch_error,        //tolerance
                         STATS *projection,        //vertical
                         float projection_scale,   //scale factor
                         inT16 &occupation_count,  //no of occupied cells
                         FPSEGPT_LIST *seg_list,   //output list
                         inT16 start,              //start of good range
                         inT16 end                 //end of good range
                        );
#endif
