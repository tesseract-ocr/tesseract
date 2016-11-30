/**********************************************************************
 * File:        classifier_factory.cpp
 * Description: Implementation of the Base Character Classifier
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

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "classifier_factory.h"
#include "conv_net_classifier.h"
#include "feature_base.h"
#include "feature_bmp.h"
#include "feature_chebyshev.h"
#include "feature_hybrid.h"
#include "hybrid_neural_net_classifier.h"

namespace tesseract {

// Creates a CharClassifier object of the appropriate type depending on the
// classifier type in the settings file
CharClassifier *CharClassifierFactory::Create(const string &data_file_path,
                                              const string &lang,
                                              LangModel *lang_mod,
                                              CharSet *char_set,
                                              TuningParams *params) {
  // create the feature extraction object
  FeatureBase *feat_extract;

  switch (params->TypeFeature()) {
    case TuningParams::BMP:
      feat_extract = new FeatureBmp(params);
      break;
    case TuningParams::CHEBYSHEV:
      feat_extract = new FeatureChebyshev(params);
      break;
    case TuningParams::HYBRID:
      feat_extract = new FeatureHybrid(params);
      break;
    default:
      fprintf(stderr, "Cube ERROR (CharClassifierFactory::Create): invalid "
              "feature type.\n");
      return NULL;
  }

  // create the classifier object
  CharClassifier *classifier_obj;
  switch (params->TypeClassifier()) {
    case TuningParams::NN:
      classifier_obj = new ConvNetCharClassifier(char_set, params,
                                                 feat_extract);
      break;
    case TuningParams::HYBRID_NN:
      classifier_obj = new HybridNeuralNetCharClassifier(char_set, params,
                                                         feat_extract);
      break;
    default:
      fprintf(stderr, "Cube ERROR (CharClassifierFactory::Create): invalid "
              "classifier type.\n");
      return NULL;
  }

  // Init the classifier
  if (!classifier_obj->Init(data_file_path, lang, lang_mod)) {
    delete classifier_obj;
    fprintf(stderr, "Cube ERROR (CharClassifierFactory::Create): unable "
            "to Init() character classifier object.\n");
    return NULL;
  }
  return classifier_obj;
}
}
