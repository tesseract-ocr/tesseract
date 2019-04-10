/******************************************************************************
 ** Filename:    mf.c
 ** Purpose:     Micro-feature interface to flexible feature extractor.
 ** Author:      Dan Johnson
 ** History:     Thu May 24 09:08:38 1990, DSJ, Created.
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
/*----------------------------------------------------------------------------
          Include Files and Type Defines
----------------------------------------------------------------------------*/
#include "mf.h"

#include "featdefs.h"
#include "mfdefs.h"
#include "mfx.h"

#include <cmath>

/*----------------------------------------------------------------------------
        Global Data Definitions and Declarations
----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------
              Private Code
----------------------------------------------------------------------------*/
/**
 * Call the old micro-feature extractor and then copy
 * the features into the new format.  Then deallocate the
 * old micro-features.
 * @param Blob  blob to extract micro-features from
 * @param cn_denorm  control parameter to feature extractor.
 * @return Micro-features for Blob.
 */
FEATURE_SET ExtractMicros(TBLOB* Blob, const DENORM& cn_denorm) {
  int NumFeatures;
  MICROFEATURES Features, OldFeatures;
  FEATURE_SET FeatureSet;
  FEATURE Feature;
  MICROFEATURE OldFeature;

  OldFeatures = BlobMicroFeatures(Blob, cn_denorm);
  if (OldFeatures == nullptr)
    return nullptr;
  NumFeatures = count (OldFeatures);
  FeatureSet = NewFeatureSet (NumFeatures);

  Features = OldFeatures;
  iterate(Features) {
    OldFeature = reinterpret_cast<MICROFEATURE>first_node (Features);
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
      ASSERT_HOST(!std::isnan(Feature->Params[i]));
    }
#endif

    AddFeature(FeatureSet, Feature);
  }
  FreeMicroFeatures(OldFeatures);
  return FeatureSet;
}                                /* ExtractMicros */
