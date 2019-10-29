/******************************************************************************
 ** Filename:    float2int.cpp
 ** Purpose:     Routines for converting float features to int features
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
/*-----------------------------------------------------------------------------
          Include Files and Type Defines
-----------------------------------------------------------------------------*/
#include "float2int.h"
#include "normmatch.h"
#include "mfoutline.h"
#include "classify.h"
#include <tesseract/helpers.h>
#include "picofeat.h"

#define MAX_INT_CHAR_NORM (INT_CHAR_NORM_RANGE - 1)

/*-----------------------------------------------------------------------------
              Public Code
-----------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
namespace tesseract {

/**
 * For each class in the unicharset, clears the corresponding
 * entry in char_norm_array.  char_norm_array is indexed by unichar_id.
 *
 * Globals:
 * - none
 *
 * @param char_norm_array array to be cleared
 */
void Classify::ClearCharNormArray(uint8_t* char_norm_array) {
  memset(char_norm_array, 0, sizeof(*char_norm_array) * unicharset.size());
}                                /* ClearCharNormArray */


/*---------------------------------------------------------------------------*/
/**
 * For each class in unicharset, computes the match between
 * norm_feature and the normalization protos for that class.
 * Converts this number to the range from 0 - 255 and stores it
 * into char_norm_array.  CharNormArray is indexed by unichar_id.
 *
 * Globals:
 * - PreTrainedTemplates current set of built-in templates
 *
 * @param norm_feature character normalization feature
 * @param[out] char_norm_array place to put results of size unicharset.size()
 */
void Classify::ComputeIntCharNormArray(const FEATURE_STRUCT& norm_feature,
                                       uint8_t* char_norm_array) {
  for (int i = 0; i < unicharset.size(); i++) {
    if (i < PreTrainedTemplates->NumClasses) {
      int norm_adjust = static_cast<int>(INT_CHAR_NORM_RANGE *
        ComputeNormMatch(i, norm_feature, false));
      char_norm_array[i] = ClipToRange(norm_adjust, 0, MAX_INT_CHAR_NORM);
    } else {
      // Classes with no templates (eg. ambigs & ligatures) default
      // to worst match.
      char_norm_array[i] = MAX_INT_CHAR_NORM;
    }
  }
}                                /* ComputeIntCharNormArray */


/*---------------------------------------------------------------------------*/
/**
 * This routine converts each floating point pico-feature
 * in Features into integer format and saves it into
 * IntFeatures.
 *
 * Globals:
 * - none
 *
 * @param Features floating point pico-features to be converted
 * @param[out] IntFeatures array to put converted features into
 */
void Classify::ComputeIntFeatures(FEATURE_SET Features,
                                  INT_FEATURE_ARRAY IntFeatures) {
  float YShift;

  if (classify_norm_method == baseline)
    YShift = BASELINE_Y_SHIFT;
  else
    YShift = Y_SHIFT;

  for (int Fid = 0; Fid < Features->NumFeatures; Fid++) {
    FEATURE Feature = Features->Features[Fid];

    IntFeatures[Fid].X =
        Bucket8For(Feature->Params[PicoFeatX], X_SHIFT, INT_FEAT_RANGE);
    IntFeatures[Fid].Y =
        Bucket8For(Feature->Params[PicoFeatY], YShift, INT_FEAT_RANGE);
    IntFeatures[Fid].Theta = CircBucketFor(Feature->Params[PicoFeatDir],
                                           ANGLE_SHIFT, INT_FEAT_RANGE);
    IntFeatures[Fid].CP_misses = 0;
  }
}                                /* ComputeIntFeatures */
}  // namespace tesseract
