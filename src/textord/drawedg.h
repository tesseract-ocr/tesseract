/**********************************************************************
 * File:        drawedg.h  (Formerly drawedge.h)
 * Description: Collection of functions to draw things to do with edge
 *detection.
 * Author:          Ray Smith
 * Created:         Thu Jun 06 13:29:20 BST 1991
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

#ifndef           DRAWEDG_H
#define           DRAWEDG_H
#ifndef GRAPHICS_DISABLED

#include          "scrollview.h"
#include          "crakedge.h"

ScrollView* create_edges_window(                //make window
                           ICOORD page_tr  //size of image
                          );
void draw_raw_edge(                   //draw the cracks
                   ScrollView* fd,         //window to draw in
                   CRACKEDGE *start,  //start of loop
                   ScrollView::Color colour      //colour to draw in
                  );
#endif
#endif
