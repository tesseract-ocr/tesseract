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
/**----------------------------------------------------------------------------
          Include Files and Type Defines
----------------------------------------------------------------------------**/
#include "float2int.h"
#include "normmatch.h"
#include "mfoutline.h"
#include "picofeat.h"

#define MAX_INT_CHAR_NORM (INT_CHAR_NORM_RANGE - 1)

/**----------------------------------------------------------------------------
              Public Code
----------------------------------------------------------------------------**/
/*---------------------------------------------------------------------------*/
void ClearCharNormArray(INT_TEMPLATES Templates,
                        CLASS_NORMALIZATION_ARRAY CharNormArray) {
/*
 **	Parameters:
 **		Templates	specifies classes currently defined
 **		CharNormArray	array to be cleared
 **	Globals: none
 **	Operation: For each class in Templates, clear the corresponding
 **		entry in CharNormArray.  CharNormArray is indexed by class
 **		indicies (as obtained from Templates) rather than class id's.
 **	Return: none
 **	Exceptions: none
 **	History: Wed Feb 20 11:20:54 1991, DSJ, Created.
 */
  int i;

  for (i = 0; i < Templates->NumClasses; i++) {
    CharNormArray[i] = 0;
  }

}                                /* ClearCharNormArray */


/*---------------------------------------------------------------------------*/
void ComputeIntCharNormArray(FEATURE NormFeature,
                             INT_TEMPLATES Templates,
                             CLASS_NORMALIZATION_ARRAY CharNormArray) {
/*
 **	Parameters:
 **		NormFeature	character normalization feature
 **		Templates	specifies classes currently defined
 **		CharNormArray	place to put results
 **	Globals: none
 **	Operation: For each class in Templates, compute the match between
 **		NormFeature and the normalization protos for that class.
 **		Convert this number to the range from 0 - 255 and store it
 **		into CharNormArray.  CharNormArray is indexed by class
 **		indicies (as obtained from Templates) rather than class id's.
 **	Return: none (results are returned in CharNormArray)
 **	Exceptions: none
 **	History: Wed Feb 20 11:20:54 1991, DSJ, Created.
 */
  int i;
  int NormAdjust;

  for (i = 0; i < Templates->NumClasses; i++) {
    NormAdjust = (int) (INT_CHAR_NORM_RANGE *
      ComputeNormMatch (Templates->ClassIdFor[i],
      NormFeature, FALSE));
    if (NormAdjust < 0)
      NormAdjust = 0;
    else if (NormAdjust > MAX_INT_CHAR_NORM)
      NormAdjust = MAX_INT_CHAR_NORM;

    CharNormArray[i] = NormAdjust;
  }

}                                /* ComputeIntCharNormArray */


/*---------------------------------------------------------------------------*/
void ComputeIntFeatures(FEATURE_SET Features, INT_FEATURE_ARRAY IntFeatures) {
/*
 **	Parameters:
 **		Features	floating point pico-features to be converted
 **		IntFeatures	array to put converted features into
 **	Globals: none
 **	Operation: This routine converts each floating point pico-feature
 **		in Features into integer format and saves it into
 **		IntFeatures.
 **	Return: none (results are returned in IntFeatures)
 **	Exceptions: none
 **	History: Wed Feb 20 10:58:45 1991, DSJ, Created.
 */
  int Fid;
  FEATURE Feature;
  FLOAT32 YShift;

  if (NormMethod == baseline)
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
