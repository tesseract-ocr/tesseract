/* -*-C-*-
 ********************************************************************************
 *
 * File:        render.c  (Formerly render.c)
 * Description:  Convert the various data type into line lists
 * Author:       Mark Seaman, OCR Technology
 * Created:      Fri Jul 28 13:14:48 1989
 * Modified:     Mon Jul 15 10:23:37 1991 (Mark Seaman) marks@hpgrlt
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
#include "render.h"
#include "blobs.h"

#ifdef __UNIX__
#include <assert.h>
#endif
#include <math.h>

#include "vecfuncs.h"

// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif

/*----------------------------------------------------------------------
              V a r i a b l e s
----------------------------------------------------------------------*/
ScrollView *blob_window = NULL;

C_COL color_list[] = {
  Red, Cyan, Yellow, Blue, Green, White
};

BOOL_VAR(wordrec_display_all_blobs, 0, "Display Blobs");

BOOL_VAR(wordrec_display_all_words, 0, "Display Words");

BOOL_VAR(wordrec_blob_pause, 0, "Blob pause");

/*----------------------------------------------------------------------
              F u n c t i o n s
----------------------------------------------------------------------*/
#ifndef GRAPHICS_DISABLED
/**********************************************************************
 * display_blob
 *
 * Macro to display blob in a window.
 **********************************************************************/
void display_blob(TBLOB *blob, C_COL color) {
  /* Size of drawable */
  if (blob_window == NULL) {
    blob_window = c_create_window ("Blobs", 520, 10,
      500, 256, -1000.0, 1000.0, 0.0, 256.0);
  }
  else {
    c_clear_window(blob_window);
  }

  render_blob(blob_window, blob, color);
}

/**********************************************************************
 * render_blob
 *
 * Create a list of line segments that represent the expanded outline
 * that was supplied as input.
 **********************************************************************/
void render_blob(void *window, TBLOB *blob, C_COL color) {
  /* No outline */
  if (!blob)
    return;

  render_outline (window, blob->outlines, color);
}


/**********************************************************************
 * render_edgepts
 *
 * Create a list of line segments that represent the expanded outline
 * that was supplied as input.
 **********************************************************************/
void render_edgepts(void *window, EDGEPT *edgept, C_COL color) {
  float x = edgept->pos.x;
  float y = edgept->pos.y;
  EDGEPT *this_edge = edgept;

  if (!edgept)
    return;

  c_line_color_index(window, color);
  c_move(window, x, y);
  do {
    this_edge = this_edge->next;
    x = this_edge->pos.x;
    y = this_edge->pos.y;
    c_draw(window, x, y);
  }
  while (edgept != this_edge);
}


/**********************************************************************
 * render_outline
 *
 * Create a list of line segments that represent the expanded outline
 * that was supplied as input.
 **********************************************************************/
void render_outline(void *window,
                    TESSLINE *outline,
                    C_COL color) {
  /* No outline */
  if (!outline)
    return;
  /* Draw Compact outline */
  if (outline->loop)
    render_edgepts (window, outline->loop, color);
  /* Add on next outlines */
  render_outline (window, outline->next, color);
}

#endif  // GRAPHICS_DISABLED
