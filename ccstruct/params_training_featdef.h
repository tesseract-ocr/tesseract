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

// Raw features extracted from a single OCR hypothesis.
// The features are non-normalized real-valued quantities with
// unbounded range and unknown distribution.
// Normalization / binarization of these features is done at a later stage.
// Note: when adding new fields to this enum make sure to modify
// kParamsTrainingRawFeatureTypeName enum accordingly.
enum ParamsTrainingRawFeatureType {
  // What dictionary (if any) was this hypothesis found in.
  // See PermuterType enum in ccstruct/ratngs.h for interpretation.
  PTRAIN_RAW_FEATURE_DICT_MATCH_TYPE,     // 0
  // Boolean indicator of whether this hypothesis is ambiguous to a known
  // dictionary word (or a valid number pattern).
  PTRAIN_RAW_FEATURE_UNAMBIG_DICT_MATCH,  // 1
  // Shape cost of the segmentation path for this hypothesis.
  PTRAIN_RAW_FEATURE_SHAPE_COST,          // 2
  // Character ngram probability of the string of unichars of this hypothesis.
  PTRAIN_RAW_FEATURE_NGRAM_PROB,          // 3
  // Number of bad/inconsistent spots in this hypothesis.
  PTRAIN_RAW_FEATURE_NUM_BAD_PUNC,        // 4
  PTRAIN_RAW_FEATURE_NUM_BAD_CASE,        // 5
  PTRAIN_RAW_FEATURE_NUM_BAD_CHAR_TYPE,   // 6
  PTRAIN_RAW_FEATURE_NUM_BAD_SPACING,     // 7
  PTRAIN_RAW_FEATURE_NUM_BAD_SCRIPT,      // 8
  PTRAIN_RAW_FEATURE_NUM_BAD_FONT,        // 9
  // Classifier-related features.
  PTRAIN_RAW_FEATURE_WORST_CERT,          // 10
  PTRAIN_RAW_FEATURE_RATING,              // 11
  // Number of classifier results that came from adapted templates.
  PTRAIN_RAW_FEATURE_ADAPTED,   // 12
  // Features potentially useful for normalization.
  PTRAIN_RAW_FEATURE_NUM_UNICHARS,        // 13
  PTRAIN_RAW_FEATURE_OUTLINE_LEN,         // 14

  PTRAIN_NUM_RAW_FEATURE_TYPES
};

static const char * const kParamsTrainingRawFeatureTypeName[] = {
    "DICT_MATCH_TYPE",     // 0
    "UNAMBIG_DICT_MATCH",  // 1
    "SHAPE_COST",          // 2
    "NGRAM_PROB",          // 3
    "NUM_BAD_PUNC",        // 4
    "NUM_BAD_CASE",        // 5
    "NUM_BAD_CHAR_TYPE",   // 6
    "NUM_BAD_SPACING",     // 7
    "NUM_BAD_SCRIPT",      // 8
    "NUM_BAD_FONT",        // 9
    "WORST_CERT",          // 10
    "RATING",              // 11
    "ADAPTED",             // 12
    "NUM_UNICHARS",        // 13
    "OUTLINE_LEN",         // 14
};

// Entry with features extracted from a single OCR hypothesis for a word.
struct ParamsTrainingHypothesis {
  ParamsTrainingHypothesis() {
    for (int i = 0; i < PTRAIN_NUM_RAW_FEATURE_TYPES; ++i) features[i] = 0.0;
  }
  float features[PTRAIN_NUM_RAW_FEATURE_TYPES];
  STRING str;  // string corresponding to word hypothesis (for debugging)
};

// A list of hypotheses explored during one run of segmentation search.
typedef GenericVector<ParamsTrainingHypothesis> ParamsTrainingHypothesisList;

// A bundle that accumulates all of the hypothesis lists explored during all
// of the runs of segmentation search on a word (e.g. a list of hypotheses
// explored on PASS1, PASS2, fix xheight pass, etc).
class ParamsTrainingBundle {
 public:
  ParamsTrainingBundle() {};
  // Starts a new hypothesis list.
  // Should be called at the beginning of a new run of the segmentation search.
  void StartHypothesisList() {
    hyp_list_vec.push_back(ParamsTrainingHypothesisList());
  }
  // Adds a new ParamsTrainingHypothesis to the current hypothesis list
  // and returns the reference to the newly added entry.
  ParamsTrainingHypothesis &AddHypothesis() {
    if (hyp_list_vec.empty()) StartHypothesisList();
    hyp_list_vec.back().push_back(ParamsTrainingHypothesis());
    return hyp_list_vec.back().back();
  }

  GenericVector<ParamsTrainingHypothesisList> hyp_list_vec;
};

}  // namespace tesseract

#endif  // TESSERACT_WORDREC_PARAMS_TRAINING_FEATDEF_H_
