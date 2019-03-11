/* -*-C-*-
 ********************************************************************************
 *
 * File:         seam.cpp  (Formerly seam.c)
 * Author:       Mark Seaman, OCR Technology
 *
 * (c) Copyright 1987, Hewlett-Packard Company.
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
 *********************************************************************************/
/*----------------------------------------------------------------------
              I n c l u d e s
----------------------------------------------------------------------*/
#include "seam.h"
#include "blobs.h"
#include "tprintf.h"

/*----------------------------------------------------------------------
        Public Function Code
----------------------------------------------------------------------*/

// Returns the bounding box of all the points in the seam.
TBOX SEAM::bounding_box() const {
  TBOX box(location_.x, location_.y, location_.x, location_.y);
  for (int s = 0; s < num_splits_; ++s) {
    box += splits_[s].bounding_box();
  }
  return box;
}

// Returns true if other can be combined into *this.
bool SEAM::CombineableWith(const SEAM& other, int max_x_dist,
                           float max_total_priority) const {
  int dist = location_.x - other.location_.x;
  if (-max_x_dist < dist && dist < max_x_dist &&
      num_splits_ + other.num_splits_ <= kMaxNumSplits &&
      priority_ + other.priority_ < max_total_priority &&
      !OverlappingSplits(other) && !SharesPosition(other)) {
    return true;
  } else {
    return false;
  }
}

// Combines other into *this. Only works if CombinableWith returned true.
void SEAM::CombineWith(const SEAM& other) {
  priority_ += other.priority_;
  location_ += other.location_;
  location_ /= 2;

  for (uint8_t s = 0; s < other.num_splits_ && num_splits_ < kMaxNumSplits; ++s)
    splits_[num_splits_++] = other.splits_[s];
}

// Returns true if the splits in *this SEAM appear OK in the sense that they
// do not cross any outlines and do not chop off any ridiculously small
// pieces.
bool SEAM::IsHealthy(const TBLOB& blob, int min_points, int min_area) const {
  // TODO(rays) Try testing all the splits. Duplicating original code for now,
  // which tested only the first.
  return num_splits_ == 0 || splits_[0].IsHealthy(blob, min_points, min_area);
}

// Computes the widthp_/widthn_ range for all existing SEAMs and for *this
// seam, which is about to be inserted at insert_index. Returns false if
// any of the computations fails, as this indicates an invalid chop.
// widthn_/widthp_ are only changed if modify is true.
bool SEAM::PrepareToInsertSeam(const GenericVector<SEAM*>& seams,
                               const GenericVector<TBLOB*>& blobs,
                               int insert_index, bool modify) {
  for (int s = 0; s < insert_index; ++s) {
    if (!seams[s]->FindBlobWidth(blobs, s, modify)) return false;
  }
  if (!FindBlobWidth(blobs, insert_index, modify)) return false;
  for (int s = insert_index; s < seams.size(); ++s) {
    if (!seams[s]->FindBlobWidth(blobs, s + 1, modify)) return false;
  }
  return true;
}

// Computes the widthp_/widthn_ range. Returns false if not all the splits
// are accounted for. widthn_/widthp_ are only changed if modify is true.
bool SEAM::FindBlobWidth(const GenericVector<TBLOB*>& blobs, int index,
                         bool modify) {
  int num_found = 0;
  if (modify) {
    widthp_ = 0;
    widthn_ = 0;
  }
  for (int s = 0; s < num_splits_; ++s) {
    const SPLIT& split = splits_[s];
    bool found_split = split.ContainedByBlob(*blobs[index]);
    // Look right.
    for (int b = index + 1; !found_split && b < blobs.size(); ++b) {
      found_split = split.ContainedByBlob(*blobs[b]);
      if (found_split && b - index > widthp_ && modify) widthp_ = b - index;
    }
    // Look left.
    for (int b = index - 1; !found_split && b >= 0; --b) {
      found_split = split.ContainedByBlob(*blobs[b]);
      if (found_split && index - b > widthn_ && modify) widthn_ = index - b;
    }
    if (found_split) ++num_found;
  }
  return num_found == num_splits_;
}

// Splits this blob into two blobs by applying the splits included in
// *this SEAM
void SEAM::ApplySeam(bool italic_blob, TBLOB* blob, TBLOB* other_blob) const {
  for (int s = 0; s < num_splits_; ++s) {
    splits_[s].SplitOutlineList(blob->outlines);
  }
  blob->ComputeBoundingBoxes();

  divide_blobs(blob, other_blob, italic_blob, location_);

  blob->EliminateDuplicateOutlines();
  other_blob->EliminateDuplicateOutlines();

  blob->CorrectBlobOrder(other_blob);
}

// Undoes ApplySeam by removing the seam between these two blobs.
// Produces one blob as a result, and deletes other_blob.
void SEAM::UndoSeam(TBLOB* blob, TBLOB* other_blob) const {
  if (blob->outlines == nullptr) {
    blob->outlines = other_blob->outlines;
    other_blob->outlines = nullptr;
  }

  TESSLINE* outline = blob->outlines;
  while (outline->next) outline = outline->next;
  outline->next = other_blob->outlines;
  other_blob->outlines = nullptr;
  delete other_blob;

  for (int s = 0; s < num_splits_; ++s) {
    splits_[s].UnsplitOutlineList(blob);
  }
  blob->ComputeBoundingBoxes();
  blob->EliminateDuplicateOutlines();
}

// Prints everything in *this SEAM.
void SEAM::Print(const char* label) const {
  tprintf(label);
  tprintf(" %6.2f @ (%d,%d), p=%d, n=%d ", priority_, location_.x, location_.y,
          widthp_, widthn_);
  for (int s = 0; s < num_splits_; ++s) {
    splits_[s].Print();
    if (s + 1 < num_splits_) tprintf(",   ");
  }
  tprintf("\n");
}

// Prints a collection of SEAMs.
/* static */
void SEAM::PrintSeams(const char* label, const GenericVector<SEAM*>& seams) {
  if (!seams.empty()) {
    tprintf("%s\n", label);
    for (int x = 0; x < seams.size(); ++x) {
      tprintf("%2d:   ", x);
      seams[x]->Print("");
    }
    tprintf("\n");
  }
}

#ifndef GRAPHICS_DISABLED
// Draws the seam in the given window.
void SEAM::Mark(ScrollView* window) const {
  for (int s = 0; s < num_splits_; ++s) splits_[s].Mark(window);
}
#endif

// Break up the blobs in this chain so that they are all independent.
// This operation should undo the affect of join_pieces.
/* static */
void SEAM::BreakPieces(const GenericVector<SEAM*>& seams,
                       const GenericVector<TBLOB*>& blobs, int first,
                       int last) {
  for (int x = first; x < last; ++x) seams[x]->Reveal();

  TESSLINE* outline = blobs[first]->outlines;
  int next_blob = first + 1;

  while (outline != nullptr && next_blob <= last) {
    if (outline->next == blobs[next_blob]->outlines) {
      outline->next = nullptr;
      outline = blobs[next_blob]->outlines;
      ++next_blob;
    } else {
      outline = outline->next;
    }
  }
}

// Join a group of base level pieces into a single blob that can then
// be classified.
/* static */
void SEAM::JoinPieces(const GenericVector<SEAM*>& seams,
                      const GenericVector<TBLOB*>& blobs, int first, int last) {
  TESSLINE* outline = blobs[first]->outlines;
  if (!outline)
    return;

  for (int x = first; x < last; ++x) {
    SEAM *seam = seams[x];
    if (x - seam->widthn_ >= first && x + seam->widthp_ < last) seam->Hide();
    while (outline->next) outline = outline->next;
    outline->next = blobs[x + 1]->outlines;
  }
}

// Hides the seam so the outlines appear not to be cut by it.
void SEAM::Hide() const {
  for (int s = 0; s < num_splits_; ++s) {
    splits_[s].Hide();
  }
}

// Undoes hide, so the outlines are cut by the seam.
void SEAM::Reveal() const {
  for (int s = 0; s < num_splits_; ++s) {
    splits_[s].Reveal();
  }
}

// Computes and returns, but does not set, the full priority of *this SEAM.
float SEAM::FullPriority(int xmin, int xmax, double overlap_knob,
                         int centered_maxwidth, double center_knob,
                         double width_change_knob) const {
  if (num_splits_ == 0) return 0.0f;
  for (int s = 1; s < num_splits_; ++s) {
    splits_[s].SplitOutline();
  }
  float full_priority =
      priority_ +
      splits_[0].FullPriority(xmin, xmax, overlap_knob, centered_maxwidth,
                              center_knob, width_change_knob);
  for (int s = num_splits_ - 1; s >= 1; --s) {
    splits_[s].UnsplitOutlines();
  }
  return full_priority;
}

/**
 * @name start_seam_list
 *
 * Initialize a list of seams that match the original number of blobs
 * present in the starting segmentation.  Each of the seams created
 * by this routine have location information only.
 */
void start_seam_list(TWERD* word, GenericVector<SEAM*>* seam_array) {
  seam_array->truncate(0);
  TPOINT location;

  for (int b = 1; b < word->NumBlobs(); ++b) {
    TBOX bbox = word->blobs[b - 1]->bounding_box();
    TBOX nbox = word->blobs[b]->bounding_box();
    location.x = (bbox.right() + nbox.left()) / 2;
    location.y = (bbox.bottom() + bbox.top() + nbox.bottom() + nbox.top()) / 4;
    seam_array->push_back(new SEAM(0.0f, location));
  }
}
