///////////////////////////////////////////////////////////////////////
// File:        colpartition.cpp
// Description: Class to hold partitions of the page that correspond
//              roughly to text lines.
// Author:      Ray Smith
// Created:     Thu Aug 14 10:54:01 PDT 2008
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

#include "colpartition.h"
#include "colpartitiongrid.h"
#include "colpartitionset.h"
#include "detlinefit.h"
#include "dppoint.h"
#include "imagefind.h"
#include "workingpartset.h"

#ifdef _MSC_VER
#pragma warning(disable:4244)  // Conversion warnings
#endif

namespace tesseract {

ELIST2IZE(ColPartition)
CLISTIZE(ColPartition)

//////////////// ColPartition Implementation ////////////////

// If multiple partners survive the partner depth test beyond this level,
// then arbitrarily pick one.
const int kMaxPartnerDepth = 4;
// Maximum change in spacing (in inches) to ignore.
const double kMaxSpacingDrift = 1.0 / 72;  // 1/72 is one point.
// Maximum fraction of line height used as an additional allowance
// for top spacing.
const double kMaxTopSpacingFraction = 0.25;
// What multiple of the largest line height should be used as an upper bound
// for whether lines are in the same text block?
const double kMaxSameBlockLineSpacing = 3;
// Maximum ratio of sizes for lines to be considered the same size.
const double kMaxSizeRatio = 1.5;
// Fraction of max of leader width and gap for max IQR of gaps.
const double kMaxLeaderGapFractionOfMax = 0.25;
// Fraction of min of leader width and gap for max IQR of gaps.
const double kMaxLeaderGapFractionOfMin = 0.5;
// Minimum number of blobs to be considered a leader.
const int kMinLeaderCount = 5;
// Cost of a cut through a leader.
const int kLeaderCutCost = 8;
// Minimum score for a STRONG_CHAIN textline.
const int kMinStrongTextValue = 6;
// Minimum score for a CHAIN textline.
const int kMinChainTextValue = 3;
// Minimum number of blobs for strong horizontal text lines.
const int kHorzStrongTextlineCount = 8;
// Minimum height (in image pixels) for strong horizontal text lines.
const int kHorzStrongTextlineHeight = 10;
// Minimum aspect ratio for strong horizontal text lines.
const int kHorzStrongTextlineAspect = 5;
// Maximum upper quartile error allowed on a baseline fit as a fraction
// of height.
const double kMaxBaselineError = 0.4375;
// Min coverage for a good baseline between vectors
const double kMinBaselineCoverage = 0.5;
// Max RMS color noise to compare colors.
const int kMaxRMSColorNoise = 128;
// Maximum distance to allow a partition color to be to use that partition
// in smoothing neighbouring types. This is a squared distance.
const int kMaxColorDistance = 900;

// blob_type is the blob_region_type_ of the blobs in this partition.
// Vertical is the direction of logical vertical on the possibly skewed image.
ColPartition::ColPartition(BlobRegionType blob_type, const ICOORD& vertical)
  : left_margin_(-MAX_INT32), right_margin_(MAX_INT32),
    median_bottom_(MAX_INT32), median_top_(-MAX_INT32), median_size_(0),
    median_left_(MAX_INT32), median_right_(-MAX_INT32), median_width_(0),
    blob_type_(blob_type), flow_(BTFT_NONE), good_blob_score_(0),
    good_width_(false), good_column_(false),
    left_key_tab_(false), right_key_tab_(false),
    left_key_(0), right_key_(0), type_(PT_UNKNOWN), vertical_(vertical),
    working_set_(NULL), last_add_was_vertical_(false), block_owned_(false),
    desperately_merged_(false),
    first_column_(-1), last_column_(-1), column_set_(NULL),
    side_step_(0), top_spacing_(0), bottom_spacing_(0),
    type_before_table_(PT_UNKNOWN), inside_table_column_(false),
    nearest_neighbor_above_(NULL), nearest_neighbor_below_(NULL),
    space_above_(0), space_below_(0), space_to_left_(0), space_to_right_(0),
    owns_blobs_(true) {
  memset(special_blobs_densities_, 0, sizeof(special_blobs_densities_));
}

// Constructs a fake ColPartition with a single fake BLOBNBOX, all made
// from a single TBOX.
// WARNING: Despite being on C_LISTs, the BLOBNBOX owns the C_BLOB and
// the ColPartition owns the BLOBNBOX!!!
// Call DeleteBoxes before deleting the ColPartition.
ColPartition* ColPartition::FakePartition(const TBOX& box,
                                          PolyBlockType block_type,
                                          BlobRegionType blob_type,
                                          BlobTextFlowType flow) {
  ColPartition* part = new ColPartition(blob_type, ICOORD(0, 1));
  part->set_type(block_type);
  part->set_flow(flow);
  part->AddBox(new BLOBNBOX(C_BLOB::FakeBlob(box)));
  part->set_left_margin(box.left());
  part->set_right_margin(box.right());
  part->SetBlobTypes();
  part->ComputeLimits();
  part->ClaimBoxes();
  return part;
}

// Constructs and returns a ColPartition with the given real BLOBNBOX,
// and sets it up to be a "big" partition (single-blob partition bigger
// than the surrounding text that may be a dropcap, two or more vertically
// touching characters, or some graphic element.
// If the given list is not NULL, the partition is also added to the list.
ColPartition* ColPartition::MakeBigPartition(BLOBNBOX* box,
                                             ColPartition_LIST* big_part_list) {
  box->set_owner(NULL);
  ColPartition* single = new ColPartition(BRT_UNKNOWN, ICOORD(0, 1));
  single->set_flow(BTFT_NONE);
  single->AddBox(box);
  single->ComputeLimits();
  single->ClaimBoxes();
  single->SetBlobTypes();
  single->set_block_owned(true);
  if (big_part_list != NULL) {
    ColPartition_IT part_it(big_part_list);
    part_it.add_to_end(single);
  }
  return single;
}

ColPartition::~ColPartition() {
  // Remove this as a partner of all partners, as we don't want them
  // referring to a deleted object.
  ColPartition_C_IT it(&upper_partners_);
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    it.data()->RemovePartner(false, this);
  }
  it.set_to_list(&lower_partners_);
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    it.data()->RemovePartner(true, this);
  }
}

// Constructs a fake ColPartition with no BLOBNBOXes to represent a
// horizontal or vertical line, given a type and a bounding box.
ColPartition* ColPartition::MakeLinePartition(BlobRegionType blob_type,
                                              const ICOORD& vertical,
                                              int left, int bottom,
                                              int right, int top) {
  ColPartition* part = new ColPartition(blob_type, vertical);
  part->bounding_box_ = TBOX(left, bottom, right, top);
  part->median_bottom_ = bottom;
  part->median_top_ = top;
  part->median_size_ = top - bottom;
  part->median_width_ = right - left;
  part->left_key_ = part->BoxLeftKey();
  part->right_key_ = part->BoxRightKey();
  return part;
}


// Adds the given box to the partition, updating the partition bounds.
// The list of boxes in the partition is updated, ensuring that no box is
// recorded twice, and the boxes are kept in increasing left position.
void ColPartition::AddBox(BLOBNBOX* bbox) {
  TBOX box = bbox->bounding_box();
  // Update the partition limits.
  if (boxes_.length() == 0) {
    bounding_box_ = box;
  } else {
    bounding_box_ += box;
  }

  if (IsVerticalType()) {
    if (!last_add_was_vertical_) {
      boxes_.sort(SortByBoxBottom<BLOBNBOX>);
      last_add_was_vertical_ = true;
    }
    boxes_.add_sorted(SortByBoxBottom<BLOBNBOX>, true, bbox);
  } else {
    if (last_add_was_vertical_) {
      boxes_.sort(SortByBoxLeft<BLOBNBOX>);
      last_add_was_vertical_ = false;
    }
    boxes_.add_sorted(SortByBoxLeft<BLOBNBOX>, true, bbox);
  }
  if (!left_key_tab_)
    left_key_ = BoxLeftKey();
  if (!right_key_tab_)
    right_key_ = BoxRightKey();
  if (TabFind::WithinTestRegion(2, box.left(), box.bottom()))
    tprintf("Added box (%d,%d)->(%d,%d) left_blob_x_=%d, right_blob_x_ = %d\n",
            box.left(), box.bottom(), box.right(), box.top(),
            bounding_box_.left(), bounding_box_.right());
}

// Removes the given box from the partition, updating the bounds.
void ColPartition::RemoveBox(BLOBNBOX* box) {
  BLOBNBOX_C_IT bb_it(&boxes_);
  for (bb_it.mark_cycle_pt(); !bb_it.cycled_list(); bb_it.forward()) {
    if (box == bb_it.data()) {
      bb_it.extract();
      ComputeLimits();
      return;
    }
  }
}

// Returns the tallest box in the partition, as measured perpendicular to the
// presumed flow of text.
BLOBNBOX* ColPartition::BiggestBox() {
  BLOBNBOX* biggest = NULL;
  BLOBNBOX_C_IT bb_it(&boxes_);
  for (bb_it.mark_cycle_pt(); !bb_it.cycled_list(); bb_it.forward()) {
    BLOBNBOX* bbox = bb_it.data();
    if (IsVerticalType()) {
      if (biggest == NULL ||
          bbox->bounding_box().width() > biggest->bounding_box().width())
        biggest = bbox;
    } else {
      if (biggest == NULL ||
          bbox->bounding_box().height() > biggest->bounding_box().height())
        biggest = bbox;
    }
  }
  return biggest;
}

// Returns the bounding box excluding the given box.
TBOX ColPartition::BoundsWithoutBox(BLOBNBOX* box) {
  TBOX result;
  BLOBNBOX_C_IT bb_it(&boxes_);
  for (bb_it.mark_cycle_pt(); !bb_it.cycled_list(); bb_it.forward()) {
    if (box != bb_it.data()) {
      result += bb_it.data()->bounding_box();
    }
  }
  return result;
}

// Claims the boxes in the boxes_list by marking them with a this owner
// pointer. If a box is already owned, then it must be owned by this.
void ColPartition::ClaimBoxes() {
  BLOBNBOX_C_IT bb_it(&boxes_);
  for (bb_it.mark_cycle_pt(); !bb_it.cycled_list(); bb_it.forward()) {
    BLOBNBOX* bblob = bb_it.data();
    ColPartition* other = bblob->owner();
    if (other == NULL) {
      // Normal case: ownership is available.
      bblob->set_owner(this);
    } else {
      ASSERT_HOST(other == this);
    }
  }
}

// NULL the owner of the blobs in this partition, so they can be deleted
// independently of the ColPartition.
void ColPartition::DisownBoxes() {
  BLOBNBOX_C_IT bb_it(&boxes_);
  for (bb_it.mark_cycle_pt(); !bb_it.cycled_list(); bb_it.forward()) {
    BLOBNBOX* bblob = bb_it.data();
    ASSERT_HOST(bblob->owner() == this || bblob->owner() == NULL);
    bblob->set_owner(NULL);
  }
}

// Delete the boxes that this partition owns.
void ColPartition::DeleteBoxes() {
  // Although the boxes_ list is a C_LIST, in some cases it owns the
  // BLOBNBOXes, as the ColPartition takes ownership from the grid,
  // and the BLOBNBOXes own the underlying C_BLOBs.
  for (BLOBNBOX_C_IT bb_it(&boxes_); !bb_it.empty(); bb_it.forward()) {
    BLOBNBOX* bblob = bb_it.extract();
    delete bblob->cblob();
    delete bblob;
  }
}

// Reflects the partition in the y-axis, assuming that its blobs have
// already been done. Corrects only a limited part of the members, since
// this function is assumed to be used shortly after initial creation, which
// is before a lot of the members are used.
void ColPartition::ReflectInYAxis() {
  ColPartition_CLIST reversed_boxes;
  ColPartition_C_IT reversed_it(&reversed_boxes);
  // Reverse the order of the boxes_.
  BLOBNBOX_C_IT bb_it(&boxes_);
  for (bb_it.mark_cycle_pt(); !bb_it.cycled_list(); bb_it.forward()) {
    reversed_it.add_before_then_move(bb_it.extract());
  }
  bb_it.add_list_after(&reversed_boxes);
  ASSERT_HOST(!left_key_tab_ && !right_key_tab_);
  int tmp = left_margin_;
  left_margin_ = -right_margin_;
  right_margin_ = -tmp;
  ComputeLimits();
}

// Returns true if this is a legal partition - meaning that the conditions
// left_margin <= bounding_box left
// left_key <= bounding box left key
// bounding box left <= bounding box right
// and likewise for right margin and key
// are all met.
bool ColPartition::IsLegal() {
  if (bounding_box_.left() > bounding_box_.right()) {
    if (textord_debug_bugs) {
      tprintf("Bounding box invalid\n");
      Print();
    }
    return false;  // Bounding box invalid.
  }
  if (left_margin_ > bounding_box_.left() ||
      right_margin_ < bounding_box_.right()) {
    if (textord_debug_bugs) {
      tprintf("Margins invalid\n");
      Print();
    }
    return false;  // Margins invalid.
  }
  if (left_key_ > BoxLeftKey() || right_key_ < BoxRightKey()) {
    if (textord_debug_bugs) {
      tprintf("Key inside box: %d v %d or %d v %d\n",
              left_key_, BoxLeftKey(), right_key_, BoxRightKey());
      Print();
    }
    return false;  // Keys inside the box.
  }
  return true;
}

// Returns true if the left and right edges are approximately equal.
bool ColPartition::MatchingColumns(const ColPartition& other) const {
  int y = (MidY() + other.MidY()) / 2;
  if (!NearlyEqual(other.LeftAtY(y) / kColumnWidthFactor,
                   LeftAtY(y) / kColumnWidthFactor, 1))
    return false;
  if (!NearlyEqual(other.RightAtY(y) / kColumnWidthFactor,
                   RightAtY(y) / kColumnWidthFactor, 1))
    return false;
  return true;
}

// Returns true if the colors match for two text partitions.
bool ColPartition::MatchingTextColor(const ColPartition& other) const {
  if (color1_[L_ALPHA_CHANNEL] > kMaxRMSColorNoise &&
      other.color1_[L_ALPHA_CHANNEL] > kMaxRMSColorNoise)
    return false;  // Too noisy.

  // Colors must match for other to count.
  double d_this1_o = ImageFind::ColorDistanceFromLine(other.color1_,
                                                      other.color2_,
                                                      color1_);
  double d_this2_o = ImageFind::ColorDistanceFromLine(other.color1_,
                                                      other.color2_,
                                                      color2_);
  double d_o1_this = ImageFind::ColorDistanceFromLine(color1_, color2_,
                                                      other.color1_);
  double d_o2_this = ImageFind::ColorDistanceFromLine(color1_, color2_,
                                                      other.color2_);
// All 4 distances must be small enough.
  return d_this1_o < kMaxColorDistance && d_this2_o < kMaxColorDistance &&
         d_o1_this < kMaxColorDistance && d_o2_this < kMaxColorDistance;
}

// Returns true if the sizes match for two text partitions,
// taking orientation into account. See also SizesSimilar.
bool ColPartition::MatchingSizes(const ColPartition& other) const {
  if (blob_type_ == BRT_VERT_TEXT || other.blob_type_ == BRT_VERT_TEXT)
    return !TabFind::DifferentSizes(median_width_, other.median_width_);
  else
    return !TabFind::DifferentSizes(median_size_, other.median_size_);
}

// Returns true if there is no tabstop violation in merging this and other.
bool ColPartition::ConfirmNoTabViolation(const ColPartition& other) const {
  if (bounding_box_.right() < other.bounding_box_.left() &&
      bounding_box_.right() < other.LeftBlobRule())
    return false;
  if (other.bounding_box_.right() < bounding_box_.left() &&
      other.bounding_box_.right() < LeftBlobRule())
    return false;
  if (bounding_box_.left() > other.bounding_box_.right() &&
      bounding_box_.left() > other.RightBlobRule())
    return false;
  if (other.bounding_box_.left() > bounding_box_.right() &&
      other.bounding_box_.left() > RightBlobRule())
    return false;
  return true;
}

// Returns true if other has a similar stroke width to this.
bool ColPartition::MatchingStrokeWidth(const ColPartition& other,
                                       double fractional_tolerance,
                                       double constant_tolerance) const {
  int match_count = 0;
  int nonmatch_count = 0;
  BLOBNBOX_C_IT box_it(const_cast<BLOBNBOX_CLIST*>(&boxes_));
  BLOBNBOX_C_IT other_it(const_cast<BLOBNBOX_CLIST*>(&other.boxes_));
  box_it.mark_cycle_pt();
  other_it.mark_cycle_pt();
  while (!box_it.cycled_list() && !other_it.cycled_list()) {
    if (box_it.data()->MatchingStrokeWidth(*other_it.data(),
                                           fractional_tolerance,
                                           constant_tolerance))
      ++match_count;
    else
      ++nonmatch_count;
    box_it.forward();
    other_it.forward();
  }
  return match_count > nonmatch_count;
}

// Returns true if base is an acceptable diacritic base char merge
// with this as the diacritic.
// Returns true if:
// (1) this is a ColPartition containing only diacritics, and
// (2) the base characters indicated on the diacritics all believably lie
// within the text line of the candidate ColPartition.
bool ColPartition::OKDiacriticMerge(const ColPartition& candidate,
                                    bool debug) const {
  BLOBNBOX_C_IT it(const_cast<BLOBNBOX_CLIST*>(&boxes_));
  int min_top = MAX_INT32;
  int max_bottom = -MAX_INT32;
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    BLOBNBOX* blob = it.data();
    if (!blob->IsDiacritic()) {
      if (debug) {
        tprintf("Blob is not a diacritic:");
        blob->bounding_box().print();
      }
      return false;  // All blobs must have diacritic bases.
    }
    if (blob->base_char_top() < min_top)
      min_top = blob->base_char_top();
    if (blob->base_char_bottom() > max_bottom)
      max_bottom = blob->base_char_bottom();
  }
  // If the intersection of all vertical ranges of all base characters
  // overlaps the median range of this, then it is OK.
  bool result = min_top > candidate.median_bottom_ &&
                max_bottom < candidate.median_top_;
  if (debug) {
    if (result)
      tprintf("OKDiacritic!\n");
    else
      tprintf("y ranges don\'t overlap: %d-%d / %d-%d\n",
              max_bottom, min_top, median_bottom_, median_top_);
  }
  return result;
}

// Sets the sort key using either the tab vector, or the bounding box if
// the tab vector is NULL. If the tab_vector lies inside the bounding_box,
// use the edge of the box as a key any way.
void ColPartition::SetLeftTab(const TabVector* tab_vector) {
  if (tab_vector != NULL) {
    left_key_ = tab_vector->sort_key();
    left_key_tab_ = left_key_ <= BoxLeftKey();
  } else {
    left_key_tab_ = false;
  }
  if (!left_key_tab_)
    left_key_ = BoxLeftKey();
}

// As SetLeftTab, but with the right.
void ColPartition::SetRightTab(const TabVector* tab_vector) {
  if (tab_vector != NULL) {
    right_key_ = tab_vector->sort_key();
    right_key_tab_ = right_key_ >= BoxRightKey();
  } else {
    right_key_tab_ = false;
  }
  if (!right_key_tab_)
    right_key_ = BoxRightKey();
}

// Copies the left/right tab from the src partition, but if take_box is
// true, copies the box instead and uses that as a key.
void ColPartition::CopyLeftTab(const ColPartition& src, bool take_box) {
  left_key_tab_ = take_box ? false : src.left_key_tab_;
  if (left_key_tab_) {
    left_key_ = src.left_key_;
  } else {
    bounding_box_.set_left(XAtY(src.BoxLeftKey(), MidY()));
    left_key_ = BoxLeftKey();
  }
  if (left_margin_ > bounding_box_.left())
    left_margin_ = src.left_margin_;
}

// As CopyLeftTab, but with the right.
void ColPartition::CopyRightTab(const ColPartition& src, bool take_box) {
  right_key_tab_ = take_box ? false : src.right_key_tab_;
  if (right_key_tab_) {
    right_key_ = src.right_key_;
  } else {
    bounding_box_.set_right(XAtY(src.BoxRightKey(), MidY()));
    right_key_ = BoxRightKey();
  }
  if (right_margin_ < bounding_box_.right())
    right_margin_ = src.right_margin_;
}

// Returns the left rule line x coord of the leftmost blob.
int ColPartition::LeftBlobRule() const {
  BLOBNBOX_C_IT it(const_cast<BLOBNBOX_CLIST*>(&boxes_));
  return it.data()->left_rule();
}
// Returns the right rule line x coord of the rightmost blob.
int ColPartition::RightBlobRule() const {
  BLOBNBOX_C_IT it(const_cast<BLOBNBOX_CLIST*>(&boxes_));
  it.move_to_last();
  return it.data()->right_rule();
}

float ColPartition::SpecialBlobsDensity(const BlobSpecialTextType type) const {
  ASSERT_HOST(type < BSTT_COUNT);
  return special_blobs_densities_[type];
}

int ColPartition::SpecialBlobsCount(const BlobSpecialTextType type) {
  ASSERT_HOST(type < BSTT_COUNT);
  BLOBNBOX_C_IT blob_it(&boxes_);
  int count = 0;
  for (blob_it.mark_cycle_pt(); !blob_it.cycled_list(); blob_it.forward()) {
    BLOBNBOX* blob = blob_it.data();
    BlobSpecialTextType blob_type = blob->special_text_type();
    if (blob_type == type) {
      count++;
    }
  }

  return count;
}

void ColPartition::SetSpecialBlobsDensity(
    const BlobSpecialTextType type, const float density) {
  ASSERT_HOST(type < BSTT_COUNT);
  special_blobs_densities_[type] = density;
}

void ColPartition::ComputeSpecialBlobsDensity() {
  memset(special_blobs_densities_, 0, sizeof(special_blobs_densities_));
  if (boxes_.empty()) {
    return;
  }

  BLOBNBOX_C_IT blob_it(&boxes_);
  for (blob_it.mark_cycle_pt(); !blob_it.cycled_list(); blob_it.forward()) {
    BLOBNBOX* blob = blob_it.data();
    BlobSpecialTextType type = blob->special_text_type();
    special_blobs_densities_[type]++;
  }

  for (int type = 0; type < BSTT_COUNT; ++type) {
    special_blobs_densities_[type] /= boxes_.length();
  }
}

// Add a partner above if upper, otherwise below.
// Add them uniquely and keep the list sorted by box left.
// Partnerships are added symmetrically to partner and this.
void ColPartition::AddPartner(bool upper, ColPartition* partner) {
  if (upper) {
    partner->lower_partners_.add_sorted(SortByBoxLeft<ColPartition>,
                                        true, this);
    upper_partners_.add_sorted(SortByBoxLeft<ColPartition>, true, partner);
  } else {
    partner->upper_partners_.add_sorted(SortByBoxLeft<ColPartition>,
                                        true, this);
    lower_partners_.add_sorted(SortByBoxLeft<ColPartition>, true, partner);
  }
}

// Removes the partner from this, but does not remove this from partner.
// This asymmetric removal is so as not to mess up the iterator that is
// working on partner's partner list.
void ColPartition::RemovePartner(bool upper, ColPartition* partner) {
  ColPartition_C_IT it(upper ? &upper_partners_ : &lower_partners_);
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    if (it.data() == partner) {
      it.extract();
      break;
    }
  }
}

// Returns the partner if the given partner is a singleton, otherwise NULL.
ColPartition* ColPartition::SingletonPartner(bool upper) {
  ColPartition_CLIST* partners = upper ? &upper_partners_ : &lower_partners_;
  if (!partners->singleton())
    return NULL;
  ColPartition_C_IT it(partners);
  return it.data();
}

// Merge with the other partition and delete it.
void ColPartition::Absorb(ColPartition* other, WidthCallback* cb) {
  // The result has to either own all of the blobs or none of them.
  // Verify the flag is consisent.
  ASSERT_HOST(owns_blobs() == other->owns_blobs());
  // TODO(nbeato): check owns_blobs better. Right now owns_blobs
  // should always be true when this is called. So there is no issues.
  if (TabFind::WithinTestRegion(2, bounding_box_.left(),
                                bounding_box_.bottom()) ||
      TabFind::WithinTestRegion(2, other->bounding_box_.left(),
                                other->bounding_box_.bottom())) {
    tprintf("Merging:");
    Print();
    other->Print();
  }

  // Update the special_blobs_densities_.
  memset(special_blobs_densities_, 0, sizeof(special_blobs_densities_));
  for (int type = 0; type < BSTT_COUNT; ++type) {
    int w1 = boxes_.length(), w2 = other->boxes_.length();
    float new_val = special_blobs_densities_[type] * w1 +
        other->special_blobs_densities_[type] * w2;
    if (!w1 || !w2) {
      special_blobs_densities_[type] = new_val / (w1 + w2);
    }
  }

  // Merge the two sorted lists.
  BLOBNBOX_C_IT it(&boxes_);
  BLOBNBOX_C_IT it2(&other->boxes_);
  for (; !it2.empty(); it2.forward()) {
    BLOBNBOX* bbox2 = it2.extract();
    ColPartition* prev_owner = bbox2->owner();
    if (prev_owner != other && prev_owner != NULL) {
      // A blob on other's list is owned by someone else; let them have it.
      continue;
    }
    ASSERT_HOST(prev_owner == other || prev_owner == NULL);
    if (prev_owner == other)
      bbox2->set_owner(this);
    it.add_to_end(bbox2);
  }
  left_margin_ = MIN(left_margin_, other->left_margin_);
  right_margin_ = MAX(right_margin_, other->right_margin_);
  if (other->left_key_ < left_key_) {
    left_key_ = other->left_key_;
    left_key_tab_ = other->left_key_tab_;
  }
  if (other->right_key_ > right_key_) {
    right_key_ = other->right_key_;
    right_key_tab_ = other->right_key_tab_;
  }
  // Combine the flow and blob_type in a sensible way.
  // Dominant flows stay.
  if (!DominatesInMerge(flow_, other->flow_)) {
    flow_ = other->flow_;
    blob_type_ = other->blob_type_;
  }
  SetBlobTypes();
  if (IsVerticalType()) {
    boxes_.sort(SortByBoxBottom<BLOBNBOX>);
    last_add_was_vertical_ = true;
  } else {
    boxes_.sort(SortByBoxLeft<BLOBNBOX>);
    last_add_was_vertical_ = false;
  }
  ComputeLimits();
  // Fix partner lists. other is going away, so remove it as a
  // partner of all its partners and add this in its place.
  for (int upper = 0; upper < 2; ++upper) {
    ColPartition_CLIST partners;
    ColPartition_C_IT part_it(&partners);
    part_it.add_list_after(upper ? &other->upper_partners_
                                 : &other->lower_partners_);
    for (part_it.move_to_first(); !part_it.empty(); part_it.forward()) {
      ColPartition* partner = part_it.extract();
      partner->RemovePartner(!upper, other);
      partner->RemovePartner(!upper, this);
      partner->AddPartner(!upper, this);
    }
  }
  delete other;
  if (cb != NULL) {
    SetColumnGoodness(cb);
  }
}

// Merge1 and merge2 are candidates to be merged, yet their combined box
// overlaps this. Is that allowed?
// Returns true if the overlap between this and the merged pair of
// merge candidates is sufficiently trivial to be allowed.
// The merged box can graze the edge of this by the ok_box_overlap
// if that exceeds the margin to the median top and bottom.
// ok_box_overlap should be set by the caller appropriate to the sizes of
// the text involved, and is usually a fraction of the median size of merge1
// and/or merge2, or this.
// TODO(rays) Determine whether vertical text needs to be considered.
bool ColPartition::OKMergeOverlap(const ColPartition& merge1,
                                  const ColPartition& merge2,
                                  int ok_box_overlap, bool debug) {
  // Vertical partitions are not allowed to be involved.
  if (IsVerticalType() || merge1.IsVerticalType() || merge2.IsVerticalType()) {
    if (debug)
      tprintf("Vertical partition\n");
    return false;
  }
  // The merging partitions must strongly overlap each other.
  if (!merge1.VSignificantCoreOverlap(merge2)) {
    if (debug)
      tprintf("Voverlap %d (%d)\n",
              merge1.VCoreOverlap(merge2),
              merge1.VSignificantCoreOverlap(merge2));
    return false;
  }
  // The merged box must not overlap the median bounds of this.
  TBOX merged_box(merge1.bounding_box());
  merged_box += merge2.bounding_box();
  if (merged_box.bottom() < median_top_ && merged_box.top() > median_bottom_ &&
      merged_box.bottom() < bounding_box_.top() - ok_box_overlap &&
      merged_box.top() > bounding_box_.bottom() + ok_box_overlap) {
    if (debug)
      tprintf("Excessive box overlap\n");
    return false;
  }
  // Looks OK!
  return true;
}

// Find the blob at which to split this to minimize the overlap with the
// given box. Returns the first blob to go in the second partition.
BLOBNBOX* ColPartition::OverlapSplitBlob(const TBOX& box) {
  if (boxes_.empty() || boxes_.singleton())
    return NULL;
  BLOBNBOX_C_IT it(&boxes_);
  TBOX left_box(it.data()->bounding_box());
  for (it.forward(); !it.at_first(); it.forward()) {
    BLOBNBOX* bbox = it.data();
    left_box += bbox->bounding_box();
    if (left_box.overlap(box))
      return bbox;
  }
  return NULL;
}

// Split this partition keeping the first half in this and returning
// the second half.
// Splits by putting the split_blob and the blobs that follow
// in the second half, and the rest in the first half.
ColPartition* ColPartition::SplitAtBlob(BLOBNBOX* split_blob) {
  ColPartition* split_part = ShallowCopy();
  split_part->set_owns_blobs(owns_blobs());
  BLOBNBOX_C_IT it(&boxes_);
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    BLOBNBOX* bbox = it.data();
    ColPartition* prev_owner = bbox->owner();
    ASSERT_HOST(!owns_blobs() || prev_owner == this || prev_owner == NULL);
    if (bbox == split_blob || !split_part->boxes_.empty()) {
      split_part->AddBox(it.extract());
      if (owns_blobs() && prev_owner != NULL)
        bbox->set_owner(split_part);
    }
  }
  ASSERT_HOST(!it.empty());
  if (split_part->IsEmpty()) {
    // Split part ended up with nothing. Possible if split_blob is not
    // in the list of blobs.
    delete split_part;
    return NULL;
  }
  right_key_tab_ = false;
  split_part->left_key_tab_ = false;
  ComputeLimits();
  // TODO(nbeato) Merge Ray's CL like this:
  // if (owns_blobs())
  //  SetBlobTextlineGoodness();
  split_part->ComputeLimits();
  // TODO(nbeato) Merge Ray's CL like this:
  // if (split_part->owns_blobs())
  //   split_part->SetBlobTextlineGoodness();
  return split_part;
}

// Split this partition at the given x coordinate, returning the right
// half and keeping the left half in this.
ColPartition* ColPartition::SplitAt(int split_x) {
  if (split_x <= bounding_box_.left() || split_x >= bounding_box_.right())
    return NULL;  // There will be no change.
  ColPartition* split_part = ShallowCopy();
  split_part->set_owns_blobs(owns_blobs());
  BLOBNBOX_C_IT it(&boxes_);
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    BLOBNBOX* bbox = it.data();
    ColPartition* prev_owner = bbox->owner();
    ASSERT_HOST(!owns_blobs() || prev_owner == this || prev_owner == NULL);
    const TBOX& box = bbox->bounding_box();
    if (box.left() >= split_x) {
      split_part->AddBox(it.extract());
      if (owns_blobs() && prev_owner != NULL)
        bbox->set_owner(split_part);
    }
  }
  ASSERT_HOST(!it.empty());
  if (split_part->IsEmpty()) {
    // Split part ended up with nothing. Possible if split_x passes
    // through the last blob.
    delete split_part;
    return NULL;
  }
  right_key_tab_ = false;
  split_part->left_key_tab_ = false;
  right_margin_ = split_x;
  split_part->left_margin_ = split_x;
  ComputeLimits();
  split_part->ComputeLimits();
  return split_part;
}

// Recalculates all the coordinate limits of the partition.
void ColPartition::ComputeLimits() {
  bounding_box_ = TBOX();  // Clear it
  BLOBNBOX_C_IT it(&boxes_);
  BLOBNBOX* bbox = NULL;
  int non_leader_count = 0;
  if (it.empty()) {
    bounding_box_.set_left(left_margin_);
    bounding_box_.set_right(right_margin_);
    bounding_box_.set_bottom(0);
    bounding_box_.set_top(0);
  } else {
    for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
      bbox = it.data();
      bounding_box_ += bbox->bounding_box();
      if (bbox->flow() != BTFT_LEADER)
        ++non_leader_count;
    }
  }
  if (!left_key_tab_)
    left_key_ = BoxLeftKey();
  if (left_key_ > BoxLeftKey() && textord_debug_bugs) {
    // TODO(rays) investigate the causes of these error messages, to find
    // out if they are genuinely harmful, or just indicative of junk input.
    tprintf("Computed left-illegal partition\n");
    Print();
  }
  if (!right_key_tab_)
    right_key_ = BoxRightKey();
  if (right_key_ < BoxRightKey() && textord_debug_bugs) {
    tprintf("Computed right-illegal partition\n");
    Print();
  }
  if (it.empty())
    return;
  if (IsImageType() || blob_type() == BRT_RECTIMAGE ||
      blob_type() == BRT_POLYIMAGE) {
    median_top_ = bounding_box_.top();
    median_bottom_ = bounding_box_.bottom();
    median_size_ = bounding_box_.height();
    median_left_ = bounding_box_.left();
    median_right_ = bounding_box_.right();
    median_width_ = bounding_box_.width();
  } else {
    STATS top_stats(bounding_box_.bottom(), bounding_box_.top() + 1);
    STATS bottom_stats(bounding_box_.bottom(), bounding_box_.top() + 1);
    STATS size_stats(0, bounding_box_.height() + 1);
    STATS left_stats(bounding_box_.left(), bounding_box_.right() + 1);
    STATS right_stats(bounding_box_.left(), bounding_box_.right() + 1);
    STATS width_stats(0, bounding_box_.width() + 1);
    for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
      bbox = it.data();
      if (non_leader_count == 0 || bbox->flow() != BTFT_LEADER) {
        TBOX box = bbox->bounding_box();
        int area = box.area();
        top_stats.add(box.top(), area);
        bottom_stats.add(box.bottom(), area);
        size_stats.add(box.height(), area);
        left_stats.add(box.left(), area);
        right_stats.add(box.right(), area);
        width_stats.add(box.width(), area);
      }
    }
    median_top_ = static_cast<int>(top_stats.median() + 0.5);
    median_bottom_ = static_cast<int>(bottom_stats.median() + 0.5);
    median_size_ = static_cast<int>(size_stats.median() + 0.5);
    median_left_ = static_cast<int>(left_stats.median() + 0.5);
    median_right_ = static_cast<int>(right_stats.median() + 0.5);
    median_width_ = static_cast<int>(width_stats.median() + 0.5);
  }

  if (right_margin_ < bounding_box_.right() && textord_debug_bugs) {
    tprintf("Made partition with bad right coords");
    Print();
  }
  if (left_margin_ > bounding_box_.left() && textord_debug_bugs) {
    tprintf("Made partition with bad left coords");
    Print();
  }
  // Fix partner lists. The bounding box has changed and partners are stored
  // in bounding box order, so remove and reinsert this as a partner
  // of all its partners.
  for (int upper = 0; upper < 2; ++upper) {
    ColPartition_CLIST partners;
    ColPartition_C_IT part_it(&partners);
    part_it.add_list_after(upper ? &upper_partners_ : &lower_partners_);
    for (part_it.move_to_first(); !part_it.empty(); part_it.forward()) {
      ColPartition* partner = part_it.extract();
      partner->RemovePartner(!upper, this);
      partner->AddPartner(!upper, this);
    }
  }
  if (TabFind::WithinTestRegion(2, bounding_box_.left(),
                                bounding_box_.bottom())) {
    tprintf("Recomputed box for partition %p\n", this);
    Print();
  }
}

// Returns the number of boxes that overlap the given box.
int ColPartition::CountOverlappingBoxes(const TBOX& box) {
  BLOBNBOX_C_IT it(&boxes_);
  int overlap_count = 0;
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    BLOBNBOX* bbox = it.data();
    if (box.overlap(bbox->bounding_box()))
      ++overlap_count;
  }
  return overlap_count;
}

// Computes and sets the type_ and first_colum_, last_column_ and column_set_.
// resolution refers to the ppi resolution of the image.
void ColPartition::SetPartitionType(int resolution, ColPartitionSet* columns) {
  int first_spanned_col = -1;
  ColumnSpanningType span_type =
      columns->SpanningType(resolution,
                            bounding_box_.left(), bounding_box_.right(),
                            MidY(), left_margin_, right_margin_,
                            &first_column_, &last_column_,
                            &first_spanned_col);
  column_set_ = columns;
  if (first_column_ < last_column_ && span_type == CST_PULLOUT &&
      !IsLineType()) {
    // Unequal columns may indicate that the pullout spans one of the columns
    // it lies in, so force it to be allocated to just that column.
    if (first_spanned_col >= 0) {
      first_column_ = first_spanned_col;
      last_column_ = first_spanned_col;
    } else {
      if ((first_column_ & 1) == 0)
        last_column_ = first_column_;
      else if ((last_column_ & 1) == 0)
        first_column_ = last_column_;
      else
        first_column_ = last_column_ = (first_column_ + last_column_) / 2;
    }
  }
  type_ = PartitionType(span_type);
}

// Returns the PartitionType from the current BlobRegionType and a column
// flow spanning type ColumnSpanningType, generated by
// ColPartitionSet::SpanningType, that indicates how the partition sits
// in the columns.
PolyBlockType ColPartition::PartitionType(ColumnSpanningType flow) const {
  if (flow == CST_NOISE) {
    if (blob_type_ != BRT_HLINE && blob_type_ != BRT_VLINE &&
        blob_type_ != BRT_RECTIMAGE && blob_type_ != BRT_VERT_TEXT)
      return PT_NOISE;
    flow = CST_FLOWING;
  }

  switch (blob_type_) {
    case BRT_NOISE:
      return PT_NOISE;
    case BRT_HLINE:
      return PT_HORZ_LINE;
    case BRT_VLINE:
      return PT_VERT_LINE;
    case BRT_RECTIMAGE:
    case BRT_POLYIMAGE:
      switch (flow) {
        case CST_FLOWING:
          return PT_FLOWING_IMAGE;
        case CST_HEADING:
          return PT_HEADING_IMAGE;
        case CST_PULLOUT:
          return PT_PULLOUT_IMAGE;
        default:
          ASSERT_HOST(!"Undefined flow type for image!");
      }
      break;
    case BRT_VERT_TEXT:
      return PT_VERTICAL_TEXT;
    case BRT_TEXT:
    case BRT_UNKNOWN:
    default:
      switch (flow) {
        case CST_FLOWING:
          return PT_FLOWING_TEXT;
        case CST_HEADING:
          return PT_HEADING_TEXT;
        case CST_PULLOUT:
          return PT_PULLOUT_TEXT;
        default:
          ASSERT_HOST(!"Undefined flow type for text!");
      }
  }
  ASSERT_HOST(!"Should never get here!");
  return PT_NOISE;
}

// Returns the first and last column touched by this partition.
// resolution refers to the ppi resolution of the image.
void ColPartition::ColumnRange(int resolution, ColPartitionSet* columns,
                               int* first_col, int* last_col) {
  int first_spanned_col = -1;
  ColumnSpanningType span_type =
      columns->SpanningType(resolution,
                            bounding_box_.left(), bounding_box_.right(),
                            MidY(), left_margin_, right_margin_,
                            first_col, last_col,
                            &first_spanned_col);
  type_ = PartitionType(span_type);
}

// Sets the internal flags good_width_ and good_column_.
void ColPartition::SetColumnGoodness(WidthCallback* cb) {
  int y = MidY();
  int width = RightAtY(y) - LeftAtY(y);
  good_width_ = cb->Run(width);
  good_column_ = blob_type_ == BRT_TEXT && left_key_tab_ && right_key_tab_;
}

// Determines whether the blobs in this partition mostly represent
// a leader (fixed pitch sequence) and sets the member blobs accordingly.
// Note that height is assumed to have been tested elsewhere, and that this
// function will find most fixed-pitch text as leader without a height filter.
// Leader detection is limited to sequences of identical width objects,
// such as .... or ----, so patterns, such as .-.-.-.-. will not be found.
bool ColPartition::MarkAsLeaderIfMonospaced() {
  bool result = false;
  // Gather statistics on the gaps between blobs and the widths of the blobs.
  int part_width = bounding_box_.width();
  STATS gap_stats(0, part_width);
  STATS width_stats(0, part_width);
  BLOBNBOX_C_IT it(&boxes_);
  BLOBNBOX* prev_blob = it.data();
  prev_blob->set_flow(BTFT_NEIGHBOURS);
  width_stats.add(prev_blob->bounding_box().width(), 1);
  int blob_count = 1;
  for (it.forward(); !it.at_first(); it.forward()) {
    BLOBNBOX* blob = it.data();
    int left = blob->bounding_box().left();
    int right = blob->bounding_box().right();
    gap_stats.add(left - prev_blob->bounding_box().right(), 1);
    width_stats.add(right - left, 1);
    blob->set_flow(BTFT_NEIGHBOURS);
    prev_blob = blob;
    ++blob_count;
  }
  double median_gap = gap_stats.median();
  double median_width = width_stats.median();
  double max_width = MAX(median_gap, median_width);
  double min_width = MIN(median_gap, median_width);
  double gap_iqr = gap_stats.ile(0.75f) - gap_stats.ile(0.25f);
  if (textord_debug_tabfind >= 4) {
    tprintf("gap iqr = %g, blob_count=%d, limits=%g,%g\n",
            gap_iqr, blob_count, max_width * kMaxLeaderGapFractionOfMax,
            min_width * kMaxLeaderGapFractionOfMin);
  }
  if (gap_iqr < max_width * kMaxLeaderGapFractionOfMax &&
      gap_iqr < min_width * kMaxLeaderGapFractionOfMin &&
      blob_count >= kMinLeaderCount) {
    // This is stable enough to be called a leader, so check the widths.
    // Since leader dashes can join, run a dp cutting algorithm and go
    // on the cost.
    int offset = static_cast<int>(ceil(gap_iqr * 2));
    int min_step = static_cast<int>(median_gap + median_width + 0.5);
    int max_step = min_step + offset;
    min_step -= offset;
    // Pad the buffer with min_step/2 on each end.
    int part_left = bounding_box_.left() - min_step / 2;
    part_width += min_step;
    DPPoint* projection = new DPPoint[part_width];
    for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
      BLOBNBOX* blob = it.data();
      int left = blob->bounding_box().left();
      int right = blob->bounding_box().right();
      int height = blob->bounding_box().height();
      for (int x = left; x < right; ++x) {
        projection[left - part_left].AddLocalCost(height);
      }
    }
    DPPoint* best_end = DPPoint::Solve(min_step, max_step, false,
                                       &DPPoint::CostWithVariance,
                                       part_width, projection);
    if (best_end != NULL && best_end->total_cost() < blob_count) {
      // Good enough. Call it a leader.
      result = true;
      for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
        BLOBNBOX* blob = it.data();
        TBOX box = blob->bounding_box();
        // If the first or last blob is spaced too much, don't mark it.
        if (it.at_first()) {
          int gap = it.data_relative(1)->bounding_box().left() -
                     blob->bounding_box().right();
          if (blob->bounding_box().width() + gap > max_step) {
            it.extract();
            continue;
          }
        }
        if (it.at_last()) {
          int gap = blob->bounding_box().left() -
                     it.data_relative(-1)->bounding_box().right();
          if (blob->bounding_box().width() + gap > max_step) {
            it.extract();
            break;
          }
        }
        blob->set_region_type(BRT_TEXT);
        blob->set_flow(BTFT_LEADER);
      }
      blob_type_ = BRT_TEXT;
      flow_ = BTFT_LEADER;
    } else if (textord_debug_tabfind) {
      if (best_end == NULL) {
        tprintf("No path\n");
      } else {
        tprintf("Total cost = %d vs allowed %d\n",
                best_end->total_cost() < blob_count);
      }
    }
    delete [] projection;
  }
  return result;
}

// Given the result of TextlineProjection::EvaluateColPartition, (positive for
// horizontal text, negative for vertical text, and near zero for non-text),
// sets the blob_type_ and flow_ for this partition to indicate whether it
// is strongly or weakly vertical or horizontal text, or non-text.
// The function assumes that the blob neighbours are valid (from
// StrokeWidth::SetNeighbours) and that those neighbours have their
// region_type() set.
void ColPartition::SetRegionAndFlowTypesFromProjectionValue(int value) {
  int blob_count = 0;        // Total # blobs.
  int good_blob_score_ = 0;  // Total # good strokewidth neighbours.
  int noisy_count = 0;       // Total # neighbours marked as noise.
  int hline_count = 0;
  int vline_count = 0;
  BLOBNBOX_C_IT it(&boxes_);
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    BLOBNBOX* blob = it.data();
    ++blob_count;
    noisy_count += blob->NoisyNeighbours();
    good_blob_score_ += blob->GoodTextBlob();
    if (blob->region_type() == BRT_HLINE) ++hline_count;
    if (blob->region_type() == BRT_VLINE) ++vline_count;
  }
  flow_ = BTFT_NEIGHBOURS;
  blob_type_ = BRT_UNKNOWN;
  if (hline_count > vline_count) {
    flow_ = BTFT_NONE;
    blob_type_ = BRT_HLINE;
  } else if (vline_count > hline_count) {
    flow_ = BTFT_NONE;
    blob_type_ = BRT_VLINE;
  } else if (value < -1 || 1 < value) {
    int long_side;
    int short_side;
    if (value > 0) {
      long_side = bounding_box_.width();
      short_side = bounding_box_.height();
      blob_type_ = BRT_TEXT;
    } else {
      long_side = bounding_box_.height();
      short_side = bounding_box_.width();
      blob_type_ = BRT_VERT_TEXT;
    }
    // We will combine the old metrics using aspect ratio and blob counts
    // with the input value by allowing a strong indication to flip the
    // STRONG_CHAIN/CHAIN flow values.
    int strong_score = blob_count >= kHorzStrongTextlineCount ? 1 : 0;
    if (short_side > kHorzStrongTextlineHeight) ++strong_score;
    if (short_side * kHorzStrongTextlineAspect < long_side) ++strong_score;
    if (abs(value) >= kMinStrongTextValue)
      flow_ = BTFT_STRONG_CHAIN;
    else if (abs(value) >= kMinChainTextValue)
      flow_ = BTFT_CHAIN;
    else
      flow_ = BTFT_NEIGHBOURS;
    // Upgrade chain to strong chain if the other indicators are good
    if (flow_ == BTFT_CHAIN && strong_score == 3)
      flow_ = BTFT_STRONG_CHAIN;
    // Downgrade strong vertical text to chain if the indicators are bad.
    if (flow_ == BTFT_STRONG_CHAIN && value < 0 && strong_score < 2)
      flow_ = BTFT_CHAIN;
  }
  if (flow_ == BTFT_NEIGHBOURS) {
    // Check for noisy neighbours.
    if (noisy_count >= blob_count) {
      flow_ = BTFT_NONTEXT;
      blob_type_= BRT_NOISE;
    }
  }
  if (TabFind::WithinTestRegion(2, bounding_box_.left(),
                                bounding_box_.bottom())) {
    tprintf("RegionFlowTypesFromProjectionValue count=%d, noisy=%d, score=%d,",
            blob_count, noisy_count, good_blob_score_);
    tprintf(" Projection value=%d, flow=%d, blob_type=%d\n",
            value, flow_, blob_type_);
    Print();
  }
  SetBlobTypes();
}

// Sets all blobs with the partition blob type and flow, but never overwrite
// leader blobs, as we need to be able to identify them later.
void ColPartition::SetBlobTypes() {
  if (!owns_blobs())
    return;
  BLOBNBOX_C_IT it(&boxes_);
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    BLOBNBOX* blob = it.data();
    if (blob->flow() != BTFT_LEADER)
      blob->set_flow(flow_);
    blob->set_region_type(blob_type_);
    ASSERT_HOST(blob->owner() == NULL || blob->owner() == this);
  }
}

// Returns true if a decent baseline can be fitted through the blobs.
// Works for both horizontal and vertical text.
bool ColPartition::HasGoodBaseline() {
  // Approximation of the baseline.
  DetLineFit linepoints;
  // Calculation of the mean height on this line segment. Note that these
  // variable names apply to the context of a horizontal line, and work
  // analogously, rather than literally in the case of a vertical line.
  int total_height = 0;
  int coverage = 0;
  int height_count = 0;
  int width = 0;
  BLOBNBOX_C_IT it(&boxes_);
  TBOX box(it.data()->bounding_box());
  // Accumulate points representing the baseline at the middle of each blob,
  // but add an additional point for each end of the line. This makes it
  // harder to fit a severe skew angle, as it is most likely not right.
  if (IsVerticalType()) {
    // For a vertical line, use the right side as the baseline.
    ICOORD first_pt(box.right(), box.bottom());
    // Use the bottom-right of the first (bottom) box, the top-right of the
    // last, and the middle-right of all others.
    linepoints.Add(first_pt);
    for (it.forward(); !it.at_last(); it.forward()) {
      BLOBNBOX* blob = it.data();
      box = blob->bounding_box();
      ICOORD box_pt(box.right(), (box.top() + box.bottom()) / 2);
      linepoints.Add(box_pt);
      total_height += box.width();
      coverage += box.height();
      ++height_count;
    }
    box = it.data()->bounding_box();
    ICOORD last_pt(box.right(), box.top());
    linepoints.Add(last_pt);
    width = last_pt.y() - first_pt.y();

  } else {
    // Horizontal lines use the bottom as the baseline.
    TBOX box(it.data()->bounding_box());
    // Use the bottom-left of the first box, the the bottom-right of the last,
    // and the middle of all others.
    ICOORD first_pt(box.left(), box.bottom());
    linepoints.Add(first_pt);
    for (it.forward(); !it.at_last(); it.forward()) {
      BLOBNBOX* blob = it.data();
      box = blob->bounding_box();
      ICOORD box_pt((box.left() + box.right()) / 2, box.bottom());
      linepoints.Add(box_pt);
      total_height += box.height();
      coverage += box.width();
      ++height_count;
    }
    box = it.data()->bounding_box();
    ICOORD last_pt(box.right(), box.bottom());
    linepoints.Add(last_pt);
    width = last_pt.x() - first_pt.x();
  }
  // Maximum median error allowed to be a good text line.
  double max_error = kMaxBaselineError * total_height / height_count;
  ICOORD start_pt, end_pt;
  double error = linepoints.Fit(&start_pt, &end_pt);
  return error < max_error && coverage >= kMinBaselineCoverage * width;
}

// Adds this ColPartition to a matching WorkingPartSet if one can be found,
// otherwise starts a new one in the appropriate column, ending the previous.
void ColPartition::AddToWorkingSet(const ICOORD& bleft, const ICOORD& tright,
                                   int resolution,
                                   ColPartition_LIST* used_parts,
                                   WorkingPartSet_LIST* working_sets) {
  if (block_owned_)
    return;  // Done it already.
  block_owned_ = true;
  WorkingPartSet_IT it(working_sets);
  // If there is an upper partner use its working_set_ directly.
  ColPartition* partner = SingletonPartner(true);
  if (partner != NULL && partner->working_set_ != NULL) {
    working_set_ = partner->working_set_;
    working_set_->AddPartition(this);
    return;
  }
  if (partner != NULL && textord_debug_bugs) {
    tprintf("Partition with partner has no working set!:");
    Print();
    partner->Print();
  }
  // Search for the column that the left edge fits in.
  WorkingPartSet* work_set = NULL;
  it.move_to_first();
  int col_index = 0;
  for (it.mark_cycle_pt(); !it.cycled_list() &&
       col_index != first_column_;
        it.forward(), ++col_index);
  if (textord_debug_tabfind >= 2) {
    tprintf("Match is %s for:", (col_index & 1) ? "Real" : "Between");
    Print();
  }
  if (it.cycled_list() && textord_debug_bugs) {
    tprintf("Target column=%d, only had %d\n", first_column_, col_index);
  }
  ASSERT_HOST(!it.cycled_list());
  work_set = it.data();
  // If last_column_ != first_column, then we need to scoop up all blocks
  // between here and the last_column_ and put back in work_set.
  if (!it.cycled_list() && last_column_ != first_column_) {
    // Find the column that the right edge falls in.
    BLOCK_LIST completed_blocks;
    TO_BLOCK_LIST to_blocks;
    for (; !it.cycled_list() && col_index <= last_column_;
         it.forward(), ++col_index) {
      WorkingPartSet* end_set = it.data();
      end_set->ExtractCompletedBlocks(bleft, tright, resolution, used_parts,
                                      &completed_blocks, &to_blocks);
    }
    work_set->InsertCompletedBlocks(&completed_blocks, &to_blocks);
  }
  working_set_ = work_set;
  work_set->AddPartition(this);
}

// From the given block_parts list, builds one or more BLOCKs and
// corresponding TO_BLOCKs, such that the line spacing is uniform in each.
// Created blocks are appended to the end of completed_blocks and to_blocks.
// The used partitions are put onto used_parts, as they may still be referred
// to in the partition grid. bleft, tright and resolution are the bounds
// and resolution of the original image.
void ColPartition::LineSpacingBlocks(const ICOORD& bleft, const ICOORD& tright,
                                     int resolution,
                                     ColPartition_LIST* block_parts,
                                     ColPartition_LIST* used_parts,
                                     BLOCK_LIST* completed_blocks,
                                     TO_BLOCK_LIST* to_blocks) {
  int page_height = tright.y() - bleft.y();
  // Compute the initial spacing stats.
  ColPartition_IT it(block_parts);
  int part_count = 0;
  int max_line_height = 0;

  // TODO(joeliu): We should add some special logic for PT_INLINE_EQUATION type
  // because their line spacing with their neighbors maybe smaller and their
  // height may be slightly larger.

  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    ColPartition* part = it.data();
    ASSERT_HOST(!part->boxes()->empty());
    STATS side_steps(0, part->bounding_box().height());
    if (part->bounding_box().height() > max_line_height)
      max_line_height = part->bounding_box().height();
    BLOBNBOX_C_IT blob_it(part->boxes());
    int prev_bottom = blob_it.data()->bounding_box().bottom();
    for (blob_it.forward(); !blob_it.at_first(); blob_it.forward()) {
      BLOBNBOX* blob = blob_it.data();
      int bottom = blob->bounding_box().bottom();
      int step = bottom - prev_bottom;
      if (step < 0)
        step = -step;
      side_steps.add(step, 1);
      prev_bottom = bottom;
    }
    part->set_side_step(static_cast<int>(side_steps.median() + 0.5));
    if (!it.at_last()) {
      ColPartition* next_part = it.data_relative(1);
      part->set_bottom_spacing(part->median_bottom() -
                               next_part->median_bottom());
      part->set_top_spacing(part->median_top() - next_part->median_top());
    } else {
      part->set_bottom_spacing(page_height);
      part->set_top_spacing(page_height);
    }
    if (textord_debug_tabfind) {
      part->Print();
      tprintf("side step = %.2f, top spacing = %d, bottom spacing=%d\n",
              side_steps.median(), part->top_spacing(), part->bottom_spacing());
    }
    ++part_count;
  }
  if (part_count == 0)
    return;

  SmoothSpacings(resolution, page_height, block_parts);

  // Move the partitions into individual block lists and make the blocks.
  BLOCK_IT block_it(completed_blocks);
  TO_BLOCK_IT to_block_it(to_blocks);
  ColPartition_LIST spacing_parts;
  ColPartition_IT sp_block_it(&spacing_parts);
  int same_block_threshold = max_line_height * kMaxSameBlockLineSpacing;
  for (it.mark_cycle_pt(); !it.empty();) {
    ColPartition* part = it.extract();
    sp_block_it.add_to_end(part);
    it.forward();
    if (it.empty() || part->bottom_spacing() > same_block_threshold ||
        !part->SpacingsEqual(*it.data(), resolution)) {
      // There is a spacing boundary. Check to see if it.data() belongs
      // better in the current block or the next one.
      if (!it.empty() && part->bottom_spacing() <= same_block_threshold) {
        ColPartition* next_part = it.data();
        // If there is a size match one-way, then the middle line goes with
        // its matched size, otherwise it goes with the smallest spacing.
        ColPartition* third_part = it.at_last() ? NULL : it.data_relative(1);
        if (textord_debug_tabfind) {
          tprintf("Spacings unequal: upper:%d/%d, lower:%d/%d,"
                  " sizes %d %d %d\n",
                  part->top_spacing(), part->bottom_spacing(),
                  next_part->top_spacing(), next_part->bottom_spacing(),
                  part->median_size(), next_part->median_size(),
                  third_part != NULL ? third_part->median_size() : 0);
        }
        // We can only consider adding the next line to the block if the sizes
        // match and the lines are close enough for their size.
        if (part->SizesSimilar(*next_part) &&
            next_part->median_size() * kMaxSameBlockLineSpacing >
                part->bottom_spacing() &&
            part->median_size() * kMaxSameBlockLineSpacing >
                part->top_spacing()) {
          // Even now, we can only add it as long as the third line doesn't
          // match in the same way and have a smaller bottom spacing.
          if (third_part == NULL ||
              !next_part->SizesSimilar(*third_part) ||
              third_part->median_size() * kMaxSameBlockLineSpacing <=
                  next_part->bottom_spacing() ||
              next_part->median_size() * kMaxSameBlockLineSpacing <=
                  next_part->top_spacing() ||
                  next_part->bottom_spacing() > part->bottom_spacing()) {
            // Add to the current block.
            sp_block_it.add_to_end(it.extract());
            it.forward();
            if (textord_debug_tabfind) {
              tprintf("Added line to current block.\n");
            }
          }
        }
      }
      TO_BLOCK* to_block = MakeBlock(bleft, tright, &spacing_parts, used_parts);
      if (to_block != NULL) {
        to_block_it.add_to_end(to_block);
        block_it.add_to_end(to_block->block);
      }
      sp_block_it.set_to_list(&spacing_parts);
    } else {
      if (textord_debug_tabfind && !it.empty()) {
        ColPartition* next_part = it.data();
        tprintf("Spacings equal: upper:%d/%d, lower:%d/%d\n",
                part->top_spacing(), part->bottom_spacing(),
                next_part->top_spacing(), next_part->bottom_spacing(),
                part->median_size(), next_part->median_size());
      }
    }
  }
}

// Helper function to clip the input pos to the given bleft, tright bounds.
static void ClipCoord(const ICOORD& bleft, const ICOORD& tright, ICOORD* pos) {
  if (pos->x() < bleft.x())
    pos->set_x(bleft.x());
  if (pos->x() > tright.x())
    pos->set_x(tright.x());
  if (pos->y() < bleft.y())
    pos->set_y(bleft.y());
  if (pos->y() > tright.y())
    pos->set_y(tright.y());
}

// Helper moves the blobs from the given list of block_parts into the block
// itself. Sets up the block for (old) textline formation correctly for
// vertical and horizontal text. The partitions are moved to used_parts
// afterwards, as they cannot be deleted yet.
static TO_BLOCK* MoveBlobsToBlock(bool vertical_text, int line_spacing,
                                  BLOCK* block,
                                  ColPartition_LIST* block_parts,
                                  ColPartition_LIST* used_parts) {
  // Make a matching TO_BLOCK and put all the BLOBNBOXes from the parts in it.
  // Move all the parts to a done list as they are no longer needed, except
  // that have have to continue to exist until the part grid is deleted.
  // Compute the median blob size as we go, as the block needs to know.
  TBOX block_box(block->bounding_box());
  STATS sizes(0, MAX(block_box.width(), block_box.height()));
  bool text_type = block->poly_block()->IsText();
  ColPartition_IT it(block_parts);
  TO_BLOCK* to_block = new TO_BLOCK(block);
  BLOBNBOX_IT blob_it(&to_block->blobs);
  ColPartition_IT used_it(used_parts);
  for (it.move_to_first(); !it.empty(); it.forward()) {
    ColPartition* part = it.extract();
    // Transfer blobs from all regions to the output blocks.
    // Blobs for non-text regions will be used to define the polygonal
    // bounds of the region.
    for (BLOBNBOX_C_IT bb_it(part->boxes()); !bb_it.empty();
         bb_it.forward()) {
      BLOBNBOX* bblob = bb_it.extract();
      if (bblob->owner() != part) {
        tprintf("Ownership incorrect for blob:");
        bblob->bounding_box().print();
        tprintf("Part=");
        part->Print();
        if (bblob->owner() == NULL) {
          tprintf("Not owned\n");
        } else {
          tprintf("Owner part:");
          bblob->owner()->Print();
        }
      }
      ASSERT_HOST(bblob->owner() == part);
      // Assert failure here is caused by arbitrarily changing the partition
      // type without also changing the blob type, such as in
      // InsertSmallBlobsAsUnknowns.
      ASSERT_HOST(!text_type || bblob->region_type() >= BRT_UNKNOWN);
      C_OUTLINE_LIST* outlines = bblob->cblob()->out_list();
      C_OUTLINE_IT ol_it(outlines);
      if (outlines->singleton()) {
        ASSERT_HOST(!text_type || ol_it.data()->pathlength() > 0);
        if (vertical_text)
          sizes.add(bblob->bounding_box().width(), 1);
        else
          sizes.add(bblob->bounding_box().height(), 1);
        blob_it.add_after_then_move(bblob);
      } else {
        // This blob has multiple outlines from CJK repair.
        // Explode the blob back into individual outlines.
        for (;!ol_it.empty(); ol_it.forward()) {
          C_OUTLINE* outline = ol_it.extract();
          BLOBNBOX* blob = BLOBNBOX::RealBlob(outline);
          if (vertical_text)
            sizes.add(blob->bounding_box().width(), 1);
          else
            sizes.add(blob->bounding_box().height(), 1);
          blob_it.add_after_then_move(blob);
        }
        delete bblob->cblob();
        delete bblob;
      }
    }
    used_it.add_to_end(part);
  }
  if (text_type && blob_it.empty()) {
    delete block;
    delete to_block;
    return NULL;
  }
  to_block->line_size = sizes.median();
  if (vertical_text) {
    int block_width = block->bounding_box().width();
    if (block_width < line_spacing)
      line_spacing = block_width;
    to_block->line_spacing = static_cast<float>(line_spacing);
    to_block->max_blob_size = static_cast<float>(block_width + 1);
  } else {
    int block_height = block->bounding_box().height();
    if (block_height < line_spacing)
      line_spacing = block_height;
    to_block->line_spacing = static_cast<float>(line_spacing);
    to_block->max_blob_size = static_cast<float>(block_height + 1);
  }
  return to_block;
}

// Constructs a block from the given list of partitions.
// Arguments are as LineSpacingBlocks above.
TO_BLOCK* ColPartition::MakeBlock(const ICOORD& bleft, const ICOORD& tright,
                                  ColPartition_LIST* block_parts,
                                  ColPartition_LIST* used_parts) {
  if (block_parts->empty())
    return NULL;  // Nothing to do.
  ColPartition_IT it(block_parts);
  ColPartition* part = it.data();
  PolyBlockType type = part->type();
  if (type == PT_VERTICAL_TEXT)
    return MakeVerticalTextBlock(bleft, tright, block_parts, used_parts);
  // LineSpacingBlocks has handed us a collection of evenly spaced lines and
  // put the average spacing in each partition, so we can just take the
  // linespacing from the first partition.
  int line_spacing = part->bottom_spacing();
  if (line_spacing < part->median_size())
    line_spacing = part->bounding_box().height();
  ICOORDELT_LIST vertices;
  ICOORDELT_IT vert_it(&vertices);
  ICOORD start, end;
  int min_x = MAX_INT32;
  int max_x = -MAX_INT32;
  int min_y = MAX_INT32;
  int max_y = -MAX_INT32;
  int iteration = 0;
  do {
    if (iteration == 0)
      ColPartition::LeftEdgeRun(&it, &start, &end);
    else
      ColPartition::RightEdgeRun(&it, &start, &end);
    ClipCoord(bleft, tright, &start);
    ClipCoord(bleft, tright, &end);
    vert_it.add_after_then_move(new ICOORDELT(start));
    vert_it.add_after_then_move(new ICOORDELT(end));
    UpdateRange(start.x(), &min_x, &max_x);
    UpdateRange(end.x(), &min_x, &max_x);
    UpdateRange(start.y(), &min_y, &max_y);
    UpdateRange(end.y(), &min_y, &max_y);
    if ((iteration == 0 && it.at_first()) ||
        (iteration == 1 && it.at_last())) {
      ++iteration;
      it.move_to_last();
    }
  } while (iteration < 2);
  if (textord_debug_tabfind)
    tprintf("Making block at (%d,%d)->(%d,%d)\n",
            min_x, min_y, max_x, max_y);
  BLOCK* block = new BLOCK("", true, 0, 0, min_x, min_y, max_x, max_y);
  block->set_poly_block(new POLY_BLOCK(&vertices, type));
  return MoveBlobsToBlock(false, line_spacing, block, block_parts, used_parts);
}

// Constructs a block from the given list of vertical text partitions.
// Currently only creates rectangular blocks.
TO_BLOCK* ColPartition::MakeVerticalTextBlock(const ICOORD& bleft,
                                              const ICOORD& tright,
                                              ColPartition_LIST* block_parts,
                                              ColPartition_LIST* used_parts) {
  if (block_parts->empty())
    return NULL;  // Nothing to do.
  ColPartition_IT it(block_parts);
  ColPartition* part = it.data();
  TBOX block_box = part->bounding_box();
  int line_spacing = block_box.width();
  PolyBlockType type = it.data()->type();
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    block_box += it.data()->bounding_box();
  }
  if (textord_debug_tabfind) {
    tprintf("Making block at:");
    block_box.print();
  }
  BLOCK* block = new BLOCK("", true, 0, 0, block_box.left(), block_box.bottom(),
                           block_box.right(), block_box.top());
  block->set_poly_block(new POLY_BLOCK(block_box, type));
  return MoveBlobsToBlock(true, line_spacing, block, block_parts, used_parts);
}

// Returns a copy of everything except the list of boxes. The resulting
// ColPartition is only suitable for keeping in a column candidate list.
ColPartition* ColPartition::ShallowCopy() const {
  ColPartition* part = new ColPartition(blob_type_, vertical_);
  part->left_margin_ = left_margin_;
  part->right_margin_ = right_margin_;
  part->bounding_box_ = bounding_box_;
  memcpy(part->special_blobs_densities_, special_blobs_densities_,
         sizeof(special_blobs_densities_));
  part->median_bottom_ = median_bottom_;
  part->median_top_ = median_top_;
  part->median_size_ = median_size_;
  part->median_left_ = median_left_;
  part->median_right_ = median_right_;
  part->median_width_ = median_width_;
  part->good_width_ = good_width_;
  part->good_column_ = good_column_;
  part->left_key_tab_ = left_key_tab_;
  part->right_key_tab_ = right_key_tab_;
  part->type_ = type_;
  part->flow_ = flow_;
  part->left_key_ = left_key_;
  part->right_key_ = right_key_;
  part->first_column_ = first_column_;
  part->last_column_ = last_column_;
  part->owns_blobs_ = false;
  return part;
}

ColPartition* ColPartition::CopyButDontOwnBlobs() {
  ColPartition* copy = ShallowCopy();
  copy->set_owns_blobs(false);
  BLOBNBOX_C_IT inserter(copy->boxes());
  BLOBNBOX_C_IT traverser(boxes());
  for (traverser.mark_cycle_pt(); !traverser.cycled_list(); traverser.forward())
    inserter.add_after_then_move(traverser.data());
  return copy;
}

#ifndef GRAPHICS_DISABLED
// Provides a color for BBGrid to draw the rectangle.
// Must be kept in sync with PolyBlockType.
ScrollView::Color  ColPartition::BoxColor() const {
  if (type_ == PT_UNKNOWN)
    return BLOBNBOX::TextlineColor(blob_type_, flow_);
  return POLY_BLOCK::ColorForPolyBlockType(type_);
}
#endif  // GRAPHICS_DISABLED

// Keep in sync with BlobRegionType.
static char kBlobTypes[BRT_COUNT + 1] = "NHSRIUVT";

// Prints debug information on this.
void ColPartition::Print() const {
  int y = MidY();
  tprintf("ColPart:%c(M%d-%c%d-B%d/%d,%d/%d)->(%dB-%d%c-%dM/%d,%d/%d)"
          " w-ok=%d, v-ok=%d, type=%d%c%d, fc=%d, lc=%d, boxes=%d"
          " ts=%d bs=%d ls=%d rs=%d\n",
          boxes_.empty() ? 'E' : ' ',
          left_margin_, left_key_tab_ ? 'T' : 'B', LeftAtY(y),
          bounding_box_.left(), median_left_,
          bounding_box_.bottom(), median_bottom_,
          bounding_box_.right(), RightAtY(y), right_key_tab_ ? 'T' : 'B',
          right_margin_, median_right_, bounding_box_.top(), median_top_,
          good_width_, good_column_, type_,
          kBlobTypes[blob_type_], flow_,
          first_column_, last_column_, boxes_.length(),
          space_above_, space_below_, space_to_left_, space_to_right_);
}

// Prints debug information on the colors.
void ColPartition::PrintColors() {
  tprintf("Colors:(%d, %d, %d)%d -> (%d, %d, %d)\n",
          color1_[COLOR_RED], color1_[COLOR_GREEN], color1_[COLOR_BLUE],
          color1_[L_ALPHA_CHANNEL],
          color2_[COLOR_RED], color2_[COLOR_GREEN], color2_[COLOR_BLUE]);
}

// Sets the types of all partitions in the run to be the max of the types.
void ColPartition::SmoothPartnerRun(int working_set_count) {
  STATS left_stats(0, working_set_count);
  STATS right_stats(0, working_set_count);
  PolyBlockType max_type = type_;
  ColPartition* partner;
  for (partner = SingletonPartner(false); partner != NULL;
       partner = partner->SingletonPartner(false)) {
    if (partner->type_ > max_type)
      max_type = partner->type_;
    if (column_set_ == partner->column_set_) {
      left_stats.add(partner->first_column_, 1);
      right_stats.add(partner->last_column_, 1);
    }
  }
  type_ = max_type;
  // TODO(rays) Either establish that it isn't necessary to set the columns,
  // or find a way to do it that does not cause an assert failure in
  // AddToWorkingSet.
#if 0
  first_column_ = left_stats.mode();
  last_column_ = right_stats.mode();
  if (last_column_ < first_column_)
    last_column_ = first_column_;
#endif

  for (partner = SingletonPartner(false); partner != NULL;
       partner = partner->SingletonPartner(false)) {
    partner->type_ = max_type;
#if 0  // See TODO above
    if (column_set_ == partner->column_set_) {
      partner->first_column_ = first_column_;
      partner->last_column_ = last_column_;
    }
#endif
  }
}

// ======= Scenario common to all Refine*Partners* functions =======
// ColPartitions are aiming to represent textlines, or horizontal slices
// of images, and we are trying to form bi-directional (upper/lower) chains
// of UNIQUE partner ColPartitions that can be made into blocks.
// The ColPartitions have previously been typed (see SetPartitionType)
// according to a combination of the content type and
// how they lie on the columns. We want to chain text into
// groups of a single type, but image ColPartitions may have been typed
// differently in different parts of the image, due to being non-rectangular.
//
// We previously ran a search for upper and lower partners, but there may
// be more than one, and they may be of mixed types, so now we wish to
// refine the partners down to at most one.
// A heading may have multiple partners:
// ===============================
// ========  ==========  =========
// ========  ==========  =========
// but it should be a different type.
// A regular flowing text line may have multiple partners:
// ==================   ===================
// =======   =================  ===========
// This could be the start of a pull-out, or it might all be in a single
// column and might be caused by tightly spaced text, bold words, bullets,
// funny punctuation etc, all of which can cause textlines to be split into
// multiple ColPartitions. Pullouts and figure captions should now be different
// types so we can more aggressively merge groups of partners that all sit
// in a single column.
//
// Cleans up the partners of the given type so that there is at most
// one partner. This makes block creation simpler.
// If get_desperate is true, goes to more desperate merge methods
// to merge flowing text before breaking partnerships.
void ColPartition::RefinePartners(PolyBlockType type, bool get_desperate,
                                  ColPartitionGrid* grid) {
  if (TypesSimilar(type_, type)) {
    RefinePartnersInternal(true, get_desperate, grid);
    RefinePartnersInternal(false, get_desperate, grid);
  } else if (type == PT_COUNT) {
    // This is the final pass. Make sure only the correctly typed
    // partners surivive, however many there are.
    RefinePartnersByType(true, &upper_partners_);
    RefinePartnersByType(false, &lower_partners_);
    // It is possible for a merge to have given a partition multiple
    // partners again, so the last resort is to use overlap which is
    // guaranteed to leave at most one partner left.
    if (!upper_partners_.empty() && !upper_partners_.singleton())
      RefinePartnersByOverlap(true, &upper_partners_);
    if (!lower_partners_.empty() && !lower_partners_.singleton())
      RefinePartnersByOverlap(false, &lower_partners_);
  }
}

////////////////// PRIVATE CODE /////////////////////////////

// Cleans up the partners above if upper is true, else below.
// If get_desperate is true, goes to more desperate merge methods
// to merge flowing text before breaking partnerships.
void ColPartition::RefinePartnersInternal(bool upper, bool get_desperate,
                                          ColPartitionGrid* grid) {
  ColPartition_CLIST* partners = upper ? &upper_partners_ : &lower_partners_;
  if (!partners->empty() && !partners->singleton()) {
    RefinePartnersByType(upper, partners);
    if (!partners->empty() && !partners->singleton()) {
      // Check for transitive partnerships and break the cycle.
      RefinePartnerShortcuts(upper, partners);
      if (!partners->empty() && !partners->singleton()) {
        // Types didn't fix it. Flowing text keeps the one with the longest
        // sequence of singleton matching partners. All others max overlap.
        if (TypesSimilar(type_, PT_FLOWING_TEXT) && get_desperate) {
          RefineTextPartnersByMerge(upper, false, partners, grid);
          if (!partners->empty() && !partners->singleton())
            RefineTextPartnersByMerge(upper, true, partners, grid);
        }
        // The last resort is to use overlap.
        if (!partners->empty() && !partners->singleton())
          RefinePartnersByOverlap(upper, partners);
      }
    }
  }
}

// Cleans up the partners above if upper is true, else below.
// Restricts the partners to only desirable types. For text and BRT_HLINE this
// means the same type_ , and for image types it means any image type.
void ColPartition::RefinePartnersByType(bool upper,
                                        ColPartition_CLIST* partners) {
  bool debug = TabFind::WithinTestRegion(2, bounding_box_.left(),
                                         bounding_box_.bottom());
  if (debug) {
    tprintf("Refining %d %s partners by type for:\n",
            partners->length(), upper ? "Upper" : "Lower");
    Print();
  }
  ColPartition_C_IT it(partners);
  // Purify text by type.
  if (!IsImageType()) {
    // Keep only partners matching type_.
    // Exception: PT_VERTICAL_TEXT is allowed to stay with the other
    // text types if it is the only partner.
    for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
      ColPartition* partner = it.data();
      if (!TypesSimilar(type_, partner->type_)) {
        if (debug) {
          tprintf("Removing partner:");
          partner->Print();
        }
        partner->RemovePartner(!upper, this);
        it.extract();
      } else if (debug) {
        tprintf("Keeping partner:");
        partner->Print();
      }
    }
  } else {
    // Only polyimages are allowed to have partners of any kind!
    for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
      ColPartition* partner = it.data();
      if (partner->blob_type() != BRT_POLYIMAGE ||
          blob_type() != BRT_POLYIMAGE) {
        if (debug) {
          tprintf("Removing partner:");
          partner->Print();
        }
        partner->RemovePartner(!upper, this);
        it.extract();
      } else if (debug) {
        tprintf("Keeping partner:");
        partner->Print();
      }
    }
  }
}

// Cleans up the partners above if upper is true, else below.
// Remove transitive partnerships: this<->a, and a<->b and this<->b.
// Gets rid of this<->b, leaving a clean chain.
// Also if we have this<->a and a<->this, then gets rid of this<->a, as
// this has multiple partners.
void ColPartition::RefinePartnerShortcuts(bool upper,
                                          ColPartition_CLIST* partners) {
  bool done_any = false;
  do {
    done_any = false;
    ColPartition_C_IT it(partners);
    for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
      ColPartition* a = it.data();
      // Check for a match between all of a's partners (it1/b1) and all
      // of this's partners (it2/b2).
      ColPartition_C_IT it1(upper ? &a->upper_partners_ : &a->lower_partners_);
      for (it1.mark_cycle_pt(); !it1.cycled_list(); it1.forward()) {
        ColPartition* b1 = it1.data();
        if (b1 == this) {
          done_any = true;
          it.extract();
          a->RemovePartner(!upper, this);
          break;
        }
        ColPartition_C_IT it2(partners);
        for (it2.mark_cycle_pt(); !it2.cycled_list(); it2.forward()) {
          ColPartition* b2 = it2.data();
          if (b1 == b2) {
            // Jackpot! b2 should not be a partner of this.
            it2.extract();
            b2->RemovePartner(!upper, this);
            done_any = true;
            // That potentially invalidated all the iterators, so break out
            // and start again.
            break;
          }
        }
        if (done_any)
          break;
      }
      if (done_any)
        break;
    }
  } while (done_any && !partners->empty() && !partners->singleton());
}

// Cleans up the partners above if upper is true, else below.
// If multiple text partners can be merged, (with each other, NOT with this),
// then do so.
// If desperate is true, then an increase in overlap with the merge is
// allowed. If the overlap increases, then the desperately_merged_ flag
// is set, indicating that the textlines probably need to be regenerated
// by aggressive line fitting/splitting, as there are probably vertically
// joined blobs that cross textlines.
void ColPartition::RefineTextPartnersByMerge(bool upper, bool desperate,
                                             ColPartition_CLIST* partners,
                                             ColPartitionGrid* grid) {
  bool debug = TabFind::WithinTestRegion(2, bounding_box_.left(),
                                         bounding_box_.bottom());
  if (debug) {
    tprintf("Refining %d %s partners by merge for:\n",
            partners->length(), upper ? "Upper" : "Lower");
    Print();
  }
  while (!partners->empty() && !partners->singleton()) {
    // Absorb will mess up the iterators, so we have to merge one partition
    // at a time and rebuild the iterators each time.
    ColPartition_C_IT it(partners);
    ColPartition* part = it.data();
    // Gather a list of merge candidates, from the list of partners, that
    // are all in the same single column. See general scenario comment above.
    ColPartition_CLIST candidates;
    ColPartition_C_IT cand_it(&candidates);
    for (it.forward(); !it.at_first(); it.forward()) {
      ColPartition* candidate = it.data();
      if (part->first_column_ == candidate->last_column_ &&
          part->last_column_ == candidate->first_column_)
        cand_it.add_after_then_move(it.data());
    }
    int overlap_increase;
    ColPartition* candidate = grid->BestMergeCandidate(part, &candidates, debug,
                                                       NULL, &overlap_increase);
    if (candidate != NULL && (overlap_increase <= 0 || desperate)) {
      if (debug) {
        tprintf("Merging:hoverlap=%d, voverlap=%d, OLI=%d\n",
                part->HCoreOverlap(*candidate), part->VCoreOverlap(*candidate),
                overlap_increase);
      }
      // Remove before merge and re-insert to keep the integrity of the grid.
      grid->RemoveBBox(candidate);
      grid->RemoveBBox(part);
      part->Absorb(candidate, NULL);
      // We modified the box of part, so re-insert it into the grid.
      grid->InsertBBox(true, true, part);
      if (overlap_increase > 0)
        part->desperately_merged_ = true;
    } else {
      break;  // Can't merge.
    }
  }
}

// Cleans up the partners above if upper is true, else below.
// Keep the partner with the biggest overlap.
void ColPartition::RefinePartnersByOverlap(bool upper,
                                           ColPartition_CLIST* partners) {
  bool debug = TabFind::WithinTestRegion(2, bounding_box_.left(),
                                         bounding_box_.bottom());
  if (debug) {
    tprintf("Refining %d %s partners by overlap for:\n",
            partners->length(), upper ? "Upper" : "Lower");
    Print();
  }
  ColPartition_C_IT it(partners);
  ColPartition* best_partner = it.data();
  // Find the partner with the best overlap.
  int best_overlap = 0;
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    ColPartition* partner = it.data();
    int overlap = MIN(bounding_box_.right(), partner->bounding_box_.right())
                - MAX(bounding_box_.left(), partner->bounding_box_.left());
    if (overlap > best_overlap) {
      best_overlap = overlap;
      best_partner = partner;
    }
  }
  // Keep only the best partner.
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    ColPartition* partner = it.data();
    if (partner != best_partner) {
      if (debug) {
        tprintf("Removing partner:");
        partner->Print();
      }
      partner->RemovePartner(!upper, this);
      it.extract();
    }
  }
}

// Return true if bbox belongs better in this than other.
bool ColPartition::ThisPartitionBetter(BLOBNBOX* bbox,
                                       const ColPartition& other) {
  TBOX box = bbox->bounding_box();
  // Margins take priority.
  int left = box.left();
  int right = box.right();
  if (left < left_margin_ || right > right_margin_)
    return false;
  if (left < other.left_margin_ || right > other.right_margin_)
    return true;
  int top = box.top();
  int bottom = box.bottom();
  int this_overlap = MIN(top, median_top_) - MAX(bottom, median_bottom_);
  int other_overlap = MIN(top, other.median_top_) -
                      MAX(bottom, other.median_bottom_);
  int this_miss = median_top_ - median_bottom_ - this_overlap;
  int other_miss = other.median_top_ - other.median_bottom_ - other_overlap;
  if (TabFind::WithinTestRegion(3, box.left(), box.bottom())) {
    tprintf("Unique on (%d,%d)->(%d,%d) overlap %d/%d, miss %d/%d, mt=%d/%d\n",
            box.left(), box.bottom(), box.right(), box.top(),
            this_overlap, other_overlap, this_miss, other_miss,
            median_top_, other.median_top_);
  }
  if (this_miss < other_miss)
    return true;
  if (this_miss > other_miss)
    return false;
  if (this_overlap > other_overlap)
    return true;
  if (this_overlap < other_overlap)
    return false;
  return median_top_ >= other.median_top_;
}

// Returns the median line-spacing between the current position and the end
// of the list.
// The iterator is passed by value so the iteration does not modify the
// caller's iterator.
static int MedianSpacing(int page_height, ColPartition_IT it) {
  STATS stats(0, page_height);
  while (!it.cycled_list()) {
    ColPartition* part = it.data();
    it.forward();
    stats.add(part->bottom_spacing(), 1);
    stats.add(part->top_spacing(), 1);
  }
  return static_cast<int>(stats.median() + 0.5);
}

// Returns true if this column partition is in the same column as
// part. This function will only work after the SetPartitionType function
// has been called on both column partitions. This is useful for
// doing a SideSearch when you want things in the same page column.
//
// Currently called by the table detection code to identify if potential table
// partitions exist in the same column.
bool ColPartition::IsInSameColumnAs(const ColPartition& part) const {
  // Overlap does not occur when last < part.first or first > part.last.
  // In other words, one is completely to the side of the other.
  // This is just DeMorgan's law applied to that so the function returns true.
  return (last_column_ >= part.first_column_) &&
         (first_column_ <= part.last_column_);
}

// Smoothes the spacings in the list into groups of equal linespacing.
// resolution is the resolution of the original image, used as a basis
// for thresholds in change of spacing. page_height is in pixels.
void ColPartition::SmoothSpacings(int resolution, int page_height,
                                  ColPartition_LIST* parts) {
  // The task would be trivial if we didn't have to allow for blips -
  // occasional offsets in spacing caused by anomolous text, such as all
  // caps, groups of descenders, joined words, Arabic etc.
  // The neighbourhood stores a consecutive group of partitions so that
  // blips can be detected correctly, yet conservatively enough to not
  // mistake genuine spacing changes for blips. See example below.
  ColPartition* neighbourhood[PN_COUNT];
  ColPartition_IT it(parts);
  it.mark_cycle_pt();
  // Although we know nothing about the spacings is this list, the median is
  // used as an approximation to allow blips.
  // If parts of this block aren't spaced to the median, then we can't
  // accept blips in those parts, but we'll recalculate it each time we
  // split the block, so the median becomes more likely to match all the text.
  int median_space = MedianSpacing(page_height, it);
  ColPartition_IT start_it(it);
  ColPartition_IT end_it(it);
  for (int i = 0; i < PN_COUNT; ++i) {
    if (i < PN_UPPER || it.cycled_list()) {
      neighbourhood[i] = NULL;
    } else {
      if (i == PN_LOWER)
        end_it = it;
      neighbourhood[i] = it.data();
      it.forward();
    }
  }
  while (neighbourhood[PN_UPPER] != NULL) {
    // Test for end of a group. Normally SpacingsEqual is true within a group,
    // but in the case of a blip, it will be false. Here is an example:
    // Line enum   Spacing below (spacing between tops of lines)
    //  1   ABOVE2    20
    //  2   ABOVE1    20
    //  3   UPPER     15
    //  4   LOWER     25
    //  5   BELOW1    20
    //  6   BELOW2    20
    // Line 4 is all in caps (regular caps), so the spacing between line 3
    // and line 4 (looking at the tops) is smaller than normal, and the
    // spacing between line 4 and line 5 is larger than normal, but the
    // two of them add to twice the normal spacing.
    // The following if has to accept unequal spacings 3 times to pass the
    // blip (20/15, 15/25 and 25/20)
    // When the blip is in the middle, OKSpacingBlip tests that one of
    // ABOVE1 and BELOW1 matches the median.
    // The first time, everything is shifted down 1, so we present
    // OKSpacingBlip with neighbourhood+1 and check that PN_UPPER is median.
    // The last time, everything is shifted up 1, so we present OKSpacingBlip
    // with neighbourhood-1 and check that PN_LOWER matches the median.
    if (neighbourhood[PN_LOWER] == NULL ||
        (!neighbourhood[PN_UPPER]->SpacingsEqual(*neighbourhood[PN_LOWER],
                                                 resolution) &&
         !OKSpacingBlip(resolution, median_space, neighbourhood) &&
         (!OKSpacingBlip(resolution, median_space, neighbourhood - 1) ||
          !neighbourhood[PN_LOWER]->SpacingEqual(median_space, resolution)) &&
         (!OKSpacingBlip(resolution, median_space, neighbourhood + 1) ||
          !neighbourhood[PN_UPPER]->SpacingEqual(median_space, resolution)))) {
      // The group has ended. PN_UPPER is the last member.
      // Compute the mean spacing over the group.
      ColPartition_IT sum_it(start_it);
      ColPartition* last_part = neighbourhood[PN_UPPER];
      double total_bottom = 0.0;
      double total_top = 0.0;
      int total_count = 0;
      ColPartition* upper = sum_it.data();
      // We do not process last_part, as its spacing is different.
      while (upper != last_part) {
        total_bottom += upper->bottom_spacing();
        total_top += upper->top_spacing();
        ++total_count;
        sum_it.forward();
        upper = sum_it.data();
      }
      if (total_count > 0) {
        // There were at least 2 lines, so set them all to the mean.
        int top_spacing = static_cast<int>(total_top / total_count + 0.5);
        int bottom_spacing = static_cast<int>(total_bottom / total_count + 0.5);
        if (textord_debug_tabfind) {
          tprintf("Spacing run ended. Cause:");
          if (neighbourhood[PN_LOWER] == NULL) {
            tprintf("No more lines\n");
          } else {
            tprintf("Spacing change. Spacings:\n");
            for (int i = 0; i < PN_COUNT; ++i) {
              if (neighbourhood[i] == NULL) {
                tprintf("NULL");
                if (i > 0 && neighbourhood[i - 1] != NULL) {
                  if (neighbourhood[i - 1]->SingletonPartner(false) != NULL) {
                    tprintf(" Lower partner:");
                    neighbourhood[i - 1]->SingletonPartner(false)->Print();
                  } else {
                    tprintf(" NULL lower partner:\n");
                  }
                } else {
                  tprintf("\n");
                }
              } else {
                tprintf("Top = %d, bottom = %d\n",
                        neighbourhood[i]->top_spacing(),
                        neighbourhood[i]->bottom_spacing());
              }
            }
          }
          tprintf("Mean spacing = %d/%d\n", top_spacing, bottom_spacing);
        }
        sum_it = start_it;
        upper = sum_it.data();
        while (upper != last_part) {
          upper->set_top_spacing(top_spacing);
          upper->set_bottom_spacing(bottom_spacing);
          if (textord_debug_tabfind) {
            tprintf("Setting mean on:");
            upper->Print();
          }
          sum_it.forward();
          upper = sum_it.data();
        }
      }
      // PN_LOWER starts the next group and end_it is the next start_it.
      start_it = end_it;
      // Recalculate the median spacing to maximize the chances of detecting
      // spacing blips.
      median_space = MedianSpacing(page_height, end_it);
    }
    // Shuffle pointers.
    for (int j = 1; j < PN_COUNT; ++j) {
      neighbourhood[j - 1] = neighbourhood[j];
    }
    if (it.cycled_list()) {
      neighbourhood[PN_COUNT - 1] = NULL;
    } else {
      neighbourhood[PN_COUNT - 1] = it.data();
      it.forward();
    }
    end_it.forward();
  }
}

// Returns true if the parts array of pointers to partitions matches the
// condition for a spacing blip. See SmoothSpacings for what this means
// and how it is used.
bool ColPartition::OKSpacingBlip(int resolution, int median_spacing,
                                 ColPartition** parts) {
  if (parts[PN_UPPER] == NULL || parts[PN_LOWER] == NULL)
    return false;
  // The blip is OK if upper and lower sum to an OK value and at least
  // one of above1 and below1 is equal to the median.
  return parts[PN_UPPER]->SummedSpacingOK(*parts[PN_LOWER],
                                          median_spacing, resolution) &&
         ((parts[PN_ABOVE1] != NULL &&
           parts[PN_ABOVE1]->SpacingEqual(median_spacing, resolution)) ||
          (parts[PN_BELOW1] != NULL &&
           parts[PN_BELOW1]->SpacingEqual(median_spacing, resolution)));
}

// Returns true if both the top and bottom spacings of this match the given
// spacing to within suitable margins dictated by the image resolution.
bool ColPartition::SpacingEqual(int spacing, int resolution) const {
  int bottom_error = BottomSpacingMargin(resolution);
  int top_error = TopSpacingMargin(resolution);
  return NearlyEqual(bottom_spacing_, spacing, bottom_error) &&
         NearlyEqual(top_spacing_, spacing, top_error);
}

// Returns true if both the top and bottom spacings of this and other
// match to within suitable margins dictated by the image resolution.
bool ColPartition::SpacingsEqual(const ColPartition& other,
                                 int resolution) const {
  int bottom_error = MAX(BottomSpacingMargin(resolution),
                         other.BottomSpacingMargin(resolution));
  int top_error = MAX(TopSpacingMargin(resolution),
                      other.TopSpacingMargin(resolution));
  return NearlyEqual(bottom_spacing_, other.bottom_spacing_, bottom_error) &&
         (NearlyEqual(top_spacing_, other.top_spacing_, top_error) ||
          NearlyEqual(top_spacing_ + other.top_spacing_, bottom_spacing_ * 2,
                      bottom_error));
}

// Returns true if the sum spacing of this and other match the given
// spacing (or twice the given spacing) to within a suitable margin dictated
// by the image resolution.
bool ColPartition::SummedSpacingOK(const ColPartition& other,
                                   int spacing, int resolution) const {
  int bottom_error = MAX(BottomSpacingMargin(resolution),
                         other.BottomSpacingMargin(resolution));
  int top_error = MAX(TopSpacingMargin(resolution),
                      other.TopSpacingMargin(resolution));
  int bottom_total = bottom_spacing_ + other.bottom_spacing_;
  int top_total = top_spacing_ + other.top_spacing_;
  return (NearlyEqual(spacing, bottom_total, bottom_error) &&
          NearlyEqual(spacing, top_total, top_error)) ||
         (NearlyEqual(spacing * 2, bottom_total, bottom_error) &&
          NearlyEqual(spacing * 2, top_total, top_error));
}

// Returns a suitable spacing margin that can be applied to bottoms of
// text lines, based on the resolution and the stored side_step_.
int ColPartition::BottomSpacingMargin(int resolution) const {
  return static_cast<int>(kMaxSpacingDrift * resolution + 0.5) + side_step_;
}

// Returns a suitable spacing margin that can be applied to tops of
// text lines, based on the resolution and the stored side_step_.
int ColPartition::TopSpacingMargin(int resolution) const {
  return static_cast<int>(kMaxTopSpacingFraction * median_size_ + 0.5) +
         BottomSpacingMargin(resolution);
}

// Returns true if the median text sizes of this and other agree to within
// a reasonable multiplicative factor.
bool ColPartition::SizesSimilar(const ColPartition& other) const {
  return median_size_ <= other.median_size_ * kMaxSizeRatio &&
         other.median_size_ <= median_size_ * kMaxSizeRatio;
}

// Helper updates margin_left and margin_right, being the bounds of the left
// margin of part of a block. Returns false and does not update the bounds if
// this partition has a disjoint margin with the established margin.
static bool UpdateLeftMargin(const ColPartition& part,
                             int* margin_left, int* margin_right) {
  const TBOX& part_box = part.bounding_box();
  int top = part_box.top();
  int bottom = part_box.bottom();
  int tl_key = part.SortKey(part.left_margin(), top);
  int tr_key = part.SortKey(part_box.left(), top);
  int bl_key = part.SortKey(part.left_margin(), bottom);
  int br_key = part.SortKey(part_box.left(), bottom);
  int left_key = MAX(tl_key, bl_key);
  int right_key = MIN(tr_key, br_key);
  if (left_key <= *margin_right && right_key >= *margin_left) {
    // This part is good - let's keep it.
    *margin_right = MIN(*margin_right, right_key);
    *margin_left = MAX(*margin_left, left_key);
    return true;
  }
  return false;
}

// Computes and returns in start, end a line segment formed from a
// forwards-iterated group of left edges of partitions that satisfy the
// condition that the intersection of the left margins is non-empty, ie the
// rightmost left margin is to the left of the leftmost left bounding box edge.
// On return the iterator is set to the start of the next run.
void ColPartition::LeftEdgeRun(ColPartition_IT* part_it,
                               ICOORD* start, ICOORD* end) {
  ColPartition* part = part_it->data();
  ColPartition* start_part = part;
  int start_y = part->bounding_box_.top();
  if (!part_it->at_first()) {
    int prev_bottom = part_it->data_relative(-1)->bounding_box_.bottom();
    if (prev_bottom < start_y)
      start_y = prev_bottom;
    else if (prev_bottom > start_y)
      start_y = (start_y + prev_bottom) / 2;
  }
  int end_y = part->bounding_box_.bottom();
  int margin_right = MAX_INT32;
  int margin_left = -MAX_INT32;
  UpdateLeftMargin(*part, &margin_left, &margin_right);
  do {
    part_it->forward();
    part = part_it->data();
  } while (!part_it->at_first() &&
           UpdateLeftMargin(*part, &margin_left, &margin_right));
  // The run ended. If we were pushed inwards, compute the next run and
  // extend it backwards into the run we just calculated to find the end of
  // this run that provides a tight box.
  int next_margin_right = MAX_INT32;
  int next_margin_left = -MAX_INT32;
  UpdateLeftMargin(*part, &next_margin_left, &next_margin_right);
  if (next_margin_left > margin_right) {
    ColPartition_IT next_it(*part_it);
    do {
      next_it.forward();
      part = next_it.data();
    } while (!next_it.at_first() &&
             UpdateLeftMargin(*part, &next_margin_left, &next_margin_right));
    // Now extend the next run backwards into the original run to get the
    // tightest fit.
    do {
      part_it->backward();
      part = part_it->data();
    } while (part != start_part &&
             UpdateLeftMargin(*part, &next_margin_left, &next_margin_right));
    part_it->forward();
  }
  // Now calculate the end_y.
  part = part_it->data_relative(-1);
  end_y = part->bounding_box_.bottom();
  if (!part_it->at_first() && part_it->data()->bounding_box_.top() < end_y)
    end_y = (end_y + part_it->data()->bounding_box_.top()) / 2;
  start->set_y(start_y);
  start->set_x(part->XAtY(margin_right, start_y));
  end->set_y(end_y);
  end->set_x(part->XAtY(margin_right, end_y));
  if (textord_debug_tabfind && !part_it->at_first())
    tprintf("Left run from y=%d to %d terminated with sum %d-%d, new %d-%d\n",
            start_y, end_y, part->XAtY(margin_left, end_y),
            end->x(), part->left_margin_, part->bounding_box_.left());
}

// Helper updates margin_left and margin_right, being the bounds of the right
// margin of part of a block. Returns false and does not update the bounds if
// this partition has a disjoint margin with the established margin.
static bool UpdateRightMargin(const ColPartition& part,
                              int* margin_left, int* margin_right) {
  const TBOX& part_box = part.bounding_box();
  int top = part_box.top();
  int bottom = part_box.bottom();
  int tl_key = part.SortKey(part_box.right(), top);
  int tr_key = part.SortKey(part.right_margin(), top);
  int bl_key = part.SortKey(part_box.right(), bottom);
  int br_key = part.SortKey(part.right_margin(), bottom);
  int left_key = MAX(tl_key, bl_key);
  int right_key = MIN(tr_key, br_key);
  if (left_key <= *margin_right && right_key >= *margin_left) {
    // This part is good - let's keep it.
    *margin_right = MIN(*margin_right, right_key);
    *margin_left = MAX(*margin_left, left_key);
    return true;
  }
  return false;
}

// Computes and returns in start, end a line segment formed from a
// backwards-iterated group of right edges of partitions that satisfy the
// condition that the intersection of the right margins is non-empty, ie the
// leftmost right margin is to the right of the rightmost right bounding box
// edge.
// On return the iterator is set to the start of the next run.
void ColPartition::RightEdgeRun(ColPartition_IT* part_it,
                                ICOORD* start, ICOORD* end) {
  ColPartition* part = part_it->data();
  ColPartition* start_part = part;
  int start_y = part->bounding_box_.bottom();
  if (!part_it->at_last()) {
    int next_y = part_it->data_relative(1)->bounding_box_.top();
    if (next_y > start_y)
      start_y = next_y;
    else if (next_y < start_y)
      start_y = (start_y + next_y) / 2;
  }
  int end_y = part->bounding_box_.top();
  int margin_right = MAX_INT32;
  int margin_left = -MAX_INT32;
  UpdateRightMargin(*part, &margin_left, &margin_right);
  do {
    part_it->backward();
    part = part_it->data();
  } while (!part_it->at_last() &&
           UpdateRightMargin(*part, &margin_left, &margin_right));
  // The run ended. If we were pushed inwards, compute the next run and
  // extend it backwards to find the end of this run for a tight box.
  int next_margin_right = MAX_INT32;
  int next_margin_left = -MAX_INT32;
  UpdateRightMargin(*part, &next_margin_left, &next_margin_right);
  if (next_margin_right < margin_left) {
    ColPartition_IT next_it(*part_it);
    do {
      next_it.backward();
      part = next_it.data();
    } while (!next_it.at_last() &&
             UpdateRightMargin(*part, &next_margin_left,
                               &next_margin_right));
    // Now extend the next run forwards into the original run to get the
    // tightest fit.
    do {
      part_it->forward();
      part = part_it->data();
    } while (part != start_part &&
             UpdateRightMargin(*part, &next_margin_left,
                               &next_margin_right));
    part_it->backward();
  }
  // Now calculate the end_y.
  part = part_it->data_relative(1);
  end_y = part->bounding_box().top();
  if (!part_it->at_last() &&
      part_it->data()->bounding_box_.bottom() > end_y)
    end_y = (end_y + part_it->data()->bounding_box_.bottom()) / 2;
  start->set_y(start_y);
  start->set_x(part->XAtY(margin_left, start_y));
  end->set_y(end_y);
  end->set_x(part->XAtY(margin_left, end_y));
  if (textord_debug_tabfind && !part_it->at_last())
    tprintf("Right run from y=%d to %d terminated with sum %d-%d, new %d-%d\n",
            start_y, end_y, end->x(), part->XAtY(margin_right, end_y),
            part->bounding_box_.right(), part->right_margin_);
}

}  // namespace tesseract.
