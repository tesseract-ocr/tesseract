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
#include "callcpp.h"
#include "scrollview.h"
#include "globals.h"
#include "classify.h"
#include <math.h>

// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif

/*----------------------------------------------------------------------------
                    Global Data Definitions and Declarations
----------------------------------------------------------------------------*/

static const uinT8 offset_table[256] = {
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

static const uinT8 next_table[256] = {
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

struct ClassPrunerData {
  int *class_count_;
  int *norm_count_;
  int *sort_key_;
  int *sort_index_;
  int max_classes_;

  ClassPrunerData(int max_classes) {
    // class_count_ and friends are referenced by indexing off of data in
    //   class pruner word sized chunks.  Each pruner word is of sized
    //   BITS_PER_WERD and each entry is NUM_BITS_PER_CLASS, so there are
    //   BITS_PER_WERD / NUM_BITS_PER_CLASS entries.
    //   See Classify::ClassPruner in intmatcher.cpp.
    max_classes_ = RoundUp(
        max_classes, WERDS_PER_CP_VECTOR * BITS_PER_WERD / NUM_BITS_PER_CLASS);
    class_count_ = new int[max_classes_];
    norm_count_ = new int[max_classes_];
    sort_key_ = new int[max_classes_ + 1];
    sort_index_ = new int[max_classes_ + 1];
    for (int i = 0; i < max_classes_; i++) {
      class_count_[i] = 0;
    }
  }

  ~ClassPrunerData() {
    delete []class_count_;
    delete []norm_count_;
    delete []sort_key_;
    delete []sort_index_;
  }

};

const float IntegerMatcher::kSEExponentialMultiplier = 0.0;
const float IntegerMatcher::kSimilarityCenter = 0.0075;

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

  int MaxNumClasses = IntTemplates->NumClasses;
  ClassPrunerData data(IntTemplates->NumClasses);
  int *ClassCount = data.class_count_;
  int *NormCount = data.norm_count_;
  int *SortKey = data.sort_key_;
  int *SortIndex = data.sort_index_;

  int out_class;
  int MaxCount;
  int NumClasses;
  FLOAT32 max_rating;            //max allowed rating
  CLASS_ID class_id;

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
        ClassCount[class_index++] += PrunerWord & CLASS_PRUNER_CLASS_MASK;
        PrunerWord >>= NUM_BITS_PER_CLASS;
        ClassCount[class_index++] += PrunerWord & CLASS_PRUNER_CLASS_MASK;
        PrunerWord >>= NUM_BITS_PER_CLASS;
        ClassCount[class_index++] += PrunerWord & CLASS_PRUNER_CLASS_MASK;
        PrunerWord >>= NUM_BITS_PER_CLASS;
        ClassCount[class_index++] += PrunerWord & CLASS_PRUNER_CLASS_MASK;
        PrunerWord >>= NUM_BITS_PER_CLASS;
        ClassCount[class_index++] += PrunerWord & CLASS_PRUNER_CLASS_MASK;
        PrunerWord >>= NUM_BITS_PER_CLASS;
        ClassCount[class_index++] += PrunerWord & CLASS_PRUNER_CLASS_MASK;
        PrunerWord >>= NUM_BITS_PER_CLASS;
        ClassCount[class_index++] += PrunerWord & CLASS_PRUNER_CLASS_MASK;
        PrunerWord >>= NUM_BITS_PER_CLASS;
        ClassCount[class_index++] += PrunerWord & CLASS_PRUNER_CLASS_MASK;
        PrunerWord >>= NUM_BITS_PER_CLASS;
        ClassCount[class_index++] += PrunerWord & CLASS_PRUNER_CLASS_MASK;
        PrunerWord >>= NUM_BITS_PER_CLASS;
        ClassCount[class_index++] += PrunerWord & CLASS_PRUNER_CLASS_MASK;
        PrunerWord >>= NUM_BITS_PER_CLASS;
        ClassCount[class_index++] += PrunerWord & CLASS_PRUNER_CLASS_MASK;
        PrunerWord >>= NUM_BITS_PER_CLASS;
        ClassCount[class_index++] += PrunerWord & CLASS_PRUNER_CLASS_MASK;
        PrunerWord >>= NUM_BITS_PER_CLASS;
        ClassCount[class_index++] += PrunerWord & CLASS_PRUNER_CLASS_MASK;
        PrunerWord >>= NUM_BITS_PER_CLASS;
        ClassCount[class_index++] += PrunerWord & CLASS_PRUNER_CLASS_MASK;
        PrunerWord >>= NUM_BITS_PER_CLASS;
        ClassCount[class_index++] += PrunerWord & CLASS_PRUNER_CLASS_MASK;
        PrunerWord >>= NUM_BITS_PER_CLASS;
        ClassCount[class_index++] += PrunerWord & CLASS_PRUNER_CLASS_MASK;
      }
    }
  }

  /* Adjust Class Counts for Number of Expected Features */
  for (class_id = 0; class_id < MaxNumClasses; class_id++) {
    if (NumFeatures < ExpectedNumFeatures[class_id]) {
      int deficit = ExpectedNumFeatures[class_id] - NumFeatures;
      ClassCount[class_id] -= ClassCount[class_id] * deficit /
        (NumFeatures * classify_cp_cutoff_strength + deficit);
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
      - ((classify_class_pruner_multiplier * NormalizationFactors[class_id])
          >> 8);
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

  if (classify_debug_level > 1) {
    cprintf ("CP:%d classes, %d features:\n", NumClasses, NumFeatures);
    for (class_id = 0; class_id < NumClasses; class_id++) {
      cprintf ("%s:C=%d, E=%d, N=%d, Rat=%d\n",
               unicharset.debug_str(SortIndex[NumClasses - class_id]).string(),
               ClassCount[SortIndex[NumClasses - class_id]],
               ExpectedNumFeatures[SortIndex[NumClasses - class_id]],
               SortKey[NumClasses - class_id],
               1010 - 1000 * SortKey[NumClasses - class_id] /
                 (CLASS_PRUNER_CLASS_MASK * NumFeatures));
    }
    if (classify_debug_level > 2) {
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
                  PrunerWord & CLASS_PRUNER_CLASS_MASK);
              PrunerWord >>= NUM_BITS_PER_CLASS;
            }
          }
        }
        cprintf ("\n");
      }
      cprintf ("Adjustments:");
      for (class_id = 0; class_id < MaxNumClasses; class_id++) {
        if (NormCount[class_id] > MaxCount)
          cprintf(" %s=%d,",
            unicharset.id_to_unichar(class_id),
            -((classify_class_pruner_multiplier *
               NormalizationFactors[class_id]) >> 8));
      }
      cprintf ("\n");
    }
  }

  /* Set Up Results */
  max_rating = 0.0f;
  for (class_id = 0, out_class = 0; class_id < NumClasses; class_id++) {
    Results[out_class].Class = SortIndex[NumClasses - class_id];
    Results[out_class].Rating =
      1.0 - SortKey[NumClasses - class_id] /
      (static_cast<float>(CLASS_PRUNER_CLASS_MASK) * NumFeatures);
    out_class++;
  }
  NumClasses = out_class;
  return NumClasses;
}

}  // namespace tesseract

/*---------------------------------------------------------------------------*/
void IntegerMatcher::Match(INT_CLASS ClassTemplate,
                           BIT_VECTOR ProtoMask,
                           BIT_VECTOR ConfigMask,
                           uinT16 BlobLength,
                           inT16 NumFeatures,
                           INT_FEATURE_ARRAY Features,
                           uinT8 NormalizationFactor,
                           INT_RESULT Result,
                           int AdaptFeatureThreshold,
                           int Debug,
                           bool SeparateDebugWindows) {
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
 **              local_matcher_multiplier_    Normalization factor multiplier
 **      Operation:
 **              IntegerMatcher returns the best configuration and rating
 **              for a single class.  The class matched against is determined
 **              by the uniqueness of the ClassTemplate parameter.  The
 **              best rating and its associated configuration are returned.
 **      Return:
 **      Exceptions: none
 **      History: Tue Feb 19 16:36:23 MST 1991, RWM, Created.
 */
  ScratchEvidence *tables = new ScratchEvidence();
  int Feature;
  int BestMatch;

  if (MatchDebuggingOn (Debug))
    cprintf ("Integer Matcher -------------------------------------------\n");

  tables->Clear(ClassTemplate);
  Result->FeatureMisses = 0;

  for (Feature = 0; Feature < NumFeatures; Feature++) {
    int csum = UpdateTablesForFeature(ClassTemplate, ProtoMask, ConfigMask,
                                      Feature, &Features[Feature],
                                      tables, Debug);
    // Count features that were missed over all configs.
    if (csum == 0)
      Result->FeatureMisses++;
  }

#ifndef GRAPHICS_DISABLED
  if (PrintProtoMatchesOn(Debug) || PrintMatchSummaryOn(Debug)) {
    DebugFeatureProtoError(ClassTemplate, ProtoMask, ConfigMask, *tables,
                           NumFeatures, Debug);
  }

  if (DisplayProtoMatchesOn(Debug)) {
    DisplayProtoDebugInfo(ClassTemplate, ProtoMask, ConfigMask,
                          *tables, SeparateDebugWindows);
  }

  if (DisplayFeatureMatchesOn(Debug)) {
    DisplayFeatureDebugInfo(ClassTemplate, ProtoMask, ConfigMask, NumFeatures,
                            Features, AdaptFeatureThreshold, Debug,
                            SeparateDebugWindows);
  }
#endif

  tables->UpdateSumOfProtoEvidences(ClassTemplate, ConfigMask, NumFeatures);
  tables->NormalizeSums(ClassTemplate, NumFeatures, NumFeatures);

  BestMatch = FindBestMatch(ClassTemplate, *tables, BlobLength,
                            NormalizationFactor, Result);

#ifndef GRAPHICS_DISABLED
  if (PrintMatchSummaryOn(Debug))
    DebugBestMatch(BestMatch, Result, BlobLength, NormalizationFactor);

  if (MatchDebuggingOn(Debug))
    cprintf("Match Complete --------------------------------------------\n");
#endif

  delete tables;
}


/*---------------------------------------------------------------------------*/
int IntegerMatcher::FindGoodProtos(
    INT_CLASS ClassTemplate,
    BIT_VECTOR ProtoMask,
    BIT_VECTOR ConfigMask,
    uinT16 BlobLength,
    inT16 NumFeatures,
    INT_FEATURE_ARRAY Features,
    PROTO_ID *ProtoArray,
    int AdaptProtoThreshold,
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
 **              AdaptProtoThreshold       Threshold for good protos
 **              Debug                     Debugger flag: 1=debugger on
 **      Globals:
 **              local_matcher_multiplier_    Normalization factor multiplier
 **      Operation:
 **              FindGoodProtos finds all protos whose normalized proto-evidence
 **              exceed classify_adapt_proto_thresh.  The list is ordered by increasing
 **              proto id number.
 **      Return:
 **              Number of good protos in ProtoArray.
 **      Exceptions: none
 **      History: Tue Mar 12 17:09:26 MST 1991, RWM, Created
 */
  ScratchEvidence *tables = new ScratchEvidence();
  int NumGoodProtos = 0;

  /* DEBUG opening heading */
  if (MatchDebuggingOn (Debug))
    cprintf
      ("Find Good Protos -------------------------------------------\n");

  tables->Clear(ClassTemplate);

  for (int Feature = 0; Feature < NumFeatures; Feature++)
    UpdateTablesForFeature(
        ClassTemplate, ProtoMask, ConfigMask, Feature, &(Features[Feature]),
        tables, Debug);

#ifndef GRAPHICS_DISABLED
  if (PrintProtoMatchesOn (Debug) || PrintMatchSummaryOn (Debug))
    DebugFeatureProtoError(ClassTemplate, ProtoMask, ConfigMask, *tables,
                           NumFeatures, Debug);
#endif

  /* Average Proto Evidences & Find Good Protos */
  for (int proto = 0; proto < ClassTemplate->NumProtos; proto++) {
    /* Compute Average for Actual Proto */
    int Temp = 0;
    for (int i = 0; i < ClassTemplate->ProtoLengths[proto]; i++)
      Temp += tables->proto_evidence_[proto][i];

    Temp /= ClassTemplate->ProtoLengths[proto];

    /* Find Good Protos */
    if (Temp >= AdaptProtoThreshold) {
      *ProtoArray = proto;
      ProtoArray++;
      NumGoodProtos++;
    }
  }

  if (MatchDebuggingOn (Debug))
    cprintf ("Match Complete --------------------------------------------\n");
  delete tables;

  return NumGoodProtos;
}


/*---------------------------------------------------------------------------*/
int IntegerMatcher::FindBadFeatures(
    INT_CLASS ClassTemplate,
    BIT_VECTOR ProtoMask,
    BIT_VECTOR ConfigMask,
    uinT16 BlobLength,
    inT16 NumFeatures,
    INT_FEATURE_ARRAY Features,
    FEATURE_ID *FeatureArray,
    int AdaptFeatureThreshold,
    int Debug) {
/*
 **  Parameters:
 **      ClassTemplate             Prototypes & tables for a class
 **      ProtoMask                 AND Mask for proto word
 **      ConfigMask                AND Mask for config word
 **      BlobLength                Length of unormalized blob
 **      NumFeatures               Number of features in blob
 **      Features                  Array of features
 **      FeatureArray              Array of bad features
 **      AdaptFeatureThreshold     Threshold for bad features
 **      Debug                     Debugger flag: 1=debugger on
 **  Operation:
 **      FindBadFeatures finds all features with maximum feature-evidence <
 **      AdaptFeatureThresh. The list is ordered by increasing feature number.
 **  Return:
 **      Number of bad features in FeatureArray.
 **  History: Tue Mar 12 17:09:26 MST 1991, RWM, Created
 */
  ScratchEvidence *tables = new ScratchEvidence();
  int NumBadFeatures = 0;

  /* DEBUG opening heading */
  if (MatchDebuggingOn(Debug))
    cprintf("Find Bad Features -------------------------------------------\n");

  tables->Clear(ClassTemplate);

  for (int Feature = 0; Feature < NumFeatures; Feature++) {
    UpdateTablesForFeature(
        ClassTemplate, ProtoMask, ConfigMask, Feature, &Features[Feature],
        tables, Debug);

    /* Find Best Evidence for Current Feature */
    int best = 0;
    for (int i = 0; i < ClassTemplate->NumConfigs; i++)
      if (tables->feature_evidence_[i] > best)
        best = tables->feature_evidence_[i];

    /* Find Bad Features */
    if (best < AdaptFeatureThreshold) {
      *FeatureArray = Feature;
      FeatureArray++;
      NumBadFeatures++;
    }
  }

#ifndef GRAPHICS_DISABLED
  if (PrintProtoMatchesOn(Debug) || PrintMatchSummaryOn(Debug))
    DebugFeatureProtoError(ClassTemplate, ProtoMask, ConfigMask, *tables,
                           NumFeatures, Debug);
#endif

  if (MatchDebuggingOn(Debug))
    cprintf("Match Complete --------------------------------------------\n");

  delete tables;
  return NumBadFeatures;
}


/*---------------------------------------------------------------------------*/
void IntegerMatcher::Init(tesseract::IntParam *classify_debug_level,
                          int classify_integer_matcher_multiplier) {
  classify_debug_level_ = classify_debug_level;

  /* Set default mode of operation of IntegerMatcher */
  SetCharNormMatch(classify_integer_matcher_multiplier);

  /* Initialize table for evidence to similarity lookup */
  for (int i = 0; i < SE_TABLE_SIZE; i++) {
    uinT32 IntSimilarity = i << (27 - SE_TABLE_BITS);
    double Similarity = ((double) IntSimilarity) / 65536.0 / 65536.0;
    double evidence = Similarity / kSimilarityCenter;
    evidence = 255.0 / (evidence * evidence + 1.0);

    if (kSEExponentialMultiplier > 0.0) {
      double scale = 1.0 - exp(-kSEExponentialMultiplier) *
        exp(kSEExponentialMultiplier * ((double) i / SE_TABLE_SIZE));
      evidence *= ClipToRange(scale, 0.0, 1.0);
    }

    similarity_evidence_table_[i] = (uinT8) (evidence + 0.5);
  }

  /* Initialize evidence computation variables */
  evidence_table_mask_ =
    ((1 << kEvidenceTableBits) - 1) << (9 - kEvidenceTableBits);
  mult_trunc_shift_bits_ = (14 - kIntEvidenceTruncBits);
  table_trunc_shift_bits_ = (27 - SE_TABLE_BITS - (mult_trunc_shift_bits_ << 1));
  evidence_mult_mask_ = ((1 << kIntEvidenceTruncBits) - 1);
}

/*--------------------------------------------------------------------------*/
void IntegerMatcher::SetBaseLineMatch() {
  local_matcher_multiplier_ = 0;
}


/*--------------------------------------------------------------------------*/
void IntegerMatcher::SetCharNormMatch(int integer_matcher_multiplier) {
  local_matcher_multiplier_ = integer_matcher_multiplier;
}


/**----------------------------------------------------------------------------
              Private Code
----------------------------------------------------------------------------**/
void ScratchEvidence::Clear(const INT_CLASS class_template) {
  memset(sum_feature_evidence_, 0,
         class_template->NumConfigs * sizeof(sum_feature_evidence_[0]));
  memset(proto_evidence_, 0,
         class_template->NumProtos * sizeof(proto_evidence_[0]));
}

void ScratchEvidence::ClearFeatureEvidence(const INT_CLASS class_template) {
  memset(feature_evidence_, 0,
         class_template->NumConfigs * sizeof(feature_evidence_[0]));
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
  cprintf("F=%3d, C=", FeatureNum);
  for (int ConfigNum = 0; ConfigNum < ConfigCount; ConfigNum++) {
    cprintf("%4d", FeatureEvidence[ConfigNum]);
  }
  cprintf("\n");
}



/*---------------------------------------------------------------------------*/
int IntegerMatcher::UpdateTablesForFeature(
    INT_CLASS ClassTemplate,
    BIT_VECTOR ProtoMask,
    BIT_VECTOR ConfigMask,
    int FeatureNum,
    INT_FEATURE Feature,
    ScratchEvidence *tables,
    int Debug) {
/*
 **  Parameters:
 **      ClassTemplate         Prototypes & tables for a class
 **      FeatureNum            Current feature number (for DEBUG only)
 **      Feature               Pointer to a feature struct
 **      tables                Evidence tables
 **      Debug                 Debugger flag: 1=debugger on
 **  Operation:
 **       For the given feature: prune protos, compute evidence,
 **       update Feature Evidence, Proto Evidence, and Sum of Feature
 **       Evidence tables.
 **  Return:
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

  tables->ClearFeatureEvidence(ClassTemplate);

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
            (((inT8) (Feature->Theta - Proto->Angle)) * kIntThetaFudge) << 1;

          if (A3 < 0)
            A3 = ~A3;
          if (M3 < 0)
            M3 = ~M3;
          A3 >>= mult_trunc_shift_bits_;
          M3 >>= mult_trunc_shift_bits_;
          if (A3 > evidence_mult_mask_)
            A3 = evidence_mult_mask_;
          if (M3 > evidence_mult_mask_)
            M3 = evidence_mult_mask_;

          A4 = (A3 * A3) + (M3 * M3);
          A4 >>= table_trunc_shift_bits_;
          if (A4 > evidence_table_mask_)
            Evidence = 0;
          else
            Evidence = similarity_evidence_table_[A4];

          if (PrintFeatureMatchesOn (Debug))
            IMDebugConfiguration (FeatureNum,
              ActualProtoNum + proto_offset,
              Evidence, ConfigMask, ConfigWord);

          ConfigWord &= *ConfigMask;

          UINT8Pointer = tables->feature_evidence_ - 8;
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

          UINT8Pointer =
            &(tables->proto_evidence_[ActualProtoNum + proto_offset][0]);
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

  if (PrintFeatureMatchesOn(Debug)) {
    IMDebugConfigurationSum(FeatureNum, tables->feature_evidence_,
                            ClassTemplate->NumConfigs);
  }

  IntPointer = tables->sum_feature_evidence_;
  UINT8Pointer = tables->feature_evidence_;
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
void IntegerMatcher::DebugFeatureProtoError(
    INT_CLASS ClassTemplate,
    BIT_VECTOR ProtoMask,
    BIT_VECTOR ConfigMask,
    const ScratchEvidence& tables,
    inT16 NumFeatures,
    int Debug) {
/*
 **      Parameters:
 **      Globals:
 **      Operation:
 **              Print debugging information for Configuations
 **      Return:
 **      Exceptions: none
 **      History: Wed Feb 27 14:12:28 MST 1991, RWM, Created.
 */
  FLOAT32 ProtoConfigs[MAX_NUM_CONFIGS];
  int ConfigNum;
  uinT32 ConfigWord;
  int ProtoSetIndex;
  uinT16 ProtoNum;
  uinT8 ProtoWordNum;
  PROTO_SET ProtoSet;
  uinT16 ActualProtoNum;

  if (PrintMatchSummaryOn(Debug)) {
    cprintf("Configuration Mask:\n");
    for (ConfigNum = 0; ConfigNum < ClassTemplate->NumConfigs; ConfigNum++)
      cprintf("%1d", (((*ConfigMask) >> ConfigNum) & 1));
    cprintf("\n");

    cprintf("Feature Error for Configurations:\n");
    for (ConfigNum = 0; ConfigNum < ClassTemplate->NumConfigs; ConfigNum++) {
      cprintf(
          " %5.1f",
          100.0 * (1.0 -
          (FLOAT32) tables.sum_feature_evidence_[ConfigNum]
          / NumFeatures / 256.0));
    }
    cprintf("\n\n\n");
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
          && (ActualProtoNum < ClassTemplate->NumProtos));
          ProtoNum++, ActualProtoNum++)
        cprintf ("%1d", (((*ProtoMask) >> ProtoNum) & 1));
        cprintf ("\n");
      }
    }
    cprintf ("\n");
  }

  for (int i = 0; i < ClassTemplate->NumConfigs; i++)
    ProtoConfigs[i] = 0;

  if (PrintProtoMatchesOn (Debug)) {
    cprintf ("Proto Evidence:\n");
    for (ProtoSetIndex = 0; ProtoSetIndex < ClassTemplate->NumProtoSets;
    ProtoSetIndex++) {
      ProtoSet = ClassTemplate->ProtoSets[ProtoSetIndex];
      ActualProtoNum = (ProtoSetIndex * PROTOS_PER_PROTO_SET);
      for (ProtoNum = 0;
           ((ProtoNum < PROTOS_PER_PROTO_SET) &&
            (ActualProtoNum < ClassTemplate->NumProtos));
           ProtoNum++, ActualProtoNum++) {
        cprintf ("P %3d =", ActualProtoNum);
        int temp = 0;
        for (int j = 0; j < ClassTemplate->ProtoLengths[ActualProtoNum]; j++) {
          uinT8 data = tables.proto_evidence_[ActualProtoNum][j];
          cprintf(" %d", data);
          temp += data;
        }

        cprintf(" = %6.4f%%\n",
                temp / 256.0 / ClassTemplate->ProtoLengths[ActualProtoNum]);

        ConfigWord = ProtoSet->Protos[ProtoNum].Configs[0];
        ConfigNum = 0;
        while (ConfigWord) {
          cprintf ("%5d", ConfigWord & 1 ? temp : 0);
          if (ConfigWord & 1)
            ProtoConfigs[ConfigNum] += temp;
          ConfigNum++;
          ConfigWord >>= 1;
        }
        cprintf("\n");
      }
    }
  }

  if (PrintMatchSummaryOn (Debug)) {
    cprintf ("Proto Error for Configurations:\n");
    for (ConfigNum = 0; ConfigNum < ClassTemplate->NumConfigs; ConfigNum++)
      cprintf (" %5.1f",
        100.0 * (1.0 -
        ProtoConfigs[ConfigNum] /
        ClassTemplate->ConfigLengths[ConfigNum] / 256.0));
    cprintf ("\n\n");
  }

  if (PrintProtoMatchesOn (Debug)) {
    cprintf ("Proto Sum for Configurations:\n");
    for (ConfigNum = 0; ConfigNum < ClassTemplate->NumConfigs; ConfigNum++)
      cprintf (" %4.1f", ProtoConfigs[ConfigNum] / 256.0);
    cprintf ("\n\n");

    cprintf ("Proto Length for Configurations:\n");
    for (ConfigNum = 0; ConfigNum < ClassTemplate->NumConfigs; ConfigNum++)
      cprintf (" %4.1f",
        (float) ClassTemplate->ConfigLengths[ConfigNum]);
    cprintf ("\n\n");
  }

}


/*---------------------------------------------------------------------------*/
void IntegerMatcher::DisplayProtoDebugInfo(
    INT_CLASS ClassTemplate,
    BIT_VECTOR ProtoMask,
    BIT_VECTOR ConfigMask,
    const ScratchEvidence& tables,
    bool SeparateDebugWindows) {
  uinT16 ProtoNum;
  uinT16 ActualProtoNum;
  PROTO_SET ProtoSet;
  int ProtoSetIndex;

  InitIntMatchWindowIfReqd();
  if (SeparateDebugWindows) {
    InitFeatureDisplayWindowIfReqd();
    InitProtoDisplayWindowIfReqd();
  }


  for (ProtoSetIndex = 0; ProtoSetIndex < ClassTemplate->NumProtoSets;
       ProtoSetIndex++) {
    ProtoSet = ClassTemplate->ProtoSets[ProtoSetIndex];
    ActualProtoNum = ProtoSetIndex * PROTOS_PER_PROTO_SET;
    for (ProtoNum = 0;
         ((ProtoNum < PROTOS_PER_PROTO_SET) &&
          (ActualProtoNum < ClassTemplate->NumProtos));
         ProtoNum++, ActualProtoNum++) {
      /* Compute Average for Actual Proto */
      int temp = 0;
      for (int i = 0; i < ClassTemplate->ProtoLengths[ActualProtoNum]; i++)
        temp += tables.proto_evidence_[ActualProtoNum][i];

      temp /= ClassTemplate->ProtoLengths[ActualProtoNum];

      if ((ProtoSet->Protos[ProtoNum]).Configs[0] & (*ConfigMask)) {
        DisplayIntProto(ClassTemplate, ActualProtoNum, temp / 255.0);
      }
    }
  }
}


/*---------------------------------------------------------------------------*/
void IntegerMatcher::DisplayFeatureDebugInfo(
    INT_CLASS ClassTemplate,
    BIT_VECTOR ProtoMask,
    BIT_VECTOR ConfigMask,
    inT16 NumFeatures,
    INT_FEATURE_ARRAY Features,
    int AdaptFeatureThreshold,
    int Debug,
    bool SeparateDebugWindows) {
  ScratchEvidence *tables = new ScratchEvidence();

  tables->Clear(ClassTemplate);

  InitIntMatchWindowIfReqd();
  if (SeparateDebugWindows) {
    InitFeatureDisplayWindowIfReqd();
    InitProtoDisplayWindowIfReqd();
  }

  for (int Feature = 0; Feature < NumFeatures; Feature++) {
    UpdateTablesForFeature(
        ClassTemplate, ProtoMask, ConfigMask, Feature, &Features[Feature],
        tables, 0);

    /* Find Best Evidence for Current Feature */
    int best = 0;
    for (int i = 0; i < ClassTemplate->NumConfigs; i++)
      if (tables->feature_evidence_[i] > best)
        best = tables->feature_evidence_[i];

    /* Update display for current feature */
    if (ClipMatchEvidenceOn(Debug)) {
      if (best < AdaptFeatureThreshold)
        DisplayIntFeature(&Features[Feature], 0.0);
      else
        DisplayIntFeature(&Features[Feature], 1.0);
    } else {
      DisplayIntFeature(&Features[Feature], best / 255.0);
    }
  }

  delete tables;
}
#endif

/*---------------------------------------------------------------------------*/
// Add sum of Proto Evidences into Sum Of Feature Evidence Array
void ScratchEvidence::UpdateSumOfProtoEvidences(
    INT_CLASS ClassTemplate, BIT_VECTOR ConfigMask, inT16 NumFeatures) {

  int *IntPointer;
  uinT32 ConfigWord;
  int ProtoSetIndex;
  uinT16 ProtoNum;
  PROTO_SET ProtoSet;
  int NumProtos;
  uinT16 ActualProtoNum;

  NumProtos = ClassTemplate->NumProtos;

  for (ProtoSetIndex = 0; ProtoSetIndex < ClassTemplate->NumProtoSets;
       ProtoSetIndex++) {
    ProtoSet = ClassTemplate->ProtoSets[ProtoSetIndex];
    ActualProtoNum = (ProtoSetIndex * PROTOS_PER_PROTO_SET);
    for (ProtoNum = 0;
         ((ProtoNum < PROTOS_PER_PROTO_SET) && (ActualProtoNum < NumProtos));
         ProtoNum++, ActualProtoNum++) {
      int temp = 0;
      for (int i = 0; i < ClassTemplate->ProtoLengths[ActualProtoNum]; i++)
        temp += proto_evidence_[ActualProtoNum] [i];

      ConfigWord = ProtoSet->Protos[ProtoNum].Configs[0];
      ConfigWord &= *ConfigMask;
      IntPointer = sum_feature_evidence_;
      while (ConfigWord) {
        if (ConfigWord & 1)
          *IntPointer += temp;
        IntPointer++;
        ConfigWord >>= 1;
      }
    }
  }
}



/*---------------------------------------------------------------------------*/
// Normalize Sum of Proto and Feature Evidence by dividing by the sum of
// the Feature Lengths and the Proto Lengths for each configuration.
void ScratchEvidence::NormalizeSums(
    INT_CLASS ClassTemplate, inT16 NumFeatures, inT32 used_features) {

  for (int i = 0; i < ClassTemplate->NumConfigs; i++) {
    sum_feature_evidence_[i] = (sum_feature_evidence_[i] << 8) /
        (NumFeatures + ClassTemplate->ConfigLengths[i]);
  }
}


/*---------------------------------------------------------------------------*/
int IntegerMatcher::FindBestMatch(
    INT_CLASS ClassTemplate,
    const ScratchEvidence &tables,
    uinT16 BlobLength,
    uinT8 NormalizationFactor,
    INT_RESULT Result) {
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
  int BestMatch = 0;
  int Best2Match = 0;
  assert(ClassTemplate->NumConfigs > 0);
  Result->Config = 0;
  Result->Config2 = 0;

  /* Find best match */
  for (int ConfigNum = 0; ConfigNum < ClassTemplate->NumConfigs; ConfigNum++) {
    int rating = tables.sum_feature_evidence_[ConfigNum];
    if (*classify_debug_level_ > 1)
      cprintf("Config %d, rating=%d\n", ConfigNum, rating);
    if (rating > BestMatch) {
      if (BestMatch > 0) {
        Result->Config2 = Result->Config;
        Best2Match = BestMatch;
      } else {
        Result->Config2 = ConfigNum;
      }
      Result->Config = ConfigNum;
      BestMatch = rating;
    } else if (rating > Best2Match) {
      Result->Config2 = ConfigNum;
      Best2Match = rating;
    }
  }

  /* Compute Certainty Rating */
  Result->Rating = ((65536.0 - BestMatch) / 65536.0 * BlobLength +
    local_matcher_multiplier_ * NormalizationFactor / 256.0) /
    (BlobLength + local_matcher_multiplier_);

  return BestMatch;
}

/*---------------------------------------------------------------------------*/
#ifndef GRAPHICS_DISABLED
// Print debug information about the best match for the current class.
void IntegerMatcher::DebugBestMatch(
    int BestMatch, INT_RESULT Result, uinT16 BlobLength,
    uinT8 NormalizationFactor) {
  cprintf("Rating          = %5.1f%%     Best Config   = %3d\n",
          100.0 * ((*Result).Rating), (int) ((*Result).Config));
  cprintf
    ("Matcher Error   = %5.1f%%     Blob Length   = %3d     Weight = %4.1f%%\n",
    100.0 * (65536.0 - BestMatch) / 65536.0, (int) BlobLength,
    100.0 * BlobLength / (BlobLength + local_matcher_multiplier_));
  cprintf
    ("Char Norm Error = %5.1f%%     Norm Strength = %3d     Weight = %4.1f%%\n",
    100.0 * NormalizationFactor / 256.0,
    local_matcher_multiplier_,
    100.0 * local_matcher_multiplier_ /
        (BlobLength + local_matcher_multiplier_));
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
