/**********************************************************************
 * File:        blobbox.h  (Formerly blobnbox.h)
 * Description: Code for the textord blob class.
 * Author:					Ray Smith
 * Created:					Thu Jul 30 09:08:51 BST 1992
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

#ifndef           BLOBBOX_H
#define           BLOBBOX_H

#include          "clst.h"
#include          "elst2.h"
#include          "werd.h"
#include          "ocrblock.h"
#include          "statistc.h"

enum PITCH_TYPE
{
  PITCH_DUNNO,                   //insufficient data
  PITCH_DEF_FIXED,               //definitely fixed
  PITCH_MAYBE_FIXED,             //could be
  PITCH_DEF_PROP,
  PITCH_MAYBE_PROP,
  PITCH_CORR_FIXED,
  PITCH_CORR_PROP
};

// The possible tab-stop types of each side of a BLOBNBOX.
enum TabType {
  TT_NONE,         // Not a tab.
  TT_DELETED,      // Not a tab after detailed analysis.
  TT_UNCONFIRMED,  // Initial designation of a tab-stop candidate.
  TT_FAKE,         // Added by interpolation.
  TT_CONFIRMED,    // Aligned with neighbours.
  TT_VLINE         // Detected as a vertical line.
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
enum BlobNeighbourDir {
  BND_LEFT,
  BND_BELOW,
  BND_RIGHT,
  BND_ABOVE,
  BND_COUNT
};

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
// ordering of the enum, but NONTEXT dominates everything else, and LEADER
// dominates nothing.
// The function is anti-symmetric (t1 > t2) === !(t2 > t1), except that
// this cannot be true if t1 == t2, so the result is undefined.
inline bool DominatesInMerge(BlobTextFlowType type1, BlobTextFlowType type2) {
  // NONTEXT dominates everything.
  if (type1 == BTFT_NONTEXT) return true;
  if (type2 == BTFT_NONTEXT) return false;
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
      ConstructionInit();
    }
    explicit BLOBNBOX(C_BLOB *srcblob) {
      box = srcblob->bounding_box();
      ConstructionInit();
      cblob_ptr = srcblob;
      area = static_cast<int>(srcblob->area());
    }
    static BLOBNBOX* RealBlob(C_OUTLINE* outline) {
      C_BLOB* blob = new C_BLOB(outline);
      return new BLOBNBOX(blob);
    }

    void rotate_box(FCOORD rotation);
    void rotate(FCOORD rotation);
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
    int GoodTextBlob() const;

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
    }
    const TBOX& reduced_box() const {
      return red_box;
    }
    void set_reduced_box(TBOX new_box) {
      red_box = new_box;
      reduced = TRUE;
    }
    inT32 enclosed_area() const {
      return area;
    }
    bool joined_to_prev() const {
      return joined != 0;
    }
    bool red_box_set() const {
      return reduced != 0;
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
    void set_diacritic_box(const TBOX& diacritic_box) {
      base_char_top_ = diacritic_box.top();
      base_char_bottom_ = diacritic_box.bottom();
    }
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

    static ScrollView::Color TextlineColor(BlobRegionType region_type,
                                           BlobTextFlowType flow_type);

#ifndef GRAPHICS_DISABLED
    // Keep in sync with BlobRegionType.
    ScrollView::Color BoxColor() const;

    void plot(ScrollView* window,                // window to draw in
              ScrollView::Color blob_colour,     // for outer bits
              ScrollView::Color child_colour) {  // for holes
      if (cblob_ptr != NULL)
        cblob_ptr->plot(window, blob_colour, child_colour);
    }
#endif

  // Initializes the bulk of the members to default values for use at
  // construction time.
  void ConstructionInit() {
    cblob_ptr = NULL;
    area = 0;
    area_stroke_width_ = 0.0f;
    horz_stroke_width_ = 0.0f;
    vert_stroke_width_ = 0.0f;
    ReInit();
  }
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
    left_rule_ = 0;
    right_rule_ = 0;
    left_crossing_rule_ = 0;
    right_crossing_rule_ = 0;
    if (area_stroke_width_ == 0.0f && area > 0 && cblob() != NULL)
      area_stroke_width_ = 2.0f * area / cblob()->perimeter();
    owner_ = NULL;
    base_char_top_ = box.top();
    base_char_bottom_ = box.bottom();
    horz_possible_ = false;
    vert_possible_ = false;
    leader_on_left_ = false;
    leader_on_right_ = false;
    ClearNeighbours();
  }

  void ClearNeighbours() {
    for (int n = 0; n < BND_COUNT; ++n) {
      neighbours_[n] = NULL;
      good_stroke_neighbours_[n] = false;
    }
  }

 private:
  C_BLOB *cblob_ptr;            // edgestep blob
  TBOX box;                     // bounding box
  TBOX red_box;                 // bounding box
  int area:30;                  // enclosed area
  int joined:1;                 // joined to prev
  int reduced:1;                // reduced box set
  int repeated_set_;            // id of the set of repeated blobs
  TabType left_tab_type_;       // Indicates tab-stop assessment
  TabType right_tab_type_;      // Indicates tab-stop assessment
  BlobRegionType region_type_;  // Type of region this blob belongs to
  BlobTextFlowType flow_;       // Quality of text flow.
  inT16 left_rule_;             // x-coord of nearest but not crossing rule line
  inT16 right_rule_;            // x-coord of nearest but not crossing rule line
  inT16 left_crossing_rule_;    // x-coord of nearest or crossing rule line
  inT16 right_crossing_rule_;   // x-coord of nearest or crossing rule line
  inT16 base_char_top_;         // y-coord of top/bottom of diacritic base,
  inT16 base_char_bottom_;      // if it exists else top/bottom of this blob.
  float horz_stroke_width_;     // Median horizontal stroke width
  float vert_stroke_width_;     // Median vertical stroke width
  float area_stroke_width_;     // Stroke width from area/perimeter ratio.
  tesseract::ColPartition* owner_;  // Who will delete me when I am not needed
  BLOBNBOX* neighbours_[BND_COUNT];
  bool good_stroke_neighbours_[BND_COUNT];
  bool horz_possible_;           // Could be part of horizontal flow.
  bool vert_possible_;           // Could be part of vertical flow.
  bool leader_on_left_;          // There is a leader to the left.
  bool leader_on_right_;         // There is a leader to the right.
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
      credibility =
        (float) (blobs.length () - kErrorWeight * new_error);
      y_origin = (float) (new_c / sqrt (1 + gradient * gradient));
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
    BOOL8 merged;
    BOOL8 all_caps;              // had no ascenders
    BOOL8 used_dm_model;         // in guessing pitch
    inT16 projection_left;       // start of projection
    inT16 projection_right;      // start of projection
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
    inT32 min_space;             // min size for real space
    inT32 max_nonspace;          // max size of non-space
    inT32 space_threshold;       // space vs nonspace
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
                                        &small_blobs, &large_blobs, NULL};
      for (BLOBNBOX_LIST** list = blobnbox_list; *list != NULL; ++list) {
        BLOBNBOX_IT it(*list);
        for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
          it.data()->rotate(rotation);
        }
      }
      // Rotate the block
      ASSERT_HOST(block->poly_block() != NULL);
      block->rotate(rotation);
      // Update the median size statistic from the blobs list.
      STATS widths(0, block->bounding_box().width());
      STATS heights(0, block->bounding_box().height());
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

      for (row_it.mark_cycle_pt (); !row_it.cycled_list ();
      row_it.forward ()) {
        row = row_it.data ();
        printf ("Row range (%g,%g), para_c=%g, blobcount=" INT32FORMAT
          "\n", row->min_y (), row->max_y (), row->parallel_c (),
          row->blob_list ()->length ());
      }
    }

    // Draw the blobs on on the various lists in the block in different colors.
#ifndef GRAPHICS_DISABLED
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
    inT32 min_space;             //min definite space
    inT32 max_nonspace;          //max definite
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
