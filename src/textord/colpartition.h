///////////////////////////////////////////////////////////////////////
// File:        colpartition.h
// Description: Class to hold partitions of the page that correspond
//              roughly to text lines.
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

#ifndef TESSERACT_TEXTORD_COLPARTITION_H_
#define TESSERACT_TEXTORD_COLPARTITION_H_

#include "bbgrid.h"
#include "blobbox.h"       // For BlobRegionType.
#include "ocrblock.h"
#include "rect.h"           // For TBOX.
#include "scrollview.h"
#include "tabfind.h"        // For WidthCallback.
#include "tabvector.h"      // For BLOBNBOX_CLIST.

#include <algorithm>

namespace tesseract {

// Number of colors in the color1, color2 arrays.
const int kRGBRMSColors = 4;

class ColPartition;
class ColPartitionSet;
class ColPartitionGrid;
class WorkingPartSet;
class WorkingPartSet_LIST;

// An enum to indicate how a partition sits on the columns.
// The order of flowing/heading/pullout must be kept consistent with
// PolyBlockType.
enum ColumnSpanningType {
  CST_NOISE,        // Strictly between columns.
  CST_FLOWING,      // Strictly within a single column.
  CST_HEADING,      // Spans multiple columns.
  CST_PULLOUT,      // Touches multiple columns, but doesn't span them.
  CST_COUNT         // Number of entries.
};

ELIST2IZEH(ColPartition)
CLISTIZEH(ColPartition)

/**
 * ColPartition is a partition of a horizontal slice of the page.
 * It starts out as a collection of blobs at a particular y-coord in the grid,
 * but ends up (after merging and uniquing) as an approximate text line.
 * ColPartitions are also used to hold a partitioning of the page into
 * columns, each representing one column. Although a ColPartition applies
 * to a given y-coordinate range, eventually, a ColPartitionSet of ColPartitions
 * emerges, which represents the columns over a wide y-coordinate range.
 */
class ColPartition : public ELIST2_LINK {
 public:
  // This empty constructor is here only so that the class can be ELISTIZED.
  // TODO(rays) change deep_copy in elst.h line 955 to take a callback copier
  // and eliminate CLASSNAME##_copier.
  ColPartition() = default;

  /**
   * @param blob_type is the blob_region_type_ of the blobs in this partition.
   * @param vertical is the direction of logical vertical on the possibly skewed image.
   */
  ColPartition(BlobRegionType blob_type, const ICOORD& vertical);
  /**
   * Constructs a fake ColPartition with no BLOBNBOXes to represent a
   * horizontal or vertical line, given a type and a bounding box.
   */
  static ColPartition* MakeLinePartition(BlobRegionType blob_type,
                                         const ICOORD& vertical,
                                         int left, int bottom,
                                         int right, int top);

  // Constructs and returns a fake ColPartition with a single fake BLOBNBOX,
  // all made from a single TBOX.
  // WARNING: Despite being on C_LISTs, the BLOBNBOX owns the C_BLOB and
  // the ColPartition owns the BLOBNBOX!!!
  // Call DeleteBoxes before deleting the ColPartition.
  static ColPartition* FakePartition(const TBOX& box,
                                     PolyBlockType block_type,
                                     BlobRegionType blob_type,
                                     BlobTextFlowType flow);

  // Constructs and returns a ColPartition with the given real BLOBNBOX,
  // and sets it up to be a "big" partition (single-blob partition bigger
  // than the surrounding text that may be a dropcap, two or more vertically
  // touching characters, or some graphic element.
  // If the given list is not nullptr, the partition is also added to the list.
  static ColPartition* MakeBigPartition(BLOBNBOX* box,
                                        ColPartition_LIST* big_part_list);

  ~ColPartition();

  // Simple accessors.
  const TBOX& bounding_box() const {
    return bounding_box_;
  }
  int left_margin() const {
    return left_margin_;
  }
  void set_left_margin(int margin) {
    left_margin_ = margin;
  }
  int right_margin() const {
    return right_margin_;
  }
  void set_right_margin(int margin) {
    right_margin_ = margin;
  }
  int median_top() const {
    return median_top_;
  }
  int median_bottom() const {
    return median_bottom_;
  }
  int median_left() const {
    return median_left_;
  }
  int median_right() const {
    return median_right_;
  }
  int median_height() const {
    return median_height_;
  }
  void set_median_height(int height) {
    median_height_ = height;
  }
  int median_width() const {
    return median_width_;
  }
  void set_median_width(int width) {
    median_width_ = width;
  }
  BlobRegionType blob_type() const {
    return blob_type_;
  }
  void set_blob_type(BlobRegionType t) {
    blob_type_ = t;
  }
  BlobTextFlowType flow() const {
    return flow_;
  }
  void set_flow(BlobTextFlowType f) {
    flow_ = f;
  }
  int good_blob_score() const {
    return good_blob_score_;
  }
  bool good_width() const {
    return good_width_;
  }
  bool good_column() const {
    return good_column_;
  }
  bool left_key_tab() const {
    return left_key_tab_;
  }
  int left_key() const {
    return left_key_;
  }
  bool right_key_tab() const {
    return right_key_tab_;
  }
  int right_key() const {
    return right_key_;
  }
  PolyBlockType type() const {
    return type_;
  }
  void set_type(PolyBlockType t) {
    type_ = t;
  }
  BLOBNBOX_CLIST* boxes() {
    return &boxes_;
  }
  int boxes_count() const {
    return boxes_.length();
  }
  void set_vertical(const ICOORD& v) {
    vertical_ = v;
  }
  ColPartition_CLIST* upper_partners() {
    return &upper_partners_;
  }
  ColPartition_CLIST* lower_partners() {
    return &lower_partners_;
  }
  void set_working_set(WorkingPartSet* working_set) {
    working_set_ = working_set;
  }
  bool block_owned() const {
    return block_owned_;
  }
  void set_block_owned(bool owned) {
    block_owned_ = owned;
  }
  bool desperately_merged() const {
    return desperately_merged_;
  }
  ColPartitionSet* column_set() const {
    return column_set_;
  }
  void set_side_step(int step) {
    side_step_ = step;
  }
  int bottom_spacing() const {
    return bottom_spacing_;
  }
  void set_bottom_spacing(int spacing) {
    bottom_spacing_ = spacing;
  }
  int top_spacing() const {
    return top_spacing_;
  }
  void set_top_spacing(int spacing) {
    top_spacing_ = spacing;
  }

  void set_table_type() {
    if (type_ != PT_TABLE) {
      type_before_table_ = type_;
      type_ = PT_TABLE;
    }
  }
  void clear_table_type() {
    if (type_ == PT_TABLE)
      type_ = type_before_table_;
  }
  bool inside_table_column() {
    return inside_table_column_;
  }
  void set_inside_table_column(bool val) {
    inside_table_column_ = val;
  }
  ColPartition* nearest_neighbor_above() const {
    return nearest_neighbor_above_;
  }
  void set_nearest_neighbor_above(ColPartition* part) {
    nearest_neighbor_above_ = part;
  }
  ColPartition* nearest_neighbor_below() const {
    return nearest_neighbor_below_;
  }
  void set_nearest_neighbor_below(ColPartition* part) {
    nearest_neighbor_below_ = part;
  }
  int space_above() const {
    return space_above_;
  }
  void set_space_above(int space) {
    space_above_ = space;
  }
  int space_below() const {
    return space_below_;
  }
  void set_space_below(int space) {
    space_below_ = space;
  }
  int space_to_left() const {
    return space_to_left_;
  }
  void set_space_to_left(int space) {
    space_to_left_ = space;
  }
  int space_to_right() const {
    return space_to_right_;
  }
  void set_space_to_right(int space) {
    space_to_right_ = space;
  }
  uint8_t* color1() {
    return color1_;
  }
  uint8_t* color2() {
    return color2_;
  }
  bool owns_blobs() const {
    return owns_blobs_;
  }
  void set_owns_blobs(bool owns_blobs) {
    // Do NOT change ownership flag when there are blobs in the list.
    // Immediately set the ownership flag when creating copies.
    ASSERT_HOST(boxes_.empty());
    owns_blobs_ = owns_blobs;
  }

  // Inline quasi-accessors that require some computation.

  // Returns the middle y-coord of the bounding box.
  int MidY() const {
    return (bounding_box_.top() + bounding_box_.bottom()) / 2;
  }
  // Returns the middle y-coord of the median top and bottom.
  int MedianY() const {
    return (median_top_ + median_bottom_) / 2;
  }
  // Returns the middle x-coord of the bounding box.
  int MidX() const {
    return (bounding_box_.left() + bounding_box_.right()) / 2;
  }
  // Returns the sort key at any given x,y.
  int SortKey(int x, int y) const {
    return TabVector::SortKey(vertical_, x, y);
  }
  // Returns the x corresponding to the sortkey, y pair.
  int XAtY(int sort_key, int y) const {
    return TabVector::XAtY(vertical_, sort_key, y);
  }
  // Returns the x difference between the two sort keys.
  int KeyWidth(int left_key, int right_key) const {
    return (right_key - left_key) / vertical_.y();
  }
  // Returns the column width between the left and right keys.
  int ColumnWidth() const {
    return KeyWidth(left_key_, right_key_);
  }
  // Returns the sort key of the box left edge.
  int BoxLeftKey() const {
    return SortKey(bounding_box_.left(), MidY());
  }
  // Returns the sort key of the box right edge.
  int BoxRightKey() const {
    return SortKey(bounding_box_.right(), MidY());
  }
  // Returns the left edge at the given y, using the sort key.
  int LeftAtY(int y) const {
    return XAtY(left_key_, y);
  }
  // Returns the right edge at the given y, using the sort key.
  int RightAtY(int y) const {
    return XAtY(right_key_, y);
  }
  // Returns true if the right edge of this is to the left of the right
  // edge of other.
  bool IsLeftOf(const ColPartition& other) const {
    return bounding_box_.right() < other.bounding_box_.right();
  }
  // Returns true if the partition contains the given x coordinate at the y.
  bool ColumnContains(int x, int y) const {
    return LeftAtY(y) - 1 <= x && x <= RightAtY(y) + 1;
  }
  // Returns true if there are no blobs in the list.
  bool IsEmpty() const {
    return boxes_.empty();
  }
  // Returns true if there is a single blob in the list.
  bool IsSingleton() const {
    return boxes_.singleton();
  }
  // Returns true if this and other overlap horizontally by bounding box.
  bool HOverlaps(const ColPartition& other) const {
    return bounding_box_.x_overlap(other.bounding_box_);
  }
  // Returns true if this and other's bounding boxes overlap vertically.
  // TODO(rays) Make HOverlaps and VOverlaps truly symmetric.
  bool VOverlaps(const ColPartition& other) const {
    return bounding_box_.y_gap(other.bounding_box_) < 0;
  }
  // Returns the vertical overlap (by median) of this and other.
  // WARNING! Only makes sense on horizontal partitions!
  int VCoreOverlap(const ColPartition& other) const {
    if (median_bottom_ == INT32_MAX || other.median_bottom_ == INT32_MAX) {
      return 0;
    }
    return std::min(median_top_, other.median_top_) -
            std::max(median_bottom_, other.median_bottom_);
  }
  // Returns the horizontal overlap (by median) of this and other.
  // WARNING! Only makes sense on vertical partitions!
  int HCoreOverlap(const ColPartition& other) const {
    return std::min(median_right_, other.median_right_) -
            std::max(median_left_, other.median_left_);
  }
  // Returns true if this and other overlap significantly vertically.
  // WARNING! Only makes sense on horizontal partitions!
  bool VSignificantCoreOverlap(const ColPartition& other) const {
    if (median_bottom_ == INT32_MAX || other.median_bottom_ == INT32_MAX) {
      return false;
    }
    int overlap = VCoreOverlap(other);
    int height = std::min(median_top_ - median_bottom_,
                     other.median_top_ - other.median_bottom_);
    return overlap * 3 > height;
  }
  // Returns true if this and other can be combined without putting a
  // horizontal step in either left or right edge of the resulting block.
  bool WithinSameMargins(const ColPartition& other) const {
    return left_margin_ <= other.bounding_box_.left() &&
           bounding_box_.left() >= other.left_margin_ &&
           bounding_box_.right() <= other.right_margin_ &&
           right_margin_ >= other.bounding_box_.right();
  }
  // Returns true if the region types (aligned_text_) match.
  // Lines never match anything, as they should never be merged or chained.
  bool TypesMatch(const ColPartition& other) const {
    return TypesMatch(blob_type_, other.blob_type_);
  }
  static bool TypesMatch(BlobRegionType type1, BlobRegionType type2) {
    return (type1 == type2 || type1 == BRT_UNKNOWN || type2 == BRT_UNKNOWN) &&
           !BLOBNBOX::IsLineType(type1) && !BLOBNBOX::IsLineType(type2);
  }

  // Returns true if the types are similar to each other.
  static bool TypesSimilar(PolyBlockType type1, PolyBlockType type2) {
    return (type1 == type2 ||
            (type1 == PT_FLOWING_TEXT && type2 == PT_INLINE_EQUATION) ||
            (type2 == PT_FLOWING_TEXT && type1 == PT_INLINE_EQUATION));
  }

  // Returns true if partitions is of horizontal line type
  bool IsLineType() const {
    return PTIsLineType(type_);
  }
  // Returns true if partitions is of image type
  bool IsImageType() const {
    return PTIsImageType(type_);
  }
  // Returns true if partitions is of text type
  bool IsTextType() const {
    return PTIsTextType(type_);
  }
  // Returns true if partitions is of pullout(inter-column) type
  bool IsPulloutType() const {
    return PTIsPulloutType(type_);
  }
  // Returns true if the partition is of an exclusively vertical type.
  bool IsVerticalType() const {
    return blob_type_ == BRT_VERT_TEXT || blob_type_ == BRT_VLINE;
  }
  // Returns true if the partition is of a definite horizontal type.
  bool IsHorizontalType() const {
    return blob_type_ == BRT_TEXT || blob_type_ == BRT_HLINE;
  }
  // Returns true is the partition is of a type that cannot be merged.
  bool IsUnMergeableType() const {
    return BLOBNBOX::UnMergeableType(blob_type_) || type_ == PT_NOISE;
  }
  // Returns true if this partition is a vertical line
  // TODO(nbeato): Use PartitionType enum when Ray's code is submitted.
  bool IsVerticalLine() const {
    return IsVerticalType() && IsLineType();
  }
  // Returns true if this partition is a horizontal line
  // TODO(nbeato): Use PartitionType enum when Ray's code is submitted.
  bool IsHorizontalLine() const {
    return IsHorizontalType() && IsLineType();
  }

  // Adds the given box to the partition, updating the partition bounds.
  // The list of boxes in the partition is updated, ensuring that no box is
  // recorded twice, and the boxes are kept in increasing left position.
  void AddBox(BLOBNBOX* box);

  // Removes the given box from the partition, updating the bounds.
  void RemoveBox(BLOBNBOX* box);

  // Returns the tallest box in the partition, as measured perpendicular to the
  // presumed flow of text.
  BLOBNBOX* BiggestBox();

  // Returns the bounding box excluding the given box.
  TBOX BoundsWithoutBox(BLOBNBOX* box);

  // Claims the boxes in the boxes_list by marking them with a this owner
  // pointer.
  void ClaimBoxes();

  // nullptr the owner of the blobs in this partition, so they can be deleted
  // independently of the ColPartition.
  void DisownBoxes();
  // nullptr the owner of the blobs in this partition that are owned by this
  // partition, so they can be deleted independently of the ColPartition.
  // Any blobs that are not owned by this partition get to keep their owner
  // without an assert failure.
  void DisownBoxesNoAssert();
  // Nulls the owner of the blobs in this partition that are owned by this
  // partition and not leader blobs, removing them from the boxes_ list, thus
  // turning this partition back to a leader partition if it contains a leader,
  // or otherwise leaving it empty. Returns true if any boxes remain.
  bool ReleaseNonLeaderBoxes();

  // Delete the boxes that this partition owns.
  void DeleteBoxes();

  // Reflects the partition in the y-axis, assuming that its blobs have
  // already been done. Corrects only a limited part of the members, since
  // this function is assumed to be used shortly after initial creation, which
  // is before a lot of the members are used.
  void ReflectInYAxis();

  // Returns true if this is a legal partition - meaning that the conditions
  // left_margin <= bounding_box left
  // left_key <= bounding box left key
  // bounding box left <= bounding box right
  // and likewise for right margin and key
  // are all met.
  bool IsLegal();

  // Returns true if the left and right edges are approximately equal.
  bool MatchingColumns(const ColPartition& other) const;

  // Returns true if the colors match for two text partitions.
  bool MatchingTextColor(const ColPartition& other) const;

  // Returns true if the sizes match for two text partitions,
  // taking orientation into account
  bool MatchingSizes(const ColPartition& other) const;

  // Returns true if there is no tabstop violation in merging this and other.
  bool ConfirmNoTabViolation(const ColPartition& other) const;

  // Returns true if other has a similar stroke width to this.
  bool MatchingStrokeWidth(const ColPartition& other,
                           double fractional_tolerance,
                           double constant_tolerance) const;
  // Returns true if candidate is an acceptable diacritic base char merge
  // with this as the diacritic.
  bool OKDiacriticMerge(const ColPartition& candidate, bool debug) const;

  // Sets the sort key using either the tab vector, or the bounding box if
  // the tab vector is nullptr. If the tab_vector lies inside the bounding_box,
  // use the edge of the box as a key any way.
  void SetLeftTab(const TabVector* tab_vector);
  void SetRightTab(const TabVector* tab_vector);

  // Copies the left/right tab from the src partition, but if take_box is
  // true, copies the box instead and uses that as a key.
  void CopyLeftTab(const ColPartition& src, bool take_box);
  void CopyRightTab(const ColPartition& src, bool take_box);

  // Returns the left rule line x coord of the leftmost blob.
  int LeftBlobRule() const;
  // Returns the right rule line x coord of the rightmost blob.
  int RightBlobRule() const;

  // Returns the density value for a particular BlobSpecialTextType.
  float SpecialBlobsDensity(const BlobSpecialTextType type) const;
  // Returns the number of blobs for a  particular BlobSpecialTextType.
  int SpecialBlobsCount(const BlobSpecialTextType type);
  // Set the density value for a particular BlobSpecialTextType, should ONLY be
  // used for debugging or testing. In production code, use
  // ComputeSpecialBlobsDensity instead.
  void SetSpecialBlobsDensity(
      const BlobSpecialTextType type, const float density);
  // Compute the SpecialTextType density of blobs, where we assume
  // that the SpecialTextType in the boxes_ has been set.
  void ComputeSpecialBlobsDensity();

  // Add a partner above if upper, otherwise below.
  // Add them uniquely and keep the list sorted by box left.
  // Partnerships are added symmetrically to partner and this.
  void AddPartner(bool upper, ColPartition* partner);
  // Removes the partner from this, but does not remove this from partner.
  // This asymmetric removal is so as not to mess up the iterator that is
  // working on partner's partner list.
  void RemovePartner(bool upper, ColPartition* partner);
  // Returns the partner if the given partner is a singleton, otherwise nullptr.
  ColPartition* SingletonPartner(bool upper);

  // Merge with the other partition and delete it.
  void Absorb(ColPartition* other, WidthCallback cb);

  // Returns true if the overlap between this and the merged pair of
  // merge candidates is sufficiently trivial to be allowed.
  // The merged box can graze the edge of this by the ok_box_overlap
  // if that exceeds the margin to the median top and bottom.
  bool OKMergeOverlap(const ColPartition& merge1, const ColPartition& merge2,
                      int ok_box_overlap, bool debug);

  // Find the blob at which to split this to minimize the overlap with the
  // given box. Returns the first blob to go in the second partition.
  BLOBNBOX* OverlapSplitBlob(const TBOX& box);

  // Split this partition keeping the first half in this and returning
  // the second half.
  // Splits by putting the split_blob and the blobs that follow
  // in the second half, and the rest in the first half.
  ColPartition* SplitAtBlob(BLOBNBOX* split_blob);

  // Splits this partition at the given x coordinate, returning the right
  // half and keeping the left half in this.
  ColPartition* SplitAt(int split_x);

  // Recalculates all the coordinate limits of the partition.
  void ComputeLimits();

  // Returns the number of boxes that overlap the given box.
  int CountOverlappingBoxes(const TBOX& box);

  // Computes and sets the type_, first_column_, last_column_ and column_set_.
  // resolution refers to the ppi resolution of the image.
  void SetPartitionType(int resolution, ColPartitionSet* columns);

  // Returns the PartitionType from the current BlobRegionType and a column
  // flow spanning type ColumnSpanningType, generated by
  // ColPartitionSet::SpanningType, that indicates how the partition sits
  // in the columns.
  PolyBlockType PartitionType(ColumnSpanningType flow) const;

  // Returns the first and last column touched by this partition.
  // resolution refers to the ppi resolution of the image.
  void ColumnRange(int resolution, ColPartitionSet* columns,
                   int* first_col, int* last_col);

  // Sets the internal flags good_width_ and good_column_.
  void SetColumnGoodness(WidthCallback cb);

  // Determines whether the blobs in this partition mostly represent
  // a leader (fixed pitch sequence) and sets the member blobs accordingly.
  // Note that height is assumed to have been tested elsewhere, and that this
  // function will find most fixed-pitch text as leader without a height filter.
  // Leader detection is limited to sequences of identical width objects,
  // such as .... or ----, so patterns, such as .-.-.-.-. will not be found.
  bool MarkAsLeaderIfMonospaced();
  // Given the result of TextlineProjection::EvaluateColPartition, (positive for
  // horizontal text, negative for vertical text, and near zero for non-text),
  // sets the blob_type_ and flow_ for this partition to indicate whether it
  // is strongly or weakly vertical or horizontal text, or non-text.
  void SetRegionAndFlowTypesFromProjectionValue(int value);

  // Sets all blobs with the partition blob type and flow, but never overwrite
  // leader blobs, as we need to be able to identify them later.
  void SetBlobTypes();

  // Returns true if a decent baseline can be fitted through the blobs.
  // Works for both horizontal and vertical text.
  bool HasGoodBaseline();

  // Adds this ColPartition to a matching WorkingPartSet if one can be found,
  // otherwise starts a new one in the appropriate column, ending the previous.
  void AddToWorkingSet(const ICOORD& bleft, const ICOORD& tright,
                       int resolution, ColPartition_LIST* used_parts,
                       WorkingPartSet_LIST* working_set);

  // From the given block_parts list, builds one or more BLOCKs and
  // corresponding TO_BLOCKs, such that the line spacing is uniform in each.
  // Created blocks are appended to the end of completed_blocks and to_blocks.
  // The used partitions are put onto used_parts, as they may still be referred
  // to in the partition grid. bleft, tright and resolution are the bounds
  // and resolution of the original image.
  static void LineSpacingBlocks(const ICOORD& bleft, const ICOORD& tright,
                                int resolution,
                                ColPartition_LIST* block_parts,
                                ColPartition_LIST* used_parts,
                                BLOCK_LIST* completed_blocks,
                                TO_BLOCK_LIST* to_blocks);
  // Constructs a block from the given list of partitions.
  // Arguments are as LineSpacingBlocks above.
  static TO_BLOCK* MakeBlock(const ICOORD& bleft, const ICOORD& tright,
                             ColPartition_LIST* block_parts,
                             ColPartition_LIST* used_parts);

  // Constructs a block from the given list of vertical text partitions.
  // Currently only creates rectangular blocks.
  static TO_BLOCK* MakeVerticalTextBlock(const ICOORD& bleft,
                                         const ICOORD& tright,
                                         ColPartition_LIST* block_parts,
                                         ColPartition_LIST* used_parts);

  // Makes a TO_ROW matching this and moves all the blobs to it, transferring
  // ownership to to returned TO_ROW.
  TO_ROW* MakeToRow();


  // Returns a copy of everything except the list of boxes. The resulting
  // ColPartition is only suitable for keeping in a column candidate list.
  ColPartition* ShallowCopy() const;
  // Returns a copy of everything with a shallow copy of the blobs.
  // The blobs are still owned by their original parent, so they are
  // treated as read-only.
  ColPartition* CopyButDontOwnBlobs();

  #ifndef GRAPHICS_DISABLED
  // Provides a color for BBGrid to draw the rectangle.
  ScrollView::Color  BoxColor() const;
  #endif // !GRAPHICS_DISABLED

  // Prints debug information on this.
  void Print() const;
  // Prints debug information on the colors.
  void PrintColors();

  // Sets the types of all partitions in the run to be the max of the types.
  void SmoothPartnerRun(int working_set_count);

  // Cleans up the partners of the given type so that there is at most
  // one partner. This makes block creation simpler.
  // If get_desperate is true, goes to more desperate merge methods
  // to merge flowing text before breaking partnerships.
  void RefinePartners(PolyBlockType type, bool get_desperate,
                      ColPartitionGrid* grid);

  // Returns true if this column partition is in the same column as
  // part. This function will only work after the SetPartitionType function
  // has been called on both column partitions. This is useful for
  // doing a SideSearch when you want things in the same page column.
  bool IsInSameColumnAs(const ColPartition& part) const;

  // Sort function to sort by bounding box.
  static int SortByBBox(const void* p1, const void* p2) {
    const ColPartition* part1 = *static_cast<const ColPartition* const*>(p1);
    const ColPartition* part2 = *static_cast<const ColPartition* const*>(p2);
    int mid_y1 = part1->bounding_box_.y_middle();
    int mid_y2 = part2->bounding_box_.y_middle();
    if ((part2->bounding_box_.bottom() <= mid_y1 &&
         mid_y1 <= part2->bounding_box_.top()) ||
        (part1->bounding_box_.bottom() <= mid_y2 &&
         mid_y2 <= part1->bounding_box_.top())) {
      // Sort by increasing x.
      return part1->bounding_box_.x_middle() - part2->bounding_box_.x_middle();
    }
    // Sort by decreasing y.
    return mid_y2 - mid_y1;
  }

  // Sets the column bounds. Primarily used in testing.
  void set_first_column(int column) {
    first_column_ = column;
  }
  void set_last_column(int column) {
    last_column_ = column;
  }

 private:
  // Cleans up the partners above if upper is true, else below.
  // If get_desperate is true, goes to more desperate merge methods
  // to merge flowing text before breaking partnerships.
  void RefinePartnersInternal(bool upper, bool get_desperate,
                              ColPartitionGrid* grid);
  // Restricts the partners to only desirable types. For text and BRT_HLINE this
  // means the same type_ , and for image types it means any image type.
  void RefinePartnersByType(bool upper, ColPartition_CLIST* partners);
  // Remove transitive partnerships: this<->a, and a<->b and this<->b.
  // Gets rid of this<->b, leaving a clean chain.
  // Also if we have this<->a and a<->this, then gets rid of this<->a, as
  // this has multiple partners.
  void RefinePartnerShortcuts(bool upper, ColPartition_CLIST* partners);
  // If multiple text partners can be merged, then do so.
  // If desperate is true, then an increase in overlap with the merge is
  // allowed. If the overlap increases, then the desperately_merged_ flag
  // is set, indicating that the textlines probably need to be regenerated
  // by aggressive line fitting/splitting, as there are probably vertically
  // joined blobs that cross textlines.
  void RefineTextPartnersByMerge(bool upper, bool desperate,
                                 ColPartition_CLIST* partners,
                                 ColPartitionGrid* grid);
  // Keep the partner with the biggest overlap.
  void RefinePartnersByOverlap(bool upper, ColPartition_CLIST* partners);

  // Return true if bbox belongs better in this than other.
  bool ThisPartitionBetter(BLOBNBOX* bbox, const ColPartition& other);

  // Smoothes the spacings in the list into groups of equal linespacing.
  // resolution is the resolution of the original image, used as a basis
  // for thresholds in change of spacing. page_height is in pixels.
  static void SmoothSpacings(int resolution, int page_height,
                             ColPartition_LIST* parts);

  // Returns true if the parts array of pointers to partitions matches the
  // condition for a spacing blip. See SmoothSpacings for what this means
  // and how it is used.
  static bool OKSpacingBlip(int resolution, int median_spacing,
                            ColPartition** parts, int offset);

  // Returns true if both the top and bottom spacings of this match the given
  // spacing to within suitable margins dictated by the image resolution.
  bool SpacingEqual(int spacing, int resolution) const;

  // Returns true if both the top and bottom spacings of this and other
  // match to within suitable margins dictated by the image resolution.
  bool SpacingsEqual(const ColPartition& other, int resolution) const;

  // Returns true if the sum spacing of this and other match the given
  // spacing (or twice the given spacing) to within a suitable margin dictated
  // by the image resolution.
  bool SummedSpacingOK(const ColPartition& other,
                       int spacing, int resolution) const;

  // Returns a suitable spacing margin that can be applied to bottoms of
  // text lines, based on the resolution and the stored side_step_.
  int BottomSpacingMargin(int resolution) const;

  // Returns a suitable spacing margin that can be applied to tops of
  // text lines, based on the resolution and the stored side_step_.
  int TopSpacingMargin(int resolution) const;

  // Returns true if the median text sizes of this and other agree to within
  // a reasonable multiplicative factor.
  bool SizesSimilar(const ColPartition& other) const;

  // Computes and returns in start, end a line segment formed from a
  // forwards-iterated group of left edges of partitions that satisfy the
  // condition that the rightmost left margin is to the left of the
  // leftmost left bounding box edge.
  // TODO(rays) Not good enough. Needs improving to tightly wrap text in both
  // directions, and to loosely wrap images.
  static void LeftEdgeRun(ColPartition_IT* part_it,
                          ICOORD* start, ICOORD* end);
  // Computes and returns in start, end a line segment formed from a
  // backwards-iterated group of right edges of partitions that satisfy the
  // condition that the leftmost right margin is to the right of the
  // rightmost right bounding box edge.
  // TODO(rays) Not good enough. Needs improving to tightly wrap text in both
  // directions, and to loosely wrap images.
  static void RightEdgeRun(ColPartition_IT* part_it,
                           ICOORD* start, ICOORD* end);

  // The margins are determined by the position of the nearest vertically
  // overlapping neighbour to the side. They indicate the maximum extent
  // that the block/column may be extended without touching something else.
  // Leftmost coordinate that the region may occupy over the y limits.
  int left_margin_ = 0;
  // Rightmost coordinate that the region may occupy over the y limits.
  int right_margin_ = 0;
  // Bounding box of all blobs in the partition.
  TBOX bounding_box_;
  // Median top and bottom of blobs in this partition.
  int median_bottom_ = 0;
  int median_top_ = 0;
  // Median height of blobs in this partition.
  int median_height_ = 0;
  // Median left and right of blobs in this partition.
  int median_left_ = 0;
  int median_right_ = 0;
  // Median width of blobs in this partition.
  int median_width_ = 0;
  // blob_region_type_ for the blobs in this partition.
  BlobRegionType blob_type_ = BRT_UNKNOWN;
  BlobTextFlowType flow_ = BTFT_NONE; // Quality of text flow.
  // Total of GoodTextBlob results for all blobs in the partition.
  int good_blob_score_ = 0;
  // True if this partition has a common width.
  bool good_width_ = false;
  // True if this is a good column candidate.
  bool good_column_ = false;
  // True if the left_key_ is from a tab vector.
  bool left_key_tab_ = false;
  // True if the right_key_ is from a tab vector.
  bool right_key_tab_ = false;
  // Left and right sort keys for the edges of the partition.
  // If the respective *_key_tab_ is true then this key came from a tab vector.
  // If not, then the class promises to keep the key equal to the sort key
  // for the respective edge of the bounding box at the MidY, so that
  // LeftAtY and RightAtY always returns an x coordinate on the line parallel
  // to vertical_ through the bounding box edge at MidY.
  int left_key_ = 0;
  int right_key_ = 0;
  // Type of this partition after looking at its relation to the columns.
  PolyBlockType type_ = PT_UNKNOWN;
  // The global vertical skew direction.
  ICOORD vertical_;
  // All boxes in the partition stored in increasing left edge coordinate.
  BLOBNBOX_CLIST boxes_;
  // The partitions above that matched this.
  ColPartition_CLIST upper_partners_;
  // The partitions below that matched this.
  ColPartition_CLIST lower_partners_;
  // The WorkingPartSet it lives in while blocks are being made.
  WorkingPartSet* working_set_ = nullptr;
  // Column_set_ is the column layout applicable to this ColPartition.
  ColPartitionSet* column_set_ = nullptr;
  // Flag is true when AddBox is sorting vertically, false otherwise.
  bool last_add_was_vertical_ = false;
  // True when the partition's ownership has been taken from the grid and
  // placed in a working set, or, after that, in the good_parts_ list.
  bool block_owned_ = false;
  // Flag to indicate that this partition was subjected to a desperate merge,
  // and therefore the textlines need rebuilding.
  bool desperately_merged_ = false;
  bool owns_blobs_ = true; // Does the partition own its blobs?
  // The first and last column that this partition applies to.
  // Flowing partitions (see type_) will have an equal first and last value
  // of the form 2n + 1, where n is the zero-based index into the partitions
  // in column_set_. (See ColPartitionSet::GetColumnByIndex).
  // Heading partitions will have unequal values of the same form.
  // Pullout partitions will have equal values, but may have even values,
  // indicating placement between columns.
  int first_column_ = -1;
  int last_column_ = -1;
  // Linespacing data.
  int side_step_ = 0;      // Median y-shift to next blob on same line.
  int top_spacing_ = 0;    // Line spacing from median_top_.
  int bottom_spacing_ = 0; // Line spacing from median_bottom_.

  // Nearest neighbor above with major x-overlap
  ColPartition* nearest_neighbor_above_ = nullptr;
  // Nearest neighbor below with major x-overlap
  ColPartition* nearest_neighbor_below_ = nullptr;
  int space_above_ = 0;    // Distance from nearest_neighbor_above
  int space_below_ = 0;    // Distance from nearest_neighbor_below
  int space_to_left_ = 0;  // Distance from the left edge of the column
  int space_to_right_ = 0; // Distance from the right edge of the column
  // Color foreground/background data.
  uint8_t color1_[kRGBRMSColors];
  uint8_t color2_[kRGBRMSColors];
  // The density of special blobs.
  float special_blobs_densities_[BSTT_COUNT];
  // Type of this partition before considering it as a table cell. This is
  // used to revert the type if a partition is first marked as a table cell but
  // later filtering steps decide it does not belong to a table
  PolyBlockType type_before_table_ = PT_UNKNOWN;
  // Check whether the current partition has been assigned to a table column.
  bool inside_table_column_ = false;
};

// Typedef it now in case it becomes a class later.
using ColPartitionGridSearch = GridSearch<ColPartition,
                   ColPartition_CLIST,
                   ColPartition_C_IT> ;

}  // namespace tesseract.

#endif  // TESSERACT_TEXTORD_COLPARTITION_H_
