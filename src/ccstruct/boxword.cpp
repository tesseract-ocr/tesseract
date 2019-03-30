///////////////////////////////////////////////////////////////////////
// File:        boxword.cpp
// Description: Class to represent the bounding boxes of the output.
// Author:      Ray Smith
//
// (C) Copyright 2010, Google Inc.
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

#include "blobs.h"
#include "boxword.h"
#include "host.h"       // for NearlyEqual
#include "normalis.h"
#include "ocrblock.h"
#include "pageres.h"

namespace tesseract {

// Clip output boxes to input blob boxes for bounds that are within this
// tolerance. Otherwise, the blob may be chopped and we have to just use
// the word bounding box.
const int kBoxClipTolerance = 2;

BoxWord::BoxWord() : length_(0) {
}

BoxWord::BoxWord(const BoxWord& src) {
  CopyFrom(src);
}

BoxWord& BoxWord::operator=(const BoxWord& src) {
  CopyFrom(src);
  return *this;
}

void BoxWord::CopyFrom(const BoxWord& src) {
  bbox_ = src.bbox_;
  length_ = src.length_;
  boxes_.clear();
  boxes_.reserve(length_);
  for (int i = 0; i < length_; ++i)
    boxes_.push_back(src.boxes_[i]);
}

// Factory to build a BoxWord from a TWERD using the DENORMs on each blob to
// switch back to original image coordinates.
BoxWord* BoxWord::CopyFromNormalized(TWERD* tessword) {
  auto* boxword = new BoxWord();
  // Count the blobs.
  boxword->length_ = tessword->NumBlobs();
  // Allocate memory.
  boxword->boxes_.reserve(boxword->length_);

  for (int b = 0; b < boxword->length_; ++b) {
    TBLOB* tblob = tessword->blobs[b];
    TBOX blob_box;
    for (TESSLINE* outline = tblob->outlines; outline != nullptr;
         outline = outline->next) {
      EDGEPT* edgept = outline->loop;
      // Iterate over the edges.
      do {
        if (!edgept->IsHidden() || !edgept->prev->IsHidden()) {
          ICOORD pos(edgept->pos.x, edgept->pos.y);
          TPOINT denormed;
          tblob->denorm().DenormTransform(nullptr, edgept->pos, &denormed);
          pos.set_x(denormed.x);
          pos.set_y(denormed.y);
          TBOX pt_box(pos, pos);
          blob_box += pt_box;
        }
        edgept = edgept->next;
      } while (edgept != outline->loop);
    }
    boxword->boxes_.push_back(blob_box);
  }
  boxword->ComputeBoundingBox();
  return boxword;
}

// Clean up the bounding boxes from the polygonal approximation by
// expanding slightly, then clipping to the blobs from the original_word
// that overlap. If not null, the block provides the inverse rotation.
void BoxWord::ClipToOriginalWord(const BLOCK* block, WERD* original_word) {
  for (int i = 0; i < length_; ++i) {
    TBOX box = boxes_[i];
    // Expand by a single pixel, as the poly approximation error is 1 pixel.
    box = TBOX(box.left() - 1, box.bottom() - 1,
               box.right() + 1, box.top() + 1);
    // Now find the original box that matches.
    TBOX original_box;
    C_BLOB_IT b_it(original_word->cblob_list());
    for (b_it.mark_cycle_pt(); !b_it.cycled_list(); b_it.forward()) {
      TBOX blob_box = b_it.data()->bounding_box();
      if (block != nullptr)
        blob_box.rotate(block->re_rotation());
      if (blob_box.major_overlap(box)) {
        original_box += blob_box;
      }
    }
    if (!original_box.null_box()) {
      if (NearlyEqual<int>(original_box.left(), box.left(), kBoxClipTolerance))
        box.set_left(original_box.left());
      if (NearlyEqual<int>(original_box.right(), box.right(),
                           kBoxClipTolerance))
        box.set_right(original_box.right());
      if (NearlyEqual<int>(original_box.top(), box.top(), kBoxClipTolerance))
        box.set_top(original_box.top());
      if (NearlyEqual<int>(original_box.bottom(), box.bottom(),
                           kBoxClipTolerance))
        box.set_bottom(original_box.bottom());
    }
    original_box = original_word->bounding_box();
    if (block != nullptr)
      original_box.rotate(block->re_rotation());
    boxes_[i] = box.intersection(original_box);
  }
  ComputeBoundingBox();
}

// Merges the boxes from start to end, not including end, and deletes
// the boxes between start and end.
void BoxWord::MergeBoxes(int start, int end) {
  start = ClipToRange(start, 0, length_);
  end = ClipToRange(end, 0, length_);
  if (end <= start + 1)
    return;
  for (int i = start + 1; i < end; ++i) {
    boxes_[start] += boxes_[i];
  }
  int shrinkage = end - 1 - start;
  length_ -= shrinkage;
  for (int i = start + 1; i < length_; ++i)
    boxes_[i] = boxes_[i + shrinkage];
  boxes_.truncate(length_);
}

// Inserts a new box before the given index.
// Recomputes the bounding box.
void BoxWord::InsertBox(int index, const TBOX& box) {
  if (index < length_)
    boxes_.insert(box, index);
  else
    boxes_.push_back(box);
  length_ = boxes_.size();
  ComputeBoundingBox();
}

// Changes the box at the given index to the new box.
// Recomputes the bounding box.
void BoxWord::ChangeBox(int index, const TBOX& box) {
  boxes_[index] = box;
  ComputeBoundingBox();
}

// Deletes the box with the given index, and shuffles up the rest.
// Recomputes the bounding box.
void BoxWord::DeleteBox(int index) {
  ASSERT_HOST(0 <= index && index < length_);
  boxes_.remove(index);
  --length_;
  ComputeBoundingBox();
}

// Deletes all the boxes stored in BoxWord.
void BoxWord::DeleteAllBoxes() {
  length_ = 0;
  boxes_.clear();
  bbox_ = TBOX();
}

// Computes the bounding box of the word.
void BoxWord::ComputeBoundingBox() {
  bbox_ = TBOX();
  for (int i = 0; i < length_; ++i)
    bbox_ += boxes_[i];
}

// This and other putatively are the same, so call the (permanent) callback
// for each blob index where the bounding boxes match.
// The callback is deleted on completion.
void BoxWord::ProcessMatchedBlobs(const TWERD& other,
                                  TessCallback1<int>* cb) const {
  for (int i = 0; i < length_ && i < other.NumBlobs(); ++i) {
    TBOX blob_box = other.blobs[i]->bounding_box();
    if (blob_box == boxes_[i])
      cb->Run(i);
  }
  delete cb;
}

}  // namespace tesseract.
