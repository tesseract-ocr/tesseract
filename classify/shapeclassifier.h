// Copyright 2011 Google Inc. All Rights Reserved.
// Author: rays@google.com (Ray Smith)
///////////////////////////////////////////////////////////////////////
// File:        shapeclassifier.h
// Description: Base interface class for classifiers that return a
//              shape index.
// Author:      Ray Smith
// Created:     Tue Sep 13 11:26:32 PDT 2011
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

#ifndef TESSERACT_CLASSIFY_SHAPECLASSIFIER_H_
#define TESSERACT_CLASSIFY_SHAPECLASSIFIER_H_

template <typename T> class GenericVector;
struct Pix;

namespace tesseract {

class ShapeTable;
class TrainingSample;

// Classifier result from a low-level classification is an index into some
// ShapeTable and a rating.
struct ShapeRating {
  ShapeRating() : shape_id(0), rating(0.0f), raw(0.0f), font(0.0f) {}
  ShapeRating(int s, float r)
    : shape_id(s), rating(r), raw(1.0f), font(0.0f) {}

  // Sort function to sort ratings appropriately by descending rating.
  static int SortDescendingRating(const void* t1, const void* t2) {
    const ShapeRating* a = reinterpret_cast<const ShapeRating *>(t1);
    const ShapeRating* b = reinterpret_cast<const ShapeRating *>(t2);
    if (a->rating > b->rating) {
      return -1;
    } else if (a->rating < b->rating) {
      return 1;
    } else {
      return a->shape_id - b->shape_id;
    }
  }

  // Index into some shape table indicates the class of the answer.
  int shape_id;
  // Rating from classifier with 1.0 perfect and 0.0 impossible.
  // Call it a probability if you must.
  float rating;
  // Subsidiary rating that a classifier may use internally.
  float raw;
  // Subsidiary rating that a classifier may use internally.
  float font;
};

// Interface base class for classifiers that produce ShapeRating results.
class ShapeClassifier {
 public:
  virtual ~ShapeClassifier() {}

  // Classifies the given [training] sample, writing to results.
  // If page_pix is not NULL, the overriding function may call
  // sample.GetSamplePix(padding, page_pix) to get an image of the sample
  // padded (with real image data) by the given padding to extract features
  // from the image of the character. Other members of TrainingSample:
  // features(), micro_features(), cn_feature(), geo_feature() may be used
  // to get the appropriate tesseract features.
  // If debug is non-zero, then various degrees of classifier dependent debug
  // information is provided.
  // If keep_this (a shape index) is >= 0, then the results should always
  // contain keep_this, and (if possible) anything of intermediate confidence.
  // (Used for answering "Why didn't it get that right?" questions.)
  // The return value is the number of classes saved in results.
  // NOTE that overriding functions MUST clear results unless the classifier
  // is working with a team of such classifiers.
  virtual int ClassifySample(const TrainingSample& sample, Pix* page_pix,
                             int debug, int keep_this,
                             GenericVector<ShapeRating>* results) = 0;

  // Provides access to the ShapeTable that this classifier works with.
  virtual const ShapeTable* GetShapeTable() const = 0;
};

}  // namespace tesseract.

#endif  // TESSERACT_CLASSIFY_SHAPECLASSIFIER_H_
