/******************************************************************************
 ** Filename:    outfeat.h
 ** Purpose:     Definition of outline features.
 ** Author:      Dan Johnson
 **
 ** (c) Copyright Hewlett-Packard Company, 1988.
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

#ifndef OUTFEAT_H
#define OUTFEAT_H

#include "fpoint.h"
#include "mfoutline.h"
#include "ocrfeatures.h"

namespace tesseract {

typedef enum {
  OutlineFeatX,
  OutlineFeatY,
  OutlineFeatLength,
  OutlineFeatDir
} OUTLINE_FEAT_PARAM_NAME;

#define MAX_OUTLINE_FEATURES (100)

/*---------------------------------------------------------------------------
          Privat Function Prototypes
----------------------------------------------------------------------------*/
void AddOutlineFeatureToSet(FPOINT *Start, FPOINT *End, FEATURE_SET FeatureSet);

void ConvertToOutlineFeatures(MFOUTLINE Outline, FEATURE_SET FeatureSet);

void NormalizeOutlineX(FEATURE_SET FeatureSet);

} // namespace tesseract

#endif
