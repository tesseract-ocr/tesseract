///////////////////////////////////////////////////////////////////////
// File:        colpartition.h
// Description: Class to hold partitions of the page that correspond
//              roughly to text lines.
// Author:      Ray Smith
// Created:     Thu Aug 14 10:50:01 PDT 2008
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

#ifndef TESSERACT_TEXTORD_COLPARTITION_H__
#define TESSERACT_TEXTORD_COLPARTITION_H__

#include "bbgrid.h"
#include "blobbox.h"       // For BlobRegionType.
#include "ndminx.h"
#include "ocrblock.h"
#include "rect.h"           // For TBOX.
#include "scrollview.h"
#include "tabfind.h"        // For WidthCallback.
#include "tabvector.h"      // For BLOBNBOX_CLIST.

namespace tesseract {


class ColPartition;
class ColPartitionSet;
class WorkingPartSet;
class WorkingPartSet_LIST;

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
  ColPartition() {
    // This empty constructor is here only so that the class can be ELISTIZED.
    // TODO(rays) change deep_copy in elst.h line 955 to take a callback copier
    // and eliminate CLASSNAME##_copier.
  }
  /**
   * @param blob_type is the blob_region_type_ of the blobs in this partition.
   * @param vertical is the direction of logical vertical on the possibly skewed image.
   */
  ColPartition(BlobRegionType blob_type, const ICOORD& vertical);
  /**
   * Constructs a fake ColPartition with no BLOBNBOXes.
   * Used for making horizontal line ColPartitions and types it accordingly.
   */
  ColPartition(const ICOORD& vertical,
               int left, int bottom, int right, int top);

  // Constructs and returns a fake ColPartition with a single fake BLOBNBOX,
  // all made from a single TBOX.
  // WARNING: Despite being on C_LISTs, the BLOBNBOX owns the C_BLOB and
  // the ColPartition owns the BLOBNBOX!!!
  // Call DeleteBoxes before deleting the ColPartition.
  static ColPartition* FakePartition(const TBOX& box);

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
  int median_size() const {
    return median_size_;
  }
  BlobRegionType blob_type() const {
    return blob_type_;
  }
  void set_blob_type(BlobRegionType t) {
    blob_type_ = t;
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
  ColPartition_CLIST* upper_partners() {
    return &upper_partners_;
  }
  ColPartition_CLIST* lower_partners() {
    return &lower_partners_;
  }
  void set_working_set(WorkingPartSet* working_set) {
    working_set_ = working_set;
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

  // Inline quasi-accessors that require some computation.

  // Returns the middle y-coord of the bounding box.
  int MidY() const {
    return (bounding_box_.top() + bounding_box_.bottom()) / 2;
  }
  // Returns the middle y-coord of the median top and bottom.
  int MedianY() const {
    return (median_top_ + median_bottom_) / 2;
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
  bool IsEmpty() {
    return boxes_.empty();
  }
  // Returns true if this and other overlap horizontally by bounding box.
  bool HOverlaps(const ColPartition& other) const {
    return bounding_box_.x_overlap(other.bounding_box_);
  }
  // Returns true if this and other can be combined without putting a
  // horizontal step in either left or right edge.
  bool HCompatible(const ColPartition& other) const {
    return left_margin_ <= other.bounding_box_.left() &&
           bounding_box_.left() >= other.left_margin_ &&
           bounding_box_.right() <= other.right_margin_ &&
           right_margin_ >= other.bounding_box_.right();
  }
  // Returns the vertical overlap (by median) of this and other.
  int VOverlap(const ColPartition& other) const {
    return MIN(median_top_, other.median_top_) -
           MAX(median_bottom_, other.median_bottom_);
  }
  // Returns true if this and other overlap significantly vertically.
  bool VOverlaps(const ColPartition& other) const {
    int overlap = VOverlap(other);
    int height = MIN(median_top_ - median_bottom_,
                     other.median_top_ - other.median_bottom_);
    return overlap * 3 > height;
  }
  // Returns true if the region types (aligned_text_) match.
  bool TypesMatch(const ColPartition& other) const {
    return TypesMatch(blob_type_, other.blob_type_);
  }
  static bool TypesMatch(BlobRegionType type1, BlobRegionType type2) {
    return type1 == type2 ||
           (type1 < BRT_UNKNOWN && type2 < BRT_UNKNOWN);
  }

  // Returns true if partitions is of horizontal line type
  bool IsLineType() {
    return POLY_BLOCK::IsLineType(type_);
  }
  // Returns true if partitions is of image type
  bool IsImageType() {
    return POLY_BLOCK::IsImageType(type_);
  }
  // Returns true if partitions is of text type
  bool IsTextType() {
    return POLY_BLOCK::IsTextType(type_);
  }

  // Adds the given box to the partition, updating the partition bounds.
  // The list of boxes in the partition is updated, ensuring that no box is
  // recorded twice, and the boxes are kept in increasing left position.
  void AddBox(BLOBNBOX* box);

  // Claims the boxes in the boxes_list by marking them with a this owner
  // pointer. If a box is already owned, then run Unique on it.
  void ClaimBoxes(WidthCallback* cb);

  // Delete the boxes that this partition owns.
  void DeleteBoxes();

  // Returns true if this is a legal partition - meaning that the conditions
  // left_margin <= bounding_box left
  // left_key <= bounding box left key
  // bounding box left <= bounding box right
  // and likewise for right margin and key
  // are all met.
  bool IsLegal();

  // Returns true if the left and right edges are approximately equal.
  bool MatchingColumns(const ColPartition& other) const;

  // Sets the sort key using either the tab vector, or the bounding box if
  // the tab vector is NULL. If the tab_vector lies inside the bounding_box,
  // use the edge of the box as a key any way.
  void SetLeftTab(const TabVector* tab_vector);
  void SetRightTab(const TabVector* tab_vector);

  // Copies the left/right tab from the src partition, but if take_box is
  // true, copies the box instead and uses that as a key.
  void CopyLeftTab(const ColPartition& src, bool take_box);
  void CopyRightTab(const ColPartition& src, bool take_box);

  // Add a partner above if upper, otherwise below.
  // Add them uniquely and keep the list sorted by box left.
  // Partnerships are added symmetrically to partner and this.
  void AddPartner(bool upper, ColPartition* partner);
  // Removes the partner from this, but does not remove this from partner.
  // This asymmetric removal is so as not to mess up the iterator that is
  // working on partner's partner list.
  void RemovePartner(bool upper, ColPartition* partner);
  // Returns the partner if the given partner is a singleton, otherwise NULL.
  ColPartition* SingletonPartner(bool upper);

  // Merge with the other partition and delete it.
  void Absorb(ColPartition* other, WidthCallback* cb);

  // Shares out any common boxes amongst the partitions, ensuring that no
  // box stays in both. Returns true if anything was done.
  bool Unique(ColPartition* other, WidthCallback* cb);

  // Splits this partition at the given x coordinate, returning the right
  // half and keeping the left half in this.
  ColPartition* SplitAt(int split_x);

  // Recalculates all the coordinate limits of the partition.
  void ComputeLimits();

  // Computes and sets the type_, first_column_, last_column_ and column_set_.
  void SetPartitionType(ColPartitionSet* columns);

  // Returns the first and last column touched by this partition.
  void ColumnRange(ColPartitionSet* columns, int* first_col, int* last_col);

  // Sets the internal flags good_width_ and good_column_.
  void SetColumnGoodness(WidthCallback* cb);

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


  // Returns a copy of everything except the list of boxes. The resulting
  // ColPartition is only suitable for keeping in a column candidate list.
  ColPartition* ShallowCopy() const;

  // Provides a color for BBGrid to draw the rectangle.
  ScrollView::Color  BoxColor() const;

  // Prints debug information on this.
  void Print();

  // Sets the types of all partitions in the run to be the max of the types.
  void SmoothPartnerRun(int working_set_count);

  // Cleans up the partners of the given type so that there is at most
  // one partner. This makes block creation simpler.
  void RefinePartners(PolyBlockType type);

 private:
  // enum to refer to the entries in a neigbourhood of lines.
  // Used by SmoothSpacings to test for blips with OKSpacingBlip.
  enum SpacingNeighbourhood {
    PN_ABOVE2,
    PN_ABOVE1,
    PN_UPPER,
    PN_LOWER,
    PN_BELOW1,
    PN_BELOW2,
    PN_COUNT
  };

  // Cleans up the partners above if upper is true, else below.
  void RefinePartnersInternal(bool upper);
  // Restricts the partners to only desirable types. For text and BRT_HLINE this
  // means the same type_ , and for image types it means any image type.
  void RefinePartnersByType(bool upper, ColPartition_CLIST* partners);
  // Remove transitive partnerships: this<->a, and a<->b and this<->b.
  // Gets rid of this<->b, leaving a clean chain.
  // Also if we have this<->a and a<->this, then gets rid of this<->a, as
  // this has multiple partners.
  void RefinePartnerShortcuts(bool upper, ColPartition_CLIST* partners);
  // Keeps the partner with the longest sequence of singleton matching partners.
  // Converts all others to pullout.
  void RefineFlowingTextPartners(bool upper, ColPartition_CLIST* partners);
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
                            ColPartition** parts);

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
  int left_margin_;
  // Rightmost coordinate that the region may occupy over the y limits.
  int right_margin_;
  // Bounding box of all blobs in the partition.
  TBOX bounding_box_;
  // Median top and bottom of blobs in this partition.
  int median_bottom_;
  int median_top_;
  // Median height of blobs in this partition.
  int median_size_;
  // blob_region_type_ for the blobs in this partition.
  BlobRegionType blob_type_;
  // True if this partition has a common width.
  bool good_width_;
  // True if this is a good column candidate.
  bool good_column_;
  // True if the left_key_ is from a tab vector.
  bool left_key_tab_;
  // True if the right_key_ is from a tab vector.
  bool right_key_tab_;
  // Left and right sort keys for the edges of the partition.
  // If the respective *_key_tab_ is true then this key came from a tab vector.
  // If not, then the class promises to keep the key equal to the sort key
  // for the respective edge of the bounding box at the MidY, so that
  // LeftAtY and RightAtY always returns an x coordinate on the line parallel
  // to vertical_ through the bounding box edge at MidY.
  int left_key_;
  int right_key_;
  // Type of this partition after looking at its relation to the columns.
  PolyBlockType type_;
  // All boxes in the partition stored in increasing left edge coordinate.
  BLOBNBOX_CLIST boxes_;
  // The global vertical skew direction.
  ICOORD vertical_;
  // The partitions above that matched this.
  ColPartition_CLIST upper_partners_;
  // The partitions below that matched this.
  ColPartition_CLIST lower_partners_;
  // The WorkingPartSet it lives in while blocks are being made.
  WorkingPartSet* working_set_;
  // True when the partition's ownership has been taken from the grid and
  // placed in a working set, or, after that, in the good_parts_ list.
  bool block_owned_;
  // The first and last column that this partition applies to.
  // Flowing partitions (see type_) will have an equal first and last value
  // of the form 2n + 1, where n is the zero-based index into the partitions
  // in column_set_. (See ColPartitionSet::GetColumnByIndex).
  // Heading partitions will have unequal values of the same form.
  // Pullout partitions will have equal values, but may have even values,
  // indicating placement between columns.
  int first_column_;
  int last_column_;
  // Column_set_ is the column layout applicable to this ColPartition.
  ColPartitionSet* column_set_;
  // Linespacing data.
  int side_step_;       // Median y-shift to next blob on same line.
  int top_spacing_;     // Line spacing from median_top_.
  int bottom_spacing_;  // Line spacing from median_bottom_.

  // Type of this partition before considering it as a table cell. This is
  // used to revert the type if a partition is first marked as a table cell but
  // later filtering steps decide it does not belong to a table
  PolyBlockType type_before_table_;
  bool inside_table_column_;  // Check whether the current partition has been
                              // assigned to a table column
  // Nearest neighbor above with major x-overlap
  ColPartition* nearest_neighbor_above_;
  // Nearest neighbor below with major x-overlap
  ColPartition* nearest_neighbor_below_;
  int space_above_;      // Distance from nearest_neighbor_above
  int space_below_;      // Distance from nearest_neighbor_below
  int space_to_left_;    // Distance from the left edge of the column
  int space_to_right_;   // Distance from the right edge of the column
};

// Typedef it now in case it becomes a class later.
typedef BBGrid<ColPartition,
               ColPartition_CLIST,
               ColPartition_C_IT> ColPartitionGrid;
typedef GridSearch<ColPartition,
               ColPartition_CLIST,
               ColPartition_C_IT> ColPartitionGridSearch;

}  // namespace tesseract.

#endif  // TESSERACT_TEXTORD_COLPARTITION_H__
