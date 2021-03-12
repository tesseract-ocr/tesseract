/******************************************************************************
 **  Filename:    intfx.h
 **  Purpose:     Interface to high level integer feature extractor.
 **  Author:      Robert Moss
 **  History:     Tue May 21 15:51:57 MDT 1991, RWM, Created.
 **
 **  (c) Copyright Hewlett-Packard Company, 1988.
 ** Licensed under the Apache License, Version 2.0 (the "License");
 ** you may not use this file except in compliance with the License.
 ** You may obtain a copy of the License at
 ** http://www.apache.org/licenses/LICENSE-2.0
 ** Unless required by applicable law or agreed to in writing, software
 ** distributed under the License is distributed on an "AS IS" BASIS,
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 ** See the License for the specific language governing permissions and
 ** limitations under the License.
 ******************************************************************************/
#ifndef INTFX_H
#define INTFX_H

#include "blobs.h"
#include "intproto.h"
#include "normalis.h"

#include <cmath>

namespace tesseract {

class DENORM;

class TrainingSample;

struct INT_FX_RESULT_STRUCT {
  int32_t Length;       // total length of all outlines
  int16_t Xmean, Ymean; // center of mass of all outlines
  int16_t Rx, Ry;       // radius of gyration
  int16_t NumBL, NumCN; // number of features extracted
  int16_t Width;        // Width of blob in BLN coords.
  uint8_t YBottom;      // Bottom of blob in BLN coords.
  uint8_t YTop;         // Top of blob in BLN coords.
};

// The standard feature length
const double kStandardFeatureLength = 64.0 / 5;

/**----------------------------------------------------------------------------
          Public Function Prototypes
----------------------------------------------------------------------------**/
TESS_API
void InitIntegerFX();

// Returns a vector representing the direction of a feature with the given
// theta direction in an INT_FEATURE_STRUCT.
TESS_API
FCOORD FeatureDirection(uint8_t theta);

// Generates a TrainingSample from a TBLOB. Extracts features and sets
// the bounding box, so classifiers that operate on the image can work.
// TODO(rays) BlobToTrainingSample must remain a global function until
// the FlexFx and FeatureDescription code can be removed and LearnBlob
// made a member of Classify.
TrainingSample *BlobToTrainingSample(const TBLOB &blob, bool nonlinear_norm,
                                     INT_FX_RESULT_STRUCT *fx_info,
                                     std::vector<INT_FEATURE_STRUCT> *bl_features);

} // namespace tesseract

#endif
