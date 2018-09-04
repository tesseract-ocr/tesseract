///////////////////////////////////////////////////////////////////////
// File:        blobgrid.h
// Description: BBGrid of BLOBNBOX with useful BLOBNBOX-specific methods.
// Copyright 2011 Google Inc. All Rights Reserved.
// Author: rays@google.com (Ray Smith)
// Created:     Sat Jun 11 10:26:01 PST 2011
//
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


#ifndef TESSERACT_TEXTORD_BLOBGRID_H_
#define TESSERACT_TEXTORD_BLOBGRID_H_

#include "bbgrid.h"
#include "blobbox.h"

CLISTIZEH(BLOBNBOX)

namespace tesseract {

using BlobGridSearch = GridSearch<BLOBNBOX, BLOBNBOX_CLIST, BLOBNBOX_C_IT>;

class BlobGrid : public BBGrid<BLOBNBOX, BLOBNBOX_CLIST, BLOBNBOX_C_IT> {
 public:
  BlobGrid(int gridsize, const ICOORD& bleft, const ICOORD& tright);
  virtual ~BlobGrid();

  // Inserts all the blobs from the given list, with x and y spreading,
  // without removing from the source list, so ownership remains with the
  // source list.
  void InsertBlobList(BLOBNBOX_LIST* blobs);
};

}  // namespace tesseract.

#endif  // TESSERACT_TEXTORD_BLOBGRID_H_
