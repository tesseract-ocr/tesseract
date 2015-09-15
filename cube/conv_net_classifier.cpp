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

#include "char_set.h"
#include "classifier_base.h"
#include "const.h"
#include "conv_net_classifier.h"
#include "cube_utils.h"
#include "feature_base.h"
#include "feature_bmp.h"
#include "tess_lang_model.h"

namespace tesseract {

ConvNetCharClassifier::ConvNetCharClassifier(CharSet *char_set,
                                             TuningParams *params,
                                             FeatureBase *feat_extract)
    : CharClassifier(char_set, params, feat_extract) {
  char_net_ = NULL;
  net_input_ = NULL;
  net_output_ = NULL;
}

ConvNetCharClassifier::~ConvNetCharClassifier() {
  if (char_net_ != NULL) {
    delete char_net_;
    char_net_ = NULL;
  }

  if (net_input_ != NULL) {
    delete []net_input_;
    net_input_ = NULL;
  }

  if (net_output_ != NULL) {
    delete []net_output_;
    net_output_ = NULL;
  }
}

/**
 * The main training function. Given a sample and a class ID the classifier
 * updates its parameters according to its learning algorithm. This function
 * is currently not implemented. TODO(ahmadab): implement end-2-end training
 */
bool ConvNetCharClassifier::Train(CharSamp *char_samp, int ClassID) {
  return false;
}

/**
 * A secondary function needed for training. Allows the trainer to set the
 * value of any train-time parameter. This function is currently not
 * implemented. TODO(ahmadab): implement end-2-end training
 */
bool ConvNetCharClassifier::SetLearnParam(char *var_name, float val) {
  // TODO(ahmadab): implementation of parameter initializing.
  return false;
}

/**
 * Folds the output of the NeuralNet using the loaded folding sets
 */
void ConvNetCharClassifier::Fold() {
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
    if (fold_set_len_[fold_set] == 0)
      continue;
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

/**
 * Compute the features of specified charsamp and feedforward the
 * specified nets
 */
bool ConvNetCharClassifier::RunNets(CharSamp *char_samp) {
  if (char_net_ == NULL) {
    fprintf(stderr, "Cube ERROR (ConvNetCharClassifier::RunNets): "
            "NeuralNet is NULL\n");
    return false;
  }
  int feat_cnt = char_net_->in_cnt();
  int class_cnt = char_set_->ClassCount();

  // allocate i/p and o/p buffers if needed
  if (net_input_ == NULL) {
    net_input_ = new float[feat_cnt];
    if (net_input_ == NULL) {
      fprintf(stderr, "Cube ERROR (ConvNetCharClassifier::RunNets): "
            "unable to allocate memory for input nodes\n");
      return false;
    }

    net_output_ = new float[class_cnt];
    if (net_output_ == NULL) {
      fprintf(stderr, "Cube ERROR (ConvNetCharClassifier::RunNets): "
            "unable to allocate memory for output nodes\n");
      return false;
    }
  }

  // compute input features
  if (feat_extract_->ComputeFeatures(char_samp, net_input_) == false) {
    fprintf(stderr, "Cube ERROR (ConvNetCharClassifier::RunNets): "
            "unable to compute features\n");
    return false;
  }

  if (char_net_ != NULL) {
    if (char_net_->FeedForward(net_input_, net_output_) == false) {
      fprintf(stderr, "Cube ERROR (ConvNetCharClassifier::RunNets): "
              "unable to run feed-forward\n");
      return false;
    }
  } else {
    return false;
  }
  Fold();
  return true;
}

/**
 * return the cost of being a char
 */
int ConvNetCharClassifier::CharCost(CharSamp *char_samp) {
  if (RunNets(char_samp) == false) {
    return 0;
  }
  return CubeUtils::Prob2Cost(1.0f - net_output_[0]);
}

/**
 * classifies a charsamp and returns an alternate list
 * of chars sorted by char costs
 */
CharAltList *ConvNetCharClassifier::Classify(CharSamp *char_samp) {
  // run the needed nets
  if (RunNets(char_samp) == false) {
    return NULL;
  }

  int class_cnt = char_set_->ClassCount();

  // create an altlist
  CharAltList *alt_list = new CharAltList(char_set_, class_cnt);
  if (alt_list == NULL) {
    fprintf(stderr, "Cube WARNING (ConvNetCharClassifier::Classify): "
            "returning emtpy CharAltList\n");
    return NULL;
  }

  for (int out = 1; out < class_cnt; out++) {
    int cost = CubeUtils::Prob2Cost(net_output_[out]);
    alt_list->Insert(out, cost);
  }

  return alt_list;
}

/**
 * Set an external net (for training purposes)
 */
void ConvNetCharClassifier::SetNet(tesseract::NeuralNet *char_net) {
  if (char_net_ != NULL) {
    delete char_net_;
    char_net_ = NULL;
  }
  char_net_ = char_net;
}

/**
 * This function will return true if the file does not exist.
 * But will fail if the it did not pass the sanity checks
 */
bool ConvNetCharClassifier::LoadFoldingSets(const string &data_file_path,
                                            const string &lang,
                                            LangModel *lang_mod) {
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
  if (fold_sets_ == NULL) {
    return false;
  }
  fold_set_len_ = new int[fold_set_cnt_];
  if (fold_set_len_ == NULL) {
    fold_set_cnt_ = 0;
    return false;
  }

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
    if (fold_sets_[fold_set] == NULL) {
      fprintf(stderr, "Cube ERROR (ConvNetCharClassifier::LoadFoldingSets): "
              "could not allocate folding set\n");
      fold_set_cnt_ = fold_set;
      return false;
    }
    for (int ch = 0; ch < fold_set_len_[fold_set]; ch++) {
      fold_sets_[fold_set][ch] = char_set_->ClassID(str32[ch]);
    }
  }
  return true;
}

/**
 * Init the classifier provided a data-path and a language string
 */
bool ConvNetCharClassifier::Init(const string &data_file_path,
                                 const string &lang,
                                 LangModel *lang_mod) {
  if (init_) {
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

/**
 * Load the classifier's Neural Nets
 * This function will return true if the net file does not exist.
 * But will fail if the net did not pass the sanity checks
 */
bool ConvNetCharClassifier::LoadNets(const string &data_file_path,
                                     const string &lang) {
  string char_net_file;

  // add the lang identifier
  char_net_file = data_file_path + lang;
  char_net_file += ".cube.nn";

  // neural network is optional
  FILE *fp = fopen(char_net_file.c_str(), "rb");
  if (fp == NULL) {
    return true;
  }
  fclose(fp);

  // load main net
  char_net_ = tesseract::NeuralNet::FromFile(char_net_file);
  if (char_net_ == NULL) {
    fprintf(stderr, "Cube ERROR (ConvNetCharClassifier::LoadNets): "
            "could not load %s\n", char_net_file.c_str());
    return false;
  }

  // validate net
  if (char_net_->in_cnt()!= feat_extract_->FeatureCnt()) {
    fprintf(stderr, "Cube ERROR (ConvNetCharClassifier::LoadNets): "
            "could not validate net %s\n", char_net_file.c_str());
    return false;
  }

  // alloc net i/o buffers
  int feat_cnt = char_net_->in_cnt();
  int class_cnt = char_set_->ClassCount();

  if (char_net_->out_cnt() != class_cnt) {
    fprintf(stderr, "Cube ERROR (ConvNetCharClassifier::LoadNets): "
            "output count (%d) and class count (%d) are not equal\n",
            char_net_->out_cnt(), class_cnt);
    return false;
  }

  // allocate i/p and o/p buffers if needed
  if (net_input_ == NULL) {
    net_input_ = new float[feat_cnt];
    if (net_input_ == NULL) {
      return false;
    }

    net_output_ = new float[class_cnt];
    if (net_output_ == NULL) {
      return false;
    }
  }

  return true;
}
}  // tesseract
