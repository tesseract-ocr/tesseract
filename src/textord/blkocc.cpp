/*****************************************************************************
 *
 * File:         blkocc.cpp  (Formerly blockocc.c)
 * Description:  Block Occupancy routines
 * Author:       Chris Newton
 *
 * (c) Copyright 1991, Hewlett-Packard Company.
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
 ******************************************************************************/

#include "blkocc.h"

#include "drawtord.h"
#include "errcode.h"

#include <cctype>
#include <cmath>

#include "helpers.h"

namespace tesseract {

double_VAR(textord_underline_threshold, 0.5, "Fraction of width occupied");

// Forward declarations of static functions
static void horizontal_cblob_projection(C_BLOB *blob,  // blob to project
                                        STATS *stats); // output
static void horizontal_coutline_projection(C_OUTLINE *outline,
                                           STATS *stats); // output

/**
 * test_underline
 *
 * Check to see if the blob is an underline.
 * Return true if it is.
 */

bool test_underline(  // look for underlines
    bool testing_on,  ///< drawing blob
    C_BLOB *blob,     ///< blob to test
    int16_t baseline, ///< coords of baseline
    int16_t xheight   ///< height of line
) {
  TDimension occ;
  STATS projection;

  auto blob_box = blob->bounding_box();
  auto blob_width = blob->bounding_box().width();
  projection.set_range(blob_box.bottom(), blob_box.top());
  if (testing_on) {
    //              blob->plot(to_win,GOLDENROD,GOLDENROD);
    //              line_color_index(to_win,GOLDENROD);
    //              move2d(to_win,blob_box.left(),baseline);
    //              draw2d(to_win,blob_box.right(),baseline);
    //              move2d(to_win,blob_box.left(),baseline+xheight);
    //              draw2d(to_win,blob_box.right(),baseline+xheight);
    tprintf("Testing underline on blob at (%d,%d)->(%d,%d), base=%d\nOccs:",
            blob->bounding_box().left(), blob->bounding_box().bottom(),
            blob->bounding_box().right(), blob->bounding_box().top(), baseline);
  }
  horizontal_cblob_projection(blob, &projection);
  int32_t desc_occ = 0;
  for (occ = blob_box.bottom(); occ < baseline; occ++) {
    if (occ <= blob_box.top() && projection.pile_count(occ) > desc_occ) {
      // max in region
      desc_occ = projection.pile_count(occ);
    }
  }
  int32_t x_occ = 0;
  for (occ = baseline; occ <= baseline + xheight; occ++) {
    if (occ >= blob_box.bottom() && occ <= blob_box.top() && projection.pile_count(occ) > x_occ) {
      // max in region
      x_occ = projection.pile_count(occ);
    }
  }
  int32_t asc_occ = 0;
  for (occ = baseline + xheight + 1; occ <= blob_box.top(); occ++) {
    if (occ >= blob_box.bottom() && projection.pile_count(occ) > asc_occ) {
      asc_occ = projection.pile_count(occ);
    }
  }
  if (testing_on) {
    tprintf("%d %d %d\n", desc_occ, x_occ, asc_occ);
  }
  if (desc_occ == 0 && x_occ == 0 && asc_occ == 0) {
    tprintf("Bottom=%d, top=%d, base=%d, x=%d\n", blob_box.bottom(), blob_box.top(), baseline,
            xheight);
    projection.print();
  }
  if (desc_occ > x_occ + x_occ && desc_occ > blob_width * textord_underline_threshold) {
    return true; // real underline
  }
  return asc_occ > x_occ + x_occ && asc_occ > blob_width * textord_underline_threshold; // overline
                                                                                        // neither
}

/**
 * horizontal_cblob_projection
 *
 * Compute the horizontal projection of a cblob from its outlines
 * and add to the given STATS.
 */

static void horizontal_cblob_projection( // project outlines
    C_BLOB *blob,                        ///< blob to project
    STATS *stats                         ///< output
) {
  // outlines of blob
  C_OUTLINE_IT out_it = blob->out_list();

  for (out_it.mark_cycle_pt(); !out_it.cycled_list(); out_it.forward()) {
    horizontal_coutline_projection(out_it.data(), stats);
  }
}

/**
 * horizontal_coutline_projection
 *
 * Compute the horizontal projection of a outline from its outlines
 * and add to the given STATS.
 */

static void horizontal_coutline_projection( // project outlines
    C_OUTLINE *outline,                     ///< outline to project
    STATS *stats                            ///< output
) {
  ICOORD pos;        // current point
  ICOORD step;       // edge step
  int32_t length;    // of outline
  int16_t stepindex; // current step
  C_OUTLINE_IT out_it = outline->child();

  pos = outline->start_pos();
  length = outline->pathlength();
  for (stepindex = 0; stepindex < length; stepindex++) {
    step = outline->step(stepindex);
    if (step.y() > 0) {
      stats->add(pos.y(), pos.x());
    } else if (step.y() < 0) {
      stats->add(pos.y() - 1, -pos.x());
    }
    pos += step;
  }

  for (out_it.mark_cycle_pt(); !out_it.cycled_list(); out_it.forward()) {
    horizontal_coutline_projection(out_it.data(), stats);
  }
}

} // namespace tesseract
