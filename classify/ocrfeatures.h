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
#include "funcdefs.h"
#include "tessclas.h"
#include "fxdefs.h"

#include <stdio.h>

#undef Min
#undef Max
#define FEAT_NAME_SIZE    80

/*define trap errors which can be caused by this module*/
#define ILLEGAL_FEATURE_PARAM 1000
#define ILLEGAL_NUM_FEATURES  1001

/* A character is described by multiple sets of extracted features.  Each
  set contains a number of features of a particular type, for example, a
  set of bays, or a set of closures, or a set of microfeatures.  Each
  feature consists of a number of parameters.  All features within a
  feature set contain the same number of parameters.  All circular
  parameters are required to be the first parameters in the feature.*/

typedef struct
{
  struct fds *Type;              /* points to description of feature type */
  FLOAT32 Params[1];             /* variable size array - params for feature */
} FEATURE_STRUCT;
typedef FEATURE_STRUCT *FEATURE;

typedef struct
{
  uinT16 NumFeatures;            /* number of features in set */
  uinT16 MaxNumFeatures;         /* maximum size of feature set */
  FEATURE Features[1];           /* variable size array of features */
} FEATURE_SET_STRUCT;
typedef FEATURE_SET_STRUCT *FEATURE_SET;

/* Define various function types which will be needed for "class methods"*/
typedef FEATURE (*FEAT_FUNC) ();
typedef FEATURE_SET (*FX_FUNC) (TBLOB *, LINE_STATS *);
typedef FLOAT32 (*PENALTY_FUNC) ();

typedef struct
{
  inT8 Circular;                 /* TRUE if dimension wraps around */
  inT8 NonEssential;             /* TRUE if dimension not used in searches */
  FLOAT32 Min;                   /* low end of range for circular dimensions */
  FLOAT32 Max;                   /* high end of range for circular dimensions */
  FLOAT32 Range;                 /* Max - Min */
  FLOAT32 HalfRange;             /* (Max - Min)/2 */
  FLOAT32 MidRange;              /* (Max + Min)/2 */
} PARAM_DESC;

typedef struct fds
{
  uinT16 NumParams;              /* total # of params */
  uinT8 NumLinearParams;         /* # of linear params */
  uinT8 NumCircularParams;       /* # of linear params */
  uinT8 MinFeatPerChar;          /* min # of feats allowed */
  uinT8 MaxFeatPerChar;          /* max # of feats allowed */
  char LongName[FEAT_NAME_SIZE]; /* long name for feature */
  char ShortName[FEAT_NAME_SIZE];/* short name for feature */
  PARAM_DESC *ParamDesc;         /* array - one per param */
} FEATURE_DESC_STRUCT;
                                 /* one per feature type */
typedef FEATURE_DESC_STRUCT *FEATURE_DESC;

typedef struct fxs
{
  FX_FUNC Extractor;             /* func to extract features */
} FEATURE_EXT_STRUCT;

/*----------------------------------------------------------------------
    Macros for defining the parameters of a new features
----------------------------------------------------------------------*/
#define StartParamDesc(Name)	\
static PARAM_DESC Name[] = {

#define DefineParam(Circular, NonEssential, Min, Max)	\
	{Circular, NonEssential, Min, Max,			\
	(Max) - (Min), (((Max) - (Min))/2.0), (((Max) + (Min))/2.0)},

#define EndParamDesc  };

/*----------------------------------------------------------------------
Macro for describing a new feature.  The parameters of the macro
are as follows:

DefineFeature (Name, NumLinear, NumCircular,
          MinFeatPerChar, MaxFeatPerChar,
          LongName, ShortName, ParamName)
----------------------------------------------------------------------*/
#define DefineFeature(Name, NL, NC, Min, Max, LN, SN, PN)		\
FEATURE_DESC_STRUCT Name = {						\
	((NL) + (NC)), NL, NC, Min, Max, LN, SN, PN};

#define DefineFeatureExt(Name, E) FEATURE_EXT_STRUCT Name = {E};

/*----------------------------------------------------------------------
        Generic routines that work for all feature types
----------------------------------------------------------------------*/
BOOL8 AddFeature(FEATURE_SET FeatureSet, FEATURE Feature);

void FreeFeature(FEATURE Feature);

void FreeFeatureSet(FEATURE_SET FeatureSet);

FEATURE NewFeature(FEATURE_DESC FeatureDesc);

FEATURE_SET NewFeatureSet(int NumFeatures);

FEATURE ReadFeature(FILE *File, FEATURE_DESC FeatureDesc);

FEATURE_SET ReadFeatureSet(FILE *File, FEATURE_DESC FeatureDesc);

void WriteFeature(FILE *File, FEATURE Feature);

void WriteFeatureSet(FILE *File, FEATURE_SET FeatureSet);

void WriteOldParamDesc(FILE *File, FEATURE_DESC FeatureDesc);
#endif
