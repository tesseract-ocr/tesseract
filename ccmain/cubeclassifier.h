// Copyright 2011 Google Inc. All Rights Reserved.
// Author: rays@google.com (Ray Smith)
///////////////////////////////////////////////////////////////////////
// File:        cubeclassifier.h
// Description: Cube implementation of a ShapeClassifier.
// Author:      Ray Smith
// Created:     Wed Nov 23 10:36:32 PST 2011
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

#ifndef THIRD_PARTY_TESSERACT_CCMAIN_CUBECLASSIFIER_H_
#define THIRD_PARTY_TESSERACT_CCMAIN_CUBECLASSIFIER_H_

#include "shapeclassifier.h"

namespace tesseract {

class Classify;
class CubeRecoContext;
class ShapeTable;
class TessClassifier;
class Tesseract;
class TrainingSample;

// Cube implementation of a ShapeClassifier.
class CubeClassifier : public ShapeClassifier {
 public:
  explicit CubeClassifier(Tesseract* tesseract);
  virtual ~CubeClassifier();

  // Classifies the given [training] sample, writing to results.
  // See ShapeClassifier for a full description.
  virtual int ClassifySample(const TrainingSample& sample, Pix* page_pix,
                             int debug, int keep_this,
                             GenericVector<ShapeRating>* results);
  // Provides access to the ShapeTable that this classifier works with.
  virtual const ShapeTable* GetShapeTable() const;

 private:
  // Cube objects.
  CubeRecoContext* cube_cntxt_;
  const ShapeTable& shape_table_;
};

// Combination of Tesseract class pruner with scoring by cube.
class CubeTessClassifier : public ShapeClassifier {
 public:
  explicit CubeTessClassifier(Tesseract* tesseract);
  virtual ~CubeTessClassifier();

  // Classifies the given [training] sample, writing to results.
  // See ShapeClassifier for a full description.
  virtual int ClassifySample(const TrainingSample& sample, Pix* page_pix,
                             int debug, int keep_this,
                             GenericVector<ShapeRating>* results);
  // Provides access to the ShapeTable that this classifier works with.
  virtual const ShapeTable* GetShapeTable() const;

 private:
  // Cube objects.
  CubeRecoContext* cube_cntxt_;
  const ShapeTable& shape_table_;
  TessClassifier* pruner_;
};

}  // namespace tesseract

#endif /* THIRD_PARTY_TESSERACT_CCMAIN_CUBECLASSIFIER_H_ */
