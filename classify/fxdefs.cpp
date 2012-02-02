/******************************************************************************
 **	Filename:    fxdefs.c
 **	Purpose:     Utility functions to be used by feature extractors.
 **	Author:      Dan Johnson
 **	History:     Sun Jan 21 15:29:02 1990, DSJ, Created.
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
#include "fxdefs.h"
#include "featdefs.h"
#include "mf.h"
#include "outfeat.h"
#include "picofeat.h"
#include "normfeat.h"

/*-----------------------------------------------------------------------------
        Global Data Definitions and Declarations
-----------------------------------------------------------------------------*/
// Definitions of extractors separated from feature definitions.
const FEATURE_EXT_STRUCT MicroFeatureExt = { ExtractMicros };
const FEATURE_EXT_STRUCT CharNormExt = { ExtractCharNormFeatures };
const FEATURE_EXT_STRUCT IntFeatExt = { ExtractIntCNFeatures };
const FEATURE_EXT_STRUCT GeoFeatExt = { ExtractIntGeoFeatures };

// MUST be kept in-sync with DescDefs in featdefs.cpp.
const FEATURE_EXT_STRUCT* ExtractorDefs[NUM_FEATURE_TYPES] = {
  &MicroFeatureExt,
  &CharNormExt,
  &IntFeatExt,
  &GeoFeatExt
};

void SetupExtractors(FEATURE_DEFS_STRUCT *FeatureDefs) {
  for (int i = 0; i < NUM_FEATURE_TYPES; ++i)
    FeatureDefs->FeatureExtractors[i] = ExtractorDefs[i];
}
