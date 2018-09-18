/**********************************************************************
 * File:        fixxht.cpp  (Formerly fixxht.c)
 * Description: Improve x_ht and look out for case inconsistencies
 * Author:      Phil Cheatle
 * Created:     Thu Aug  5 14:11:08 BST 1993
 *
 * (C) Copyright 1992, Hewlett-Packard Ltd.
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

#include <algorithm>
#include <cstring>
#include <cctype>
#include "params.h"
#include "float2int.h"
#include "tesseractclass.h"

namespace tesseract {

// Fixxht overview.
// Premise: Initial estimate of x-height is adequate most of the time, but
// occasionally it is incorrect. Most notable causes of failure are:
// 1. Small caps, where the top of the caps is the same as the body text
// xheight. For small caps words the xheight needs to be reduced to correctly
// recognize the caps in the small caps word.
// 2. All xheight lines, such as summer. Here the initial estimate will have
// guessed that the blob tops are caps and will have placed the xheight too low.
// 3. Noise/logos beside words, or changes in font size on a line. Such
// things can blow the statistics and cause an incorrect estimate.
// 4. Incorrect baseline. Can happen when 2 columns are incorrectly merged.
// In this case the x-height is often still correct.
//
// Algorithm.
// Compare the vertical position (top only) of alphnumerics in a word with
// the range of positions in training data (in the unicharset).
// See CountMisfitTops. If any characters disagree sufficiently with the
// initial xheight estimate, then recalculate the xheight, re-run OCR on
// the word, and if the number of vertical misfits goes down, along with
// either the word rating or certainty, then keep the new xheight.
// The new xheight is calculated as follows:ComputeCompatibleXHeight
// For each alphanumeric character that has a vertically misplaced top
// (a misfit), yet its bottom is within the acceptable range (ie it is not
// likely a sub-or super-script) calculate the range of acceptable xheight
// positions from its range of tops, and give each value in the range a
// number of votes equal to the distance of its top from its acceptance range.
// The x-height position with the median of the votes becomes the new
// x-height. This assumes that most characters will be correctly recognized
// even if the x-height is incorrect. This is not a terrible assumption, but
// it is not great. An improvement would be to use a classifier that does
// not care about vertical position or scaling at all.
// Separately collect stats on shifted baselines and apply the same logic to
// computing a best-fit shift to fix the error. If the baseline needs to be
// shifted, but the x-height is OK, returns the original x-height along with
// the baseline shift to indicate that recognition needs to re-run.

// If the max-min top of a unicharset char is bigger than kMaxCharTopRange
// then the char top cannot be used to judge misfits or suggest a new top.
const int kMaxCharTopRange = 48;

// Returns the number of misfit blob tops in this word.
int Tesseract::CountMisfitTops(WERD_RES *word_res) {
  int bad_blobs = 0;
  int num_blobs = word_res->rebuild_word->NumBlobs();
  for (int blob_id = 0; blob_id < num_blobs; ++blob_id) {
    TBLOB* blob = word_res->rebuild_word->blobs[blob_id];
    UNICHAR_ID class_id = word_res->best_choice->unichar_id(blob_id);
    if (unicharset.get_isalpha(class_id) || unicharset.get_isdigit(class_id)) {
      int top = blob->bounding_box().top();
      if (top >= INT_FEAT_RANGE)
        top = INT_FEAT_RANGE - 1;
      int min_bottom, max_bottom, min_top, max_top;
      unicharset.get_top_bottom(class_id, &min_bottom, &max_bottom,
                                &min_top, &max_top);
      if (max_top - min_top > kMaxCharTopRange)
        continue;
      bool bad =  top < min_top - x_ht_acceptance_tolerance ||
                  top > max_top + x_ht_acceptance_tolerance;
      if (bad)
        ++bad_blobs;
      if (debug_x_ht_level >= 1) {
        tprintf("Class %s is %s with top %d vs limits of %d->%d, +/-%d\n",
                unicharset.id_to_unichar(class_id),
                bad ? "Misfit" : "OK", top, min_top, max_top,
                static_cast<int>(x_ht_acceptance_tolerance));
      }
    }
  }
  return bad_blobs;
}

// Returns a new x-height maximally compatible with the result in word_res.
// See comment above for overall algorithm.
float Tesseract::ComputeCompatibleXheight(WERD_RES *word_res,
                                          float* baseline_shift) {
  STATS top_stats(0, UINT8_MAX);
  STATS shift_stats(-UINT8_MAX, UINT8_MAX);
  int bottom_shift = 0;
  int num_blobs = word_res->rebuild_word->NumBlobs();
  do {
    top_stats.clear();
    shift_stats.clear();
    for (int blob_id = 0; blob_id < num_blobs; ++blob_id) {
      TBLOB* blob = word_res->rebuild_word->blobs[blob_id];
      UNICHAR_ID class_id = word_res->best_choice->unichar_id(blob_id);
      if (unicharset.get_isalpha(class_id) ||
          unicharset.get_isdigit(class_id)) {
        int top = blob->bounding_box().top() + bottom_shift;
        // Clip the top to the limit of normalized feature space.
        if (top >= INT_FEAT_RANGE)
          top = INT_FEAT_RANGE - 1;
        int bottom = blob->bounding_box().bottom() + bottom_shift;
        int min_bottom, max_bottom, min_top, max_top;
        unicharset.get_top_bottom(class_id, &min_bottom, &max_bottom,
                                  &min_top, &max_top);
        // Chars with a wild top range would mess up the result so ignore them.
        if (max_top - min_top > kMaxCharTopRange)
          continue;
        int misfit_dist = std::max((min_top - x_ht_acceptance_tolerance) - top,
                            top - (max_top + x_ht_acceptance_tolerance));
        int height = top - kBlnBaselineOffset;
        if (debug_x_ht_level >= 2) {
          tprintf("Class %s: height=%d, bottom=%d,%d top=%d,%d, actual=%d,%d: ",
                  unicharset.id_to_unichar(class_id),
                  height, min_bottom, max_bottom, min_top, max_top,
                  bottom, top);
        }
        // Use only chars that fit in the expected bottom range, and where
        // the range of tops is sensibly near the xheight.
        if (min_bottom <= bottom + x_ht_acceptance_tolerance &&
            bottom - x_ht_acceptance_tolerance <= max_bottom &&
            min_top > kBlnBaselineOffset &&
            max_top - kBlnBaselineOffset >= kBlnXHeight &&
            misfit_dist > 0) {
          // Compute the x-height position using proportionality between the
          // actual height and expected height.
          int min_xht = DivRounded(height * kBlnXHeight,
                                   max_top - kBlnBaselineOffset);
          int max_xht = DivRounded(height * kBlnXHeight,
                                   min_top - kBlnBaselineOffset);
          if (debug_x_ht_level >= 2) {
            tprintf(" xht range min=%d, max=%d\n", min_xht, max_xht);
          }
          // The range of expected heights gets a vote equal to the distance
          // of the actual top from the expected top.
          for (int y = min_xht; y <= max_xht; ++y)
            top_stats.add(y, misfit_dist);
        } else if ((min_bottom > bottom + x_ht_acceptance_tolerance ||
                    bottom - x_ht_acceptance_tolerance > max_bottom) &&
                   bottom_shift == 0) {
          // Get the range of required bottom shift.
          int min_shift = min_bottom - bottom;
          int max_shift = max_bottom - bottom;
          if (debug_x_ht_level >= 2) {
            tprintf(" bottom shift min=%d, max=%d\n", min_shift, max_shift);
          }
          // The range of expected shifts gets a vote equal to the min distance
          // of the actual bottom from the expected bottom, spread over the
          // range of its acceptance.
          int misfit_weight = abs(min_shift);
          if (max_shift > min_shift)
            misfit_weight /= max_shift - min_shift;
          for (int y = min_shift; y <= max_shift; ++y)
            shift_stats.add(y, misfit_weight);
        } else {
          if (bottom_shift == 0) {
            // Things with bottoms that are already ok need to say so, on the
            // 1st iteration only.
            shift_stats.add(0, kBlnBaselineOffset);
          }
          if (debug_x_ht_level >= 2) {
            tprintf(" already OK\n");
          }
        }
      }
    }
    if (shift_stats.get_total() > top_stats.get_total()) {
      bottom_shift = IntCastRounded(shift_stats.median());
      if (debug_x_ht_level >= 2) {
        tprintf("Applying bottom shift=%d\n", bottom_shift);
      }
    }
  } while (bottom_shift != 0 &&
           top_stats.get_total() < shift_stats.get_total());
  // Baseline shift is opposite sign to the bottom shift.
  *baseline_shift = -bottom_shift / word_res->denorm.y_scale();
  if (debug_x_ht_level >= 2) {
    tprintf("baseline shift=%g\n", *baseline_shift);
  }
  if (top_stats.get_total() == 0)
    return bottom_shift != 0 ? word_res->x_height : 0.0f;
  // The new xheight is just the median vote, which is then scaled out
  // of BLN space back to pixel space to get the x-height in pixel space.
  float new_xht = top_stats.median();
  if (debug_x_ht_level >= 2) {
    tprintf("Median xht=%f\n", new_xht);
    tprintf("Mode20:A: New x-height = %f (norm), %f (orig)\n",
            new_xht, new_xht / word_res->denorm.y_scale());
  }
  // The xheight must change by at least x_ht_min_change to be used.
  if (fabs(new_xht - kBlnXHeight) >= x_ht_min_change)
    return new_xht / word_res->denorm.y_scale();
  else
    return bottom_shift != 0 ? word_res->x_height : 0.0f;
}

}  // namespace tesseract
