///////////////////////////////////////////////////////////////////////
// File:        boxword.h
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

#ifndef TESSERACT_CSTRUCT_BOXWORD_H_
#define TESSERACT_CSTRUCT_BOXWORD_H_

#include "rect.h" // for TBOX

#include <functional> // for std::function

namespace tesseract {

class BLOCK;
class WERD;
struct TWERD;

// Class to hold an array of bounding boxes for an output word and
// the bounding box of the whole word.
class BoxWord {
public:
  BoxWord();
  explicit BoxWord(const BoxWord &src);
  ~BoxWord() = default;

  BoxWord &operator=(const BoxWord &src);

  void CopyFrom(const BoxWord &src);

  // Factory to build a BoxWord from a TWERD using the DENORMs on each blob to
  // switch back to original image coordinates.
  static BoxWord *CopyFromNormalized(TWERD *tessword);

  // Clean up the bounding boxes from the polygonal approximation by
  // expanding slightly, then clipping to the blobs from the original_word
  // that overlap. If not null, the block provides the inverse rotation.
  void ClipToOriginalWord(const BLOCK *block, WERD *original_word);

  // Merges the boxes from start to end, not including end, and deletes
  // the boxes between start and end.
  void MergeBoxes(unsigned start, unsigned end);

  // Inserts a new box before the given index.
  // Recomputes the bounding box.
  void InsertBox(unsigned index, const TBOX &box);

  // Changes the box at the given index to the new box.
  // Recomputes the bounding box.
  void ChangeBox(unsigned index, const TBOX &box);

  // Deletes the box with the given index, and shuffles up the rest.
  // Recomputes the bounding box.
  void DeleteBox(unsigned index);

  // Deletes all the boxes stored in BoxWord.
  void DeleteAllBoxes();

  // This and other putatively are the same, so call the (permanent) callback
  // for each blob index where the bounding boxes match.
  // The callback is deleted on completion.
  void ProcessMatchedBlobs(const TWERD &other,
                           const std::function<void(int)> &cb) const;

  const TBOX &bounding_box() const {
    return bbox_;
  }
  unsigned length() const {
    return length_;
  }
  const TBOX &BlobBox(unsigned index) const {
    return boxes_[index];
  }

private:
  void ComputeBoundingBox();

  TBOX bbox_;
  unsigned length_;
  std::vector<TBOX> boxes_;
};

} // namespace tesseract.

#endif // TESSERACT_CSTRUCT_BOXWORD_H_
