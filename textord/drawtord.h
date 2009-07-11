/**********************************************************************
 * File:        drawtord.h  (Formerly drawto.h)
 * Description: Draw things to do with textord.
 * Author:		Ray Smith
 * Created:		Thu Jul 30 15:40:57 BST 1992
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

#ifndef           DRAWTORD_H
#define           DRAWTORD_H

#include          "varable.h"
#include          "scrollview.h"
#include          "pitsync1.h"
#include          "blobbox.h"
#include          "notdll.h"

#define NO_SMD        "none"

extern BOOL_VAR_H (textord_show_fixed_cuts, FALSE,
"Draw fixed pitch cell boundaries");
extern STRING_VAR_H (to_debugfile, DEBUG_WIN_NAME, "Name of debugfile");
extern STRING_VAR_H (to_smdfile, NO_SMD, "Name of SMD file");
extern ScrollView* to_win;
extern FILE *to_debug;
void create_to_win(                //make features win
                   ICOORD page_tr  //size of page
                  );
void close_to_win();  //make features win
void create_todebug_win();  //make gradients win
void plot_box_list(                      //make gradients win
                   ScrollView* win,           //window to draw in
                   BLOBNBOX_LIST *list,  //blob list
                   ScrollView::Color body_colour    //colour to draw
                  );
void plot_to_row(                 //draw a row
                 TO_ROW *row,     //row to draw
                 ScrollView::Color colour,   //colour to draw in
                 FCOORD rotation  //rotation for line
                );
void plot_parallel_row(                 //draw a row
                       TO_ROW *row,     //row to draw
                       float gradient,  //gradients of lines
                       inT32 left,      //edge of block
                       ScrollView::Color colour,   //colour to draw in
                       FCOORD rotation  //rotation for line
                      );
void draw_occupation (           //draw projection
inT32 xleft,                     //edge of block
inT32 ybottom,                   //bottom of block
inT32 min_y,                     //coordinate limits
inT32 max_y, inT32 occupation[], //projection counts
inT32 thresholds[]               //for drop out
);
void draw_meanlines(                  //draw a block
                    TO_BLOCK *block,  //block to draw
                    float gradient,   //gradients of lines
                    inT32 left,       //edge of block
                    ScrollView::Color colour,    //colour to draw in
                    FCOORD rotation   //rotation for line
                   );
void plot_word_decisions(              //draw words
                         ScrollView* win,   //window tro draw in
                         inT16 pitch,  //of block
                         TO_ROW *row   //row to draw
                        );
void plot_fp_cells(                        //draw words
                   ScrollView* win,             //window tro draw in
                   ScrollView::Color colour,          //colour of lines
                   BLOBNBOX_IT *blob_it,   //blobs
                   inT16 pitch,            //of block
                   inT16 blob_count,       //no of real blobs
                   STATS *projection,      //vertical
                   inT16 projection_left,  //edges //scale factor
                   inT16 projection_right,
                   float projection_scale);
void plot_fp_cells2(                        //draw words
                    ScrollView* win,             //window tro draw in
                    ScrollView::Color colour,          //colour of lines
                    TO_ROW *row,            //for location
                    FPSEGPT_LIST *seg_list  //segments to plot
                   );
void plot_row_cells(                       //draw words
                    ScrollView* win,            //window tro draw in
                    ScrollView::Color colour,         //colour of lines
                    TO_ROW *row,           //for location
                    float xshift,          //amount of shift
                    ICOORDELT_LIST *cells  //cells to draw
                   );
#endif
