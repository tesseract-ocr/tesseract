///////////////////////////////////////////////////////////////////////
// File:        tabvector.h
// Description: Class to hold a near-vertical vector representing a tab-stop.
// Author:      Ray Smith
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

#ifndef TESSERACT_TEXTORD_TABVECTOR_H_
#define TESSERACT_TEXTORD_TABVECTOR_H_

#include "bbgrid.h"
#include "blobgrid.h"
#include "clst.h"
#include "elst.h"
#include "elst2.h"
#include "rect.h"

#include <algorithm>

class BLOBNBOX;
class ScrollView;

namespace tesseract {

extern double_VAR_H(textord_tabvector_vertical_gap_fraction);
extern double_VAR_H(textord_tabvector_vertical_box_ratio);

// The alignment type that a tab vector represents.
// Keep this enum synced with kAlignmentNames in tabvector.cpp.
enum TabAlignment {
  TA_LEFT_ALIGNED,
  TA_LEFT_RAGGED,
  TA_CENTER_JUSTIFIED,
  TA_RIGHT_ALIGNED,
  TA_RIGHT_RAGGED,
  TA_SEPARATOR,
  TA_COUNT
};

// Forward declarations. The classes use their own list types, so we
// need to make the list types first.
class TabFind;
class TabVector;
class TabConstraint;

ELIST2IZEH(TabVector)
CLISTIZEH(TabVector)
ELISTIZEH(TabConstraint)

// TabConstraint is a totally self-contained class to maintain
// a list of [min,max] constraints, each referring to a TabVector.
// The constraints are manipulated through static methods that act
// on a list of constraints. The list itself is cooperatively owned
// by the TabVectors of the constraints on the list and managed
// by implicit reference counting via the elements of the list.
class TabConstraint : public ELIST<TabConstraint>::LINK {
public:
  // This empty constructor is here only so that the class can be ELISTIZED.
  // TODO(rays) change deep_copy in elst.h line 955 to take a callback copier
  // and eliminate CLASSNAME##_copier.
  TabConstraint() = default;

  // Create a constraint for the top or bottom of this TabVector.
  static void CreateConstraint(TabVector *vector, bool is_top);

  // Test to see if the constraints are compatible enough to merge.
  static bool CompatibleConstraints(TabConstraint_LIST *list1, TabConstraint_LIST *list2);

  // Merge the lists of constraints and update the TabVector pointers.
  // The second list is deleted.
  static void MergeConstraints(TabConstraint_LIST *list1, TabConstraint_LIST *list2);

  // Set all the tops and bottoms as appropriate to a mean of the
  // constrained range. Delete all the constraints and list.
  static void ApplyConstraints(TabConstraint_LIST *constraints);

private:
  TabConstraint(TabVector *vector, bool is_top);

  // Get the max of the mins and the min of the maxes.
  static void GetConstraints(TabConstraint_LIST *constraints, int *y_min, int *y_max);

  // The TabVector this constraint applies to.
  TabVector *vector_;
  // If true then we refer to the top of the vector_.
  bool is_top_;
  // The allowed range of this vector_.
  int y_min_;
  int y_max_;
};

// Class to hold information about a single vector
// that represents a tab stop or a rule line.
class TabVector : public ELIST2<TabVector>::LINK {
public:
  // TODO(rays) fix this in elst.h line 1076, where it should use the
  // copy constructor instead of operator=.
  TabVector() = default;
  ~TabVector() = default;

  // Public factory to build a TabVector from a list of boxes.
  // The TabVector will be of the given alignment type.
  // The input vertical vector is used in fitting, and the output
  // vertical_x, vertical_y have the resulting line vector added to them
  // if the alignment is not ragged.
  // The extended_start_y and extended_end_y are the maximum possible
  // extension to the line segment that can be used to align with others.
  // The input CLIST of BLOBNBOX good_points is consumed and taken over.
  static TabVector *FitVector(TabAlignment alignment, ICOORD vertical, int extended_start_y,
                              int extended_end_y, BLOBNBOX_CLIST *good_points, int *vertical_x,
                              int *vertical_y);

  // Build a ragged TabVector by copying another's direction, shifting it
  // to match the given blob, and making its initial extent the height
  // of the blob, but its extended bounds from the bounds of the original.
  TabVector(const TabVector &src, TabAlignment alignment, const ICOORD &vertical_skew,
            BLOBNBOX *blob);

  // Copies basic attributes of a tab vector for simple operations.
  // Copies things such startpt, endpt, range, width.
  // Does not copy things such as partners, boxes, or constraints.
  // This is useful if you only need vector information for processing, such
  // as in the table detection code.
  TabVector *ShallowCopy() const;

  // Simple accessors.
  const ICOORD &startpt() const {
    return startpt_;
  }
  const ICOORD &endpt() const {
    return endpt_;
  }
  int extended_ymax() const {
    return extended_ymax_;
  }
  int extended_ymin() const {
    return extended_ymin_;
  }
  int sort_key() const {
    return sort_key_;
  }
  int mean_width() const {
    return mean_width_;
  }
  void set_top_constraints(TabConstraint_LIST *constraints) {
    top_constraints_ = constraints;
  }
  void set_bottom_constraints(TabConstraint_LIST *constraints) {
    bottom_constraints_ = constraints;
  }
  TabVector_CLIST *partners() {
    return &partners_;
  }
  void set_startpt(const ICOORD &start) {
    startpt_ = start;
  }
  void set_endpt(const ICOORD &end) {
    endpt_ = end;
  }
  bool intersects_other_lines() const {
    return intersects_other_lines_;
  }
  void set_intersects_other_lines(bool value) {
    intersects_other_lines_ = value;
  }

  // Inline quasi-accessors that require some computation.

  // Compute the x coordinate at the given y coordinate.
  int XAtY(int y) const {
    int height = endpt_.y() - startpt_.y();
    if (height != 0) {
      return (y - startpt_.y()) * (endpt_.x() - startpt_.x()) / height + startpt_.x();
    } else {
      return startpt_.x();
    }
  }

  // Compute the vertical overlap with the other TabVector.
  int VOverlap(const TabVector &other) const {
    return std::min(other.endpt_.y(), endpt_.y()) - std::max(other.startpt_.y(), startpt_.y());
  }
  // Compute the vertical overlap with the given y bounds.
  int VOverlap(int top_y, int bottom_y) const {
    return std::min(top_y, static_cast<int>(endpt_.y())) -
           std::max(bottom_y, static_cast<int>(startpt_.y()));
  }
  // Compute the extended vertical overlap with the given y bounds.
  int ExtendedOverlap(int top_y, int bottom_y) const {
    return std::min(top_y, extended_ymax_) - std::max(bottom_y, extended_ymin_);
  }

  // Return true if this is a left tab stop, either aligned, or ragged.
  bool IsLeftTab() const {
    return alignment_ == TA_LEFT_ALIGNED || alignment_ == TA_LEFT_RAGGED;
  }
  // Return true if this is a right tab stop, either aligned, or ragged.
  bool IsRightTab() const {
    return alignment_ == TA_RIGHT_ALIGNED || alignment_ == TA_RIGHT_RAGGED;
  }
  // Return true if this is a separator.
  bool IsSeparator() const {
    return alignment_ == TA_SEPARATOR;
  }
  // Return true if this is a center aligned tab stop.
  bool IsCenterTab() const {
    return alignment_ == TA_CENTER_JUSTIFIED;
  }
  // Return true if this is a ragged tab top, either left or right.
  bool IsRagged() const {
    return alignment_ == TA_LEFT_RAGGED || alignment_ == TA_RIGHT_RAGGED;
  }

  // Return true if this vector is to the left of the other in terms
  // of sort_key_.
  bool IsLeftOf(const TabVector &other) const {
    return sort_key_ < other.sort_key_;
  }

  // Return true if the vector has no partners.
  bool Partnerless() {
    return partners_.empty();
  }

  // Return the number of tab boxes in this vector.
  int BoxCount() {
    return boxes_.length();
  }

  // Lock the vector from refits by clearing the boxes_ list.
  void Freeze() {
    boxes_.shallow_clear();
  }

  // Flip x and y on the ends so a vector can be created from flipped input.
  void XYFlip() {
    int x = startpt_.y();
    startpt_.set_y(startpt_.x());
    startpt_.set_x(x);
    x = endpt_.y();
    endpt_.set_y(endpt_.x());
    endpt_.set_x(x);
  }

  // Reflect the tab vector in the y-axis.
  void ReflectInYAxis() {
    startpt_.set_x(-startpt_.x());
    endpt_.set_x(-endpt_.x());
    sort_key_ = -sort_key_;
    if (alignment_ == TA_LEFT_ALIGNED) {
      alignment_ = TA_RIGHT_ALIGNED;
    } else if (alignment_ == TA_RIGHT_ALIGNED) {
      alignment_ = TA_LEFT_ALIGNED;
    }
    if (alignment_ == TA_LEFT_RAGGED) {
      alignment_ = TA_RIGHT_RAGGED;
    } else if (alignment_ == TA_RIGHT_RAGGED) {
      alignment_ = TA_LEFT_RAGGED;
    }
  }

  // Separate function to compute the sort key for a given coordinate pair.
  static int SortKey(const ICOORD &vertical, int x, int y) {
    ICOORD pt(x, y);
    return pt * vertical;
  }

  // Return the x at the given y for the given sort key.
  static int XAtY(const ICOORD &vertical, int sort_key, int y) {
    if (vertical.y() != 0) {
      return (vertical.x() * y + sort_key) / vertical.y();
    } else {
      return sort_key;
    }
  }

  // Sort function for E2LIST::sort to sort by sort_key_.
  static int SortVectorsByKey(const TabVector *tv1, const TabVector *tv2) {
    return tv1->sort_key_ - tv2->sort_key_;
  }

  // More complex members.

  // Extend this vector to include the supplied blob if it doesn't
  // already have it.
  void ExtendToBox(BLOBNBOX *blob);

  // Set the ycoord of the start and move the xcoord to match.
  void SetYStart(int start_y);
  // Set the ycoord of the end and move the xcoord to match.
  void SetYEnd(int end_y);

  // Rotate the ends by the given vector.
  void Rotate(const FCOORD &rotation);

  // Setup the initial constraints, being the limits of
  // the vector and the extended ends.
  void SetupConstraints();

  // Setup the constraints between the partners of this TabVector.
  void SetupPartnerConstraints();

  // Setup the constraints between this and its partner.
  void SetupPartnerConstraints(TabVector *partner);

  // Use the constraints to modify the top and bottom.
  void ApplyConstraints();

  // Merge close tab vectors of the same side that overlap.
  static void MergeSimilarTabVectors(const ICOORD &vertical, TabVector_LIST *vectors,
                                     BlobGrid *grid);

  // Return true if this vector is the same side, overlaps, and close
  // enough to the other to be merged.
  bool SimilarTo(const ICOORD &vertical, const TabVector &other, BlobGrid *grid) const;

  // Eat the other TabVector into this and delete it.
  void MergeWith(const ICOORD &vertical, TabVector *other);

  // Add a new element to the list of partner TabVectors.
  // Partners must be added in order of increasing y coordinate of the text line
  // that makes them partners.
  // Groups of identical partners are merged into one.
  void AddPartner(TabVector *partner);

  // Return true if other is a partner of this.
  bool IsAPartner(const TabVector *other);

  // Print basic information about this tab vector.
  void Print(const char *prefix);

  // Print basic information about this tab vector and every box in it.
  void Debug(const char *prefix);

  // Draw this tabvector in place in the given window.
  void Display(ScrollView *tab_win);

  // Refit the line and/or re-evaluate the vector if the dirty flags are set.
  void FitAndEvaluateIfNeeded(const ICOORD &vertical, TabFind *finder);

  // Evaluate the vector in terms of coverage of its length by good-looking
  // box edges. A good looking box is one where its nearest neighbour on the
  // inside is nearer than half the distance its nearest neighbour on the
  // outside of the putative column. Bad boxes are removed from the line.
  // A second pass then further filters boxes by requiring that the gutter
  // width be a minimum fraction of the mean gutter along the line.
  void Evaluate(const ICOORD &vertical, TabFind *finder);

  // (Re)Fit a line to the stored points. Returns false if the line
  // is degenerate. Although the TabVector code mostly doesn't care about the
  // direction of lines, XAtY would give silly results for a horizontal line.
  // The class is mostly aimed at use for vertical lines representing
  // horizontal tab stops.
  bool Fit(ICOORD vertical, bool force_parallel);

  // Return the partner of this TabVector if the vector qualifies as
  // being a vertical text line, otherwise nullptr.
  TabVector *VerticalTextlinePartner();

  // Return the matching tabvector if there is exactly one partner, or
  // nullptr otherwise.  This can be used after matching is done, eg. by
  // VerticalTextlinePartner(), without checking if the line is vertical.
  TabVector *GetSinglePartner();

private:
  // Constructor is private as the static factory is the external way
  // to build a TabVector.
  TabVector(int extended_ymin, int extended_ymax, TabAlignment alignment, BLOBNBOX_CLIST *boxes);

  // Delete this, but first, repoint all the partners to point to
  // replacement. If replacement is nullptr, then partner relationships
  // are removed.
  void Delete(TabVector *replacement);

private:
  // The bottom of the tab line.
  ICOORD startpt_;
  // The top of the tab line.
  ICOORD endpt_;
  // The lowest y that the vector might extend to.
  int extended_ymin_ = 0;
  // The highest y that the vector might extend to.
  int extended_ymax_ = 0;
  // Perpendicular distance of vector from a given vertical for sorting.
  int sort_key_ = 0;
  // Result of Evaluate 0-100. Coverage of line with good boxes.
  int percent_score_ = 0;
  // The mean width of the blobs. Meaningful only for separator lines.
  int mean_width_ = 0;
  // True if the boxes_ list has been modified, so a refit is needed.
  bool needs_refit_ = false;
  // True if a fit has been done, so re-evaluation is needed.
  bool needs_evaluation_ = false;
  // True if a separator line intersects at least 2 other lines.
  bool intersects_other_lines_ = false;
  // The type of this TabVector.
  TabAlignment alignment_ = TA_LEFT_ALIGNED;
  // The list of boxes whose edges are aligned at this TabVector.
  BLOBNBOX_CLIST boxes_;
  // List of TabVectors that have a connection with this via a text line.
  TabVector_CLIST partners_;
  // Constraints used to resolve the exact location of the top and bottom
  // of the tab line.
  TabConstraint_LIST *top_constraints_ = nullptr;
  TabConstraint_LIST *bottom_constraints_ = nullptr;
};

} // namespace tesseract.

#endif // TESSERACT_TEXTORD_TABVECTOR_H_
