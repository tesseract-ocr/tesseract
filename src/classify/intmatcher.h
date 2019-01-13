/******************************************************************************
 ** Filename:    intmatcher.h
 ** Purpose:     Interface to high level generic classifier routines.
 ** Author:      Robert Moss
 ** History:     Wed Feb 13 15:24:15 MST 1991, RWM, Created.
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
#ifndef   INTMATCHER_H
#define   INTMATCHER_H

#include "params.h"

// Character fragments could be present in the trained templaes
// but turned on/off on the language-by-language basis or depending
// on particular properties of the corpus (e.g. when we expect the
// images to have low exposure).
extern BOOL_VAR_H(disable_character_fragments, FALSE,
                  "Do not include character fragments in the"
                  " results of the classifier");

extern INT_VAR_H(classify_integer_matcher_multiplier, 10,
                 "Integer Matcher Multiplier  0-255:   ");


/**----------------------------------------------------------------------------
          Include Files and Type Defines
----------------------------------------------------------------------------**/
#include "intproto.h"
#include "cutoffs.h"

namespace tesseract {
struct UnicharRating;
}

struct CP_RESULT_STRUCT {
  CP_RESULT_STRUCT() : Rating(0.0f), Class(0) {}

  float Rating;
  CLASS_ID Class;
};

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

#define  SE_TABLE_BITS    9
#define  SE_TABLE_SIZE  512

struct ScratchEvidence {
  uint8_t feature_evidence_[MAX_NUM_CONFIGS];
  int sum_feature_evidence_[MAX_NUM_CONFIGS];
  uint8_t proto_evidence_[MAX_NUM_PROTOS][MAX_PROTO_INDEX];

  void Clear(const INT_CLASS class_template);
  void ClearFeatureEvidence(const INT_CLASS class_template);
  void NormalizeSums(INT_CLASS ClassTemplate, int16_t NumFeatures,
                     int32_t used_features);
  void UpdateSumOfProtoEvidences(
    INT_CLASS ClassTemplate, BIT_VECTOR ConfigMask, int16_t NumFeatures);
};


class IntegerMatcher {
 public:
  // Integer Matcher Theta Fudge (0-255).
  static const int kIntThetaFudge = 128;
  // Bits in Similarity to Evidence Lookup (8-9).
  static const int kEvidenceTableBits = 9;
  // Integer Evidence Truncation Bits (8-14).
  static const int kIntEvidenceTruncBits = 14;
  // Similarity to Evidence Table Exponential Multiplier.
  static const float kSEExponentialMultiplier;
  // Center of Similarity Curve.
  static const float kSimilarityCenter;

  IntegerMatcher(tesseract::IntParam *classify_debug_level);

  void Match(INT_CLASS ClassTemplate,
             BIT_VECTOR ProtoMask,
             BIT_VECTOR ConfigMask,
             int16_t NumFeatures,
             const INT_FEATURE_STRUCT* Features,
             tesseract::UnicharRating* Result,
             int AdaptFeatureThreshold,
             int Debug,
             bool SeparateDebugWindows);

  // Applies the CN normalization factor to the given rating and returns
  // the modified rating.
  float ApplyCNCorrection(float rating, int blob_length,
                          int normalization_factor, int matcher_multiplier);

  int FindGoodProtos(INT_CLASS ClassTemplate,
                     BIT_VECTOR ProtoMask,
                     BIT_VECTOR ConfigMask,
                     uint16_t BlobLength,
                     int16_t NumFeatures,
                     INT_FEATURE_ARRAY Features,
                     PROTO_ID *ProtoArray,
                     int AdaptProtoThreshold,
                     int Debug);

  int FindBadFeatures(INT_CLASS ClassTemplate,
                      BIT_VECTOR ProtoMask,
                      BIT_VECTOR ConfigMask,
                      uint16_t BlobLength,
                      int16_t NumFeatures,
                      INT_FEATURE_ARRAY Features,
                      FEATURE_ID *FeatureArray,
                      int AdaptFeatureThreshold,
                      int Debug);

 private:
  int UpdateTablesForFeature(
      INT_CLASS ClassTemplate,
      BIT_VECTOR ProtoMask,
      BIT_VECTOR ConfigMask,
      int FeatureNum,
      const INT_FEATURE_STRUCT* Feature,
      ScratchEvidence *evidence,
      int Debug);

  int FindBestMatch(INT_CLASS ClassTemplate,
                    const ScratchEvidence &tables,
                    tesseract::UnicharRating* Result);

#ifndef GRAPHICS_DISABLED
  void DebugFeatureProtoError(
      INT_CLASS ClassTemplate,
      BIT_VECTOR ProtoMask,
      BIT_VECTOR ConfigMask,
      const ScratchEvidence &tables,
      int16_t NumFeatures,
      int Debug);

  void DisplayProtoDebugInfo(
      INT_CLASS ClassTemplate,
      BIT_VECTOR ProtoMask,
      BIT_VECTOR ConfigMask,
      const ScratchEvidence &tables,
      bool SeparateDebugWindows);

  void DisplayFeatureDebugInfo(
      INT_CLASS ClassTemplate,
      BIT_VECTOR ProtoMask,
      BIT_VECTOR ConfigMask,
      int16_t NumFeatures,
      const INT_FEATURE_STRUCT* Features,
      int AdaptFeatureThreshold,
      int Debug,
      bool SeparateDebugWindows);
#endif

 private:
  tesseract::IntParam *classify_debug_level_;
  uint8_t similarity_evidence_table_[SE_TABLE_SIZE];
  uint32_t evidence_table_mask_;
  uint32_t mult_trunc_shift_bits_;
  uint32_t table_trunc_shift_bits_;
  uint32_t evidence_mult_mask_;
};

/**----------------------------------------------------------------------------
          Private Function Prototypes
----------------------------------------------------------------------------**/
void IMDebugConfiguration(INT_FEATURE FeatureNum,
                          uint16_t ActualProtoNum,
                          uint8_t Evidence,
                          BIT_VECTOR ConfigMask,
                          uint32_t ConfigWord);

void IMDebugConfigurationSum(INT_FEATURE FeatureNum,
                             uint8_t *FeatureEvidence,
                             int32_t ConfigCount);

void HeapSort (int n, int ra[], int rb[]);

/**----------------------------------------------------------------------------
        Global Data Definitions and Declarations
----------------------------------------------------------------------------**/
#endif
