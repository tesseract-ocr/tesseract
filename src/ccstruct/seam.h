/* -*-C-*-
 ********************************************************************************
 *
 * File:        seam.h
 * Author:      Mark Seaman, SW Productivity
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
#ifndef SEAM_H
#define SEAM_H

// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif

/*----------------------------------------------------------------------
              I n c l u d e s
----------------------------------------------------------------------*/
#include "blobs.h"
#include "split.h"

/*----------------------------------------------------------------------
              T y p e s
----------------------------------------------------------------------*/
using PRIORITY = float;          /*  PRIORITY  */

class SEAM {
 public:
  // A seam with no splits
  SEAM(float priority, const TPOINT& location)
      : priority_(priority),
        location_(location),
        widthp_(0),
        widthn_(0),
        num_splits_(0) {}
  // A seam with a single split point.
  SEAM(float priority, const TPOINT& location, const SPLIT& split)
      : priority_(priority),
        location_(location),
        widthp_(0),
        widthn_(0),
        num_splits_(1) {
    splits_[0] = split;
  }
  // Default copy constructor, operator= and destructor are OK!

  // Accessors.
  float priority() const { return priority_; }
  void set_priority(float priority) { priority_ = priority; }
  bool HasAnySplits() const { return num_splits_ > 0; }

  // Returns the bounding box of all the points in the seam.
  TBOX bounding_box() const;

  // Returns true if other can be combined into *this.
  bool CombineableWith(const SEAM& other, int max_x_dist,
                       float max_total_priority) const;
  // Combines other into *this. Only works if CombinableWith returned true.
  void CombineWith(const SEAM& other);

  // Returns true if the given blob contains all splits of *this SEAM.
  bool ContainedByBlob(const TBLOB& blob) const {
    for (int s = 0; s < num_splits_; ++s) {
      if (!splits_[s].ContainedByBlob(blob)) return false;
    }
    return true;
  }

  // Returns true if the given EDGEPT is used by this SEAM, checking only
  // the EDGEPT pointer, not the coordinates.
  bool UsesPoint(const EDGEPT* point) const {
    for (int s = 0; s < num_splits_; ++s) {
      if (splits_[s].UsesPoint(point)) return true;
    }
    return false;
  }
  // Returns true if *this and other share any common point, by coordinates.
  bool SharesPosition(const SEAM& other) const {
    for (int s = 0; s < num_splits_; ++s) {
      for (int t = 0; t < other.num_splits_; ++t)
        if (splits_[s].SharesPosition(other.splits_[t])) return true;
    }
    return false;
  }
  // Returns true if *this and other have any vertically overlapping splits.
  bool OverlappingSplits(const SEAM& other) const {
    for (int s = 0; s < num_splits_; ++s) {
      TBOX split1_box = splits_[s].bounding_box();
      for (int t = 0; t < other.num_splits_; ++t) {
        TBOX split2_box = other.splits_[t].bounding_box();
        if (split1_box.y_overlap(split2_box)) return true;
      }
    }
    return false;
  }

  // Marks the edgepts used by the seam so the segments made by the cut
  // never get split further by another seam in the future.
  void Finalize() {
    for (int s = 0; s < num_splits_; ++s) {
      splits_[s].point1->MarkChop();
      splits_[s].point2->MarkChop();
    }
  }

  // Returns true if the splits in *this SEAM appear OK in the sense that they
  // do not cross any outlines and do not chop off any ridiculously small
  // pieces.
  bool IsHealthy(const TBLOB& blob, int min_points, int min_area) const;

  // Computes the widthp_/widthn_ range for all existing SEAMs and for *this
  // seam, which is about to be inserted at insert_index. Returns false if
  // any of the computations fails, as this indicates an invalid chop.
  // widthn_/widthp_ are only changed if modify is true.
  bool PrepareToInsertSeam(const GenericVector<SEAM*>& seams,
                           const GenericVector<TBLOB*>& blobs, int insert_index,
                           bool modify);
  // Computes the widthp_/widthn_ range. Returns false if not all the splits
  // are accounted for. widthn_/widthp_ are only changed if modify is true.
  bool FindBlobWidth(const GenericVector<TBLOB*>& blobs, int index,
                     bool modify);

  // Splits this blob into two blobs by applying the splits included in
  // *this SEAM
  void ApplySeam(bool italic_blob, TBLOB* blob, TBLOB* other_blob) const;
  // Undoes ApplySeam by removing the seam between these two blobs.
  // Produces one blob as a result, and deletes other_blob.
  void UndoSeam(TBLOB* blob, TBLOB* other_blob) const;

  // Prints everything in *this SEAM.
  void Print(const char* label) const;
  // Prints a collection of SEAMs.
  static void PrintSeams(const char* label, const GenericVector<SEAM*>& seams);
#ifndef GRAPHICS_DISABLED
  // Draws the seam in the given window.
  void Mark(ScrollView* window) const;
#endif

  // Break up the blobs in this chain so that they are all independent.
  // This operation should undo the affect of join_pieces.
  static void BreakPieces(const GenericVector<SEAM*>& seams,
                          const GenericVector<TBLOB*>& blobs, int first,
                          int last);
  // Join a group of base level pieces into a single blob that can then
  // be classified.
  static void JoinPieces(const GenericVector<SEAM*>& seams,
                         const GenericVector<TBLOB*>& blobs, int first,
                         int last);

  // Hides the seam so the outlines appear not to be cut by it.
  void Hide() const;
  // Undoes hide, so the outlines are cut by the seam.
  void Reveal() const;

  // Computes and returns, but does not set, the full priority of *this SEAM.
  // The arguments here are config parameters defined in Wordrec. Add chop_
  // to the beginning of the name.
  float FullPriority(int xmin, int xmax, double overlap_knob,
                     int centered_maxwidth, double center_knob,
                     double width_change_knob) const;

 private:
  // Maximum number of splits that a SEAM can hold.
  static const uint8_t kMaxNumSplits = 3;
  // Priority of this split. Lower is better.
  float priority_;
  // Position of the middle of the seam.
  TPOINT location_;
  // A range such that all splits in *this SEAM are contained within blobs in
  // the range [index - widthn_,index + widthp_] where index is the index of
  // this SEAM in the seams vector.
  int8_t widthp_;
  int8_t widthn_;
  // Number of splits_ that are used.
  uint8_t num_splits_;
  // Set of pairs of points that are the ends of each split in the SEAM.
  SPLIT splits_[kMaxNumSplits];
};

/*----------------------------------------------------------------------
              F u n c t i o n s
----------------------------------------------------------------------*/

void start_seam_list(TWERD* word, GenericVector<SEAM*>* seam_array);

#endif
