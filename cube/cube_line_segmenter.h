/**********************************************************************
 * File:        cube_page_segmenter.h
 * Description: Declaration of the Cube Page Segmenter Class
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

// TODO(ahmadab)
// This is really a makeshift line segmenter that works well for Arabic
// This should eventually be replaced by Ray Smith's Page segmenter
// There are lots of magic numbers below that were determined empirically
// but not thoroughly tested

#ifndef CUBE_LINE_SEGMENTER_H
#define CUBE_LINE_SEGMENTER_H

#include "cube_reco_context.h"
#include "allheaders.h"

namespace tesseract {

class CubeLineSegmenter {
 public:
  CubeLineSegmenter(CubeRecoContext *cntxt, Pix *img);
  ~CubeLineSegmenter();

  // Accessor functions
  Pix *PostProcessedImage() {
    if (init_ == false && Init() == false) {
      return NULL;
    }
    return img_;
  }
  int ColumnCnt() {
    if (init_ == false && Init() == false) {
      return NULL;
    }
    return columns_->n;
  }
  Box *Column(int col) {
    if (init_ == false && Init() == false) {
      return NULL;
    }

    return columns_->boxa->box[col];
  }
  int LineCnt() {
    if (init_ == false && Init() == false) {
      return NULL;
    }

    return line_cnt_;
  }
  Pixa *ConComps() {
    if (init_ == false && Init() == false) {
      return NULL;
    }

    return con_comps_;
  }
  Pixaa *Columns() {
    if (init_ == false && Init() == false) {
      return NULL;
    }

    return columns_;
  }
  inline double AlefHgtEst() { return est_alef_hgt_; }
  inline double DotHgtEst() { return est_dot_hgt_; }
  Pix *Line(int line, Box **line_box);

 private:
  static const float kMinValidLineHgtRatio;
  static const int kLineSepMorphMinHgt;
  static const int kHgtBins;
  static const int kMaxConnCompHgt;
  static const int kMaxConnCompWid;
  static const int kMaxHorzAspectRatio;
  static const int kMaxVertAspectRatio;
  static const int kMinWid;
  static const int kMinHgt;
  static const double kMaxValidLineRatio;

  // Cube Reco context
  CubeRecoContext *cntxt_;
  // Original image
  Pix *orig_img_;
  // Post processed image
  Pix *img_;
  // Init flag
  bool init_;
  // Output Line and column info
  int line_cnt_;
  Pixaa *columns_;
  Pixa *con_comps_;
  Pixa *lines_pixa_;
  // Estimates for sizes of ALEF and DOT needed for Arabic analysis
  double est_alef_hgt_;
  double est_dot_hgt_;

  // Init the page analysis
  bool Init();
  // Performs line segmentation
  bool LineSegment();
  // Cleanup function
  Pix *CleanUp(Pix *pix);
  // compute validity ratio for a line
  double ValidityRatio(Pix *line_mask_pix, Box *line_box);
  // validate line
  bool ValidLine(Pix *line_mask_pix, Box *line_box);
  // split a line continuously until valid or fail
  Pixa *SplitLine(Pix *line_mask_pix, Box *line_box);
  // do a desperate attempt at cracking lines
  Pixa *CrackLine(Pix *line_mask_pix, Box *line_box);
  Pixa *CrackLine(Pix *line_mask_pix, Box *line_box, int line_cnt);
  // Checks of a line is too small
  bool SmallLine(Box *line_box);
  // Compute the connected components in a line
  Boxa * ComputeLineConComps(Pix *line_mask_pix, Box *line_box,
                             Pixa **con_comps_pixa);
  // create a union of two arbitrary pix
  Pix *PixUnion(Pix *dest_pix, Box *dest_box, Pix *src_pix, Box *src_box);
  // create a union of a pixa subset
  Pix *Pixa2Pix(Pixa *pixa, Box **dest_box, int start_pix, int pix_cnt);
  // create a union of a pixa
  Pix *Pixa2Pix(Pixa *pixa, Box **dest_box);
  // merges a number of lines into one line given a bounding box and a mask
  bool MergeLine(Pix *line_mask_pix, Box *line_box,
                 Pixa *lines, Boxaa *lines_con_comps);
  // Creates new set of lines from the computed columns
  bool AddLines(Pixa *lines);
  // Estimate the parameters of the font(s) used in the page
  bool EstimateFontParams();
  // perform a vertical Closing with the specified threshold
  // returning the resulting conn comps as a pixa
  Pixa *VerticalClosing(Pix *pix, int thresold, Boxa **boxa);
  // Index the specific pixa using RTL reading order
  int *IndexRTL(Pixa *pixa);
  // Implements a rudimentary page & line segmenter
  bool FindLines();
};
}

#endif  // CUBE_LINE_SEGMENTER_H
