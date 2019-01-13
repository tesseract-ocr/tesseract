/**********************************************************************
 * File:        drawedg.cpp  (Formerly drawedge.c)
 * Description: Collection of functions to draw things to do with edge
 *              detection.
 *  Author:     Ray Smith
 *  Created:    Thu Jun 06 13:29:20 BST 1991
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

#include "drawedg.h"

// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif

#ifndef GRAPHICS_DISABLED

/** title of window */
#define IMAGE_WIN_NAME    "Edges"
#define IMAGE_XPOS      250
/** default position */
#define IMAGE_YPOS      0

/**
 * @name create_edges_window
 *
 * Create the edges window.
 * @param page_tr size of image
 */

ScrollView* create_edges_window(ICOORD page_tr) {
  ScrollView* image_win;              //image window

                                 //create the window
  image_win = new ScrollView (IMAGE_WIN_NAME, IMAGE_XPOS, IMAGE_YPOS, 0, 0, page_tr.x (),  page_tr.y ());
  return image_win;              //window
}


/**
 * @name draw_raw_edge
 *
 * Draw the raw steps to the given window in the given colour.
 * @param fd window to draw in
 * @param start start of loop
 * @param colour colour to draw in
 */

void draw_raw_edge(ScrollView* fd,
                   CRACKEDGE *start,
                   ScrollView::Color colour) {
  CRACKEDGE *edgept;             //current point

  fd->Pen(colour);
  edgept = start;
  fd->SetCursor(edgept->pos.x (), edgept->pos.y ());
  do {
    do
    edgept = edgept->next;
                                 //merge straight lines
    while (edgept != start && edgept->prev->stepx == edgept->stepx && edgept->prev->stepy == edgept->stepy);

                                 //draw lines
  fd->DrawTo(edgept->pos.x (), edgept->pos.y ());
  }
  while (edgept != start);
}

#endif  // GRAPHICS_DISABLED
