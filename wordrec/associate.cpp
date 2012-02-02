///////////////////////////////////////////////////////////////////////
// File:        associate.cpp
// Description: Functions for scoring segmentation paths according to
//              their character widths, gap widths and seam cuts.
// Author:      Daria Antonova
// Created:     Mon Mar 8 11:26:43 PDT 2010
//
// (C) Copyright 2010, Google Inc.
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
///////////////////////////////////////////////////////////////////////


#include <stdio.h>
#ifdef __UNIX__
#include <assert.h>
#endif
#include <math.h>

#include "associate.h"
#include "baseline.h"
#include "normalis.h"

namespace tesseract {

const float AssociateUtils::kMaxFixedPitchCharAspectRatio = 2.0f;
const float AssociateUtils::kMinGap = 0.03f;

void AssociateUtils::ComputeStats(int col, int row,
                                  const AssociateStats *parent_stats,
                                  int parent_path_length,
                                  bool fixed_pitch,
                                  float max_char_wh_ratio,
                                  const DENORM *denorm,
                                  CHUNKS_RECORD *chunks_record,
                                  int debug_level,
                                  AssociateStats *stats) {
  stats->Clear();

  if (debug_level > 0) {
    tprintf("AssociateUtils::ComputeStats() for col=%d, row=%d%s\n",
            col, row, fixed_pitch ? " (fixed pitch)" : "");
  }
  float normalizing_height = BASELINE_SCALE;
  // TODO(rays/daria) Can unicharset.script_has_xheight be useful here?
  if (fixed_pitch && denorm != NULL && denorm->row() != NULL) {
    // For fixed pitch language like CJK, we use the full text height
    // as the normalizing factor so we are not dependent on xheight
    // calculation.
    if (denorm->row()->body_size() > 0.0f) {
      normalizing_height = denorm->y_scale() * denorm->row()->body_size();
    } else {
      normalizing_height = denorm->y_scale() *
          (denorm->row()->x_height() + denorm->row()->ascenders());
    }
    if (debug_level > 0) {
      tprintf("normalizing height = %g (scale %g xheight %g ascenders %g)\n",
              normalizing_height, denorm->y_scale(), denorm->row()->x_height(),
              denorm->row()->ascenders());
    }
  }
  float wh_ratio =
    GetChunksWidth(chunks_record->chunk_widths, col, row) / normalizing_height;
  if (debug_level) tprintf("wh_ratio %g\n", wh_ratio);
  if (wh_ratio > max_char_wh_ratio) stats->bad_shape = true;
  if (fixed_pitch) {
    bool end_row = (row == (chunks_record->ratings->dimension() - 1));

    // Ensure that the blob has gaps on the left and the right sides
    // (except for beginning and ending punctuation) and that there is
    // no cutting through ink at the blob boundaries.
    if (col > 0) {
      float left_gap =
        GetChunksGap(chunks_record->chunk_widths, col-1) / normalizing_height;
      SEAM *left_seam =
        static_cast<SEAM *>(array_value(chunks_record->splits, col-1));
      if (debug_level) {
        tprintf("left_gap %g, left_seam %g\n", left_gap, left_seam->priority);
      }
      if ((!end_row && left_gap < kMinGap) || left_seam->priority > 0.0f) {
        stats->bad_shape = true;
      }
    }
    float right_gap = 0.0f;
    if (!end_row) {
      right_gap =
        GetChunksGap(chunks_record->chunk_widths, row) / normalizing_height;
      SEAM *right_seam =
        static_cast<SEAM *>(array_value(chunks_record->splits, row));
      if (debug_level) {
        tprintf("right_gap %g right_seam %g\n",
                right_gap, right_seam->priority);
      }
      if (right_gap < kMinGap || right_seam->priority > 0.0f) {
        stats->bad_shape = true;
        if (right_gap < kMinGap) stats->bad_fixed_pitch_right_gap = true;
      }
    }

    // Impose additional segmentation penalties if blob widths or gaps
    // distribution don't fit a fixed-pitch model.
    // Since we only know the widths and gaps of the path explored so far,
    // the means and variances are computed for the path so far (not
    // considering characters to the right of the last character on the path).
    stats->full_wh_ratio = wh_ratio + right_gap;
    if (parent_stats != NULL) {
      stats->full_wh_ratio_total =
        (parent_stats->full_wh_ratio_total + stats->full_wh_ratio);
      float mean =
        stats->full_wh_ratio_total / static_cast<float>(parent_path_length+1);
      stats->full_wh_ratio_var =
        parent_stats->full_wh_ratio_var + pow(mean-stats->full_wh_ratio, 2);
    } else {
      stats->full_wh_ratio_total = stats->full_wh_ratio;
    }
    if (debug_level) {
      tprintf("full_wh_ratio %g full_wh_ratio_total %g full_wh_ratio_var %g\n",
              stats->full_wh_ratio, stats->full_wh_ratio_total,
              stats->full_wh_ratio_var);
    }

    stats->shape_cost =
      FixedPitchWidthCost(wh_ratio, right_gap, end_row, max_char_wh_ratio);

    // For some reason Tesseract prefers to treat the whole CJ words
    // as one blob when the initial segmentation is particularly bad.
    // This hack is to avoid favoring such states.
    if (col == 0 && end_row && wh_ratio > max_char_wh_ratio) {
      stats->shape_cost += 10;
    }
    stats->shape_cost += stats->full_wh_ratio_var;
    if (debug_level) tprintf("shape_cost %g\n", stats->shape_cost);
  }
}

int AssociateUtils::GetChunksWidth(WIDTH_RECORD *width_record,
                                   int start_blob, int last_blob) {
  int result = 0;
  for (int x = start_blob * 2; x <= last_blob * 2; x++)
    result += width_record->widths[x];
  return result;
}

float AssociateUtils::FixedPitchWidthCost(float norm_width,
                                          float right_gap,
                                          bool end_pos,
                                          float max_char_wh_ratio) {
  float cost = 0.0f;
  if (norm_width > max_char_wh_ratio) cost += norm_width;
  if (norm_width > kMaxFixedPitchCharAspectRatio)
    cost += norm_width * norm_width;  // extra penalty for merging CJK chars
  // Penalize skinny blobs, except for punctuation in the last position.
  if (norm_width+right_gap < 0.5f && !end_pos) {
    cost += 1.0f - (norm_width + right_gap);
  }
  return cost;
}

}  // namespace tesseract
