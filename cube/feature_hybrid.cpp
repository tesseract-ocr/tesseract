/**********************************************************************
 * File:        feature_chebyshev.cpp
 * Description: Implementation of the Chebyshev coefficients Feature Class
 * Author:    Ahmad Abdulkader
 * Created:   2008
 *
 * (C) Copyright 2008, Google Inc.
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

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string>
#include <vector>
#include <algorithm>
#include "feature_base.h"
#include "feature_hybrid.h"
#include "cube_utils.h"
#include "const.h"
#include "char_samp.h"

namespace tesseract {

FeatureHybrid::FeatureHybrid(TuningParams *params)
    :FeatureBase(params) {
  feature_bmp_ = new FeatureBmp(params);
  feature_chebyshev_ = new FeatureChebyshev(params);
}

FeatureHybrid::~FeatureHybrid() {
  delete feature_bmp_;
  delete feature_chebyshev_;
}

// Render a visualization of the features to a CharSamp.
// This is mainly used by visual-debuggers
CharSamp *FeatureHybrid::ComputeFeatureBitmap(CharSamp *char_samp) {
  return char_samp;
}


// Compute the features of a given CharSamp
bool FeatureHybrid::ComputeFeatures(CharSamp *char_samp, float *features) {
  if (feature_bmp_ == NULL || feature_chebyshev_ == NULL) {
    return false;
  }
  if (!feature_bmp_->ComputeFeatures(char_samp, features)) {
    return false;
  }
  return feature_chebyshev_->ComputeFeatures(char_samp,
    features + feature_bmp_->FeatureCnt());
}

}  // namespace tesseract
