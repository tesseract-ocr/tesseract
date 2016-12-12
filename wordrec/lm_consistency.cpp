///////////////////////////////////////////////////////////////////////
// File:        lm_consistency.cpp
// Description: Struct for recording consistency of the paths  representing
//              OCR hypotheses.
// Author:      Rika Antonova
// Created:     Mon Jun 20 11:26:43 PST 2012
//
// (C) Copyright 2012, Google Inc.
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
////////////////////////////////////////////////////////////////////////

#include "lm_consistency.h"

#include "associate.h"
#include "dict.h"
#include "ratngs.h"

namespace tesseract {

void LMConsistencyInfo::ComputeXheightConsistency(
    const BLOB_CHOICE *b, bool is_punc) {
  if (xht_decision == XH_INCONSISTENT)
    return;  // It isn't going to get any better.

  // Compute xheight consistency.
  bool parent_null = xht_sp < 0;
  int parent_sp = xht_sp;
  // Debug strings.
  if (b->yshift() > LMConsistencyInfo::kShiftThresh) {
    xht_sp = LMConsistencyInfo::kSUP;
  } else if (b->yshift() < -LMConsistencyInfo::kShiftThresh) {
    xht_sp = LMConsistencyInfo::kSUB;
  } else {
    xht_sp = LMConsistencyInfo::kNORM;
  }
  xht_count[xht_sp]++;
  if (is_punc) xht_count_punc[xht_sp]++;
  if (!parent_null) {
    xpos_entropy += abs(parent_sp - xht_sp);
  }
  // TODO(eger): Figure out a better way to account for small caps.
  // For the first character not y-shifted, we only care if it is too small.
  // Too large is common in drop caps and small caps.
  // int16_t small_xht = b->min_xheight();
  //  if (parent_vse == nullptr && sp == LanguageModelConsistencyInfo::kNORM) {
  //  small_xht = 0;
  // }
  IntersectRange(b->min_xheight(), b->max_xheight(),
                 &(xht_lo[xht_sp]), &(xht_hi[xht_sp]));


  // Compute xheight inconsistency kinds.
  if (parent_null) {
    if (xht_count[kNORM] == 1) {
      xht_decision = XH_GOOD;
    } else {
      xht_decision = XH_SUBNORMAL;
    }
    return;
  }

  // When we intersect the ranges of xheights in pixels for all characters in
  // each position (subscript, normal, superscript),
  // How much range must be left?  0? [exactly one pixel height for xheight] 1?
  // TODO(eger): Extend this code to take a prior for the rest of the line.
  const int kMinIntersectedXHeightRange = 0;
  for (int i = 0; i < kNumPos; i++) {
    if (xht_lo[i] > xht_hi[i] - kMinIntersectedXHeightRange) {
      xht_decision = XH_INCONSISTENT;
      return;
    }
  }

  // Reject as improbable anything where there's much punctuation in subscript
  // or superscript regions.
  if (xht_count_punc[kSUB] > xht_count[kSUB] * 0.4 ||
      xht_count_punc[kSUP] > xht_count[kSUP] * 0.4) {
    xht_decision = XH_INCONSISTENT;
    return;
  }

  // Now check that the subscript and superscript aren't too small relative to
  // the mainline.
  double mainline_xht = static_cast<double>(xht_lo[kNORM]);
  double kMinSizeRatio = 0.4;
  if (mainline_xht > 0.0 &&
      (static_cast<double>(xht_hi[kSUB]) / mainline_xht < kMinSizeRatio ||
       static_cast<double>(xht_hi[kSUP]) / mainline_xht < kMinSizeRatio)) {
    xht_decision = XH_INCONSISTENT;
    return;
  }
  // TODO(eger): Check into inconsistency of super/subscript y offsets.
  if (xpos_entropy > kMaxEntropy) {
    xht_decision = XH_INCONSISTENT;
    return;
  }
  if (xht_count[kSUB] == 0 && xht_count[kSUP] == 0) {
    xht_decision = XH_GOOD;
    return;
  }
  xht_decision = XH_SUBNORMAL;
}

}  // namespace tesseract
