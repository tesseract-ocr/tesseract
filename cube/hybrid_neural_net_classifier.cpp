/**********************************************************************
 * File:        charclassifier.cpp
 * Description: Implementation of Convolutional-NeuralNet Character Classifier
 * Author:    Ahmad Abdulkader
 * Created:   2007
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

#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <wctype.h>

#include "classifier_base.h"
#include "char_set.h"
#include "const.h"
#include "conv_net_classifier.h"
#include "cube_utils.h"
#include "feature_base.h"
#include "feature_bmp.h"
#include "hybrid_neural_net_classifier.h"
#include "tess_lang_model.h"

namespace tesseract {

HybridNeuralNetCharClassifier::HybridNeuralNetCharClassifier(
    CharSet *char_set,
    TuningParams *params,
    FeatureBase *feat_extract)
    : CharClassifier(char_set, params, feat_extract) {
  net_input_ = NULL;
  net_output_ = NULL;
}

HybridNeuralNetCharClassifier::~HybridNeuralNetCharClassifier() {
  for (int net_idx = 0; net_idx < nets_.size(); net_idx++) {
    if (nets_[net_idx] != NULL) {
      delete nets_[net_idx];
    }
  }
  nets_.clear();

  if (net_input_ != NULL) {
    delete []net_input_;
    net_input_ = NULL;
  }

  if (net_output_ != NULL) {
    delete []net_output_;
    net_output_ = NULL;
  }
}

// The main training function. Given a sample and a class ID the classifier
// updates its parameters according to its learning algorithm. This function
// is currently not implemented. TODO(ahmadab): implement end-2-end training
bool HybridNeuralNetCharClassifier::Train(CharSamp *char_samp, int ClassID) {
  return false;
}

// A secondary function needed for training. Allows the trainer to set the
// value of any train-time parameter. This function is currently not
// implemented. TODO(ahmadab): implement end-2-end training
bool HybridNeuralNetCharClassifier::SetLearnParam(char *var_name, float val) {
  // TODO(ahmadab): implementation of parameter initializing.
  return false;
}

// Folds the output of the NeuralNet using the loaded folding sets
void HybridNeuralNetCharClassifier::Fold() {
  // in case insensitive mode
  if (case_sensitive_ == false) {
    int class_cnt = char_set_->ClassCount();
    // fold case
    for (int class_id = 0; class_id < class_cnt; class_id++) {
      // get class string
      const char_32 *str32 = char_set_->ClassString(class_id);
      // get the upper case form of the string
      string_32 upper_form32 = str32;
      for (int ch = 0; ch < upper_form32.length(); ch++) {
        if (iswalpha(static_cast<int>(upper_form32[ch])) != 0) {
          upper_form32[ch] = towupper(upper_form32[ch]);
        }
      }

      // find out the upperform class-id if any
      int upper_class_id =
          char_set_->ClassID(reinterpret_cast<const char_32 *>(
              upper_form32.c_str()));
      if (upper_class_id != -1 && class_id != upper_class_id) {
        float max_out = MAX(net_output_[class_id], net_output_[upper_class_id]);
        net_output_[class_id] = max_out;
        net_output_[upper_class_id] = max_out;
      }
    }
  }

  // The folding sets specify how groups of classes should be folded
  // Folding involved assigning a min-activation to all the members
  // of the folding set. The min-activation is a fraction of the max-activation
  // of the members of the folding set
  for (int fold_set = 0; fold_set < fold_set_cnt_; fold_set++) {
    float max_prob = net_output_[fold_sets_[fold_set][0]];

    for (int ch = 1; ch < fold_set_len_[fold_set]; ch++) {
      if (net_output_[fold_sets_[fold_set][ch]] > max_prob) {
        max_prob = net_output_[fold_sets_[fold_set][ch]];
      }
    }
    for (int ch = 0; ch < fold_set_len_[fold_set]; ch++) {
      net_output_[fold_sets_[fold_set][ch]] = MAX(max_prob * kFoldingRatio,
          net_output_[fold_sets_[fold_set][ch]]);
    }
  }
}

// compute the features of specified charsamp and
// feedforward the specified nets
bool HybridNeuralNetCharClassifier::RunNets(CharSamp *char_samp) {
  int feat_cnt = feat_extract_->FeatureCnt();
  int class_cnt = char_set_->ClassCount();

  // allocate i/p and o/p buffers if needed
  if (net_input_ == NULL) {
    net_input_ = new float[feat_cnt];
    net_output_ = new float[class_cnt];
  }

  // compute input features
  if (feat_extract_->ComputeFeatures(char_samp, net_input_) == false) {
    return false;
  }

  // go through all the nets
  memset(net_output_, 0, class_cnt * sizeof(*net_output_));
  float *inputs = net_input_;
  for (int net_idx = 0; net_idx < nets_.size(); net_idx++) {
    // run each net
    vector<float> net_out(class_cnt, 0.0);
    if (!nets_[net_idx]->FeedForward(inputs, &net_out[0])) {
      return false;
    }
    // add the output values
    for (int class_idx = 0; class_idx < class_cnt; class_idx++) {
      net_output_[class_idx] += (net_out[class_idx] * net_wgts_[net_idx]);
    }
    // increment inputs pointer
    inputs += nets_[net_idx]->in_cnt();
  }

  Fold();

  return true;
}

// return the cost of being a char
int HybridNeuralNetCharClassifier::CharCost(CharSamp *char_samp) {
  // it is by design that a character cost is equal to zero
  // when no nets are present. This is the case during training.
  if (RunNets(char_samp) == false) {
    return 0;
  }

  return CubeUtils::Prob2Cost(1.0f - net_output_[0]);
}

// classifies a charsamp and returns an alternate list
// of chars sorted by char costs
CharAltList *HybridNeuralNetCharClassifier::Classify(CharSamp *char_samp) {
  // run the needed nets
  if (RunNets(char_samp) == false) {
    return NULL;
  }

  int class_cnt = char_set_->ClassCount();

  // create an altlist
  CharAltList *alt_list = new CharAltList(char_set_, class_cnt);

  for (int out = 1; out < class_cnt; out++) {
    int cost = CubeUtils::Prob2Cost(net_output_[out]);
    alt_list->Insert(out, cost);
  }

  return alt_list;
}

// set an external net (for training purposes)
void HybridNeuralNetCharClassifier::SetNet(tesseract::NeuralNet *char_net) {
}

// Load folding sets
// This function returns true on success or if the file can't be read,
// returns false if an error is encountered.
bool HybridNeuralNetCharClassifier::LoadFoldingSets(
    const string &data_file_path, const string &lang, LangModel *lang_mod) {
  fold_set_cnt_ = 0;
  string fold_file_name;
  fold_file_name = data_file_path + lang;
  fold_file_name += ".cube.fold";

  // folding sets are optional
  FILE *fp = fopen(fold_file_name.c_str(), "rb");
  if (fp == NULL) {
    return true;
  }
  fclose(fp);

  string fold_sets_str;
  if (!CubeUtils::ReadFileToString(fold_file_name,
                                   &fold_sets_str)) {
    return false;
  }

  // split into lines
  vector<string> str_vec;
  CubeUtils::SplitStringUsing(fold_sets_str, "\r\n", &str_vec);
  fold_set_cnt_ = str_vec.size();
  fold_sets_ = new int *[fold_set_cnt_];
  fold_set_len_ = new int[fold_set_cnt_];

  for (int fold_set = 0; fold_set < fold_set_cnt_; fold_set++) {
    reinterpret_cast<TessLangModel *>(lang_mod)->RemoveInvalidCharacters(
        &str_vec[fold_set]);

    // if all or all but one character are invalid, invalidate this set
    if (str_vec[fold_set].length() <= 1) {
      fprintf(stderr, "Cube WARNING (ConvNetCharClassifier::LoadFoldingSets): "
              "invalidating folding set %d\n", fold_set);
      fold_set_len_[fold_set] = 0;
      fold_sets_[fold_set] = NULL;
      continue;
    }

    string_32 str32;
    CubeUtils::UTF8ToUTF32(str_vec[fold_set].c_str(), &str32);
    fold_set_len_[fold_set] = str32.length();
    fold_sets_[fold_set] = new int[fold_set_len_[fold_set]];
    for (int ch = 0; ch < fold_set_len_[fold_set]; ch++) {
      fold_sets_[fold_set][ch] = char_set_->ClassID(str32[ch]);
    }
  }
  return true;
}

// Init the classifier provided a data-path and a language string
bool HybridNeuralNetCharClassifier::Init(const string &data_file_path,
                                         const string &lang,
                                         LangModel *lang_mod) {
  if (init_ == true) {
    return true;
  }

  // load the nets if any. This function will return true if the net file
  // does not exist. But will fail if the net did not pass the sanity checks
  if (!LoadNets(data_file_path, lang)) {
    return false;
  }

  // load the folding sets if any. This function will return true if the
  // file does not exist. But will fail if the it did not pass the sanity checks
  if (!LoadFoldingSets(data_file_path, lang, lang_mod)) {
    return false;
  }

  init_ = true;
  return true;
}

// Load the classifier's Neural Nets
// This function will return true if the net file does not exist.
// But will fail if the net did not pass the sanity checks
bool HybridNeuralNetCharClassifier::LoadNets(const string &data_file_path,
                                             const string &lang) {
  string hybrid_net_file;
  string junk_net_file;

  // add the lang identifier
  hybrid_net_file = data_file_path + lang;
  hybrid_net_file += ".cube.hybrid";

  // neural network is optional
  FILE *fp = fopen(hybrid_net_file.c_str(), "rb");
  if (fp == NULL) {
    return true;
  }
  fclose(fp);

  string str;
  if (!CubeUtils::ReadFileToString(hybrid_net_file, &str)) {
    return false;
  }

  // split into lines
  vector<string> str_vec;
  CubeUtils::SplitStringUsing(str, "\r\n", &str_vec);
  if (str_vec.empty()) {
    return false;
  }

  // create and add the nets
  nets_.resize(str_vec.size(), NULL);
  net_wgts_.resize(str_vec.size(), 0);
  int total_input_size = 0;
  for (int net_idx = 0; net_idx < str_vec.size(); net_idx++) {
    // parse the string
    vector<string> tokens_vec;
    CubeUtils::SplitStringUsing(str_vec[net_idx], " \t", &tokens_vec);
    // has to be 2 tokens, net name and input size
    if (tokens_vec.size() != 2) {
      return false;
    }
    // load the net
    string net_file_name = data_file_path + tokens_vec[0];
    nets_[net_idx] = tesseract::NeuralNet::FromFile(net_file_name);
    if (nets_[net_idx] == NULL) {
      return false;
    }
    // parse the input size and validate it
    net_wgts_[net_idx] = atof(tokens_vec[1].c_str());
    if (net_wgts_[net_idx] < 0.0) {
      return false;
    }
    total_input_size += nets_[net_idx]->in_cnt();
  }
  // validate total input count
  if (total_input_size != feat_extract_->FeatureCnt()) {
    return false;
  }
  // success
  return true;
}
}  // tesseract
