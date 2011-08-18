/**********************************************************************
 * File:        drawtord.cpp  (Formerly drawto.c)
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

#include "mfcpch.h"
#include          "pithsync.h"
#include          "topitch.h"
#include          "drawtord.h"

// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif

#define TO_WIN_XPOS     0       //default window pos
#define TO_WIN_YPOS     0
#define TO_WIN_NAME     "Textord"
                                 //title of window

#define EXTERN

EXTERN BOOL_VAR (textord_show_fixed_cuts, FALSE,
"Draw fixed pitch cell boundaries");

EXTERN ScrollView* to_win = NULL;

/**********************************************************************
 * create_to_win
 *
 * Create the to window used to show the fit.
 **********************************************************************/
#ifndef GRAPHICS_DISABLED

void create_to_win(ICOORD page_tr) {
  to_win = new ScrollView(TO_WIN_NAME, TO_WIN_XPOS, TO_WIN_YPOS,
                          page_tr.x() + 1, page_tr.y() + 1,
                          page_tr.x(), page_tr.y(), true);
}


void close_to_win() {
  // to_win is leaked, but this enables the user to view the contents.
  if (to_win != NULL) {
    to_win->Update();
  }
}


/**********************************************************************
 * plot_box_list
 *
 * Draw a list of blobs.
 **********************************************************************/

void plot_box_list(                      //make gradients win
                   ScrollView* win,           //window to draw in
                   BLOBNBOX_LIST *list,  //blob list
                   ScrollView::Color body_colour    //colour to draw
                  ) {
  BLOBNBOX_IT it = list;         //iterator

  win->Pen(body_colour);
  win->Brush(ScrollView::NONE);
  for (it.mark_cycle_pt (); !it.cycled_list (); it.forward ()) {
    it.data ()->bounding_box ().plot (win);
  }
}


/**********************************************************************
 * plot_to_row
 *
 * Draw the blobs of a row in a given colour and draw the line fit.
 **********************************************************************/

void plot_to_row(                 //draw a row
                 TO_ROW *row,     //row to draw
                 ScrollView::Color colour,   //colour to draw in
                 FCOORD rotation  //rotation for line
                ) {
  FCOORD plot_pt;                //point to plot
                                 //blobs
  BLOBNBOX_IT it = row->blob_list ();
  float left, right;             //end of row

  if (it.empty ()) {
    tprintf ("No blobs in row at %g\n", row->parallel_c ());
    return;
  }
  left = it.data ()->bounding_box ().left ();
  it.move_to_last ();
  right = it.data ()->bounding_box ().right ();
  plot_blob_list (to_win, row->blob_list (), colour, ScrollView::BROWN);
  to_win->Pen(colour);
  plot_pt = FCOORD (left, row->line_m () * left + row->line_c ());
  plot_pt.rotate (rotation);
  to_win->SetCursor(plot_pt.x (), plot_pt.y ());
  plot_pt = FCOORD (right, row->line_m () * right + row->line_c ());
  plot_pt.rotate (rotation);
  to_win->DrawTo(plot_pt.x (), plot_pt.y ());
}


/**********************************************************************
 * plot_parallel_row
 *
 * Draw the blobs of a row in a given colour and draw the line fit.
 **********************************************************************/

void plot_parallel_row(                 //draw a row
                       TO_ROW *row,     //row to draw
                       float gradient,  //gradients of lines
                       inT32 left,      //edge of block
                       ScrollView::Color colour,   //colour to draw in
                       FCOORD rotation  //rotation for line
                      ) {
  FCOORD plot_pt;                //point to plot
                                 //blobs
  BLOBNBOX_IT it = row->blob_list ();
  float fleft = (float) left;    //floating version
  float right;                   //end of row

  //      left=it.data()->bounding_box().left();
  it.move_to_last ();
  right = it.data ()->bounding_box ().right ();
  plot_blob_list (to_win, row->blob_list (), colour, ScrollView::BROWN);
  to_win->Pen(colour);
  plot_pt = FCOORD (fleft, gradient * left + row->max_y ());
  plot_pt.rotate (rotation);
  to_win->SetCursor(plot_pt.x (), plot_pt.y ());
  plot_pt = FCOORD (fleft, gradient * left + row->min_y ());
  plot_pt.rotate (rotation);
  to_win->DrawTo(plot_pt.x (), plot_pt.y ());
  plot_pt = FCOORD (fleft, gradient * left + row->parallel_c ());
  plot_pt.rotate (rotation);
  to_win->SetCursor(plot_pt.x (), plot_pt.y ());
  plot_pt = FCOORD (right, gradient * right + row->parallel_c ());
  plot_pt.rotate (rotation);
  to_win->DrawTo(plot_pt.x (), plot_pt.y ());
}


/**********************************************************************
 * draw_occupation
 *
 * Draw the row occupation with points above the threshold in white
 * and points below the threshold in black.
 **********************************************************************/

void
draw_occupation (                //draw projection
inT32 xleft,                     //edge of block
inT32 ybottom,                   //bottom of block
inT32 min_y,                     //coordinate limits
inT32 max_y, inT32 occupation[], //projection counts
inT32 thresholds[]               //for drop out
) {
  inT32 line_index;              //pixel coord
  ScrollView::Color colour;                 //of histogram
  float fleft = (float) xleft;   //float version

  colour = ScrollView::WHITE;
  to_win->Pen(colour);
  to_win->SetCursor(fleft, (float) ybottom);
  for (line_index = min_y; line_index <= max_y; line_index++) {
    if (occupation[line_index - min_y] < thresholds[line_index - min_y]) {
      if (colour != ScrollView::BLUE) {
        colour = ScrollView::BLUE;
	to_win->Pen(colour);
      }
    }
    else {
      if (colour != ScrollView::WHITE) {
        colour = ScrollView::WHITE;
	to_win->Pen(colour);
      }
    }
  to_win->DrawTo(fleft + occupation[line_index - min_y] / 10.0,      (float) line_index);
  }
  colour=ScrollView::STEEL_BLUE;
  to_win->Pen(colour);
  to_win->SetCursor(fleft, (float) ybottom);
  for (line_index = min_y; line_index <= max_y; line_index++) {
     to_win->DrawTo(fleft + thresholds[line_index - min_y] / 10.0,      (float) line_index);
  }
}


/**********************************************************************
 * draw_meanlines
 *
 * Draw the meanlines of the given block in the given colour.
 **********************************************************************/

void draw_meanlines(                  //draw a block
                    TO_BLOCK *block,  //block to draw
                    float gradient,   //gradients of lines
                    inT32 left,       //edge of block
                    ScrollView::Color colour,    //colour to draw in
                    FCOORD rotation   //rotation for line
                   ) {
  FCOORD plot_pt;                //point to plot
                                 //rows
  TO_ROW_IT row_it = block->get_rows ();
  TO_ROW *row;                   //current row
  BLOBNBOX_IT blob_it;           //blobs
  float right;                   //end of row
  to_win->Pen(colour);
  for (row_it.mark_cycle_pt (); !row_it.cycled_list (); row_it.forward ()) {
    row = row_it.data ();
    blob_it.set_to_list (row->blob_list ());
    blob_it.move_to_last ();
    right = blob_it.data ()->bounding_box ().right ();
    plot_pt =
      FCOORD ((float) left,
      gradient * left + row->parallel_c () + row->xheight);
    plot_pt.rotate (rotation);
  to_win->SetCursor(plot_pt.x (), plot_pt.y ());
    plot_pt =
      FCOORD ((float) right,
      gradient * right + row->parallel_c () + row->xheight);
    plot_pt.rotate (rotation);
    to_win->DrawTo (plot_pt.x (), plot_pt.y ());
  }
}


/**********************************************************************
 * plot_word_decisions
 *
 * Plot a row with words in different colours and fuzzy spaces
 * highlighted.
 **********************************************************************/

void plot_word_decisions(              //draw words
                         ScrollView* win,   //window tro draw in
                         inT16 pitch,  //of block
                         TO_ROW *row   //row to draw
                        ) {
  ScrollView::Color colour = ScrollView::MAGENTA;       //current colour
  ScrollView::Color rect_colour;            //fuzzy colour
  inT32 prev_x;                  //end of prev blob
  inT16 blob_count;              //blobs in word
  BLOBNBOX *blob;                //current blob
  TBOX blob_box;                  //bounding box
                                 //iterator
  BLOBNBOX_IT blob_it = row->blob_list ();
  BLOBNBOX_IT start_it = blob_it;//word start

  rect_colour = ScrollView::BLACK;
  prev_x = -MAX_INT16;
  blob_count = 0;
  for (blob_it.mark_cycle_pt (); !blob_it.cycled_list (); blob_it.forward ()) {
    blob = blob_it.data ();
    blob_box = blob->bounding_box ();
    if (!blob->joined_to_prev ()
    && blob_box.left () - prev_x > row->max_nonspace) {
      if ((blob_box.left () - prev_x >= row->min_space
        || blob_box.left () - prev_x > row->space_threshold)
      && blob_count > 0) {
        if (pitch > 0 && textord_show_fixed_cuts)
          plot_fp_cells (win, colour, &start_it, pitch, blob_count,
            &row->projection, row->projection_left,
            row->projection_right,
            row->xheight * textord_projection_scale);
        blob_count = 0;
        start_it = blob_it;
      }
      if (colour == ScrollView::MAGENTA)
        colour = ScrollView::RED;
      else
        colour = (ScrollView::Color) (colour + 1);
      if (blob_box.left () - prev_x < row->min_space) {
        if (blob_box.left () - prev_x > row->space_threshold)
          rect_colour = ScrollView::GOLDENROD;
        else
          rect_colour = ScrollView::CORAL;
        //fill_color_index(win, rect_colour);
        win->Brush(rect_colour);
        win->Rectangle (prev_x, blob_box.bottom (),
          blob_box.left (), blob_box.top ());
      }
    }
    if (!blob->joined_to_prev())
      prev_x = blob_box.right();
    if (blob->cblob () != NULL)
      blob->cblob ()->plot (win, colour, colour);
    if (!blob->joined_to_prev() && blob->cblob() != NULL)
      blob_count++;
  }
  if (pitch > 0 && textord_show_fixed_cuts && blob_count > 0)
    plot_fp_cells (win, colour, &start_it, pitch, blob_count,
      &row->projection, row->projection_left,
      row->projection_right,
      row->xheight * textord_projection_scale);
}


/**********************************************************************
 * plot_fp_cells
 *
 * Make a list of fixed pitch cuts and draw them.
 **********************************************************************/

void plot_fp_cells(                        //draw words
                   ScrollView* win,             //window tro draw in
                   ScrollView::Color colour,          //colour of lines
                   BLOBNBOX_IT *blob_it,   //blobs
                   inT16 pitch,            //of block
                   inT16 blob_count,       //no of real blobs
                   STATS *projection,      //vertical
                   inT16 projection_left,  //edges //scale factor
                   inT16 projection_right,
                   float projection_scale) {
  inT16 occupation;              //occupied cells
  TBOX word_box;                  //bounding box
  FPSEGPT_LIST seg_list;         //list of cuts
  FPSEGPT_IT seg_it;
  FPSEGPT *segpt;                //current point

  if (pitsync_linear_version)
    check_pitch_sync2 (blob_it, blob_count, pitch, 2, projection,
      projection_left, projection_right,
      projection_scale, occupation, &seg_list, 0, 0);
  else
    check_pitch_sync (blob_it, blob_count, pitch, 2, projection, &seg_list);
  word_box = blob_it->data ()->bounding_box ();
  for (; blob_count > 0; blob_count--)
    word_box += box_next (blob_it);
  seg_it.set_to_list (&seg_list);
  for (seg_it.mark_cycle_pt (); !seg_it.cycled_list (); seg_it.forward ()) {
    segpt = seg_it.data ();
    if (segpt->faked) {
         colour = ScrollView::WHITE;
         win->Pen(colour);  }
    else {
      win->Pen(colour); }
    win->Line(segpt->position (), word_box.bottom (),segpt->position (), word_box.top ());
  }
}


/**********************************************************************
 * plot_fp_cells2
 *
 * Make a list of fixed pitch cuts and draw them.
 **********************************************************************/

void plot_fp_cells2(                        //draw words
                    ScrollView* win,             //window tro draw in
                    ScrollView::Color colour,          //colour of lines
                    TO_ROW *row,            //for location
                    FPSEGPT_LIST *seg_list  //segments to plot
                   ) {
  TBOX word_box;                  //bounding box
  FPSEGPT_IT seg_it = seg_list;
                                 //blobs in row
  BLOBNBOX_IT blob_it = row->blob_list ();
  FPSEGPT *segpt;                //current point

  word_box = blob_it.data ()->bounding_box ();
  for (blob_it.mark_cycle_pt (); !blob_it.cycled_list ();)
    word_box += box_next (&blob_it);
  for (seg_it.mark_cycle_pt (); !seg_it.cycled_list (); seg_it.forward ()) {
    segpt = seg_it.data ();
    if (segpt->faked) {
         colour = ScrollView::WHITE;
         win->Pen(colour); }
    else {
     win->Pen(colour); }
     win->Line(segpt->position (), word_box.bottom (),segpt->position (), word_box.top ());
  }
}


/**********************************************************************
 * plot_row_cells
 *
 * Make a list of fixed pitch cuts and draw them.
 **********************************************************************/

void plot_row_cells(                       //draw words
                    ScrollView* win,            //window tro draw in
                    ScrollView::Color colour,         //colour of lines
                    TO_ROW *row,           //for location
                    float xshift,          //amount of shift
                    ICOORDELT_LIST *cells  //cells to draw
                   ) {
  TBOX word_box;                  //bounding box
  ICOORDELT_IT cell_it = cells;
                                 //blobs in row
  BLOBNBOX_IT blob_it = row->blob_list ();
  ICOORDELT *cell;               //current cell

  word_box = blob_it.data ()->bounding_box ();
  for (blob_it.mark_cycle_pt (); !blob_it.cycled_list ();)
    word_box += box_next (&blob_it);
  win->Pen(colour);
  for (cell_it.mark_cycle_pt (); !cell_it.cycled_list (); cell_it.forward ()) {
    cell = cell_it.data ();
    win->Line(cell->x () + xshift, word_box.bottom (), cell->x () + xshift, word_box.top ());
  }
}

#endif  // GRAPHICS_DISABLED

