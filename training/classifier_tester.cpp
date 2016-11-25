// Copyright 2011 Google Inc. All Rights Reserved.
// Author: rays@google.com (Ray Smith)

// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

//  Filename: classifier_tester.cpp
//  Purpose:  Tests a character classifier on data as formatted for training,
//            but doesn't have to be the same as the training data.
//  Author:   Ray Smith

#include <stdio.h>
#ifndef USE_STD_NAMESPACE
#include "base/commandlineflags.h"
#endif  // USE_STD_NAMESPACE
#include "baseapi.h"
#include "commontraining.h"
#ifndef NO_CUBE_BUILD
#include "cubeclassifier.h"
#endif  // NO_CUBE_BUILD
#include "mastertrainer.h"
#include "params.h"
#include "strngs.h"
#include "tessclassifier.h"

STRING_PARAM_FLAG(classifier, "", "Classifier to test");
STRING_PARAM_FLAG(lang, "eng", "Language to test");
STRING_PARAM_FLAG(tessdata_dir, "", "Directory of traineddata files");
DECLARE_INT_PARAM_FLAG(debug_level);
DECLARE_STRING_PARAM_FLAG(T);

enum ClassifierName {
  CN_PRUNER,
  CN_FULL,
#ifndef NO_CUBE_BUILD
  CN_CUBE,
  CN_CUBETESS,
#endif  // NO_CUBE_BUILD
  CN_COUNT
};

const char* names[] = {"pruner", "full",
#ifndef NO_CUBE_BUILD
                       "cube", "cubetess",
#endif  // NO_CUBE_BUILD
                       NULL};

static tesseract::ShapeClassifier* InitializeClassifier(
    const char* classifer_name, const UNICHARSET& unicharset,
    int argc, char **argv,
    tesseract::TessBaseAPI** api) {
  // Decode the classifier string.
  ClassifierName classifier = CN_COUNT;
  for (int c = 0; c < CN_COUNT; ++c) {
    if (strcmp(classifer_name, names[c]) == 0) {
      classifier = static_cast<ClassifierName>(c);
      break;
    }
  }
  if (classifier == CN_COUNT) {
    fprintf(stderr, "Invalid classifier name:%s\n", FLAGS_classifier.c_str());
    return NULL;
  }

  // We need to initialize tesseract to test.
  *api = new tesseract::TessBaseAPI;
  tesseract::OcrEngineMode engine_mode = tesseract::OEM_TESSERACT_ONLY;
#ifndef NO_CUBE_BUILD
  if (classifier == CN_CUBE || classifier == CN_CUBETESS)
    engine_mode = tesseract::OEM_TESSERACT_CUBE_COMBINED;
#endif  // NO_CUBE_BUILD
  tesseract::Tesseract* tesseract = NULL;
  tesseract::Classify* classify = NULL;
  if (
#ifndef NO_CUBE_BUILD
      classifier == CN_CUBE || classifier == CN_CUBETESS ||
#endif  // NO_CUBE_BUILD
      classifier == CN_PRUNER || classifier == CN_FULL) {
#ifndef NO_CUBE_BUILD
    (*api)->SetVariable("cube_debug_level", "2");
#endif  // NO_CUBE_BUILD
    if ((*api)->Init(FLAGS_tessdata_dir.c_str(), FLAGS_lang.c_str(),
                 engine_mode) < 0) {
      fprintf(stderr, "Tesseract initialization failed!\n");
      return NULL;
    }
    tesseract = const_cast<tesseract::Tesseract*>((*api)->tesseract());
    classify = reinterpret_cast<tesseract::Classify*>(tesseract);
    if (classify->shape_table() == NULL) {
      fprintf(stderr, "Tesseract must contain a ShapeTable!\n");
      return NULL;
    }
  }
  tesseract::ShapeClassifier* shape_classifier = NULL;

  if (!FLAGS_T.empty()) {
    const char* config_name;
    while ((config_name = GetNextFilename(argc, argv)) != NULL) {
      tprintf("Reading config file %s ...\n", config_name);
      (*api)->ReadConfigFile(config_name);
    }
  }
  if (classifier == CN_PRUNER) {
    shape_classifier = new tesseract::TessClassifier(true, classify);
  } else if (classifier == CN_FULL) {
    shape_classifier = new tesseract::TessClassifier(false, classify);
#ifndef NO_CUBE_BUILD
  } else if (classifier == CN_CUBE) {
    shape_classifier = new tesseract::CubeClassifier(tesseract);
  } else if (classifier == CN_CUBETESS) {
    shape_classifier = new tesseract::CubeTessClassifier(tesseract);
#endif  // NO_CUBE_BUILD
  } else {
    fprintf(stderr, "%s tester not yet implemented\n", classifer_name);
    return NULL;
  }
  tprintf("Testing classifier %s:\n", classifer_name);
  return shape_classifier;
}

// This program has complex setup requirements, so here is some help:
// Two different modes, tr files and serialized mastertrainer.
// From tr files:
//   classifier_tester -U unicharset -F font_properties -X xheights
//     -classifier x -lang lang [-output_trainer trainer] *.tr
// From a serialized trainer:
//  classifier_tester -input_trainer trainer [-lang lang] -classifier x
//
// In the first case, the unicharset must be the unicharset from within
// the classifier under test, and the font_properties and xheights files must
// match the files used during training.
// In the second case, the trainer file must have been prepared from
// some previous run of shapeclustering, mftraining, or classifier_tester
// using the same conditions as above, ie matching unicharset/font_properties.
//
// Available values of classifier (x above) are:
// pruner   : Tesseract class pruner only.
// full     : Tesseract full classifier.
// cube     : Cube classifier. (Not possible with an input trainer.)
// cubetess : Tesseract class pruner with rescoring by Cube.  (Not possible
//            with an input trainer.)
int main(int argc, char **argv) {
  ParseArguments(&argc, &argv);
  STRING file_prefix;
  tesseract::MasterTrainer* trainer = tesseract::LoadTrainingData(
      argc, argv, false, NULL, &file_prefix);
  tesseract::TessBaseAPI* api;
  // Decode the classifier string.
  tesseract::ShapeClassifier* shape_classifier = InitializeClassifier(
      FLAGS_classifier.c_str(), trainer->unicharset(), argc, argv, &api);
  if (shape_classifier == NULL) {
    fprintf(stderr, "Classifier init failed!:%s\n", FLAGS_classifier.c_str());
    return 1;
  }

  // We want to test junk as well if it is available.
  // trainer->IncludeJunk();
  // We want to test with replicated samples too.
  trainer->ReplicateAndRandomizeSamplesIfRequired();

  trainer->TestClassifierOnSamples(tesseract:: CT_UNICHAR_TOP1_ERR,
                                   MAX(3, FLAGS_debug_level), false,
                                   shape_classifier, NULL);
  delete shape_classifier;
  delete api;
  delete trainer;

  return 0;
} /* main */






