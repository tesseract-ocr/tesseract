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

#include "ocrfeatures.h"

#include <array>  // for std::array
#include <string> // for std::string

namespace tesseract {

/* Enumerate the different types of features currently defined. */
#define NUM_FEATURE_TYPES 4
extern TESS_API const char *const kMicroFeatureType;
extern TESS_API const char *const kCNFeatureType;
extern TESS_API const char *const kIntFeatureType;
extern TESS_API const char *const kGeoFeatureType;

/* A character is described by multiple sets of extracted features.  Each
  set contains a number of features of a particular type, for example, a
  set of bays, or a set of closures, or a set of microfeatures.  Each
  feature consists of a number of parameters.  All features within a
  feature set contain the same number of parameters.*/

struct FEATURE_DEFS_STRUCT {
  int32_t NumFeatureTypes;
  const FEATURE_DESC_STRUCT *FeatureDesc[NUM_FEATURE_TYPES];
};
using FEATURE_DEFS = FEATURE_DEFS_STRUCT *;

struct CHAR_DESC_STRUCT {
  /// Allocate a new character description, initialize its
  /// feature sets to be empty, and return it.
  CHAR_DESC_STRUCT(const FEATURE_DEFS_STRUCT &FeatureDefs) {
    NumFeatureSets = FeatureDefs.NumFeatureTypes;
  }

  /// Release the memory consumed by the specified character
  /// description and all of the features in that description.
  ~CHAR_DESC_STRUCT() {
    for (size_t i = 0; i < NumFeatureSets; i++) {
      delete FeatureSets[i];
    }
  }

  uint32_t NumFeatureSets;
  std::array<FEATURE_SET_STRUCT *, NUM_FEATURE_TYPES> FeatureSets;
};

/*----------------------------------------------------------------------
    Generic functions for manipulating character descriptions
----------------------------------------------------------------------*/
TESS_API
void InitFeatureDefs(FEATURE_DEFS_STRUCT *featuredefs);

bool ValidCharDescription(const FEATURE_DEFS_STRUCT &FeatureDefs, CHAR_DESC_STRUCT *CharDesc);

void WriteCharDescription(const FEATURE_DEFS_STRUCT &FeatureDefs, CHAR_DESC_STRUCT *CharDesc, std::string &str);

TESS_API
CHAR_DESC_STRUCT *ReadCharDescription(const FEATURE_DEFS_STRUCT &FeatureDefs, FILE *File);

TESS_API
uint32_t ShortNameToFeatureType(const FEATURE_DEFS_STRUCT &FeatureDefs, const char *ShortName);

/**----------------------------------------------------------------------------
        Global Data Definitions and Declarations
----------------------------------------------------------------------------**/
extern const FEATURE_DESC_STRUCT MicroFeatureDesc;
extern TESS_API const FEATURE_DESC_STRUCT PicoFeatDesc;
extern const FEATURE_DESC_STRUCT CharNormDesc;
extern const FEATURE_DESC_STRUCT OutlineFeatDesc;
extern const FEATURE_DESC_STRUCT IntFeatDesc;
extern const FEATURE_DESC_STRUCT GeoFeatDesc;

} // namespace tesseract

#endif
