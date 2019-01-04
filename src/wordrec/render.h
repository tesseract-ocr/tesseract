/* -*-C-*-
 ********************************************************************************
 *
 * File:        render.h  (Formerly render.h)
 * Description:  Convert the various data type into line lists
 * Author:       Mark Seaman, OCR Technology
 * Created:      Fri Jul 28 13:14:48 1989
 * Modified:     Fri Apr 26 09:59:45 1991 (Mark Seaman) marks@hpgrlt
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
#ifndef RENDER_H
#define RENDER_H

#include "callcpp.h"  // for C_COL
#include "params.h"   // for BOOL_VAR_H, BoolParam

class ScrollView;

struct EDGEPT;
struct TBLOB;
struct TESSLINE;

/*----------------------------------------------------------------------
              V a r i a b l e s
----------------------------------------------------------------------*/
extern ScrollView *blob_window;        /* Window for blobs */
extern C_COL color_list[];       /* Colors for outlines */

extern BOOL_VAR_H(wordrec_display_all_blobs, 0, "Display Blobs");

extern BOOL_VAR_H(wordrec_display_all_words, 0, "Display Words");

extern BOOL_VAR_H(wordrec_blob_pause, 0, "Blob pause");

#define NUM_COLORS 6

/*----------------------------------------------------------------------
              F u n c t i o n s
----------------------------------------------------------------------*/
void display_blob(TBLOB *blob, C_COL color);

void render_blob(void *window, TBLOB *blob, C_COL color);

void render_edgepts(void *window, EDGEPT *edgept, C_COL color);

void render_outline(void *window,
                    TESSLINE *outline,
                    C_COL color);

#endif
