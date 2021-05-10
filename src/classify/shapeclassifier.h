///////////////////////////////////////////////////////////////////////
// File:        shapeclassifier.h
// Description: Base interface class for classifiers that return a
//              shape index.
// Author:      Ray Smith
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

#include "image.h"

#include <tesseract/unichar.h>

#include <vector>

struct Pix;

namespace tesseract {

class ScrollView;
class UNICHARSET;

struct ShapeRating;
class ShapeTable;
class TrainingSample;
class TrainingSampleSet;
struct UnicharRating;

// Interface base class for classifiers that produce ShapeRating results.
class TESS_API ShapeClassifier {
public:
  virtual ~ShapeClassifier() = default;

  // Classifies the given [training] sample, writing to results.
  // If page_pix is not nullptr, the overriding function may call
  // sample.GetSamplePix(padding, page_pix) to get an image of the sample
  // padded (with real image data) by the given padding to extract features
  // from the image of the character. Other members of TrainingSample:
  // features(), micro_features(), cn_feature(), geo_feature() may be used
  // to get the appropriate tesseract features.
  // If debug is non-zero, then various degrees of classifier dependent debug
  // information is provided.
  // If keep_this (a UNICHAR_ID) is >= 0, then the results should always
  // contain keep_this, and (if possible) anything of intermediate confidence.
  // (Used for answering "Why didn't it get that right?" questions.) It must
  // be a UNICHAR_ID as the callers have no clue how to choose the best shape
  // that may contain a desired answer.
  // The return value is the number of classes saved in results.
  // NOTE that overriding functions MUST clear and sort the results by
  // descending rating unless the classifier is working with a team of such
  // classifiers.
  // NOTE: Neither overload of ClassifySample is pure, but at least one must
  // be overridden by a classifier in order for it to do anything.
  virtual int UnicharClassifySample(const TrainingSample &sample, Image page_pix, int debug,
                                    UNICHAR_ID keep_this, std::vector<UnicharRating> *results);

protected:
  virtual int ClassifySample(const TrainingSample &sample, Image page_pix, int debug,
                             UNICHAR_ID keep_this, std::vector<ShapeRating> *results);

public:
  // Returns the shape that contains unichar_id that has the best result.
  // If result is not nullptr, it is set with the shape_id and rating.
  // Returns -1 if ClassifySample fails to provide any result containing
  // unichar_id. BestShapeForUnichar does not need to be overridden if
  // ClassifySample respects the keep_this rule.
  virtual int BestShapeForUnichar(const TrainingSample &sample, Image page_pix,
                                  UNICHAR_ID unichar_id, ShapeRating *result);

  // Provides access to the ShapeTable that this classifier works with.
  virtual const ShapeTable *GetShapeTable() const = 0;
  // Provides access to the UNICHARSET that this classifier works with.
  // Must be overridden IFF GetShapeTable() returns nullptr.
  virtual const UNICHARSET &GetUnicharset() const;

  // Visual debugger classifies the given sample, displays the results and
  // solicits user input to display other classifications. Returns when
  // the user has finished with debugging the sample.
  // Probably doesn't need to be overridden if the subclass provides
  // DisplayClassifyAs.
  void DebugDisplay(const TrainingSample &sample, Image page_pix, UNICHAR_ID unichar_id);

  // Displays classification as the given unichar_id. Creates as many windows
  // as it feels fit, using index as a guide for placement. Adds any created
  // windows to the windows output and returns a new index that may be used
  // by any subsequent classifiers. Caller waits for the user to view and
  // then destroys the windows by clearing the vector.
  virtual int DisplayClassifyAs(const TrainingSample &sample, Image page_pix, UNICHAR_ID unichar_id,
                                int index, std::vector<ScrollView *> &windows);

  // Prints debug information on the results. context is some introductory/title
  // message.
  virtual void UnicharPrintResults(const char *context,
                                   const std::vector<UnicharRating> &results) const;
  virtual void PrintResults(const char *context, const std::vector<ShapeRating> &results) const;

protected:
  // Removes any result that has all its unichars covered by a better choice,
  // regardless of font.
  void FilterDuplicateUnichars(std::vector<ShapeRating> *results) const;
};

} // namespace tesseract.

#endif // TESSERACT_CLASSIFY_SHAPECLASSIFIER_H_
