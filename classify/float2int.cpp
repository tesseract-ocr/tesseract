/******************************************************************************
 **	Filename:    float2int.c
 **	Purpose:     Routines for converting float features to int features
 **	Author:      Dan Johnson
 **	History:     Wed Mar 13 07:47:48 1991, DSJ, Created.
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
/*-----------------------------------------------------------------------------
          Include Files and Type Defines
-----------------------------------------------------------------------------*/
#include "float2int.h"
#include "normmatch.h"
#include "mfoutline.h"
#include "classify.h"
#include "picofeat.h"

#define MAX_INT_CHAR_NORM (INT_CHAR_NORM_RANGE - 1)

/*-----------------------------------------------------------------------------
              Public Code
-----------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
namespace tesseract {

/**
 * For each class in Templates, clear the corresponding
 * entry in CharNormArray.  CharNormArray is indexed by class
 * indicies (as obtained from Templates) rather than class id's.
 *
 * Globals: 
 * - none
 *
 * @param Templates specifies classes currently defined
 * @param CharNormArray array to be cleared
 *
 * @note Exceptions: none
 * @note History: Wed Feb 20 11:20:54 1991, DSJ, Created.
 */
void ClearCharNormArray(INT_TEMPLATES Templates,
                        CLASS_NORMALIZATION_ARRAY CharNormArray) {
  int i;

  for (i = 0; i < Templates->NumClasses; i++) {
    CharNormArray[i] = 0;
  }

}                                /* ClearCharNormArray */


/*---------------------------------------------------------------------------*/
/** 
 * For each class in Templates, compute the match between
 * NormFeature and the normalization protos for that class.
 * Convert this number to the range from 0 - 255 and store it
 * into CharNormArray.  CharNormArray is indexed by class
 * indicies (as obtained from Templates) rather than class id's.
 *
 * Globals: 
 * - none
 *
 * @param NormFeature character normalization feature
 * @param Templates specifies classes currently defined
 * @param[out] CharNormArray place to put results
 *
 * @note Exceptions: none
 * @note History: Wed Feb 20 11:20:54 1991, DSJ, Created.
 */
void Classify::ComputeIntCharNormArray(
  FEATURE NormFeature, INT_TEMPLATES Templates,
  CLASS_NORMALIZATION_ARRAY CharNormArray) {
  int i;
  int NormAdjust;

  for (i = 0; i < Templates->NumClasses; i++) {
    NormAdjust = (int) (INT_CHAR_NORM_RANGE *
      ComputeNormMatch (i, NormFeature, FALSE));
    if (NormAdjust < 0)
      NormAdjust = 0;
    else if (NormAdjust > MAX_INT_CHAR_NORM)
      NormAdjust = MAX_INT_CHAR_NORM;

    CharNormArray[i] = NormAdjust;
  }
}                                /* ComputeIntCharNormArray */

}  // namespace tesseract

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
 *
 * @note Exceptions: none
 * @note History: Wed Feb 20 10:58:45 1991, DSJ, Created.
 */
void ComputeIntFeatures(FEATURE_SET Features, INT_FEATURE_ARRAY IntFeatures) {
  int Fid;
  FEATURE Feature;
  FLOAT32 YShift;

  if (classify_norm_method == baseline)
    YShift = BASELINE_Y_SHIFT;
  else
    YShift = Y_SHIFT;

  for (Fid = 0; Fid < Features->NumFeatures; Fid++) {
    Feature = Features->Features[Fid];

    IntFeatures[Fid].X = BucketFor (Feature->Params[PicoFeatX],
      X_SHIFT, INT_FEAT_RANGE);
    IntFeatures[Fid].Y = BucketFor (Feature->Params[PicoFeatY],
      YShift, INT_FEAT_RANGE);
    IntFeatures[Fid].Theta = CircBucketFor (Feature->Params[PicoFeatDir],
      ANGLE_SHIFT, INT_FEAT_RANGE);
    IntFeatures[Fid].CP_misses = 0;
  }
}                                /* ComputeIntFeatures */
