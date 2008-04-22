/**********************************************************************
 * File:        scanedg.h  (Formerly scanedge.h)
 * Description: Raster scanning crack based edge extractor.
 * Author:					Ray Smith
 * Created:					Fri Mar 22 16:11:50 GMT 1991
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

#ifndef           SCANEDG_H
#define           SCANEDG_H

#include          "varable.h"
#include          "scrollview.h"
#include          "img.h"
#include          "pdblock.h"
#include          "crakedge.h"

DLLSYM void block_edges(                      //get edges in a block
                        IMAGE *t_image,       //threshold image
                        PDBLK *block,         //block in image
                        ICOORD page_tr        //corner of page
                       );
void make_margins(                         //get a line
                  PDBLK *block,            //block in image
                  BLOCK_LINE_IT *line_it,  //for old style
                  uinT8 *pixels,           //pixels to strip
                  uinT8 margin,            //white-out pixel
                  inT16 left,              //block edges
                  inT16 right,
                  inT16 y                  //line coord
                 );
void whiteout_block(                 //clean it
                    IMAGE *t_image,  //threshold image
                    PDBLK *block     //block in image
                   );
void line_edges (                //scan for edges
inT16 x,                         //coord of line start
inT16 y,                         //coord of line
inT16 xext,                      //width of line
uinT8 uppercolour,               //start of prev line
uinT8 * bwpos,                   //thresholded line
CRACKEDGE ** prevline            //edges in progress
);
CRACKEDGE *h_edge (              //horizontal edge
inT16 x,                         //xposition
inT16 y,                         //y position
inT8 sign,                       //sign of edge
CRACKEDGE * join                 //edge to join to
);
CRACKEDGE *v_edge (              //vertical edge
inT16 x,                         //xposition
inT16 y,                         //y position
inT8 sign,                       //sign of edge
CRACKEDGE * join                 //edge to join to
);
void join_edges(                   //join edge fragments
                CRACKEDGE *edge1,  //edges to join
                CRACKEDGE *edge2   //no specific order
               );
void free_crackedges(                  //really free them
                     CRACKEDGE *start  //start of loop
                    );
#endif
