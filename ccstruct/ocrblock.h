/**********************************************************************
 * File:        ocrblock.h  (Formerly block.h)
 * Description: Page block class definition.
 * Author:		Ray Smith
 * Created:		Thu Mar 14 17:32:01 GMT 1991
 *
 * (C) Copyright 1991, Hewlett-Packard Ltd.
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

#ifndef           OCRBLOCK_H
#define           OCRBLOCK_H

#include          "img.h"
#include          "ocrrow.h"
#include          "pdblock.h"

class BLOCK;                     //forward decl

ELISTIZEH_S (BLOCK)
class BLOCK:public ELIST_LINK, public PDBLK
//page block
{
  friend class BLOCK_RECT_IT;    //block iterator

 public:
  BLOCK()
    : re_rotation_(1.0f, 0.0f),
      classify_rotation_(1.0f, 0.0f),
      skew_(1.0f, 0.0f) {
    hand_poly = NULL;
  }
  BLOCK(const char *name,  //< filename
        BOOL8 prop,        //< proportional
        inT16 kern,        //< kerning
        inT16 space,       //< spacing
        inT16 xmin,        //< bottom left
        inT16 ymin,
        inT16 xmax,        //< top right
        inT16 ymax);

  ~BLOCK () {
  }

  /**
   * set space size etc.
   * @param prop proportional
   * @param kern inter char size
   * @param space inter word size
   * @param ch_pitch pitch if fixed
   */
  void set_stats(BOOL8 prop,
                 inT16 kern,
                 inT16 space,
                 inT16 ch_pitch) {
    proportional = prop;
    kerning = (inT8) kern;
    spacing = space;
    pitch = ch_pitch;
  }
  /// set char size
  void set_xheight(inT32 height) {
    xheight = height;
  }
  /// set font class
  void set_font_class(inT16 font) {
    font_class = font;
  }
  /// return proportional
  BOOL8 prop() const {
    return proportional;
  }
  /// return pitch
  inT32 fixed_pitch() const {
    return pitch;
  }
  /// return kerning
  inT16 kern() const {
    return kerning;
  }
  /// return font class
  inT16 font() const {
    return font_class;
  }
  /// return spacing
  inT16 space() const {
    return spacing;
  }
  /// return filename
  const char *name() const {
    return filename.string ();
  }
  /// return xheight
  inT32 x_height() const {
    return xheight;
  }
  float cell_over_xheight() const {
    return cell_over_xheight_;
  }
  void set_cell_over_xheight(float ratio) {
    cell_over_xheight_ = ratio;
  }
  /// get rows
  ROW_LIST *row_list() {
    return &rows;
  }
  /// get blobs
  C_BLOB_LIST *blob_list() {
    return &c_blobs;
  }
  C_BLOB_LIST *reject_blobs() {
    return &rej_blobs;
  }
  FCOORD re_rotation() const {
    return re_rotation_;         // How to transform coords back to image.
  }
  void set_re_rotation(const FCOORD& rotation) {
    re_rotation_ = rotation;
  }
  FCOORD classify_rotation() const {
    return classify_rotation_;   // Apply this before classifying.
  }
  void set_classify_rotation(const FCOORD& rotation) {
    classify_rotation_ = rotation;
  }
  FCOORD skew() const {
    return skew_;                // Direction of true horizontal.
  }
  void set_skew(const FCOORD& skew) {
    skew_ = skew;
  }
  const ICOORD& median_size() const {
    return median_size_;
  }
  void set_median_size(int x, int y) {
    median_size_.set_x(x);
    median_size_.set_y(y);
  }

  void rotate(const FCOORD& rotation);

  /// decreasing y order
  void sort_rows();

  /// shrink white space
  void compress();

  /// check proportional
  void check_pitch();

  /// shrink white space and move by vector
  void compress(const ICOORD vec);

  /// dump whole table
  void print(FILE *fp, BOOL8 dump);

  /// set ptrs to counts
  void prep_serialise() {
    filename.prep_serialise();
    rows.prep_serialise();
    c_blobs.prep_serialise();
    rej_blobs.prep_serialise();
    leftside.prep_serialise();
    rightside.prep_serialise();
  }

  void dump(FILE *f) {
    filename.dump(f);
    rows.dump(f);
    c_blobs.dump(f);
    rej_blobs.dump(f);
    leftside.dump(f);
    rightside.dump(f);
  }

  /// read external bits
  void de_dump(FILE *f) {
    filename.de_dump(f);
    rows.de_dump(f);
    c_blobs.de_dump(f);
    rej_blobs.de_dump(f);
    leftside.de_dump(f);
    rightside.de_dump(f);
  }

  make_serialise(BLOCK)

  BLOCK& operator=(const BLOCK & source);

 private:
  BOOL8 proportional;          //< proportional
  inT8 kerning;                //< inter blob gap
  inT16 spacing;               //< inter word gap
  inT16 pitch;                 //< pitch of non-props
  inT16 font_class;            //< correct font class
  inT32 xheight;               //< height of chars
  float cell_over_xheight_;    //< Ratio of cell height to xheight.
  STRING filename;             //< name of block
  ROW_LIST rows;               //< rows in block
  C_BLOB_LIST c_blobs;         //< before textord
  C_BLOB_LIST rej_blobs;       //< duff stuff
  FCOORD re_rotation_;         //< How to transform coords back to image.
  FCOORD classify_rotation_;   //< Apply this before classifying.
  FCOORD skew_;                //< Direction of true horizontal.
  ICOORD median_size_;         //< Median size of blobs.
};

int decreasing_top_order(const void *row1, const void *row2);

#endif
