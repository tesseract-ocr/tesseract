///////////////////////////////////////////////////////////////////////
// File:        tabvector.cpp
// Description: Class to hold a near-vertical vector representing a tab-stop.
// Author:      Ray Smith
// Created:     Thu Apr 10 16:28:01 PST 2008
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

#ifdef _MSC_VER
#pragma warning(disable:4244)  // Conversion warnings
#endif

#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif

#include "tabvector.h"
#include "blobbox.h"
#include "colfind.h"
#include "colpartitionset.h"
#include "detlinefit.h"
#include "statistc.h"

namespace tesseract {

// Multiple of height used as a gutter for evaluation search.
const int kGutterMultiple = 4;
// Multiple of neighbour gap that we expect the gutter gap to be at minimum.
const int kGutterToNeighbourRatio = 3;
// Pixel distance for tab vectors to be considered the same.
const int kSimilarVectorDist = 10;
// Pixel distance for ragged tab vectors to be considered the same if there
// is nothing in the overlap box
const int kSimilarRaggedDist = 50;
// Max multiple of height to allow filling in between blobs when evaluating.
const int kMaxFillinMultiple = 11;
// Min fraction of mean gutter size to allow a gutter on a good tab blob.
const double kMinGutterFraction = 0.5;
// Multiple of 1/n lines as a minimum gutter in evaluation.
const double kLineCountReciprocal = 4.0;
// Constant add-on for minimum gutter for aligned tabs.
const double kMinAlignedGutter = 0.25;
// Constant add-on for minimum gutter for ragged tabs.
const double kMinRaggedGutter = 1.5;

double_VAR(textord_tabvector_vertical_gap_fraction, 0.5,
  "max fraction of mean blob width allowed for vertical gaps in vertical text");

double_VAR(textord_tabvector_vertical_box_ratio, 0.5,
  "Fraction of box matches required to declare a line vertical");

ELISTIZE(TabConstraint)

// Create a constraint for the top or bottom of this TabVector.
void TabConstraint::CreateConstraint(TabVector* vector, bool is_top) {
  TabConstraint* constraint = new TabConstraint(vector, is_top);
  TabConstraint_LIST* constraints = new TabConstraint_LIST;
  TabConstraint_IT it(constraints);
  it.add_to_end(constraint);
  if (is_top)
    vector->set_top_constraints(constraints);
  else
    vector->set_bottom_constraints(constraints);
}

// Test to see if the constraints are compatible enough to merge.
bool TabConstraint::CompatibleConstraints(TabConstraint_LIST* list1,
                                          TabConstraint_LIST* list2) {
  if (list1 == list2)
    return false;
  int y_min = -MAX_INT32;
  int y_max = MAX_INT32;
  if (textord_debug_tabfind > 3)
    tprintf("Testing constraint compatibility\n");
  GetConstraints(list1, &y_min, &y_max);
  GetConstraints(list2, &y_min, &y_max);
  if (textord_debug_tabfind > 3)
    tprintf("Resulting range = [%d,%d]\n", y_min, y_max);
  return y_max >= y_min;
}

// Merge the lists of constraints and update the TabVector pointers.
// The second list is deleted.
void TabConstraint::MergeConstraints(TabConstraint_LIST* list1,
                                     TabConstraint_LIST* list2) {
  if (list1 == list2)
    return;
  TabConstraint_IT it(list2);
  if (textord_debug_tabfind > 3)
    tprintf("Merging constraints\n");
  // The vectors of all constraints on list2 are now going to be on list1.
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    TabConstraint* constraint = it.data();
    if (textord_debug_tabfind> 3)
      constraint->vector_->Print("Merge");
    if (constraint->is_top_)
      constraint->vector_->set_top_constraints(list1);
    else
      constraint->vector_->set_bottom_constraints(list1);
  }
  it = list1;
  it.add_list_before(list2);
  delete list2;
}

// Set all the tops and bottoms as appropriate to a mean of the
// constrained range. Delete all the constraints and list.
void TabConstraint::ApplyConstraints(TabConstraint_LIST* constraints) {
  int y_min = -MAX_INT32;
  int y_max = MAX_INT32;
  GetConstraints(constraints, &y_min, &y_max);
  int y = (y_min + y_max) / 2;
  TabConstraint_IT it(constraints);
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    TabConstraint* constraint = it.data();
    TabVector* v = constraint->vector_;
    if (constraint->is_top_) {
      v->SetYEnd(y);
      v->set_top_constraints(NULL);
    } else {
      v->SetYStart(y);
      v->set_bottom_constraints(NULL);
    }
  }
  delete constraints;
}

TabConstraint::TabConstraint(TabVector* vector, bool is_top)
  : vector_(vector), is_top_(is_top) {
  if (is_top) {
    y_min_ = vector->endpt().y();
    y_max_ = vector->extended_ymax();
  } else {
    y_max_ = vector->startpt().y();
    y_min_ = vector->extended_ymin();
  }
}

// Get the max of the mins and the min of the maxes.
void TabConstraint::GetConstraints(TabConstraint_LIST* constraints,
                                   int* y_min, int* y_max) {
  TabConstraint_IT it(constraints);
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    TabConstraint* constraint = it.data();
    if (textord_debug_tabfind > 3) {
      tprintf("Constraint is [%d,%d]", constraint->y_min_, constraint->y_max_);
      constraint->vector_->Print(" for");
    }
    *y_min = MAX(*y_min, constraint->y_min_);
    *y_max = MIN(*y_max, constraint->y_max_);
  }
}

ELIST2IZE(TabVector)
CLISTIZE(TabVector)

// The constructor is private. See the bottom of the file...

TabVector::~TabVector() {
}


// Public factory to build a TabVector from a list of boxes.
// The TabVector will be of the given alignment type.
// The input vertical vector is used in fitting, and the output
// vertical_x, vertical_y have the resulting line vector added to them
// if the alignment is not ragged.
// The extended_start_y and extended_end_y are the maximum possible
// extension to the line segment that can be used to align with others.
// The input CLIST of BLOBNBOX good_points is consumed and taken over.
TabVector* TabVector::FitVector(TabAlignment alignment, ICOORD vertical,
                                int  extended_start_y, int extended_end_y,
                                BLOBNBOX_CLIST* good_points,
                                int* vertical_x, int* vertical_y) {
  TabVector* vector = new TabVector(extended_start_y, extended_end_y,
                                    alignment, good_points);
  if (!vector->Fit(vertical, false)) {
    delete vector;
    return NULL;
  }
  if (!vector->IsRagged()) {
    vertical = vector->endpt_ - vector->startpt_;
    int weight = vector->BoxCount();
    *vertical_x += vertical.x() * weight;
    *vertical_y += vertical.y() * weight;
  }
  return vector;
}

// Build a ragged TabVector by copying another's direction, shifting it
// to match the given blob, and making its initial extent the height
// of the blob, but its extended bounds from the bounds of the original.
TabVector::TabVector(const TabVector& src, TabAlignment alignment,
                     const ICOORD& vertical_skew, BLOBNBOX* blob)
  : extended_ymin_(src.extended_ymin_), extended_ymax_(src.extended_ymax_),
    sort_key_(0), percent_score_(0), mean_width_(0),
    needs_refit_(true), needs_evaluation_(true), intersects_other_lines_(false),
    alignment_(alignment),
    top_constraints_(NULL), bottom_constraints_(NULL) {
  BLOBNBOX_C_IT it(&boxes_);
  it.add_to_end(blob);
  TBOX box = blob->bounding_box();
  if (IsLeftTab()) {
    startpt_ = box.botleft();
    endpt_ = box.topleft();
  } else {
    startpt_ = box.botright();
    endpt_ = box.topright();
  }
  sort_key_ = SortKey(vertical_skew,
                      (startpt_.x() + endpt_.x()) / 2,
                      (startpt_.y() + endpt_.y()) / 2);
  if (textord_debug_tabfind > 3)
    Print("Constructed a new tab vector:");
}

// Copies basic attributes of a tab vector for simple operations.
// Copies things such startpt, endpt, range.
// Does not copy things such as partners, boxes, or constraints.
// This is useful if you only need vector information for processing, such
// as in the table detection code.
TabVector* TabVector::ShallowCopy() const {
  TabVector* copy = new TabVector();
  copy->startpt_ = startpt_;
  copy->endpt_ = endpt_;
  copy->alignment_ = alignment_;
  copy->extended_ymax_ = extended_ymax_;
  copy->extended_ymin_ = extended_ymin_;
  copy->intersects_other_lines_ = intersects_other_lines_;
  return copy;
}

// Extend this vector to include the supplied blob if it doesn't
// already have it.
void TabVector::ExtendToBox(BLOBNBOX* new_blob) {
  TBOX new_box = new_blob->bounding_box();
  BLOBNBOX_C_IT it(&boxes_);
  if (!it.empty()) {
    BLOBNBOX* blob = it.data();
    TBOX box = blob->bounding_box();
    while (!it.at_last() && box.top() <= new_box.top()) {
      if (blob == new_blob)
        return;  // We have it already.
      it.forward();
      blob = it.data();
      box = blob->bounding_box();
    }
    if (box.top() >= new_box.top()) {
      it.add_before_stay_put(new_blob);
      needs_refit_ = true;
      return;
    }
  }
  needs_refit_ = true;
  it.add_after_stay_put(new_blob);
}

// Set the ycoord of the start and move the xcoord to match.
void TabVector::SetYStart(int start_y) {
  startpt_.set_x(XAtY(start_y));
  startpt_.set_y(start_y);
}
// Set the ycoord of the end and move the xcoord to match.
void TabVector::SetYEnd(int end_y) {
  endpt_.set_x(XAtY(end_y));
  endpt_.set_y(end_y);
}

// Rotate the ends by the given vector. Auto flip start and end if needed.
void TabVector::Rotate(const FCOORD& rotation) {
  startpt_.rotate(rotation);
  endpt_.rotate(rotation);
  int dx = endpt_.x() - startpt_.x();
  int dy = endpt_.y() - startpt_.y();
  if ((dy < 0 && abs(dy) > abs(dx)) || (dx < 0 && abs(dx) > abs(dy))) {
    // Need to flip start/end.
    ICOORD tmp = startpt_;
    startpt_ = endpt_;
    endpt_ = tmp;
  }
}

// Setup the initial constraints, being the limits of
// the vector and the extended ends.
void TabVector::SetupConstraints() {
  TabConstraint::CreateConstraint(this, false);
  TabConstraint::CreateConstraint(this, true);
}

// Setup the constraints between the partners of this TabVector.
void TabVector::SetupPartnerConstraints() {
  // With the first and last partner, we want a common bottom and top,
  // respectively, and for each change of partner, we want a common
  // top of first with bottom of next.
  TabVector_C_IT it(&partners_);
  TabVector* prev_partner = NULL;
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    TabVector* partner = it.data();
    if (partner->top_constraints_ == NULL ||
        partner->bottom_constraints_ == NULL) {
      partner->Print("Impossible: has no constraints");
      Print("This vector has it as a partner");
      continue;
    }
    if (prev_partner == NULL) {
      // This is the first partner, so common bottom.
      if (TabConstraint::CompatibleConstraints(bottom_constraints_,
                                               partner->bottom_constraints_))
        TabConstraint::MergeConstraints(bottom_constraints_,
                                        partner->bottom_constraints_);
    } else {
      // We need prev top to be common with partner bottom.
      if (TabConstraint::CompatibleConstraints(prev_partner->top_constraints_,
                                               partner->bottom_constraints_))
        TabConstraint::MergeConstraints(prev_partner->top_constraints_,
                                        partner->bottom_constraints_);
    }
    prev_partner = partner;
    if (it.at_last()) {
      // This is the last partner, so common top.
      if (TabConstraint::CompatibleConstraints(top_constraints_,
                                               partner->top_constraints_))
        TabConstraint::MergeConstraints(top_constraints_,
                                        partner->top_constraints_);
    }
  }
}

// Setup the constraints between this and its partner.
void TabVector::SetupPartnerConstraints(TabVector* partner) {
  if (TabConstraint::CompatibleConstraints(bottom_constraints_,
                                           partner->bottom_constraints_))
    TabConstraint::MergeConstraints(bottom_constraints_,
                                    partner->bottom_constraints_);
  if (TabConstraint::CompatibleConstraints(top_constraints_,
                                           partner->top_constraints_))
    TabConstraint::MergeConstraints(top_constraints_,
                                    partner->top_constraints_);
}

// Use the constraints to modify the top and bottom.
void TabVector::ApplyConstraints() {
  if (top_constraints_ != NULL)
    TabConstraint::ApplyConstraints(top_constraints_);
  if (bottom_constraints_ != NULL)
    TabConstraint::ApplyConstraints(bottom_constraints_);
}

// Merge close tab vectors of the same side that overlap.
void TabVector::MergeSimilarTabVectors(const ICOORD& vertical,
                                       TabVector_LIST* vectors,
                                       BlobGrid* grid) {
  TabVector_IT it1(vectors);
  for (it1.mark_cycle_pt(); !it1.cycled_list(); it1.forward()) {
    TabVector* v1 = it1.data();
    TabVector_IT it2(it1);
    for (it2.forward(); !it2.at_first(); it2.forward()) {
      TabVector* v2 = it2.data();
      if (v2->SimilarTo(vertical, *v1, grid)) {
        // Merge into the forward one, in case the combined vector now
        // overlaps one in between.
        if (textord_debug_tabfind) {
          v2->Print("Merging");
          v1->Print("by deleting");
        }
        v2->MergeWith(vertical, it1.extract());
        if (textord_debug_tabfind) {
          v2->Print("Producing");
        }
        ICOORD merged_vector = v2->endpt();
        merged_vector -= v2->startpt();
        if (textord_debug_tabfind && abs(merged_vector.x()) > 100) {
          v2->Print("Garbage result of merge?");
        }
        break;
      }
    }
  }
}

// Return true if this vector is the same side, overlaps, and close
// enough to the other to be merged.
bool TabVector::SimilarTo(const ICOORD& vertical,
                          const TabVector& other, BlobGrid* grid) const {
  if ((IsRightTab() && other.IsRightTab()) ||
      (IsLeftTab() && other.IsLeftTab())) {
    // If they don't overlap, at least in extensions, then there is no chance.
    if (ExtendedOverlap(other.extended_ymax_, other.extended_ymin_) < 0)
      return false;
    // A fast approximation to the scale factor of the sort_key_.
    int v_scale = abs(vertical.y());
    if (v_scale == 0)
      v_scale = 1;
    // If they are close enough, then OK.
    if (sort_key_ + kSimilarVectorDist * v_scale >= other.sort_key_ &&
        sort_key_ - kSimilarVectorDist * v_scale <= other.sort_key_)
      return true;
    // Ragged tabs get a bigger threshold.
    if (!IsRagged() || !other.IsRagged() ||
        sort_key_ + kSimilarRaggedDist * v_scale < other.sort_key_ ||
        sort_key_ - kSimilarRaggedDist * v_scale > other.sort_key_)
      return false;
    if (grid == NULL) {
      // There is nothing else to test!
      return true;
    }
    // If there is nothing in the rectangle between the vector that is going to
    // move, and the place it is moving to, then they can be merged.
    // Setup a vertical search for any blob.
    const TabVector* mover = (IsRightTab() &&
       sort_key_ < other.sort_key_) ? this : &other;
    int top_y = mover->endpt_.y();
    int bottom_y = mover->startpt_.y();
    int left = MIN(mover->XAtY(top_y), mover->XAtY(bottom_y));
    int right = MAX(mover->XAtY(top_y), mover->XAtY(bottom_y));
    int shift = abs(sort_key_ - other.sort_key_) / v_scale;
    if (IsRightTab()) {
      right += shift;
    } else {
      left -= shift;
    }

    GridSearch<BLOBNBOX, BLOBNBOX_CLIST, BLOBNBOX_C_IT> vsearch(grid);
    vsearch.StartVerticalSearch(left, right, top_y);
    BLOBNBOX* blob;
    while ((blob = vsearch.NextVerticalSearch(true)) != NULL) {
      TBOX box = blob->bounding_box();
      if (box.top() > bottom_y)
        return true;  // Nothing found.
      if (box.bottom() < top_y)
        continue;  // Doesn't overlap.
      int left_at_box = XAtY(box.bottom());
      int right_at_box = left_at_box;
      if (IsRightTab())
        right_at_box += shift;
      else
        left_at_box -= shift;
      if (MIN(right_at_box, box.right()) > MAX(left_at_box, box.left()))
        return false;
    }
    return true;  // Nothing found.
  }
  return false;
}

// Eat the other TabVector into this and delete it.
void TabVector::MergeWith(const ICOORD& vertical, TabVector* other) {
  extended_ymin_ = MIN(extended_ymin_, other->extended_ymin_);
  extended_ymax_ = MAX(extended_ymax_, other->extended_ymax_);
  if (other->IsRagged()) {
    alignment_ = other->alignment_;
  }
  // Merge sort the two lists of boxes.
  BLOBNBOX_C_IT it1(&boxes_);
  BLOBNBOX_C_IT it2(&other->boxes_);
  while (!it2.empty()) {
    BLOBNBOX* bbox2 = it2.extract();
    it2.forward();
    TBOX box2 = bbox2->bounding_box();
    BLOBNBOX* bbox1 = it1.data();
    TBOX box1 = bbox1->bounding_box();
    while (box1.bottom() < box2.bottom() && !it1.at_last()) {
      it1.forward();
      bbox1 = it1.data();
      box1 = bbox1->bounding_box();
    }
    if (box1.bottom() < box2.bottom()) {
      it1.add_to_end(bbox2);
    } else if (bbox1 != bbox2) {
      it1.add_before_stay_put(bbox2);
    }
  }
  Fit(vertical, true);
  other->Delete(this);
}

// Add a new element to the list of partner TabVectors.
// Partners must be added in order of increasing y coordinate of the text line
// that makes them partners.
// Groups of identical partners are merged into one.
void TabVector::AddPartner(TabVector* partner) {
  if (IsSeparator() || partner->IsSeparator())
    return;
  TabVector_C_IT it(&partners_);
  if (!it.empty()) {
    it.move_to_last();
    if (it.data() == partner)
      return;
  }
  it.add_after_then_move(partner);
}

// Return true if other is a partner of this.
bool TabVector::IsAPartner(const TabVector* other) {
  TabVector_C_IT it(&partners_);
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    if (it.data() == other)
      return true;
  }
  return false;
}

// These names must be synced with the TabAlignment enum in tabvector.h.
const char* kAlignmentNames[] = {
  "Left Aligned",
  "Left Ragged",
  "Center",
  "Right Aligned",
  "Right Ragged",
  "Separator"
};

// Print basic information about this tab vector.
void TabVector::Print(const char* prefix) {
  if (this == NULL) {
    tprintf("%s <null>\n", prefix);
  } else {
    tprintf("%s %s (%d,%d)->(%d,%d) w=%d s=%d, sort key=%d, boxes=%d,"
            " partners=%d\n",
            prefix, kAlignmentNames[alignment_],
            startpt_.x(), startpt_.y(), endpt_.x(), endpt_.y(),
            mean_width_, percent_score_, sort_key_,
            boxes_.length(), partners_.length());
  }
}

// Print basic information about this tab vector and every box in it.
void TabVector::Debug(const char* prefix) {
  Print(prefix);
  BLOBNBOX_C_IT it(&boxes_);
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    BLOBNBOX* bbox = it.data();
    const TBOX& box = bbox->bounding_box();
    tprintf("Box at (%d,%d)->(%d,%d)\n",
            box.left(), box.bottom(), box.right(), box.top());
  }
}

// Draw this tabvector in place in the given window.
void TabVector::Display(ScrollView* tab_win) {
#ifndef GRAPHICS_DISABLED
  if (textord_debug_printable)
    tab_win->Pen(ScrollView::BLUE);
  else if (alignment_ == TA_LEFT_ALIGNED)
    tab_win->Pen(ScrollView::LIME_GREEN);
  else if (alignment_ == TA_LEFT_RAGGED)
    tab_win->Pen(ScrollView::DARK_GREEN);
  else if (alignment_ == TA_RIGHT_ALIGNED)
    tab_win->Pen(ScrollView::PINK);
  else if (alignment_ == TA_RIGHT_RAGGED)
    tab_win->Pen(ScrollView::CORAL);
  else
    tab_win->Pen(ScrollView::WHITE);
  tab_win->Line(startpt_.x(), startpt_.y(), endpt_.x(), endpt_.y());
  tab_win->Pen(ScrollView::GREY);
  tab_win->Line(startpt_.x(), startpt_.y(), startpt_.x(), extended_ymin_);
  tab_win->Line(endpt_.x(), extended_ymax_, endpt_.x(), endpt_.y());
  char score_buf[64];
  snprintf(score_buf, sizeof(score_buf), "%d", percent_score_);
  tab_win->TextAttributes("Times", 50, false, false, false);
  tab_win->Text(startpt_.x(), startpt_.y(), score_buf);
#endif
}

// Refit the line and/or re-evaluate the vector if the dirty flags are set.
void TabVector::FitAndEvaluateIfNeeded(const ICOORD& vertical,
                                       TabFind* finder) {
  if (needs_refit_)
    Fit(vertical, true);
  if (needs_evaluation_)
    Evaluate(vertical, finder);
}

// Evaluate the vector in terms of coverage of its length by good-looking
// box edges. A good looking box is one where its nearest neighbour on the
// inside is nearer than half the distance its nearest neighbour on the
// outside of the putative column. Bad boxes are removed from the line.
// A second pass then further filters boxes by requiring that the gutter
// width be a minimum fraction of the mean gutter along the line.
void TabVector::Evaluate(const ICOORD& vertical, TabFind* finder) {
  bool debug = false;
  needs_evaluation_ = false;
  int length = endpt_.y() - startpt_.y();
  if (length == 0 || boxes_.empty()) {
    percent_score_ = 0;
    Print("Zero length in evaluate");
    return;
  }
  // Compute the mean box height.
  BLOBNBOX_C_IT it(&boxes_);
  int mean_height = 0;
  int height_count = 0;
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    BLOBNBOX* bbox = it.data();
    const TBOX& box = bbox->bounding_box();
    int height = box.height();
    mean_height += height;
    ++height_count;
  }
  mean_height /= height_count;
  int max_gutter = kGutterMultiple * mean_height;
  if (IsRagged()) {
    // Ragged edges face a tougher test in that the gap must always be within
    // the height of the blob.
    max_gutter = kGutterToNeighbourRatio * mean_height;
  }

  STATS gutters(0, max_gutter + 1);
  // Evaluate the boxes for their goodness, calculating the coverage as we go.
  // Remove boxes that are not good and shorten the list to the first and
  // last good boxes.
  int num_deleted_boxes = 0;
  bool text_on_image = false;
  int good_length = 0;
  const TBOX* prev_good_box = NULL;
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    BLOBNBOX* bbox = it.data();
    const TBOX& box = bbox->bounding_box();
    int mid_y = (box.top() + box.bottom()) / 2;
    if (TabFind::WithinTestRegion(2, XAtY(box.bottom()), box.bottom())) {
      if (!debug) {
        tprintf("After already deleting %d boxes, ", num_deleted_boxes);
        Print("Starting evaluation");
      }
      debug = true;
    }
    // A good box is one where the nearest neighbour on the inside is closer
    // than half the distance to the nearest neighbour on the outside
    // (of the putative column).
    bool left = IsLeftTab();
    int tab_x = XAtY(mid_y);
    int gutter_width;
    int neighbour_gap;
    finder->GutterWidthAndNeighbourGap(tab_x, mean_height, max_gutter, left,
                                       bbox, &gutter_width, &neighbour_gap);
    if (debug) {
      tprintf("Box (%d,%d)->(%d,%d) has gutter %d, ndist %d\n",
              box.left(), box.bottom(), box.right(), box.top(),
              gutter_width, neighbour_gap);
    }
    // Now we can make the test.
    if (neighbour_gap * kGutterToNeighbourRatio <= gutter_width) {
      // A good box contributes its height to the good_length.
      good_length += box.top() - box.bottom();
      gutters.add(gutter_width, 1);
      // Two good boxes together contribute the gap between them
      // to the good_length as well, as long as the gap is not
      // too big.
      if (prev_good_box != NULL) {
        int vertical_gap = box.bottom() - prev_good_box->top();
        double size1 = sqrt(static_cast<double>(prev_good_box->area()));
        double size2 = sqrt(static_cast<double>(box.area()));
        if (vertical_gap < kMaxFillinMultiple * MIN(size1, size2))
          good_length += vertical_gap;
        if (debug) {
          tprintf("Box and prev good, gap=%d, target %g, goodlength=%d\n",
                  vertical_gap, kMaxFillinMultiple * MIN(size1, size2),
                  good_length);
        }
      } else {
        // Adjust the start to the first good box.
        SetYStart(box.bottom());
      }
      prev_good_box = &box;
      if (bbox->flow() == BTFT_TEXT_ON_IMAGE)
        text_on_image = true;
    } else {
      // Get rid of boxes that are not good.
      if (debug) {
        tprintf("Bad Box (%d,%d)->(%d,%d) with gutter %d, ndist %d\n",
                box.left(), box.bottom(), box.right(), box.top(),
                gutter_width, neighbour_gap);
      }
      it.extract();
      ++num_deleted_boxes;
    }
  }
  if (debug) {
    Print("Evaluating:");
  }
  // If there are any good boxes, do it again, except this time get rid of
  // boxes that have a gutter that is a small fraction of the mean gutter.
  // This filters out ends that run into a coincidental gap in the text.
  int search_top = endpt_.y();
  int search_bottom = startpt_.y();
  int median_gutter = IntCastRounded(gutters.median());
  if (gutters.get_total() > 0) {
    prev_good_box = NULL;
    for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
      BLOBNBOX* bbox = it.data();
      const TBOX& box = bbox->bounding_box();
      int mid_y = (box.top() + box.bottom()) / 2;
      // A good box is one where the gutter width is at least some constant
      // fraction of the mean gutter width.
      bool left = IsLeftTab();
      int tab_x = XAtY(mid_y);
      int max_gutter = kGutterMultiple * mean_height;
      if (IsRagged()) {
        // Ragged edges face a tougher test in that the gap must always be
        // within the height of the blob.
        max_gutter = kGutterToNeighbourRatio * mean_height;
      }
      int gutter_width;
      int neighbour_gap;
      finder->GutterWidthAndNeighbourGap(tab_x, mean_height, max_gutter, left,
                                         bbox, &gutter_width, &neighbour_gap);
      // Now we can make the test.
      if (gutter_width >= median_gutter * kMinGutterFraction) {
        if (prev_good_box == NULL) {
          // Adjust the start to the first good box.
          SetYStart(box.bottom());
          search_bottom = box.top();
        }
        prev_good_box = &box;
        search_top = box.bottom();
      } else {
        // Get rid of boxes that are not good.
        if (debug) {
          tprintf("Bad Box (%d,%d)->(%d,%d) with gutter %d, mean gutter %d\n",
                  box.left(), box.bottom(), box.right(), box.top(),
                  gutter_width, median_gutter);
        }
        it.extract();
        ++num_deleted_boxes = true;
      }
    }
  }
  // If there has been a good box, adjust the end.
  if (prev_good_box != NULL) {
    SetYEnd(prev_good_box->top());
    // Compute the percentage of the vector that is occupied by good boxes.
    int length = endpt_.y() - startpt_.y();
    percent_score_ = 100 * good_length / length;
    if (num_deleted_boxes > 0) {
      needs_refit_ = true;
      FitAndEvaluateIfNeeded(vertical, finder);
      if (boxes_.empty())
        return;
    }
    // Test the gutter over the whole vector, instead of just at the boxes.
    int required_shift;
    if (search_bottom > search_top) {
      search_bottom = startpt_.y();
      search_top = endpt_.y();
    }
    double min_gutter_width = kLineCountReciprocal / boxes_.length();
    min_gutter_width += IsRagged() ? kMinRaggedGutter : kMinAlignedGutter;
    min_gutter_width *= mean_height;
    int max_gutter_width = IntCastRounded(min_gutter_width) + 1;
    if (median_gutter > max_gutter_width)
      max_gutter_width = median_gutter;
    int gutter_width = finder->GutterWidth(search_bottom, search_top, *this,
                                           text_on_image, max_gutter_width,
                                           &required_shift);
    if (gutter_width < min_gutter_width) {
      if (debug) {
        tprintf("Rejecting bad tab Vector with %d gutter vs %g min\n",
                gutter_width, min_gutter_width);
      }
      boxes_.shallow_clear();
      percent_score_ = 0;
    } else if (debug) {
      tprintf("Final gutter %d, vs limit of %g, required shift = %d\n",
              gutter_width, min_gutter_width, required_shift);
    }
  } else {
    // There are no good boxes left, so score is 0.
    percent_score_ = 0;
  }

  if (debug) {
    Print("Evaluation complete:");
  }
}

// (Re)Fit a line to the stored points. Returns false if the line
// is degenerate. Althougth the TabVector code mostly doesn't care about the
// direction of lines, XAtY would give silly results for a horizontal line.
// The class is mostly aimed at use for vertical lines representing
// horizontal tab stops.
bool TabVector::Fit(ICOORD vertical, bool force_parallel) {
  needs_refit_ = false;
  if (boxes_.empty()) {
    // Don't refit something with no boxes, as that only happens
    // in Evaluate, and we don't want to end up with a zero vector.
    if (!force_parallel)
      return false;
    // If we are forcing parallel, then we just need to set the sort_key_.
    ICOORD midpt = startpt_;
    midpt += endpt_;
    midpt /= 2;
    sort_key_ = SortKey(vertical, midpt.x(), midpt.y());
    return startpt_.y() != endpt_.y();
  }
  if (!force_parallel && !IsRagged()) {
    // Use a fitted line as the vertical.
    DetLineFit linepoints;
    BLOBNBOX_C_IT it(&boxes_);
    // Fit a line to all the boxes in the list.
    for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
      BLOBNBOX* bbox = it.data();
      TBOX box = bbox->bounding_box();
      int x1 = IsRightTab() ? box.right() : box.left();
      ICOORD boxpt(x1, box.bottom());
      linepoints.Add(boxpt);
      if (it.at_last()) {
        ICOORD top_pt(x1, box.top());
        linepoints.Add(top_pt);
      }
    }
    linepoints.Fit(&startpt_, &endpt_);
    if (startpt_.y() != endpt_.y()) {
      vertical = endpt_;
      vertical -= startpt_;
    }
  }
  int start_y = startpt_.y();
  int end_y = endpt_.y();
  sort_key_ = IsLeftTab() ? MAX_INT32 : -MAX_INT32;
  BLOBNBOX_C_IT it(&boxes_);
  // Choose a line parallel to the vertical such that all boxes are on the
  // correct side of it.
  mean_width_ = 0;
  int width_count = 0;
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    BLOBNBOX* bbox = it.data();
    TBOX box = bbox->bounding_box();
    mean_width_ += box.width();
    ++width_count;
    int x1 = IsRightTab() ? box.right() : box.left();
    // Test both the bottom and the top, as one will be more extreme, depending
    // on the direction of skew.
    int bottom_y = box.bottom();
    int top_y = box.top();
    int key = SortKey(vertical, x1, bottom_y);
    if (IsLeftTab() == (key < sort_key_)) {
      sort_key_ = key;
      startpt_ = ICOORD(x1, bottom_y);
    }
    key = SortKey(vertical, x1, top_y);
    if (IsLeftTab() == (key < sort_key_)) {
      sort_key_ = key;
      startpt_ = ICOORD(x1, top_y);
    }
    if (it.at_first())
      start_y = bottom_y;
    if (it.at_last())
      end_y = top_y;
  }
  if (width_count > 0) {
    mean_width_ = (mean_width_ + width_count - 1) / width_count;
  }
  endpt_ = startpt_ + vertical;
  needs_evaluation_ = true;
  if (start_y != end_y) {
    // Set the ends of the vector to fully include the first and last blobs.
    startpt_.set_x(XAtY(vertical, sort_key_, start_y));
    startpt_.set_y(start_y);
    endpt_.set_x(XAtY(vertical, sort_key_, end_y));
    endpt_.set_y(end_y);
    return true;
  }
  return false;
}

// Returns the singleton partner if there is one, or NULL otherwise.
TabVector* TabVector::GetSinglePartner() {
  if (!partners_.singleton())
    return NULL;
  TabVector_C_IT partner_it(&partners_);
  TabVector* partner = partner_it.data();
  return partner;
}

// Return the partner of this TabVector if the vector qualifies as
// being a vertical text line, otherwise NULL.
TabVector* TabVector::VerticalTextlinePartner() {
  if (!partners_.singleton())
    return NULL;
  TabVector_C_IT partner_it(&partners_);
  TabVector* partner = partner_it.data();
  BLOBNBOX_C_IT box_it1(&boxes_);
  BLOBNBOX_C_IT box_it2(&partner->boxes_);
  // Count how many boxes are also in the other list.
  // At the same time, gather the mean width and median vertical gap.
  if (textord_debug_tabfind > 1) {
    Print("Testing for vertical text");
    partner->Print("           partner");
  }
  int num_matched = 0;
  int num_unmatched = 0;
  int total_widths = 0;
  int width = startpt().x() - partner->startpt().x();
  if (width < 0)
    width = -width;
  STATS gaps(0, width * 2);
  BLOBNBOX* prev_bbox = NULL;
  box_it2.mark_cycle_pt();
  for (box_it1.mark_cycle_pt(); !box_it1.cycled_list(); box_it1.forward()) {
    BLOBNBOX* bbox = box_it1.data();
    TBOX box = bbox->bounding_box();
    if (prev_bbox != NULL) {
      gaps.add(box.bottom() - prev_bbox->bounding_box().top(), 1);
    }
    while (!box_it2.cycled_list() && box_it2.data() != bbox &&
           box_it2.data()->bounding_box().bottom() < box.bottom()) {
      box_it2.forward();
    }
    if (!box_it2.cycled_list() && box_it2.data() == bbox &&
        bbox->region_type() >= BRT_UNKNOWN &&
        (prev_bbox == NULL || prev_bbox->region_type() >= BRT_UNKNOWN))
      ++num_matched;
    else
      ++num_unmatched;
    total_widths += box.width();
    prev_bbox = bbox;
  }
  double avg_width = total_widths * 1.0 / (num_unmatched + num_matched);
  double max_gap = textord_tabvector_vertical_gap_fraction * avg_width;
  int min_box_match = static_cast<int>((num_matched + num_unmatched) *
                                       textord_tabvector_vertical_box_ratio);
  bool is_vertical = (gaps.get_total() > 0 &&
                      num_matched >= min_box_match &&
                      gaps.median() <= max_gap);
  if (textord_debug_tabfind > 1) {
    tprintf("gaps=%d, matched=%d, unmatched=%d, min_match=%d "
            "median gap=%.2f, width=%.2f max_gap=%.2f Vertical=%s\n",
            gaps.get_total(), num_matched, num_unmatched, min_box_match,
            gaps.median(), avg_width, max_gap, is_vertical?"Yes":"No");
  }
  return (is_vertical) ? partner : NULL;
}

// The constructor is private.
TabVector::TabVector(int extended_ymin, int extended_ymax,
                     TabAlignment alignment, BLOBNBOX_CLIST* boxes)
  : extended_ymin_(extended_ymin), extended_ymax_(extended_ymax),
    sort_key_(0), percent_score_(0), mean_width_(0),
    needs_refit_(true), needs_evaluation_(true), alignment_(alignment),
    top_constraints_(NULL), bottom_constraints_(NULL) {
  BLOBNBOX_C_IT it(&boxes_);
  it.add_list_after(boxes);
}

// Delete this, but first, repoint all the partners to point to
// replacement. If replacement is NULL, then partner relationships
// are removed.
void TabVector::Delete(TabVector* replacement) {
  TabVector_C_IT it(&partners_);
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    TabVector* partner = it.data();
    TabVector_C_IT p_it(&partner->partners_);
    // If partner already has replacement in its list, then make
    // replacement null, and just remove this TabVector when we find it.
    TabVector* partner_replacement = replacement;
    for (p_it.mark_cycle_pt(); !p_it.cycled_list(); p_it.forward()) {
      TabVector* p_partner = p_it.data();
      if (p_partner == partner_replacement) {
        partner_replacement = NULL;
        break;
      }
    }
    // Remove all references to this, and replace with replacement if not NULL.
    for (p_it.mark_cycle_pt(); !p_it.cycled_list(); p_it.forward()) {
      TabVector* p_partner = p_it.data();
      if (p_partner == this) {
        p_it.extract();
        if (partner_replacement != NULL)
          p_it.add_before_stay_put(partner_replacement);
      }
    }
    if (partner_replacement != NULL) {
      partner_replacement->AddPartner(partner);
    }
  }
  delete this;
}


}  // namespace tesseract.
