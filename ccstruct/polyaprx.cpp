/**********************************************************************
 * File:        polyaprx.cpp  (Formerly polygon.c)
 * Description: Code for polygonal approximation from old edgeprog.
 * Author:      Ray Smith
 * Created:     Thu Nov 25 11:42:04 GMT 1993
 *
 * (C) Copyright 1993, Hewlett-Packard Ltd.
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
#include          <stdio.h>
#ifdef __UNIX__
#include          <assert.h>
#endif
#define FASTEDGELENGTH    256
#include          "polyaprx.h"
#include          "params.h"
#include          "tprintf.h"

#define EXTERN

EXTERN BOOL_VAR (poly_debug, FALSE, "Debug old poly");
EXTERN BOOL_VAR (poly_wide_objects_better, TRUE,
"More accurate approx on wide things");


#define FIXED       4            /*OUTLINE point is fixed */

#define RUNLENGTH     1          /*length of run */

#define DIR         2            /*direction of run */

#define FLAGS       0

#define fixed_dist      20       //really an int_variable
#define approx_dist     15       //really an int_variable

const int par1 = 4500 / (approx_dist * approx_dist);
const int par2 = 6750 / (approx_dist * approx_dist);


/**********************************************************************
 * tesspoly_outline
 *
 * Approximate an outline from chain codes form using the old tess algorithm.
 **********************************************************************/


TESSLINE* ApproximateOutline(C_OUTLINE* c_outline) {
  EDGEPT *edgept;                // converted steps
  TBOX loop_box;                  // bounding box
  inT32 area;                    // loop area
  EDGEPT stack_edgepts[FASTEDGELENGTH];  // converted path
  EDGEPT* edgepts = stack_edgepts;

  // Use heap memory if the stack buffer is not big enough.
  if (c_outline->pathlength() > FASTEDGELENGTH)
    edgepts = new EDGEPT[c_outline->pathlength()];

  loop_box = c_outline->bounding_box();
  area = loop_box.height();
  if (!poly_wide_objects_better && loop_box.width() > area)
    area = loop_box.width();
  area *= area;
  edgept = edgesteps_to_edgepts(c_outline, edgepts);
  fix2(edgepts, area);
  edgept = poly2 (edgepts, area);  // 2nd approximation.
  EDGEPT* startpt = edgept;
  EDGEPT* result = NULL;
  EDGEPT* prev_result = NULL;
  do {
    EDGEPT* new_pt = new EDGEPT;
    new_pt->pos = edgept->pos;
    new_pt->prev = prev_result;
    if (prev_result == NULL) {
      result = new_pt;
    } else {
      prev_result->next = new_pt;
      new_pt->prev = prev_result;
    }
    prev_result = new_pt;
    edgept = edgept->next;
  }
  while (edgept != startpt);
  prev_result->next = result;
  result->prev = prev_result;
  if (edgepts != stack_edgepts)
    delete [] edgepts;
  return TESSLINE::BuildFromOutlineList(result);
}


/**********************************************************************
 * edgesteps_to_edgepts
 *
 * Convert a C_OUTLINE to EDGEPTs.
 **********************************************************************/

EDGEPT *
edgesteps_to_edgepts (           //convert outline
C_OUTLINE * c_outline,           //input
EDGEPT edgepts[]                 //output is array
) {
  inT32 length;                  //steps in path
  ICOORD pos;                    //current coords
  inT32 stepindex;               //current step
  inT32 stepinc;                 //increment
  inT32 epindex;                 //current EDGEPT
  inT32 count;                   //repeated steps
  ICOORD vec;                    //for this 8 step
  ICOORD prev_vec;
  inT8 epdir;                    //of this step
  DIR128 prevdir;                //prvious dir
  DIR128 dir;                    //of this step

  pos = c_outline->start_pos (); //start of loop
  length = c_outline->pathlength ();
  stepindex = 0;
  epindex = 0;
  prevdir = -1;
  count = 0;
  do {
    dir = c_outline->step_dir (stepindex);
    vec = c_outline->step (stepindex);
    if (stepindex < length - 1
    && c_outline->step_dir (stepindex + 1) - dir == -32) {
      dir += 128 - 16;
      vec += c_outline->step (stepindex + 1);
      stepinc = 2;
    }
    else
      stepinc = 1;
    if (count == 0) {
      prevdir = dir;
      prev_vec = vec;
    }
    if (prevdir.get_dir () != dir.get_dir ()) {
      edgepts[epindex].pos.x = pos.x ();
      edgepts[epindex].pos.y = pos.y ();
      prev_vec *= count;
      edgepts[epindex].vec.x = prev_vec.x ();
      edgepts[epindex].vec.y = prev_vec.y ();
      pos += prev_vec;
      edgepts[epindex].flags[RUNLENGTH] = count;
      edgepts[epindex].prev = &edgepts[epindex - 1];
      edgepts[epindex].flags[FLAGS] = 0;
      edgepts[epindex].next = &edgepts[epindex + 1];
      prevdir += 64;
      epdir = (DIR128) 0 - prevdir;
      epdir >>= 4;
      epdir &= 7;
      edgepts[epindex].flags[DIR] = epdir;
      epindex++;
      prevdir = dir;
      prev_vec = vec;
      count = 1;
    }
    else
      count++;
    stepindex += stepinc;
  }
  while (stepindex < length);
  edgepts[epindex].pos.x = pos.x ();
  edgepts[epindex].pos.y = pos.y ();
  prev_vec *= count;
  edgepts[epindex].vec.x = prev_vec.x ();
  edgepts[epindex].vec.y = prev_vec.y ();
  pos += prev_vec;
  edgepts[epindex].flags[RUNLENGTH] = count;
  edgepts[epindex].flags[FLAGS] = 0;
  edgepts[epindex].prev = &edgepts[epindex - 1];
  edgepts[epindex].next = &edgepts[0];
  prevdir += 64;
  epdir = (DIR128) 0 - prevdir;
  epdir >>= 4;
  epdir &= 7;
  edgepts[epindex].flags[DIR] = epdir;
  edgepts[0].prev = &edgepts[epindex];
  ASSERT_HOST (pos.x () == c_outline->start_pos ().x ()
    && pos.y () == c_outline->start_pos ().y ());
  return &edgepts[0];
}


/**********************************************************************
 *fix2(start,area) fixes points on the outline according to a trial method*
 **********************************************************************/

//#pragma OPT_LEVEL 1                                                                           /*stop compiler bugs*/

void fix2(                //polygonal approx
          EDGEPT *start,  /*loop to approimate */
          int area) {
  register EDGEPT *edgept;       /*current point */
  register EDGEPT *edgept1;
  register EDGEPT *loopstart;    /*modified start of loop */
  register EDGEPT *linestart;    /*start of line segment */
  register int dir1, dir2;       /*directions of line */
  register int sum1, sum2;       /*lengths in dir1,dir2 */
  int stopped;                   /*completed flag */
  int fixed_count;               //no of fixed points
  int d01, d12, d23, gapmin;
  TPOINT d01vec, d12vec, d23vec;
  register EDGEPT *edgefix, *startfix;
  register EDGEPT *edgefix0, *edgefix1, *edgefix2, *edgefix3;

  edgept = start;                /*start of loop */
  while (((edgept->flags[DIR] - edgept->prev->flags[DIR] + 1) & 7) < 3
    && (dir1 =
    (edgept->prev->flags[DIR] - edgept->next->flags[DIR]) & 7) != 2
    && dir1 != 6)
    edgept = edgept->next;       /*find suitable start */
  loopstart = edgept;            /*remember start */

  stopped = 0;                   /*not finished yet */
  edgept->flags[FLAGS] |= FIXED; /*fix it */
  do {
    linestart = edgept;          /*possible start of line */
    dir1 = edgept->flags[DIR];   /*first direction */
                                 /*length of dir1 */
    sum1 = edgept->flags[RUNLENGTH];
    edgept = edgept->next;
    dir2 = edgept->flags[DIR];   /*2nd direction */
                                 /*length in dir2 */
    sum2 = edgept->flags[RUNLENGTH];
    if (((dir1 - dir2 + 1) & 7) < 3) {
      while (edgept->prev->flags[DIR] == edgept->next->flags[DIR]) {
        edgept = edgept->next;   /*look at next */
        if (edgept->flags[DIR] == dir1)
                                 /*sum lengths */
          sum1 += edgept->flags[RUNLENGTH];
        else
          sum2 += edgept->flags[RUNLENGTH];
      }

      if (edgept == loopstart)
        stopped = 1;             /*finished */
      if (sum2 + sum1 > 2
        && linestart->prev->flags[DIR] == dir2
        && (linestart->prev->flags[RUNLENGTH] >
      linestart->flags[RUNLENGTH] || sum2 > sum1)) {
                                 /*start is back one */
        linestart = linestart->prev;
        linestart->flags[FLAGS] |= FIXED;
      }

      if (((edgept->next->flags[DIR] - edgept->flags[DIR] + 1) & 7) >= 3
        || (edgept->flags[DIR] == dir1 && sum1 >= sum2)
        || ((edgept->prev->flags[RUNLENGTH] < edgept->flags[RUNLENGTH]
        || (edgept->flags[DIR] == dir2 && sum2 >= sum1))
          && linestart->next != edgept))
        edgept = edgept->next;
    }
                                 /*sharp bend */
    edgept->flags[FLAGS] |= FIXED;
  }
                                 /*do whole loop */
  while (edgept != loopstart && !stopped);

  edgept = start;
  do {
    if (((edgept->flags[RUNLENGTH] >= 8) &&
      (edgept->flags[DIR] != 2) && (edgept->flags[DIR] != 6)) ||
      ((edgept->flags[RUNLENGTH] >= 8) &&
    ((edgept->flags[DIR] == 2) || (edgept->flags[DIR] == 6)))) {
      edgept->flags[FLAGS] |= FIXED;
      edgept1 = edgept->next;
      edgept1->flags[FLAGS] |= FIXED;
    }
    edgept = edgept->next;
  }
  while (edgept != start);

  edgept = start;
  do {
                                 /*single fixed step */
    if (edgept->flags[FLAGS] & FIXED && edgept->flags[RUNLENGTH] == 1
                                 /*and neighours free */
      && edgept->next->flags[FLAGS] & FIXED && (edgept->prev->flags[FLAGS] & FIXED) == 0
                                 /*same pair of dirs */
      && (edgept->next->next->flags[FLAGS] & FIXED) == 0 && edgept->prev->flags[DIR] == edgept->next->flags[DIR] && edgept->prev->prev->flags[DIR] == edgept->next->next->flags[DIR]
    && ((edgept->prev->flags[DIR] - edgept->flags[DIR] + 1) & 7) < 3) {
                                 /*unfix it */
      edgept->flags[FLAGS] &= ~FIXED;
      edgept->next->flags[FLAGS] &= ~FIXED;
    }
    edgept = edgept->next;       /*do all points */
  }
  while (edgept != start);       /*until finished */

  stopped = 0;
  if (area < 450)
    area = 450;

  gapmin = area * fixed_dist * fixed_dist / 44000;

  edgept = start;
  fixed_count = 0;
  do {
    if (edgept->flags[FLAGS] & FIXED)
      fixed_count++;
    edgept = edgept->next;
  }
  while (edgept != start);
  while ((edgept->flags[FLAGS] & FIXED) == 0)
    edgept = edgept->next;
  edgefix0 = edgept;

  edgept = edgept->next;
  while ((edgept->flags[FLAGS] & FIXED) == 0)
    edgept = edgept->next;
  edgefix1 = edgept;

  edgept = edgept->next;
  while ((edgept->flags[FLAGS] & FIXED) == 0)
    edgept = edgept->next;
  edgefix2 = edgept;

  edgept = edgept->next;
  while ((edgept->flags[FLAGS] & FIXED) == 0)
    edgept = edgept->next;
  edgefix3 = edgept;

  startfix = edgefix2;

  do {
    if (fixed_count <= 3)
      break;                     //already too few
    point_diff (d12vec, edgefix1->pos, edgefix2->pos);
    d12 = LENGTH (d12vec);
    // TODO(rays) investigate this change:
    // Only unfix a point if it is part of a low-curvature section
    // of outline and the total angle change of the outlines is
    // less than 90 degrees, ie the scalar product is positive.
    // if (d12 <= gapmin && SCALAR(edgefix0->vec, edgefix2->vec) > 0) {
    if (d12 <= gapmin) {
      point_diff (d01vec, edgefix0->pos, edgefix1->pos);
      d01 = LENGTH (d01vec);
      point_diff (d23vec, edgefix2->pos, edgefix3->pos);
      d23 = LENGTH (d23vec);
      if (d01 > d23) {
        edgefix2->flags[FLAGS] &= ~FIXED;
        fixed_count--;
      }
      else {
        edgefix1->flags[FLAGS] &= ~FIXED;
        fixed_count--;
        edgefix1 = edgefix2;
      }
    }
    else {
      edgefix0 = edgefix1;
      edgefix1 = edgefix2;
    }
    edgefix2 = edgefix3;
    edgept = edgept->next;
    while ((edgept->flags[FLAGS] & FIXED) == 0) {
      if (edgept == startfix)
        stopped = 1;
      edgept = edgept->next;
    }
    edgefix3 = edgept;
    edgefix = edgefix2;
  }
  while ((edgefix != startfix) && (!stopped));
}


//#pragma OPT_LEVEL 2                                                                           /*stop compiler bugs*/

/**********************************************************************
 *poly2(startpt,area,path) applies a second approximation to the outline
 *using the points which have been fixed by the first approximation*
 **********************************************************************/

EDGEPT *poly2(                  //second poly
              EDGEPT *startpt,  /*start of loop */
              int area          /*area of blob box */
             ) {
  register EDGEPT *edgept;       /*current outline point */
  EDGEPT *loopstart;             /*starting point */
  register EDGEPT *linestart;    /*start of line */
  register int edgesum;          /*correction count */

  if (area < 1200)
    area = 1200;                 /*minimum value */

  loopstart = NULL;              /*not found it yet */
  edgept = startpt;              /*start of loop */

  do {
                                 /*current point fixed */
    if (edgept->flags[FLAGS] & FIXED
                                 /*and next not */
    && (edgept->next->flags[FLAGS] & FIXED) == 0) {
      loopstart = edgept;        /*start of repoly */
      break;
    }
    edgept = edgept->next;       /*next point */
  }
  while (edgept != startpt);     /*until found or finished */

  if (loopstart == NULL && (startpt->flags[FLAGS] & FIXED) == 0) {
                                 /*fixed start of loop */
    startpt->flags[FLAGS] |= FIXED;
    loopstart = startpt;         /*or start of loop */
  }
  if (loopstart) {
    do {
      edgept = loopstart;        /*first to do */
      do {
        linestart = edgept;
        edgesum = 0;             /*sum of lengths */
        do {
                                 /*sum lengths */
          edgesum += edgept->flags[RUNLENGTH];
          edgept = edgept->next; /*move on */
        }
        while ((edgept->flags[FLAGS] & FIXED) == 0
          && edgept != loopstart && edgesum < 126);
        if (poly_debug)
          tprintf
            ("Poly2:starting at (%d,%d)+%d=(%d,%d),%d to (%d,%d)\n",
            linestart->pos.x, linestart->pos.y, linestart->flags[DIR],
            linestart->vec.x, linestart->vec.y, edgesum, edgept->pos.x,
            edgept->pos.y);
                                 /*reapproximate */
        cutline(linestart, edgept, area);

        while ((edgept->next->flags[FLAGS] & FIXED)
          && edgept != loopstart)
          edgept = edgept->next; /*look for next non-fixed */
      }
                                 /*do all the loop */
      while (edgept != loopstart);
      edgesum = 0;
      do {
        if (edgept->flags[FLAGS] & FIXED)
          edgesum++;
        edgept = edgept->next;
      }
                                 //count fixed pts
      while (edgept != loopstart);
      if (edgesum < 3)
        area /= 2;               //must have 3 pts
    }
    while (edgesum < 3);
    do {
      linestart = edgept;
      do {
        edgept = edgept->next;
      }
      while ((edgept->flags[FLAGS] & FIXED) == 0);
      linestart->next = edgept;
      edgept->prev = linestart;
      linestart->vec.x = edgept->pos.x - linestart->pos.x;
      linestart->vec.y = edgept->pos.y - linestart->pos.y;
    }
    while (edgept != loopstart);
  }
  else
    edgept = startpt;            /*start of loop */

  loopstart = edgept;            /*new start */
  return loopstart;              /*correct exit */
}


/**********************************************************************
 *cutline(first,last,area) straightens out a line by partitioning
 *and joining the ends by a straight line*
 **********************************************************************/

void cutline(                //recursive refine
             EDGEPT *first,  /*ends of line */
             EDGEPT *last,
             int area        /*area of object */
            ) {
  register EDGEPT *edge;         /*current edge */
  TPOINT vecsum;                 /*vector sum */
  int vlen;                      /*approx length of vecsum */
  TPOINT vec;                    /*accumulated vector */
  EDGEPT *maxpoint;              /*worst point */
  int maxperp;                   /*max deviation */
  register int perp;             /*perp distance */
  int ptcount;                   /*no of points */
  int squaresum;                 /*sum of perps */

  edge = first;                  /*start of line */
  if (edge->next == last)
    return;                      /*simple line */

                                 /*vector sum */
  vecsum.x = last->pos.x - edge->pos.x;
  vecsum.y = last->pos.y - edge->pos.y;
  if (vecsum.x == 0 && vecsum.y == 0) {
                                 /*special case */
    vecsum.x = -edge->prev->vec.x;
    vecsum.y = -edge->prev->vec.y;
  }
                                 /*absolute value */
  vlen = vecsum.x > 0 ? vecsum.x : -vecsum.x;
  if (vecsum.y > vlen)
    vlen = vecsum.y;             /*maximum */
  else if (-vecsum.y > vlen)
    vlen = -vecsum.y;            /*absolute value */

  vec.x = edge->vec.x;           /*accumulated vector */
  vec.y = edge->vec.y;
  maxperp = 0;                   /*none yet */
  squaresum = ptcount = 0;
  edge = edge->next;             /*move to actual point */
  maxpoint = edge;               /*in case there isn't one */
  do {
    perp = CROSS (vec, vecsum);  /*get perp distance */
    if (perp != 0) {
      perp *= perp;              /*squared deviation */
    }
    squaresum += perp;           /*sum squares */
    ptcount++;                   /*count points */
    if (poly_debug)
      tprintf ("Cutline:Final perp=%d\n", perp);
    if (perp > maxperp) {
      maxperp = perp;
      maxpoint = edge;           /*find greatest deviation */
    }
    vec.x += edge->vec.x;        /*accumulate vectors */
    vec.y += edge->vec.y;
    edge = edge->next;
  }
  while (edge != last);          /*test all line */

  perp = LENGTH (vecsum);
  ASSERT_HOST (perp != 0);

  if (maxperp < 256 * MAX_INT16) {
    maxperp <<= 8;
    maxperp /= perp;             /*true max perp */
  }
  else {
    maxperp /= perp;
    maxperp <<= 8;               /*avoid overflow */
  }
  if (squaresum < 256 * MAX_INT16)
                                 /*mean squared perp */
    perp = (squaresum << 8) / (perp * ptcount);
  else
                                 /*avoid overflow */
    perp = (squaresum / perp << 8) / ptcount;

  if (poly_debug)
    tprintf ("Cutline:A=%d, max=%.2f(%.2f%%), msd=%.2f(%.2f%%)\n",
      area, maxperp / 256.0, maxperp * 200.0 / area,
      perp / 256.0, perp * 300.0 / area);
  if (maxperp * par1 >= 10 * area || perp * par2 >= 10 * area || vlen >= 126) {
    maxpoint->flags[FLAGS] |= FIXED;
                                 /*partitions */
    cutline(first, maxpoint, area);
    cutline(maxpoint, last, area);
  }
}
