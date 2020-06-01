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

#include <algorithm>
#include <cstdio>
#ifdef GOOGLE_TESSERACT
#include "base/commandlineflags.h"
#endif  // GOOGLE_TESSERACT
#include <tesseract/baseapi.h>
#include "commontraining.h"
#include "mastertrainer.h"
#include "params.h"
#include <tesseract/strngs.h>
#include "tessclassifier.h"
#include "tesseractclass.h"

static STRING_PARAM_FLAG(classifier, "", "Classifier to test");
static STRING_PARAM_FLAG(lang, "eng", "Language to test");
static STRING_PARAM_FLAG(tessdata_dir, "", "Directory of traineddata files");

enum ClassifierName {
  CN_PRUNER,
  CN_FULL,
  CN_COUNT
};

static const char* names[] = {"pruner", "full"};

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
    return nullptr;
  }

  // We need to initialize tesseract to test.
  *api = new tesseract::TessBaseAPI;
  tesseract::OcrEngineMode engine_mode = tesseract::OEM_TESSERACT_ONLY;
  tesseract::Tesseract* tesseract = nullptr;
  tesseract::Classify* classify = nullptr;
  if (
      classifier == CN_PRUNER || classifier == CN_FULL) {
    if ((*api)->Init(FLAGS_tessdata_dir.c_str(), FLAGS_lang.c_str(),
                 engine_mode) < 0) {
      fprintf(stderr, "Tesseract initialization failed!\n");
      return nullptr;
    }
    tesseract = const_cast<tesseract::Tesseract*>((*api)->tesseract());
    classify = static_cast<tesseract::Classify*>(tesseract);
    if (classify->shape_table() == nullptr) {
      fprintf(stderr, "Tesseract must contain a ShapeTable!\n");
      return nullptr;
    }
  }
  tesseract::ShapeClassifier* shape_classifier = nullptr;

  if (classifier == CN_PRUNER) {
    shape_classifier = new tesseract::TessClassifier(true, classify);
  } else if (classifier == CN_FULL) {
    shape_classifier = new tesseract::TessClassifier(false, classify);
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
//            with an input trainer.)
int main(int argc, char **argv) {
  tesseract::CheckSharedLibraryVersion();
  ParseArguments(&argc, &argv);
  STRING file_prefix;
  tesseract::MasterTrainer* trainer =
      tesseract::LoadTrainingData(argc, argv, false, nullptr, &file_prefix);
  tesseract::TessBaseAPI* api;
  // Decode the classifier string.
  tesseract::ShapeClassifier* shape_classifier = InitializeClassifier(
      FLAGS_classifier.c_str(), trainer->unicharset(), argc, argv, &api);
  if (shape_classifier == nullptr) {
    fprintf(stderr, "Classifier init failed!:%s\n", FLAGS_classifier.c_str());
    return 1;
  }

  // We want to test junk as well if it is available.
  // trainer->IncludeJunk();
  // We want to test with replicated samples too.
  trainer->ReplicateAndRandomizeSamplesIfRequired();

  trainer->TestClassifierOnSamples(tesseract::CT_UNICHAR_TOP1_ERR,
                                   std::max(3, static_cast<int>(FLAGS_debug_level)), false,
                                   shape_classifier, nullptr);
  delete shape_classifier;
  delete api;
  delete trainer;

  return 0;
} /* main */
