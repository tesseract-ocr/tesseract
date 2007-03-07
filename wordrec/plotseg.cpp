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
/*----------------------------------------------------------------------
              I n c l u d e s
----------------------------------------------------------------------*/
#include "plotseg.h"
#include "callcpp.h"
#include "tessclas.h"
#include "blobs.h"
#include "debug.h"
#include "const.h"
#include <math.h>

/*----------------------------------------------------------------------
              V a r i a b l e s
----------------------------------------------------------------------*/
void *segm_window = NULL;

make_int_var (display_segmentations, 0, make_display_seg,
9, 2, toggle_segmentations, "Display Segmentations");

/*----------------------------------------------------------------------
              F u n c t i o n s
----------------------------------------------------------------------*/
/**********************************************************************
 * display_segmentation
 *
 * Display all the words on the page into a window.
 **********************************************************************/
void display_segmentation(TBLOB *chunks, SEARCH_STATE segmentation) { 
  void *window;

  /* Destroy old data */
  /* If no window create it */
  if (segm_window == NULL) {
    segm_window = c_create_window ("Segmentation", 5, 10,
      500, 128, -1000.0, 1000.0, 0.0, 256.0);
  }
  else {
    c_clear_window(segm_window); 
  }

  window = segm_window;

  render_segmentation(window, chunks, segmentation); 
  /* Put data in the window */
  c_make_current(window); 
}


/**********************************************************************
 * init_plotseg
 *
 * Intialize the plotseg control variables.
 **********************************************************************/
void init_plotseg() { 
  make_display_seg(); 
}


/**********************************************************************
 * render_segmentation
 *
 * Create a list of line segments that represent the list of chunks
 * using the correct segmentation that was supplied as input.
 **********************************************************************/
void render_segmentation(void *window,
                         TBLOB *chunks,
                         SEARCH_STATE segmentation) {
  TPOINT origin;
  TBLOB *blob;
  C_COL color = Black;
  int char_num = -1;
  int chunks_left = 0;

  blobs_origin(chunks, &origin); 

  iterate_blobs(blob, chunks) { 

    if (chunks_left-- == 0) {
      color = color_list[++char_num % NUM_COLORS];

      if (char_num < segmentation[0])
        chunks_left = segmentation[char_num + 1];
      else
        chunks_left = MAXINT;
    }
    render_outline (window, blob->outlines, origin, color);
  }
}
