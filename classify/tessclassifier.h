// Copyright 2011 Google Inc. All Rights Reserved.
// Author: rays@google.com (Ray Smith)
///////////////////////////////////////////////////////////////////////
// File:        tessclassifier.h
// Description: Tesseract implementation of a ShapeClassifier.
// Author:      Ray Smith
// Created:     Tue Nov 22 14:10:45 PST 2011
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

#ifndef THIRD_PARTY_TESSERACT_CLASSIFY_TESSCLASSIFIER_H_
#define THIRD_PARTY_TESSERACT_CLASSIFY_TESSCLASSIFIER_H_

#include "shapeclassifier.h"

namespace tesseract {

class Classify;
class TrainingSample;

// Tesseract implementation of a ShapeClassifier.
// Due to limitations in the content of TrainingSample, this currently
// only works for the static classifier and only works if the ShapeTable
// in classify is not NULL.
class TessClassifier : public ShapeClassifier {
 public:
  TessClassifier(bool pruner_only, tesseract::Classify* classify)
    : pruner_only_(pruner_only), classify_(classify) {}
  virtual ~TessClassifier() {}

  // Classifies the given [training] sample, writing to results.
  // See ShapeClassifier for a full description.
  virtual int ClassifySample(const TrainingSample& sample, Pix* page_pix,
                             int debug, int keep_this,
                             GenericVector<ShapeRating>* results);
  // Provides access to the ShapeTable that this classifier works with.
  virtual const ShapeTable* GetShapeTable() const;

 private:
  // Indicates that this classifier is to use just the ClassPruner, or the
  // full classifier if false.
  bool pruner_only_;
  // Borrowed pointer to the actual Tesseract classifier.
  tesseract::Classify* classify_;
};


}  // namespace tesseract





#endif /* THIRD_PARTY_TESSERACT_CLASSIFY_TESSCLASSIFIER_H_ */
