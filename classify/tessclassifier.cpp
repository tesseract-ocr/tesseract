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
int TessClassifier::ClassifySample(const TrainingSample& sample,
                                   Pix* page_pix, int debug, int keep_this,
                                   GenericVector<ShapeRating>* results) {
  if (debug) {
    classify_->matcher_debug_level.set_value(debug ? 2 : 0);
    classify_->matcher_debug_flags.set_value(debug ? 25 : 0);
    classify_->classify_debug_level.set_value(debug ? 3 : 0);
  } else {
    classify_->classify_debug_level.set_value(debug ? 2 : 0);
  }
  classify_->CharNormTrainingSample(pruner_only_, sample, results);
  return results->size();
}

// Provides access to the ShapeTable that this classifier works with.
const ShapeTable* TessClassifier::GetShapeTable() const {
  return classify_->shape_table();
}

}  // namespace tesseract


