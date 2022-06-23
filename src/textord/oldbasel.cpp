/**********************************************************************
 * File:        oldbasel.cpp  (Formerly oldbl.c)
 * Description: A re-implementation of the old baseline algorithm.
 * Author:      Ray Smith
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

// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#  include "config_auto.h"
#endif

#include "oldbasel.h"

#include "ccstruct.h"
#include "detlinefit.h"
#include "drawtord.h"
#include "makerow.h"
#include "quadlsq.h"
#include "statistc.h"
#include "textord.h"
#include "tprintf.h"

#include <cmath>
#include <vector> // for std::vector

#include <algorithm>

namespace tesseract {

static BOOL_VAR(textord_really_old_xheight, false, "Use original wiseowl xheight");
BOOL_VAR(textord_oldbl_debug, false, "Debug old baseline generation");
static BOOL_VAR(textord_debug_baselines, false, "Debug baseline generation");
static BOOL_VAR(textord_oldbl_paradef, true, "Use para default mechanism");
static BOOL_VAR(textord_oldbl_split_splines, true, "Split stepped splines");
static BOOL_VAR(textord_oldbl_merge_parts, true, "Merge suspect partitions");
static BOOL_VAR(oldbl_corrfix, true, "Improve correlation of heights");
static BOOL_VAR(oldbl_xhfix, false, "Fix bug in modes threshold for xheights");
static BOOL_VAR(textord_ocropus_mode, false, "Make baselines for ocropus");
static double_VAR(oldbl_xhfract, 0.4, "Fraction of est allowed in calc");
static INT_VAR(oldbl_holed_losscount, 10, "Max lost before fallback line used");
static double_VAR(oldbl_dot_error_size, 1.26, "Max aspect ratio of a dot");
static double_VAR(textord_oldbl_jumplimit, 0.15, "X fraction for new partition");

#define TURNLIMIT 1            /*min size for turning point */
#define X_HEIGHT_FRACTION 0.7  /*x-height/caps height */
#define DESCENDER_FRACTION 0.5 /*descender/x-height */
#define MIN_ASC_FRACTION 0.20  /*min size of ascenders */
#define MIN_DESC_FRACTION 0.25 /*min size of descenders */
#define MINASCRISE 2.0         /*min ascender/desc step */
#define MAXHEIGHTVARIANCE 0.15 /*accepted variation in x-height */
#define MAXHEIGHT 300          /*max blob height */
#define MAXOVERLAP 0.1         /*max 10% missed overlap */
#define MAXBADRUN 2            /*max non best for failed */
#define HEIGHTBUCKETS 200      /* Num of buckets */
#define MODENUM 10
#define MAXPARTS 6
#define SPLINESIZE 23

#define ABS(x) ((x) < 0 ? (-(x)) : (x))

/**********************************************************************
 * make_old_baselines
 *
 * Top level function to make baselines the old way.
 **********************************************************************/

void Textord::make_old_baselines(TO_BLOCK *block, // block to do
                                 bool testing_on, // correct orientation
                                 float gradient) {
  QSPLINE *prev_baseline; // baseline of previous row
  TO_ROW *row;            // current row
  TO_ROW_IT row_it = block->get_rows();
  BLOBNBOX_IT blob_it;

  prev_baseline = nullptr; // nothing yet
  for (row_it.mark_cycle_pt(); !row_it.cycled_list(); row_it.forward()) {
    row = row_it.data();
    find_textlines(block, row, 2, nullptr);
    if (row->xheight <= 0 && prev_baseline != nullptr) {
      find_textlines(block, row, 2, prev_baseline);
    }
    if (row->xheight > 0) { // was a good one
      prev_baseline = &row->baseline;
    } else {
      prev_baseline = nullptr;
      blob_it.set_to_list(row->blob_list());
      if (textord_debug_baselines) {
        tprintf("Row baseline generation failed on row at (%d,%d)\n",
                blob_it.data()->bounding_box().left(), blob_it.data()->bounding_box().bottom());
      }
    }
  }
  correlate_lines(block, gradient);
  block->block->set_xheight(block->xheight);
}

/**********************************************************************
 * correlate_lines
 *
 * Correlate the x-heights and ascender heights of a block to fill-in
 * the ascender height and descender height for rows without one.
 * Also fix baselines of rows without a decent fit.
 **********************************************************************/

void Textord::correlate_lines(TO_BLOCK *block, float gradient) {
  int rowcount; /*no of rows to do */
  int rowindex; /*no of row */
                // iterator
  TO_ROW_IT row_it = block->get_rows();

  rowcount = row_it.length();
  if (rowcount == 0) {
    // default value
    block->xheight = block->line_size;
    return; /*none to do */
  }
  // array of ptrs
  std::vector<TO_ROW *> rows(rowcount);
  rowindex = 0;
  for (row_it.mark_cycle_pt(); !row_it.cycled_list(); row_it.forward()) {
    // make array
    rows[rowindex++] = row_it.data();
  }

  /*try to fix bad lines */
  correlate_neighbours(block, &rows[0], rowcount);

  if (textord_really_old_xheight || textord_old_xheight) {
    block->xheight = static_cast<float>(correlate_with_stats(&rows[0], rowcount, block));
    if (block->xheight <= 0) {
      block->xheight = block->line_size * tesseract::CCStruct::kXHeightFraction;
    }
    if (block->xheight < textord_min_xheight) {
      block->xheight = (float)textord_min_xheight;
    }
  } else {
    compute_block_xheight(block, gradient);
  }
}

/**********************************************************************
 * correlate_neighbours
 *
 * Try to fix rows that had a bad spline fit by using neighbours.
 **********************************************************************/

void Textord::correlate_neighbours(TO_BLOCK *block, // block rows are in.
                                   TO_ROW **rows,   // rows of block.
                                   int rowcount) {  // no of rows to do.
  TO_ROW *row;                                      /*current row */
  int rowindex;                                     /*no of row */
  int otherrow;                                     /*second row */
  int upperrow;                                     /*row above to use */
  int lowerrow;                                     /*row below to use */
  float biggest;

  for (rowindex = 0; rowindex < rowcount; rowindex++) {
    row = rows[rowindex]; /*current row */
    if (row->xheight < 0) {
      /*quadratic failed */
      for (otherrow = rowindex - 2;
           otherrow >= 0 && (rows[otherrow]->xheight < 0.0 ||
                             !row->baseline.overlap(&rows[otherrow]->baseline, MAXOVERLAP));
           otherrow--) {
      }
      upperrow = otherrow; /*decent row above */
      for (otherrow = rowindex + 1;
           otherrow < rowcount && (rows[otherrow]->xheight < 0.0 ||
                                   !row->baseline.overlap(&rows[otherrow]->baseline, MAXOVERLAP));
           otherrow++) {
      }
      lowerrow = otherrow; /*decent row below */
      if (upperrow >= 0) {
        find_textlines(block, row, 2, &rows[upperrow]->baseline);
      }
      if (row->xheight < 0 && lowerrow < rowcount) {
        find_textlines(block, row, 2, &rows[lowerrow]->baseline);
      }
      if (row->xheight < 0) {
        if (upperrow >= 0) {
          find_textlines(block, row, 1, &rows[upperrow]->baseline);
        } else if (lowerrow < rowcount) {
          find_textlines(block, row, 1, &rows[lowerrow]->baseline);
        }
      }
    }
  }

  for (biggest = 0.0f, rowindex = 0; rowindex < rowcount; rowindex++) {
    row = rows[rowindex]; /*current row */
    if (row->xheight < 0) { /*linear failed */
                            /*make do */
      row->xheight = -row->xheight;
    }
    biggest = std::max(biggest, row->xheight);
  }
}

/**********************************************************************
 * correlate_with_stats
 *
 * correlate the x-heights and ascender heights of a block to fill-in
 * the ascender height and descender height for rows without one.
 **********************************************************************/

int Textord::correlate_with_stats(TO_ROW **rows, // rows of block.
                                  int rowcount,  // no of rows to do.
                                  TO_BLOCK *block) {
  TO_ROW *row;         /*current row */
  int rowindex;        /*no of row */
  float lineheight;    /*mean x-height */
  float ascheight;     /*average ascenders */
  float minascheight;  /*min allowed ascheight */
  int xcount;          /*no of samples for xheight */
  float fullheight;    /*mean top height */
  int fullcount;       /*no of samples */
  float descheight;    /*mean descender drop */
  float mindescheight; /*min allowed descheight */
  int desccount;       /*no of samples */

  /*no samples */
  xcount = fullcount = desccount = 0;
  lineheight = ascheight = fullheight = descheight = 0.0;
  for (rowindex = 0; rowindex < rowcount; rowindex++) {
    row = rows[rowindex];         /*current row */
    if (row->ascrise > 0.0) {     /*got ascenders? */
      lineheight += row->xheight; /*average x-heights */
      ascheight += row->ascrise;  /*average ascenders */
      xcount++;
    } else {
      fullheight += row->xheight; /*assume full height */
      fullcount++;
    }
    if (row->descdrop < 0.0) { /*got descenders? */
                               /*average descenders */
      descheight += row->descdrop;
      desccount++;
    }
  }

  if (xcount > 0 && (!oldbl_corrfix || xcount >= fullcount)) {
    lineheight /= xcount; /*average x-height */
                          /*average caps height */
    fullheight = lineheight + ascheight / xcount;
    /*must be decent size */
    if (fullheight < lineheight * (1 + MIN_ASC_FRACTION)) {
      fullheight = lineheight * (1 + MIN_ASC_FRACTION);
    }
  } else {
    fullheight /= fullcount; /*average max height */
                             /*guess x-height */
    lineheight = fullheight * X_HEIGHT_FRACTION;
  }
  if (desccount > 0 && (!oldbl_corrfix || desccount >= rowcount / 2)) {
    descheight /= desccount; /*average descenders */
  } else {
    /*guess descenders */
    descheight = -lineheight * DESCENDER_FRACTION;
  }

  if (lineheight > 0.0f) {
    block->block->set_cell_over_xheight((fullheight - descheight) / lineheight);
  }

  minascheight = lineheight * MIN_ASC_FRACTION;
  mindescheight = -lineheight * MIN_DESC_FRACTION;
  for (rowindex = 0; rowindex < rowcount; rowindex++) {
    row = rows[rowindex]; /*do each row */
    row->all_caps = false;
    if (row->ascrise / row->xheight < MIN_ASC_FRACTION) {
      /*no ascenders */
      if (row->xheight >= lineheight * (1 - MAXHEIGHTVARIANCE) &&
          row->xheight <= lineheight * (1 + MAXHEIGHTVARIANCE)) {
        row->ascrise = fullheight - lineheight;
        /*set to average */
        row->xheight = lineheight;

      } else if (row->xheight >= fullheight * (1 - MAXHEIGHTVARIANCE) &&
                 row->xheight <= fullheight * (1 + MAXHEIGHTVARIANCE)) {
        row->ascrise = row->xheight - lineheight;
        /*set to average */
        row->xheight = lineheight;
        row->all_caps = true;
      } else {
        row->ascrise = (fullheight - lineheight) * row->xheight / fullheight;
        /*scale it */
        row->xheight -= row->ascrise;
        row->all_caps = true;
      }
      if (row->ascrise < minascheight) {
        row->ascrise = row->xheight * ((1.0 - X_HEIGHT_FRACTION) / X_HEIGHT_FRACTION);
      }
    }
    if (row->descdrop > mindescheight) {
      if (row->xheight >= lineheight * (1 - MAXHEIGHTVARIANCE) &&
          row->xheight <= lineheight * (1 + MAXHEIGHTVARIANCE)) {
        /*set to average */
        row->descdrop = descheight;
      } else {
        row->descdrop = -row->xheight * DESCENDER_FRACTION;
      }
    }
  }
  return static_cast<int>(lineheight); // block xheight
}

/**********************************************************************
 * find_textlines
 *
 * Compute the baseline for the given row.
 **********************************************************************/

void Textord::find_textlines(TO_BLOCK *block,   // block row is in
                             TO_ROW *row,       // row to do
                             int degree,        // required approximation
                             QSPLINE *spline) { // starting spline
  int partcount;                                /*no of partitions of */
  bool holed_line = false;                      // lost too many blobs
  int bestpart;                                 /*biggest partition */
  int partsizes[MAXPARTS];                      /*no in each partition */
  int lineheight;                               /*guessed x-height */
  float jumplimit;                              /*allowed delta change */
  int blobcount;                                /*no of blobs on line */
  int pointcount;                               /*no of coords */
  int xstarts[SPLINESIZE + 1];                  // segment boundaries
  int segments;                                 // no of segments

  // no of blobs in row
  blobcount = row->blob_list()->length();
  // partition no of each blob
  std::vector<char> partids(blobcount);
  // useful sample points
  std::vector<int> xcoords(blobcount);
  // useful sample points
  std::vector<int> ycoords(blobcount);
  // edges of blob rectangles
  std::vector<TBOX> blobcoords(blobcount);
  // diffs from 1st approx
  std::vector<float> ydiffs(blobcount);

  lineheight = get_blob_coords(row, static_cast<int>(block->line_size), &blobcoords[0], holed_line,
                               blobcount);
  /*limit for line change */
  jumplimit = lineheight * textord_oldbl_jumplimit;
  if (jumplimit < MINASCRISE) {
    jumplimit = MINASCRISE;
  }

  if (textord_oldbl_debug) {
    tprintf("\nInput height=%g, Estimate x-height=%d pixels, jumplimit=%.2f\n", block->line_size,
            lineheight, jumplimit);
  }
  if (holed_line) {
    make_holed_baseline(&blobcoords[0], blobcount, spline, &row->baseline, row->line_m());
  } else {
    make_first_baseline(&blobcoords[0], blobcount, &xcoords[0], &ycoords[0], spline, &row->baseline,
                        jumplimit);
  }
#ifndef GRAPHICS_DISABLED
  if (textord_show_final_rows) {
    row->baseline.plot(to_win, ScrollView::GOLDENROD);
  }
#endif
  if (blobcount > 1) {
    bestpart = partition_line(&blobcoords[0], blobcount, &partcount, &partids[0], partsizes,
                              &row->baseline, jumplimit, &ydiffs[0]);
    pointcount = partition_coords(&blobcoords[0], blobcount, &partids[0], bestpart, &xcoords[0],
                                  &ycoords[0]);
    segments = segment_spline(&blobcoords[0], blobcount, &xcoords[0], &ycoords[0], degree,
                              pointcount, xstarts);
    if (!holed_line) {
      do {
        row->baseline = QSPLINE(xstarts, segments, &xcoords[0], &ycoords[0], pointcount, degree);
      } while (textord_oldbl_split_splines &&
               split_stepped_spline(&row->baseline, jumplimit / 2, &xcoords[0], xstarts, segments));
    }
    find_lesser_parts(row, &blobcoords[0], blobcount, &partids[0], partsizes, partcount, bestpart);

  } else {
    row->xheight = -1.0f; /*failed */
    row->descdrop = 0.0f;
    row->ascrise = 0.0f;
  }
  row->baseline.extrapolate(row->line_m(), block->block->pdblk.bounding_box().left(),
                            block->block->pdblk.bounding_box().right());

  if (textord_really_old_xheight) {
    old_first_xheight(row, &blobcoords[0], lineheight, blobcount, &row->baseline, jumplimit);
  } else if (textord_old_xheight) {
    make_first_xheight(row, &blobcoords[0], lineheight, static_cast<int>(block->line_size),
                       blobcount, &row->baseline, jumplimit);
  } else {
    compute_row_xheight(row, block->block->classify_rotation(), row->line_m(), block->line_size);
  }
}

/**********************************************************************
 * get_blob_coords
 *
 * Fill the blobcoords array with the coordinates of the blobs
 * in the row. The return value is the first guess at the line height.
 **********************************************************************/

int get_blob_coords(    // get boxes
    TO_ROW *row,        // row to use
    int32_t lineheight, // block level
    TBOX *blobcoords,   // output boxes
    bool &holed_line,   // lost a lot of blobs
    int &outcount       // no of real blobs
) {
  // blobs
  BLOBNBOX_IT blob_it = row->blob_list();
  int blobindex;    /*no along text line */
  int losscount;    // lost blobs
  int maxlosscount; // greatest lost blobs
  /*height stat collection */
  STATS heightstat(0, MAXHEIGHT - 1);

  if (blob_it.empty()) {
    return 0; // none
  }
  maxlosscount = 0;
  losscount = 0;
  blob_it.mark_cycle_pt();
  blobindex = 0;
  do {
    blobcoords[blobindex] = box_next_pre_chopped(&blob_it);
    if (blobcoords[blobindex].height() > lineheight * 0.25) {
      heightstat.add(blobcoords[blobindex].height(), 1);
    }
    if (blobindex == 0 || blobcoords[blobindex].height() > lineheight * 0.25 ||
        blob_it.cycled_list()) {
      blobindex++; /*no of merged blobs */
      losscount = 0;
    } else {
      if (blobcoords[blobindex].height() < blobcoords[blobindex].width() * oldbl_dot_error_size &&
          blobcoords[blobindex].width() < blobcoords[blobindex].height() * oldbl_dot_error_size) {
        // counts as dot
        blobindex++;
        losscount = 0;
      } else {
        losscount++; // lost it
        if (losscount > maxlosscount) {
          // remember max
          maxlosscount = losscount;
        }
      }
    }
  } while (!blob_it.cycled_list());

  holed_line = maxlosscount > oldbl_holed_losscount;
  outcount = blobindex; /*total blobs */

  if (heightstat.get_total() > 1) {
    /*guess x-height */
    return static_cast<int>(heightstat.ile(0.25));
  } else {
    return blobcoords[0].height();
  }
}

/**********************************************************************
 * make_first_baseline
 *
 * Make the first estimate at a baseline, either by shifting
 * a supplied previous spline, or by doing a piecewise linear
 * approximation using all the blobs.
 **********************************************************************/

void make_first_baseline( // initial approximation
    TBOX blobcoords[],    /*blob bounding boxes */
    int blobcount,        /*no of blobcoords */
    int xcoords[],        /*coords for spline */
    int ycoords[],        /*approximator */
    QSPLINE *spline,      /*initial spline */
    QSPLINE *baseline,    /*output spline */
    float jumplimit       /*guess half descenders */
) {
  int leftedge;              /*left edge of line */
  int rightedge;             /*right edge of line */
  int blobindex;             /*current blob */
  int segment;               /*current segment */
  float prevy, thisy, nexty; /*3 y coords */
  float y1, y2, y3;          /*3 smooth blobs */
  float maxmax, minmin;      /*absolute limits */
  int x2 = 0;                /*right edge of old y3 */
  int ycount;                /*no of ycoords in use */
  float yturns[SPLINESIZE];  /*y coords of turn pts */
  int xturns[SPLINESIZE];    /*xcoords of turn pts */
  int xstarts[SPLINESIZE + 1];
  int segments; // no of segments
  ICOORD shift; // shift of spline

  prevy = 0;
  /*left edge of row */
  leftedge = blobcoords[0].left();
  /*right edge of line */
  rightedge = blobcoords[blobcount - 1].right();
  if (spline == nullptr       /*no given spline */
      || spline->segments < 3 /*or trivial */
                              /*or too non-overlap */
      || spline->xcoords[1] > leftedge + MAXOVERLAP * (rightedge - leftedge) ||
      spline->xcoords[spline->segments - 1] < rightedge - MAXOVERLAP * (rightedge - leftedge)) {
    if (textord_oldbl_paradef) {
      return; // use default
    }
    xstarts[0] = blobcoords[0].left() - 1;
    for (blobindex = 0; blobindex < blobcount; blobindex++) {
      xcoords[blobindex] = (blobcoords[blobindex].left() + blobcoords[blobindex].right()) / 2;
      ycoords[blobindex] = blobcoords[blobindex].bottom();
    }
    xstarts[1] = blobcoords[blobcount - 1].right() + 1;
    segments = 1; /*no of segments */

    /*linear */
    *baseline = QSPLINE(xstarts, segments, xcoords, ycoords, blobcount, 1);

    if (blobcount >= 3) {
      y1 = y2 = y3 = 0.0f;
      ycount = 0;
      segment = 0; /*no of segments */
      maxmax = minmin = 0.0f;
      thisy = ycoords[0] - baseline->y(xcoords[0]);
      nexty = ycoords[1] - baseline->y(xcoords[1]);
      for (blobindex = 2; blobindex < blobcount; blobindex++) {
        prevy = thisy; /*shift ycoords */
        thisy = nexty;
        nexty = ycoords[blobindex] - baseline->y(xcoords[blobindex]);
        /*middle of smooth y */
        if (ABS(thisy - prevy) < jumplimit && ABS(thisy - nexty) < jumplimit) {
          y1 = y2; /*shift window */
          y2 = y3;
          y3 = thisy; /*middle point */
          ycount++;
          /*local max */
          if (ycount >= 3 && ((y1 < y2 && y2 >= y3)
                              /*local min */
                              || (y1 > y2 && y2 <= y3))) {
            if (segment < SPLINESIZE - 2) {
              /*turning pt */
              xturns[segment] = x2;
              yturns[segment] = y2;
              segment++; /*no of spline segs */
            }
          }
          if (ycount == 1) {
            maxmax = minmin = y3; /*initialise limits */
          } else {
            if (y3 > maxmax) {
              maxmax = y3; /*biggest max */
            }
            if (y3 < minmin) {
              minmin = y3; /*smallest min */
            }
          }
          /*possible turning pt */
          x2 = blobcoords[blobindex - 1].right();
        }
      }

      jumplimit *= 1.2f;
      /*must be wavy */
      if (maxmax - minmin > jumplimit) {
        ycount = segment; /*no of segments */
        for (blobindex = 0, segment = 1; blobindex < ycount; blobindex++) {
          if (yturns[blobindex] > minmin + jumplimit || yturns[blobindex] < maxmax - jumplimit) {
            /*significant peak */
            if (segment == 1 || yturns[blobindex] > prevy + jumplimit ||
                yturns[blobindex] < prevy - jumplimit) {
              /*different to previous */
              xstarts[segment] = xturns[blobindex];
              segment++;
              prevy = yturns[blobindex];
            }
            /*bigger max */
            else if ((prevy > minmin + jumplimit && yturns[blobindex] > prevy)
                     /*smaller min */
                     || (prevy < maxmax - jumplimit && yturns[blobindex] < prevy)) {
              xstarts[segment - 1] = xturns[blobindex];
              /*improved previous */
              prevy = yturns[blobindex];
            }
          }
        }
        xstarts[segment] = blobcoords[blobcount - 1].right() + 1;
        segments = segment; /*no of segments */
                            /*linear */
        *baseline = QSPLINE(xstarts, segments, xcoords, ycoords, blobcount, 1);
      }
    }
  } else {
    *baseline = *spline; /*copy it */
    shift =
        ICOORD(0, static_cast<int16_t>(blobcoords[0].bottom() - spline->y(blobcoords[0].right())));
    baseline->move(shift);
  }
}

/**********************************************************************
 * make_holed_baseline
 *
 * Make the first estimate at a baseline, either by shifting
 * a supplied previous spline, or by doing a piecewise linear
 * approximation using all the blobs.
 **********************************************************************/

void make_holed_baseline( // initial approximation
    TBOX blobcoords[],    /*blob bounding boxes */
    int blobcount,        /*no of blobcoords */
    QSPLINE *spline,      /*initial spline */
    QSPLINE *baseline,    /*output spline */
    float gradient        // of line
) {
  int leftedge;  /*left edge of line */
  int rightedge; /*right edge of line */
  int blobindex; /*current blob */
  float x;       // centre of row
  ICOORD shift;  // shift of spline

  tesseract::DetLineFit lms; // straight baseline
  int32_t xstarts[2];        // straight line
  double coeffs[3];
  float c; // line parameter

  /*left edge of row */
  leftedge = blobcoords[0].left();
  /*right edge of line */
  rightedge = blobcoords[blobcount - 1].right();
  for (blobindex = 0; blobindex < blobcount; blobindex++) {
    lms.Add(ICOORD((blobcoords[blobindex].left() + blobcoords[blobindex].right()) / 2,
                   blobcoords[blobindex].bottom()));
  }
  lms.ConstrainedFit(gradient, &c);
  xstarts[0] = leftedge;
  xstarts[1] = rightedge;
  coeffs[0] = 0;
  coeffs[1] = gradient;
  coeffs[2] = c;
  *baseline = QSPLINE(1, xstarts, coeffs);
  if (spline != nullptr        /*no given spline */
      && spline->segments >= 3 /*or trivial */
                               /*or too non-overlap */
      && spline->xcoords[1] <= leftedge + MAXOVERLAP * (rightedge - leftedge) &&
      spline->xcoords[spline->segments - 1] >= rightedge - MAXOVERLAP * (rightedge - leftedge)) {
    *baseline = *spline; /*copy it */
    x = (leftedge + rightedge) / 2.0;
    shift = ICOORD(0, static_cast<int16_t>(gradient * x + c - spline->y(x)));
    baseline->move(shift);
  }
}

/**********************************************************************
 * partition_line
 *
 * Partition a row of blobs into different groups of continuous
 * y position. jumplimit specifies the max allowable limit on a jump
 * before a new partition is started.
 * The return value is the biggest partition
 **********************************************************************/

int partition_line(    // partition blobs
    TBOX blobcoords[], // bounding boxes
    int blobcount,     /*no of blobs on row */
    int *numparts,     /*number of partitions */
    char partids[],    /*partition no of each blob */
    int partsizes[],   /*no in each partition */
    QSPLINE *spline,   /*curve to fit to */
    float jumplimit,   /*allowed delta change */
    float ydiffs[]     /*diff from spline */
) {
  int blobindex;             /*no along text line */
  int bestpart;              /*best new partition */
  int biggestpart;           /*part with most members */
  float diff;                /*difference from line */
  int startx;                /*index of start blob */
  float partdiffs[MAXPARTS]; /*step between parts */

  for (bestpart = 0; bestpart < MAXPARTS; bestpart++) {
    partsizes[bestpart] = 0; /*zero them all */
  }

  startx = get_ydiffs(blobcoords, blobcount, spline, ydiffs);
  *numparts = 1; /*1 partition */
  bestpart = -1; /*first point */
  float drift = 0.0f;
  float last_delta = 0.0f;
  for (blobindex = startx; blobindex < blobcount; blobindex++) {
    /*do each blob in row */
    diff = ydiffs[blobindex]; /*diff from line */
    if (textord_oldbl_debug) {
      tprintf("%d(%d,%d), ", blobindex, blobcoords[blobindex].left(),
              blobcoords[blobindex].bottom());
    }
    bestpart =
        choose_partition(diff, partdiffs, bestpart, jumplimit, &drift, &last_delta, numparts);
    /*record partition */
    partids[blobindex] = bestpart;
    partsizes[bestpart]++; /*another in it */
  }

  bestpart = -1; /*first point */
  drift = 0.0f;
  last_delta = 0.0f;
  partsizes[0]--; /*doing 1st pt again */
                  /*do each blob in row */
  for (blobindex = startx; blobindex >= 0; blobindex--) {
    diff = ydiffs[blobindex]; /*diff from line */
    if (textord_oldbl_debug) {
      tprintf("%d(%d,%d), ", blobindex, blobcoords[blobindex].left(),
              blobcoords[blobindex].bottom());
    }
    bestpart =
        choose_partition(diff, partdiffs, bestpart, jumplimit, &drift, &last_delta, numparts);
    /*record partition */
    partids[blobindex] = bestpart;
    partsizes[bestpart]++; /*another in it */
  }

  for (biggestpart = 0, bestpart = 1; bestpart < *numparts; bestpart++) {
    if (partsizes[bestpart] >= partsizes[biggestpart]) {
      biggestpart = bestpart; /*new biggest */
    }
  }
  if (textord_oldbl_merge_parts) {
    merge_oldbl_parts(blobcoords, blobcount, partids, partsizes, biggestpart, jumplimit);
  }
  return biggestpart; /*biggest partition */
}

/**********************************************************************
 * merge_oldbl_parts
 *
 * For any adjacent group of blobs in a different part, put them in the
 * main part if they fit closely to neighbours in the main part.
 **********************************************************************/

void merge_oldbl_parts( // partition blobs
    TBOX blobcoords[],  // bounding boxes
    int blobcount,      /*no of blobs on row */
    char partids[],     /*partition no of each blob */
    int partsizes[],    /*no in each partition */
    int biggestpart,    // major partition
    float jumplimit     /*allowed delta change */
) {
  bool found_one; // found a bestpart blob
  bool close_one; // found was close enough
  int blobindex;  /*no along text line */
  int prevpart;   // previous iteration
  int runlength;  // no in this part
  float diff;     /*difference from line */
  int startx;     /*index of start blob */
  int test_blob;  // another index
  FCOORD coord;   // blob coordinate
  float m, c;     // fitted line
  QLSQ stats;     // line stuff

  prevpart = biggestpart;
  runlength = 0;
  startx = 0;
  for (blobindex = 0; blobindex < blobcount; blobindex++) {
    if (partids[blobindex] != prevpart) {
      //                      tprintf("Partition change at (%d,%d) from %d to %d
      //                      after run of %d\n",
      //                              blobcoords[blobindex].left(),blobcoords[blobindex].bottom(),
      //                              prevpart,partids[blobindex],runlength);
      if (prevpart != biggestpart && runlength > MAXBADRUN) {
        stats.clear();
        for (test_blob = startx; test_blob < blobindex; test_blob++) {
          coord = FCOORD((blobcoords[test_blob].left() + blobcoords[test_blob].right()) / 2.0,
                         blobcoords[test_blob].bottom());
          stats.add(coord.x(), coord.y());
        }
        stats.fit(1);
        m = stats.get_b();
        c = stats.get_c();
        if (textord_oldbl_debug) {
          tprintf("Fitted line y=%g x + %g\n", m, c);
        }
        found_one = false;
        close_one = false;
        for (test_blob = 1;
             !found_one && (startx - test_blob >= 0 || blobindex + test_blob <= blobcount);
             test_blob++) {
          if (startx - test_blob >= 0 && partids[startx - test_blob] == biggestpart) {
            found_one = true;
            coord = FCOORD(
                (blobcoords[startx - test_blob].left() + blobcoords[startx - test_blob].right()) /
                    2.0,
                blobcoords[startx - test_blob].bottom());
            diff = m * coord.x() + c - coord.y();
            if (textord_oldbl_debug) {
              tprintf("Diff of common blob to suspect part=%g at (%g,%g)\n", diff, coord.x(),
                      coord.y());
            }
            if (diff < jumplimit && -diff < jumplimit) {
              close_one = true;
            }
          }
          if (blobindex + test_blob <= blobcount &&
              partids[blobindex + test_blob - 1] == biggestpart) {
            found_one = true;
            coord = FCOORD((blobcoords[blobindex + test_blob - 1].left() +
                            blobcoords[blobindex + test_blob - 1].right()) /
                               2.0,
                           blobcoords[blobindex + test_blob - 1].bottom());
            diff = m * coord.x() + c - coord.y();
            if (textord_oldbl_debug) {
              tprintf("Diff of common blob to suspect part=%g at (%g,%g)\n", diff, coord.x(),
                      coord.y());
            }
            if (diff < jumplimit && -diff < jumplimit) {
              close_one = true;
            }
          }
        }
        if (close_one) {
          if (textord_oldbl_debug) {
            tprintf(
                "Merged %d blobs back into part %d from %d starting at "
                "(%d,%d)\n",
                runlength, biggestpart, prevpart, blobcoords[startx].left(),
                blobcoords[startx].bottom());
          }
          // switch sides
          partsizes[prevpart] -= runlength;
          for (test_blob = startx; test_blob < blobindex; test_blob++) {
            partids[test_blob] = biggestpart;
          }
        }
      }
      prevpart = partids[blobindex];
      runlength = 1;
      startx = blobindex;
    } else {
      runlength++;
    }
  }
}

/**********************************************************************
 * get_ydiffs
 *
 * Get the differences between the blobs and the spline,
 * putting them in ydiffs.  The return value is the index
 * of the blob in the middle of the "best behaved" region
 **********************************************************************/

int get_ydiffs(        // evaluate differences
    TBOX blobcoords[], // bounding boxes
    int blobcount,     /*no of blobs */
    QSPLINE *spline,   /*approximating spline */
    float ydiffs[]     /*output */
) {
  int blobindex; /*current blob */
  int xcentre;   /*xcoord */
  int lastx;     /*last xcentre */
  float diffsum; /*sum of diffs */
  float diff;    /*current difference */
  float drift;   /*sum of spline steps */
  float bestsum; /*smallest diffsum */
  int bestindex; /*index of bestsum */

  diffsum = 0.0f;
  bestindex = 0;
  bestsum = static_cast<float>(INT32_MAX);
  drift = 0.0f;
  lastx = blobcoords[0].left();
  /*do each blob in row */
  for (blobindex = 0; blobindex < blobcount; blobindex++) {
    /*centre of blob */
    xcentre = (blobcoords[blobindex].left() + blobcoords[blobindex].right()) >> 1;
    // step functions in spline
    drift += spline->step(lastx, xcentre);
    lastx = xcentre;
    diff = blobcoords[blobindex].bottom();
    diff -= spline->y(xcentre);
    diff += drift;
    ydiffs[blobindex] = diff; /*store difference */
    if (blobindex > 2) {
      /*remove old one */
      diffsum -= ABS(ydiffs[blobindex - 3]);
    }
    diffsum += ABS(diff); /*add new one */
    if (blobindex >= 2 && diffsum < bestsum) {
      bestsum = diffsum;         /*find min sum */
      bestindex = blobindex - 1; /*middle of set */
    }
  }
  return bestindex;
}

/**********************************************************************
 * choose_partition
 *
 * Choose a partition for the point and return the index.
 **********************************************************************/

int choose_partition(                              // select partition
    float diff,                                    /*diff from spline */
    float partdiffs[],                             /*diff on all parts */
    int lastpart,                                  /*last assigned partition */
    float jumplimit,                               /*new part threshold */
    float *drift, float *lastdelta, int *partcount /*no of partitions */
) {
  int partition;   /*partition no */
  int bestpart;    /*best new partition */
  float bestdelta; /*best gap from a part */
  float delta;     /*diff from part */

  if (lastpart < 0) {
    partdiffs[0] = diff;
    lastpart = 0; /*first point */
    *drift = 0.0f;
    *lastdelta = 0.0f;
  }
  /*adjusted diff from part */
  delta = diff - partdiffs[lastpart] - *drift;
  if (textord_oldbl_debug) {
    tprintf("Diff=%.2f, Delta=%.3f, Drift=%.3f, ", diff, delta, *drift);
  }
  if (ABS(delta) > jumplimit / 2) {
    /*delta on part 0 */
    bestdelta = diff - partdiffs[0] - *drift;
    bestpart = 0; /*0 best so far */
    for (partition = 1; partition < *partcount; partition++) {
      delta = diff - partdiffs[partition] - *drift;
      if (ABS(delta) < ABS(bestdelta)) {
        bestdelta = delta;
        bestpart = partition; /*part with nearest jump */
      }
    }
    delta = bestdelta;
    /*too far away */
    if (ABS(bestdelta) > jumplimit && *partcount < MAXPARTS) { /*and spare part left */
      bestpart = (*partcount)++;                               /*best was new one */
                                                               /*start new one */
      partdiffs[bestpart] = diff - *drift;
      delta = 0.0f;
    }
  } else {
    bestpart = lastpart; /*best was last one */
  }

  if (bestpart == lastpart &&
      (ABS(delta - *lastdelta) < jumplimit / 2 || ABS(delta) < jumplimit / 2)) {
    /*smooth the drift */
    *drift = (3 * *drift + delta) / 3;
  }
  *lastdelta = delta;

  if (textord_oldbl_debug) {
    tprintf("P=%d\n", bestpart);
  }

  return bestpart;
}

/**********************************************************************
 * partition_coords
 *
 * Get the x,y coordinates of all points in the bestpart and put them
 * in xcoords,ycoords. Return the number of points found.
 **********************************************************************/

int partition_coords(  // find relevant coords
    TBOX blobcoords[], // bounding boxes
    int blobcount,     /*no of blobs in row */
    char partids[],    /*partition no of each blob */
    int bestpart,      /*best new partition */
    int xcoords[],     /*points to work on */
    int ycoords[]      /*points to work on */
) {
  int blobindex;  /*no along text line */
  int pointcount; /*no of points */

  pointcount = 0;
  for (blobindex = 0; blobindex < blobcount; blobindex++) {
    if (partids[blobindex] == bestpart) {
      /*centre of blob */
      xcoords[pointcount] = (blobcoords[blobindex].left() + blobcoords[blobindex].right()) >> 1;
      ycoords[pointcount++] = blobcoords[blobindex].bottom();
    }
  }
  return pointcount; /*no of points found */
}

/**********************************************************************
 * segment_spline
 *
 * Segment the row at midpoints between maxima and minima of the x,y pairs.
 * The xstarts of the segments are returned and the number found.
 **********************************************************************/

int segment_spline(             // make xstarts
    TBOX blobcoords[],          // boundign boxes
    int blobcount,              /*no of blobs in row */
    int xcoords[],              /*points to work on */
    int ycoords[],              /*points to work on */
    int degree, int pointcount, /*no of points */
    int xstarts[]               // result
) {
  int ptindex;                /*no along text line */
  int segment;                /*partition no */
  int lastmin, lastmax;       /*possible turn points */
  int turnpoints[SPLINESIZE]; /*good turning points */
  int turncount;              /*no of turning points */
  int max_x;                  // max specified coord

  xstarts[0] = xcoords[0] - 1; // leftmost defined pt
  max_x = xcoords[pointcount - 1] + 1;
  if (degree < 2) {
    pointcount = 0;
  }
  turncount = 0; /*no turning points yet */
  if (pointcount > 3) {
    ptindex = 1;
    lastmax = lastmin = 0; /*start with first one */
    while (ptindex < pointcount - 1 && turncount < SPLINESIZE - 1) {
      /*minimum */
      if (ycoords[ptindex - 1] > ycoords[ptindex] && ycoords[ptindex] <= ycoords[ptindex + 1]) {
        if (ycoords[ptindex] < ycoords[lastmax] - TURNLIMIT) {
          if (turncount == 0 || turnpoints[turncount - 1] != lastmax) {
            /*new max point */
            turnpoints[turncount++] = lastmax;
          }
          lastmin = ptindex; /*latest minimum */
        } else if (ycoords[ptindex] < ycoords[lastmin]) {
          lastmin = ptindex; /*lower minimum */
        }
      }

      /*maximum */
      if (ycoords[ptindex - 1] < ycoords[ptindex] && ycoords[ptindex] >= ycoords[ptindex + 1]) {
        if (ycoords[ptindex] > ycoords[lastmin] + TURNLIMIT) {
          if (turncount == 0 || turnpoints[turncount - 1] != lastmin) {
            /*new min point */
            turnpoints[turncount++] = lastmin;
          }
          lastmax = ptindex; /*latest maximum */
        } else if (ycoords[ptindex] > ycoords[lastmax]) {
          lastmax = ptindex; /*higher maximum */
        }
      }
      ptindex++;
    }
    /*possible global min */
    if (ycoords[ptindex] < ycoords[lastmax] - TURNLIMIT &&
        (turncount == 0 || turnpoints[turncount - 1] != lastmax)) {
      if (turncount < SPLINESIZE - 1) {
        /*2 more turns */
        turnpoints[turncount++] = lastmax;
      }
      if (turncount < SPLINESIZE - 1) {
        turnpoints[turncount++] = ptindex;
      }
    } else if (ycoords[ptindex] > ycoords[lastmin] + TURNLIMIT
               /*possible global max */
               && (turncount == 0 || turnpoints[turncount - 1] != lastmin)) {
      if (turncount < SPLINESIZE - 1) {
        /*2 more turns */
        turnpoints[turncount++] = lastmin;
      }
      if (turncount < SPLINESIZE - 1) {
        turnpoints[turncount++] = ptindex;
      }
    } else if (turncount > 0 && turnpoints[turncount - 1] == lastmin &&
               turncount < SPLINESIZE - 1) {
      if (ycoords[ptindex] > ycoords[lastmax]) {
        turnpoints[turncount++] = ptindex;
      } else {
        turnpoints[turncount++] = lastmax;
      }
    } else if (turncount > 0 && turnpoints[turncount - 1] == lastmax &&
               turncount < SPLINESIZE - 1) {
      if (ycoords[ptindex] < ycoords[lastmin]) {
        turnpoints[turncount++] = ptindex;
      } else {
        turnpoints[turncount++] = lastmin;
      }
    }
  }

  if (textord_oldbl_debug && turncount > 0) {
    tprintf("First turn is %d at (%d,%d)\n", turnpoints[0], xcoords[turnpoints[0]],
            ycoords[turnpoints[0]]);
  }
  for (segment = 1; segment < turncount; segment++) {
    /*centre y coord */
    lastmax = (ycoords[turnpoints[segment - 1]] + ycoords[turnpoints[segment]]) / 2;

    /* fix alg so that it works with both rising and falling sections */
    if (ycoords[turnpoints[segment - 1]] < ycoords[turnpoints[segment]]) {
      /*find rising y centre */
      for (ptindex = turnpoints[segment - 1] + 1;
           ptindex < turnpoints[segment] && ycoords[ptindex + 1] <= lastmax; ptindex++) {
      }
    } else {
      /*find falling y centre */
      for (ptindex = turnpoints[segment - 1] + 1;
           ptindex < turnpoints[segment] && ycoords[ptindex + 1] >= lastmax; ptindex++) {
      }
    }

    /*centre x */
    xstarts[segment] = (xcoords[ptindex - 1] + xcoords[ptindex] + xcoords[turnpoints[segment - 1]] +
                        xcoords[turnpoints[segment]] + 2) /
                       4;
    /*halfway between turns */
    if (textord_oldbl_debug) {
      tprintf("Turn %d is %d at (%d,%d), mid pt is %d@%d, final @%d\n", segment,
              turnpoints[segment], xcoords[turnpoints[segment]], ycoords[turnpoints[segment]],
              ptindex - 1, xcoords[ptindex - 1], xstarts[segment]);
    }
  }

  xstarts[segment] = max_x;
  return segment; /*no of splines */
}

/**********************************************************************
 * split_stepped_spline
 *
 * Re-segment the spline in cases where there is a big step function.
 * Return true if any were done.
 **********************************************************************/

bool split_stepped_spline( // make xstarts
    QSPLINE *baseline,     // current shot
    float jumplimit,       // max step function
    int *xcoords,          /*points to work on */
    int *xstarts,          // result
    int &segments          // no of segments
) {
  bool doneany; // return value
  int segment;  /*partition no */
  int startindex, centreindex, endindex;
  float leftcoord, rightcoord;
  int leftindex, rightindex;
  float step; // spline step

  doneany = false;
  startindex = 0;
  for (segment = 1; segment < segments - 1; segment++) {
    step = baseline->step((xstarts[segment - 1] + xstarts[segment]) / 2.0,
                          (xstarts[segment] + xstarts[segment + 1]) / 2.0);
    if (step < 0) {
      step = -step;
    }
    if (step > jumplimit) {
      while (xcoords[startindex] < xstarts[segment - 1]) {
        startindex++;
      }
      centreindex = startindex;
      while (xcoords[centreindex] < xstarts[segment]) {
        centreindex++;
      }
      endindex = centreindex;
      while (xcoords[endindex] < xstarts[segment + 1]) {
        endindex++;
      }
      if (segments >= SPLINESIZE) {
        if (textord_debug_baselines) {
          tprintf("Too many segments to resegment spline!!\n");
        }
      } else if (endindex - startindex >= textord_spline_medianwin * 3) {
        while (centreindex - startindex < textord_spline_medianwin * 3 / 2) {
          centreindex++;
        }
        while (endindex - centreindex < textord_spline_medianwin * 3 / 2) {
          centreindex--;
        }
        leftindex = (startindex + startindex + centreindex) / 3;
        rightindex = (centreindex + endindex + endindex) / 3;
        leftcoord = (xcoords[startindex] * 2 + xcoords[centreindex]) / 3.0;
        rightcoord = (xcoords[centreindex] + xcoords[endindex] * 2) / 3.0;
        while (xcoords[leftindex] > leftcoord &&
               leftindex - startindex > textord_spline_medianwin) {
          leftindex--;
        }
        while (xcoords[leftindex] < leftcoord &&
               centreindex - leftindex > textord_spline_medianwin / 2) {
          leftindex++;
        }
        if (xcoords[leftindex] - leftcoord > leftcoord - xcoords[leftindex - 1]) {
          leftindex--;
        }
        while (xcoords[rightindex] > rightcoord &&
               rightindex - centreindex > textord_spline_medianwin / 2) {
          rightindex--;
        }
        while (xcoords[rightindex] < rightcoord &&
               endindex - rightindex > textord_spline_medianwin) {
          rightindex++;
        }
        if (xcoords[rightindex] - rightcoord > rightcoord - xcoords[rightindex - 1]) {
          rightindex--;
        }
        if (textord_debug_baselines) {
          tprintf("Splitting spline at %d with step %g at (%d,%d)\n", xstarts[segment],
                  baseline->step((xstarts[segment - 1] + xstarts[segment]) / 2.0,
                                 (xstarts[segment] + xstarts[segment + 1]) / 2.0),
                  (xcoords[leftindex - 1] + xcoords[leftindex]) / 2,
                  (xcoords[rightindex - 1] + xcoords[rightindex]) / 2);
        }
        insert_spline_point(xstarts, segment, (xcoords[leftindex - 1] + xcoords[leftindex]) / 2,
                            (xcoords[rightindex - 1] + xcoords[rightindex]) / 2, segments);
        doneany = true;
      } else if (textord_debug_baselines) {
        tprintf("Resegmenting spline failed - insufficient pts (%d,%d,%d,%d)\n", startindex,
                centreindex, endindex, (int32_t)textord_spline_medianwin);
      }
    }
    //              else tprintf("Spline step at %d is %g\n",
    //                      xstarts[segment],
    //                      baseline->step((xstarts[segment-1]+xstarts[segment])/2.0,
    //                      (xstarts[segment]+xstarts[segment+1])/2.0));
  }
  return doneany;
}

/**********************************************************************
 * insert_spline_point
 *
 * Insert a new spline point and shuffle up the others.
 **********************************************************************/

void insert_spline_point(     // get descenders
    int xstarts[],            // starts to shuffle
    int segment,              // insertion pt
    int coord1,               // coords to add
    int coord2, int &segments // total segments
) {
  int index; // for shuffling

  for (index = segments; index > segment; index--) {
    xstarts[index + 1] = xstarts[index];
  }
  segments++;
  xstarts[segment] = coord1;
  xstarts[segment + 1] = coord2;
}

/**********************************************************************
 * find_lesser_parts
 *
 * Average the step from the spline for the other partitions
 * and find the commonest partition which has a descender.
 **********************************************************************/

void find_lesser_parts( // get descenders
    TO_ROW *row,        // row to process
    TBOX blobcoords[],  // bounding boxes
    int blobcount,      /*no of blobs */
    char partids[],     /*partition of each blob */
    int partsizes[],    /*size of each part */
    int partcount,      /*no of partitions */
    int bestpart        /*biggest partition */
) {
  int blobindex;             /*index of blob */
  int partition;             /*current partition */
  int xcentre;               /*centre of blob */
  int poscount;              /*count of best up step */
  int negcount;              /*count of best down step */
  float partsteps[MAXPARTS]; /*average step to part */
  float bestneg;             /*best down step */
  int runlength;             /*length of bad run */
  int biggestrun;            /*biggest bad run */

  biggestrun = 0;
  for (partition = 0; partition < partcount; partition++) {
    partsteps[partition] = 0.0; /*zero accumulators */
  }
  for (runlength = 0, blobindex = 0; blobindex < blobcount; blobindex++) {
    xcentre = (blobcoords[blobindex].left() + blobcoords[blobindex].right()) >> 1;
    /*in other parts */
    int part_id = static_cast<int>(static_cast<unsigned char>(partids[blobindex]));
    if (part_id != bestpart) {
      runlength++; /*run of non bests */
      if (runlength > biggestrun) {
        biggestrun = runlength;
      }
      partsteps[part_id] += blobcoords[blobindex].bottom() - row->baseline.y(xcentre);
    } else {
      runlength = 0;
    }
  }
  if (biggestrun > MAXBADRUN) {
    row->xheight = -1.0f; /*failed */
  } else {
    row->xheight = 1.0f; /*success */
  }
  poscount = negcount = 0;
  bestneg = 0.0; /*no step yet */
  for (partition = 0; partition < partcount; partition++) {
    if (partition != bestpart) {
      // by jetsoft divide by zero possible
      if (partsizes[partition] == 0) {
        partsteps[partition] = 0;
      } else {
        partsteps[partition] /= partsizes[partition];
      }
      //

      if (partsteps[partition] >= MINASCRISE && partsizes[partition] > poscount) {
        poscount = partsizes[partition];
      }
      if (partsteps[partition] <= -MINASCRISE && partsizes[partition] > negcount) {
        /*ascender rise */
        bestneg = partsteps[partition];
        /*2nd most popular */
        negcount = partsizes[partition];
      }
    }
  }
  /*average x-height */
  partsteps[bestpart] /= blobcount;
  row->descdrop = bestneg;
}

/**********************************************************************
 * old_first_xheight
 *
 * Makes an x-height spline by copying the baseline and shifting it.
 * It estimates the x-height across the line to use as the shift.
 * It also finds the ascender height if it can.
 **********************************************************************/

void old_first_xheight( // the wiseowl way
    TO_ROW *row,        /*current row */
    TBOX blobcoords[],  /*blob bounding boxes */
    int initialheight,  // initial guess
    int blobcount,      /*blobs in blobcoords */
    QSPLINE *baseline,  /*established */
    float jumplimit     /*min ascender height */
) {
  int blobindex; /*current blob */
                 /*height statistics */
  STATS heightstat(0, MAXHEIGHT - 1);
  int height;      /*height of blob */
  int xcentre;     /*centre of blob */
  int lineheight;  /*approx xheight */
  float ascenders; /*ascender sum */
  int asccount;    /*no of ascenders */
  float xsum;      /*xheight sum */
  int xcount;      /*xheight count */
  float diff;      /*height difference */

  if (blobcount > 1) {
    for (blobindex = 0; blobindex < blobcount; blobindex++) {
      xcentre = (blobcoords[blobindex].left() + blobcoords[blobindex].right()) / 2;
      /*height of blob */
      height = static_cast<int>(blobcoords[blobindex].top() - baseline->y(xcentre) + 0.5);
      if (height > initialheight * oldbl_xhfract && height > textord_min_xheight) {
        heightstat.add(height, 1);
      }
    }
    if (heightstat.get_total() > 3) {
      lineheight = static_cast<int>(heightstat.ile(0.25));
      if (lineheight <= 0) {
        lineheight = static_cast<int>(heightstat.ile(0.5));
      }
    } else {
      lineheight = initialheight;
    }
  } else {
    lineheight =
        static_cast<int>(blobcoords[0].top() -
                         baseline->y((blobcoords[0].left() + blobcoords[0].right()) / 2) + 0.5);
  }

  xsum = 0.0f;
  xcount = 0;
  for (ascenders = 0.0f, asccount = 0, blobindex = 0; blobindex < blobcount; blobindex++) {
    xcentre = (blobcoords[blobindex].left() + blobcoords[blobindex].right()) / 2;
    diff = blobcoords[blobindex].top() - baseline->y(xcentre);
    /*is it ascender */
    if (diff > lineheight + jumplimit) {
      ascenders += diff;
      asccount++; /*count ascenders */
    } else if (diff > lineheight - jumplimit) {
      xsum += diff; /*mean xheight */
      xcount++;
    }
  }
  if (xcount > 0) {
    xsum /= xcount; /*average xheight */
  } else {
    xsum = static_cast<float>(lineheight); /*guess it */
  }
  row->xheight *= xsum;
  if (asccount > 0) {
    row->ascrise = ascenders / asccount - xsum;
  } else {
    row->ascrise = 0.0f; /*had none */
  }
  if (row->xheight == 0) {
    row->xheight = -1.0f;
  }
}

/**********************************************************************
 * make_first_xheight
 *
 * Makes an x-height spline by copying the baseline and shifting it.
 * It estimates the x-height across the line to use as the shift.
 * It also finds the ascender height if it can.
 **********************************************************************/

void make_first_xheight( // find xheight
    TO_ROW *row,         /*current row */
    TBOX blobcoords[],   /*blob bounding boxes */
    int lineheight,      // initial guess
    int init_lineheight, // block level guess
    int blobcount,       /*blobs in blobcoords */
    QSPLINE *baseline,   /*established */
    float jumplimit      /*min ascender height */
) {
  STATS heightstat(0, HEIGHTBUCKETS - 1);
  int lefts[HEIGHTBUCKETS];
  int rights[HEIGHTBUCKETS];
  int modelist[MODENUM];
  int blobindex;
  int mode_count; // blobs to count in thr
  int sign_bit;
  int mode_threshold;
  const int kBaselineTouch = 2;  // This really should change with resolution.
  const int kGoodStrength = 8;   // Strength of baseline-touching heights.
  const float kMinHeight = 0.25; // Min fraction of lineheight to use.

  sign_bit = row->xheight > 0 ? 1 : -1;

  memset(lefts, 0, HEIGHTBUCKETS * sizeof(lefts[0]));
  memset(rights, 0, HEIGHTBUCKETS * sizeof(rights[0]));
  mode_count = 0;
  for (blobindex = 0; blobindex < blobcount; blobindex++) {
    int xcenter = (blobcoords[blobindex].left() + blobcoords[blobindex].right()) / 2;
    float base = baseline->y(xcenter);
    float bottomdiff = std::fabs(base - blobcoords[blobindex].bottom());
    int strength = textord_ocropus_mode && bottomdiff <= kBaselineTouch ? kGoodStrength : 1;
    int height = static_cast<int>(blobcoords[blobindex].top() - base + 0.5);
    if (blobcoords[blobindex].height() > init_lineheight * kMinHeight) {
      if (height > lineheight * oldbl_xhfract && height > textord_min_xheight) {
        heightstat.add(height, strength);
        if (height < HEIGHTBUCKETS) {
          if (xcenter > rights[height]) {
            rights[height] = xcenter;
          }
          if (xcenter > 0 && (lefts[height] == 0 || xcenter < lefts[height])) {
            lefts[height] = xcenter;
          }
        }
      }
      mode_count += strength;
    }
  }

  mode_threshold = static_cast<int>(blobcount * 0.1);
  if (oldbl_dot_error_size > 1 || oldbl_xhfix) {
    mode_threshold = static_cast<int>(mode_count * 0.1);
  }

  if (textord_oldbl_debug) {
    tprintf("blobcount=%d, mode_count=%d, mode_t=%d\n", blobcount, mode_count, mode_threshold);
  }
  find_top_modes(&heightstat, HEIGHTBUCKETS, modelist, MODENUM);
  if (textord_oldbl_debug) {
    for (blobindex = 0; blobindex < MODENUM; blobindex++) {
      tprintf("mode[%d]=%d ", blobindex, modelist[blobindex]);
    }
    tprintf("\n");
  }
  pick_x_height(row, modelist, lefts, rights, &heightstat, mode_threshold);

  if (textord_oldbl_debug) {
    tprintf("Output xheight=%g\n", row->xheight);
  }
  if (row->xheight < 0 && textord_oldbl_debug) {
    tprintf("warning: Row Line height < 0; %4.2f\n", row->xheight);
  }

  if (sign_bit < 0) {
    row->xheight = -row->xheight;
  }
}

/**********************************************************************
 * find_top_modes
 *
 * Fill the input array with the indices of the top ten modes of the
 * input distribution.
 **********************************************************************/

const int kMinModeFactorOcropus = 32;
const int kMinModeFactor = 12;

void find_top_modes(            // get modes
    STATS *stats,               // stats to hack
    int statnum,                // no of piles
    int modelist[], int modenum // no of modes to get
) {
  int mode_count;
  int last_i = 0;
  int last_max = INT32_MAX;
  int i;
  int mode;
  int total_max = 0;
  int mode_factor = textord_ocropus_mode ? kMinModeFactorOcropus : kMinModeFactor;

  for (mode_count = 0; mode_count < modenum; mode_count++) {
    mode = 0;
    for (i = 0; i < statnum; i++) {
      if (stats->pile_count(i) > stats->pile_count(mode)) {
        if ((stats->pile_count(i) < last_max) ||
            ((stats->pile_count(i) == last_max) && (i > last_i))) {
          mode = i;
        }
      }
    }
    last_i = mode;
    last_max = stats->pile_count(last_i);
    total_max += last_max;
    if (last_max <= total_max / mode_factor) {
      mode = 0;
    }
    modelist[mode_count] = mode;
  }
}

/**********************************************************************
 * pick_x_height
 *
 * Choose based on the height modes the best x height value.
 **********************************************************************/

void pick_x_height(TO_ROW *row, // row to do
                   int modelist[], int lefts[], int rights[], STATS *heightstat,
                   int mode_threshold) {
  int x;
  int y;
  int z;
  float ratio;
  int found_one_bigger = false;
  int best_x_height = 0;
  int best_asc = 0;
  int num_in_best;

  for (x = 0; x < MODENUM; x++) {
    for (y = 0; y < MODENUM; y++) {
      /* Check for two modes */
      if (modelist[x] && modelist[y] && heightstat->pile_count(modelist[x]) > mode_threshold &&
          (!textord_ocropus_mode || std::min(rights[modelist[x]], rights[modelist[y]]) >
                                        std::max(lefts[modelist[x]], lefts[modelist[y]]))) {
        ratio = static_cast<float>(modelist[y]) / static_cast<float>(modelist[x]);
        if (1.2 < ratio && ratio < 1.8) {
          /* Two modes found */
          best_x_height = modelist[x];
          num_in_best = heightstat->pile_count(modelist[x]);

          /* Try to get one higher */
          do {
            found_one_bigger = false;
            for (z = 0; z < MODENUM; z++) {
              if (modelist[z] == best_x_height + 1 &&
                  (!textord_ocropus_mode || std::min(rights[modelist[x]], rights[modelist[y]]) >
                                                std::max(lefts[modelist[x]], lefts[modelist[y]]))) {
                ratio = static_cast<float>(modelist[y]) / static_cast<float>(modelist[z]);
                if ((1.2 < ratio && ratio < 1.8) &&
                    /* Should be half of best */
                    heightstat->pile_count(modelist[z]) > num_in_best * 0.5) {
                  best_x_height++;
                  found_one_bigger = true;
                  break;
                }
              }
            }
          } while (found_one_bigger);

          /* try to get a higher ascender */

          best_asc = modelist[y];
          num_in_best = heightstat->pile_count(modelist[y]);

          /* Try to get one higher */
          do {
            found_one_bigger = false;
            for (z = 0; z < MODENUM; z++) {
              if (modelist[z] > best_asc &&
                  (!textord_ocropus_mode || std::min(rights[modelist[x]], rights[modelist[y]]) >
                                                std::max(lefts[modelist[x]], lefts[modelist[y]]))) {
                ratio = static_cast<float>(modelist[z]) / static_cast<float>(best_x_height);
                if ((1.2 < ratio && ratio < 1.8) &&
                    /* Should be half of best */
                    heightstat->pile_count(modelist[z]) > num_in_best * 0.5) {
                  best_asc = modelist[z];
                  found_one_bigger = true;
                  break;
                }
              }
            }
          } while (found_one_bigger);

          row->xheight = static_cast<float>(best_x_height);
          row->ascrise = static_cast<float>(best_asc) - best_x_height;
          return;
        }
      }
    }
  }

  best_x_height = modelist[0]; /* Single Mode found */
  num_in_best = heightstat->pile_count(best_x_height);
  do {
    /* Try to get one higher */
    found_one_bigger = false;
    for (z = 1; z < MODENUM; z++) {
      /* Should be half of best */
      if ((modelist[z] == best_x_height + 1) &&
          (heightstat->pile_count(modelist[z]) > num_in_best * 0.5)) {
        best_x_height++;
        found_one_bigger = true;
        break;
      }
    }
  } while (found_one_bigger);

  row->ascrise = 0.0f;
  row->xheight = static_cast<float>(best_x_height);
  if (row->xheight == 0) {
    row->xheight = -1.0f;
  }
}

} // namespace tesseract
