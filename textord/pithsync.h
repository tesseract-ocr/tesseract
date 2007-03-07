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
#include          "varable.h"
#include          "statistc.h"
#include          "notdll.h"

class FPSEGPT_LIST;

class FPCUTPT
{
  public:
    FPCUTPT() {  //empty
    }
    void setup (                 //start of cut
      FPCUTPT cutpts[],          //predecessors
      INT16 array_origin,        //start coord
      STATS * projection,        //occupation
      INT16 zero_count,          //official zero
      INT16 pitch,               //proposed pitch
      INT16 x,                   //position
      INT16 offset);             //dist to gap

    void assign (                //evaluate cut
      FPCUTPT cutpts[],          //predecessors
      INT16 array_origin,        //start coord
      INT16 x,                   //position
      BOOL8 faking,              //faking this one
      BOOL8 mid_cut,             //doing free cut
      INT16 offset,              //extra cost dist
      STATS * projection,        //occupation
      float projection_scale,    //scaling
      INT16 zero_count,          //official zero
      INT16 pitch,               //proposed pitch
      INT16 pitch_error);        //allowed tolerance

    void assign_cheap (          //evaluate cut
      FPCUTPT cutpts[],          //predecessors
      INT16 array_origin,        //start coord
      INT16 x,                   //position
      BOOL8 faking,              //faking this one
      BOOL8 mid_cut,             //doing free cut
      INT16 offset,              //extra cost dist
      STATS * projection,        //occupation
      float projection_scale,    //scaling
      INT16 zero_count,          //official zero
      INT16 pitch,               //proposed pitch
      INT16 pitch_error);        //allowed tolerance

    INT32 position() {  //acces func
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
    INT16 cheap_cuts() const {  //no of mi cuts
      return mid_cuts;
    }
    INT16 index() const { 
      return region_index;
    }

    BOOL8 faked;                 //faked split point
    BOOL8 terminal;              //successful end
    INT16 fake_count;            //total fakes to here

  private:
    INT16 region_index;          //cut serial number
    INT16 mid_cuts;              //no of cheap cuts
    INT32 xpos;                  //location
    UINT32 back_balance;         //proj backwards
    UINT32 fwd_balance;          //proj forwards
    FPCUTPT *pred;               //optimal previous
    double mean_sum;             //mean so far
    double sq_sum;               //summed distsances
    double cost;                 //cost function
};
double check_pitch_sync2(                          //find segmentation
                         BLOBNBOX_IT *blob_it,     //blobs to do
                         INT16 blob_count,         //no of blobs
                         INT16 pitch,              //pitch estimate
                         INT16 pitch_error,        //tolerance
                         STATS *projection,        //vertical
                         INT16 projection_left,    //edges //scale factor
                         INT16 projection_right,
                         float projection_scale,
                         INT16 &occupation_count,  //no of occupied cells
                         FPSEGPT_LIST *seg_list,   //output list
                         INT16 start,              //start of good range
                         INT16 end                 //end of good range
                        );
double check_pitch_sync3(                          //find segmentation
                         INT16 projection_left,    //edges //to be considered 0
                         INT16 projection_right,
                         INT16 zero_count,
                         INT16 pitch,              //pitch estimate
                         INT16 pitch_error,        //tolerance
                         STATS *projection,        //vertical
                         float projection_scale,   //scale factor
                         INT16 &occupation_count,  //no of occupied cells
                         FPSEGPT_LIST *seg_list,   //output list
                         INT16 start,              //start of good range
                         INT16 end                 //end of good range
                        );
#endif
