/**********************************************************************
 * File:        tesseract_cube_combiner.h
 * Description: Declaration of the Tesseract & Cube results combiner Class
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

// The TesseractCubeCombiner class provides the functionality of combining
// the recognition results of Tesseract and Cube at the word level

#include <algorithm>
#include <string>
#include <vector>
#include <wctype.h>

#include "tesseract_cube_combiner.h"

#include "cube_object.h"
#include "cube_reco_context.h"
#include "cube_utils.h"
#include "neural_net.h"
#include "tesseractclass.h"
#include "word_altlist.h"

namespace tesseract {

TesseractCubeCombiner::TesseractCubeCombiner(CubeRecoContext *cube_cntxt) {
  cube_cntxt_ = cube_cntxt;
  combiner_net_ = NULL;
}

TesseractCubeCombiner::~TesseractCubeCombiner() {
  if (combiner_net_ != NULL) {
    delete combiner_net_;
    combiner_net_ = NULL;
  }
}

bool TesseractCubeCombiner::LoadCombinerNet() {
  ASSERT_HOST(cube_cntxt_);
  // Compute the path of the combiner net
  string data_path;
  cube_cntxt_->GetDataFilePath(&data_path);
  string net_file_name =  data_path + cube_cntxt_->Lang() +
                          ".tesseract_cube.nn";

  // Return false if file does not exist
  FILE *fp = fopen(net_file_name.c_str(), "rb");
  if (fp == NULL)
    return false;
  else
    fclose(fp);

  // Load and validate net
  combiner_net_ = NeuralNet::FromFile(net_file_name);
  if (combiner_net_ == NULL) {
    tprintf("Could not read combiner net file %s", net_file_name.c_str());
    return false;
  } else if (combiner_net_->out_cnt() != 2) {
    tprintf("Invalid combiner net file %s! Output count != 2\n",
            net_file_name.c_str());
    delete combiner_net_;
    combiner_net_ = NULL;
    return false;
  }
  return true;
}

// Normalize a UTF-8 string. Converts the UTF-8 string to UTF32 and optionally
// strips punc and/or normalizes case and then converts back
string TesseractCubeCombiner::NormalizeString(const string &str,
                                              bool remove_punc,
                                              bool norm_case) {
  // convert to UTF32
  string_32 str32;
  CubeUtils::UTF8ToUTF32(str.c_str(), &str32);
  // strip punc and normalize
  string_32 new_str32;
  for (int idx = 0; idx < str32.length(); idx++) {
    // if no punc removal is required or not a punctuation character
    if (!remove_punc || iswpunct(str32[idx]) == 0) {
      char_32 norm_char = str32[idx];
      // normalize case if required
      if (norm_case && iswalpha(norm_char)) {
        norm_char = towlower(norm_char);
      }
      new_str32.push_back(norm_char);
    }
  }
  // convert back to UTF8
  string new_str;
  CubeUtils::UTF32ToUTF8(new_str32.c_str(), &new_str);
  return new_str;
}

// Compares 2 strings optionally ignoring punctuation
int TesseractCubeCombiner::CompareStrings(const string &str1,
                                          const string &str2,
                                          bool ignore_punc,
                                          bool ignore_case) {
  if (!ignore_punc && !ignore_case) {
    return str1.compare(str2);
  }
  string norm_str1 = NormalizeString(str1, ignore_punc, ignore_case);
  string norm_str2 = NormalizeString(str2, ignore_punc, ignore_case);
  return norm_str1.compare(norm_str2);
}

// Check if a string is a valid Tess dict word or not
bool TesseractCubeCombiner::ValidWord(const string &str) {
  return (cube_cntxt_->TesseractObject()->getDict().valid_word(str.c_str())
          > 0);
}

// Public method for computing the combiner features. The agreement
// output parameter will be true if both answers are identical,
// and false otherwise.
bool TesseractCubeCombiner::ComputeCombinerFeatures(const string &tess_str,
                                                    int tess_confidence,
                                                    CubeObject *cube_obj,
                                                    WordAltList *cube_alt_list,
                                                    vector<double> *features,
                                                    bool *agreement) {
  features->clear();
  *agreement = false;
  if (cube_alt_list == NULL || cube_alt_list->AltCount() <= 0)
    return false;

  // Get Cube's best string; return false if empty
  char_32 *cube_best_str32 = cube_alt_list->Alt(0);
  if (cube_best_str32 == NULL || CubeUtils::StrLen(cube_best_str32) < 1)
    return false;
  string cube_best_str;
  int cube_best_cost = cube_alt_list->AltCost(0);
  int cube_best_bigram_cost = 0;
  bool cube_best_bigram_cost_valid = true;
  if (cube_cntxt_->Bigrams())
    cube_best_bigram_cost = cube_cntxt_->Bigrams()->
        Cost(cube_best_str32, cube_cntxt_->CharacterSet());
  else
    cube_best_bigram_cost_valid = false;
  CubeUtils::UTF32ToUTF8(cube_best_str32, &cube_best_str);

  // Get Tesseract's UTF32 string
  string_32 tess_str32;
  CubeUtils::UTF8ToUTF32(tess_str.c_str(), &tess_str32);

  // Compute agreement flag
  *agreement = (tess_str.compare(cube_best_str) == 0);

  // Get Cube's second best string; if empty, return false
  char_32 *cube_next_best_str32;
  string cube_next_best_str;
  int cube_next_best_cost = WORST_COST;
  if (cube_alt_list->AltCount() > 1) {
    cube_next_best_str32 = cube_alt_list->Alt(1);
    if (cube_next_best_str32 == NULL ||
        CubeUtils::StrLen(cube_next_best_str32) == 0) {
      return false;
    }
    cube_next_best_cost = cube_alt_list->AltCost(1);
    CubeUtils::UTF32ToUTF8(cube_next_best_str32, &cube_next_best_str);
  }
  // Rank of Tesseract's top result in Cube's alternate list
  int tess_rank = 0;
  for (tess_rank = 0; tess_rank < cube_alt_list->AltCount(); tess_rank++) {
    string alt_str;
    CubeUtils::UTF32ToUTF8(cube_alt_list->Alt(tess_rank), &alt_str);
    if (alt_str == tess_str)
      break;
  }

  // Cube's cost for tesseract's result. Note that this modifies the
  // state of cube_obj, including its alternate list by calling RecognizeWord()
  int tess_cost = cube_obj->WordCost(tess_str.c_str());
  // Cube's bigram cost of Tesseract's string
  int tess_bigram_cost = 0;
  int tess_bigram_cost_valid = true;
  if (cube_cntxt_->Bigrams())
    tess_bigram_cost = cube_cntxt_->Bigrams()->
        Cost(tess_str32.c_str(), cube_cntxt_->CharacterSet());
  else
    tess_bigram_cost_valid = false;

  // Tesseract confidence
  features->push_back(tess_confidence);
  // Cube cost of Tesseract string
  features->push_back(tess_cost);
  // Cube Rank of Tesseract string
  features->push_back(tess_rank);
  // length of Tesseract OCR string
  features->push_back(tess_str.length());
  // Tesseract OCR string in dictionary
  features->push_back(ValidWord(tess_str));
  if (tess_bigram_cost_valid) {
    // bigram cost of Tesseract string
    features->push_back(tess_bigram_cost);
  }
  // Cube tess_cost of Cube best string
  features->push_back(cube_best_cost);
  // Cube tess_cost of Cube next best string
  features->push_back(cube_next_best_cost);
  // length of Cube string
  features->push_back(cube_best_str.length());
  // Cube string in dictionary
  features->push_back(ValidWord(cube_best_str));
  if (cube_best_bigram_cost_valid) {
    // bigram cost of Cube string
    features->push_back(cube_best_bigram_cost);
  }
  // case-insensitive string comparison, including punctuation
  int compare_nocase_punc = CompareStrings(cube_best_str.c_str(),
                                           tess_str.c_str(), false, true);
  features->push_back(compare_nocase_punc == 0);
  // case-sensitive string comparison, ignoring punctuation
  int compare_case_nopunc = CompareStrings(cube_best_str.c_str(),
                                           tess_str.c_str(), true, false);
  features->push_back(compare_case_nopunc == 0);
  // case-insensitive string comparison, ignoring punctuation
  int compare_nocase_nopunc = CompareStrings(cube_best_str.c_str(),
                                             tess_str.c_str(), true, true);
  features->push_back(compare_nocase_nopunc == 0);
  return true;
}

// The CubeObject parameter is used for 2 purposes: 1) to retrieve
// cube's alt list, and 2) to compute cube's word cost for the
// tesseract result. The call to CubeObject::WordCost() modifies
// the object's alternate list, so previous state will be lost.
float TesseractCubeCombiner::CombineResults(WERD_RES *tess_res,
                                            CubeObject *cube_obj) {
  // If no combiner is loaded or the cube object is undefined,
  // tesseract wins with probability 1.0
  if (combiner_net_ == NULL || cube_obj == NULL) {
    tprintf("Cube WARNING (TesseractCubeCombiner::CombineResults): "
            "Cube objects not initialized; defaulting to Tesseract\n");
    return 1.0;
  }

  // Retrieve the alternate list from the CubeObject's current state.
  // If the alt list empty, tesseract wins with probability 1.0
  WordAltList *cube_alt_list = cube_obj->AlternateList();
  if (cube_alt_list == NULL)
    cube_alt_list = cube_obj->RecognizeWord();
  if (cube_alt_list == NULL || cube_alt_list->AltCount() <= 0) {
    tprintf("Cube WARNING (TesseractCubeCombiner::CombineResults): "
            "Cube returned no results; defaulting to Tesseract\n");
    return 1.0;
  }
  return CombineResults(tess_res, cube_obj, cube_alt_list);
}

// The alt_list parameter is expected to have been extracted from the
// CubeObject that recognized the word to be combined. The cube_obj
// parameter passed may be either same instance or a separate instance to
// be used only by the combiner. In both cases, its alternate
// list will be modified by an internal call to RecognizeWord().
float TesseractCubeCombiner::CombineResults(WERD_RES *tess_res,
                                            CubeObject *cube_obj,
                                            WordAltList *cube_alt_list) {
  // If no combiner is loaded or the cube object is undefined, or the
  // alt list is empty, tesseract wins with probability 1.0
  if (combiner_net_ == NULL || cube_obj == NULL ||
      cube_alt_list == NULL || cube_alt_list->AltCount() <= 0) {
    tprintf("Cube WARNING (TesseractCubeCombiner::CombineResults): "
            "Cube result cannot be retrieved; defaulting to Tesseract\n");
    return 1.0;
  }

  // Tesseract result string, tesseract confidence, and cost of
  // tesseract result according to cube
  string tess_str = tess_res->best_choice->unichar_string().string();
  // Map certainty [-20.0, 0.0] to confidence [0, 100]
  int tess_confidence = MIN(100, MAX(1, static_cast<int>(
      100 + (5 * tess_res->best_choice->certainty()))));

  // Compute the combiner features. If feature computation fails or
  // answers are identical, tesseract wins with probability 1.0
  vector<double> features;
  bool agreement;
  bool combiner_success = ComputeCombinerFeatures(tess_str, tess_confidence,
                                                  cube_obj, cube_alt_list,
                                                  &features, &agreement);
  if (!combiner_success || agreement)
    return 1.0;

  // Classify combiner feature vector and return output (probability
  // of tesseract class).
  double net_out[2];
  if (!combiner_net_->FeedForward(&features[0], net_out))
    return 1.0;
  return net_out[1];
}
}
