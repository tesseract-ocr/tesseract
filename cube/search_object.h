/**********************************************************************
 * File:        search_object.h
 * Description: Declaration of the Beam Search Object Class
 * Author:    Ahmad Abdulkader
 * Created:   2008
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

// The SearchObject class represents a char_samp (a word bitmap) that is
// being searched for characters (or recognizeable entities).
// This is an abstract class that all SearchObjects should inherit from
// A SearchObject class provides methods to:
// 1- Returns the count of segments
// 2- Recognize a segment range
// 3- Creates a CharSamp for a segment range

#ifndef SEARCH_OBJECT_H
#define SEARCH_OBJECT_H

#include "char_altlist.h"
#include "char_samp.h"
#include "cube_reco_context.h"

namespace tesseract {
class SearchObject {
 public:
  explicit SearchObject(CubeRecoContext *cntxt) { cntxt_ = cntxt; }
  virtual ~SearchObject() {}

  virtual int SegPtCnt() = 0;
  virtual CharAltList *RecognizeSegment(int start_pt, int end_pt) = 0;
  virtual CharSamp *CharSample(int start_pt, int end_pt) = 0;
  virtual Box* CharBox(int start_pt, int end_pt) = 0;

  virtual int SpaceCost(int seg_pt) = 0;
  virtual int NoSpaceCost(int seg_pt) = 0;
  virtual int NoSpaceCost(int start_pt, int end_pt) = 0;

 protected:
  CubeRecoContext *cntxt_;
};
}

#endif  // SEARCH_OBJECT_H
