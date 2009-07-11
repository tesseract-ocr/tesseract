/******************************************************************************
 **	Filename:    intmatcher.h
 **	Purpose:     Interface to high level generic classifier routines.
 **	Author:      Robert Moss
 **	History:     Wed Feb 13 15:24:15 MST 1991, RWM, Created.
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
#ifndef   INTMATCHER_H
#define   INTMATCHER_H

#include "varable.h"

// Character fragments could be present in the trained templaes
// but turned on/off on the language-by-language basis or depending
// on particular properties of the corpus (e.g. when we expect the
// images to have low exposure).
extern BOOL_VAR_H(disable_character_fragments, FALSE,
                  "Do not include character fragments in the"
                  " results of the classifier");

extern INT_VAR_H(classify_integer_matcher_multiplier, 14,
                 "Integer Matcher Multiplier  0-255:   ");


/**----------------------------------------------------------------------------
          Include Files and Type Defines
----------------------------------------------------------------------------**/
#include "intproto.h"
#include "cutoffs.h"

typedef struct
{
  FLOAT32 Rating;
  uinT8 Config;
  uinT8 Config2;
  uinT16 FeatureMisses;
}


INT_RESULT_STRUCT, *INT_RESULT;

typedef struct
{
  FLOAT32 Rating;
  INT_RESULT_STRUCT IMResult;
  CLASS_ID Class;
}


CP_RESULT_STRUCT;

/*typedef CLASS_ID CLASS_PRUNER_RESULTS [MAX_NUM_CLASSES];	*/
typedef CP_RESULT_STRUCT CLASS_PRUNER_RESULTS[MAX_NUM_CLASSES];

typedef uinT8 CLASS_NORMALIZATION_ARRAY[MAX_NUM_CLASSES];

/*----------------------------------------------------------------------------
            Variables
-----------------------------------------------------------------------------*/

extern INT_VAR_H(classify_adapt_proto_thresh, 230,
                 "Threshold for good protos during adaptive 0-255:   ");

extern INT_VAR_H(classify_adapt_feature_thresh, 230,
                 "Threshold for good features during adaptive 0-255:   ");

/**----------------------------------------------------------------------------
          Public Function Prototypes
----------------------------------------------------------------------------**/

void IntegerMatcher(INT_CLASS ClassTemplate,
                    BIT_VECTOR ProtoMask,
                    BIT_VECTOR ConfigMask,
                    uinT16 BlobLength,
                    inT16 NumFeatures,
                    INT_FEATURE_ARRAY Features,
                    uinT8 NormalizationFactor,
                    INT_RESULT Result,
                    int Debug);

int FindGoodProtos(INT_CLASS ClassTemplate,
                   BIT_VECTOR ProtoMask,
                   BIT_VECTOR ConfigMask,
                   uinT16 BlobLength,
                   inT16 NumFeatures,
                   INT_FEATURE_ARRAY Features,
                   PROTO_ID *ProtoArray,
                   int Debug);

int FindBadFeatures(INT_CLASS ClassTemplate,
                    BIT_VECTOR ProtoMask,
                    BIT_VECTOR ConfigMask,
                    uinT16 BlobLength,
                    inT16 NumFeatures,
                    INT_FEATURE_ARRAY Features,
                    FEATURE_ID *FeatureArray,
                    int Debug);

void InitIntegerMatcher();

void PrintIntMatcherStats(FILE *f);

void SetProtoThresh(FLOAT32 Threshold);

void SetFeatureThresh(FLOAT32 Threshold);

void SetBaseLineMatch();

void SetCharNormMatch();

/**----------------------------------------------------------------------------
          Private Function Prototypes
----------------------------------------------------------------------------**/
void IMClearTables (INT_CLASS ClassTemplate,
int SumOfFeatureEvidence[MAX_NUM_CONFIGS],
uinT8 ProtoEvidence[MAX_NUM_PROTOS][MAX_PROTO_INDEX]);

void IMClearFeatureEvidenceTable (uinT8 FeatureEvidence[MAX_NUM_CONFIGS],
int NumConfigs);

void IMDebugConfiguration(INT_FEATURE FeatureNum,
                          uinT16 ActualProtoNum,
                          uinT8 Evidence,
                          BIT_VECTOR ConfigMask,
                          uinT32 ConfigWord);

void IMDebugConfigurationSum(INT_FEATURE FeatureNum,
                             uinT8 *FeatureEvidence,
                             inT32 ConfigCount);

int IMUpdateTablesForFeature (INT_CLASS ClassTemplate,
BIT_VECTOR ProtoMask,
BIT_VECTOR ConfigMask,
int FeatureNum,
INT_FEATURE Feature,
uinT8 FeatureEvidence[MAX_NUM_CONFIGS],
int SumOfFeatureEvidence[MAX_NUM_CONFIGS],
uinT8
ProtoEvidence[MAX_NUM_PROTOS][MAX_PROTO_INDEX],
int Debug);

#ifndef GRAPHICS_DISABLED
void IMDebugFeatureProtoError (INT_CLASS ClassTemplate,
BIT_VECTOR ProtoMask,
BIT_VECTOR ConfigMask,
int SumOfFeatureEvidence[MAX_NUM_CONFIGS],
uinT8
ProtoEvidence[MAX_NUM_PROTOS][MAX_PROTO_INDEX],
inT16 NumFeatures, int Debug);

void IMDisplayProtoDebugInfo (INT_CLASS ClassTemplate,
BIT_VECTOR ProtoMask,
BIT_VECTOR ConfigMask,
uinT8
ProtoEvidence[MAX_NUM_PROTOS][MAX_PROTO_INDEX],
int Debug);

void IMDisplayFeatureDebugInfo(INT_CLASS ClassTemplate,
                               BIT_VECTOR ProtoMask,
                               BIT_VECTOR ConfigMask,
                               inT16 NumFeatures,
                               INT_FEATURE_ARRAY Features,
                               int Debug);
#endif

void IMUpdateSumOfProtoEvidences (INT_CLASS ClassTemplate,
BIT_VECTOR ConfigMask,
int SumOfFeatureEvidence[MAX_NUM_CONFIGS],
uinT8
ProtoEvidence[MAX_NUM_PROTOS]
[MAX_PROTO_INDEX], inT16 NumFeatures);

void IMNormalizeSumOfEvidences (INT_CLASS ClassTemplate,
int SumOfFeatureEvidence[MAX_NUM_CONFIGS],
inT16 NumFeatures, inT32 used_features);

int IMFindBestMatch (INT_CLASS ClassTemplate,
int SumOfFeatureEvidence[MAX_NUM_CONFIGS],
uinT16 BlobLength,
uinT8 NormalizationFactor, INT_RESULT Result);

#ifndef GRAPHICS_DISABLED
void IMDebugBestMatch(int BestMatch,
                      INT_RESULT Result,
                      uinT16 BlobLength,
                      uinT8 NormalizationFactor);
#endif

void HeapSort (int n, register int ra[], register int rb[]);

/**----------------------------------------------------------------------------
        Global Data Definitions and Declarations
----------------------------------------------------------------------------**/
extern uinT32 EvidenceMultMask;
#endif
