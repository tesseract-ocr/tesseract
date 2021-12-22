/**********************************************************************
 * File:        scanedg.cpp  (Formerly scanedge.c)
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

#include "scanedg.h"

#include "crakedge.h"
#include "edgloop.h"
#include "pdblock.h"

#include <allheaders.h>

#include <memory> // std::unique_ptr

namespace tesseract {

#define WHITE_PIX 1 /*thresholded colours */
#define BLACK_PIX 0
// Flips between WHITE_PIX and BLACK_PIX.
#define FLIP_COLOUR(pix) (1 - (pix))

struct CrackPos {
  CRACKEDGE **free_cracks; // Freelist for fast allocation.
  int x;                   // Position of new edge.
  int y;
};

static void free_crackedges(CRACKEDGE *start);

static void join_edges(CRACKEDGE *edge1, CRACKEDGE *edge2, CRACKEDGE **free_cracks,
                       C_OUTLINE_IT *outline_it);

static void line_edges(TDimension x, TDimension y, TDimension xext, uint8_t uppercolour, uint8_t *bwpos,
                       CRACKEDGE **prevline, CRACKEDGE **free_cracks, C_OUTLINE_IT *outline_it);

static void make_margins(PDBLK *block, BLOCK_LINE_IT *line_it, uint8_t *pixels, uint8_t margin,
                         TDimension left, TDimension right, TDimension y);

static CRACKEDGE *h_edge(int sign, CRACKEDGE *join, CrackPos *pos);
static CRACKEDGE *v_edge(int sign, CRACKEDGE *join, CrackPos *pos);

/**********************************************************************
 * block_edges
 *
 * Extract edges from a PDBLK.
 **********************************************************************/

void block_edges(Image t_pix,   // thresholded image
                 PDBLK *block, // block in image
                 C_OUTLINE_IT *outline_it) {
  ICOORD bleft; // bounding box
  ICOORD tright;
  BLOCK_LINE_IT line_it = block; // line iterator

  int width = pixGetWidth(t_pix);
  int height = pixGetHeight(t_pix);
  int wpl = pixGetWpl(t_pix);
  // lines in progress
  std::unique_ptr<CRACKEDGE *[]> ptrline(new CRACKEDGE *[width + 1]);
  CRACKEDGE *free_cracks = nullptr;

  block->bounding_box(bleft, tright); // block box
  ASSERT_HOST(tright.x() <= width);
  ASSERT_HOST(tright.y() <= height);
  int block_width = tright.x() - bleft.x();
  for (int x = block_width; x >= 0; x--) {
    ptrline[x] = nullptr; //  no lines in progress
  }

  std::unique_ptr<uint8_t[]> bwline(new uint8_t[width]);

  const uint8_t margin = WHITE_PIX;

  for (int y = tright.y() - 1; y >= bleft.y() - 1; y--) {
    if (y >= bleft.y() && y < tright.y()) {
      // Get the binary pixels from the image.
      l_uint32 *line = pixGetData(t_pix) + wpl * (height - 1 - y);
      for (int x = 0; x < block_width; ++x) {
        bwline[x] = GET_DATA_BIT(line, x + bleft.x()) ^ 1;
      }
      make_margins(block, &line_it, bwline.get(), margin, bleft.x(), tright.x(), y);
    } else {
      memset(bwline.get(), margin, block_width * sizeof(bwline[0]));
    }
    line_edges(bleft.x(), y, block_width, margin, bwline.get(), ptrline.get(), &free_cracks,
               outline_it);
  }

  free_crackedges(free_cracks); // really free them
}

/**********************************************************************
 * make_margins
 *
 * Get an image line and set to margin non-text pixels.
 **********************************************************************/

static void make_margins(   // get a line
    PDBLK *block,           // block in image
    BLOCK_LINE_IT *line_it, // for old style
    uint8_t *pixels,        // pixels to strip
    uint8_t margin,         // white-out pixel
    TDimension left,        // block edges
    TDimension right,
    TDimension y            // line coord
) {
  ICOORDELT_IT seg_it;

  if (block->poly_block() != nullptr) {
    std::unique_ptr<PB_LINE_IT> lines(new PB_LINE_IT(block->poly_block()));
    const std::unique_ptr</*non-const*/ ICOORDELT_LIST> segments(lines->get_line(y));
    if (!segments->empty()) {
      seg_it.set_to_list(segments.get());
      seg_it.mark_cycle_pt();
      auto start = seg_it.data()->x();
      auto xext = seg_it.data()->y();
      for (auto xindex = left; xindex < right; xindex++) {
        if (xindex >= start && !seg_it.cycled_list()) {
          xindex = start + xext - 1;
          seg_it.forward();
          start = seg_it.data()->x();
          xext = seg_it.data()->y();
        } else {
          pixels[xindex - left] = margin;
        }
      }
    } else {
      for (auto xindex = left; xindex < right; xindex++) {
        pixels[xindex - left] = margin;
      }
    }
  } else {
    TDimension xext;  // of segment
    auto start = line_it->get_line(y, xext);
    for (auto xindex = left; xindex < start; xindex++) {
      pixels[xindex - left] = margin;
    }
    for (auto xindex = start + xext; xindex < right; xindex++) {
      pixels[xindex - left] = margin;
    }
  }
}

/**********************************************************************
 * line_edges
 *
 * Scan a line for edges and update the edges in progress.
 * When edges close into loops, send them for approximation.
 **********************************************************************/

static void line_edges(TDimension x,         // coord of line start
                       TDimension y,         // coord of line
                       TDimension xext,      // width of line
                       uint8_t uppercolour,  // start of prev line
                       uint8_t *bwpos,       // thresholded line
                       CRACKEDGE **prevline, // edges in progress
                       CRACKEDGE **free_cracks, C_OUTLINE_IT *outline_it) {
  CrackPos pos = {free_cracks, x, y};
  int xmax;              // max x coord
  int prevcolour;        // of previous pixel
  CRACKEDGE *current;    // current h edge
  CRACKEDGE *newcurrent; // new h edge

  xmax = x + xext;          // max allowable coord
  prevcolour = uppercolour; // forced plain margin
  current = nullptr;        // nothing yet

  // do each pixel
  for (; pos.x < xmax; pos.x++, prevline++) {
    const int colour = *bwpos++; // current pixel
    if (*prevline != nullptr) {
      // changed above
      // change colour
      uppercolour = FLIP_COLOUR(uppercolour);
      if (colour == prevcolour) {
        if (colour == uppercolour) {
          // finish a line
          join_edges(current, *prevline, free_cracks, outline_it);
          current = nullptr; // no edge now
        } else {
          // new horiz edge
          current = h_edge(uppercolour - colour, *prevline, &pos);
        }
        *prevline = nullptr; // no change this time
      } else {
        if (colour == uppercolour) {
          *prevline = v_edge(colour - prevcolour, *prevline, &pos);
        // 8 vs 4 connection
        } else if (colour == WHITE_PIX) {
          join_edges(current, *prevline, free_cracks, outline_it);
          current = h_edge(uppercolour - colour, nullptr, &pos);
          *prevline = v_edge(colour - prevcolour, current, &pos);
        } else {
          newcurrent = h_edge(uppercolour - colour, *prevline, &pos);
          *prevline = v_edge(colour - prevcolour, current, &pos);
          current = newcurrent; // right going h edge
        }
        prevcolour = colour; // remember new colour
      }
    } else {
      if (colour != prevcolour) {
        *prevline = current = v_edge(colour - prevcolour, current, &pos);
        prevcolour = colour;
      }
      if (colour != uppercolour) {
        current = h_edge(uppercolour - colour, current, &pos);
      } else {
        current = nullptr; // no edge now
      }
    }
  }
  if (current != nullptr) {
    // out of block
    if (*prevline != nullptr) { // got one to join to?
      join_edges(current, *prevline, free_cracks, outline_it);
      *prevline = nullptr; // tidy now
    } else {
      // fake vertical
      *prevline = v_edge(FLIP_COLOUR(prevcolour) - prevcolour, current, &pos);
    }
  } else if (*prevline != nullptr) {
    // continue fake
    *prevline = v_edge(FLIP_COLOUR(prevcolour) - prevcolour, *prevline, &pos);
  }
}

/**********************************************************************
 * h_edge
 *
 * Create a new horizontal CRACKEDGE and join it to the given edge.
 **********************************************************************/

static CRACKEDGE *h_edge(int sign,        // sign of edge
                         CRACKEDGE *join, // edge to join to
                         CrackPos *pos) {
  CRACKEDGE *newpt; // return value

  if (*pos->free_cracks != nullptr) {
    newpt = *pos->free_cracks;
    *pos->free_cracks = newpt->next; // get one fast
  } else {
    newpt = new CRACKEDGE;
  }
  newpt->pos.set_y(pos->y + 1); // coords of pt
  newpt->stepy = 0;             // edge is horizontal

  if (sign > 0) {
    newpt->pos.set_x(pos->x + 1); // start location
    newpt->stepx = -1;
    newpt->stepdir = 0;
  } else {
    newpt->pos.set_x(pos->x); // start location
    newpt->stepx = 1;
    newpt->stepdir = 2;
  }

  if (join == nullptr) {
    newpt->next = newpt; // ptrs to other ends
    newpt->prev = newpt;
  } else {
    if (newpt->pos.x() + newpt->stepx == join->pos.x() && newpt->pos.y() == join->pos.y()) {
      newpt->prev = join->prev; // update other ends
      newpt->prev->next = newpt;
      newpt->next = join; // join up
      join->prev = newpt;
    } else {
      newpt->next = join->next; // update other ends
      newpt->next->prev = newpt;
      newpt->prev = join; // join up
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

static CRACKEDGE *v_edge(int sign, // sign of edge
                         CRACKEDGE *join, CrackPos *pos) {
  CRACKEDGE *newpt; // return value

  if (*pos->free_cracks != nullptr) {
    newpt = *pos->free_cracks;
    *pos->free_cracks = newpt->next; // get one fast
  } else {
    newpt = new CRACKEDGE;
  }
  newpt->pos.set_x(pos->x); // coords of pt
  newpt->stepx = 0;         // edge is vertical

  if (sign > 0) {
    newpt->pos.set_y(pos->y); // start location
    newpt->stepy = 1;
    newpt->stepdir = 3;
  } else {
    newpt->pos.set_y(pos->y + 1); // start location
    newpt->stepy = -1;
    newpt->stepdir = 1;
  }

  if (join == nullptr) {
    newpt->next = newpt; // ptrs to other ends
    newpt->prev = newpt;
  } else {
    if (newpt->pos.x() == join->pos.x() && newpt->pos.y() + newpt->stepy == join->pos.y()) {
      newpt->prev = join->prev; // update other ends
      newpt->prev->next = newpt;
      newpt->next = join; // join up
      join->prev = newpt;
    } else {
      newpt->next = join->next; // update other ends
      newpt->next->prev = newpt;
      newpt->prev = join; // join up
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

static void join_edges(CRACKEDGE *edge1, // edges to join
                       CRACKEDGE *edge2, // no specific order
                       CRACKEDGE **free_cracks, C_OUTLINE_IT *outline_it) {
  if (edge1->pos.x() + edge1->stepx != edge2->pos.x() ||
      edge1->pos.y() + edge1->stepy != edge2->pos.y()) {
    CRACKEDGE *tempedge = edge1;
    edge1 = edge2; // swap around
    edge2 = tempedge;
  }

  if (edge1->next == edge2) {
    // already closed
    complete_edge(edge1, outline_it);
    // attach freelist to end
    edge1->prev->next = *free_cracks;
    *free_cracks = edge1; // and free list
  } else {
    // update opposite ends
    edge2->prev->next = edge1->next;
    edge1->next->prev = edge2->prev;
    edge1->next = edge2; // make joins
    edge2->prev = edge1;
  }
}

/**********************************************************************
 * free_crackedges
 *
 * Really free the CRACKEDGEs by giving them back to delete.
 **********************************************************************/

static void free_crackedges(CRACKEDGE *start) {
  CRACKEDGE *current; // current edge to free
  CRACKEDGE *next;    // next one to free

  for (current = start; current != nullptr; current = next) {
    next = current->next;
    delete current; // delete them all
  }
}

} // namespace tesseract
