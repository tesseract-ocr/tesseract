/* -*-C-*-
 ********************************************************************************
 *
 * File:        plotedges.h  (Formerly plotedges.h)
 * Description:  Convert the various data type into line lists
 * Author:       Mark Seaman, OCR Technology
 * Created:      Fri Jul 28 13:14:48 1989
 * Modified:     Mon May 13 09:34:51 1991 (Mark Seaman) marks@hpgrlt
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
#ifndef PLOTEDGES_H
#define PLOTEDGES_H

#include "callcpp.h"
#include "oldlist.h"
#include "tessclas.h"
#include "split.h"

/*----------------------------------------------------------------------
              V a r i a b l e s
----------------------------------------------------------------------*/
extern ScrollView *edge_window;        /* Window for edges */

/*----------------------------------------------------------------------
            Macros
----------------------------------------------------------------------*/
/**********************************************************************
 * update_edge_window
 *
 * Refresh the display of the edge window.
 **********************************************************************/
#define update_edge_window()             \
if (wordrec_display_splits) {                  \
	c_make_current (edge_window);      \
}                                      \


/**********************************************************************
 * edge_window_wait
 *
 * Wait for someone to click in the edges window.
 **********************************************************************/

#define edge_window_wait()  \
if (wordrec_display_splits) window_wait (edge_window)

/*----------------------------------------------------------------------
              F u n c t i o n s
---------------------------------------------------------------------*/
void display_edgepts(LIST outlines);

void draw_blob_edges(TBLOB *blob);

void mark_outline(EDGEPT *edgept);

void mark_split(SPLIT *split);
#endif
