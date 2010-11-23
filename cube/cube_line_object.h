/**********************************************************************
 * File:        cube_line_object.h
 * Description: Declaration of the Cube Line Object Class
 * Author:    Ahmad Abdulkader
 * Created:   2007
 *
 * (C) Copyright 2008, Google Inc.
 ** Licensed under the Apache License, Version 2.0 (the "License");
 ** you may not use this file except in compliance with the License.
 ** You may obtain a copy of the License at
 ** http://www.apache.org/licenses/LICENSE-2.0
 ** Unless required by applicable law or agreed to in writing, software
 ** distributed under the License is distributed on an "AS IS" BASIS,
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 ** See the License for the specific language governing permissions and
 ** limitations under the License.
 *
 **********************************************************************/

// The CubeLineObject implements an objects that holds a line of text
// Each line is broken into phrases. Phrases are blocks within the line that
// are unambiguously separate collections of words

#ifndef CUBE_LINE_OBJECT_H
#define CUBE_LINE_OBJECT_H

#include "cube_reco_context.h"
#include "cube_object.h"
#include "allheaders.h"

namespace tesseract {
class CubeLineObject {
 public:
  CubeLineObject(CubeRecoContext *cntxt, Pix *pix);
  ~CubeLineObject();

  // accessors
  inline int PhraseCount() {
    if (!processed_ && !Process()) {
      return 0;
    }
    return phrase_cnt_;
  }
  inline CubeObject **Phrases() {
    if (!processed_ && !Process()) {
      return NULL;
    }
    return phrases_;
  }

 private:
  CubeRecoContext *cntxt_;
  bool own_pix_;
  bool processed_;
  Pix *line_pix_;
  CubeObject **phrases_;
  int phrase_cnt_;
  bool Process();
  // Compute the least word breaking threshold that is required to produce a
  // valid set of phrases. Phrases are validated using the Aspect ratio
  // constraints specified in the language specific Params object
  int ComputeWordBreakThreshold(int con_comp_cnt, ConComp **con_comps,
                                bool rtl);
};
}

#endif  // CUBE_LINE_OBJECT_H
