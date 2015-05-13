// Copyright 2011 Google Inc. All Rights Reserved.
// Author: rays@google.com (Ray Smith)
///////////////////////////////////////////////////////////////////////
// File:        cubeclassifier.cpp
// Description: Cube implementation of a ShapeClassifier.
// Author:      Ray Smith
// Created:     Wed Nov 23 10:39:45 PST 2011
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

#include "cubeclassifier.h"

#include "char_altlist.h"
#include "char_set.h"
#include "cube_object.h"
#include "cube_reco_context.h"
#include "tessclassifier.h"
#include "tesseractclass.h"
#include "trainingsample.h"
#include "unicharset.h"

namespace tesseract {

CubeClassifier::CubeClassifier(tesseract::Tesseract* tesseract)
    : cube_cntxt_(tesseract->GetCubeRecoContext()),
      shape_table_(*tesseract->shape_table()) {
}
CubeClassifier::~CubeClassifier() {
}

// Classifies the given [training] sample, writing to results.
// See ShapeClassifier for a full description.
int CubeClassifier::UnicharClassifySample(
    const TrainingSample& sample, Pix* page_pix, int debug,
    UNICHAR_ID keep_this, GenericVector<UnicharRating>* results) {
  results->clear();
  if (page_pix == NULL) return 0;

  ASSERT_HOST(cube_cntxt_ != NULL);
  const TBOX& char_box = sample.bounding_box();
  CubeObject* cube_obj = new tesseract::CubeObject(
      cube_cntxt_, page_pix, char_box.left(),
      pixGetHeight(page_pix) - char_box.top(),
      char_box.width(), char_box.height());
  CharAltList* alt_list = cube_obj->RecognizeChar();
  if (alt_list != NULL) {
    alt_list->Sort();
    CharSet* char_set = cube_cntxt_->CharacterSet();
    for (int i = 0; i < alt_list->AltCount(); ++i) {
      // Convert cube representation to a shape_id.
      int alt_id = alt_list->Alt(i);
      int unichar_id = char_set->UnicharID(char_set->ClassString(alt_id));
      if (unichar_id >= 0)
        results->push_back(UnicharRating(unichar_id, alt_list->AltProb(i)));
    }
    delete alt_list;
  }
  delete cube_obj;
  return results->size();
}

// Provides access to the ShapeTable that this classifier works with.
const ShapeTable* CubeClassifier::GetShapeTable() const {
  return &shape_table_;
}

CubeTessClassifier::CubeTessClassifier(tesseract::Tesseract* tesseract)
    : cube_cntxt_(tesseract->GetCubeRecoContext()),
      shape_table_(*tesseract->shape_table()),
      pruner_(new TessClassifier(true, tesseract)) {
}
CubeTessClassifier::~CubeTessClassifier() {
  delete pruner_;
}

// Classifies the given [training] sample, writing to results.
// See ShapeClassifier for a full description.
int CubeTessClassifier::UnicharClassifySample(
    const TrainingSample& sample, Pix* page_pix, int debug,
    UNICHAR_ID keep_this, GenericVector<UnicharRating>* results) {
  int num_results = pruner_->UnicharClassifySample(sample, page_pix, debug,
                                                   keep_this, results);
  if (page_pix == NULL) return num_results;

  ASSERT_HOST(cube_cntxt_ != NULL);
  const TBOX& char_box = sample.bounding_box();
  CubeObject* cube_obj = new tesseract::CubeObject(
      cube_cntxt_, page_pix, char_box.left(),
      pixGetHeight(page_pix) - char_box.top(),
      char_box.width(), char_box.height());
  CharAltList* alt_list = cube_obj->RecognizeChar();
  CharSet* char_set = cube_cntxt_->CharacterSet();
  if (alt_list != NULL) {
    for (int r = 0; r < num_results; ++r) {
      // Get the best cube probability of the unichar in the result.
      double best_prob = 0.0;
      for (int i = 0; i < alt_list->AltCount(); ++i) {
        int alt_id = alt_list->Alt(i);
        int unichar_id = char_set->UnicharID(char_set->ClassString(alt_id));
        if (unichar_id == (*results)[r].unichar_id &&
            alt_list->AltProb(i) > best_prob) {
          best_prob = alt_list->AltProb(i);
        }
      }
      (*results)[r].rating = best_prob;
    }
    delete alt_list;
    // Re-sort by rating.
    results->sort(&UnicharRating::SortDescendingRating);
  }
  delete cube_obj;
  return results->size();
}

// Provides access to the ShapeTable that this classifier works with.
const ShapeTable* CubeTessClassifier::GetShapeTable() const {
  return &shape_table_;
}

}  // namespace tesseract



