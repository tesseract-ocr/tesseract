/**********************************************************************
 * File:        feature_base.h
 * Description: Declaration of the Feature Base Class
 * Author:    Ping Ping (xiupingping), Ahmad Abdulkader
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

// The FeatureBase class is the base class for any Feature Extraction class
// It provided 3 pure virtual functions (to inherit):
// 1- FeatureCnt: A method to returns the count of features
// 2- ComputeFeatures: A method to compute the features for a given CharSamp
// 3- ComputeFeatureBitmap: A method to render a visualization of the features
// to a CharSamp. This is mainly used by visual-debuggers

#ifndef FEATURE_BASE_H
#define FEATURE_BASE_H

#include "char_samp.h"
#include "tuning_params.h"

namespace tesseract {
class FeatureBase {
 public:
  explicit FeatureBase(TuningParams *params)
      : params_(params) {
  }
  virtual ~FeatureBase() {}

  // Compute the features for a given CharSamp
  virtual bool ComputeFeatures(CharSamp *char_samp, float *features) = 0;
  // Render a visualization of the features to a CharSamp.
  // This is mainly used by visual-debuggers
  virtual CharSamp *ComputeFeatureBitmap(CharSamp *char_samp) = 0;
  // Returns the count of features
  virtual int FeatureCnt() = 0;

 protected:
  TuningParams *params_;
};
}

#endif  // FEATURE_BASE_H

