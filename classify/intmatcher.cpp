/******************************************************************************
 **      Filename:    intmatcher.c
 **      Purpose:     Generic high level classification routines.
 **      Author:      Robert Moss
 **      History:     Wed Feb 13 17:35:28 MST 1991, RWM, Created.
 **                   Mon Mar 11 16:33:02 MST 1991, RWM, Modified to add
 **                        support for adaptive matching.
 **      (c) Copyright Hewlett-Packard Company, 1988.
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
#include "intmatcher.h"
#include "tordvars.h"
#include "callcpp.h"
#include <math.h>

#define CLASS_MASK_SIZE ((MAX_NUM_CLASSES*NUM_BITS_PER_CLASS \
		+BITS_PER_WERD-1)/BITS_PER_WERD)

/**----------------------------------------------------------------------------
                    Global Data Definitions and Declarations
----------------------------------------------------------------------------**/
#define  SE_TABLE_BITS    9
#define  SE_TABLE_SIZE  512
#define TEMPLATE_CACHE 2
static UINT8 SimilarityEvidenceTable[SE_TABLE_SIZE];
static UINT8 offset_table[256] = {
  255, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
  4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
  5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
  4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
  6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
  4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
  5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
  4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
  7, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
  4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
  5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
  4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
  6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
  4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
  5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
  4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0
};
static UINT8 next_table[256] = {
  0, 0, 0, 0x2, 0, 0x4, 0x4, 0x6, 0, 0x8, 0x8, 0x0a, 0x08, 0x0c, 0x0c, 0x0e,
  0, 0x10, 0x10, 0x12, 0x10, 0x14, 0x14, 0x16, 0x10, 0x18, 0x18, 0x1a, 0x18,
  0x1c, 0x1c, 0x1e,
  0, 0x20, 0x20, 0x22, 0x20, 0x24, 0x24, 0x26, 0x20, 0x28, 0x28, 0x2a, 0x28,
  0x2c, 0x2c, 0x2e,
  0x20, 0x30, 0x30, 0x32, 0x30, 0x34, 0x34, 0x36, 0x30, 0x38, 0x38, 0x3a,
  0x38, 0x3c, 0x3c, 0x3e,
  0, 0x40, 0x40, 0x42, 0x40, 0x44, 0x44, 0x46, 0x40, 0x48, 0x48, 0x4a, 0x48,
  0x4c, 0x4c, 0x4e,
  0x40, 0x50, 0x50, 0x52, 0x50, 0x54, 0x54, 0x56, 0x50, 0x58, 0x58, 0x5a,
  0x58, 0x5c, 0x5c, 0x5e,
  0x40, 0x60, 0x60, 0x62, 0x60, 0x64, 0x64, 0x66, 0x60, 0x68, 0x68, 0x6a,
  0x68, 0x6c, 0x6c, 0x6e,
  0x60, 0x70, 0x70, 0x72, 0x70, 0x74, 0x74, 0x76, 0x70, 0x78, 0x78, 0x7a,
  0x78, 0x7c, 0x7c, 0x7e,
  0, 0x80, 0x80, 0x82, 0x80, 0x84, 0x84, 0x86, 0x80, 0x88, 0x88, 0x8a, 0x88,
  0x8c, 0x8c, 0x8e,
  0x80, 0x90, 0x90, 0x92, 0x90, 0x94, 0x94, 0x96, 0x90, 0x98, 0x98, 0x9a,
  0x98, 0x9c, 0x9c, 0x9e,
  0x80, 0xa0, 0xa0, 0xa2, 0xa0, 0xa4, 0xa4, 0xa6, 0xa0, 0xa8, 0xa8, 0xaa,
  0xa8, 0xac, 0xac, 0xae,
  0xa0, 0xb0, 0xb0, 0xb2, 0xb0, 0xb4, 0xb4, 0xb6, 0xb0, 0xb8, 0xb8, 0xba,
  0xb8, 0xbc, 0xbc, 0xbe,
  0x80, 0xc0, 0xc0, 0xc2, 0xc0, 0xc4, 0xc4, 0xc6, 0xc0, 0xc8, 0xc8, 0xca,
  0xc8, 0xcc, 0xcc, 0xce,
  0xc0, 0xd0, 0xd0, 0xd2, 0xd0, 0xd4, 0xd4, 0xd6, 0xd0, 0xd8, 0xd8, 0xda,
  0xd8, 0xdc, 0xdc, 0xde,
  0xc0, 0xe0, 0xe0, 0xe2, 0xe0, 0xe4, 0xe4, 0xe6, 0xe0, 0xe8, 0xe8, 0xea,
  0xe8, 0xec, 0xec, 0xee,
  0xe0, 0xf0, 0xf0, 0xf2, 0xf0, 0xf4, 0xf4, 0xf6, 0xf0, 0xf8, 0xf8, 0xfa,
  0xf8, 0xfc, 0xfc, 0xfe
};
static int cp_maxes[128] = {
  100,
  100, 100, 100, 100, 100, 100, 100, 100,
  100, 100, 100, 100, 100, 100, 100, 100,
  100, 100, 100, 100, 100, 100, 100, 100,
  100, 100, 100, 100, 100, 100, 100, 100,
  194,                           //!
  100,                           //"
  182,                           //#
  224,                           //$
  203,                           //%
  242,                           //&
  245,                           //'
  226,                           //(
  190,                           //)
  244,                           //*
  195,                           //+
  254,                           //,
  253,                           //-
  253,                           //.
  206,                           ///
  253,                           //0
  234,                           //1
  252,                           //2
  246,                           //3
  253,                           //4
  160,                           //5
  202,                           //6
  199,                           //7
  171,                           //8
  227,                           //9
  208,                           //:
  188,                           //;
  60,                            //<
  221,                           //=
  138,                           //>
  108,                           //?
  98,                            //@
  251,                           //A
  214,                           //B
  230,                           //C
  252,                           //D
  237,                           //E
  217,                           //F
  233,                           //G
  174,                           //H
  216,                           //I
  210,                           //J
  252,                           //K
  253,                           //L
  233,                           //M
  243,                           //N
  240,                           //O
  230,                           //P
  167,                           //Q
  248,                           //R
  250,                           //S
  232,                           //T
  209,                           //U
  193,                           //V
  254,                           //W
  146,                           //X
  198,                           //Y
  107,                           //Z
  167,                           //[
  163,                           //
  73,                            //]
  16,                            //^
  199,                           //_
  162,                           //`
  251,                           //a
  250,                           //b
  254,                           //c
  253,                           //d
  252,                           //e
  253,                           //f
  248,                           //g
  251,                           //h
  254,                           //i
  201,                           //j
  224,                           //k
  253,                           //l
  242,                           //m
  254,                           //n
  254,                           //o
  253,                           //p
  246,                           //q
  254,                           //r
  254,                           //s
  254,                           //t
  245,                           //u
  221,                           //v
  230,                           //w
  251,                           //x
  243,                           //y
  133,                           //z
  35,                            //{
  100,                           //|
  143,                           //}
  100,                           //~
  100
};

static float cp_ratios[128] = {
  1.5f,
  1.5f, 1.5f, 1.5f, 1.5f, 1.5f, 1.5f, 1.5f, 1.5f,
  1.5f, 1.5f, 1.5f, 1.5f, 1.5f, 1.5f, 1.5f, 1.5f,
  1.5f, 1.5f, 1.5f, 1.5f, 1.5f, 1.5f, 1.5f, 1.5f,
  1.5f, 1.5f, 1.5f, 1.5f, 1.5f, 1.5f, 1.5f, 1.5f,
  2.24775,                       //!
  1.5,                           //"
  1.90376,                       //#
  1.61443,                       //$
  1.87857,                       //%
  2.29167,                       //&
  7.4,                           //'
  4.7,                           //(
  9.4,                           //)
  2.13014,                       //*
  1.53175,                       //+
  2.86957,                       //,
  7.4,                           //-
  7.4,                           //.
  9.4,                           ///
  8.1,                           //0
  12.6,                          //1
  2.7439,                        //2
  4.22222,                       //3
  2.57447,                       //4
  2.93902,                       //5
  4.23684,                       //6
  6,                             //7
  2.78889,                       //8
  3.55,                          //9
  8.5,                           //:
  2.4,                           //;
  1.5,                           //<
  1.94737,                       //=
  1.89394,                       //>
  1.5,                           //?
  1.5,                           //@
  3.125,                         //A
  5.5,                           //B
  6.1,                           //C
  6,                             //D
  2.78205,                       //E
  2.03763,                       //F
  2.73256,                       //G
  2.57692,                       //H
  11.8,                          //I
  7.1,                           //J
  1.85227,                       //K
  7.4,                           //L
  2.26056,                       //M
  2.46078,                       //N
  6.85714,                       //O
  3.45238,                       //P
  2.47222,                       //Q
  3.74,                          //R
  10.2,                          //S
  3.08065,                       //T
  6.1,                           //U
  9.5,                           //V
  7.1,                           //W
  7.9,                           //X
  2.55714,                       //Y
  7.7,                           //Z
  2,                             //[
  1.5,                           //
  2.55714,                       //]
  1.5,                           //^
  1.80065,                       //_
  1.69512,                       //`
  5.34,                          //a
  7.3,                           //b
  6.43333,                       //c
  4.10606,                       //d
  4.41667,                       //e
  12.6,                          //f
  3.7093,                        //g
  2.38889,                       //h
  5.5,                           //i
  4.03125,                       //j
  2.24561,                       //k
  11.5,                          //l
  3.5,                           //m
  5.63333,                       //n
  11,                            //o
  2.52667,                       //p
  2.1129,                        //q
  6.56667,                       //r
  6.42857,                       //s
  11.4,                          //t
  3.62,                          //u
  2.77273,                       //v
  2.90909,                       //w
  6.5,                           //x
  4.98387,                       //y
  2.92857,                       //z
  1.5,                           //{
  1.5,                           //|
  2.02128,                       //}
  1.5,                           //~
  1.5f
};
static INT8 miss_table[256] = {
  4, 4, 4, 3, 4, 4, 4, 3, 4, 4, 4, 3, 3, 3, 3, 2,
  4, 4, 4, 3, 4, 4, 4, 3, 4, 4, 4, 3, 3, 3, 3, 2,
  4, 4, 4, 3, 4, 4, 4, 3, 4, 4, 4, 3, 3, 3, 3, 2,
  3, 3, 3, 2, 3, 3, 3, 2, 3, 3, 3, 2, 2, 2, 2, 1,
  4, 4, 4, 3, 4, 4, 4, 3, 4, 4, 4, 3, 3, 3, 3, 2,
  4, 4, 4, 3, 4, 4, 4, 3, 4, 4, 4, 3, 3, 3, 3, 2,
  4, 4, 4, 3, 4, 4, 4, 3, 4, 4, 4, 3, 3, 3, 3, 2,
  3, 3, 3, 2, 3, 3, 3, 2, 3, 3, 3, 2, 2, 2, 2, 1,
  4, 4, 4, 3, 4, 4, 4, 3, 4, 4, 4, 3, 3, 3, 3, 2,
  4, 4, 4, 3, 4, 4, 4, 3, 4, 4, 4, 3, 3, 3, 3, 2,
  4, 4, 4, 3, 4, 4, 4, 3, 4, 4, 4, 3, 3, 3, 3, 2,
  3, 3, 3, 2, 3, 3, 3, 2, 3, 3, 3, 2, 2, 2, 2, 1,
  3, 3, 3, 2, 3, 3, 3, 2, 3, 3, 3, 2, 2, 2, 2, 1,
  3, 3, 3, 2, 3, 3, 3, 2, 3, 3, 3, 2, 2, 2, 2, 1,
  3, 3, 3, 2, 3, 3, 3, 2, 3, 3, 3, 2, 2, 2, 2, 1,
  2, 2, 2, 1, 2, 2, 2, 1, 2, 2, 2, 1, 1, 1, 1, 0
};

static UINT32 EvidenceTableMask;

static UINT32 MultTruncShiftBits;

static UINT32 TableTruncShiftBits;

UINT32 EvidenceMultMask;

static INT16 LocalMatcherMultiplier;

make_int_var (ClassPrunerThreshold, 229, MakeClassPrunerThreshold,
16, 20, SetClassPrunerThreshold,
"Class Pruner Threshold 0-255:        ");

make_int_var (ClassPrunerMultiplier, 15, MakeClassPrunerMultiplier,
16, 21, SetClassPrunerMultiplier,
"Class Pruner Multiplier 0-255:       ");

make_int_var (IntegerMatcherMultiplier, 14, MakeIntegerMatcherMultiplier,
16, 22, SetIntegerMatcherMultiplier,
"Integer Matcher Multiplier  0-255:   ");

make_int_var (IntThetaFudge, 128, MakeIntThetaFudge,
16, 23, SetIntThetaFudge,
"Integer Matcher Theta Fudge 0-255:   ");

make_float_var (CPCutoffStrength, 0.15, MakeCPCutoffStrength,
16, 24, SetCPCutoffStrength,
"Class Pruner CutoffStrength:         ");

make_int_var (EvidenceTableBits, 9, MakeEvidenceTableBits,
16, 25, SetEvidenceTableBits,
"Bits in Similarity to Evidence Lookup  8-9:   ");

make_int_var (IntEvidenceTruncBits, 14, MakeIntEvidenceTruncBits,
16, 26, SetIntEvidenceTruncBits,
"Integer Evidence Truncation Bits (Distance) 8-14:   ");

make_float_var (SEExponentialMultiplier, 0, MakeSEExponentialMultiplier,
16, 27, SetSEExponentialMultiplier,
"Similarity to Evidence Table Exponential Multiplier: ");

make_float_var (SimilarityCenter, 0.0075, MakeSimilarityCenter,
16, 28, SetSimilarityCenter, "Center of Similarity Curve: ");

make_int_var (AdaptProtoThresh, 230, MakeAdaptProtoThresh,
16, 29, SetAdaptProtoThresh,
"Threshold for good protos during adaptive 0-255:   ");

make_int_var (AdaptFeatureThresh, 230, MakeAdaptFeatureThresh,
16, 30, SetAdaptFeatureThresh,
"Threshold for good features during adaptive 0-255:   ");
//extern int display_ratings;
//extern "C" int                                        newcp_ratings_on;
//extern "C" double                             newcp_prune_threshold;
//extern "C" double                             tessedit_cp_ratio;
//extern "C" int                                        feature_prune_percentile;
//extern INT32                                  cp_maps[4];

int protoword_lookups;
int zero_protowords;
int proto_shifts;
int set_proto_bits;
int config_shifts;
int set_config_bits;

/**----------------------------------------------------------------------------
              Public Code
----------------------------------------------------------------------------**/
/*---------------------------------------------------------------------------*/
int ClassPruner(INT_TEMPLATES IntTemplates,
                INT16 NumFeatures,
                INT_FEATURE_ARRAY Features,
                CLASS_NORMALIZATION_ARRAY NormalizationFactors,
                CLASS_CUTOFF_ARRAY ExpectedNumFeatures,
                CLASS_PRUNER_RESULTS Results,
                int Debug) {
/*
 **      Parameters:
 **              IntTemplates           Class pruner tables
 **              NumFeatures            Number of features in blob
 **              Features               Array of features
 **              NormalizationFactors   Array of fudge factors from blob
 **                                     normalization process
 **                                     (by CLASS_INDEX)
 **              ExpectedNumFeatures    Array of expected number of features
 **                                     for each class
 **                                     (by CLASS_INDEX)
 **              Results                Sorted Array of pruned classes
 **                                     (by CLASS_ID)
 **              Debug                  Debugger flag: 1=debugger on
 **      Globals:
 **              ClassPrunerThreshold   Cutoff threshold
 **              ClassPrunerMultiplier  Normalization factor multiplier
 **      Operation:
 **              Prune the classes using a modified fast match table.
 **              Return a sorted list of classes along with the number
 **              of pruned classes in that list.
 **      Return: Number of pruned classes.
 **      Exceptions: none
 **      History: Tue Feb 19 10:24:24 MST 1991, RWM, Created.
 */
  UINT32 PrunerWord;
  INT32 class_index;             //index to class
  int Word;
  UINT32 *BasePrunerAddress;
  UINT32 feature_address;        //current feature index
  INT_FEATURE feature;           //current feature
  CLASS_PRUNER *ClassPruner;
  int PrunerSet;
  int NumPruners;
  INT32 feature_index;           //current feature

  static INT32 ClassCount[MAX_NUM_CLASSES - 1];
  static INT16 NormCount[MAX_NUM_CLASSES - 1];
  static INT16 SortKey[MAX_NUM_CLASSES];
  static UINT8 SortIndex[MAX_NUM_CLASSES];
  CLASS_INDEX Class;
  int out_class;
  int MaxNumClasses;
  int MaxCount;
  int NumClasses;
  FLOAT32 max_rating;            //max allowed rating
  INT32 *ClassCountPtr;
  INT8 classch;

  MaxNumClasses = NumClassesIn (IntTemplates);

  /* Clear Class Counts */
  ClassCountPtr = &(ClassCount[0]);
  for (Class = 0; Class < MaxNumClasses; Class++) {
    *ClassCountPtr++ = 0;
  }

  /* Update Class Counts */
  NumPruners = NumClassPrunersIn (IntTemplates);
  for (feature_index = 0; feature_index < NumFeatures; feature_index++) {
    feature = &Features[feature_index];
    feature->CP_misses = 0;
    feature_address = (((feature->X * NUM_CP_BUCKETS >> 8) * NUM_CP_BUCKETS
      +
      (feature->Y * NUM_CP_BUCKETS >> 8)) *
      NUM_CP_BUCKETS +
      (feature->Theta * NUM_CP_BUCKETS >> 8)) << 1;
    ClassPruner = ClassPrunersFor (IntTemplates);
    class_index = 0;
    for (PrunerSet = 0; PrunerSet < NumPruners; PrunerSet++, ClassPruner++) {
      BasePrunerAddress = (UINT32 *) (*ClassPruner) + feature_address;

      for (Word = 0; Word < WERDS_PER_CP_VECTOR; Word++) {
        PrunerWord = *BasePrunerAddress++;
        ClassCount[class_index++] += cp_maps[PrunerWord & 3];
        PrunerWord >>= 2;
        ClassCount[class_index++] += cp_maps[PrunerWord & 3];
        PrunerWord >>= 2;
        ClassCount[class_index++] += cp_maps[PrunerWord & 3];
        PrunerWord >>= 2;
        ClassCount[class_index++] += cp_maps[PrunerWord & 3];
        PrunerWord >>= 2;
        ClassCount[class_index++] += cp_maps[PrunerWord & 3];
        PrunerWord >>= 2;
        ClassCount[class_index++] += cp_maps[PrunerWord & 3];
        PrunerWord >>= 2;
        ClassCount[class_index++] += cp_maps[PrunerWord & 3];
        PrunerWord >>= 2;
        ClassCount[class_index++] += cp_maps[PrunerWord & 3];
        PrunerWord >>= 2;
        ClassCount[class_index++] += cp_maps[PrunerWord & 3];
        PrunerWord >>= 2;
        ClassCount[class_index++] += cp_maps[PrunerWord & 3];
        PrunerWord >>= 2;
        ClassCount[class_index++] += cp_maps[PrunerWord & 3];
        PrunerWord >>= 2;
        ClassCount[class_index++] += cp_maps[PrunerWord & 3];
        PrunerWord >>= 2;
        ClassCount[class_index++] += cp_maps[PrunerWord & 3];
        PrunerWord >>= 2;
        ClassCount[class_index++] += cp_maps[PrunerWord & 3];
        PrunerWord >>= 2;
        ClassCount[class_index++] += cp_maps[PrunerWord & 3];
        PrunerWord >>= 2;
        ClassCount[class_index++] += cp_maps[PrunerWord & 3];
      }
    }
  }

  /* Adjust Class Counts for Number of Expected Features */
  for (Class = 0; Class < MaxNumClasses; Class++)
    if (NumFeatures < ExpectedNumFeatures[Class])
      ClassCount[Class] =
        (int) (((FLOAT32) (ClassCount[Class] * NumFeatures)) /
        (NumFeatures +
        CPCutoffStrength * (ExpectedNumFeatures[Class] -
        NumFeatures)));

  /* Adjust Class Counts for Normalization Factors */
  MaxCount = 0;
  for (Class = 0; Class < MaxNumClasses; Class++) {
    NormCount[Class] = ClassCount[Class]
      - ((ClassPrunerMultiplier * NormalizationFactors[Class]) >> 8)
      * cp_maps[3] / 3;
    if (NormCount[Class] > MaxCount)
      MaxCount = NormCount[Class];
  }

  /* Prune Classes */
  MaxCount *= ClassPrunerThreshold;
  MaxCount >>= 8;
  /* Select Classes */
  if (MaxCount < 1)
    MaxCount = 1;
  NumClasses = 0;
  for (Class = 0; Class < MaxNumClasses; Class++)
  if (NormCount[Class] >= MaxCount) {
    NumClasses++;
    SortIndex[NumClasses] = Class;
    SortKey[NumClasses] = NormCount[Class];
  }

  /* Sort Classes using Heapsort Algorithm */
  if (NumClasses > 1)
    HeapSort(NumClasses, SortKey, SortIndex);

  if (display_ratings > 1) {
    cprintf ("CP:%d classes, %d features:\n", NumClasses, NumFeatures);
    for (Class = 0; Class < NumClasses; Class++) {
      classch =
        ClassIdForIndex (IntTemplates, SortIndex[NumClasses - Class]);
      cprintf ("%c:C=%d, E=%d, N=%d, Rat=%d\n", classch,
        ClassCount[SortIndex[NumClasses - Class]],
        ExpectedNumFeatures[SortIndex[NumClasses - Class]],
        SortKey[NumClasses - Class],
        (int) (10 +
        1000 * (1.0f -
        SortKey[NumClasses -
        Class] / ((float) cp_maps[3] *
        NumFeatures))));
    }
    if (display_ratings > 2) {
      NumPruners = NumClassPrunersIn (IntTemplates);
      for (feature_index = 0; feature_index < NumFeatures;
      feature_index++) {
        cprintf ("F=%3d,", feature_index);
        feature = &Features[feature_index];
        feature->CP_misses = 0;
        feature_address =
          (((feature->X * NUM_CP_BUCKETS >> 8) * NUM_CP_BUCKETS +
          (feature->Y * NUM_CP_BUCKETS >> 8)) * NUM_CP_BUCKETS +
          (feature->Theta * NUM_CP_BUCKETS >> 8)) << 1;
        ClassPruner = ClassPrunersFor (IntTemplates);
        class_index = 0;
        for (PrunerSet = 0; PrunerSet < NumPruners;
        PrunerSet++, ClassPruner++) {
          BasePrunerAddress = (UINT32 *) (*ClassPruner)
            + feature_address;

          for (Word = 0; Word < WERDS_PER_CP_VECTOR; Word++) {
            PrunerWord = *BasePrunerAddress++;
            for (Class = 0; Class < 16; Class++, class_index++) {
              if (NormCount[class_index] >= MaxCount)
                cprintf (" %c=%d,",
                  ClassIdForIndex (IntTemplates,
                  class_index),
                  PrunerWord & 3);
              PrunerWord >>= 2;
            }
          }
        }
        cprintf ("\n");
      }
      cprintf ("Adjustments:");
      for (Class = 0; Class < MaxNumClasses; Class++) {
        if (NormCount[Class] > MaxCount)
          cprintf (" %c=%d,",
            ClassIdForIndex (IntTemplates, Class),
            -((ClassPrunerMultiplier *
            NormalizationFactors[Class]) >> 8) * cp_maps[3] /
            3);
      }
      cprintf ("\n");
    }
  }

  /* Set Up Results */
  max_rating = 0.0f;
  for (Class = 0, out_class = 0; Class < NumClasses; Class++) {
    Results[out_class].Class =
      ClassIdForIndex (IntTemplates, SortIndex[NumClasses - Class]);
    Results[out_class].config_mask = (UINT32) - 1;
    Results[out_class].Rating =
      1.0 - SortKey[NumClasses -
      Class] / ((float) cp_maps[3] * NumFeatures);
    /**/ Results[out_class].Rating2 =
      1.0 - SortKey[NumClasses -
      Class] / ((float) cp_maps[3] * NumFeatures);
    if (tessedit_cp_ratio == 0.0 || Class == 0
      || Results[out_class].Rating * 1000 + 10 <
      cp_maxes[Results[out_class].Class]
      && Results[out_class].Rating * 1000 + 10 <
      (Results[0].Rating * 1000 +
      10) * cp_ratios[Results[out_class].Class])
      out_class++;
  }
  NumClasses = out_class;
  if (blob_type != 0) {
    cp_classes = NumClasses;
    if (NumClasses > 0) {
      cp_chars[0] = Results[0].Class;
      cp_ratings[0] = (int) (1000 * Results[0].Rating + 10);
      cp_confs[0] = (int) (1000 * Results[0].Rating2 + 10);
      if (NumClasses > 1) {
        cp_chars[1] = Results[1].Class;
        cp_ratings[1] = (int) (1000 * Results[1].Rating + 10);
        cp_confs[1] = (int) (1000 * Results[1].Rating2 + 10);
      }
      else {
        cp_chars[1] = '~';
        cp_ratings[1] = -1;
        cp_confs[1] = -1;
      }
    }
    else {
      cp_chars[0] = '~';
      cp_ratings[0] = -1;
      cp_confs[0] = -1;
    }
    cp_bestindex = -1;
    cp_bestrating = -1;
    cp_bestconf = -1;
    for (Class = 0; Class < NumClasses; Class++) {
      classch = Results[Class].Class;
      if (classch == blob_answer) {
        cp_bestindex = Class;
        cp_bestrating = (int) (1000 * Results[Class].Rating + 10);
        cp_bestconf = (int) (1000 * Results[Class].Rating2 + 10);
      }
    }
  }
  return NumClasses;

}


/*---------------------------------------------------------------------------*/
int feature_pruner(INT_TEMPLATES IntTemplates,
                   INT16 NumFeatures,
                   INT_FEATURE_ARRAY Features,
                   INT32 NumClasses,
                   CLASS_PRUNER_RESULTS Results) {
/*
 **      Parameters:
 **              IntTemplates           Class pruner tables
 **              NumFeatures            Number of features in blob
 **              Features               Array of features
 **              NormalizationFactors   Array of fudge factors from blob
 **                                     normalization process
 **                                     (by CLASS_INDEX)
 **              ExpectedNumFeatures    Array of expected number of features
 **                                     for each class
 **                                     (by CLASS_INDEX)
 **              Results                Sorted Array of pruned classes
 **                                     (by CLASS_ID)
 **              Debug                  Debugger flag: 1=debugger on
 **      Globals:
 **              ClassPrunerThreshold   Cutoff threshold
 **              ClassPrunerMultiplier  Normalization factor multiplier
 **      Operation:
 **              Prune the classes using a modified fast match table.
 **              Return a sorted list of classes along with the number
 **              of pruned classes in that list.
 **      Return: Number of pruned classes.
 **      Exceptions: none
 **      History: Tue Feb 19 10:24:24 MST 1991, RWM, Created.
 */
  UINT32 PrunerWord;
  CLASS_PRUNER *ClassPruner;
  INT32 class_index;             //index to class
  INT32 result_index;            //CP results index
  int PrunerSet;
  int NumPruners;
  int Word;
  INT_FEATURE feature;           //current feature
  INT32 feature_index;           //current feature
  INT32 CP_misses;               //missed features
  UINT32 feature_address;        //current feature index
  UINT32 *BasePrunerAddress;
  int MaxNumClasses;
  UINT32 class_mask[CLASS_MASK_SIZE];
  INT32 miss_histogram[MAX_NUM_CLASSES];

  MaxNumClasses = NumClassesIn (IntTemplates);
  for (class_index = 0; class_index < MaxNumClasses; class_index++)
    miss_histogram[class_index] = 0;

  /* Create class mask */
  for (class_index = 0; class_index < CLASS_MASK_SIZE; class_index++)
    class_mask[class_index] = (UINT32) - 1;
  for (result_index = 0; result_index < NumClasses; result_index++) {
    class_index =
      IndexForClassId (IntTemplates, Results[result_index].Class);
    class_mask[class_index / CLASSES_PER_CP_WERD] &=
      ~(3 << (class_index % CLASSES_PER_CP_WERD) * 2);
  }

  /* Update Class Counts */
  NumPruners = NumClassPrunersIn (IntTemplates);
  for (feature_index = 0; feature_index < NumFeatures; feature_index++) {
    feature = &Features[feature_index];
    feature_address = (((feature->X * NUM_CP_BUCKETS >> 8) * NUM_CP_BUCKETS
      +
      (feature->Y * NUM_CP_BUCKETS >> 8)) *
      NUM_CP_BUCKETS +
      (feature->Theta * NUM_CP_BUCKETS >> 8)) << 1;
    CP_misses = 0;
    ClassPruner = ClassPrunersFor (IntTemplates);
    class_index = 0;
    for (PrunerSet = 0; PrunerSet < NumPruners; PrunerSet++, ClassPruner++) {
      BasePrunerAddress = (UINT32 *) (*ClassPruner) + feature_address;

      for (Word = 0; Word < WERDS_PER_CP_VECTOR; Word++) {
        PrunerWord = *BasePrunerAddress++;
        PrunerWord |= class_mask[class_index++];
        CP_misses += miss_table[PrunerWord & 255];
        PrunerWord >>= 8;
        CP_misses += miss_table[PrunerWord & 255];
        PrunerWord >>= 8;
        CP_misses += miss_table[PrunerWord & 255];
        PrunerWord >>= 8;
        CP_misses += miss_table[PrunerWord & 255];
      }
    }
    feature->CP_misses = CP_misses;
    if (display_ratings > 1) {
      cprintf ("F=%d: misses=%d\n", feature_index, CP_misses);
    }
    miss_histogram[CP_misses]++;
  }

  CP_misses = 0;
  feature_index = NumFeatures * feature_prune_percentile / 100;
  for (class_index = MaxNumClasses - 1; class_index >= 0; class_index--) {
    CP_misses += miss_histogram[class_index];
    if (CP_misses >= feature_index)
      break;
  }

  if (display_ratings > 1) {
    cprintf ("FP:Selected miss factor of %d for %d features (%g%%)\n",
      class_index, CP_misses, 100.0 * CP_misses / NumFeatures);
  }
  return class_index;
}


/*---------------------------------------------------------------------------*/
int prune_configs(INT_TEMPLATES IntTemplates,
                  INT32 min_misses,
                  INT16 NumFeatures,
                  INT_FEATURE_ARRAY Features,
                  CLASS_NORMALIZATION_ARRAY NormalizationFactors,
                  INT32 class_count,
                  UINT16 BlobLength,
                  CLASS_PRUNER_RESULTS Results,
                  int Debug) {
/*
 **      Parameters:
 **              IntTemplates           Class pruner tables
 **              NumFeatures            Number of features in blob
 **              Features               Array of features
 **              NormalizationFactors   Array of fudge factors from blob
 **                                     normalization process
 **                                     (by CLASS_INDEX)
 **              ExpectedNumFeatures    Array of expected number of features
 **                                     for each class
 **                                     (by CLASS_INDEX)
 **              Results                Sorted Array of pruned classes
 **                                     (by CLASS_ID)
 **              Debug                  Debugger flag: 1=debugger on
 **      Globals:
 **              ClassPrunerThreshold   Cutoff threshold
 **              ClassPrunerMultiplier  Normalization factor multiplier
 **      Operation:
 **              Prune the classes using a modified fast match table.
 **              Return a sorted list of classes along with the number
 **              of pruned classes in that list.
 **      Return: Number of pruned classes.
 **      Exceptions: none
 **      History: Tue Feb 19 10:24:24 MST 1991, RWM, Created.
 */
  INT32 classindex;              //current Results index
  CLASS_INDEX Class;             //current class
  INT_CLASS ClassTemplate;       //info on current class
  FLOAT32 best_rating;           //best over all classes
  FLOAT32 best_class_rating;     //best over all classes
  INT32 output_count;            //number of classes out
  INT32 best_index;              //for sorting
  INT_RESULT_STRUCT IntResult;
                                 //results of pruning
  CLASS_PRUNER_RESULTS new_results;

  best_class_rating = 9999.0f;
  for (classindex = 0; classindex < class_count; classindex++) {
    Class = IndexForClassId (IntTemplates, Results[classindex].Class);
    ClassTemplate = ClassForIndex (IntTemplates, Class);
    PruningMatcher (ClassTemplate, BlobLength, NumFeatures, Features,
      min_misses, NormalizationFactors[Class],
      &IntResult, Debug);

    /* Prune configs */
                                 //save old rating
    new_results[classindex].Rating2 = Results[classindex].Rating;
                                 //save new rating
    new_results[classindex].Rating = IntResult.Rating;
                                 //save new rating
    new_results[classindex].config_mask = (1 << IntResult.Config) | (1 << IntResult.Config2);
                                 //save old class
    new_results[classindex].Class = Results[classindex].Class;

    if (display_ratings > 1) {
      cprintf ("PC:%c: old=%g, best_rating=%g, config1=%d, config2=%d\n",
        Results[classindex].Class,
        Results[classindex].Rating2,
        IntResult.Rating, IntResult.Config, IntResult.Config2);
    }

    if (IntResult.Rating < best_class_rating)
      best_class_rating = IntResult.Rating;
  }
  /* Select Classes */
  best_class_rating *= newcp_prune_threshold;

  output_count = 0;
  do {
    best_rating = best_class_rating;
    best_index = -1;
    for (classindex = 0; classindex < class_count; classindex++) {
      if (new_results[classindex].Rating <= best_rating) {
        best_rating = new_results[classindex].Rating;
        best_index = classindex;
      }
    }
    if (best_index >= 0) {
      Results[output_count].Class = new_results[best_index].Class;
      Results[output_count].Rating = best_rating;
      Results[output_count].Rating2 = new_results[best_index].Rating2;
      Results[output_count].config_mask =
        new_results[best_index].config_mask;
      new_results[best_index].Rating = 9999.0f;
      output_count++;
    }
  }
  while (best_index >= 0);

  if (display_ratings > 1) {
    cprintf ("%d classes reduced to %d\n", class_count, output_count);
    for (classindex = 0; classindex < output_count; classindex++) {
      cprintf ("%c=%g/%g/0x%x, ",
        Results[classindex].Class,
        Results[classindex].Rating,
        Results[classindex].Rating2,
        Results[classindex].config_mask);
    }
    cprintf ("\n");
  }
  return output_count;
}


/*---------------------------------------------------------------------------*/
void PruningMatcher(INT_CLASS ClassTemplate,
                    UINT16 BlobLength,
                    INT16 NumFeatures,
                    INT_FEATURE_ARRAY Features,
                    INT32 min_misses,
                    UINT8 NormalizationFactor,
                    INT_RESULT Result,
                    int Debug) {
/*
 **      Parameters:
 **              ClassTemplate             Prototypes & tables for a class
 **              BlobLength                Length of unormalized blob
 **              NumFeatures               Number of features in blob
 **              Features                  Array of features
 **              NormalizationFactor       Fudge factor from blob
 **                                        normalization process
 **              Result                    Class rating & configuration:
 **                                        (0.0 -> 1.0), 0=good, 1=bad
 **              Debug                     Debugger flag: 1=debugger on
 **      Globals:
 **              LocalMatcherMultiplier    Normalization factor multiplier
 **              IntThetaFudge             Theta fudge factor used for
 **                                        evidence calculation
 **      Operation:
 **              IntegerMatcher returns the best configuration and rating
 **              for a single class.  The class matched against is determined
 **              by the uniqueness of the ClassTemplate parameter.  The
 **              best rating and its associated configuration are returned.
 **      Return:
 **      Exceptions: none
 **      History: Tue Feb 19 16:36:23 MST 1991, RWM, Created.
 */
  static UINT8 FeatureEvidence[MAX_NUM_CONFIGS];
  static int SumOfFeatureEvidence[MAX_NUM_CONFIGS];
  int *IntPointer;
  int Feature;
  int BestMatch;
  int used_features;
  int NumConfigs;

  if (MatchDebuggingOn (Debug))
    cprintf ("Pruning Matcher -------------------------------------------\n");

  IntPointer = SumOfFeatureEvidence;
  for (NumConfigs = NumIntConfigsIn (ClassTemplate); NumConfigs > 0;
    NumConfigs--)
  *IntPointer++ = 0;

  for (Feature = 0, used_features = 0; Feature < NumFeatures; Feature++) {
    if (Features[Feature].CP_misses >= min_misses) {
      PMUpdateTablesForFeature (ClassTemplate, Feature,
        &(Features[Feature]), FeatureEvidence,
        SumOfFeatureEvidence, Debug);
      used_features++;
    }
  }

  IMNormalizeSumOfEvidences(ClassTemplate,
                            SumOfFeatureEvidence,
                            NumFeatures,
                            used_features);

  BestMatch =
    IMFindBestMatch(ClassTemplate,
                    SumOfFeatureEvidence,
                    BlobLength,
                    NormalizationFactor,
                    Result);

#ifndef GRAPHICS_DISABLED
  if (PrintMatchSummaryOn (Debug))
    IMDebugBestMatch(BestMatch, Result, BlobLength, NormalizationFactor);
#endif

  if (MatchDebuggingOn (Debug))
    cprintf ("Match Complete --------------------------------------------\n");

}


/*---------------------------------------------------------------------------*/
void config_mask_to_proto_mask(INT_CLASS ClassTemplate,
                               BIT_VECTOR config_mask,
                               BIT_VECTOR proto_mask) {
  UINT32 ConfigWord;
  int ProtoSetIndex;
  UINT32 ProtoNum;
  PROTO_SET ProtoSet;
  int NumProtos;
  UINT32 ActualProtoNum;

  NumProtos = NumIntProtosIn (ClassTemplate);

  zero_all_bits (proto_mask, WordsInVectorOfSize (MAX_NUM_PROTOS));
  for (ProtoSetIndex = 0; ProtoSetIndex < NumProtoSetsIn (ClassTemplate);
  ProtoSetIndex++) {
    ProtoSet = ProtoSetIn (ClassTemplate, ProtoSetIndex);
    ActualProtoNum = (ProtoSetIndex * PROTOS_PER_PROTO_SET);
    for (ProtoNum = 0;
      ((ProtoNum < PROTOS_PER_PROTO_SET)
    && (ActualProtoNum < NumProtos)); ProtoNum++, ActualProtoNum++) {
      ConfigWord = (ProtoSet->Protos[ProtoNum]).Configs[0];
      ConfigWord &= *config_mask;
      if (ConfigWord != 0) {
        proto_mask[ActualProtoNum / 32] |= 1 << (ActualProtoNum % 32);
      }
    }
  }
}


/*---------------------------------------------------------------------------*/
void IntegerMatcher(INT_CLASS ClassTemplate,
                    BIT_VECTOR ProtoMask,
                    BIT_VECTOR ConfigMask,
                    UINT16 BlobLength,
                    INT16 NumFeatures,
                    INT_FEATURE_ARRAY Features,
                    INT32 min_misses,
                    UINT8 NormalizationFactor,
                    INT_RESULT Result,
                    int Debug) {
/*
 **      Parameters:
 **              ClassTemplate             Prototypes & tables for a class
 **              BlobLength                Length of unormalized blob
 **              NumFeatures               Number of features in blob
 **              Features                  Array of features
 **              NormalizationFactor       Fudge factor from blob
 **                                        normalization process
 **              Result                    Class rating & configuration:
 **                                        (0.0 -> 1.0), 0=good, 1=bad
 **              Debug                     Debugger flag: 1=debugger on
 **      Globals:
 **              LocalMatcherMultiplier    Normalization factor multiplier
 **              IntThetaFudge             Theta fudge factor used for
 **                                        evidence calculation
 **      Operation:
 **              IntegerMatcher returns the best configuration and rating
 **              for a single class.  The class matched against is determined
 **              by the uniqueness of the ClassTemplate parameter.  The
 **              best rating and its associated configuration are returned.
 **      Return:
 **      Exceptions: none
 **      History: Tue Feb 19 16:36:23 MST 1991, RWM, Created.
 */
  static UINT8 FeatureEvidence[MAX_NUM_CONFIGS];
  static int SumOfFeatureEvidence[MAX_NUM_CONFIGS];
  static UINT8 ProtoEvidence[MAX_NUM_PROTOS][MAX_PROTO_INDEX];
  int Feature;
  int BestMatch;
  int used_features;

  if (MatchDebuggingOn (Debug))
    cprintf ("Integer Matcher -------------------------------------------\n");

  IMClearTables(ClassTemplate, SumOfFeatureEvidence, ProtoEvidence);

  for (Feature = 0, used_features = 0; Feature < NumFeatures; Feature++) {
    if (Features[Feature].CP_misses >= min_misses) {
      IMUpdateTablesForFeature (ClassTemplate, ProtoMask, ConfigMask,
        Feature, &(Features[Feature]),
        FeatureEvidence, SumOfFeatureEvidence,
        ProtoEvidence, Debug);
      used_features++;
    }
  }

#ifndef GRAPHICS_DISABLED
  if (PrintProtoMatchesOn (Debug) || PrintMatchSummaryOn (Debug))
    IMDebugFeatureProtoError(ClassTemplate,
                             ProtoMask,
                             ConfigMask,
                             SumOfFeatureEvidence,
                             ProtoEvidence,
                             NumFeatures,
                             Debug);

  if (DisplayProtoMatchesOn (Debug))
    IMDisplayProtoDebugInfo(ClassTemplate,
                            ProtoMask,
                            ConfigMask,
                            ProtoEvidence,
                            Debug);

  if (DisplayFeatureMatchesOn (Debug))
    IMDisplayFeatureDebugInfo(ClassTemplate,
                              ProtoMask,
                              ConfigMask,
                              NumFeatures,
                              Features,
                              Debug);
#endif

  IMUpdateSumOfProtoEvidences(ClassTemplate,
                              ConfigMask,
                              SumOfFeatureEvidence,
                              ProtoEvidence,
                              NumFeatures);

  IMNormalizeSumOfEvidences(ClassTemplate,
                            SumOfFeatureEvidence,
                            NumFeatures,
                            used_features);

  BestMatch =
    IMFindBestMatch(ClassTemplate,
                    SumOfFeatureEvidence,
                    BlobLength,
                    NormalizationFactor,
                    Result);

#ifndef GRAPHICS_DISABLED
  if (PrintMatchSummaryOn (Debug))
    IMDebugBestMatch(BestMatch, Result, BlobLength, NormalizationFactor);

  if (MatchDebuggingOn (Debug))
    cprintf ("Match Complete --------------------------------------------\n");
#endif

}


/*---------------------------------------------------------------------------*/
int FindGoodProtos(INT_CLASS ClassTemplate,
                   BIT_VECTOR ProtoMask,
                   BIT_VECTOR ConfigMask,
                   UINT16 BlobLength,
                   INT16 NumFeatures,
                   INT_FEATURE_ARRAY Features,
                   PROTO_ID *ProtoArray,
                   int Debug) {
/*
 **      Parameters:
 **              ClassTemplate             Prototypes & tables for a class
 **              ProtoMask                 AND Mask for proto word
 **              ConfigMask                AND Mask for config word
 **              BlobLength                Length of unormalized blob
 **              NumFeatures               Number of features in blob
 **              Features                  Array of features
 **              ProtoArray                Array of good protos
 **              Debug                     Debugger flag: 1=debugger on
 **      Globals:
 **              LocalMatcherMultiplier    Normalization factor multiplier
 **              IntThetaFudge             Theta fudge factor used for
 **                                        evidence calculation
 **              AdaptProtoThresh          Threshold for good protos
 **      Operation:
 **              FindGoodProtos finds all protos whose normalized proto-evidence
 **              exceed AdaptProtoThresh.  The list is ordered by increasing
 **              proto id number.
 **      Return:
 **              Number of good protos in ProtoArray.
 **      Exceptions: none
 **      History: Tue Mar 12 17:09:26 MST 1991, RWM, Created
 */
  static UINT8 FeatureEvidence[MAX_NUM_CONFIGS];
  static int SumOfFeatureEvidence[MAX_NUM_CONFIGS];
  static UINT8 ProtoEvidence[MAX_NUM_PROTOS][MAX_PROTO_INDEX];
  int Feature;
  register UINT8 *UINT8Pointer;
  register int ProtoIndex;
  int NumProtos;
  int NumGoodProtos;
  UINT16 ActualProtoNum;
  register int Temp;

  /* DEBUG opening heading */
  if (MatchDebuggingOn (Debug))
    cprintf
      ("Find Good Protos -------------------------------------------\n");

  IMClearTables(ClassTemplate, SumOfFeatureEvidence, ProtoEvidence);

  for (Feature = 0; Feature < NumFeatures; Feature++)
    IMUpdateTablesForFeature (ClassTemplate, ProtoMask, ConfigMask, Feature,
      &(Features[Feature]), FeatureEvidence,
      SumOfFeatureEvidence, ProtoEvidence, Debug);

#ifndef GRAPHICS_DISABLED
  if (PrintProtoMatchesOn (Debug) || PrintMatchSummaryOn (Debug))
    IMDebugFeatureProtoError(ClassTemplate,
                             ProtoMask,
                             ConfigMask,
                             SumOfFeatureEvidence,
                             ProtoEvidence,
                             NumFeatures,
                             Debug);
#endif

  /* Average Proto Evidences & Find Good Protos */
  NumProtos = NumIntProtosIn (ClassTemplate);
  NumGoodProtos = 0;
  for (ActualProtoNum = 0; ActualProtoNum < NumProtos; ActualProtoNum++) {
    /* Compute Average for Actual Proto */
    Temp = 0;
    UINT8Pointer = &(ProtoEvidence[ActualProtoNum][0]);
    for (ProtoIndex = LengthForProtoId (ClassTemplate, ActualProtoNum);
      ProtoIndex > 0; ProtoIndex--, UINT8Pointer++)
    Temp += *UINT8Pointer;

    Temp /= LengthForProtoId (ClassTemplate, ActualProtoNum);

    /* Find Good Protos */
    if (Temp >= AdaptProtoThresh) {
      *ProtoArray = ActualProtoNum;
      ProtoArray++;
      NumGoodProtos++;
    }
  }

  if (MatchDebuggingOn (Debug))
    cprintf ("Match Complete --------------------------------------------\n");

  return NumGoodProtos;

}


/*---------------------------------------------------------------------------*/
int FindBadFeatures(INT_CLASS ClassTemplate,
                    BIT_VECTOR ProtoMask,
                    BIT_VECTOR ConfigMask,
                    UINT16 BlobLength,
                    INT16 NumFeatures,
                    INT_FEATURE_ARRAY Features,
                    FEATURE_ID *FeatureArray,
                    int Debug) {
/*
 **      Parameters:
 **              ClassTemplate             Prototypes & tables for a class
 **              ProtoMask                 AND Mask for proto word
 **              ConfigMask                AND Mask for config word
 **              BlobLength                Length of unormalized blob
 **              NumFeatures               Number of features in blob
 **              Features                  Array of features
 **              FeatureArray              Array of bad features
 **              Debug                     Debugger flag: 1=debugger on
 **      Globals:
 **              LocalMatcherMultiplier    Normalization factor multiplier
 **              IntThetaFudge             Theta fudge factor used for
 **                                        evidence calculation
 **              AdaptFeatureThresh        Threshold for bad features
 **      Operation:
 **              FindBadFeatures finds all features whose maximum feature-evidence
 **              was less than AdaptFeatureThresh.  The list is ordered by increasing
 **              feature number.
 **      Return:
 **              Number of bad features in FeatureArray.
 **      Exceptions: none
 **      History: Tue Mar 12 17:09:26 MST 1991, RWM, Created
 */
  static UINT8 FeatureEvidence[MAX_NUM_CONFIGS];
  static int SumOfFeatureEvidence[MAX_NUM_CONFIGS];
  static UINT8 ProtoEvidence[MAX_NUM_PROTOS][MAX_PROTO_INDEX];
  int Feature;
  register UINT8 *UINT8Pointer;
  register int ConfigNum;
  int NumConfigs;
  int NumBadFeatures;
  register int Temp;

  /* DEBUG opening heading */
  if (MatchDebuggingOn (Debug))
    cprintf
      ("Find Bad Features -------------------------------------------\n");

  IMClearTables(ClassTemplate, SumOfFeatureEvidence, ProtoEvidence);

  NumBadFeatures = 0;
  NumConfigs = NumIntConfigsIn (ClassTemplate);
  for (Feature = 0; Feature < NumFeatures; Feature++) {
    IMUpdateTablesForFeature (ClassTemplate, ProtoMask, ConfigMask, Feature,
      &(Features[Feature]), FeatureEvidence,
      SumOfFeatureEvidence, ProtoEvidence, Debug);

    /* Find Best Evidence for Current Feature */
    Temp = 0;
    UINT8Pointer = FeatureEvidence;
    for (ConfigNum = 0; ConfigNum < NumConfigs; ConfigNum++, UINT8Pointer++)
      if (*UINT8Pointer > Temp)
        Temp = *UINT8Pointer;

    /* Find Bad Features */
    if (Temp < AdaptFeatureThresh) {
      *FeatureArray = Feature;
      FeatureArray++;
      NumBadFeatures++;
    }
  }

#ifndef GRAPHICS_DISABLED
  if (PrintProtoMatchesOn (Debug) || PrintMatchSummaryOn (Debug))
    IMDebugFeatureProtoError(ClassTemplate,
                             ProtoMask,
                             ConfigMask,
                             SumOfFeatureEvidence,
                             ProtoEvidence,
                             NumFeatures,
                             Debug);
#endif

  if (MatchDebuggingOn (Debug))
    cprintf ("Match Complete --------------------------------------------\n");

  return NumBadFeatures;

}


/*---------------------------------------------------------------------------*/
void InitIntegerMatcher() {
  int i;
  UINT32 IntSimilarity;
  double Similarity;
  double Evidence;
  double ScaleFactor;

  /* Set default mode of operation of IntegerMatcher */
  SetCharNormMatch();

  /* Initialize table for evidence to similarity lookup */
  for (i = 0; i < SE_TABLE_SIZE; i++) {
    IntSimilarity = i << (27 - SE_TABLE_BITS);
    Similarity = ((double) IntSimilarity) / 65536.0 / 65536.0;
    Evidence = Similarity / SimilarityCenter;
    Evidence *= Evidence;
    Evidence += 1.0;
    Evidence = 1.0 / Evidence;
    Evidence *= 255.0;

    if (SEExponentialMultiplier > 0.0) {
      ScaleFactor = 1.0 - exp (-SEExponentialMultiplier) *
        exp (SEExponentialMultiplier * ((double) i / SE_TABLE_SIZE));
      if (ScaleFactor > 1.0)
        ScaleFactor = 1.0;
      if (ScaleFactor < 0.0)
        ScaleFactor = 0.0;
      Evidence *= ScaleFactor;
    }

    SimilarityEvidenceTable[i] = (UINT8) (Evidence + 0.5);
  }

  /* Initialize evidence computation variables */
  EvidenceTableMask =
    ((1 << EvidenceTableBits) - 1) << (9 - EvidenceTableBits);
  MultTruncShiftBits = (14 - IntEvidenceTruncBits);
  TableTruncShiftBits = (27 - SE_TABLE_BITS - (MultTruncShiftBits << 1));
  EvidenceMultMask = ((1 << IntEvidenceTruncBits) - 1);

}


/*---------------------------------------------------------------------------*/
void InitIntegerMatcherVars() {
  MakeClassPrunerThreshold();
  MakeClassPrunerMultiplier();
  MakeIntegerMatcherMultiplier();
  MakeIntThetaFudge();
  MakeCPCutoffStrength();
  MakeEvidenceTableBits();
  MakeIntEvidenceTruncBits();
  MakeSEExponentialMultiplier();
  MakeSimilarityCenter();
}


/*-------------------------------------------------------------------------*/
void PrintIntMatcherStats(FILE *f) {
  fprintf (f, "protoword_lookups=%d, zero_protowords=%d, proto_shifts=%d\n",
    protoword_lookups, zero_protowords, proto_shifts);
  fprintf (f, "set_proto_bits=%d, config_shifts=%d, set_config_bits=%d\n",
    set_proto_bits, config_shifts, set_config_bits);
}


/*-------------------------------------------------------------------------*/
void SetProtoThresh(FLOAT32 Threshold) {
  AdaptProtoThresh = (int) (255 * Threshold);
  if (AdaptProtoThresh < 0)
    AdaptProtoThresh = 0;
  if (AdaptProtoThresh > 255)
    AdaptProtoThresh = 255;
}


/*---------------------------------------------------------------------------*/
void SetFeatureThresh(FLOAT32 Threshold) {
  AdaptFeatureThresh = (int) (255 * Threshold);
  if (AdaptFeatureThresh < 0)
    AdaptFeatureThresh = 0;
  if (AdaptFeatureThresh > 255)
    AdaptFeatureThresh = 255;
}


/*--------------------------------------------------------------------------*/
void SetBaseLineMatch() {
  LocalMatcherMultiplier = 0;
}


/*--------------------------------------------------------------------------*/
void SetCharNormMatch() {
  LocalMatcherMultiplier = IntegerMatcherMultiplier;
}


/**----------------------------------------------------------------------------
              Private Code
----------------------------------------------------------------------------**/
/*---------------------------------------------------------------------------*/
void
IMClearTables (INT_CLASS ClassTemplate,
int SumOfFeatureEvidence[MAX_NUM_CONFIGS],
UINT8 ProtoEvidence[MAX_NUM_PROTOS][MAX_PROTO_INDEX]) {
/*
 **      Parameters:
 **              SumOfFeatureEvidence  Sum of Feature Evidence Table
 **              NumConfigs            Number of Configurations
 **              ProtoEvidence         Prototype Evidence Table
 **              NumProtos             Number of Prototypes
 **      Globals:
 **      Operation:
 **              Clear SumOfFeatureEvidence and ProtoEvidence tables.
 **      Return:
 **      Exceptions: none
 **      History: Wed Feb 27 14:12:28 MST 1991, RWM, Created.
 */
  register UINT8 *UINT8Pointer;
  register int *IntPointer;
  register int ConfigNum;
  int NumConfigs;
  register UINT16 ProtoNum;
  int NumProtos;
  register int ProtoIndex;

  NumProtos = NumIntProtosIn (ClassTemplate);
  NumConfigs = NumIntConfigsIn (ClassTemplate);

  IntPointer = SumOfFeatureEvidence;
  for (ConfigNum = 0; ConfigNum < NumConfigs; ConfigNum++, IntPointer++)
    *IntPointer = 0;
  UINT8Pointer = (UINT8 *) ProtoEvidence;
  for (ProtoNum = 0; ProtoNum < NumProtos; ProtoNum++)
    for (ProtoIndex = 0; ProtoIndex < MAX_PROTO_INDEX;
    ProtoIndex++, UINT8Pointer++)
  *UINT8Pointer = 0;

}


/*---------------------------------------------------------------------------*/
void
IMClearFeatureEvidenceTable (UINT8 FeatureEvidence[MAX_NUM_CONFIGS],
int NumConfigs) {
/*
 **      Parameters:
 **              FeatureEvidence  Feature Evidence Table
 **              NumConfigs       Number of Configurations
 **      Globals:
 **      Operation:
 **              Clear FeatureEvidence table.
 **      Return:
 **      Exceptions: none
 **      History: Wed Feb 27 14:12:28 MST 1991, RWM, Created.
 */
  register UINT8 *UINT8Pointer;
  register int ConfigNum;

  UINT8Pointer = FeatureEvidence;
  for (ConfigNum = 0; ConfigNum < NumConfigs; ConfigNum++, UINT8Pointer++)
    *UINT8Pointer = 0;

}


/*---------------------------------------------------------------------------*/
void IMDebugConfiguration(int FeatureNum,
                          UINT16 ActualProtoNum,
                          UINT8 Evidence,
                          BIT_VECTOR ConfigMask,
                          UINT32 ConfigWord) {
/*
 **      Parameters:
 **      Globals:
 **      Operation:
 **              Print debugging information for Configuations
 **      Return:
 **      Exceptions: none
 **      History: Wed Feb 27 14:12:28 MST 1991, RWM, Created.
 */
  cprintf ("F = %3d, P = %3d, E = %3d, Configs = ",
    FeatureNum, (int) ActualProtoNum, (int) Evidence);
  while (ConfigWord) {
    if (ConfigWord & 1)
      cprintf ("1");
    else
      cprintf ("0");
    ConfigWord >>= 1;
  }
  cprintf ("\n");
}


/*---------------------------------------------------------------------------*/
void IMDebugConfigurationSum(int FeatureNum,
                             UINT8 *FeatureEvidence,
                             INT32 ConfigCount) {
/*
 **      Parameters:
 **      Globals:
 **      Operation:
 **              Print debugging information for Configuations
 **      Return:
 **      Exceptions: none
 **      History: Wed Feb 27 14:12:28 MST 1991, RWM, Created.
 */
  int ConfigNum;

  cprintf ("F=%3d, C=", (int) FeatureNum);

  for (ConfigNum = 0; ConfigNum < ConfigCount; ConfigNum++) {
    cprintf ("%4d", FeatureEvidence[ConfigNum]);
  }
  cprintf ("\n");

}


/*---------------------------------------------------------------------------*/
void
PMUpdateTablesForFeature (INT_CLASS ClassTemplate,
int FeatureNum,
INT_FEATURE Feature,
UINT8 FeatureEvidence[MAX_NUM_CONFIGS],
int SumOfFeatureEvidence[MAX_NUM_CONFIGS],
int Debug) {
/*
 **      Parameters:
 **              ClassTemplate         Prototypes & tables for a class
 **              FeatureNum            Current feature number (for DEBUG only)
 **              Feature               Pointer to a feature struct
 **              FeatureEvidence       Feature Evidence Table
 **              SumOfFeatureEvidence  Sum of Feature Evidence Table
 **              ProtoEvidence         Prototype Evidence Table
 **              Debug                 Debugger flag: 1=debugger on
 **      Globals:
 **      Operation:
 **              For the given feature: prune protos, compute evidence, update Feature Evidence,
 **              Proto Evidence, and Sum of Feature Evidence tables.
 **      Return:
 **      Exceptions: none
 **      History: Wed Feb 27 14:12:28 MST 1991, RWM, Created.
 */
  UINT8 config_byte;
  UINT8 proto_byte;
  UINT8 Evidence;
  INT32 config_offset;
  UINT8 *UINT8Pointer;
  UINT32 ConfigWord;
  UINT32 ProtoWord;
  INT32 M3;
  INT32 A3;
  UINT32 A4;
  INT32 proto_word_offset;
  INT32 proto_offset;
  UINT32 ProtoNum;
  UINT32 ActualProtoNum;
  PROTO_SET ProtoSet;
  UINT32 *ProtoPrunerPtr;
  INT_PROTO Proto;
  int ProtoSetIndex;
  UINT32 XFeatureAddress;
  UINT32 YFeatureAddress;
  UINT32 ThetaFeatureAddress;
  int *IntPointer;
  int ConfigNum;

  IMClearFeatureEvidenceTable (FeatureEvidence,
    NumIntConfigsIn (ClassTemplate));

  /* Precompute Feature Address offset for Proto Pruning */
  XFeatureAddress = ((Feature->X >> 2) << 1);
  YFeatureAddress = (NUM_PP_BUCKETS << 1) + ((Feature->Y >> 2) << 1);
  ThetaFeatureAddress = (NUM_PP_BUCKETS << 2) + ((Feature->Theta >> 2) << 1);

  for (ProtoSetIndex = 0, ActualProtoNum = 0;
  ProtoSetIndex < NumProtoSetsIn (ClassTemplate); ProtoSetIndex++) {
    ProtoSet = ProtoSetIn (ClassTemplate, ProtoSetIndex);
    ProtoPrunerPtr = (UINT32 *) ((*ProtoSet).ProtoPruner);
    for (ProtoNum = 0; ProtoNum < PROTOS_PER_PROTO_SET;
      ProtoNum += (PROTOS_PER_PROTO_SET >> 1), ActualProtoNum +=
    (PROTOS_PER_PROTO_SET >> 1), ProtoPrunerPtr++) {
      /* Prune Protos of current Proto Set */
      ProtoWord = *(ProtoPrunerPtr + XFeatureAddress);
      ProtoWord &= *(ProtoPrunerPtr + YFeatureAddress);
      ProtoWord &= *(ProtoPrunerPtr + ThetaFeatureAddress);

      if (ProtoWord != 0) {
        proto_byte = ProtoWord & 0xff;
        ProtoWord >>= 8;
        proto_word_offset = 0;
        while (ProtoWord != 0 || proto_byte != 0) {
          while (proto_byte == 0) {
            proto_byte = ProtoWord & 0xff;
            ProtoWord >>= 8;
            proto_word_offset += 8;
          }
          proto_offset = offset_table[proto_byte] + proto_word_offset;
          proto_byte = next_table[proto_byte];
          /* Compute Evidence */
          Proto = &(ProtoSet->Protos[ProtoNum + proto_offset]);
          ConfigWord = Proto->Configs[0];
          A3 = (((Proto->A * (Feature->X - 128)) << 1)
            - (Proto->B * (Feature->Y - 128)) + (Proto->C << 9));
          M3 =
            (((INT8) (Feature->Theta - Proto->Angle)) *
            IntThetaFudge) << 1;

          if (A3 < 0)
            A3 = ~A3;
          if (M3 < 0)
            M3 = ~M3;
          A3 >>= MultTruncShiftBits;
          M3 >>= MultTruncShiftBits;
          if (A3 > EvidenceMultMask)
            A3 = EvidenceMultMask;
          if (M3 > EvidenceMultMask)
            M3 = EvidenceMultMask;

          A4 = (A3 * A3) + (M3 * M3);
          A4 >>= TableTruncShiftBits;
          if (A4 > EvidenceTableMask)
            Evidence = 0;
          else
            Evidence = SimilarityEvidenceTable[A4];

          UINT8Pointer = FeatureEvidence - 8;
          config_byte = 0;
          while (ConfigWord != 0 || config_byte != 0) {
            while (config_byte == 0) {
              config_byte = ConfigWord & 0xff;
              ConfigWord >>= 8;
              UINT8Pointer += 8;
            }
            config_offset = offset_table[config_byte];
            config_byte = next_table[config_byte];
            if (Evidence > UINT8Pointer[config_offset])
              UINT8Pointer[config_offset] = Evidence;
          }
        }
      }
    }
  }

  if (PrintFeatureMatchesOn (Debug))
    IMDebugConfigurationSum (FeatureNum, FeatureEvidence,
      NumIntConfigsIn (ClassTemplate));
  IntPointer = SumOfFeatureEvidence;
  UINT8Pointer = FeatureEvidence;
  for (ConfigNum = NumIntConfigsIn (ClassTemplate); ConfigNum > 0;
    ConfigNum--)
  *IntPointer++ += (*UINT8Pointer++);
}


/*---------------------------------------------------------------------------*/
void
IMUpdateTablesForFeature (INT_CLASS ClassTemplate,
BIT_VECTOR ProtoMask,
BIT_VECTOR ConfigMask,
int FeatureNum,
INT_FEATURE Feature,
UINT8 FeatureEvidence[MAX_NUM_CONFIGS],
int SumOfFeatureEvidence[MAX_NUM_CONFIGS],
UINT8
ProtoEvidence[MAX_NUM_PROTOS][MAX_PROTO_INDEX],
int Debug) {
/*
 **      Parameters:
 **              ClassTemplate         Prototypes & tables for a class
 **              FeatureNum            Current feature number (for DEBUG only)
 **              Feature               Pointer to a feature struct
 **              FeatureEvidence       Feature Evidence Table
 **              SumOfFeatureEvidence  Sum of Feature Evidence Table
 **              ProtoEvidence         Prototype Evidence Table
 **              Debug                 Debugger flag: 1=debugger on
 **      Globals:
 **      Operation:
 **              For the given feature: prune protos, compute evidence, update Feature Evidence,
 **              Proto Evidence, and Sum of Feature Evidence tables.
 **      Return:
 **      Exceptions: none
 **      History: Wed Feb 27 14:12:28 MST 1991, RWM, Created.
 */
  register UINT32 ConfigWord;
  register UINT32 ProtoWord;
  register UINT32 ProtoNum;
  register UINT32 ActualProtoNum;
  UINT8 proto_byte;
  INT32 proto_word_offset;
  INT32 proto_offset;
  UINT8 config_byte;
  INT32 config_offset;
  PROTO_SET ProtoSet;
  UINT32 *ProtoPrunerPtr;
  INT_PROTO Proto;
  int ProtoSetIndex;
  UINT8 Evidence;
  UINT32 XFeatureAddress;
  UINT32 YFeatureAddress;
  UINT32 ThetaFeatureAddress;
  register UINT8 *UINT8Pointer;
  register int ProtoIndex;
  UINT8 Temp;
  register int *IntPointer;
  int ConfigNum;
  register INT32 M3;
  register INT32 A3;
  register UINT32 A4;

  IMClearFeatureEvidenceTable (FeatureEvidence,
    NumIntConfigsIn (ClassTemplate));

  /* Precompute Feature Address offset for Proto Pruning */
  XFeatureAddress = ((Feature->X >> 2) << 1);
  YFeatureAddress = (NUM_PP_BUCKETS << 1) + ((Feature->Y >> 2) << 1);
  ThetaFeatureAddress = (NUM_PP_BUCKETS << 2) + ((Feature->Theta >> 2) << 1);

  for (ProtoSetIndex = 0, ActualProtoNum = 0;
  ProtoSetIndex < NumProtoSetsIn (ClassTemplate); ProtoSetIndex++) {
    ProtoSet = ProtoSetIn (ClassTemplate, ProtoSetIndex);
    ProtoPrunerPtr = (UINT32 *) ((*ProtoSet).ProtoPruner);
    for (ProtoNum = 0; ProtoNum < PROTOS_PER_PROTO_SET;
      ProtoNum += (PROTOS_PER_PROTO_SET >> 1), ActualProtoNum +=
    (PROTOS_PER_PROTO_SET >> 1), ProtoMask++, ProtoPrunerPtr++) {
      /* Prune Protos of current Proto Set */
      ProtoWord = *(ProtoPrunerPtr + XFeatureAddress);
      ProtoWord &= *(ProtoPrunerPtr + YFeatureAddress);
      ProtoWord &= *(ProtoPrunerPtr + ThetaFeatureAddress);
      ProtoWord &= *ProtoMask;

      if (ProtoWord != 0) {
        proto_byte = ProtoWord & 0xff;
        ProtoWord >>= 8;
        proto_word_offset = 0;
        while (ProtoWord != 0 || proto_byte != 0) {
          while (proto_byte == 0) {
            proto_byte = ProtoWord & 0xff;
            ProtoWord >>= 8;
            proto_word_offset += 8;
          }
          proto_offset = offset_table[proto_byte] + proto_word_offset;
          proto_byte = next_table[proto_byte];
          Proto = &(ProtoSet->Protos[ProtoNum + proto_offset]);
          ConfigWord = Proto->Configs[0];
          A3 = (((Proto->A * (Feature->X - 128)) << 1)
            - (Proto->B * (Feature->Y - 128)) + (Proto->C << 9));
          M3 =
            (((INT8) (Feature->Theta - Proto->Angle)) *
            IntThetaFudge) << 1;

          if (A3 < 0)
            A3 = ~A3;
          if (M3 < 0)
            M3 = ~M3;
          A3 >>= MultTruncShiftBits;
          M3 >>= MultTruncShiftBits;
          if (A3 > EvidenceMultMask)
            A3 = EvidenceMultMask;
          if (M3 > EvidenceMultMask)
            M3 = EvidenceMultMask;

          A4 = (A3 * A3) + (M3 * M3);
          A4 >>= TableTruncShiftBits;
          if (A4 > EvidenceTableMask)
            Evidence = 0;
          else
            Evidence = SimilarityEvidenceTable[A4];

          if (PrintFeatureMatchesOn (Debug))
            IMDebugConfiguration (FeatureNum,
              ActualProtoNum + proto_offset,
              Evidence, ConfigMask, ConfigWord);

          ConfigWord &= *ConfigMask;

          UINT8Pointer = FeatureEvidence - 8;
          config_byte = 0;
          while (ConfigWord != 0 || config_byte != 0) {
            while (config_byte == 0) {
              config_byte = ConfigWord & 0xff;
              ConfigWord >>= 8;
              UINT8Pointer += 8;
              //                                              config_shifts++;
            }
            config_offset = offset_table[config_byte];
            config_byte = next_table[config_byte];
            if (Evidence > UINT8Pointer[config_offset])
              UINT8Pointer[config_offset] = Evidence;
          }

          UINT8Pointer =
            &(ProtoEvidence[ActualProtoNum + proto_offset][0]);
          for (ProtoIndex =
            LengthForProtoId (ClassTemplate,
            ActualProtoNum + proto_offset);
          ProtoIndex > 0; ProtoIndex--, UINT8Pointer++) {
            if (Evidence > *UINT8Pointer) {
              Temp = *UINT8Pointer;
              *UINT8Pointer = Evidence;
              Evidence = Temp;
            }
            else if (Evidence == 0)
              break;
          }
        }
      }
    }
  }

  if (PrintFeatureMatchesOn (Debug))
    IMDebugConfigurationSum (FeatureNum, FeatureEvidence,
      NumIntConfigsIn (ClassTemplate));
  IntPointer = SumOfFeatureEvidence;
  UINT8Pointer = FeatureEvidence;
  for (ConfigNum = NumIntConfigsIn (ClassTemplate); ConfigNum > 0;
    ConfigNum--)
  *IntPointer++ += (*UINT8Pointer++);

}


/*---------------------------------------------------------------------------*/
#ifndef GRAPHICS_DISABLED
void
IMDebugFeatureProtoError (INT_CLASS ClassTemplate,
BIT_VECTOR ProtoMask,
BIT_VECTOR ConfigMask,
int SumOfFeatureEvidence[MAX_NUM_CONFIGS],
UINT8
ProtoEvidence[MAX_NUM_PROTOS][MAX_PROTO_INDEX],
INT16 NumFeatures, int Debug) {
/*
 **      Parameters:
 **      Globals:
 **      Operation:
 **              Print debugging information for Configuations
 **      Return:
 **      Exceptions: none
 **      History: Wed Feb 27 14:12:28 MST 1991, RWM, Created.
 */
  UINT8 *UINT8Pointer;
  int *IntPointer;
  FLOAT32 ProtoConfigs[MAX_NUM_CONFIGS];
  int ConfigNum;
  UINT32 ConfigWord;
  int ProtoSetIndex;
  UINT16 ProtoNum;
  UINT8 ProtoWordNum;
  PROTO_SET ProtoSet;
  int ProtoIndex;
  int NumProtos;
  UINT16 ActualProtoNum;
  int Temp;
  int NumConfigs;

  NumProtos = NumIntProtosIn (ClassTemplate);
  NumConfigs = NumIntConfigsIn (ClassTemplate);

  if (PrintMatchSummaryOn (Debug)) {
    cprintf ("Configuration Mask:\n");
    for (ConfigNum = 0; ConfigNum < NumConfigs; ConfigNum++)
      cprintf ("%1d", (((*ConfigMask) >> ConfigNum) & 1));
    cprintf ("\n");

    cprintf ("Feature Error for Configurations:\n");
    for (ConfigNum = 0; ConfigNum < NumConfigs; ConfigNum++)
      cprintf (" %5.1f",
        100.0 * (1.0 -
        (FLOAT32) SumOfFeatureEvidence[ConfigNum] /
        NumFeatures / 256.0));
    cprintf ("\n\n\n");
  }

  if (PrintMatchSummaryOn (Debug)) {
    cprintf ("Proto Mask:\n");
    for (ProtoSetIndex = 0; ProtoSetIndex < NumProtoSetsIn (ClassTemplate);
    ProtoSetIndex++) {
      ActualProtoNum = (ProtoSetIndex * PROTOS_PER_PROTO_SET);
      for (ProtoWordNum = 0; ProtoWordNum < 2;
      ProtoWordNum++, ProtoMask++) {
        ActualProtoNum = (ProtoSetIndex * PROTOS_PER_PROTO_SET);
        for (ProtoNum = 0;
          ((ProtoNum < (PROTOS_PER_PROTO_SET >> 1))
          && (ActualProtoNum < NumProtos));
          ProtoNum++, ActualProtoNum++)
        cprintf ("%1d", (((*ProtoMask) >> ProtoNum) & 1));
        cprintf ("\n");
      }
    }
    cprintf ("\n");
  }

  for (ConfigNum = 0; ConfigNum < NumConfigs; ConfigNum++)
    ProtoConfigs[ConfigNum] = 0;

  if (PrintProtoMatchesOn (Debug)) {
    cprintf ("Proto Evidence:\n");
    for (ProtoSetIndex = 0; ProtoSetIndex < NumProtoSetsIn (ClassTemplate);
    ProtoSetIndex++) {
      ProtoSet = ProtoSetIn (ClassTemplate, ProtoSetIndex);
      ActualProtoNum = (ProtoSetIndex * PROTOS_PER_PROTO_SET);
      for (ProtoNum = 0;
        ((ProtoNum < PROTOS_PER_PROTO_SET)
        && (ActualProtoNum < NumProtos));
      ProtoNum++, ActualProtoNum++) {
        cprintf ("P %3d =", ActualProtoNum);
        Temp = 0;
        UINT8Pointer = &(ProtoEvidence[ActualProtoNum][0]);
        for (ProtoIndex = 0;
          ProtoIndex < LengthForProtoId (ClassTemplate,
          ActualProtoNum);
        ProtoIndex++, UINT8Pointer++) {
          cprintf (" %d", *UINT8Pointer);
          Temp += *UINT8Pointer;
        }

        cprintf (" = %6.4f%%\n", Temp /
          256.0 / LengthForProtoId (ClassTemplate,
          ActualProtoNum));

        ConfigWord = (ProtoSet->Protos[ProtoNum]).Configs[0];
        IntPointer = SumOfFeatureEvidence;
        ConfigNum = 0;
        while (ConfigWord) {
          cprintf ("%5d", ConfigWord & 1 ? Temp : 0);
          if (ConfigWord & 1)
            ProtoConfigs[ConfigNum] += Temp;
          IntPointer++;
          ConfigNum++;
          ConfigWord >>= 1;
        }
        cprintf ("\n");
      }
    }
  }

  if (PrintMatchSummaryOn (Debug)) {
    cprintf ("Proto Error for Configurations:\n");
    for (ConfigNum = 0; ConfigNum < NumConfigs; ConfigNum++)
      cprintf (" %5.1f",
        100.0 * (1.0 -
        ProtoConfigs[ConfigNum] /
        LengthForConfigId (ClassTemplate,
        ConfigNum) / 256.0));
    cprintf ("\n\n");
  }

  if (PrintProtoMatchesOn (Debug)) {
    cprintf ("Proto Sum for Configurations:\n");
    for (ConfigNum = 0; ConfigNum < NumConfigs; ConfigNum++)
      cprintf (" %4.1f", ProtoConfigs[ConfigNum] / 256.0);
    cprintf ("\n\n");

    cprintf ("Proto Length for Configurations:\n");
    for (ConfigNum = 0; ConfigNum < NumConfigs; ConfigNum++)
      cprintf (" %4.1f",
        (float) LengthForConfigId (ClassTemplate, ConfigNum));
    cprintf ("\n\n");
  }

}


/*---------------------------------------------------------------------------*/
void
IMDisplayProtoDebugInfo (INT_CLASS ClassTemplate,
BIT_VECTOR ProtoMask,
BIT_VECTOR ConfigMask,
UINT8 ProtoEvidence[MAX_NUM_PROTOS][MAX_PROTO_INDEX],
int Debug) {
  register UINT8 *UINT8Pointer;
  register UINT32 ConfigWord;
  register UINT16 ProtoNum;
  register UINT16 ActualProtoNum;
  PROTO_SET ProtoSet;
  int ProtoSetIndex;
  int ProtoIndex;
  int NumProtos;
  register int Temp;

  extern void *IntMatchWindow;

  if (IntMatchWindow == NULL) {
    IntMatchWindow = c_create_window ("IntMatchWindow", 50, 200,
      520, 520,
      -130.0, 130.0, -130.0, 130.0);
  }
  NumProtos = NumIntProtosIn (ClassTemplate);

  for (ProtoSetIndex = 0; ProtoSetIndex < NumProtoSetsIn (ClassTemplate);
  ProtoSetIndex++) {
    ProtoSet = ProtoSetIn (ClassTemplate, ProtoSetIndex);
    ActualProtoNum = (ProtoSetIndex * PROTOS_PER_PROTO_SET);
    for (ProtoNum = 0;
      ((ProtoNum < PROTOS_PER_PROTO_SET)
    && (ActualProtoNum < NumProtos)); ProtoNum++, ActualProtoNum++) {
      /* Compute Average for Actual Proto */
      Temp = 0;
      UINT8Pointer = &(ProtoEvidence[ActualProtoNum][0]);
      for (ProtoIndex = LengthForProtoId (ClassTemplate, ActualProtoNum);
        ProtoIndex > 0; ProtoIndex--, UINT8Pointer++)
      Temp += *UINT8Pointer;

      Temp /= LengthForProtoId (ClassTemplate, ActualProtoNum);

      ConfigWord = (ProtoSet->Protos[ProtoNum]).Configs[0];
      ConfigWord &= *ConfigMask;
      if (ConfigWord)
        /* Update display for current proto */
      if (ClipMatchEvidenceOn (Debug)) {
        if (Temp < AdaptProtoThresh)
          DisplayIntProto (ClassTemplate, ActualProtoNum,
            (Temp / 255.0));
        else
          DisplayIntProto (ClassTemplate, ActualProtoNum,
            (Temp / 255.0));
      }
      else {
        DisplayIntProto (ClassTemplate, ActualProtoNum,
          (Temp / 255.0));
      }
    }
  }
}


/*---------------------------------------------------------------------------*/
void IMDisplayFeatureDebugInfo(INT_CLASS ClassTemplate,
                               BIT_VECTOR ProtoMask,
                               BIT_VECTOR ConfigMask,
                               INT16 NumFeatures,
                               INT_FEATURE_ARRAY Features,
                               int Debug) {
  static UINT8 FeatureEvidence[MAX_NUM_CONFIGS];
  static int SumOfFeatureEvidence[MAX_NUM_CONFIGS];
  static UINT8 ProtoEvidence[MAX_NUM_PROTOS][MAX_PROTO_INDEX];
  int Feature;
  register UINT8 *UINT8Pointer;
  register int ConfigNum;
  int NumConfigs;
  register int Temp;

  IMClearTables(ClassTemplate, SumOfFeatureEvidence, ProtoEvidence);

  NumConfigs = NumIntConfigsIn (ClassTemplate);
  for (Feature = 0; Feature < NumFeatures; Feature++) {
    IMUpdateTablesForFeature (ClassTemplate, ProtoMask, ConfigMask, Feature,
      &(Features[Feature]), FeatureEvidence,
      SumOfFeatureEvidence, ProtoEvidence, 0);

    /* Find Best Evidence for Current Feature */
    Temp = 0;
    UINT8Pointer = FeatureEvidence;
    for (ConfigNum = 0; ConfigNum < NumConfigs; ConfigNum++, UINT8Pointer++)
      if (*UINT8Pointer > Temp)
        Temp = *UINT8Pointer;

    /* Update display for current feature */
    if (ClipMatchEvidenceOn (Debug)) {
      if (Temp < AdaptFeatureThresh)
        DisplayIntFeature (&(Features[Feature]), 0.0);
      else
        DisplayIntFeature (&(Features[Feature]), 1.0);
    }
    else {
      DisplayIntFeature (&(Features[Feature]), (Temp / 255.0));
    }
  }
}
#endif

/*---------------------------------------------------------------------------*/
void
IMUpdateSumOfProtoEvidences (INT_CLASS ClassTemplate,
BIT_VECTOR ConfigMask,
int SumOfFeatureEvidence[MAX_NUM_CONFIGS],
UINT8
ProtoEvidence[MAX_NUM_PROTOS][MAX_PROTO_INDEX],
INT16 NumFeatures) {
/*
 **      Parameters:
 **      Globals:
 **      Operation:
 **              Add sum of Proto Evidences into Sum Of Feature Evidence Array
 **      Return:
 **      Exceptions: none
 **      History: Wed Feb 27 14:12:28 MST 1991, RWM, Created.
 */
  register UINT8 *UINT8Pointer;
  register int *IntPointer;
  register UINT32 ConfigWord;
  int ProtoSetIndex;
  register UINT16 ProtoNum;
  PROTO_SET ProtoSet;
  register int ProtoIndex;
  int NumProtos;
  UINT16 ActualProtoNum;
  int Temp;

  NumProtos = NumIntProtosIn (ClassTemplate);

  for (ProtoSetIndex = 0; ProtoSetIndex < NumProtoSetsIn (ClassTemplate);
  ProtoSetIndex++) {
    ProtoSet = ProtoSetIn (ClassTemplate, ProtoSetIndex);
    ActualProtoNum = (ProtoSetIndex * PROTOS_PER_PROTO_SET);
    for (ProtoNum = 0;
      ((ProtoNum < PROTOS_PER_PROTO_SET)
    && (ActualProtoNum < NumProtos)); ProtoNum++, ActualProtoNum++) {
      Temp = 0;
      UINT8Pointer = &(ProtoEvidence[ActualProtoNum][0]);
      for (ProtoIndex = LengthForProtoId (ClassTemplate, ActualProtoNum);
        ProtoIndex > 0; ProtoIndex--, UINT8Pointer++)
      Temp += *UINT8Pointer;

      ConfigWord = (ProtoSet->Protos[ProtoNum]).Configs[0];
      ConfigWord &= *ConfigMask;
      IntPointer = SumOfFeatureEvidence;
      while (ConfigWord) {
        if (ConfigWord & 1)
          *IntPointer += Temp;
        IntPointer++;
        ConfigWord >>= 1;
      }
    }
  }
}


/*---------------------------------------------------------------------------*/
void
PMNormalizeSumOfEvidences (INT_CLASS ClassTemplate,
int SumOfFeatureEvidence[MAX_NUM_CONFIGS],
INT16 NumFeatures, INT32 used_features) {
/*
 **      Parameters:
 **      Globals:
 **      Operation:
 **              Normalize Sum of Proto and Feature Evidence by dividing by
 **              the sum of the Feature Lengths and the Proto Lengths for each
 **              configuration.
 **      Return:
 **      Exceptions: none
 **      History: Wed Feb 27 14:12:28 MST 1991, RWM, Created.
 */
  register int *IntPointer;
  register int ConfigNum;
  int NumConfigs;

  NumConfigs = NumIntConfigsIn (ClassTemplate);
  if (used_features <= 0)
    used_features = 1;

  IntPointer = SumOfFeatureEvidence;
  for (ConfigNum = 0; ConfigNum < NumConfigs; ConfigNum++, IntPointer++)
    *IntPointer = (*IntPointer << 8) / used_features;
}


/*---------------------------------------------------------------------------*/
void
IMNormalizeSumOfEvidences (INT_CLASS ClassTemplate,
int SumOfFeatureEvidence[MAX_NUM_CONFIGS],
INT16 NumFeatures, INT32 used_features) {
/*
 **      Parameters:
 **      Globals:
 **      Operation:
 **              Normalize Sum of Proto and Feature Evidence by dividing by
 **              the sum of the Feature Lengths and the Proto Lengths for each
 **              configuration.
 **      Return:
 **      Exceptions: none
 **      History: Wed Feb 27 14:12:28 MST 1991, RWM, Created.
 */
  register int *IntPointer;
  register int ConfigNum;
  int NumConfigs;

  NumConfigs = NumIntConfigsIn (ClassTemplate);

  IntPointer = SumOfFeatureEvidence;
  for (ConfigNum = 0; ConfigNum < NumConfigs; ConfigNum++, IntPointer++)
    *IntPointer = (*IntPointer << 8) /
      (NumFeatures + LengthForConfigId (ClassTemplate, ConfigNum));
}


/*---------------------------------------------------------------------------*/
int
IMFindBestMatch (INT_CLASS ClassTemplate,
int SumOfFeatureEvidence[MAX_NUM_CONFIGS],
UINT16 BlobLength,
UINT8 NormalizationFactor, INT_RESULT Result) {
/*
 **      Parameters:
 **      Globals:
 **      Operation:
 **              Find the best match for the current class and update the Result
 **              with the configuration and match rating.
 **      Return:
 **              The best normalized sum of evidences
 **      Exceptions: none
 **      History: Wed Feb 27 14:12:28 MST 1991, RWM, Created.
 */
  register int *IntPointer;
  register int ConfigNum;
  register int NumConfigs;
  register int BestMatch;
  register int Best2Match;

  NumConfigs = NumIntConfigsIn (ClassTemplate);

  /* Find best match */
  BestMatch = 0;
  Best2Match = 0;
  IntPointer = SumOfFeatureEvidence;
  for (ConfigNum = 0; ConfigNum < NumConfigs; ConfigNum++, IntPointer++) {
    if (display_ratings > 1)
      cprintf ("Config %d, rating=%d\n", ConfigNum, *IntPointer);
    if (*IntPointer > BestMatch) {
      if (BestMatch > 0) {
        Result->Config2 = Result->Config;
        Best2Match = BestMatch;
      }
      else
        Result->Config2 = ConfigNum;
      Result->Config = ConfigNum;
      BestMatch = *IntPointer;
    }
    else if (*IntPointer > Best2Match) {
      Result->Config2 = ConfigNum;
      Best2Match = *IntPointer;
    }
  }

  /* Compute Certainty Rating */
  (*Result).Rating = ((65536.0 - BestMatch) / 65536.0 * BlobLength +
    LocalMatcherMultiplier * NormalizationFactor / 256.0) /
    (BlobLength + LocalMatcherMultiplier);

  return BestMatch;
}


/*---------------------------------------------------------------------------*/
#ifndef GRAPHICS_DISABLED
void IMDebugBestMatch(int BestMatch,
                      INT_RESULT Result,
                      UINT16 BlobLength,
                      UINT8 NormalizationFactor) {
/*
 **      Parameters:
 **      Globals:
 **      Operation:
 **              Find the best match for the current class and update the Result
 **      Return:
 **      Exceptions: none
 **      History: Wed Feb 27 14:12:28 MST 1991, RWM, Created.
 */
  cprintf ("Rating          = %5.1f%%     Best Config   = %3d\n",
    100.0 * ((*Result).Rating), (int) ((*Result).Config));
  cprintf
    ("Matcher Error   = %5.1f%%     Blob Length   = %3d     Weight = %4.1f%%\n",
    100.0 * (65536.0 - BestMatch) / 65536.0, (int) BlobLength,
    100.0 * BlobLength / (BlobLength + LocalMatcherMultiplier));
  cprintf
    ("Char Norm Error = %5.1f%%     Norm Strength = %3d     Weight = %4.1f%%\n",
    100.0 * NormalizationFactor / 256.0, LocalMatcherMultiplier,
    100.0 * LocalMatcherMultiplier / (BlobLength + LocalMatcherMultiplier));
}
#endif

/*---------------------------------------------------------------------------*/
void
HeapSort (int n, register INT16 ra[], register UINT8 rb[]) {
/*
 **      Parameters:
 **              n      Number of elements to sort
 **              ra     Key array [1..n]
 **              rb     Index array [1..n]
 **      Globals:
 **      Operation:
 **              Sort Key array in ascending order using heap sort
 **              algorithm.  Also sort Index array that is tied to
 **              the key array.
 **      Return:
 **      Exceptions: none
 **      History: Tue Feb 19 10:24:24 MST 1991, RWM, Created.
 */
  register int i, rra, rrb;
  int l, j, ir;

  l = (n >> 1) + 1;
  ir = n;
  for (;;) {
    if (l > 1) {
      rra = ra[--l];
      rrb = rb[l];
    }
    else {
      rra = ra[ir];
      rrb = rb[ir];
      ra[ir] = ra[1];
      rb[ir] = rb[1];
      if (--ir == 1) {
        ra[1] = rra;
        rb[1] = rrb;
        return;
      }
    }
    i = l;
    j = l << 1;
    while (j <= ir) {
      if (j < ir && ra[j] < ra[j + 1])
        ++j;
      if (rra < ra[j]) {
        ra[i] = ra[j];
        rb[i] = rb[j];
        j += (i = j);
      }
      else
        j = ir + 1;
    }
    ra[i] = rra;
    rb[i] = rrb;
  }
}
