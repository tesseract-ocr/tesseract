/**********************************************************************
 * File:        blobbox.h  (Formerly blobnbox.h)
 * Description: Code for the textord blob class.
 * Author:      Ray Smith
 *
 * (C) Copyright 1992, Hewlett-Packard Ltd.
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
 **********************************************************************/

#ifndef BLOBBOX_H
#define BLOBBOX_H

#include <cinttypes>           // for PRId32
#include <cmath>               // for std::sqrt
#include <cstdint>             // for int16_t, int32_t
#include "elst.h"              // for ELIST_ITERATOR, ELISTIZEH, ELIST_LINK
#include "elst2.h"             // for ELIST2_ITERATOR, ELIST2IZEH, ELIST2_LINK
#include "errcode.h"           // for ASSERT_HOST
#include "ocrblock.h"          // for BLOCK
#include "params.h"            // for DoubleParam, double_VAR_H
#include "pdblock.h"           // for PDBLK
#include "points.h"            // for FCOORD, ICOORD, ICOORDELT_LIST
#include "quspline.h"          // for QSPLINE
#include "rect.h"              // for TBOX
#include "scrollview.h"        // for ScrollView, ScrollView::Color
#include "statistc.h"          // for STATS
#include "stepblob.h"          // for C_BLOB
#include "tprintf.h"           // for tprintf
#include "werd.h"              // for WERD_LIST

class C_OUTLINE;

struct Pix;

enum PITCH_TYPE
{
  PITCH_DUNNO,                   // insufficient data
  PITCH_DEF_FIXED,               // definitely fixed
  PITCH_MAYBE_FIXED,             // could be
  PITCH_DEF_PROP,
  PITCH_MAYBE_PROP,
  PITCH_CORR_FIXED,
  PITCH_CORR_PROP
};

// The possible tab-stop types of each side of a BLOBNBOX.
// The ordering is important, as it is used for deleting dead-ends in the
// search. ALIGNED, CONFIRMED and VLINE should remain greater than the
// non-aligned, unset, or deleted members.
enum TabType {
  TT_NONE,           // Not a tab.
  TT_DELETED,        // Not a tab after detailed analysis.
  TT_MAYBE_RAGGED,   // Initial designation of a tab-stop candidate.
  TT_MAYBE_ALIGNED,  // Initial designation of a tab-stop candidate.
  TT_CONFIRMED,      // Aligned with neighbours.
  TT_VLINE           // Detected as a vertical line.
};

// The possible region types of a BLOBNBOX.
// Note: keep all the text types > BRT_UNKNOWN and all the image types less.
// Keep in sync with kBlobTypes in colpartition.cpp and BoxColor, and the
// *Type static functions below.
enum BlobRegionType {
  BRT_NOISE,      // Neither text nor image.
  BRT_HLINE,      // Horizontal separator line.
  BRT_VLINE,      // Vertical separator line.
  BRT_RECTIMAGE,  // Rectangular image.
  BRT_POLYIMAGE,  // Non-rectangular image.
  BRT_UNKNOWN,    // Not determined yet.
  BRT_VERT_TEXT,  // Vertical alignment, not necessarily vertically oriented.
  BRT_TEXT,       // Convincing text.

  BRT_COUNT       // Number of possibilities.
};

// enum for elements of arrays that refer to neighbours.
// NOTE: keep in this order, so ^2 can be used to flip direction.
enum BlobNeighbourDir {
  BND_LEFT,
  BND_BELOW,
  BND_RIGHT,
  BND_ABOVE,
  BND_COUNT
};

// enum for special type of text characters, such as math symbol or italic.
enum BlobSpecialTextType {
  BSTT_NONE,  // No special.
  BSTT_ITALIC,  // Italic style.
  BSTT_DIGIT,  // Digit symbols.
  BSTT_MATH,  // Mathmatical symobls (not including digit).
  BSTT_UNCLEAR,  // Characters with low recognition rate.
  BSTT_SKIP,  // Characters that we skip labeling (usually too small).
  BSTT_COUNT
};

inline BlobNeighbourDir DirOtherWay(BlobNeighbourDir dir) {
  return static_cast<BlobNeighbourDir>(dir ^ 2);
}

// BlobTextFlowType indicates the quality of neighbouring information
// related to a chain of connected components, either horizontally or
// vertically. Also used by ColPartition for the collection of blobs
// within, which should all have the same value in most cases.
enum BlobTextFlowType {
  BTFT_NONE,           // No text flow set yet.
  BTFT_NONTEXT,        // Flow too poor to be likely text.
  BTFT_NEIGHBOURS,     // Neighbours support flow in this direction.
  BTFT_CHAIN,          // There is a weak chain of text in this direction.
  BTFT_STRONG_CHAIN,   // There is a strong chain of text in this direction.
  BTFT_TEXT_ON_IMAGE,  // There is a strong chain of text on an image.
  BTFT_LEADER,         // Leader dots/dashes etc.
  BTFT_COUNT
};

// Returns true if type1 dominates type2 in a merge. Mostly determined by the
// ordering of the enum, LEADER is weak and dominates nothing.
// The function is anti-symmetric (t1 > t2) === !(t2 > t1), except that
// this cannot be true if t1 == t2, so the result is undefined.
inline bool DominatesInMerge(BlobTextFlowType type1, BlobTextFlowType type2) {
  // LEADER always loses.
  if (type1 == BTFT_LEADER) return false;
  if (type2 == BTFT_LEADER) return true;
  // With those out of the way, the ordering of the enum determines the result.
  return type1 >= type2;
}

namespace tesseract {
class ColPartition;
}

class BLOBNBOX;
ELISTIZEH (BLOBNBOX)
class BLOBNBOX:public ELIST_LINK
{
  public:
    BLOBNBOX() {
      ReInit();
    }
    explicit BLOBNBOX(C_BLOB *srcblob) {
      box = srcblob->bounding_box();
      ReInit();
      cblob_ptr = srcblob;
      area = static_cast<int>(srcblob->area());
    }
    ~BLOBNBOX() {
      if (owns_cblob_) delete cblob_ptr;
    }
    static BLOBNBOX* RealBlob(C_OUTLINE* outline) {
      auto* blob = new C_BLOB(outline);
      return new BLOBNBOX(blob);
    }

    // Rotates the box and the underlying blob.
    void rotate(FCOORD rotation);

    // Methods that act on the box without touching the underlying blob.
    // Reflect the box in the y-axis, leaving the underlying blob untouched.
    void reflect_box_in_y_axis();
    // Rotates the box by the angle given by rotation.
    // If the blob is a diacritic, then only small rotations for skew
    // correction can be applied.
    void rotate_box(FCOORD rotation);
    // Moves just the box by the given vector.
    void translate_box(ICOORD v) {
      if (IsDiacritic()) {
        box.move(v);
        base_char_top_ += v.y();
        base_char_bottom_ += v.y();
      } else {
        box.move(v);
        set_diacritic_box(box);
      }
    }
    void merge(BLOBNBOX *nextblob);
    void really_merge(BLOBNBOX* other);
    void chop(                        // fake chop blob
              BLOBNBOX_IT *start_it,  // location of this
              BLOBNBOX_IT *blob_it,   // iterator
              FCOORD rotation,        // for landscape
              float xheight);         // line height

    void NeighbourGaps(int gaps[BND_COUNT]) const;
    void MinMaxGapsClipped(int* h_min, int* h_max,
                           int* v_min, int* v_max) const;
    void CleanNeighbours();
    // Returns positive if there is at least one side neighbour that has a
    // similar stroke width and is not on the other side of a rule line.
    int GoodTextBlob() const;
    // Returns the number of side neighbours that are of type BRT_NOISE.
    int NoisyNeighbours() const;

    // Returns true if the blob is noise and has no owner.
    bool DeletableNoise() const {
      return owner() == nullptr && region_type() == BRT_NOISE;
    }

    // Returns true, and sets vert_possible/horz_possible if the blob has some
    // feature that makes it individually appear to flow one way.
    // eg if it has a high aspect ratio, yet has a complex shape, such as a
    // joined word in Latin, Arabic, or Hindi, rather than being a -, I, l, 1.
    bool DefiniteIndividualFlow();

    // Returns true if there is no tabstop violation in merging this and other.
    bool ConfirmNoTabViolation(const BLOBNBOX& other) const;

    // Returns true if other has a similar stroke width to this.
    bool MatchingStrokeWidth(const BLOBNBOX& other,
                             double fractional_tolerance,
                             double constant_tolerance) const;

    // Returns a bounding box of the outline contained within the
    // given horizontal range.
    TBOX BoundsWithinLimits(int left, int right);

    // Estimates and stores the baseline position based on the shape of the
    // outline.
    void EstimateBaselinePosition();

    // Simple accessors.
    const TBOX& bounding_box() const {
      return box;
    }
    // Set the bounding box. Use with caution.
    // Normally use compute_bounding_box instead.
    void set_bounding_box(const TBOX& new_box) {
      box = new_box;
      base_char_top_ = box.top();
      base_char_bottom_ = box.bottom();
    }
    void compute_bounding_box() {
      box = cblob_ptr->bounding_box();
      base_char_top_ = box.top();
      base_char_bottom_ = box.bottom();
      baseline_y_ = box.bottom();
    }
    const TBOX& reduced_box() const {
      return red_box;
    }
    void set_reduced_box(TBOX new_box) {
      red_box = new_box;
      reduced = true;
    }
    int32_t enclosed_area() const {
      return area;
    }
    bool joined_to_prev() const {
      return joined;
    }
    bool red_box_set() const {
      return reduced;
    }
    int repeated_set() const {
      return repeated_set_;
    }
    void set_repeated_set(int set_id) {
      repeated_set_ = set_id;
    }
    C_BLOB *cblob() const {
      return cblob_ptr;
    }
    TabType left_tab_type() const {
      return left_tab_type_;
    }
    void set_left_tab_type(TabType new_type) {
      left_tab_type_ = new_type;
    }
    TabType right_tab_type() const {
      return right_tab_type_;
    }
    void set_right_tab_type(TabType new_type) {
      right_tab_type_ = new_type;
    }
    BlobRegionType region_type() const {
      return region_type_;
    }
    void set_region_type(BlobRegionType new_type) {
      region_type_ = new_type;
    }
    BlobSpecialTextType special_text_type() const {
      return spt_type_;
    }
    void set_special_text_type(BlobSpecialTextType new_type) {
      spt_type_ = new_type;
    }
    BlobTextFlowType flow() const {
      return flow_;
    }
    void set_flow(BlobTextFlowType value) {
      flow_ = value;
    }
    bool vert_possible() const {
      return vert_possible_;
    }
    void set_vert_possible(bool value) {
      vert_possible_ = value;
    }
    bool horz_possible() const {
      return horz_possible_;
    }
    void set_horz_possible(bool value) {
      horz_possible_ = value;
    }
    int left_rule() const {
      return left_rule_;
    }
    void set_left_rule(int new_left) {
      left_rule_ = new_left;
    }
    int right_rule() const {
      return right_rule_;
    }
    void set_right_rule(int new_right) {
      right_rule_ = new_right;
    }
    int left_crossing_rule() const {
      return left_crossing_rule_;
    }
    void set_left_crossing_rule(int new_left) {
      left_crossing_rule_ = new_left;
    }
    int right_crossing_rule() const {
      return right_crossing_rule_;
    }
    void set_right_crossing_rule(int new_right) {
      right_crossing_rule_ = new_right;
    }
    float horz_stroke_width() const {
      return horz_stroke_width_;
    }
    void set_horz_stroke_width(float width) {
      horz_stroke_width_ = width;
    }
    float vert_stroke_width() const {
      return vert_stroke_width_;
    }
    void set_vert_stroke_width(float width) {
      vert_stroke_width_ = width;
    }
    float area_stroke_width() const {
      return area_stroke_width_;
    }
    tesseract::ColPartition* owner() const {
      return owner_;
    }
    void set_owner(tesseract::ColPartition* new_owner) {
      owner_ = new_owner;
    }
    bool leader_on_left() const {
      return leader_on_left_;
    }
    void set_leader_on_left(bool flag) {
      leader_on_left_ = flag;
    }
    bool leader_on_right() const {
      return leader_on_right_;
    }
    void set_leader_on_right(bool flag) {
      leader_on_right_ = flag;
    }
    BLOBNBOX* neighbour(BlobNeighbourDir n) const {
      return neighbours_[n];
    }
    bool good_stroke_neighbour(BlobNeighbourDir n) const {
      return good_stroke_neighbours_[n];
    }
    void set_neighbour(BlobNeighbourDir n, BLOBNBOX* neighbour, bool good) {
      neighbours_[n] = neighbour;
      good_stroke_neighbours_[n] = good;
    }
    bool IsDiacritic() const {
      return base_char_top_ != box.top() || base_char_bottom_ != box.bottom();
    }
    int base_char_top() const {
      return base_char_top_;
    }
    int base_char_bottom() const {
      return base_char_bottom_;
    }
    int baseline_position() const {
      return baseline_y_;
    }
    int line_crossings() const {
      return line_crossings_;
    }
    void set_line_crossings(int value) {
      line_crossings_ = value;
    }
    void set_diacritic_box(const TBOX& diacritic_box) {
      base_char_top_ = diacritic_box.top();
      base_char_bottom_ = diacritic_box.bottom();
    }
    BLOBNBOX* base_char_blob() const {
      return base_char_blob_;
    }
    void set_base_char_blob(BLOBNBOX* blob) {
      base_char_blob_ = blob;
    }
    void set_owns_cblob(bool value) { owns_cblob_ = value; }

    bool UniquelyVertical() const {
      return vert_possible_ && !horz_possible_;
    }
    bool UniquelyHorizontal() const {
      return horz_possible_ && !vert_possible_;
    }

    // Returns true if the region type is text.
    static bool IsTextType(BlobRegionType type) {
      return type == BRT_TEXT || type == BRT_VERT_TEXT;
    }
    // Returns true if the region type is image.
    static bool IsImageType(BlobRegionType type) {
      return type == BRT_RECTIMAGE || type == BRT_POLYIMAGE;
    }
    // Returns true if the region type is line.
    static bool IsLineType(BlobRegionType type) {
      return type == BRT_HLINE || type == BRT_VLINE;
    }
    // Returns true if the region type cannot be merged.
    static bool UnMergeableType(BlobRegionType type) {
      return IsLineType(type) || IsImageType(type);
    }
    // Helper to call CleanNeighbours on all blobs on the list.
    static void CleanNeighbours(BLOBNBOX_LIST* blobs);
    // Helper to delete all the deletable blobs on the list.
    static void DeleteNoiseBlobs(BLOBNBOX_LIST* blobs);
    // Helper to compute edge offsets for  all the blobs on the list.
    // See coutln.h for an explanation of edge offsets.
    static void ComputeEdgeOffsets(Pix* thresholds, Pix* grey,
                                   BLOBNBOX_LIST* blobs);

#ifndef GRAPHICS_DISABLED
    // Helper to draw all the blobs on the list in the given body_colour,
    // with child outlines in the child_colour.
    static void PlotBlobs(BLOBNBOX_LIST* list,
                          ScrollView::Color body_colour,
                          ScrollView::Color child_colour,
                          ScrollView* win);
    // Helper to draw only DeletableNoise blobs (unowned, BRT_NOISE) on the
    // given list in the given body_colour, with child outlines in the
    // child_colour.
    static void PlotNoiseBlobs(BLOBNBOX_LIST* list,
                               ScrollView::Color body_colour,
                               ScrollView::Color child_colour,
                               ScrollView* win);

    static ScrollView::Color TextlineColor(BlobRegionType region_type,
                                           BlobTextFlowType flow_type);

    // Keep in sync with BlobRegionType.
    ScrollView::Color BoxColor() const;

    void plot(ScrollView* window,                // window to draw in
              ScrollView::Color blob_colour,     // for outer bits
              ScrollView::Color child_colour);   // for holes
#endif

  // Initializes members set by StrokeWidth and beyond, without discarding
  // stored area and strokewidth values, which are expensive to calculate.
  void ReInit() {
    joined = false;
    reduced = false;
    repeated_set_ = 0;
    left_tab_type_ = TT_NONE;
    right_tab_type_ = TT_NONE;
    region_type_ = BRT_UNKNOWN;
    flow_ = BTFT_NONE;
    spt_type_ = BSTT_SKIP;
    left_rule_ = 0;
    right_rule_ = 0;
    left_crossing_rule_ = 0;
    right_crossing_rule_ = 0;
    if (area_stroke_width_ == 0.0f && area > 0 && cblob() != nullptr
        && cblob()->perimeter()!=0)
      area_stroke_width_ = 2.0f * area / cblob()->perimeter();
    owner_ = nullptr;
    base_char_top_ = box.top();
    base_char_bottom_ = box.bottom();
    baseline_y_ = box.bottom();
    line_crossings_ = 0;
    base_char_blob_ = nullptr;
    horz_possible_ = false;
    vert_possible_ = false;
    leader_on_left_ = false;
    leader_on_right_ = false;
    ClearNeighbours();
  }

  void ClearNeighbours() {
    for (int n = 0; n < BND_COUNT; ++n) {
      neighbours_[n] = nullptr;
      good_stroke_neighbours_[n] = false;
    }
  }

 private:
  C_BLOB* cblob_ptr = nullptr;  // edgestep blob
  TBOX box;                     // bounding box
  TBOX red_box;                 // bounding box
  int32_t area = 0;             // enclosed area
  int32_t repeated_set_ = 0;    // id of the set of repeated blobs
  TabType left_tab_type_ = TT_NONE;  // Indicates tab-stop assessment
  TabType right_tab_type_ = TT_NONE; // Indicates tab-stop assessment
  BlobRegionType region_type_ = BRT_UNKNOWN; // Type of region this blob belongs to
  BlobTextFlowType flow_ = BTFT_NONE;       // Quality of text flow.
  BlobSpecialTextType spt_type_;   // Special text type.
  bool joined = false;          // joined to prev
  bool reduced = false;         // reduced box set
  int16_t left_rule_ = 0;           // x-coord of nearest but not crossing rule line
  int16_t right_rule_ = 0;          // x-coord of nearest but not crossing rule line
  int16_t left_crossing_rule_;  // x-coord of nearest or crossing rule line
  int16_t right_crossing_rule_; // x-coord of nearest or crossing rule line
  int16_t base_char_top_;       // y-coord of top/bottom of diacritic base,
  int16_t base_char_bottom_;    // if it exists else top/bottom of this blob.
  int16_t baseline_y_;          // Estimate of baseline position.
  int32_t line_crossings_;      // Number of line intersections touched.
  BLOBNBOX* base_char_blob_;    // The blob that was the base char.
  tesseract::ColPartition* owner_;  // Who will delete me when I am not needed
  BLOBNBOX* neighbours_[BND_COUNT];
  float horz_stroke_width_ = 0.0f; // Median horizontal stroke width
  float vert_stroke_width_ = 0.0f; // Median vertical stroke width
  float area_stroke_width_ = 0.0f; // Stroke width from area/perimeter ratio.
  bool good_stroke_neighbours_[BND_COUNT];
  bool horz_possible_;           // Could be part of horizontal flow.
  bool vert_possible_;           // Could be part of vertical flow.
  bool leader_on_left_;          // There is a leader to the left.
  bool leader_on_right_;         // There is a leader to the right.
  // Iff true, then the destructor should delete the cblob_ptr.
  // TODO(rays) migrate all uses to correctly setting this flag instead of
  // deleting the C_BLOB before deleting the BLOBNBOX.
  bool owns_cblob_ = false;
};

class TO_ROW: public ELIST2_LINK
{
  public:
    static const int kErrorWeight = 3;

    TO_ROW() {
      clear();
    }                            //empty
    TO_ROW(                 //constructor
           BLOBNBOX *blob,  //from first blob
           float top,       //of row //target height
           float bottom,
           float row_size);

    void print() const;
    float max_y() const {  //access function
      return y_max;
    }
    float min_y() const {
      return y_min;
    }
    float mean_y() const {
      return (y_min + y_max) / 2.0f;
    }
    float initial_min_y() const {
      return initial_y_min;
    }
    float line_m() const {  //access to line fit
      return m;
    }
    float line_c() const {
      return c;
    }
    float line_error() const {
      return error;
    }
    float parallel_c() const {
      return para_c;
    }
    float parallel_error() const {
      return para_error;
    }
    float believability() const {  //baseline goodness
      return credibility;
    }
    float intercept() const {  //real parallel_c
      return y_origin;
    }
    void add_blob(                 //put in row
                  BLOBNBOX *blob,  //blob to add
                  float top,       //of row //target height
                  float bottom,
                  float row_size);
    void insert_blob(  //put in row in order
                     BLOBNBOX *blob);

    BLOBNBOX_LIST *blob_list() {  //get list
      return &blobs;
    }

    void set_line(              //set line spec
                  float new_m,  //line to set
                  float new_c,
                  float new_error) {
      m = new_m;
      c = new_c;
      error = new_error;
    }
    void set_parallel_line(                 //set fixed gradient line
                           float gradient,  //page gradient
                           float new_c,
                           float new_error) {
      para_c = new_c;
      para_error = new_error;
      credibility = blobs.length() -  kErrorWeight * new_error;
      y_origin = new_c / std::sqrt(1 + gradient * gradient);
      //real intercept
    }
    void set_limits(                  //set min,max
                    float new_min,    //bottom and
                    float new_max) {  //top of row
      y_min = new_min;
      y_max = new_max;
    }
    void compute_vertical_projection();
    //get projection

    bool rep_chars_marked() const {
      return num_repeated_sets_ != -1;
    }
    void clear_rep_chars_marked() {
      num_repeated_sets_ = -1;
    }
    int num_repeated_sets() const {
      return num_repeated_sets_;
    }
    void set_num_repeated_sets(int num_sets) {
      num_repeated_sets_ = num_sets;
    }

                                 // true when dead
    bool merged = false;
    bool all_caps;              // had no ascenders
    bool used_dm_model;         // in guessing pitch
    int16_t projection_left;       // start of projection
    int16_t projection_right;      // start of projection
    PITCH_TYPE pitch_decision;   // how strong is decision
    float fixed_pitch;           // pitch or 0
    float fp_space;              // sp if fixed pitch
    float fp_nonsp;              // nonsp if fixed pitch
    float pr_space;              // sp if prop
    float pr_nonsp;              // non sp if prop
    float spacing;               // to "next" row
    float xheight;               // of line
    int xheight_evidence;        // number of blobs of height xheight
    float ascrise;               // ascenders
    float descdrop;              // descenders
    float body_size;             // of CJK characters.  Assumed to be
                                 // xheight+ascrise for non-CJK text.
    int32_t min_space;             // min size for real space
    int32_t max_nonspace;          // max size of non-space
    int32_t space_threshold;       // space vs nonspace
    float kern_size;             // average non-space
    float space_size;            // average space
    WERD_LIST rep_words;         // repeated chars
    ICOORDELT_LIST char_cells;   // fixed pitch cells
    QSPLINE baseline;            // curved baseline
    STATS projection;            // vertical projection

  private:
    void clear();  // clear all values to reasonable defaults

    BLOBNBOX_LIST blobs;         //blobs in row
    float y_min;                 //coords
    float y_max;
    float initial_y_min;
    float m, c;                  //line spec
    float error;                 //line error
    float para_c;                //constrained fit
    float para_error;
    float y_origin;              //rotated para_c;
    float credibility;           //baseline believability
    int num_repeated_sets_;      // number of sets of repeated blobs
                                 // set to -1 if we have not searched
                                 // for repeated blobs in this row yet
};

ELIST2IZEH (TO_ROW)
class TO_BLOCK:public ELIST_LINK
{
  public:
    TO_BLOCK() : pitch_decision(PITCH_DUNNO) {
      clear();
    }                            //empty
    TO_BLOCK(                    //constructor
             BLOCK *src_block);  //real block
    ~TO_BLOCK();

    void clear();  // clear all scalar members.

    TO_ROW_LIST *get_rows() {  //access function
      return &row_list;
    }

    // Rotate all the blobnbox lists and the underlying block. Then update the
    // median size statistic from the blobs list.
    void rotate(const FCOORD& rotation) {
      BLOBNBOX_LIST* blobnbox_list[] = {&blobs, &underlines, &noise_blobs,
                                        &small_blobs, &large_blobs, nullptr};
      for (BLOBNBOX_LIST** list = blobnbox_list; *list != nullptr; ++list) {
        BLOBNBOX_IT it(*list);
        for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
          it.data()->rotate(rotation);
        }
      }
      // Rotate the block
      ASSERT_HOST(block->pdblk.poly_block() != nullptr);
      block->rotate(rotation);
      // Update the median size statistic from the blobs list.
      STATS widths(0, block->pdblk.bounding_box().width());
      STATS heights(0, block->pdblk.bounding_box().height());
      BLOBNBOX_IT blob_it(&blobs);
      for (blob_it.mark_cycle_pt(); !blob_it.cycled_list(); blob_it.forward()) {
        widths.add(blob_it.data()->bounding_box().width(), 1);
        heights.add(blob_it.data()->bounding_box().height(), 1);
      }
      block->set_median_size(static_cast<int>(widths.median() + 0.5),
                             static_cast<int>(heights.median() + 0.5));
    }

    void print_rows() {  //debug info
      TO_ROW_IT row_it = &row_list;
      TO_ROW *row;

      for (row_it.mark_cycle_pt(); !row_it.cycled_list();
           row_it.forward()) {
        row = row_it.data();
        tprintf("Row range (%g,%g), para_c=%g, blobcount=%" PRId32 "\n",
                row->min_y(), row->max_y(), row->parallel_c(),
                row->blob_list()->length());
      }
    }

    // Reorganizes the blob lists with a different definition of small, medium
    // and large, compared to the original definition.
    // Height is still the primary filter key, but medium width blobs of small
    // height become medium, and very wide blobs of small height stay small.
    void ReSetAndReFilterBlobs();

    // Deletes noise blobs from all lists where not owned by a ColPartition.
    void DeleteUnownedNoise();

    // Computes and stores the edge offsets on each blob for use in feature
    // extraction, using greyscale if the supplied grey and thresholds pixes
    // are 8-bit or otherwise (if nullptr or not 8 bit) the original binary
    // edge step outlines.
    // Thresholds must either be the same size as grey or an integer down-scale
    // of grey.
    // See coutln.h for an explanation of edge offsets.
    void ComputeEdgeOffsets(Pix* thresholds, Pix* grey);

#ifndef GRAPHICS_DISABLED
    // Draw the noise blobs from all lists in red.
    void plot_noise_blobs(ScrollView* to_win);
    // Draw the blobs on on the various lists in the block in different colors.
    void plot_graded_blobs(ScrollView* to_win);
#endif

    BLOBNBOX_LIST blobs;         //medium size
    BLOBNBOX_LIST underlines;    //underline blobs
    BLOBNBOX_LIST noise_blobs;   //very small
    BLOBNBOX_LIST small_blobs;   //fairly small
    BLOBNBOX_LIST large_blobs;   //big blobs
    BLOCK *block;                //real block
    PITCH_TYPE pitch_decision;   //how strong is decision
    float line_spacing;          //estimate
    // line_size is a lower-bound estimate of the font size in pixels of
    // the text in the block (with ascenders and descenders), being a small
    // (1.25) multiple of the median height of filtered blobs.
    // In most cases the font size will be bigger, but it will be closer
    // if the text is allcaps, or in a no-x-height script.
    float line_size;             //estimate
    float max_blob_size;         //line assignment limit
    float baseline_offset;       //phase shift
    float xheight;               //median blob size
    float fixed_pitch;           //pitch or 0
    float kern_size;             //average non-space
    float space_size;            //average space
    int32_t min_space;             //min definite space
    int32_t max_nonspace;          //max definite
    float fp_space;              //sp if fixed pitch
    float fp_nonsp;              //nonsp if fixed pitch
    float pr_space;              //sp if prop
    float pr_nonsp;              //non sp if prop
    TO_ROW *key_row;             //starting row

   private:
    TO_ROW_LIST row_list;        //temporary rows
};

ELISTIZEH (TO_BLOCK)
extern double_VAR_H (textord_error_weight, 3,
"Weighting for error in believability");
void find_cblob_limits(                  //get y limits
                       C_BLOB *blob,     //blob to search
                       float leftx,      //x limits
                       float rightx,
                       FCOORD rotation,  //for landscape
                       float &ymin,      //output y limits
                       float &ymax);
void find_cblob_vlimits(               //get y limits
                        C_BLOB *blob,  //blob to search
                        float leftx,   //x limits
                        float rightx,
                        float &ymin,   //output y limits
                        float &ymax);
void find_cblob_hlimits(                //get x limits
                        C_BLOB *blob,   //blob to search
                        float bottomy,  //y limits
                        float topy,
                        float &xmin,    //output x limits
                        float &xymax);
C_BLOB *crotate_cblob(                 //rotate it
                      C_BLOB *blob,    //blob to search
                      FCOORD rotation  //for landscape
                     );
TBOX box_next(                 //get bounding box
             BLOBNBOX_IT *it  //iterator to blobds
            );
TBOX box_next_pre_chopped(                 //get bounding box
                         BLOBNBOX_IT *it  //iterator to blobds
                        );
void vertical_cblob_projection(               //project outlines
                               C_BLOB *blob,  //blob to project
                               STATS *stats   //output
                              );
void vertical_coutline_projection(                     //project outlines
                                  C_OUTLINE *outline,  //outline to project
                                  STATS *stats         //output
                                 );
#ifndef GRAPHICS_DISABLED
void plot_blob_list(ScrollView* win,                   // window to draw in
                    BLOBNBOX_LIST *list,               // blob list
                    ScrollView::Color body_colour,     // colour to draw
                    ScrollView::Color child_colour);   // colour of child
#endif  // GRAPHICS_DISABLED
#endif
