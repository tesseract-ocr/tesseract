/**********************************************************************
 * File:        fixxht.cpp  (Formerly fixxht.c)
 * Description: Improve x_ht and look out for case inconsistencies
 * Author:		Phil Cheatle
 * Created:		Thu Aug  5 14:11:08 BST 1993
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

#include          <string.h>
#include          <ctype.h>
#include          "params.h"
#include          "float2int.h"
#include          "tesseractclass.h"

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
float Tesseract::ComputeCompatibleXheight(WERD_RES *word_res) {
  STATS top_stats(0, MAX_UINT8);
  int num_blobs = word_res->rebuild_word->NumBlobs();
  for (int blob_id = 0; blob_id < num_blobs; ++blob_id) {
    TBLOB* blob = word_res->rebuild_word->blobs[blob_id];
    UNICHAR_ID class_id = word_res->best_choice->unichar_id(blob_id);
    if (unicharset.get_isalpha(class_id) || unicharset.get_isdigit(class_id)) {
      int top = blob->bounding_box().top();
      // Clip the top to the limit of normalized feature space.
      if (top >= INT_FEAT_RANGE)
        top = INT_FEAT_RANGE - 1;
      int bottom = blob->bounding_box().bottom();
      int min_bottom, max_bottom, min_top, max_top;
      unicharset.get_top_bottom(class_id, &min_bottom, &max_bottom,
                                &min_top, &max_top);
      // Chars with a wild top range would mess up the result so ignore them.
      if (max_top - min_top > kMaxCharTopRange)
        continue;
      int misfit_dist = MAX((min_top - x_ht_acceptance_tolerance) - top,
                          top - (max_top + x_ht_acceptance_tolerance));
      int height = top - kBlnBaselineOffset;
      if (debug_x_ht_level >= 20) {
        tprintf("Class %s: height=%d, bottom=%d,%d top=%d,%d, actual=%d,%d : ",
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
        if (debug_x_ht_level >= 20) {
          tprintf(" xht range min=%d, max=%d\n",
                  min_xht, max_xht);
        }
        // The range of expected heights gets a vote equal to the distance
        // of the actual top from the expected top.
        for (int y = min_xht; y <= max_xht; ++y)
          top_stats.add(y, misfit_dist);
      } else if (debug_x_ht_level >= 20) {
        tprintf(" already OK\n");
      }
    }
  }
  if (top_stats.get_total() == 0)
    return 0.0f;
  // The new xheight is just the median vote, which is then scaled out
  // of BLN space back to pixel space to get the x-height in pixel space.
  float new_xht = top_stats.median();
  if (debug_x_ht_level >= 20) {
    tprintf("Median xht=%f\n", new_xht);
    tprintf("Mode20:A: New x-height = %f (norm), %f (orig)\n",
            new_xht, new_xht / word_res->denorm.y_scale());
  }
  // The xheight must change by at least x_ht_min_change to be used.
  if (fabs(new_xht - kBlnXHeight) >= x_ht_min_change)
    return new_xht / word_res->denorm.y_scale();
  else
    return 0.0f;
}

}  // namespace tesseract
