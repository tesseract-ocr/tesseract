/**********************************************************************
 * File:        feature_chebyshev.cpp
 * Description: Implementation of the Chebyshev coefficients Feature Class
 * Author:    Ahmad Abdulkader
 * Created:   2008
 *
 * (C) Copyright 2008, Google Inc.
 ** Licensed under the Apache License, Version 2.0 (the "License");
 ** you may not use this file except in compliance with the License.
 ** You may obtain a copy of the License at
 ** http://www.apache.org/licenses/LICENSE-2.0
 ** Unless required by applicable law or agreed to in writing, software
 ** distributed under the License is distributed on an "AS IS" BASIS,
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 ** See the License for the specific language governing permissions and
 ** limitations under the License.
 *
 **********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string>
#include <vector>
#include <algorithm>
#include "feature_base.h"
#include "feature_chebyshev.h"
#include "cube_utils.h"
#include "const.h"
#include "char_samp.h"

namespace tesseract {

FeatureChebyshev::FeatureChebyshev(TuningParams *params)
    : FeatureBase(params) {
}

FeatureChebyshev::~FeatureChebyshev() {
}

// Render a visualization of the features to a CharSamp.
// This is mainly used by visual-debuggers
CharSamp *FeatureChebyshev::ComputeFeatureBitmap(CharSamp *char_samp) {
  return char_samp;
}

// Compute Chebyshev coefficients for the specified vector
void FeatureChebyshev::ChebyshevCoefficients(const vector<float> &input,
                                             int coeff_cnt, float *coeff) {
  // re-sample function
  int input_range = (input.size() - 1);
  vector<float> resamp(coeff_cnt);
  for (int samp_idx = 0; samp_idx < coeff_cnt; samp_idx++) {
    // compute sampling position
    float samp_pos = input_range *
                     (1 + cos(M_PI * (samp_idx + 0.5) / coeff_cnt)) / 2;
    // interpolate
    int samp_start = static_cast<int>(samp_pos);
    int samp_end = static_cast<int>(samp_pos + 0.5);
    float func_delta = input[samp_end] - input[samp_start];
    resamp[samp_idx] = input[samp_start] +
                       ((samp_pos - samp_start) * func_delta);
  }
  // compute the coefficients
  float normalizer = 2.0 / coeff_cnt;
  for (int coeff_idx = 0; coeff_idx < coeff_cnt; coeff_idx++, coeff++) {
    double sum = 0.0;
    for (int samp_idx = 0; samp_idx < coeff_cnt; samp_idx++) {
        sum += resamp[samp_idx] * cos(M_PI * coeff_idx * (samp_idx + 0.5) /
                                      coeff_cnt);
    }
    (*coeff) = (normalizer * sum);
  }
}

// Compute the features of a given CharSamp
bool FeatureChebyshev::ComputeFeatures(CharSamp *char_samp, float *features) {
  return ComputeChebyshevCoefficients(char_samp, features);
}

// Compute the Chebyshev coefficients of a given CharSamp
bool FeatureChebyshev::ComputeChebyshevCoefficients(CharSamp *char_samp,
                                                    float *features) {
  if (char_samp->NormBottom() <= 0) {
    return false;
  }
  unsigned char *raw_data = char_samp->RawData();
  int stride = char_samp->Stride();
  // compute the height of the word
  int word_hgt = (255 * (char_samp->Top() + char_samp->Height()) /
                  char_samp->NormBottom());
  // compute left & right profiles
  vector<float> left_profile(word_hgt, 0.0);
  vector<float> right_profile(word_hgt, 0.0);
  unsigned char *line_data = raw_data;
  for (int y = 0; y < char_samp->Height(); y++, line_data += stride) {
    int min_x = char_samp->Width();
    int max_x = -1;
    for (int x = 0; x < char_samp->Width(); x++) {
      if (line_data[x] == 0) {
        UpdateRange(x, &min_x, &max_x);
      }
    }
    left_profile[char_samp->Top() + y] =
        1.0 * (min_x == char_samp->Width() ? 0 : (min_x + 1)) /
        char_samp->Width();
    right_profile[char_samp->Top() + y] =
        1.0 * (max_x == -1 ? 0 : char_samp->Width() - max_x) /
        char_samp->Width();
  }

  // compute top and bottom profiles
  vector<float> top_profile(char_samp->Width(), 0);
  vector<float> bottom_profile(char_samp->Width(), 0);
  for (int x = 0; x < char_samp->Width(); x++) {
    int min_y = word_hgt;
    int max_y = -1;
    line_data = raw_data;
    for (int y = 0; y < char_samp->Height(); y++, line_data += stride) {
      if (line_data[x] == 0) {
        UpdateRange(y + char_samp->Top(), &min_y, &max_y);
      }
    }
    top_profile[x] = 1.0 * (min_y == word_hgt ? 0 : (min_y + 1)) / word_hgt;
    bottom_profile[x] = 1.0 * (max_y == -1 ? 0 : (word_hgt - max_y)) / word_hgt;
  }

  // compute the chebyshev coefficients of each profile
  ChebyshevCoefficients(left_profile, kChebychevCoefficientCnt, features);
  ChebyshevCoefficients(top_profile, kChebychevCoefficientCnt,
                        features + kChebychevCoefficientCnt);
  ChebyshevCoefficients(right_profile, kChebychevCoefficientCnt,
                        features + (2 * kChebychevCoefficientCnt));
  ChebyshevCoefficients(bottom_profile, kChebychevCoefficientCnt,
                        features + (3 * kChebychevCoefficientCnt));
  return true;
}
}  // namespace tesseract
