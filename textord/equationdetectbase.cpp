///////////////////////////////////////////////////////////////////////
// File:        equationdetectbase.cpp
// Description: The base class equation detection class.
// Author:      Zongyi (Joe) Liu (joeliu@google.com)
// Created:     Fri Aug 31 11:13:01 PST 2011
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

#include "allheaders.h"
#include "blobbox.h"
#include "equationdetectbase.h"

namespace tesseract {

EquationDetectBase::EquationDetectBase() {
}

EquationDetectBase::~EquationDetectBase() {
}

void EquationDetectBase::RenderSpecialText(Pix* pix,
                                           BLOBNBOX* blob) {
  ASSERT_HOST(pix != NULL && pixGetDepth(pix) == 32 && blob != NULL);
  const TBOX& tbox = blob->bounding_box();
  int height = pixGetHeight(pix);
  const int box_width = 5;

  // Coordinate translation: tesseract use left bottom as the original, while
  // leptonica uses left top as the original.
  Box *box = boxCreate(tbox.left(), height - tbox.top(),
                         tbox.width(), tbox.height());
  switch (blob->special_text_type()) {
    case BSTT_MATH:  // Red box.
      pixRenderBoxArb(pix, box, box_width, 255, 0, 0);
      break;
    case BSTT_DIGIT:  // cyan box.
      pixRenderBoxArb(pix, box, box_width, 0, 255, 255);
      break;
    case BSTT_ITALIC:  // Green box.
      pixRenderBoxArb(pix, box, box_width, 0, 255, 0);
      break;
    case BSTT_UNCLEAR:  // blue box.
      pixRenderBoxArb(pix, box, box_width, 0, 255, 0);
      break;
    case BSTT_NONE:
    default:
      // yellow box.
      pixRenderBoxArb(pix, box, box_width, 255, 255, 0);
      break;
  }
  boxDestroy(&box);
}

};  // namespace tesseract
