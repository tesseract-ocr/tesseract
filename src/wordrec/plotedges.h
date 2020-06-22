/******************************************************************************
 *
 * File:         plotedges.h
 * Description:  Convert the various data type into line lists
 * Author:       Mark Seaman, OCR Technology
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
 *****************************************************************************/

#ifndef PLOTEDGES_H
#define PLOTEDGES_H

#include "oldlist.h"  // for LIST

class ScrollView;

struct EDGEPT;
struct TBLOB;

/*----------------------------------------------------------------------
              V a r i a b l e s
----------------------------------------------------------------------*/
extern ScrollView *edge_window;        /* Window for edges */

/*----------------------------------------------------------------------
              F u n c t i o n s
---------------------------------------------------------------------*/
void display_edgepts(LIST outlines);

void draw_blob_edges(TBLOB *blob);

void mark_outline(EDGEPT *edgept);

#endif
