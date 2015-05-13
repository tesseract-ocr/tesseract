/******************************************************************************
 **	Filename:    features.h
 **	Purpose:     Generic definition of a feature.
 **	Author:      Dan Johnson
 **	History:     Sun May 20 10:28:30 1990, DSJ, Created.
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
#ifndef   FEATURES_H
#define   FEATURES_H

/**----------------------------------------------------------------------------
          Include Files and Type Defines
----------------------------------------------------------------------------**/
#include "blobs.h"

#include <stdio.h>

class DENORM;
struct INT_FX_RESULT_STRUCT;

#undef Min
#undef Max
#define FEAT_NAME_SIZE    80

// define trap errors which can be caused by this module
#define ILLEGAL_FEATURE_PARAM 1000
#define ILLEGAL_NUM_FEATURES  1001

// A character is described by multiple sets of extracted features.  Each
// set contains a number of features of a particular type, for example, a
// set of bays, or a set of closures, or a set of microfeatures.  Each
// feature consists of a number of parameters.  All features within a
// feature set contain the same number of parameters.  All circular
// parameters are required to be the first parameters in the feature.

struct PARAM_DESC {
  inT8 Circular;                   // TRUE if dimension wraps around
  inT8 NonEssential;               // TRUE if dimension not used in searches
  FLOAT32 Min;                     // low end of range for circular dimensions
  FLOAT32 Max;                     // high end of range for circular dimensions
  FLOAT32 Range;                   // Max - Min
  FLOAT32 HalfRange;               // (Max - Min)/2
  FLOAT32 MidRange;                // (Max + Min)/2
};

struct FEATURE_DESC_STRUCT {
  uinT16 NumParams;                // total # of params
  const char *ShortName;           // short name for feature
  const PARAM_DESC *ParamDesc;     // array - one per param
};
typedef FEATURE_DESC_STRUCT *FEATURE_DESC;

struct FEATURE_STRUCT {
  const FEATURE_DESC_STRUCT *Type;  // points to description of feature type
  FLOAT32 Params[1];                // variable size array - params for feature
};
typedef FEATURE_STRUCT *FEATURE;

struct FEATURE_SET_STRUCT {
  uinT16 NumFeatures;            // number of features in set
  uinT16 MaxNumFeatures;         // maximum size of feature set
  FEATURE Features[1];           // variable size array of features
};
typedef FEATURE_SET_STRUCT *FEATURE_SET;

// A generic character description as a char pointer. In reality, it will be
// a pointer to some data structure. Paired feature extractors/matchers need
// to agree on the data structure to be used, however, the high level
// classifier does not need to know the details of this data structure.
typedef char *CHAR_FEATURES;

/*----------------------------------------------------------------------
    Macros for defining the parameters of a new features
----------------------------------------------------------------------*/
#define StartParamDesc(Name)	\
const PARAM_DESC Name[] = {

#define DefineParam(Circular, NonEssential, Min, Max)	\
	{Circular, NonEssential, Min, Max,			\
	(Max) - (Min), (((Max) - (Min))/2.0), (((Max) + (Min))/2.0)},

#define EndParamDesc  };

/*----------------------------------------------------------------------
Macro for describing a new feature.  The parameters of the macro
are as follows:

DefineFeature (Name, NumLinear, NumCircular, ShortName, ParamName)
----------------------------------------------------------------------*/
#define DefineFeature(Name, NL, NC, SN, PN)		\
const FEATURE_DESC_STRUCT Name = {				\
	((NL) + (NC)), SN, PN};

/*----------------------------------------------------------------------
        Generic routines that work for all feature types
----------------------------------------------------------------------*/
BOOL8 AddFeature(FEATURE_SET FeatureSet, FEATURE Feature);

void FreeFeature(FEATURE Feature);

void FreeFeatureSet(FEATURE_SET FeatureSet);

FEATURE NewFeature(const FEATURE_DESC_STRUCT *FeatureDesc);

FEATURE_SET NewFeatureSet(int NumFeatures);

FEATURE ReadFeature(FILE *File, const FEATURE_DESC_STRUCT *FeatureDesc);

FEATURE_SET ReadFeatureSet(FILE *File, const FEATURE_DESC_STRUCT *FeatureDesc);

void WriteFeature(FEATURE Feature, STRING* str);

void WriteFeatureSet(FEATURE_SET FeatureSet, STRING* str);

#endif
