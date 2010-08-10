///////////////////////////////////////////////////////////////////////
// File:        strokewidth.h
// Description: Subclass of BBGrid to find uniformity of strokewidth.
// Author:      Ray Smith
// Created:     Mon Mar 31 16:17:01 PST 2008
//
// (C) Copyright 2008, Google Inc.
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

#ifndef TESSERACT_TEXTORD_STROKEWIDTH_H__
#define TESSERACT_TEXTORD_STROKEWIDTH_H__

#include "bbgrid.h"         // Base class.
#include "tabvector.h"      // For BLOBNBOX_CLIST.

class TO_BLOCK;
class ScrollView;

namespace tesseract {

class TabFind;

/**
 * The StrokeWidth class holds all the normal and large blobs.
 * It is used to find good large blobs and move them to the normal blobs
 * by virtue of having a reasonable strokewidth compatible neighbour.
 */
class StrokeWidth : public BBGrid<BLOBNBOX, BLOBNBOX_CLIST, BLOBNBOX_C_IT> {
 public:
  StrokeWidth(int gridsize, const ICOORD& bleft, const ICOORD& tright);
  virtual ~StrokeWidth();

  // Puts the block blobs (normal and large) into the grid.
  void InsertBlobs(TO_BLOCK* block, TabFind* line_grid);

  // Moves the large blobs that have good stroke-width neighbours to the normal
  // blobs list.
  void MoveGoodLargeBlobs(int resolution, TO_BLOCK* block);

  // Displays the blobs green or red according to whether they are good or not.
  ScrollView* DisplayGoodBlobs(const char* window_name, ScrollView* tab_win);

  // Handles a click event in a display window.
  virtual void HandleClick(int x, int y);

 private:
  // Returns true if there is at least one side neighbour that has a similar
  // stroke width.
  bool GoodTextBlob(BLOBNBOX* blob);
};

}  // namespace tesseract.

#endif  // TESSERACT_TEXTORD_STROKEWIDTH_H__

