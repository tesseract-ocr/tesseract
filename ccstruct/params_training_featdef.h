///////////////////////////////////////////////////////////////////////
// File:        params_training_featdef.h
// Description: Feature definitions for params training.
// Author:      Rika Antonova
// Created:     Mon Nov 28 11:26:42 PDT 2011
//
// (C) Copyright 2011, Google Inc.
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
///////////////////////////////////////////////////////////////////////

#ifndef TESSERACT_WORDREC_PARAMS_TRAINING_FEATDEF_H_
#define TESSERACT_WORDREC_PARAMS_TRAINING_FEATDEF_H_

#include "genericvector.h"
#include "strngs.h"

namespace tesseract {

// Maximum number of unichars in the small and medium sized words
static const int kMaxSmallWordUnichars = 3;
static const int kMaxMediumWordUnichars = 6;

// Raw features extracted from a single OCR hypothesis.
// The features are normalized (by outline length or number of unichars as
// appropriate) real-valued quantities with unbounded range and
// unknown distribution.
// Normalization / binarization of these features is done at a later stage.
// Note: when adding new fields to this enum make sure to modify
// kParamsTrainingFeatureTypeName
enum kParamsTrainingFeatureType {
  // Digits
  PTRAIN_DIGITS_SHORT,             // 0
  PTRAIN_DIGITS_MED,               // 1
  PTRAIN_DIGITS_LONG,              // 2
  // Number or pattern (NUMBER_PERM, USER_PATTERN_PERM)
  PTRAIN_NUM_SHORT,                // 3
  PTRAIN_NUM_MED,                  // 4
  PTRAIN_NUM_LONG,                 // 5
  // Document word (DOC_DAWG_PERM)
  PTRAIN_DOC_SHORT,                // 6
  PTRAIN_DOC_MED,                  // 7
  PTRAIN_DOC_LONG,                 // 8
  // Word (SYSTEM_DAWG_PERM, USER_DAWG_PERM, COMPOUND_PERM)
  PTRAIN_DICT_SHORT,               // 9
  PTRAIN_DICT_MED,                 // 10
  PTRAIN_DICT_LONG,                // 11
  // Frequent word (FREQ_DAWG_PERM)
  PTRAIN_FREQ_SHORT,               // 12
  PTRAIN_FREQ_MED,                 // 13
  PTRAIN_FREQ_LONG,                // 14
  PTRAIN_SHAPE_COST_PER_CHAR,      // 15
  PTRAIN_NGRAM_COST_PER_CHAR,      // 16
  PTRAIN_NUM_BAD_PUNC,             // 17
  PTRAIN_NUM_BAD_CASE,             // 18
  PTRAIN_XHEIGHT_CONSISTENCY,      // 19
  PTRAIN_NUM_BAD_CHAR_TYPE,        // 20
  PTRAIN_NUM_BAD_SPACING,          // 21
  PTRAIN_NUM_BAD_FONT,             // 22
  PTRAIN_RATING_PER_CHAR,          // 23

  PTRAIN_NUM_FEATURE_TYPES
};

static const char * const kParamsTrainingFeatureTypeName[] = {
    "PTRAIN_DIGITS_SHORT",             // 0
    "PTRAIN_DIGITS_MED",               // 1
    "PTRAIN_DIGITS_LONG",              // 2
    "PTRAIN_NUM_SHORT",                // 3
    "PTRAIN_NUM_MED",                  // 4
    "PTRAIN_NUM_LONG",                 // 5
    "PTRAIN_DOC_SHORT",                // 6
    "PTRAIN_DOC_MED",                  // 7
    "PTRAIN_DOC_LONG",                 // 8
    "PTRAIN_DICT_SHORT",               // 9
    "PTRAIN_DICT_MED",                 // 10
    "PTRAIN_DICT_LONG",                // 11
    "PTRAIN_FREQ_SHORT",               // 12
    "PTRAIN_FREQ_MED",                 // 13
    "PTRAIN_FREQ_LONG",                // 14
    "PTRAIN_SHAPE_COST_PER_CHAR",      // 15
    "PTRAIN_NGRAM_COST_PER_CHAR",      // 16
    "PTRAIN_NUM_BAD_PUNC",             // 17
    "PTRAIN_NUM_BAD_CASE",             // 18
    "PTRAIN_XHEIGHT_CONSISTENCY",      // 19
    "PTRAIN_NUM_BAD_CHAR_TYPE",        // 20
    "PTRAIN_NUM_BAD_SPACING",          // 21
    "PTRAIN_NUM_BAD_FONT",             // 22
    "PTRAIN_RATING_PER_CHAR",          // 23
};

// Returns the index of the given feature (by name),
// or -1 meaning the feature is unknown.
int ParamsTrainingFeatureByName(const char *name);


// Entry with features extracted from a single OCR hypothesis for a word.
struct ParamsTrainingHypothesis {
  ParamsTrainingHypothesis() : cost(0.0) {
    memset(features, 0, sizeof(float) * PTRAIN_NUM_FEATURE_TYPES);
  }
  ParamsTrainingHypothesis(const ParamsTrainingHypothesis &other) {
    memcpy(features, other.features,
           sizeof(float) * PTRAIN_NUM_FEATURE_TYPES);
    str = other.str;
    cost = other.cost;
  }
  float features[PTRAIN_NUM_FEATURE_TYPES];
  STRING str;  // string corresponding to word hypothesis (for debugging)
  float cost;  // path cost computed by segsearch
};

// A list of hypotheses explored during one run of segmentation search.
typedef GenericVector<ParamsTrainingHypothesis> ParamsTrainingHypothesisList;

// A bundle that accumulates all of the hypothesis lists explored during all
// of the runs of segmentation search on a word (e.g. a list of hypotheses
// explored on PASS1, PASS2, fix xheight pass, etc).
class ParamsTrainingBundle {
 public:
  ParamsTrainingBundle() {}
  // Starts a new hypothesis list.
  // Should be called at the beginning of a new run of the segmentation search.
  void StartHypothesisList() {
    hyp_list_vec.push_back(ParamsTrainingHypothesisList());
  }
  // Adds a new ParamsTrainingHypothesis to the current hypothesis list
  // and returns the reference to the newly added entry.
  ParamsTrainingHypothesis &AddHypothesis(
      const ParamsTrainingHypothesis &other) {
    if (hyp_list_vec.empty()) StartHypothesisList();
    hyp_list_vec.back().push_back(ParamsTrainingHypothesis(other));
    return hyp_list_vec.back().back();
  }

  GenericVector<ParamsTrainingHypothesisList> hyp_list_vec;
};

}  // namespace tesseract

#endif  // TESSERACT_WORDREC_PARAMS_TRAINING_FEATDEF_H_
