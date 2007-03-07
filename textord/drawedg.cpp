/**********************************************************************
 * File:        drawedg.c  (Formerly drawedge.c)
 * Description: Collection of functions to draw things to do with edge detection.
 * Author:					Ray Smith
 * Created:					Thu Jun 06 13:29:20 BST 1991
 *
 * (C) Copyright 1991, Hewlett-Packard Ltd.
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
#include          "drawedg.h"
#include          "evnts.h"

#define IMAGE_WIN_NAME    "Edges"//title of window
#define IMAGE_XPOS      250
#define IMAGE_YPOS      0        //default position
#define CTRLD         '\004'     //control D

#define EXTERN

/**********************************************************************
 * create_edges_window
 *
 * Create the edges window.
 **********************************************************************/

WINDOW create_edges_window(                //make window
                           ICOORD page_tr  //size of image
                          ) {
  WINDOW image_win;              //image window

                                 //create the window
  image_win = create_window (IMAGE_WIN_NAME, SCROLLINGWIN, IMAGE_XPOS, IMAGE_YPOS, 0, 0, (float) 0.0, page_tr.x (), (float) 0.0, page_tr.y (), TRUE, FALSE, FALSE, FALSE);
  #ifdef __MSW32__
  set_selection_handler(image_win, win_selection_handler); 
  #endif
  overlap_picture_ops(TRUE); 
  return image_win;              //window
}


/**********************************************************************
 * draw_raw_edge
 *
 * Draw the raw steps to the given window in the given colour.
 **********************************************************************/

void draw_raw_edge(                   //draw the cracks
                   WINDOW fd,         //window to draw in
                   CRACKEDGE *start,  //start of loop
                   COLOUR colour      //colour to draw in
                  ) {
  CRACKEDGE *edgept;             //current point

  line_color_index(fd, colour);  //in white
  edgept = start;
  move2d (fd, edgept->pos.x (), edgept->pos.y ());
  do {
    do
    edgept = edgept->next;
                                 //merge straight lines
    while (edgept != start && edgept->prev->stepx == edgept->stepx && edgept->prev->stepy == edgept->stepy);

                                 //draw lines
    draw2d (fd, edgept->pos.x (), edgept->pos.y ());
  }
  while (edgept != start);
}

