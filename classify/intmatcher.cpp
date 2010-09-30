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
/*----------------------------------------------------------------------------
                          Include Files and Type Defines
----------------------------------------------------------------------------*/
#include "intmatcher.h"
#include "intproto.h"
#include "tordvars.h"
#include "callcpp.h"
#include "scrollview.h"
#include "globals.h"
#include "classify.h"
#include <math.h>

// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif

#define CLASS_MASK_SIZE ((MAX_NUM_CLASSES*NUM_BITS_PER_CLASS \
		+BITS_PER_WERD-1)/BITS_PER_WERD)

/*----------------------------------------------------------------------------
                    Global Data Definitions and Declarations
----------------------------------------------------------------------------*/
#define  SE_TABLE_BITS    9
#define  SE_TABLE_SIZE  512
#define TEMPLATE_CACHE 2
static uinT8 SimilarityEvidenceTable[SE_TABLE_SIZE];
static uinT8 offset_table[256] = {
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
static uinT8 next_table[256] = {
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

static uinT32 EvidenceTableMask;

static uinT32 MultTruncShiftBits;

static uinT32 TableTruncShiftBits;

uinT32 EvidenceMultMask;

static inT16 LocalMatcherMultiplier;

INT_VAR(classify_class_pruner_threshold, 229,
        "Class Pruner Threshold 0-255:        ");

INT_VAR(classify_class_pruner_multiplier, 30,
        "Class Pruner Multiplier 0-255:       ");

INT_VAR(classify_integer_matcher_multiplier, 14,
        "Integer Matcher Multiplier  0-255:   ");

INT_VAR(classify_int_theta_fudge, 128,
        "Integer Matcher Theta Fudge 0-255:   ");

INT_VAR(classify_cp_cutoff_strength, 7,
        "Class Pruner CutoffStrength:         ");

INT_VAR(classify_evidence_table_bits, 9,
        "Bits in Similarity to Evidence Lookup  8-9:   ");

INT_VAR(classify_int_evidence_trunc_bits, 14,
        "Integer Evidence Truncation Bits (Distance) 8-14:   ");

double_VAR(classify_se_exponential_multiplier, 0,
                "Similarity to Evidence Table Exponential Multiplier: ");

double_VAR(classify_similarity_center, 0.0075,
           "Center of Similarity Curve: ");

INT_VAR(classify_adapt_proto_thresh, 230,
        "Threshold for good protos during adaptive 0-255:   ");

INT_VAR(classify_adapt_feature_thresh, 230,
        "Threshold for good features during adaptive 0-255:   ");

BOOL_VAR(disable_character_fragments, FALSE,
         "Do not include character fragments in the"
         " results of the classifier");

BOOL_VAR(matcher_debug_separate_windows, FALSE,
         "Use two different windows for debugging the matching: "
         "One for the protos and one for the features.");

int protoword_lookups;
int zero_protowords;
int proto_shifts;
int set_proto_bits;
int config_shifts;
int set_config_bits;

/*----------------------------------------------------------------------------
              Public Code
----------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
namespace tesseract {
int Classify::ClassPruner(INT_TEMPLATES IntTemplates,
                          inT16 NumFeatures,
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
 **              classify_class_pruner_threshold   Cutoff threshold
 **              classify_class_pruner_multiplier  Normalization factor multiplier
 **      Operation:
 **              Prune the classes using a modified fast match table.
 **              Return a sorted list of classes along with the number
 **              of pruned classes in that list.
 **      Return: Number of pruned classes.
 **      Exceptions: none
 **      History: Tue Feb 19 10:24:24 MST 1991, RWM, Created.
 */
  uinT32 PrunerWord;
  inT32 class_index;             //index to class
  int Word;
  uinT32 *BasePrunerAddress;
  uinT32 feature_address;        //current feature index
  INT_FEATURE feature;           //current feature
  CLASS_PRUNER *ClassPruner;
  int PrunerSet;
  int NumPruners;
  inT32 feature_index;           //current feature

  static int ClassCount[MAX_NUM_CLASSES];
  static int NormCount[MAX_NUM_CLASSES];
  static int SortKey[MAX_NUM_CLASSES + 1];
  static int SortIndex[MAX_NUM_CLASSES + 1];
  int out_class;
  int MaxNumClasses;
  int MaxCount;
  int NumClasses;
  FLOAT32 max_rating;            //max allowed rating
  int *ClassCountPtr;
  CLASS_ID class_id;

  MaxNumClasses = IntTemplates->NumClasses;

  /* Clear Class Counts */
  ClassCountPtr = &(ClassCount[0]);
  for (class_id = 0; class_id < MaxNumClasses; class_id++) {
    *ClassCountPtr++ = 0;
  }

  /* Update Class Counts */
  NumPruners = IntTemplates->NumClassPruners;
  for (feature_index = 0; feature_index < NumFeatures; feature_index++) {
    feature = &Features[feature_index];
    feature_address = (((feature->X * NUM_CP_BUCKETS >> 8) * NUM_CP_BUCKETS +
                        (feature->Y * NUM_CP_BUCKETS >> 8)) * NUM_CP_BUCKETS +
                       (feature->Theta * NUM_CP_BUCKETS >> 8)) << 1;
    ClassPruner = IntTemplates->ClassPruner;
    class_index = 0;

    for (PrunerSet = 0; PrunerSet < NumPruners; PrunerSet++, ClassPruner++) {
      BasePrunerAddress = (uinT32 *) (*ClassPruner) + feature_address;

      for (Word = 0; Word < WERDS_PER_CP_VECTOR; Word++) {
        PrunerWord = *BasePrunerAddress++;
        // This inner loop is unrolled to speed up the ClassPruner.
        // Currently gcc would not unroll it unless it is set to O3
        // level of optimization or -funroll-loops is specified.
        /*
        uinT32 class_mask = (1 << NUM_BITS_PER_CLASS) - 1;
        for (int bit = 0; bit < BITS_PER_WERD/NUM_BITS_PER_CLASS; bit++) {
          ClassCount[class_index++] += PrunerWord & class_mask;
          PrunerWord >>= NUM_BITS_PER_CLASS;
        }
        */
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
  for (class_id = 0; class_id < MaxNumClasses; class_id++) {
    if (NumFeatures < ExpectedNumFeatures[class_id]) {
      int deficit = ExpectedNumFeatures[class_id] - NumFeatures;
      ClassCount[class_id] -= ClassCount[class_id] * deficit /
                           (NumFeatures*classify_cp_cutoff_strength + deficit);
    }
    if (!unicharset.get_enabled(class_id))
      ClassCount[class_id] = 0;  // This char is disabled!

    // Do not include character fragments in the class pruner
    // results if disable_character_fragments is true.
    if (disable_character_fragments && unicharset.get_fragment(class_id)) {
      ClassCount[class_id] = 0;
    }
  }

  /* Adjust Class Counts for Normalization Factors */
  MaxCount = 0;
  for (class_id = 0; class_id < MaxNumClasses; class_id++) {
    NormCount[class_id] = ClassCount[class_id]
      - ((classify_class_pruner_multiplier * NormalizationFactors[class_id]) >> 8)
      * cp_maps[3] / 3;
    if (NormCount[class_id] > MaxCount &&
        // This additional check is added in order to ensure that
        // the classifier will return at least one non-fragmented
        // character match.
        // TODO(daria): verify that this helps accuracy and does not
        // hurt performance.
        !unicharset.get_fragment(class_id)) {
      MaxCount = NormCount[class_id];
    }
  }

  /* Prune Classes */
  MaxCount *= classify_class_pruner_threshold;
  MaxCount >>= 8;
  /* Select Classes */
  if (MaxCount < 1)
    MaxCount = 1;
  NumClasses = 0;
  for (class_id = 0; class_id < MaxNumClasses; class_id++) {
    if (NormCount[class_id] >= MaxCount) {
      NumClasses++;
      SortIndex[NumClasses] = class_id;
      SortKey[NumClasses] = NormCount[class_id];
    }
  }

  /* Sort Classes using Heapsort Algorithm */
  if (NumClasses > 1)
    HeapSort(NumClasses, SortKey, SortIndex);

  if (tord_display_ratings > 1) {
    cprintf ("CP:%d classes, %d features:\n", NumClasses, NumFeatures);
    for (class_id = 0; class_id < NumClasses; class_id++) {
      cprintf ("%s:C=%d, E=%d, N=%d, Rat=%d\n",
               unicharset.debug_str(SortIndex[NumClasses - class_id]).string(),
               ClassCount[SortIndex[NumClasses - class_id]],
               ExpectedNumFeatures[SortIndex[NumClasses - class_id]],
               SortKey[NumClasses - class_id],
               1010 - 1000 * SortKey[NumClasses - class_id] /
                 (cp_maps[3] * NumFeatures));
    }
    if (tord_display_ratings > 2) {
      NumPruners = IntTemplates->NumClassPruners;
      for (feature_index = 0; feature_index < NumFeatures;
      feature_index++) {
        cprintf ("F=%3d,", feature_index);
        feature = &Features[feature_index];
        feature_address =
          (((feature->X * NUM_CP_BUCKETS >> 8) * NUM_CP_BUCKETS +
          (feature->Y * NUM_CP_BUCKETS >> 8)) * NUM_CP_BUCKETS +
          (feature->Theta * NUM_CP_BUCKETS >> 8)) << 1;
        ClassPruner = IntTemplates->ClassPruner;
        class_index = 0;
        for (PrunerSet = 0; PrunerSet < NumPruners;
        PrunerSet++, ClassPruner++) {
          BasePrunerAddress = (uinT32 *) (*ClassPruner)
            + feature_address;

          for (Word = 0; Word < WERDS_PER_CP_VECTOR; Word++) {
            PrunerWord = *BasePrunerAddress++;
            for (class_id = 0; class_id < 16; class_id++, class_index++) {
              if (NormCount[class_index] >= MaxCount)
                cprintf (" %s=%d,",
                  unicharset.id_to_unichar(class_index),
                  PrunerWord & 3);
              PrunerWord >>= 2;
            }
          }
        }
        cprintf ("\n");
      }
      cprintf ("Adjustments:");
      for (class_id = 0; class_id < MaxNumClasses; class_id++) {
        if (NormCount[class_id] > MaxCount)
          cprintf (" %s=%d,",
            unicharset.id_to_unichar(class_id),
            -((classify_class_pruner_multiplier *
            NormalizationFactors[class_id]) >> 8) * cp_maps[3] / 3);
      }
      cprintf ("\n");
    }
  }

  /* Set Up Results */
  max_rating = 0.0f;
  for (class_id = 0, out_class = 0; class_id < NumClasses; class_id++) {
    Results[out_class].Class = SortIndex[NumClasses - class_id];
    Results[out_class].Rating =
      1.0 - SortKey[NumClasses -
      class_id] / ((float) cp_maps[3] * NumFeatures);
    out_class++;
  }
  NumClasses = out_class;
  return NumClasses;

}
}  // namespace tesseract

/*---------------------------------------------------------------------------*/
void IntegerMatcher(INT_CLASS ClassTemplate,
                    BIT_VECTOR ProtoMask,
                    BIT_VECTOR ConfigMask,
                    uinT16 BlobLength,
                    inT16 NumFeatures,
                    INT_FEATURE_ARRAY Features,
                    uinT8 NormalizationFactor,
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
 **              classify_int_theta_fudge             Theta fudge factor used for
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
  static uinT8 FeatureEvidence[MAX_NUM_CONFIGS];
  static int SumOfFeatureEvidence[MAX_NUM_CONFIGS];
  static uinT8 ProtoEvidence[MAX_NUM_PROTOS][MAX_PROTO_INDEX];
  int Feature;
  int BestMatch;

  if (MatchDebuggingOn (Debug))
    cprintf ("Integer Matcher -------------------------------------------\n");

  IMClearTables(ClassTemplate, SumOfFeatureEvidence, ProtoEvidence);
  Result->FeatureMisses = 0;

  for (Feature = 0; Feature < NumFeatures; Feature++) {
    int csum = IMUpdateTablesForFeature(ClassTemplate, ProtoMask, ConfigMask,
                                        Feature, &(Features[Feature]),
                                        FeatureEvidence, SumOfFeatureEvidence,
                                        ProtoEvidence, Debug);
    // Count features that were missed over all configs.
    if (csum == 0)
      Result->FeatureMisses++;
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
                            NumFeatures);

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
                   uinT16 BlobLength,
                   inT16 NumFeatures,
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
 **              classify_int_theta_fudge             Theta fudge factor used for
 **                                        evidence calculation
 **              classify_adapt_proto_thresh          Threshold for good protos
 **      Operation:
 **              FindGoodProtos finds all protos whose normalized proto-evidence
 **              exceed classify_adapt_proto_thresh.  The list is ordered by increasing
 **              proto id number.
 **      Return:
 **              Number of good protos in ProtoArray.
 **      Exceptions: none
 **      History: Tue Mar 12 17:09:26 MST 1991, RWM, Created
 */
  static uinT8 FeatureEvidence[MAX_NUM_CONFIGS];
  static int SumOfFeatureEvidence[MAX_NUM_CONFIGS];
  static uinT8 ProtoEvidence[MAX_NUM_PROTOS][MAX_PROTO_INDEX];
  int Feature;
  register uinT8 *UINT8Pointer;
  register int ProtoIndex;
  int NumProtos;
  int NumGoodProtos;
  uinT16 ActualProtoNum;
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
  NumProtos = ClassTemplate->NumProtos;
  NumGoodProtos = 0;
  for (ActualProtoNum = 0; ActualProtoNum < NumProtos; ActualProtoNum++) {
    /* Compute Average for Actual Proto */
    Temp = 0;
    UINT8Pointer = &(ProtoEvidence[ActualProtoNum][0]);
    for (ProtoIndex = ClassTemplate->ProtoLengths[ActualProtoNum];
      ProtoIndex > 0; ProtoIndex--, UINT8Pointer++)
    Temp += *UINT8Pointer;

    Temp /= ClassTemplate->ProtoLengths[ActualProtoNum];

    /* Find Good Protos */
    if (Temp >= classify_adapt_proto_thresh) {
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
                    uinT16 BlobLength,
                    inT16 NumFeatures,
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
 **              classify_int_theta_fudge             Theta fudge factor used for
 **                                        evidence calculation
 **              classify_adapt_feature_thresh        Threshold for bad features
 **      Operation:
 **              FindBadFeatures finds all features whose maximum feature-evidence
 **              was less than classify_adapt_feature_thresh.  The list is ordered by increasing
 **              feature number.
 **      Return:
 **              Number of bad features in FeatureArray.
 **      Exceptions: none
 **      History: Tue Mar 12 17:09:26 MST 1991, RWM, Created
 */
  static uinT8 FeatureEvidence[MAX_NUM_CONFIGS];
  static int SumOfFeatureEvidence[MAX_NUM_CONFIGS];
  static uinT8 ProtoEvidence[MAX_NUM_PROTOS][MAX_PROTO_INDEX];
  int Feature;
  register uinT8 *UINT8Pointer;
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
  NumConfigs = ClassTemplate->NumConfigs;
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
    if (Temp < classify_adapt_feature_thresh) {
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
  uinT32 IntSimilarity;
  double Similarity;
  double Evidence;
  double ScaleFactor;

  /* Set default mode of operation of IntegerMatcher */
  SetCharNormMatch();

  /* Initialize table for evidence to similarity lookup */
  for (i = 0; i < SE_TABLE_SIZE; i++) {
    IntSimilarity = i << (27 - SE_TABLE_BITS);
    Similarity = ((double) IntSimilarity) / 65536.0 / 65536.0;
    Evidence = Similarity / classify_similarity_center;
    Evidence *= Evidence;
    Evidence += 1.0;
    Evidence = 1.0 / Evidence;
    Evidence *= 255.0;

    if (classify_se_exponential_multiplier > 0.0) {
      ScaleFactor = 1.0 - exp (-classify_se_exponential_multiplier) *
        exp (classify_se_exponential_multiplier * ((double) i / SE_TABLE_SIZE));
      if (ScaleFactor > 1.0)
        ScaleFactor = 1.0;
      if (ScaleFactor < 0.0)
        ScaleFactor = 0.0;
      Evidence *= ScaleFactor;
    }

    SimilarityEvidenceTable[i] = (uinT8) (Evidence + 0.5);
  }

  /* Initialize evidence computation variables */
  EvidenceTableMask =
    ((1 << classify_evidence_table_bits) - 1) << (9 - classify_evidence_table_bits);
  MultTruncShiftBits = (14 - classify_int_evidence_trunc_bits);
  TableTruncShiftBits = (27 - SE_TABLE_BITS - (MultTruncShiftBits << 1));
  EvidenceMultMask = ((1 << classify_int_evidence_trunc_bits) - 1);

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
  classify_adapt_proto_thresh.set_value(255 * Threshold);
  if (classify_adapt_proto_thresh < 0)
    classify_adapt_proto_thresh.set_value(0);
  if (classify_adapt_proto_thresh > 255)
    classify_adapt_proto_thresh.set_value(255);
}


/*---------------------------------------------------------------------------*/
void SetFeatureThresh(FLOAT32 Threshold) {
  classify_adapt_feature_thresh.set_value(255 * Threshold);
  if (classify_adapt_feature_thresh < 0)
    classify_adapt_feature_thresh.set_value(0);
  if (classify_adapt_feature_thresh > 255)
    classify_adapt_feature_thresh.set_value(255);
}


/*--------------------------------------------------------------------------*/
void SetBaseLineMatch() {
  LocalMatcherMultiplier = 0;
}


/*--------------------------------------------------------------------------*/
void SetCharNormMatch() {
  LocalMatcherMultiplier = classify_integer_matcher_multiplier;
}


/**----------------------------------------------------------------------------
              Private Code
----------------------------------------------------------------------------**/
/*---------------------------------------------------------------------------*/
void
IMClearTables (INT_CLASS ClassTemplate,
int SumOfFeatureEvidence[MAX_NUM_CONFIGS],
uinT8 ProtoEvidence[MAX_NUM_PROTOS][MAX_PROTO_INDEX]) {
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
  int NumProtos = ClassTemplate->NumProtos;
  int NumConfigs = ClassTemplate->NumConfigs;

  memset(SumOfFeatureEvidence, 0,
         NumConfigs * sizeof(SumOfFeatureEvidence[0]));
  memset(ProtoEvidence, 0,
         NumProtos * sizeof(ProtoEvidence[0]));
}


/*---------------------------------------------------------------------------*/
void
IMClearFeatureEvidenceTable (uinT8 FeatureEvidence[MAX_NUM_CONFIGS],
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
  memset(FeatureEvidence, 0, NumConfigs * sizeof(*FeatureEvidence));
}


/*---------------------------------------------------------------------------*/
void IMDebugConfiguration(int FeatureNum,
                          uinT16 ActualProtoNum,
                          uinT8 Evidence,
                          BIT_VECTOR ConfigMask,
                          uinT32 ConfigWord) {
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
                             uinT8 *FeatureEvidence,
                             inT32 ConfigCount) {
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
int
IMUpdateTablesForFeature (INT_CLASS ClassTemplate,
BIT_VECTOR ProtoMask,
BIT_VECTOR ConfigMask,
int FeatureNum,
INT_FEATURE Feature,
uinT8 FeatureEvidence[MAX_NUM_CONFIGS],
int SumOfFeatureEvidence[MAX_NUM_CONFIGS],
uinT8
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
  register uinT32 ConfigWord;
  register uinT32 ProtoWord;
  register uinT32 ProtoNum;
  register uinT32 ActualProtoNum;
  uinT8 proto_byte;
  inT32 proto_word_offset;
  inT32 proto_offset;
  uinT8 config_byte;
  inT32 config_offset;
  PROTO_SET ProtoSet;
  uinT32 *ProtoPrunerPtr;
  INT_PROTO Proto;
  int ProtoSetIndex;
  uinT8 Evidence;
  uinT32 XFeatureAddress;
  uinT32 YFeatureAddress;
  uinT32 ThetaFeatureAddress;
  register uinT8 *UINT8Pointer;
  register int ProtoIndex;
  uinT8 Temp;
  register int *IntPointer;
  int ConfigNum;
  register inT32 M3;
  register inT32 A3;
  register uinT32 A4;

  IMClearFeatureEvidenceTable(FeatureEvidence, ClassTemplate->NumConfigs);

  /* Precompute Feature Address offset for Proto Pruning */
  XFeatureAddress = ((Feature->X >> 2) << 1);
  YFeatureAddress = (NUM_PP_BUCKETS << 1) + ((Feature->Y >> 2) << 1);
  ThetaFeatureAddress = (NUM_PP_BUCKETS << 2) + ((Feature->Theta >> 2) << 1);

  for (ProtoSetIndex = 0, ActualProtoNum = 0;
  ProtoSetIndex < ClassTemplate->NumProtoSets; ProtoSetIndex++) {
    ProtoSet = ClassTemplate->ProtoSets[ProtoSetIndex];
    ProtoPrunerPtr = (uinT32 *) ((*ProtoSet).ProtoPruner);
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
            (((inT8) (Feature->Theta - Proto->Angle)) *
            classify_int_theta_fudge) << 1;

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
            ClassTemplate->ProtoLengths[ActualProtoNum + proto_offset];
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
      ClassTemplate->NumConfigs);
  IntPointer = SumOfFeatureEvidence;
  UINT8Pointer = FeatureEvidence;
  int SumOverConfigs = 0;
  for (ConfigNum = ClassTemplate->NumConfigs; ConfigNum > 0; ConfigNum--) {
    int evidence = *UINT8Pointer++;
    SumOverConfigs += evidence;
    *IntPointer++ += evidence;
  }
  return SumOverConfigs;
}


/*---------------------------------------------------------------------------*/
#ifndef GRAPHICS_DISABLED
void
IMDebugFeatureProtoError (INT_CLASS ClassTemplate,
BIT_VECTOR ProtoMask,
BIT_VECTOR ConfigMask,
int SumOfFeatureEvidence[MAX_NUM_CONFIGS],
uinT8
ProtoEvidence[MAX_NUM_PROTOS][MAX_PROTO_INDEX],
inT16 NumFeatures, int Debug) {
/*
 **      Parameters:
 **      Globals:
 **      Operation:
 **              Print debugging information for Configuations
 **      Return:
 **      Exceptions: none
 **      History: Wed Feb 27 14:12:28 MST 1991, RWM, Created.
 */
  uinT8 *UINT8Pointer;
  int *IntPointer;
  FLOAT32 ProtoConfigs[MAX_NUM_CONFIGS];
  int ConfigNum;
  uinT32 ConfigWord;
  int ProtoSetIndex;
  uinT16 ProtoNum;
  uinT8 ProtoWordNum;
  PROTO_SET ProtoSet;
  int ProtoIndex;
  int NumProtos;
  uinT16 ActualProtoNum;
  int Temp;
  int NumConfigs;

  NumProtos = ClassTemplate->NumProtos;
  NumConfigs = ClassTemplate->NumConfigs;

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
    for (ProtoSetIndex = 0; ProtoSetIndex < ClassTemplate->NumProtoSets;
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
    for (ProtoSetIndex = 0; ProtoSetIndex < ClassTemplate->NumProtoSets;
    ProtoSetIndex++) {
      ProtoSet = ClassTemplate->ProtoSets[ProtoSetIndex];
      ActualProtoNum = (ProtoSetIndex * PROTOS_PER_PROTO_SET);
      for (ProtoNum = 0;
        ((ProtoNum < PROTOS_PER_PROTO_SET)
        && (ActualProtoNum < NumProtos));
      ProtoNum++, ActualProtoNum++) {
        cprintf ("P %3d =", ActualProtoNum);
        Temp = 0;
        UINT8Pointer = &(ProtoEvidence[ActualProtoNum][0]);
        for (ProtoIndex = 0;
          ProtoIndex < ClassTemplate->ProtoLengths[ActualProtoNum];
        ProtoIndex++, UINT8Pointer++) {
          cprintf (" %d", *UINT8Pointer);
          Temp += *UINT8Pointer;
        }

        cprintf (" = %6.4f%%\n", Temp /
          256.0 / ClassTemplate->ProtoLengths[ActualProtoNum]);

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
        ClassTemplate->ConfigLengths[ConfigNum] / 256.0));
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
        (float) ClassTemplate->ConfigLengths[ConfigNum]);
    cprintf ("\n\n");
  }

}


/*---------------------------------------------------------------------------*/
void
IMDisplayProtoDebugInfo (INT_CLASS ClassTemplate,
BIT_VECTOR ProtoMask,
BIT_VECTOR ConfigMask,
uinT8 ProtoEvidence[MAX_NUM_PROTOS][MAX_PROTO_INDEX],
int Debug) {
  register uinT8 *UINT8Pointer;
  register uinT32 ConfigWord;
  register uinT16 ProtoNum;
  register uinT16 ActualProtoNum;
  PROTO_SET ProtoSet;
  int ProtoSetIndex;
  int ProtoIndex;
  int NumProtos;
  register int Temp;

  InitIntMatchWindowIfReqd();
  if (matcher_debug_separate_windows) {
    InitFeatureDisplayWindowIfReqd();
    InitProtoDisplayWindowIfReqd();
  }

  NumProtos = ClassTemplate->NumProtos;

  for (ProtoSetIndex = 0; ProtoSetIndex < ClassTemplate->NumProtoSets;
  ProtoSetIndex++) {
    ProtoSet = ClassTemplate->ProtoSets[ProtoSetIndex];
    ActualProtoNum = (ProtoSetIndex * PROTOS_PER_PROTO_SET);
    for (ProtoNum = 0;
      ((ProtoNum < PROTOS_PER_PROTO_SET)
    && (ActualProtoNum < NumProtos)); ProtoNum++, ActualProtoNum++) {
      /* Compute Average for Actual Proto */
      Temp = 0;
      UINT8Pointer = &(ProtoEvidence[ActualProtoNum][0]);
      for (ProtoIndex = ClassTemplate->ProtoLengths[ActualProtoNum];
        ProtoIndex > 0; ProtoIndex--, UINT8Pointer++)
      Temp += *UINT8Pointer;

      Temp /= ClassTemplate->ProtoLengths[ActualProtoNum];

      ConfigWord = (ProtoSet->Protos[ProtoNum]).Configs[0];
      ConfigWord &= *ConfigMask;
      if (ConfigWord) {
        /* Update display for current proto */
        if (ClipMatchEvidenceOn (Debug)) {
          if (Temp < classify_adapt_proto_thresh)
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
}


/*---------------------------------------------------------------------------*/
void IMDisplayFeatureDebugInfo(INT_CLASS ClassTemplate,
                               BIT_VECTOR ProtoMask,
                               BIT_VECTOR ConfigMask,
                               inT16 NumFeatures,
                               INT_FEATURE_ARRAY Features,
                               int Debug) {
  static uinT8 FeatureEvidence[MAX_NUM_CONFIGS];
  static int SumOfFeatureEvidence[MAX_NUM_CONFIGS];
  static uinT8 ProtoEvidence[MAX_NUM_PROTOS][MAX_PROTO_INDEX];
  int Feature;
  register uinT8 *UINT8Pointer;
  register int ConfigNum;
  int NumConfigs;
  register int Temp;

  IMClearTables(ClassTemplate, SumOfFeatureEvidence, ProtoEvidence);

  InitIntMatchWindowIfReqd();
  if (matcher_debug_separate_windows) {
    InitFeatureDisplayWindowIfReqd();
    InitProtoDisplayWindowIfReqd();
  }

  NumConfigs = ClassTemplate->NumConfigs;
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
      if (Temp < classify_adapt_feature_thresh)
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
uinT8
ProtoEvidence[MAX_NUM_PROTOS][MAX_PROTO_INDEX],
inT16 NumFeatures) {
/*
 **      Parameters:
 **      Globals:
 **      Operation:
 **              Add sum of Proto Evidences into Sum Of Feature Evidence Array
 **      Return:
 **      Exceptions: none
 **      History: Wed Feb 27 14:12:28 MST 1991, RWM, Created.
 */
  register uinT8 *UINT8Pointer;
  register int *IntPointer;
  register uinT32 ConfigWord;
  int ProtoSetIndex;
  register uinT16 ProtoNum;
  PROTO_SET ProtoSet;
  register int ProtoIndex;
  int NumProtos;
  uinT16 ActualProtoNum;
  int Temp;

  NumProtos = ClassTemplate->NumProtos;

  for (ProtoSetIndex = 0; ProtoSetIndex < ClassTemplate->NumProtoSets;
  ProtoSetIndex++) {
    ProtoSet = ClassTemplate->ProtoSets[ProtoSetIndex];
    ActualProtoNum = (ProtoSetIndex * PROTOS_PER_PROTO_SET);
    for (ProtoNum = 0;
      ((ProtoNum < PROTOS_PER_PROTO_SET)
    && (ActualProtoNum < NumProtos)); ProtoNum++, ActualProtoNum++) {
      Temp = 0;
      UINT8Pointer = &(ProtoEvidence[ActualProtoNum][0]);
      for (ProtoIndex = ClassTemplate->ProtoLengths[ActualProtoNum];
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
IMNormalizeSumOfEvidences (INT_CLASS ClassTemplate,
int SumOfFeatureEvidence[MAX_NUM_CONFIGS],
inT16 NumFeatures, inT32 used_features) {
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

  NumConfigs = ClassTemplate->NumConfigs;

  IntPointer = SumOfFeatureEvidence;
  for (ConfigNum = 0; ConfigNum < NumConfigs; ConfigNum++, IntPointer++)
    *IntPointer = (*IntPointer << 8) /
      (NumFeatures + ClassTemplate->ConfigLengths[ConfigNum]);
}


/*---------------------------------------------------------------------------*/
int
IMFindBestMatch (INT_CLASS ClassTemplate,
int SumOfFeatureEvidence[MAX_NUM_CONFIGS],
uinT16 BlobLength,
uinT8 NormalizationFactor, INT_RESULT Result) {
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

  NumConfigs = ClassTemplate->NumConfigs;

  /* Find best match */
  BestMatch = 0;
  Best2Match = 0;
  IntPointer = SumOfFeatureEvidence;
  for (ConfigNum = 0; ConfigNum < NumConfigs; ConfigNum++, IntPointer++) {
    if (tord_display_ratings > 1)
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
                      uinT16 BlobLength,
                      uinT8 NormalizationFactor) {
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
HeapSort (int n, register int ra[], register int rb[]) {
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
