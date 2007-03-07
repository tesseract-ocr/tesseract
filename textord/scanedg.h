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
#include          "grphics.h"
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
                  UINT8 *pixels,           //pixels to strip
                  UINT8 margin,            //white-out pixel
                  INT16 left,              //block edges
                  INT16 right,
                  INT16 y                  //line coord
                 );
void whiteout_block(                 //clean it
                    IMAGE *t_image,  //threshold image
                    PDBLK *block     //block in image
                   );
void line_edges (                //scan for edges
INT16 x,                         //coord of line start
INT16 y,                         //coord of line
INT16 xext,                      //width of line
UINT8 uppercolour,               //start of prev line
UINT8 * bwpos,                   //thresholded line
CRACKEDGE ** prevline            //edges in progress
);
CRACKEDGE *h_edge (              //horizontal edge
INT16 x,                         //xposition
INT16 y,                         //y position
INT8 sign,                       //sign of edge
CRACKEDGE * join                 //edge to join to
);
CRACKEDGE *v_edge (              //vertical edge
INT16 x,                         //xposition
INT16 y,                         //y position
INT8 sign,                       //sign of edge
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
