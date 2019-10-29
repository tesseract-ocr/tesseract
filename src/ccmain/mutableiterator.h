///////////////////////////////////////////////////////////////////////
// File:        mutableiterator.h
// Description: Iterator for tesseract results providing access to
//              both high-level API and Tesseract internal data structures.
// Author:      David Eger
// Created:     Thu Feb 24 19:01:06 PST 2011
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

#ifndef TESSERACT_CCMAIN_MUTABLEITERATOR_H_
#define TESSERACT_CCMAIN_MUTABLEITERATOR_H_

#include <tesseract/resultiterator.h>

class BLOB_CHOICE_IT;

namespace tesseract {

class Tesseract;

// Class to iterate over tesseract results, providing access to all levels
// of the page hierarchy, without including any tesseract headers or having
// to handle any tesseract structures.
// WARNING! This class points to data held within the TessBaseAPI class, and
// therefore can only be used while the TessBaseAPI class still exists and
// has not been subjected to a call of Init, SetImage, Recognize, Clear, End
// DetectOS, or anything else that changes the internal PAGE_RES.
// See tesseract/apitypes.h for the definition of PageIteratorLevel.
// See also base class PageIterator, which contains the bulk of the interface.
// ResultIterator adds text-specific methods for access to OCR output.
// MutableIterator adds access to internal data structures.

class MutableIterator : public ResultIterator {
 public:
  // See argument descriptions in ResultIterator()
  MutableIterator(PAGE_RES* page_res, Tesseract* tesseract,
                  int scale, int scaled_yres,
                  int rect_left, int rect_top,
                  int rect_width, int rect_height)
      : ResultIterator(
          LTRResultIterator(page_res, tesseract, scale, scaled_yres, rect_left,
                            rect_top, rect_width, rect_height)) {}
  ~MutableIterator() override;

  // See PageIterator and ResultIterator for most calls.

  // Return access to Tesseract internals.
  const PAGE_RES_IT *PageResIt() const { return it_; }
};

}  // namespace tesseract.

#endif  // TESSERACT_CCMAIN_MUTABLEITERATOR_H_
