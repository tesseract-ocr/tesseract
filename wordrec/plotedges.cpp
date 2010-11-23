/* -*-C-*-
 ********************************************************************************
 *
 * File:        plotedges.c  (Formerly plotedges.c)
 * Description:  Graphics routines for "Edges" and "Outlines" windows
 * Author:       Mark Seaman, OCR Technology
 * Created:      Fri Jul 28 13:14:48 1989
 * Modified:     Tue Jul  9 17:22:22 1991 (Mark Seaman) marks@hpgrlt
 * Language:     C
 * Package:      N/A
 * Status:       Experimental (Do Not Distribute)
 *
 * (c) Copyright 1989, Hewlett-Packard Company.
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
 *********************************************************************************/
#ifdef __UNIX__
#include <assert.h>
#endif

#include "plotedges.h"
#include "render.h"
#include "split.h"

// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif

#ifndef GRAPHICS_DISABLED

/*----------------------------------------------------------------------
              V a r i a b l e s
----------------------------------------------------------------------*/
ScrollView *edge_window = NULL;

/*----------------------------------------------------------------------
              F u n c t i o n s
----------------------------------------------------------------------*/
/**********************************************************************
 * display_edgepts
 *
 * Macro to display edge points in a window.
 **********************************************************************/
void display_edgepts(LIST outlines) {
  void *window;
  /* Set up window */
  if (edge_window == NULL) {
    edge_window = c_create_window ("Edges", 750, 150,
      400, 128, -400.0, 400.0, 0.0, 256.0);
  }
  else {
    c_clear_window(edge_window);
  }
  /* Render the outlines */
  window = edge_window;
  /* Reclaim old memory */
  iterate(outlines) {
    render_edgepts (window, (EDGEPT *) first_node (outlines), White);
  }
}


/**********************************************************************
 * draw_blob_edges
 *
 * Display the edges of this blob in the edges window.
 **********************************************************************/
void draw_blob_edges(TBLOB *blob) {
  TESSLINE *ol;
  LIST edge_list = NIL_LIST;

  if (wordrec_display_splits) {
    for (ol = blob->outlines; ol != NULL; ol = ol->next)
      push_on (edge_list, ol->loop);
    display_edgepts(edge_list);
    destroy(edge_list);
  }
}


/**********************************************************************
 * mark_outline
 *
 * Make a mark on the edges window at a particular location.
 **********************************************************************/
void mark_outline(EDGEPT *edgept) {  /* Start of point list */
  void *window = edge_window;
  float x = edgept->pos.x;
  float y = edgept->pos.y;

  c_line_color_index(window, Red);
  c_move(window, x, y);

  x -= 4;
  y -= 12;
  c_draw(window, x, y);

  x -= 2;
  y += 4;
  c_draw(window, x, y);

  x -= 4;
  y += 2;
  c_draw(window, x, y);

  x += 10;
  y += 6;
  c_draw(window, x, y);

  c_make_current(window);
}


/**********************************************************************
 * mark_split
 *
 * Set up the marks list to be displayed in subsequent updates and draw
 * the marks in the current window.  The marks are stored in the second
 * sublist. The first sublist is left unmodified.
 **********************************************************************/
void mark_split(SPLIT *split) {
  void *window = edge_window;

  c_line_color_index(window, Green);
  c_move (window, (float) split->point1->pos.x, (float) split->point1->pos.y);
  c_draw (window, (float) split->point2->pos.x, (float) split->point2->pos.y);
  c_make_current(window);
}

#endif  // GRAPHICS_DISABLED
