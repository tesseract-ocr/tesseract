/**********************************************************************
 * File:        conv_net_classifier.h
 * Description: Declaration of Convolutional-NeuralNet Character Classifier
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

#ifndef HYBRID_NEURAL_NET_CLASSIFIER_H
#define HYBRID_NEURAL_NET_CLASSIFIER_H

#include <string>
#include <vector>

#include "char_samp.h"
#include "char_altlist.h"
#include "char_set.h"
#include "classifier_base.h"
#include "feature_base.h"
#include "lang_model.h"
#include "neural_net.h"
#include "tuning_params.h"

namespace tesseract {

// Folding Ratio is the ratio of the max-activation of members of a folding
// set that is used to compute the min-activation of the rest of the set
// static const float kFoldingRatio = 0.75;  // see conv_net_classifier.h

class HybridNeuralNetCharClassifier : public CharClassifier {
 public:
  HybridNeuralNetCharClassifier(CharSet *char_set, TuningParams *params,
      FeatureBase *feat_extract);
  virtual ~HybridNeuralNetCharClassifier();
  // The main training function. Given a sample and a class ID the classifier
  // updates its parameters according to its learning algorithm. This function
  // is currently not implemented. TODO(ahmadab): implement end-2-end training
  virtual bool Train(CharSamp *char_samp, int ClassID);
  // A secondary function needed for training. Allows the trainer to set the
  // value of any train-time parameter. This function is currently not
  // implemented. TODO(ahmadab): implement end-2-end training
  virtual bool SetLearnParam(char *var_name, float val);
  // Externally sets the Neural Net used by the classifier. Used for training
  void SetNet(tesseract::NeuralNet *net);

  // Classifies an input charsamp and return a CharAltList object containing
  // the possible candidates and corresponding scores
  virtual CharAltList *Classify(CharSamp *char_samp);
  // Computes the cost of a specific charsamp being a character (versus a
  // non-character: part-of-a-character OR more-than-one-character)
  virtual int CharCost(CharSamp *char_samp);

 private:
  // Neural Net object used for classification
  vector<tesseract::NeuralNet *> nets_;
  vector<float> net_wgts_;

  // data buffers used to hold Neural Net inputs and outputs
  float *net_input_;
  float *net_output_;

  // Init the classifier provided a data-path and a language string
  virtual bool Init(const string &data_file_path, const string &lang,
                    LangModel *lang_mod);
  // Loads the NeuralNets needed for the classifier
  bool LoadNets(const string &data_file_path, const string &lang);
  // Load folding sets
  // This function returns true on success or if the file can't be read,
  // returns false if an error is encountered.
  virtual bool LoadFoldingSets(const string &data_file_path,
                               const string &lang,
                               LangModel *lang_mod);
  // Folds the output of the NeuralNet using the loaded folding sets
  virtual void Fold();
  // Scales the input char_samp and feeds it to the NeuralNet as input
  bool RunNets(CharSamp *char_samp);
};
}
#endif  // HYBRID_NEURAL_NET_CLASSIFIER_H
