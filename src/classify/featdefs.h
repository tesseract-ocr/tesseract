/******************************************************************************
 ** Filename:    featdefs.h
 ** Purpose:     Definitions of currently defined feature types.
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

#ifndef FEATDEFS_H
#define FEATDEFS_H

/**----------------------------------------------------------------------------
          Include Files and Type Defines
----------------------------------------------------------------------------**/
#include "ocrfeatures.h"

/* Enumerate the different types of features currently defined. */
#define NUM_FEATURE_TYPES 4
extern TESS_API const char* const kMicroFeatureType;
extern TESS_API const char* const kCNFeatureType;
extern TESS_API const char* const kIntFeatureType;
extern TESS_API const char* const kGeoFeatureType;

/* A character is described by multiple sets of extracted features.  Each
  set contains a number of features of a particular type, for example, a
  set of bays, or a set of closures, or a set of microfeatures.  Each
  feature consists of a number of parameters.  All features within a
  feature set contain the same number of parameters.*/

struct CHAR_DESC_STRUCT {
  uint32_t NumFeatureSets;
  FEATURE_SET FeatureSets[NUM_FEATURE_TYPES];
};
using CHAR_DESC = CHAR_DESC_STRUCT *;

struct FEATURE_DEFS_STRUCT {
  int32_t NumFeatureTypes;
  const FEATURE_DESC_STRUCT* FeatureDesc[NUM_FEATURE_TYPES];
};
using FEATURE_DEFS = FEATURE_DEFS_STRUCT *;

/*----------------------------------------------------------------------
    Generic functions for manipulating character descriptions
----------------------------------------------------------------------*/
void InitFeatureDefs(FEATURE_DEFS_STRUCT *featuredefs);

void FreeCharDescription(CHAR_DESC CharDesc);

CHAR_DESC NewCharDescription(const FEATURE_DEFS_STRUCT &FeatureDefs);

bool ValidCharDescription(const FEATURE_DEFS_STRUCT &FeatureDefs,
                          CHAR_DESC CharDesc);

void WriteCharDescription(const FEATURE_DEFS_STRUCT& FeatureDefs,
                          CHAR_DESC CharDesc, STRING* str);

CHAR_DESC ReadCharDescription(const FEATURE_DEFS_STRUCT &FeatureDefs,
                              FILE *File);

uint32_t ShortNameToFeatureType(const FEATURE_DEFS_STRUCT &FeatureDefs,
                                const char *ShortName);

/**----------------------------------------------------------------------------
        Global Data Definitions and Declarations
----------------------------------------------------------------------------**/
extern const FEATURE_DESC_STRUCT MicroFeatureDesc;
extern TESS_API const FEATURE_DESC_STRUCT PicoFeatDesc;
extern const FEATURE_DESC_STRUCT CharNormDesc;
extern const FEATURE_DESC_STRUCT OutlineFeatDesc;
extern const FEATURE_DESC_STRUCT IntFeatDesc;
extern const FEATURE_DESC_STRUCT GeoFeatDesc;
#endif
