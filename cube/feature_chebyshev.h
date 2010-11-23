/**********************************************************************
 * File:        feature_chebyshev.h
 * Description: Declaration of the Chebyshev coefficients Feature Class
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

// The FeatureChebyshev class implements a Bitmap feature extractor class. It
// inherits from the FeatureBase class
// The feature vector is the composed of the chebyshev coefficients of 4 time
// sequences. The time sequences are the left, top, right & bottom
// bitmap profiles of the input samples

#ifndef FEATURE_CHEBYSHEV_H
#define FEATURE_CHEBYSHEV_H

#include "char_samp.h"
#include "feature_base.h"

namespace tesseract {
class FeatureChebyshev : public FeatureBase {
 public:
  explicit FeatureChebyshev(TuningParams *params);
  virtual ~FeatureChebyshev();
  // Render a visualization of the features to a CharSamp.
  // This is mainly used by visual-debuggers
  virtual CharSamp *ComputeFeatureBitmap(CharSamp *samp);
  // Compute the features for a given CharSamp
  virtual bool ComputeFeatures(CharSamp *samp, float *features);
  // Returns the count of features
  virtual int FeatureCnt() {
    return (4 * kChebychevCoefficientCnt);
  }

 protected:
  static const int kChebychevCoefficientCnt = 40;
  // Compute Chebychev coefficients for the specified vector
  void ChebyshevCoefficients(const vector<float> &input,
                             int coeff_cnt, float *coeff);
  // Compute the features for a given CharSamp
  bool ComputeChebyshevCoefficients(CharSamp *samp, float *features);
};
}

#endif  // FEATURE_CHEBYSHEV_H
