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

#ifndef USE_STD_NAMESPACE
#include "base/commandlineflags.h"
#endif
#include "baseapi.h"
#include "commontraining.h"
#include "cubeclassifier.h"
#include "mastertrainer.h"
#include "params.h"
#include "strngs.h"
#include "tessclassifier.h"

STRING_PARAM_FLAG(classifier, "", "Classifier to test");
STRING_PARAM_FLAG(lang, "eng", "Language to test");
STRING_PARAM_FLAG(tessdata_dir, "", "Directory of traineddata files");

enum ClassifierName {
  CN_PRUNER,
  CN_FULL,
  CN_CUBE,
  CN_CUBETESS,
  CN_COUNT
};

const char* names[] = {"pruner", "full", "cube", "cubetess", NULL };

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
  // Decode the classifier string.
  ClassifierName classifier = CN_COUNT;
  for (int c = 0; c < CN_COUNT; ++c) {
    if (strcmp(FLAGS_classifier.c_str(), names[c]) == 0) {
      classifier = static_cast<ClassifierName>(c);
      break;
    }
  }
  if (classifier == CN_COUNT) {
    fprintf(stderr, "Invalid classifier name:%s\n", FLAGS_classifier.c_str());
    return 1;
  }

  STRING file_prefix;
  tesseract::MasterTrainer* trainer = tesseract::LoadTrainingData(
      argc, argv, true, NULL, &file_prefix);
  // We want to test junk as well if it is available.
  trainer->IncludeJunk();
  // We want to test with replicated samples too.
  trainer->ReplicateAndRandomizeSamplesIfRequired();

  // We need to initialize tesseract to test.
  tesseract::TessBaseAPI api;
  tesseract::OcrEngineMode engine_mode = tesseract::OEM_TESSERACT_ONLY;
  if (classifier == CN_CUBE || classifier == CN_CUBETESS)
    engine_mode = tesseract::OEM_TESSERACT_CUBE_COMBINED;
  if (api.Init(FLAGS_tessdata_dir.c_str(), FLAGS_lang.c_str(),
               engine_mode) < 0) {
    fprintf(stderr, "Tesseract initialization failed!\n");
    return 1;
  }
  tesseract::ShapeClassifier* shape_classifier = NULL;
  tesseract::Tesseract* tesseract =
      const_cast<tesseract::Tesseract*>(api.tesseract());
  tesseract::Classify* classify =
      reinterpret_cast<tesseract::Classify*>(tesseract);
  // Copy the shape_table from the classifier and add the space character if
  // not already present to count junk.
  tesseract::ShapeTable shape_table;
  shape_table.set_unicharset(classify->shape_table()->unicharset());
  shape_table.AppendMasterShapes(*classify->shape_table());
  if (shape_table.FindShape(0, -1) < 0)
    shape_table.AddShape(0, 0);
  if (classifier == CN_PRUNER) {
    shape_classifier = new tesseract::TessClassifier(true, classify);
  } else if (classifier == CN_FULL) {
    shape_classifier = new tesseract::TessClassifier(false, classify);
  } else if (classifier == CN_CUBE) {
    shape_classifier = new tesseract::CubeClassifier(tesseract);
  } else if (classifier == CN_CUBETESS) {
    shape_classifier = new tesseract::CubeTessClassifier(tesseract);
  } else {
    fprintf(stderr, "%s tester not yet implemented\n",
            FLAGS_classifier.c_str());
    return 1;
  }
  tprintf("Testing classifier %s:\n", FLAGS_classifier.c_str());
  trainer->TestClassifierOnSamples(3, false, shape_classifier, NULL);
  if (classifier != CN_CUBE && classifier != CN_CUBETESS) {
    // Test with replicated samples as well.
    trainer->TestClassifierOnSamples(3, true, shape_classifier, NULL);
  }
  delete shape_classifier;
  delete trainer;

  return 0;
} /* main */






