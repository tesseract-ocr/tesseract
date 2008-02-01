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

/**----------------------------------------------------------------------------
          Include Files and Type Defines
----------------------------------------------------------------------------**/
#include "debug.h"
#include "intproto.h"
#include "cutoffs.h"

typedef struct
{
  FLOAT32 Rating;
  UINT8 Config;
  UINT8 Config2;
  UINT16 FeatureMisses;
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

typedef UINT8 CLASS_NORMALIZATION_ARRAY[MAX_NUM_CLASSES];

/*----------------------------------------------------------------------------
            Variables
-----------------------------------------------------------------------------*/
extern int AdaptProtoThresh;
extern int AdaptFeatureThresh;

/**----------------------------------------------------------------------------
          Public Function Prototypes
----------------------------------------------------------------------------**/
int ClassPruner(INT_TEMPLATES IntTemplates,
                INT16 NumFeatures,
                INT_FEATURE_ARRAY Features,
                CLASS_NORMALIZATION_ARRAY NormalizationFactors,
                CLASS_CUTOFF_ARRAY ExpectedNumFeatures,
                CLASS_PRUNER_RESULTS Results,
                int Debug);

void IntegerMatcher(INT_CLASS ClassTemplate,
                    BIT_VECTOR ProtoMask,
                    BIT_VECTOR ConfigMask,
                    UINT16 BlobLength,
                    INT16 NumFeatures,
                    INT_FEATURE_ARRAY Features,
                    UINT8 NormalizationFactor,
                    INT_RESULT Result,
                    int Debug);

int FindGoodProtos(INT_CLASS ClassTemplate,
                   BIT_VECTOR ProtoMask,
                   BIT_VECTOR ConfigMask,
                   UINT16 BlobLength,
                   INT16 NumFeatures,
                   INT_FEATURE_ARRAY Features,
                   PROTO_ID *ProtoArray,
                   int Debug);

int FindBadFeatures(INT_CLASS ClassTemplate,
                    BIT_VECTOR ProtoMask,
                    BIT_VECTOR ConfigMask,
                    UINT16 BlobLength,
                    INT16 NumFeatures,
                    INT_FEATURE_ARRAY Features,
                    FEATURE_ID *FeatureArray,
                    int Debug);

void InitIntegerMatcher();

void InitIntegerMatcherVars();

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
UINT8 ProtoEvidence[MAX_NUM_PROTOS][MAX_PROTO_INDEX]);

void IMClearFeatureEvidenceTable (UINT8 FeatureEvidence[MAX_NUM_CONFIGS],
int NumConfigs);

void IMDebugConfiguration(INT_FEATURE FeatureNum,
                          UINT16 ActualProtoNum,
                          UINT8 Evidence,
                          BIT_VECTOR ConfigMask,
                          UINT32 ConfigWord);

void IMDebugConfigurationSum(INT_FEATURE FeatureNum,
                             UINT8 *FeatureEvidence,
                             INT32 ConfigCount);

int IMUpdateTablesForFeature (INT_CLASS ClassTemplate,
BIT_VECTOR ProtoMask,
BIT_VECTOR ConfigMask,
int FeatureNum,
INT_FEATURE Feature,
UINT8 FeatureEvidence[MAX_NUM_CONFIGS],
int SumOfFeatureEvidence[MAX_NUM_CONFIGS],
UINT8
ProtoEvidence[MAX_NUM_PROTOS][MAX_PROTO_INDEX],
int Debug);

#ifndef GRAPHICS_DISABLED
void IMDebugFeatureProtoError (INT_CLASS ClassTemplate,
BIT_VECTOR ProtoMask,
BIT_VECTOR ConfigMask,
int SumOfFeatureEvidence[MAX_NUM_CONFIGS],
UINT8
ProtoEvidence[MAX_NUM_PROTOS][MAX_PROTO_INDEX],
INT16 NumFeatures, int Debug);

void IMDisplayProtoDebugInfo (INT_CLASS ClassTemplate,
BIT_VECTOR ProtoMask,
BIT_VECTOR ConfigMask,
UINT8
ProtoEvidence[MAX_NUM_PROTOS][MAX_PROTO_INDEX],
int Debug);

void IMDisplayFeatureDebugInfo(INT_CLASS ClassTemplate,
                               BIT_VECTOR ProtoMask,
                               BIT_VECTOR ConfigMask,
                               INT16 NumFeatures,
                               INT_FEATURE_ARRAY Features,
                               int Debug);
#endif

void IMUpdateSumOfProtoEvidences (INT_CLASS ClassTemplate,
BIT_VECTOR ConfigMask,
int SumOfFeatureEvidence[MAX_NUM_CONFIGS],
UINT8
ProtoEvidence[MAX_NUM_PROTOS]
[MAX_PROTO_INDEX], INT16 NumFeatures);

void IMNormalizeSumOfEvidences (INT_CLASS ClassTemplate,
int SumOfFeatureEvidence[MAX_NUM_CONFIGS],
INT16 NumFeatures, INT32 used_features);

int IMFindBestMatch (INT_CLASS ClassTemplate,
int SumOfFeatureEvidence[MAX_NUM_CONFIGS],
UINT16 BlobLength,
UINT8 NormalizationFactor, INT_RESULT Result);

#ifndef GRAPHICS_DISABLED
void IMDebugBestMatch(int BestMatch,
                      INT_RESULT Result,
                      UINT16 BlobLength,
                      UINT8 NormalizationFactor);
#endif

void HeapSort (int n, register int ra[], register int rb[]);

/**----------------------------------------------------------------------------
        Global Data Definitions and Declarations
----------------------------------------------------------------------------**/
extern int IntegerMatcherMultiplier;

extern UINT32 EvidenceMultMask;
#endif
