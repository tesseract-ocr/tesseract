/**********************************************************************
 * File:        feature_bmp.cpp
 * Description: Implementation of the Bitmap Feature Class
 * Author:    Ahmad Abdulkader
 * Created:   2007
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
#include <string>
#include "feature_base.h"
#include "feature_bmp.h"
#include "cube_utils.h"
#include "const.h"
#include "char_samp.h"

namespace tesseract {

FeatureBmp::FeatureBmp(TuningParams *params)
    :FeatureBase(params) {
  conv_grid_size_ = params->ConvGridSize();
}

FeatureBmp::~FeatureBmp() {
}

// Render a visualization of the features to a CharSamp.
// This is mainly used by visual-debuggers
CharSamp *FeatureBmp::ComputeFeatureBitmap(CharSamp *char_samp) {
  return char_samp->Scale(conv_grid_size_, conv_grid_size_);
}

// Compute the features for a given CharSamp
bool FeatureBmp::ComputeFeatures(CharSamp *char_samp, float *features) {
  return char_samp->ComputeFeatures(conv_grid_size_, features);
}
}

