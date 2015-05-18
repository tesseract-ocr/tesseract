// Copyright 2011 Google Inc. All Rights Reserved.
// Author: rays@google.com (Ray Smith)
///////////////////////////////////////////////////////////////////////
// File:        tessclassifier.cpp
// Description: Tesseract implementation of a ShapeClassifier.
// Author:      Ray Smith
// Created:     Tue Nov 22 14:16:25 PST 2011
//
// (C) Copyright 2011, Google Inc.
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

#include "tessclassifier.h"

#include "classify.h"
#include "trainingsample.h"

namespace tesseract {

// Classifies the given [training] sample, writing to results.
// See ShapeClassifier for a full description.
int TessClassifier::UnicharClassifySample(
    const TrainingSample& sample, Pix* page_pix, int debug,
    UNICHAR_ID keep_this, GenericVector<UnicharRating>* results) {
  int old_matcher_level = classify_->matcher_debug_level;
  int old_matcher_flags = classify_->matcher_debug_flags;
  int old_classify_level = classify_->classify_debug_level;
  if (debug) {
    // Explicitly set values of various control parameters to generate debug
    // output if required, restoring the old values after classifying.
    classify_->matcher_debug_level.set_value(2);
    classify_->matcher_debug_flags.set_value(25);
    classify_->classify_debug_level.set_value(3);
  }
  classify_->CharNormTrainingSample(pruner_only_, keep_this, sample, results);
  if (debug) {
    classify_->matcher_debug_level.set_value(old_matcher_level);
    classify_->matcher_debug_flags.set_value(old_matcher_flags);
    classify_->classify_debug_level.set_value(old_classify_level);
  }
  return results->size();
}

// Provides access to the ShapeTable that this classifier works with.
const ShapeTable* TessClassifier::GetShapeTable() const {
  return classify_->shape_table();
}
// Provides access to the UNICHARSET that this classifier works with.
// Only needs to be overridden if GetShapeTable() can return NULL.
const UNICHARSET& TessClassifier::GetUnicharset() const {
  return classify_->unicharset;
}

// Displays classification as the given shape_id. Creates as many windows
// as it feels fit, using index as a guide for placement. Adds any created
// windows to the windows output and returns a new index that may be used
// by any subsequent classifiers. Caller waits for the user to view and
// then destroys the windows by clearing the vector.
int TessClassifier::DisplayClassifyAs(
    const TrainingSample& sample, Pix* page_pix, int unichar_id, int index,
    PointerVector<ScrollView>* windows) {
  int shape_id = unichar_id;
  // TODO(rays) Fix this so it works with both flat and real shapetables.
  //  if (GetShapeTable() != NULL)
  //  shape_id = BestShapeForUnichar(sample, page_pix, unichar_id, NULL);
  if (shape_id < 0) return index;
  if (UnusedClassIdIn(classify_->PreTrainedTemplates, shape_id)) {
    tprintf("No built-in templates for class/shape %d\n", shape_id);
    return index;
  }
  classify_->ShowBestMatchFor(shape_id, sample.features(),
                              sample.num_features());
  return index;
}

}  // namespace tesseract


