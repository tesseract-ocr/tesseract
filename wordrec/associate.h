///////////////////////////////////////////////////////////////////////
// File:        associate.h
// Description: Structs, classes, typedefs useful for the segmentation
//              search. Functions for scoring segmentation paths according
//              to their character widths, gap widths and seam cuts.
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

#ifndef ASSOCIATE_H
#define ASSOCIATE_H

#include "blobs.h"
#include "elst.h"
#include "ratngs.h"
#include "seam.h"
#include "split.h"

class WERD_RES;

namespace tesseract {

// Statisitcs about character widths, gaps and seams.
struct AssociateStats {
  AssociateStats() { Clear(); }

  void Clear() {
    shape_cost = 0.0f;
    bad_shape = false;
    full_wh_ratio = 0.0f;
    full_wh_ratio_total = 0.0f;
    full_wh_ratio_var = 0.0f;
    bad_fixed_pitch_right_gap = false;
    bad_fixed_pitch_wh_ratio = false;
    gap_sum = 0;
  }

  void Print() { tprintf("AssociateStats: s(%g %d)\n", shape_cost, bad_shape); }

  float shape_cost;           // cost of blob shape
  bool bad_shape;             // true if the shape of the blob is unacceptable
  float full_wh_ratio;        // width-to-hight ratio + gap on the right
  float full_wh_ratio_total;  // sum of width-to-hight ratios
                              // on the path terminating at this blob
  float full_wh_ratio_var;    // variance of full_wh_ratios on the path
  bool bad_fixed_pitch_right_gap;  // true if there is no gap before
                                   // the blob on the right
  bool bad_fixed_pitch_wh_ratio;   // true if the blobs has width-to-hight
                                   // ratio > kMaxFixedPitchCharAspectRatio
  int gap_sum;  // sum of gaps within the blob
};

// Utility functions for scoring segmentation paths according to their
// character widths, gap widths, seam characteristics.
class AssociateUtils {
 public:
  static const float kMaxFixedPitchCharAspectRatio;
  static const float kMinGap;

  // Returns outline length of the given blob is computed as:
  // rating_cert_scale * rating / certainty
  // Since from Wordrec::SegSearch() in segsearch.cpp
  // rating_cert_scale = -1.0 * getDict().certainty_scale / rating_scale
  // And from Classify::ConvertMatchesToChoices() in adaptmatch.cpp
  // Rating = Certainty = next.rating
  // Rating *= rating_scale * Results->BlobLength
  // Certainty *= -(getDict().certainty_scale)
  static inline float ComputeOutlineLength(float rating_cert_scale,
                                           const BLOB_CHOICE &b) {
    return rating_cert_scale * b.rating() / b.certainty();
  }
  static inline float ComputeRating(float rating_cert_scale,
                                    float cert, int width) {
    return static_cast<float>(width) * cert / rating_cert_scale;
  }

  // Computes character widths, gaps and seams stats given the
  // AssociateStats of the path so far, col, row of the blob that
  // is being added to the path, and WERD_RES containing information
  // about character widths, gaps and seams.
  // Fills associate_cost with the combined shape, gap and seam cost
  // of adding a unichar from (col, row) to the path (note that since
  // this function could be used to compute the prioritization for
  // pain points, (col, row) entry might not be classified yet; thus
  // information in the (col, row) entry of the ratings matrix is not used).
  //
  // Note: the function assumes that word_res, stats and
  // associate_cost pointers are not NULL.
  static void ComputeStats(int col, int row,
                           const AssociateStats *parent_stats,
                           int parent_path_length,
                           bool fixed_pitch,
                           float max_char_wh_ratio,
                           WERD_RES *word_res,
                           bool debug,
                           AssociateStats *stats);

  // Returns the width cost for fixed-pitch text.
  static float FixedPitchWidthCost(float norm_width, float right_gap,
                                   bool end_pos, float max_char_wh_ratio);

  // Returns the gap cost for fixed-pitch text (penalizes vertically
  // overlapping components).
  static inline float FixedPitchGapCost(float norm_gap, bool end_pos) {
    return (norm_gap < 0.05 && !end_pos) ? 5.0f : 0.0f;
  }
};

}  // namespace tesseract

#endif
