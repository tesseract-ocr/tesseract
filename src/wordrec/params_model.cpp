///////////////////////////////////////////////////////////////////////
// File:        params_model.cpp
// Description: Trained language model parameters.
// Author:      David Eger
//
// (C) Copyright 2012, Google Inc.
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

#include "params_model.h"

#include <cctype>
#include <cmath>
#include <cstdio>

#include "bitvector.h"
#include "tprintf.h"

namespace tesseract {

// Scale factor to apply to params model scores.
static const float kScoreScaleFactor = 100.0f;
// Minimum cost result to return.
static const float kMinFinalCost = 0.001f;
// Maximum cost result to return.
static const float kMaxFinalCost = 100.0f;

void ParamsModel::Print() {
  for (int p = 0; p < PTRAIN_NUM_PASSES; ++p) {
    tprintf("ParamsModel for pass %d lang %s\n", p, lang_.c_str());
    for (int i = 0; i < weights_vec_[p].size(); ++i) {
      tprintf("%s = %g\n", kParamsTrainingFeatureTypeName[i],
              weights_vec_[p][i]);
    }
  }
}

void ParamsModel::Copy(const ParamsModel &other_model) {
  for (int p = 0; p < PTRAIN_NUM_PASSES; ++p) {
    weights_vec_[p] = other_model.weights_for_pass(
        static_cast<PassEnum>(p));
  }
}

// Given a (modifiable) line, parse out a key / value pair.
// Return true on success.
bool ParamsModel::ParseLine(char *line, char** key, float *val) {
  if (line[0] == '#')
    return false;
  int end_of_key = 0;
  while (line[end_of_key] &&
         !(isascii(line[end_of_key]) && isspace(line[end_of_key]))) {
    end_of_key++;
  }
  if (!line[end_of_key]) {
    tprintf("ParamsModel::Incomplete line %s\n", line);
    return false;
  }
  line[end_of_key++] = 0;
  *key = line;
  if (sscanf(line + end_of_key, " %f", val) != 1)
    return false;
  return true;
}

// Applies params model weights to the given features.
// Assumes that features is an array of size PTRAIN_NUM_FEATURE_TYPES.
// The cost is set to a number that can be multiplied by the outline length,
// as with the old ratings scheme. This enables words of different length
// and combinations of words to be compared meaningfully.
float ParamsModel::ComputeCost(const float features[]) const {
  float unnorm_score = 0.0;
  for (int f = 0; f < PTRAIN_NUM_FEATURE_TYPES; ++f) {
    unnorm_score += weights_vec_[pass_][f] * features[f];
  }
  return ClipToRange(-unnorm_score / kScoreScaleFactor,
                     kMinFinalCost, kMaxFinalCost);
}

bool ParamsModel::Equivalent(const ParamsModel &that) const {
  float epsilon = 0.0001;
  for (int p = 0; p < PTRAIN_NUM_PASSES; ++p) {
    if (weights_vec_[p].size() != that.weights_vec_[p].size()) return false;
    for (int i = 0; i < weights_vec_[p].size(); i++) {
      if (weights_vec_[p][i] != that.weights_vec_[p][i] &&
          fabs(weights_vec_[p][i] - that.weights_vec_[p][i]) > epsilon)
        return false;
    }
  }
  return true;
}

bool ParamsModel::LoadFromFp(const char *lang, TFile *fp) {
  const int kMaxLineSize = 100;
  char line[kMaxLineSize];
  BitVector present;
  present.Init(PTRAIN_NUM_FEATURE_TYPES);
  lang_ = lang;
  // Load weights for passes with adaption on.
  GenericVector<float> &weights = weights_vec_[pass_];
  weights.init_to_size(PTRAIN_NUM_FEATURE_TYPES, 0.0);

  while (fp->FGets(line, kMaxLineSize) != nullptr) {
    char *key = nullptr;
    float value;
    if (!ParseLine(line, &key, &value))
      continue;
    int idx = ParamsTrainingFeatureByName(key);
    if (idx < 0) {
      tprintf("ParamsModel::Unknown parameter %s\n", key);
      continue;
    }
    if (!present[idx]) {
      present.SetValue(idx, true);
    }
    weights[idx] = value;
  }
  bool complete = (present.NumSetBits() == PTRAIN_NUM_FEATURE_TYPES);
  if (!complete) {
    for (int i = 0; i < PTRAIN_NUM_FEATURE_TYPES; i++) {
      if (!present[i]) {
        tprintf("Missing field %s.\n", kParamsTrainingFeatureTypeName[i]);
      }
    }
    lang_ = "";
    weights.truncate(0);
  }
  return complete;
}

bool ParamsModel::SaveToFile(const char *full_path) const {
  const GenericVector<float> &weights = weights_vec_[pass_];
  if (weights.size() != PTRAIN_NUM_FEATURE_TYPES) {
    tprintf("Refusing to save ParamsModel that has not been initialized.\n");
    return false;
  }
  FILE *fp = fopen(full_path, "wb");
  if (!fp) {
    tprintf("Could not open %s for writing.\n", full_path);
    return false;
  }
  bool all_good = true;
  for (int i = 0; i < weights.size(); i++) {
    if (fprintf(fp, "%s %f\n", kParamsTrainingFeatureTypeName[i], weights[i])
        < 0) {
      all_good = false;
    }
  }
  fclose(fp);
  return all_good;
}

}  // namespace tesseract
