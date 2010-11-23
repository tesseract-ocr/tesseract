/**********************************************************************
 * File:        scanedg.c  (Formerly scanedge.c)
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

#include "mfcpch.h"
#include          "edgloop.h"
#include          "scanedg.h"

#define WHITE_PIX     1          /*thresholded colours */
#define BLACK_PIX     0
                                 /*W->B->W */
#define FLIP_COLOUR(pix)  (1-(pix))

/**********************************************************************
 * block_edges
 *
 * Extract edges from a PDBLK.
 **********************************************************************/

void block_edges(IMAGE *t_image,       // thresholded image
                 PDBLK *block,         // block in image
                 C_OUTLINE_IT* outline_it) {
  uinT8 margin;                  // margin colour
  inT16 x;                       // line coords
  inT16 y;                       // current line
  ICOORD bleft;                  // bounding box
  ICOORD tright;
  ICOORD block_bleft;            // bounding box
  ICOORD block_tright;
  int xindex;                    // index to pixel
  BLOCK_LINE_IT line_it = block; // line iterator
  IMAGELINE bwline;              // thresholded line
                                 // lines in progress
  CRACKEDGE **ptrline = new CRACKEDGE*[t_image->get_xsize()+1];
  CRACKEDGE *free_cracks = NULL;

  block->bounding_box(bleft, tright);  // block box
  block_bleft = bleft;
  block_tright = tright;
  for (x = tright.x() - bleft.x(); x >= 0; x--)
    ptrline[x] = NULL;           //no lines in progress

  bwline.init(t_image->get_xsize());

  margin = WHITE_PIX;

  for (y = tright.y() - 1; y >= bleft.y() - 1; y--) {
    if (y >= block_bleft.y() && y < block_tright.y()) {
      t_image->get_line(bleft.x(), y, tright.x() - bleft.x(), &bwline, 0);
      make_margins(block, &line_it, bwline.pixels, margin, bleft.x(),
                   tright.x(), y);
    } else {
      x = tright.x() - bleft.x();
      for (xindex = 0; xindex < x; xindex++)
        bwline.pixels[xindex] = margin;
    }
    line_edges(bleft.x(), y, tright.x() - bleft.x(),
               margin, bwline.pixels, ptrline, &free_cracks, outline_it);
  }

  free_crackedges(free_cracks);  // really free them
  delete[] ptrline;
}


/**********************************************************************
 * make_margins
 *
 * Get an image line and set to margin non-text pixels.
 **********************************************************************/

void make_margins(                         //get a line
                  PDBLK *block,            //block in image
                  BLOCK_LINE_IT *line_it,  //for old style
                  uinT8 *pixels,           //pixels to strip
                  uinT8 margin,            //white-out pixel
                  inT16 left,              //block edges
                  inT16 right,
                  inT16 y                  //line coord
                 ) {
  PB_LINE_IT *lines;
  ICOORDELT_LIST *segments;      //bits of a line
  ICOORDELT_IT seg_it;
  inT32 start;                   //of segment
  inT16 xext;                    //of segment
  int xindex;                    //index to pixel

  if (block->poly_block () != NULL) {
    lines = new PB_LINE_IT (block->poly_block ());
    segments = lines->get_line (y);
    if (!segments->empty ()) {
      seg_it.set_to_list (segments);
      seg_it.mark_cycle_pt ();
      start = seg_it.data ()->x ();
      xext = seg_it.data ()->y ();
      for (xindex = left; xindex < right; xindex++) {
        if (xindex >= start && !seg_it.cycled_list ()) {
          xindex = start + xext - 1;
          seg_it.forward ();
          start = seg_it.data ()->x ();
          xext = seg_it.data ()->y ();
        }
        else
          pixels[xindex - left] = margin;
      }
    }
    else {
      for (xindex = left; xindex < right; xindex++)
        pixels[xindex - left] = margin;
    }
    delete segments;
    delete lines;
  }
  else {
    start = line_it->get_line (y, xext);
    for (xindex = left; xindex < start; xindex++)
      pixels[xindex - left] = margin;
    for (xindex = start + xext; xindex < right; xindex++)
      pixels[xindex - left] = margin;
  }
}


/**********************************************************************
 * whiteout_block
 *
 * Extract edges from a PDBLK.
 **********************************************************************/

void whiteout_block(                 //clean it
                    IMAGE *t_image,  //threshold image
                    PDBLK *block     //block in image
                   ) {
  inT16 x;                       //line coords
  inT16 y;                       //current line
  inT16 xext;                    //line width
  int xindex;                    //index to pixel
  uinT8 *dest;                   //destination pixel
  TBOX block_box;                 //bounding box
  BLOCK_LINE_IT line_it = block; //line iterator
  IMAGELINE bwline;              //thresholded line

  block_box = block->bounding_box ();
  for (y = block_box.bottom (); y < block_box.top (); y++) {
                                 //find line limits
    x = line_it.get_line (y, xext);
    t_image->get_line (x, y, xext, &bwline, 0);
    dest = bwline.pixels;        //destination pixel
    for (xindex = 0; xindex < xext; xindex++)
      *dest++ = 1;
    t_image->put_line (x, y, xext, &bwline, 0);
  }
}


/**********************************************************************
 * line_edges
 *
 * Scan a line for edges and update the edges in progress.
 * When edges close into loops, send them for approximation.
 **********************************************************************/

void line_edges(inT16 x,                         // coord of line start
                inT16 y,                         // coord of line
                inT16 xext,                      // width of line
                uinT8 uppercolour,               // start of prev line
                uinT8 * bwpos,                   // thresholded line
                CRACKEDGE ** prevline,           // edges in progress
                CRACKEDGE **free_cracks,
                C_OUTLINE_IT* outline_it) {
  CrackPos pos = {free_cracks, x, y };
  int xmax;                      // max x coord
  int colour;                    // of current pixel
  int prevcolour;                // of previous pixel
  CRACKEDGE *current;            // current h edge
  CRACKEDGE *newcurrent;         // new h edge

  xmax = x + xext;               // max allowable coord
  prevcolour = uppercolour;      // forced plain margin
  current = NULL;                // nothing yet

                                 // do each pixel
  for (; pos.x < xmax; pos.x++, prevline++) {
    colour = *bwpos++;           // current pixel
    if (*prevline != NULL) {
                                 // changed above
                                 // change colour
      uppercolour = FLIP_COLOUR(uppercolour);
      if (colour == prevcolour) {
        if (colour == uppercolour) {
                                 // finish a line
          join_edges(current, *prevline, free_cracks, outline_it);
          current = NULL;        // no edge now
        } else {
                                 // new horiz edge
          current = h_edge(uppercolour - colour, *prevline, &pos);
        }
        *prevline = NULL;        // no change this time
      } else {
        if (colour == uppercolour)
          *prevline = v_edge(colour - prevcolour, *prevline, &pos);
                                 // 8 vs 4 connection
        else if (colour == WHITE_PIX) {
          join_edges(current, *prevline, free_cracks, outline_it);
          current = h_edge(uppercolour - colour, NULL, &pos);
          *prevline = v_edge(colour - prevcolour, current, &pos);
        } else {
          newcurrent = h_edge(uppercolour - colour, *prevline, &pos);
          *prevline = v_edge(colour - prevcolour, current, &pos);
          current = newcurrent;  // right going h edge
        }
        prevcolour = colour;     // remember new colour
      }
    } else {
      if (colour != prevcolour) {
        *prevline = current = v_edge(colour - prevcolour, current, &pos);
        prevcolour = colour;
      }
      if (colour != uppercolour)
        current = h_edge(uppercolour - colour, current, &pos);
      else
        current = NULL;          // no edge now
    }
  }
  if (current != NULL) {
                                 // out of block
    if (*prevline != NULL) {     // got one to join to?
      join_edges(current, *prevline, free_cracks, outline_it);
      *prevline = NULL;          // tidy now
    } else {
                                 // fake vertical
      *prevline = v_edge(FLIP_COLOUR(prevcolour)-prevcolour, current, &pos);
    }
  } else if (*prevline != NULL) {
                                 //continue fake
    *prevline = v_edge(FLIP_COLOUR(prevcolour)-prevcolour, *prevline, &pos);
  }
}


/**********************************************************************
 * h_edge
 *
 * Create a new horizontal CRACKEDGE and join it to the given edge.
 **********************************************************************/

CRACKEDGE *h_edge(int sign,                       // sign of edge
                  CRACKEDGE* join,                // edge to join to
                  CrackPos* pos) {
  CRACKEDGE *newpt;              // return value

  if (*pos->free_cracks != NULL) {
    newpt = *pos->free_cracks;
    *pos->free_cracks = newpt->next;  // get one fast
  } else {
    newpt = new CRACKEDGE;
  }
  newpt->pos.set_y(pos->y + 1);       // coords of pt
  newpt->stepy = 0;              // edge is horizontal

  if (sign > 0) {
    newpt->pos.set_x(pos->x + 1);     // start location
    newpt->stepx = -1;
    newpt->stepdir = 0;
  } else {
    newpt->pos.set_x(pos->x);        // start location
    newpt->stepx = 1;
    newpt->stepdir = 2;
  }

  if (join == NULL) {
    newpt->next = newpt;         // ptrs to other ends
    newpt->prev = newpt;
  } else {
    if (newpt->pos.x() + newpt->stepx == join->pos.x()
    && newpt->pos.y() == join->pos.y()) {
      newpt->prev = join->prev;  // update other ends
      newpt->prev->next = newpt;
      newpt->next = join;        // join up
      join->prev = newpt;
    } else {
      newpt->next = join->next;  // update other ends
      newpt->next->prev = newpt;
      newpt->prev = join;        // join up
      join->next = newpt;
    }
  }
  return newpt;
}


/**********************************************************************
 * v_edge
 *
 * Create a new vertical CRACKEDGE and join it to the given edge.
 **********************************************************************/

CRACKEDGE *v_edge(int sign,                       // sign of edge
                  CRACKEDGE* join,
                  CrackPos* pos) {
  CRACKEDGE *newpt;              // return value

  if (*pos->free_cracks != NULL) {
    newpt = *pos->free_cracks;
    *pos->free_cracks = newpt->next;  // get one fast
  } else {
    newpt = new CRACKEDGE;
  }
  newpt->pos.set_x(pos->x);           // coords of pt
  newpt->stepx = 0;              // edge is vertical

  if (sign > 0) {
    newpt->pos.set_y(pos->y);         // start location
    newpt->stepy = 1;
    newpt->stepdir = 3;
  } else {
    newpt->pos.set_y(pos->y + 1);     // start location
    newpt->stepy = -1;
    newpt->stepdir = 1;
  }

  if (join == NULL) {
    newpt->next = newpt;         //ptrs to other ends
    newpt->prev = newpt;
  } else {
    if (newpt->pos.x() == join->pos.x()
    && newpt->pos.y() + newpt->stepy == join->pos.y()) {
      newpt->prev = join->prev;  // update other ends
      newpt->prev->next = newpt;
      newpt->next = join;        // join up
      join->prev = newpt;
    } else {
      newpt->next = join->next;  // update other ends
      newpt->next->prev = newpt;
      newpt->prev = join;        // join up
      join->next = newpt;
    }
  }
  return newpt;
}


/**********************************************************************
 * join_edges
 *
 * Join 2 edges together. Send the outline for approximation when a
 * closed loop is formed.
 **********************************************************************/

void join_edges(CRACKEDGE *edge1,  // edges to join
                CRACKEDGE *edge2,   // no specific order
                CRACKEDGE **free_cracks,
                C_OUTLINE_IT* outline_it) {
  if (edge1->pos.x() + edge1->stepx != edge2->pos.x()
  || edge1->pos.y() + edge1->stepy != edge2->pos.y()) {
    CRACKEDGE *tempedge = edge1;
    edge1 = edge2;               // swap araound
    edge2 = tempedge;
  }

  if (edge1->next == edge2) {
                                 // already closed
    complete_edge(edge1, outline_it);
                                 // attach freelist to end
    edge1->prev->next = *free_cracks;
    *free_cracks = edge1;         // and free list
  } else {
                                 // update opposite ends
    edge2->prev->next = edge1->next;
    edge1->next->prev = edge2->prev;
    edge1->next = edge2;         // make joins
    edge2->prev = edge1;
  }
}


/**********************************************************************
 * free_crackedges
 *
 * Really free the CRACKEDGEs by giving them back to delete.
 **********************************************************************/

void free_crackedges(CRACKEDGE *start) {
  CRACKEDGE *current;            // current edge to free
  CRACKEDGE *next;               // next one to free

  for (current = start; current != NULL; current = next) {
    next = current->next;
    delete current;              // delete them all
  }
}
