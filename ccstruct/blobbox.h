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

#include          "varable.h"
#include          "clst.h"
#include          "elst2.h"
#include          "werd.h"
#include          "ocrblock.h"
#include          "statistc.h"

extern double_VAR_H (textord_error_weight, 3,
"Weighting for error in believability");

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
// Keep in sync with kBlobTypes in colpartition.cpp and BoxColor below.
enum BlobRegionType {
  BRT_NOISE,      // Neither text nor image.
  BRT_HLINE,      // Horizontal separator line.
  BRT_RECTIMAGE,  // Rectangular image.
  BRT_POLYIMAGE,  // Non-rectangular image.
  BRT_UNKNOWN,    // Not determined yet.
  BRT_VERT_TEXT,  // Vertical alignment, not necessarily vertically oriented.
  BRT_TEXT,       // Convincing text.

  BRT_COUNT       // Number of possibilities.
};

namespace tesseract {
class ColPartition;
}

class BLOBNBOX;
ELISTIZEH (BLOBNBOX)
class BLOBNBOX:public ELIST_LINK
{
  public:
    BLOBNBOX() {
      blob_ptr = NULL;
      cblob_ptr = NULL;
      area = 0;
      Init();
    }
    explicit BLOBNBOX(PBLOB *srcblob) {
      blob_ptr = srcblob;
      cblob_ptr = NULL;
      box = srcblob->bounding_box ();
      area = (int) srcblob->area ();
      Init();
    }
    explicit BLOBNBOX(C_BLOB *srcblob) {
      blob_ptr = NULL;
      cblob_ptr = srcblob;
      box = srcblob->bounding_box ();
      area = (int) srcblob->area ();
      Init();
    }

    void rotate_box(FCOORD vec) {
      box.rotate(vec);
    }
    void translate_box(ICOORD v) {
      box.move(v);
    }
    void merge(BLOBNBOX *nextblob);
    void chop(                        // fake chop blob
              BLOBNBOX_IT *start_it,  // location of this
              BLOBNBOX_IT *blob_it,   // iterator
              FCOORD rotation,        // for landscape
              float xheight);         // line height

    // Simple accessors.
    const TBOX& bounding_box() const {
      return box;
    }
    void compute_bounding_box() {
      box = cblob_ptr != NULL ? cblob_ptr->bounding_box()
                              : blob_ptr->bounding_box();
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
    PBLOB *blob() const {
      return blob_ptr;
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
    tesseract::ColPartition* owner() const {
      return owner_;
    }
    void set_owner(tesseract::ColPartition* new_owner) {
      owner_ = new_owner;
    }
    void set_noise_flag(bool flag) {
      noise_flag_ = flag;
    }
    bool noise_flag() const {
      return noise_flag_;
    }

#ifndef GRAPHICS_DISABLED
    // Keep in sync with BlobRegionType.
    ScrollView::Color BoxColor() const {
      switch (region_type_) {
      case BRT_HLINE:
        return ScrollView::YELLOW;
      case BRT_RECTIMAGE:
        return ScrollView::RED;
      case BRT_POLYIMAGE:
        return ScrollView::ORANGE;
      case BRT_UNKNOWN:
        return ScrollView::CYAN;
      case BRT_VERT_TEXT:
        return ScrollView::GREEN;
      case BRT_TEXT:
        return ScrollView::BLUE;
      case BRT_NOISE:
      default:
        return ScrollView::GREY;
      }
    }

    void plot(ScrollView* window,                // window to draw in
              ScrollView::Color blob_colour,     // for outer bits
              ScrollView::Color child_colour) {  // for holes
      if (blob_ptr != NULL)
        blob_ptr->plot(window, blob_colour, child_colour);
      if (cblob_ptr != NULL)
        cblob_ptr->plot(window, blob_colour, child_colour);
    }
#endif

    NEWDELETE2(BLOBNBOX)

 private:
  // Initializes the bulk of the members to default values.
  void Init() {
    joined = false;
    reduced = false;
    repeated_set_ = 0;
    left_tab_type_ = TT_NONE;
    right_tab_type_ = TT_NONE;
    region_type_ = BRT_UNKNOWN;
    left_rule_ = 0;
    right_rule_ = 0;
    left_crossing_rule_ = 0;
    right_crossing_rule_ = 0;
    horz_stroke_width_ = 0.0f;
    vert_stroke_width_ = 0.0f;
    owner_ = NULL;
    noise_flag_ = false;
  }

  PBLOB *blob_ptr;              // poly blob
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
  inT16 left_rule_;             // x-coord of nearest but not crossing rule line
  inT16 right_rule_;            // x-coord of nearest but not crossing rule line
  inT16 left_crossing_rule_;    // x-coord of nearest or crossing rule line
  inT16 right_crossing_rule_;   // x-coord of nearest or crossing rule line
  float horz_stroke_width_;     // Median horizontal stroke width
  float vert_stroke_width_;     // Median vertical stroke width
  tesseract::ColPartition* owner_;  // Who will delete me when I am not needed
  // Was the blob flagged as noise in the initial filtering step
  bool noise_flag_;
};

class TO_ROW:public ELIST2_LINK
{
  public:
    TO_ROW() {
      num_repeated_sets_ = -1;
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
        (float) (blobs.length () - textord_error_weight * new_error);
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
    NEWDELETE2 (TO_ROW) BOOL8 merged;
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
    TO_BLOCK() {
    }                            //empty
    TO_BLOCK(                    //constructor
             BLOCK *src_block);  //real block
    ~TO_BLOCK();

    TO_ROW_LIST *get_rows() {  //access function
      return &row_list;
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

    NEWDELETE2 (TO_BLOCK) private:
    TO_ROW_LIST row_list;        //temporary rows
};

ELISTIZEH (TO_BLOCK)
extern double_VAR_H (textord_error_weight, 3,
"Weighting for error in believability");
void find_blob_limits(                  //get y limits
                      PBLOB *blob,      //blob to search
                      float leftx,      //x limits
                      float rightx,
                      FCOORD rotation,  //for landscape
                      float &ymin,      //output y limits
                      float &ymax);
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
PBLOB *rotate_blob(                 //get y limits
                   PBLOB *blob,     //blob to search
                   FCOORD rotation  //vector to rotate by
                  );
PBLOB *rotate_cblob(                 //rotate it
                    C_BLOB *blob,    //blob to search
                    float xheight,   //for poly approx
                    FCOORD rotation  //for landscape
                   );
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
void vertical_blob_projection(              //project outlines
                              PBLOB *blob,  //blob to project
                              STATS *stats  //output
                             );
                                 //project outlines
void vertical_outline_projection(OUTLINE *outline,  //outline to project
                                 STATS *stats       //output
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
#endif //GRAPHICS_DISABLED
#endif
