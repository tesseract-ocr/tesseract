/******************************************************************************
 **	Filename:    normfeat.c
 **	Purpose:     Definition of char normalization features.
 **	Author:      Dan Johnson
 **	History:     12/14/90, DSJ, Created.
 **
 **	(c) Copyright Hewlett-Packard Company, 1988.
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
/**----------------------------------------------------------------------------
          Include Files and Type Defines
----------------------------------------------------------------------------**/
#include "normfeat.h"

#include "intfx.h"
#include "featdefs.h"
#include "mfoutline.h"

/**----------------------------------------------------------------------------
              Public Code
----------------------------------------------------------------------------**/

// Return the length of the outline in baseline normalized form.
FLOAT32 ActualOutlineLength(FEATURE Feature) {
  return (Feature->Params[CharNormLength] * LENGTH_COMPRESSION);
}


/*---------------------------------------------------------------------------*/
// Return the character normalization feature for a blob.
//
// The features returned are in a scale where the x-height has been
// normalized to live in the region y = [-0.25 .. 0.25].  Example ranges
// for English below are based on the Linux font collection on 2009-12-04:
//
//   Params[CharNormY]
//     The y coordinate of the grapheme's centroid.
//     English: [-0.27, 0.71]
//
//   Params[CharNormLength]
//     The length of the grapheme's outline (tiny segments discarded),
//     divided by 10.0=LENGTH_COMPRESSION.
//     English: [0.16, 0.85]
//
//   Params[CharNormRx]
//     The radius of gyration about the x axis, as measured from CharNormY.
//     English: [0.011, 0.34]
//
//   Params[CharNormRy]
//     The radius of gyration about the y axis, as measured from
//     the x center of the grapheme's bounding box.
//     English: [0.011, 0.31]
//
FEATURE_SET ExtractCharNormFeatures(const INT_FX_RESULT_STRUCT& fx_info) {
  FEATURE_SET feature_set = NewFeatureSet(1);
  FEATURE feature = NewFeature(&CharNormDesc);

  feature->Params[CharNormY] =
      MF_SCALE_FACTOR * (fx_info.Ymean - kBlnBaselineOffset);
  feature->Params[CharNormLength] =
      MF_SCALE_FACTOR * fx_info.Length / LENGTH_COMPRESSION;
  feature->Params[CharNormRx] = MF_SCALE_FACTOR * fx_info.Rx;
  feature->Params[CharNormRy] = MF_SCALE_FACTOR * fx_info.Ry;

  AddFeature(feature_set, feature);

  return feature_set;
}                                /* ExtractCharNormFeatures */
