/**********************************************************************
 * File:        char_samp.h
 * Description: Declaration of a Character Bitmap Sample Class
 * Author:    Ahmad Abdulkader
 * Created:   2007
 *
 * (C) Copyright 2008, Google Inc.
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

// The CharSamp inherits the Bmp8 class that represents images of
// words, characters and segments throughout Cube
// CharSamp adds more data members to hold the physical location of the image
// in a page, page number in a book if available.
// It also holds the label (GT) of the image that might correspond to a single
// character or a word
// It also provides methods for segmenting, scaling and cropping of the sample

#ifndef CHAR_SAMP_H
#define CHAR_SAMP_H

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include "bmp_8.h"
#include "string_32.h"

namespace tesseract {

class CharSamp : public Bmp8 {
 public:
  CharSamp();
  CharSamp(int wid, int hgt);
  CharSamp(int left, int top, int wid, int hgt);
  ~CharSamp();
  // accessor methods
  unsigned short Left() const { return left_; }
  unsigned short Right() const { return left_ + wid_; }
  unsigned short Top() const { return top_; }
  unsigned short Bottom() const { return top_ + hgt_; }
  unsigned short Page() const { return page_; }
  unsigned short NormTop() const { return norm_top_; }
  unsigned short NormBottom() const { return norm_bottom_; }
  unsigned short NormAspectRatio() const { return norm_aspect_ratio_; }
  unsigned short FirstChar() const { return first_char_; }
  unsigned short LastChar() const { return last_char_; }
  char_32 Label() const {
    if (label32_ == NULL || LabelLen() != 1) {
      return 0;
    }
    return label32_[0];
  }
  char_32 * StrLabel() const { return label32_; }
  string stringLabel() const;

  void SetLeft(unsigned short left) { left_ = left; }
  void SetTop(unsigned short top) { top_ = top; }
  void SetPage(unsigned short page) { page_ = page; }
  void SetLabel(char_32 label) {
    if (label32_ != NULL) {
      delete []label32_;
    }
    label32_ = new char_32[2];
    if (label32_ != NULL) {
      label32_[0] = label;
      label32_[1] = 0;
    }
  }
  void SetLabel(const char_32 *label32) {
    if (label32_ != NULL) {
      delete []label32_;
      label32_ = NULL;
    }
    if (label32 != NULL) {
      // remove any byte order markes if any
      if (label32[0] == 0xfeff) {
        label32++;
      }
      int len = LabelLen(label32);
      label32_ = new char_32[len + 1];
      if (label32_ != NULL) {
        memcpy(label32_, label32, len * sizeof(*label32));
        label32_[len] = 0;
      }
    }
  }
  void SetLabel(string str);
  void SetNormTop(unsigned short norm_top) { norm_top_ = norm_top; }
  void SetNormBottom(unsigned short norm_bottom) {
    norm_bottom_ = norm_bottom;
  }
  void SetNormAspectRatio(unsigned short norm_aspect_ratio) {
    norm_aspect_ratio_ = norm_aspect_ratio;
  }
  void SetFirstChar(unsigned short first_char) {
    first_char_ = first_char;
  }
  void SetLastChar(unsigned short last_char) {
    last_char_ = last_char;
  }

  // Saves the charsamp to a dump file
  bool Save2CharDumpFile(FILE *fp) const;
  // Crops the underlying image and returns a new CharSamp with the
  // same character information but new dimensions. Warning: does not
  // necessarily set the normalized top and bottom correctly since
  // those depend on its location within the word (or CubeSearchObject).
  CharSamp *Crop();
  // Computes the connected components of the char sample
  ConComp **Segment(int *seg_cnt, bool right_2_left, int max_hist_wnd,
                    int min_con_comp_size) const;
  // returns a copy of the charsamp that is scaled to the
  // specified width and height
  CharSamp *Scale(int wid, int hgt, bool isotropic = true);
  // returns a Clone of the charsample
  CharSamp *Clone() const;
  // computes the features corresponding to the char sample
  bool ComputeFeatures(int conv_grid_size, float *features);
  // Load a Char Samp from a dump file
  static CharSamp *FromCharDumpFile(CachedFile *fp);
  static CharSamp *FromCharDumpFile(FILE *fp);
  static CharSamp *FromCharDumpFile(unsigned char **raw_data);
  static CharSamp *FromRawData(int left, int top, int wid, int hgt,
    unsigned char *data);
  static CharSamp *FromConComps(ConComp **concomp_array,
                                int strt_concomp, int seg_flags_size,
                                int *seg_flags, bool *left_most,
                                bool *right_most, int word_hgt);
  static int AuxFeatureCnt() { return (5); }
  // Return the length of the label string
  int LabelLen() const { return LabelLen(label32_); }
  static int LabelLen(const char_32 *label32) {
    if (label32 == NULL) {
      return 0;
    }
    int len = 0;
    while (label32[++len] != 0);
    return len;
  }
 private:
  char_32 * label32_;
  unsigned short page_;
  unsigned short left_;
  unsigned short top_;
  // top of sample normalized to a word height of 255
  unsigned short norm_top_;
  // bottom of sample normalized to a word height of 255
  unsigned short norm_bottom_;
  // 255 * ratio of character width to (width + height)
  unsigned short norm_aspect_ratio_;
  unsigned short first_char_;
  unsigned short last_char_;
};

}

#endif  // CHAR_SAMP_H
