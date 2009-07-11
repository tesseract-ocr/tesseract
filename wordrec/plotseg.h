/* -*-C-*-
 ********************************************************************************
 *
 * File:        plotseg.h  (Formerly plotseg.h)
 * Description:
 * Author:       Mark Seaman, SW Productivity
 * Created:      Fri Oct 16 14:37:00 1987
 * Modified:     Fri Apr 26 10:03:32 1991 (Mark Seaman) marks@hpgrlt
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
#ifndef PLOTSEG_H
#define PLOTSEG_H

/*----------------------------------------------------------------------
              I n c l u d e s
----------------------------------------------------------------------*/
#include "states.h"
#include "render.h"

/*----------------------------------------------------------------------
              V a r i a b l e s
----------------------------------------------------------------------*/
extern ScrollView *segm_window;
extern INT_VAR_H(wordrec_display_segmentations, 0, "Display Segmentations");

/*----------------------------------------------------------------------
              F u n c t i o n s
----------------------------------------------------------------------*/
void display_segmentation(TBLOB *chunks, SEARCH_STATE segmentation);

void render_segmentation(ScrollView *window,
                         TBLOB *chunks,
                         SEARCH_STATE segmentation);
#endif
