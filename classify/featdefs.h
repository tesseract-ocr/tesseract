/******************************************************************************
 **	Filename:    featdefs.h
 **	Purpose:     Definitions of currently defined feature types.
 **	Author:      Dan Johnson
 **	History:     Mon May 21 08:28:01 1990, DSJ, Created.
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
#ifndef   FEATDEFS_H
#define   FEATDEFS_H

/**----------------------------------------------------------------------------
          Include Files and Type Defines
----------------------------------------------------------------------------**/
#include "ocrfeatures.h"

/* Enumerate the different types of features currently defined. */
#define NUM_FEATURE_TYPES 4

/* define error traps which can be triggered by this module.*/
#define ILLEGAL_SHORT_NAME  2000

/* A character is described by multiple sets of extracted features.  Each
  set contains a number of features of a particular type, for example, a
  set of bays, or a set of closures, or a set of microfeatures.  Each
  feature consists of a number of parameters.  All features within a
  feature set contain the same number of parameters.*/

typedef struct
{
  uinT32 NumFeatureSets;
  FEATURE_SET FeatureSets[NUM_FEATURE_TYPES];
} CHAR_DESC_STRUCT;
typedef CHAR_DESC_STRUCT *CHAR_DESC;

typedef struct
{
  uinT32 NumFeatureTypes;
  FEATURE_DESC FeatureDesc[NUM_FEATURE_TYPES];
  FEATURE_EXT_STRUCT* FeatureExtractors[NUM_FEATURE_TYPES];
  int FeatureEnabled[NUM_FEATURE_TYPES];
} FEATURE_DEFS_STRUCT;
typedef FEATURE_DEFS_STRUCT *FEATURE_DEFS;

/*----------------------------------------------------------------------
    Generic functions for manipulating character descriptions
----------------------------------------------------------------------*/
void FreeCharDescription(CHAR_DESC CharDesc);

CHAR_DESC NewCharDescription();

void WriteCharDescription(FILE *File, CHAR_DESC CharDesc);

CHAR_DESC ReadCharDescription(FILE *File);

int ShortNameToFeatureType(const char *ShortName);

/**----------------------------------------------------------------------------
        Global Data Definitions and Declarations
----------------------------------------------------------------------------**/
extern FEATURE_DEFS_STRUCT FeatureDefs;
#endif
