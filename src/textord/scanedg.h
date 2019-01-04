/**********************************************************************
 * File:        scanedg.h  (Formerly scanedge.h)
 * Description: Raster scanning crack based edge extractor.
 * Author:      Ray Smith
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

#include          "params.h"
#include          "scrollview.h"
#include          "pdblock.h"
#include          "crakedge.h"

class C_OUTLINE_IT;

struct CrackPos {
  CRACKEDGE** free_cracks;   // Freelist for fast allocation.
  int x;                     // Position of new edge.
  int y;
};

struct Pix;

void block_edges(Pix *t_image,         // thresholded image
                 PDBLK *block,         // block in image
                 C_OUTLINE_IT* outline_it);
void make_margins(PDBLK *block,            // block in image
                  BLOCK_LINE_IT *line_it,  // for old style
                  uint8_t *pixels,           // pixels to strip
                  uint8_t margin,            // white-out pixel
                  int16_t left,              // block edges
                  int16_t right,
                  int16_t y);                // line coord                 );
void line_edges(int16_t x,                     // coord of line start
                int16_t y,                     // coord of line
                int16_t xext,                  // width of line
                uint8_t uppercolour,           // start of prev line
                uint8_t * bwpos,               // thresholded line
                CRACKEDGE ** prevline,       // edges in progress
                CRACKEDGE **free_cracks,
                C_OUTLINE_IT* outline_it);
CRACKEDGE *h_edge(int sign,                  // sign of edge
                  CRACKEDGE * join,          // edge to join to
                  CrackPos* pos);
CRACKEDGE *v_edge(int sign,                  // sign of edge
                  CRACKEDGE * join,          // edge to join to
                  CrackPos* pos);
void join_edges(CRACKEDGE *edge1,            // edges to join
                CRACKEDGE *edge2,            // no specific order
                CRACKEDGE **free_cracks,
                C_OUTLINE_IT* outline_it);
void free_crackedges(CRACKEDGE *start);

#endif
