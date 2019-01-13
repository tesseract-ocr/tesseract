///////////////////////////////////////////////////////////////////////
// File:        blobgrid.cpp
// Description: BBGrid of BLOBNBOX with useful BLOBNBOX-specific methods.
// Copyright 2011 Google Inc. All Rights Reserved.
// Author: rays@google.com (Ray Smith)
// Created:     Sat Jun 11 10:30:01 PST 2011
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

#include "blobgrid.h"

namespace tesseract {

BlobGrid::BlobGrid(int gridsize, const ICOORD& bleft, const ICOORD& tright)
  : BBGrid<BLOBNBOX, BLOBNBOX_CLIST, BLOBNBOX_C_IT>(gridsize, bleft, tright) {
}

// Destructor.
// It is defined here, so the compiler can create a single vtable
// instead of weak vtables in every compilation unit.
BlobGrid::~BlobGrid() = default;

// Inserts all the blobs from the given list, with x and y spreading,
// without removing from the source list, so ownership remains with the
// source list.
void BlobGrid::InsertBlobList(BLOBNBOX_LIST* blobs) {
  BLOBNBOX_IT blob_it(blobs);
  for (blob_it.mark_cycle_pt(); !blob_it.cycled_list(); blob_it.forward()) {
    BLOBNBOX* blob = blob_it.data();
    if (!blob->joined_to_prev())
      InsertBBox(true, true, blob);
  }
}

}  // namespace tesseract.
