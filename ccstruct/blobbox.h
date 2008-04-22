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

class BLOBNBOX;
ELISTIZEH (BLOBNBOX)
class BLOBNBOX:public ELIST_LINK
{
  public:
    BLOBNBOX() {  //empty
      blob_ptr = NULL;
      cblob_ptr = NULL;
      joined = FALSE;
      reduced = FALSE;
      area = 0;
    }
    BLOBNBOX(  //constructor
             PBLOB *srcblob) {
      blob_ptr = srcblob;
      cblob_ptr = NULL;
      box = srcblob->bounding_box ();
      joined = FALSE;
      reduced = FALSE;
      area = (int) srcblob->area ();
    }
    BLOBNBOX(  //constructor
             C_BLOB *srcblob) {
      blob_ptr = NULL;
      cblob_ptr = srcblob;
      box = srcblob->bounding_box ();
      joined = FALSE;
      reduced = FALSE;
      area = (int) srcblob->area ();
    }

                                 //get bounding box
    const TBOX &bounding_box() const {
      return box;
    }
                                 //get bounding box
    const TBOX &reduced_box() const {
      return red_box;
    }
    void set_reduced_box(  //set other box
                         TBOX new_box) {
      red_box = new_box;
      reduced = TRUE;
    }
    inT32 enclosed_area() const {  //get area
      return area;
    }

    void rotate_box(  //just box
                    FCOORD vec) {
      box.rotate (vec);
    }

    BOOL8 joined_to_prev() const {  //access function
      return joined != 0;
    }
    BOOL8 red_box_set() const {  //access function
      return reduced != 0;
    }
    void merge(  //merge with next
               BLOBNBOX *nextblob);
    void chop(                        //fake chop blob
              BLOBNBOX_IT *start_it,  //location of this
              BLOBNBOX_IT *blob_it,   //iterator
              FCOORD rotation,        //for landscape
              float xheight);         //line height

    PBLOB *blob() {  //access function
      return blob_ptr;
    }
    C_BLOB *cblob() {  //access function
      return cblob_ptr;
    }

#ifndef GRAPHICS_DISABLED
    void plot(                        //draw one
              ScrollView* window,          //window to draw in
              ScrollView::Color blob_colour,     //for outer bits
              ScrollView::Color child_colour) {  //for holes
      if (blob_ptr != NULL)
        blob_ptr->plot (window, blob_colour, child_colour);
      if (cblob_ptr != NULL)
        cblob_ptr->plot (window, blob_colour, child_colour);
    }
#endif

    NEWDELETE2 (BLOBNBOX) private:
    int area:30;                 //enclosed area
    int joined:1;                //joined to prev
    int reduced:1;               //reduced box set
    TBOX box;                     //bounding box
    TBOX red_box;                 //bounding box
    PBLOB *blob_ptr;             //poly blob
    C_BLOB *cblob_ptr;           //edgestep blob
};

class TO_ROW:public ELIST2_LINK
{
  public:
    TO_ROW() {
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

                                 //true when dead
    NEWDELETE2 (TO_ROW) BOOL8 merged;
    BOOL8 all_caps;              //had no ascenders
    BOOL8 used_dm_model;         //in guessing pitch
    inT16 projection_left;       //start of projection
    inT16 projection_right;      //start of projection
    PITCH_TYPE pitch_decision;   //how strong is decision
    float fixed_pitch;           //pitch or 0
    float fp_space;              //sp if fixed pitch
    float fp_nonsp;              //nonsp if fixed pitch
    float pr_space;              //sp if prop
    float pr_nonsp;              //non sp if prop
    float spacing;               //to "next" row
    float xheight;               //of line
    float ascrise;               //ascenders
    float descdrop;              //descenders
    inT32 min_space;             //min size for real space
    inT32 max_nonspace;          //max size of non-space
    inT32 space_threshold;       //space vs nonspace
    float kern_size;             //average non-space
    float space_size;            //average space
    WERD_LIST rep_words;         //repeated chars
    ICOORDELT_LIST char_cells;   //fixed pitch cells
    QSPLINE baseline;            //curved baseline
    STATS projection;            //vertical projection

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
#endif
