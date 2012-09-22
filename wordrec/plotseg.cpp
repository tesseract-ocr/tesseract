/* -*-C-*-
 ********************************************************************************
 *
 * File:        plotseg.c  (Formerly plotseg.c)
 * Description:
 * Author:       Mark Seaman, OCR Technology
 * Created:      Fri Oct 16 14:37:00 1987
 * Modified:     Fri Apr 26 10:03:05 1991 (Mark Seaman) marks@hpgrlt
 * Language:     C
 * Package:      N/A
 * Status:       Reusable Software Component
 *
 * (c) Copyright 1987, Hewlett-Packard Company.
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

// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif

#ifndef GRAPHICS_DISABLED

/*----------------------------------------------------------------------
              I n c l u d e s
----------------------------------------------------------------------*/
#include "plotseg.h"
#include "callcpp.h"
#include "scrollview.h"
#include "blobs.h"
#include "const.h"
#include <math.h>

/*----------------------------------------------------------------------
              V a r i a b l e s
----------------------------------------------------------------------*/
ScrollView *segm_window = NULL;

INT_VAR(wordrec_display_segmentations, 0, "Display Segmentations");

/*----------------------------------------------------------------------
              F u n c t i o n s
----------------------------------------------------------------------*/
/**********************************************************************
 * display_segmentation
 *
 * Display all the words on the page into a window.
 **********************************************************************/
void display_segmentation(TBLOB *chunks, SEARCH_STATE segmentation) {
  /* If no window create it */
  if (segm_window == NULL) {
    segm_window = c_create_window ("Segmentation", 5, 10,
      500, 256, -1000.0, 1000.0, 0.0, 256.0);
  }
  else {
    c_clear_window(segm_window);
  }

  render_segmentation(segm_window, chunks, segmentation);
  /* Put data in the window */
  c_make_current(segm_window);
}

/**********************************************************************
 * render_segmentation
 *
 * Create a list of line segments that represent the list of chunks
 * using the correct segmentation that was supplied as input.
 **********************************************************************/
void render_segmentation(ScrollView *window,
                         TBLOB *chunks,
                         SEARCH_STATE segmentation) {
  TBLOB *blob;
  C_COL color = Black;
  int char_num = -1;
  int chunks_left = 0;

  TBOX bbox;
  if (chunks) bbox = chunks->bounding_box();

  for (blob = chunks; blob != NULL; blob = blob->next) {
    bbox += blob->bounding_box();
    if (chunks_left-- == 0) {
      color = color_list[++char_num % NUM_COLORS];

      if (char_num < segmentation[0])
        chunks_left = segmentation[char_num + 1];
      else
        chunks_left = MAX_INT32;
    }
    render_outline(window, blob->outlines, color);
  }
  window->ZoomToRectangle(bbox.left(), bbox.top(),
                          bbox.right(), bbox.bottom());
}

#endif  // GRPAHICS_DISABLED
