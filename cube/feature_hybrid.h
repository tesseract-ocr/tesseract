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

// The FeatureHybrid class implements a Bitmap feature extractor class. It
// inherits from the FeatureBase class
// This class describes the a hybrid feature vector composed by combining
// the bitmap and the chebyshev feature vectors

#ifndef FEATURE_HYBRID_H
#define FEATURE_HYBRID_H

#include "char_samp.h"
#include "feature_bmp.h"
#include "feature_chebyshev.h"

namespace tesseract {
class FeatureHybrid : public FeatureBase {
 public:
  explicit FeatureHybrid(TuningParams *params);
  virtual ~FeatureHybrid();
  // Render a visualization of the features to a CharSamp.
  // This is mainly used by visual-debuggers
  virtual CharSamp *ComputeFeatureBitmap(CharSamp *samp);
  // Compute the features for a given CharSamp
  virtual bool ComputeFeatures(CharSamp *samp, float *features);
  // Returns the count of features
  virtual int FeatureCnt() {
    if (feature_bmp_ == NULL || feature_chebyshev_ == NULL) {
      return 0;
    }
    return feature_bmp_->FeatureCnt() + feature_chebyshev_->FeatureCnt();
  }

 protected:
  FeatureBmp *feature_bmp_;
  FeatureChebyshev *feature_chebyshev_;
};
}

#endif  // FEATURE_HYBRID_H
