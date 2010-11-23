/**********************************************************************
 * File:        feature_bmp.h
 * Description: Declaration of the Bitmap Feature Class
 * Author:    PingPing xiu (xiupingping) & Ahmad Abdulkader
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

// The FeatureBmp class implements a Bitmap feature extractor class. It
// inherits from the FeatureBase class
// The Bitmap feature vectors is the the bitmap of the specified CharSamp
// scaled to a fixed grid size and then augmented by a 5 aux features that
// describe the size, aspect ration and placement within a word

#ifndef FEATURE_BMP_H
#define FEATURE_BMP_H

#include "char_samp.h"
#include "feature_base.h"

namespace tesseract {
class FeatureBmp : public FeatureBase {
 public:
  explicit FeatureBmp(TuningParams *params);
  virtual ~FeatureBmp();
  // Render a visualization of the features to a CharSamp.
  // This is mainly used by visual-debuggers
  virtual CharSamp *ComputeFeatureBitmap(CharSamp *samp);
    // Compute the features for a given CharSamp
  virtual bool ComputeFeatures(CharSamp *samp, float *features);
  // Returns the count of features
  virtual int FeatureCnt() {
    return 5 + (conv_grid_size_ * conv_grid_size_);
  }

 protected:
  // grid size, cached from the TuningParams object
  int conv_grid_size_;
};
}

#endif  // FEATURE_BMP_H
