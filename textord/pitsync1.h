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

#ifndef           PITSYNC1_H
#define           PITSYNC1_H

#include          "elst.h"
#include          "clst.h"
#include          "blobbox.h"
#include          "params.h"
#include          "statistc.h"
#include          "pithsync.h"

class FPSEGPT_LIST;

class FPSEGPT:public ELIST_LINK
{
  public:
    FPSEGPT() {  //empty
    }
    FPSEGPT(           //constructor
            inT16 x);  //position
    FPSEGPT(                           //constructor
            inT16 x,                   //position
            BOOL8 faking,              //faking this one
            inT16 offset,              //extra cost dist
            inT16 region_index,        //segment number
            inT16 pitch,               //proposed pitch
            inT16 pitch_error,         //allowed tolerance
            FPSEGPT_LIST *prev_list);  //previous segment
    FPSEGPT(FPCUTPT *cutpt);  //build from new type

    inT32 position() {  // access func
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
    inT16 cheap_cuts() const {  //no of cheap cuts
      return mid_cuts;
    }

                                 //faked split point
    BOOL8 faked;
    BOOL8 terminal;              //successful end
    inT16 fake_count;            //total fakes to here

  private:
    inT16 mid_cuts;              //no of cheap cuts
    inT32 xpos;                  //location
    FPSEGPT *pred;               //optimal previous
    double mean_sum;             //mean so far
    double sq_sum;               //summed distsances
    double cost;                 //cost function
};

ELISTIZEH (FPSEGPT) CLISTIZEH (FPSEGPT_LIST)
extern
INT_VAR_H (pitsync_linear_version, 0, "Use new fast algorithm");
extern
double_VAR_H (pitsync_joined_edge, 0.75,
"Dist inside big blob for chopping");
extern
double_VAR_H (pitsync_offset_freecut_fraction, 0.25,
"Fraction of cut for free cuts");
extern
INT_VAR_H (pitsync_fake_depth, 1, "Max advance fake generation");
double check_pitch_sync(                        //find segmentation
                        BLOBNBOX_IT *blob_it,   //blobs to do
                        inT16 blob_count,       //no of blobs
                        inT16 pitch,            //pitch estimate
                        inT16 pitch_error,      //tolerance
                        STATS *projection,      //vertical
                        FPSEGPT_LIST *seg_list  //output list
                       );
void make_illegal_segment(                          //find segmentation
                          FPSEGPT_LIST *prev_list,  //previous segments
                          TBOX blob_box,             //bounding box
                          BLOBNBOX_IT blob_it,      //iterator
                          inT16 region_index,       //number of segment
                          inT16 pitch,              //pitch estimate
                          inT16 pitch_error,        //tolerance
                          FPSEGPT_LIST *seg_list    //output list
                         );
inT16 vertical_torow_projection(                   //project whole row
                                TO_ROW *row,       //row to do
                                STATS *projection  //output
                               );
void vertical_cblob_projection(               //project outlines
                               C_BLOB *blob,  //blob to project
                               STATS *stats   //output
                              );
void vertical_coutline_projection(                     //project outlines
                                  C_OUTLINE *outline,  //outline to project
                                  STATS *stats         //output
                                 );
#endif
