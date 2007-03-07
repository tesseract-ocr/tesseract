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
#include          "debugwin.h"

#define TO_WIN_XPOS     -1       //default window pos
#define TO_WIN_YPOS     0
#define TO_WIN_NAME     "Textord"
                                 //title of window
#define DEBUG_WIN_NAME    "TODebug"
#define DEBUG_XPOS      0
#define DEBUG_YPOS      120
#define DEBUG_XSIZE     80
#define DEBUG_YSIZE     32
#define YMAX        3508
#define XMAX        2550

#define EXTERN

EXTERN BOOL_VAR (textord_show_fixed_cuts, FALSE,
"Draw fixed pitch cell boundaries");
EXTERN STRING_VAR (to_debugfile, DEBUG_WIN_NAME, "Name of debugfile");
EXTERN STRING_VAR (to_smdfile, NO_SMD, "Name of SMD file");

EXTERN WINDOW to_win = NO_WINDOW;
EXTERN FILE *to_debug = NULL;

/**********************************************************************
 * create_to_win
 *
 * Create the to window used to show the fit.
 **********************************************************************/

void create_to_win(                //make features win
                   ICOORD page_tr  //size of page
                  ) {
  if (strcmp (to_smdfile.string (), NO_SMD)) {
    to_win = create_window (to_smdfile.string (), SMDWINDOW,
      0, 0, page_tr.x () + 1, page_tr.y () + 1,
      0.0, page_tr.x (), 0.0, page_tr.y (),
      TRUE, FALSE, TRUE, TRUE);
  }
  else {
    to_win = create_window (TO_WIN_NAME, SCROLLINGWIN,
      TO_WIN_XPOS, TO_WIN_YPOS, 0, 0,
      0.0, page_tr.x (), 0.0, page_tr.y (),
      TRUE, FALSE, TRUE, TRUE);
  }
}


void close_to_win() {  //make features win
  if (to_win != NO_WINDOW && strcmp (to_smdfile.string (), NO_SMD)) {
    destroy_window(to_win); 
    overlap_picture_ops(TRUE); 
  }
}


/**********************************************************************
 * create_todebug_win
 *
 * Create the to window used to show the fit.
 **********************************************************************/

void create_todebug_win() {  //make gradients win
  if (strcmp (to_debugfile.string (), DEBUG_WIN_NAME) != 0)
    //              create_debug_window();
    //      else
    to_debug = fopen (to_debugfile.string (), "w");
}


/**********************************************************************
 * plot_blob_list
 *
 * Draw a list of blobs.
 **********************************************************************/

void plot_blob_list(                      //make gradients win
                    WINDOW win,           //window to draw in
                    BLOBNBOX_LIST *list,  //blob list
                    COLOUR body_colour,   //colour to draw
                    COLOUR child_colour   //colour of child
                   ) {
  BLOBNBOX_IT it = list;         //iterator

  for (it.mark_cycle_pt (); !it.cycled_list (); it.forward ()) {
    it.data ()->plot (win, body_colour, child_colour);
  }
}


/**********************************************************************
 * plot_box_list
 *
 * Draw a list of blobs.
 **********************************************************************/

void plot_box_list(                      //make gradients win
                   WINDOW win,           //window to draw in
                   BLOBNBOX_LIST *list,  //blob list
                   COLOUR body_colour    //colour to draw
                  ) {
  BLOBNBOX_IT it = list;         //iterator

  perimeter_color_index(win, body_colour); 
  interior_style(win, INT_HOLLOW, TRUE); 
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
                 COLOUR colour,   //colour to draw in
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
  plot_blob_list (to_win, row->blob_list (), colour, BROWN);
  line_color_index(to_win, colour); 
  plot_pt = FCOORD (left, row->line_m () * left + row->line_c ());
  plot_pt.rotate (rotation);
  move2d (to_win, plot_pt.x (), plot_pt.y ());
  plot_pt = FCOORD (right, row->line_m () * right + row->line_c ());
  plot_pt.rotate (rotation);
  draw2d (to_win, plot_pt.x (), plot_pt.y ());
}


/**********************************************************************
 * plot_parallel_row
 *
 * Draw the blobs of a row in a given colour and draw the line fit.
 **********************************************************************/

void plot_parallel_row(                 //draw a row
                       TO_ROW *row,     //row to draw
                       float gradient,  //gradients of lines
                       INT32 left,      //edge of block
                       COLOUR colour,   //colour to draw in
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
  plot_blob_list (to_win, row->blob_list (), colour, BROWN);
  line_color_index(to_win, colour); 
  plot_pt = FCOORD (fleft, gradient * left + row->max_y ());
  plot_pt.rotate (rotation);
  move2d (to_win, plot_pt.x (), plot_pt.y ());
  plot_pt = FCOORD (fleft, gradient * left + row->min_y ());
  plot_pt.rotate (rotation);
  draw2d (to_win, plot_pt.x (), plot_pt.y ());
  plot_pt = FCOORD (fleft, gradient * left + row->parallel_c ());
  plot_pt.rotate (rotation);
  move2d (to_win, plot_pt.x (), plot_pt.y ());
  plot_pt = FCOORD (right, gradient * right + row->parallel_c ());
  plot_pt.rotate (rotation);
  draw2d (to_win, plot_pt.x (), plot_pt.y ());
}


/**********************************************************************
 * draw_occupation
 *
 * Draw the row occupation with points above the threshold in white
 * and points below the threshold in black.
 **********************************************************************/

void
draw_occupation (                //draw projection
INT32 xleft,                     //edge of block
INT32 ybottom,                   //bottom of block
INT32 min_y,                     //coordinate limits
INT32 max_y, INT32 occupation[], //projection counts
INT32 thresholds[]               //for drop out
) {
  INT32 line_index;              //pixel coord
  COLOUR colour;                 //of histogram
  float fleft = (float) xleft;   //float version

  colour = WHITE;
  line_color_index(to_win, colour); 
  move2d (to_win, fleft, (float) ybottom);
  for (line_index = min_y; line_index <= max_y; line_index++) {
    if (occupation[line_index - min_y] < thresholds[line_index - min_y]) {
      if (colour != BLUE) {
        colour = BLUE;
        line_color_index(to_win, colour); 
      }
    }
    else {
      if (colour != WHITE) {
        colour = WHITE;
        line_color_index(to_win, colour); 
      }
    }
    draw2d (to_win, fleft + occupation[line_index - min_y] / 10.0,
      (float) line_index);
  }
  line_color_index(to_win, STEEL_BLUE); 
  move2d (to_win, fleft, (float) ybottom);
  for (line_index = min_y; line_index <= max_y; line_index++) {
    draw2d (to_win, fleft + thresholds[line_index - min_y] / 10.0,
      (float) line_index);
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
                    INT32 left,       //edge of block
                    COLOUR colour,    //colour to draw in
                    FCOORD rotation   //rotation for line
                   ) {
  FCOORD plot_pt;                //point to plot
                                 //rows
  TO_ROW_IT row_it = block->get_rows ();
  TO_ROW *row;                   //current row
  BLOBNBOX_IT blob_it;           //blobs
  float right;                   //end of row

  line_color_index(to_win, colour); 
  for (row_it.mark_cycle_pt (); !row_it.cycled_list (); row_it.forward ()) {
    row = row_it.data ();
    blob_it.set_to_list (row->blob_list ());
    blob_it.move_to_last ();
    right = blob_it.data ()->bounding_box ().right ();
    plot_pt =
      FCOORD ((float) left,
      gradient * left + row->parallel_c () + row->xheight);
    plot_pt.rotate (rotation);
    move2d (to_win, plot_pt.x (), plot_pt.y ());
    plot_pt =
      FCOORD ((float) right,
      gradient * right + row->parallel_c () + row->xheight);
    plot_pt.rotate (rotation);
    draw2d (to_win, plot_pt.x (), plot_pt.y ());
  }
}


/**********************************************************************
 * plot_word_decisions
 *
 * Plot a row with words in different colours and fuzzy spaces
 * highlighted.
 **********************************************************************/

void plot_word_decisions(              //draw words
                         WINDOW win,   //window tro draw in
                         INT16 pitch,  //of block
                         TO_ROW *row   //row to draw
                        ) {
  COLOUR colour = MAGENTA;       //current colour
  COLOUR rect_colour;            //fuzzy colour
  INT32 prev_x;                  //end of prev blob
  INT16 blob_count;              //blobs in word
  BLOBNBOX *blob;                //current blob
  BOX blob_box;                  //bounding box
                                 //iterator
  BLOBNBOX_IT blob_it = row->blob_list ();
  BLOBNBOX_IT start_it = blob_it;//word start

  interior_style(win, INT_SOLID, FALSE); 
  rect_colour = BLACK;
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
      if (colour == MAGENTA)
        colour = RED;
      else
        colour = (COLOUR) (colour + 1);
      if (blob_box.left () - prev_x < row->min_space) {
        if (blob_box.left () - prev_x > row->space_threshold)
          rect_colour = GOLDENROD;
        else
          rect_colour = CORAL;
        fill_color_index(win, rect_colour); 
        rectangle (win, (float) prev_x, blob_box.bottom (),
          blob_box.left (), blob_box.top ());
      }
    }
    if (!blob->joined_to_prev ())
      prev_x = blob_box.right ();
    if (blob->blob () != NULL)
                                 //draw it
      blob->blob ()->plot (win, colour, colour);
    if (blob->cblob () != NULL)
      blob->cblob ()->plot (win, colour, colour);
    if (!blob->joined_to_prev ()
      && (blob->blob () != NULL || blob->cblob () != NULL))
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
                   WINDOW win,             //window tro draw in
                   COLOUR colour,          //colour of lines
                   BLOBNBOX_IT *blob_it,   //blobs
                   INT16 pitch,            //of block
                   INT16 blob_count,       //no of real blobs
                   STATS *projection,      //vertical
                   INT16 projection_left,  //edges //scale factor
                   INT16 projection_right,
                   float projection_scale) {
  INT16 occupation;              //occupied cells
  BOX word_box;                  //bounding box
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
    if (segpt->faked)
      line_color_index(win, WHITE); 
    else
      line_color_index(win, colour); 
    move2d (win, segpt->position (), word_box.bottom ());
    draw2d (win, segpt->position (), word_box.top ());
  }
}


/**********************************************************************
 * plot_fp_cells2
 *
 * Make a list of fixed pitch cuts and draw them.
 **********************************************************************/

void plot_fp_cells2(                        //draw words
                    WINDOW win,             //window tro draw in
                    COLOUR colour,          //colour of lines
                    TO_ROW *row,            //for location
                    FPSEGPT_LIST *seg_list  //segments to plot
                   ) {
  BOX word_box;                  //bounding box
  FPSEGPT_IT seg_it = seg_list;
                                 //blobs in row
  BLOBNBOX_IT blob_it = row->blob_list ();
  FPSEGPT *segpt;                //current point

  word_box = blob_it.data ()->bounding_box ();
  for (blob_it.mark_cycle_pt (); !blob_it.cycled_list ();)
    word_box += box_next (&blob_it);
  for (seg_it.mark_cycle_pt (); !seg_it.cycled_list (); seg_it.forward ()) {
    segpt = seg_it.data ();
    if (segpt->faked)
      line_color_index(win, WHITE); 
    else
      line_color_index(win, colour); 
    move2d (win, segpt->position (), word_box.bottom ());
    draw2d (win, segpt->position (), word_box.top ());
  }
}


/**********************************************************************
 * plot_row_cells
 *
 * Make a list of fixed pitch cuts and draw them.
 **********************************************************************/

void plot_row_cells(                       //draw words
                    WINDOW win,            //window tro draw in
                    COLOUR colour,         //colour of lines
                    TO_ROW *row,           //for location
                    float xshift,          //amount of shift
                    ICOORDELT_LIST *cells  //cells to draw
                   ) {
  BOX word_box;                  //bounding box
  ICOORDELT_IT cell_it = cells;
                                 //blobs in row
  BLOBNBOX_IT blob_it = row->blob_list ();
  ICOORDELT *cell;               //current cell

  word_box = blob_it.data ()->bounding_box ();
  for (blob_it.mark_cycle_pt (); !blob_it.cycled_list ();)
    word_box += box_next (&blob_it);
  line_color_index(win, colour); 
  for (cell_it.mark_cycle_pt (); !cell_it.cycled_list (); cell_it.forward ()) {
    cell = cell_it.data ();
    move2d (win, cell->x () + xshift, word_box.bottom ());
    draw2d (win, cell->x () + xshift, word_box.top ());
  }
}
