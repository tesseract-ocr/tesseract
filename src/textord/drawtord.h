/**********************************************************************
 * File:        drawtord.h  (Formerly drawto.h)
 * Description: Draw things to do with textord.
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

#ifndef DRAWTORD_H
#define DRAWTORD_H

#include "blobbox.h"
#include "params.h"
#include "pitsync1.h"
#include "scrollview.h"

namespace tesseract {

#define NO_SMD "none"

extern BOOL_VAR_H(textord_show_fixed_cuts);
extern ScrollView *to_win;
extern FILE *to_debug;
// Creates a static display window for textord, and returns a pointer to it.
ScrollView *create_to_win(ICOORD page_tr);
void close_to_win();              // Destroy the textord window.
void create_todebug_win();        // make gradients win
void plot_box_list(               // make gradients win
    ScrollView *win,              // window to draw in
    BLOBNBOX_LIST *list,          // blob list
    ScrollView::Color body_colour // colour to draw
);
void plot_to_row(             // draw a row
    TO_ROW *row,              // row to draw
    ScrollView::Color colour, // colour to draw in
    FCOORD rotation           // rotation for line
);
void plot_parallel_row(       // draw a row
    TO_ROW *row,              // row to draw
    float gradient,           // gradients of lines
    int32_t left,             // edge of block
    ScrollView::Color colour, // colour to draw in
    FCOORD rotation           // rotation for line
);
void draw_occupation(                    // draw projection
    int32_t xleft,                       // edge of block
    int32_t ybottom,                     // bottom of block
    int32_t min_y,                       // coordinate limits
    int32_t max_y, int32_t occupation[], // projection counts
    int32_t thresholds[]                 // for drop out
);
void draw_meanlines(          // draw a block
    TO_BLOCK *block,          // block to draw
    float gradient,           // gradients of lines
    int32_t left,             // edge of block
    ScrollView::Color colour, // colour to draw in
    FCOORD rotation           // rotation for line
);
void plot_word_decisions( // draw words
    ScrollView *win,      // window tro draw in
    int16_t pitch,        // of block
    TO_ROW *row           // row to draw
);
void plot_fp_cells(           // draw words
    ScrollView *win,          // window tro draw in
    ScrollView::Color colour, // colour of lines
    BLOBNBOX_IT *blob_it,     // blobs
    int16_t pitch,            // of block
    int16_t blob_count,       // no of real blobs
    STATS *projection,        // vertical
    int16_t projection_left,  // edges //scale factor
    int16_t projection_right, float projection_scale);
void plot_fp_cells2(          // draw words
    ScrollView *win,          // window tro draw in
    ScrollView::Color colour, // colour of lines
    TO_ROW *row,              // for location
    FPSEGPT_LIST *seg_list    // segments to plot
);
void plot_row_cells(          // draw words
    ScrollView *win,          // window tro draw in
    ScrollView::Color colour, // colour of lines
    TO_ROW *row,              // for location
    float xshift,             // amount of shift
    ICOORDELT_LIST *cells     // cells to draw
);

} // namespace tesseract

#endif
