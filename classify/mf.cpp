/******************************************************************************
 **	Filename:    mf.c
 **	Purpose:     Micro-feature interface to flexible feature extractor.
 **	Author:      Dan Johnson
 **	History:     Thu May 24 09:08:38 1990, DSJ, Created.
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
#include "mf.h"

#include "featdefs.h"
#include "mfdefs.h"
#include "mfx.h"

#include <math.h>

/**----------------------------------------------------------------------------
        Global Data Definitions and Declarations
----------------------------------------------------------------------------**/
/**----------------------------------------------------------------------------
              Private Code
----------------------------------------------------------------------------**/
/*---------------------------------------------------------------------------*/
FEATURE_SET ExtractMicros(TBLOB *Blob, const DENORM& bl_denorm,
                          const DENORM& cn_denorm,
                          const INT_FX_RESULT_STRUCT& fx_info) {
/*
 **	Parameters:
 **		Blob		blob to extract micro-features from
 **		denorm  control parameter to feature extractor.
 **	Globals: none
 **	Operation: Call the old micro-feature extractor and then copy
 **		the features into the new format.  Then deallocate the
 **		old micro-features.
 **	Return: Micro-features for Blob.
 **	Exceptions: none
 **	History: Wed May 23 18:06:38 1990, DSJ, Created.
 */
  int NumFeatures;
  MICROFEATURES Features, OldFeatures;
  FEATURE_SET FeatureSet;
  FEATURE Feature;
  MICROFEATURE OldFeature;

  OldFeatures = (MICROFEATURES)BlobMicroFeatures(Blob, bl_denorm, cn_denorm,
                                                 fx_info);
  if (OldFeatures == NULL)
    return NULL;
  NumFeatures = count (OldFeatures);
  FeatureSet = NewFeatureSet (NumFeatures);

  Features = OldFeatures;
  iterate(Features) {
    OldFeature = (MICROFEATURE) first_node (Features);
    Feature = NewFeature (&MicroFeatureDesc);
    Feature->Params[MFDirection] = OldFeature[ORIENTATION];
    Feature->Params[MFXPosition] = OldFeature[XPOSITION];
    Feature->Params[MFYPosition] = OldFeature[YPOSITION];
    Feature->Params[MFLength] = OldFeature[MFLENGTH];

    // Bulge features are deprecated and should not be used.  Set to 0.
    Feature->Params[MFBulge1] = 0.0f;
    Feature->Params[MFBulge2] = 0.0f;

#ifndef _WIN32
    // Assert that feature parameters are well defined.
    int i;
    for (i = 0; i < Feature->Type->NumParams; i++) {
      ASSERT_HOST(!isnan(Feature->Params[i]));
    }
#endif

    AddFeature(FeatureSet, Feature);
  }
  FreeMicroFeatures(OldFeatures);
  return FeatureSet;
}                                /* ExtractMicros */
