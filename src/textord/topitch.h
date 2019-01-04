/**********************************************************************
 * File:        topitch.h  (Formerly to_pitch.h)
 * Description: Code to determine fixed pitchness and the pitch if fixed.
 * Author:      Ray Smith
 *
 * (C) Copyright 1993, Hewlett-Packard Ltd.
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

#ifndef           TOPITCH_H
#define           TOPITCH_H

#include          "blobbox.h"

namespace tesseract {
class Tesseract;
}
extern BOOL_VAR_H (textord_debug_pitch_test, FALSE,
"Debug on fixed pitch test");
extern BOOL_VAR_H (textord_debug_pitch_metric, FALSE,
"Write full metric stuff");
extern BOOL_VAR_H (textord_show_row_cuts, FALSE, "Draw row-level cuts");
extern BOOL_VAR_H (textord_show_page_cuts, FALSE, "Draw page-level cuts");
extern BOOL_VAR_H (textord_pitch_cheat, FALSE,
"Use correct answer for fixed/prop");
extern BOOL_VAR_H (textord_blockndoc_fixed, TRUE,
"Attempt whole doc/block fixed pitch");
extern BOOL_VAR_H (textord_fast_pitch_test, FALSE,
"Do even faster pitch algorithm");
extern double_VAR_H (textord_projection_scale, 0.125,
"Ding rate for mid-cuts");
extern double_VAR_H (textord_balance_factor, 2.0,
"Ding rate for unbalanced char cells");

void compute_fixed_pitch(ICOORD page_tr,              // top right
                         TO_BLOCK_LIST* port_blocks,  // input list
                         float gradient,              // page skew
                         FCOORD rotation,             // for drawing
                         bool testing_on);           // correct orientation
void fix_row_pitch(                        //get some value
                   TO_ROW *bad_row,        //row to fix
                   TO_BLOCK *bad_block,    //block of bad_row
                   TO_BLOCK_LIST *blocks,  //blocks to scan
                   int32_t row_target,       //number of row
                   int32_t block_target      //number of block
                  );
void compute_block_pitch(TO_BLOCK* block,     // input list
                         FCOORD rotation,      // for drawing
                         int32_t block_index,    // block number
                         bool testing_on);    // correct orientation
bool compute_rows_pitch(                    //find line stats
        TO_BLOCK* block,    //block to do
        int32_t block_index,  //block number
        bool testing_on    //correct orientation
);
bool try_doc_fixed(                             //determine pitch
        ICOORD page_tr,              //top right
        TO_BLOCK_LIST* port_blocks,  //input list
        float gradient               //page skew
);
bool try_block_fixed(                   //find line stats
        TO_BLOCK* block,   //block to do
        int32_t block_index  //block number
);
bool try_rows_fixed(                    //find line stats
        TO_BLOCK* block,    //block to do
        int32_t block_index,  //block number
        bool testing_on    //correct orientation
);
void print_block_counts(                   //find line stats
                        TO_BLOCK *block,   //block to do
                        int32_t block_index  //block number
                       );
void count_block_votes(                   //find line stats
                       TO_BLOCK *block,   //block to do
                       int32_t &def_fixed,  //add to counts
                       int32_t &def_prop,
                       int32_t &maybe_fixed,
                       int32_t &maybe_prop,
                       int32_t &corr_fixed,
                       int32_t &corr_prop,
                       int32_t &dunno);
bool row_pitch_stats(                  //find line stats
        TO_ROW* row,      //current row
        int32_t maxwidth,   //of spaces
        bool testing_on  //correct orientation
);
bool find_row_pitch(                    //find lines
        TO_ROW* row,        //row to do
        int32_t maxwidth,     //max permitted space
        int32_t dm_gap,       //ignorable gaps
        TO_BLOCK* block,    //block of row
        int32_t block_index,  //block_number
        int32_t row_index,    //number of row
        bool testing_on    //correct orientation
);
bool fixed_pitch_row(                   //find lines
        TO_ROW* row,       //row to do
        BLOCK* block,
        int32_t block_index  //block_number
);
bool count_pitch_stats(                       //find lines
        TO_ROW* row,           //row to do
        STATS* gap_stats,      //blob gaps
        STATS* pitch_stats,    //centre-centre stats
        float initial_pitch,   //guess at pitch
        float min_space,       //estimate space size
        bool ignore_outsize,  //discard big objects
        bool split_outsize,   //split big objects
        int32_t dm_gap           //ignorable gaps
);
float tune_row_pitch(                             //find fp cells
        TO_ROW* row,                 //row to do
        STATS* projection,           //vertical projection
        int16_t projection_left,       //edge of projection
        int16_t projection_right,      //edge of projection
        float space_size,            //size of blank
        float& initial_pitch,        //guess at pitch
        float& best_sp_sd,           //space sd
        int16_t& best_mid_cuts,        //no of cheap cuts
        ICOORDELT_LIST* best_cells,  //row cells
        bool testing_on             //inidividual words
);
float tune_row_pitch2(                             //find fp cells
        TO_ROW* row,                 //row to do
        STATS* projection,           //vertical projection
        int16_t projection_left,       //edge of projection
        int16_t projection_right,      //edge of projection
        float space_size,            //size of blank
        float& initial_pitch,        //guess at pitch
        float& best_sp_sd,           //space sd
        int16_t& best_mid_cuts,        //no of cheap cuts
        ICOORDELT_LIST* best_cells,  //row cells
        bool testing_on             //inidividual words
);
float compute_pitch_sd(         //find fp cells
        TO_ROW* row,                    //row to do
        STATS* projection,              //vertical projection
        int16_t projection_left,           //edge
        int16_t projection_right,          //edge
        float space_size,                //size of blank
        float initial_pitch,             //guess at pitch
        float& sp_sd,                    //space sd
        int16_t& mid_cuts,                //no of free cuts
        ICOORDELT_LIST* row_cells,      //list of chop pts
        bool testing_on,                //inidividual words
        int16_t start = 0,                 //start of good range
        int16_t end = 0                    //end of good range
);
float compute_pitch_sd2(        //find fp cells
        TO_ROW* row,                    //row to do
        STATS* projection,              //vertical projection
        int16_t projection_left,           //edge
        int16_t projection_right,          //edge
        float initial_pitch,             //guess at pitch
        int16_t& occupation,              //no of occupied cells
        int16_t& mid_cuts,                //no of free cuts
        ICOORDELT_LIST* row_cells,      //list of chop pts
        bool testing_on,                //inidividual words
        int16_t start = 0,                 //start of good range
        int16_t end = 0                    //end of good range
);
void print_pitch_sd(                        //find fp cells
                    TO_ROW *row,            //row to do
                    STATS *projection,      //vertical projection
                    int16_t projection_left,  //edges //size of blank
                    int16_t projection_right,
                    float space_size,
                    float initial_pitch     //guess at pitch
                   );
void find_repeated_chars(TO_BLOCK* block,    // Block to search.
                         bool testing_on);  // Debug mode.
void plot_fp_word(                  //draw block of words
                  TO_BLOCK *block,  //block to draw
                  float pitch,      //pitch to draw with
                  float nonspace    //for space threshold
                 );
#endif
