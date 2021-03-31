///////////////////////////////////////////////////////////////////////
// File:        equationdetectbase.h
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

#ifndef TESSERACT_TEXTORD_EQUATIONDETECTBASE_H_
#define TESSERACT_TEXTORD_EQUATIONDETECTBASE_H_

#include "image.h"

class BLOBNBOX_LIST;
class TO_BLOCK;
struct Pix;

namespace tesseract {

class ColPartitionGrid;
class ColPartitionSet;
class BLOBNBOX;

class TESS_API EquationDetectBase {
public:
  EquationDetectBase() = default;
  virtual ~EquationDetectBase();

  // Iterate over the blobs inside to_block, and set the blobs that we want to
  // process to BSTT_NONE. (By default, they should be BSTT_SKIP). The function
  // returns 0 upon success.
  virtual int LabelSpecialText(TO_BLOCK *to_block) = 0;

  // Interface to find possible equation partition grid from part_grid. This
  // should be called after IdentifySpecialText function.
  virtual int FindEquationParts(ColPartitionGrid *part_grid, ColPartitionSet **best_columns) = 0;

  // Debug function: Render a bounding box on pix based on the value of its
  // special_text_type, specifically:
  // BSTT_MATH: red box
  // BSTT_DIGIT: cyan box
  // BSTT_ITALIC: green box
  // BSTT_UNCLEAR: blue box
  // All others: yellow box
  static void RenderSpecialText(Image pix, BLOBNBOX *blob);
};

} // namespace tesseract

#endif // TESSERACT_TEXTORD_EQUATIONDETECTBASE_H_
