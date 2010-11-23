/**********************************************************************
 * File:        cube_page_segmenter.cpp
 * Description: Implementation of the Cube Page Segmenter Class
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

#include "cube_line_segmenter.h"
#include "ndminx.h"

namespace tesseract {
// constants that worked for Arabic page segmenter
const int CubeLineSegmenter::kLineSepMorphMinHgt = 20;
const int CubeLineSegmenter::kHgtBins = 20;
const double CubeLineSegmenter::kMaxValidLineRatio = 3.2;
const int CubeLineSegmenter::kMaxConnCompHgt = 150;
const int CubeLineSegmenter::kMaxConnCompWid = 500;
const int CubeLineSegmenter::kMaxHorzAspectRatio = 50;
const int CubeLineSegmenter::kMaxVertAspectRatio = 20;
const int CubeLineSegmenter::kMinWid = 2;
const int CubeLineSegmenter::kMinHgt = 2;
const float CubeLineSegmenter::kMinValidLineHgtRatio = 2.5;

CubeLineSegmenter::CubeLineSegmenter(CubeRecoContext *cntxt, Pix *img) {
  cntxt_ = cntxt;
  orig_img_ = img;
  img_ = NULL;
  lines_pixa_ = NULL;
  init_ = false;
  line_cnt_ = 0;
  columns_ = NULL;
  con_comps_ = NULL;
  est_alef_hgt_ = 0.0;
  est_dot_hgt_ = 0.0;
}

CubeLineSegmenter::~CubeLineSegmenter() {
  if (img_ != NULL) {
    pixDestroy(&img_);
    img_ = NULL;
  }

  if (lines_pixa_ != NULL) {
    pixaDestroy(&lines_pixa_);
    lines_pixa_ = NULL;
  }

  if (con_comps_ != NULL) {
    pixaDestroy(&con_comps_);
    con_comps_ = NULL;
  }

  if (columns_ != NULL) {
    pixaaDestroy(&columns_);
    columns_ = NULL;
  }
}

// compute validity ratio for a line
double CubeLineSegmenter::ValidityRatio(Pix *line_mask_pix, Box *line_box) {
  return line_box->h / est_alef_hgt_;
}

// validate line
bool CubeLineSegmenter::ValidLine(Pix *line_mask_pix, Box *line_box) {
  double validity_ratio = ValidityRatio(line_mask_pix, line_box);

  return validity_ratio < kMaxValidLineRatio;
}

// perform a vertical Closing with the specified threshold
// returning the resulting conn comps as a pixa
Pixa *CubeLineSegmenter::VerticalClosing(Pix *pix,
    int threshold, Boxa **boxa) {
  char sequence_str[16];

    // do the morphology
  sprintf(sequence_str, "c100.%d", threshold);
  Pix *morphed_pix = pixMorphCompSequence(pix, sequence_str, 0);
  if (morphed_pix == NULL) {
    return NULL;
  }

    // get the resulting lines by computing concomps
  Pixa *pixac;
  (*boxa) = pixConnComp(morphed_pix, &pixac, 8);

  pixDestroy(&morphed_pix);

  if ((*boxa) == NULL) {
    return NULL;
  }

  return pixac;
}

// do a desperate attempt at cracking lines
Pixa *CubeLineSegmenter::CrackLine(Pix *cracked_line_pix,
                                   Box *cracked_line_box, int line_cnt) {
  // create lines pixa array
  Pixa **lines_pixa = new Pixa*[line_cnt];
  if (lines_pixa == NULL) {
    return NULL;
  }

  memset(lines_pixa, 0, line_cnt * sizeof(*lines_pixa));

  // compute line conn comps
  Pixa *line_con_comps_pix;
  Boxa *line_con_comps = ComputeLineConComps(cracked_line_pix,
    cracked_line_box, &line_con_comps_pix);

  if (line_con_comps == NULL) {
    delete []lines_pixa;
    return false;
  }

  // assign each conn comp to the a line based on its centroid
  for (int con = 0; con < line_con_comps->n; con++) {
    Box *con_box = line_con_comps->box[con];
    Pix *con_pix = line_con_comps_pix->pix[con];
    int mid_y = (con_box->y - cracked_line_box->y) + (con_box->h / 2),
      line_idx = MIN(line_cnt - 1,
                     (mid_y * line_cnt / cracked_line_box->h));

    // create the line if it has not been created?
    if (lines_pixa[line_idx] == NULL) {
      lines_pixa[line_idx] = pixaCreate(line_con_comps->n);
      if (lines_pixa[line_idx] == NULL) {
        delete []lines_pixa;
        boxaDestroy(&line_con_comps);
        pixaDestroy(&line_con_comps_pix);
        return false;
      }
    }

    // add the concomp to the line
    if (pixaAddPix(lines_pixa[line_idx], con_pix, L_CLONE) != 0 ||
        pixaAddBox(lines_pixa[line_idx], con_box, L_CLONE)) {
      delete []lines_pixa;
      boxaDestroy(&line_con_comps);
      pixaDestroy(&line_con_comps_pix);
    }
  }

  // create the lines pixa
  Pixa *lines = pixaCreate(line_cnt);
  bool success = true;

  // create and check the validity of the lines
  for (int line = 0; line < line_cnt; line++) {
    Pixa *line_pixa = lines_pixa[line];

    // skip invalid lines
    if (line_pixa == NULL) {
      continue;
    }

    // merge the pix, check the validity of the line
    // and add it to the lines pixa
    Box *line_box;
    Pix *line_pix = Pixa2Pix(line_pixa, &line_box);
    if (line_pix == NULL ||
        line_box == NULL ||
        ValidLine(line_pix, line_box) == false ||
        pixaAddPix(lines, line_pix, L_INSERT) != 0 ||
        pixaAddBox(lines, line_box, L_INSERT) != 0) {
      if (line_pix != NULL) {
        pixDestroy(&line_pix);
      }

      if (line_box != NULL) {
        boxDestroy(&line_box);
      }

      success = false;

      break;
    }
  }

  // cleanup
  for (int line = 0; line < line_cnt; line++) {
    if (lines_pixa[line] != NULL) {
      pixaDestroy(&lines_pixa[line]);
    }
  }

  delete []lines_pixa;
  boxaDestroy(&line_con_comps);
  pixaDestroy(&line_con_comps_pix);

  if (success == false) {
    pixaDestroy(&lines);
    lines = NULL;
  }

  return lines;
}

// do a desperate attempt at cracking lines
Pixa *CubeLineSegmenter::CrackLine(Pix *cracked_line_pix,
                                   Box *cracked_line_box) {
  // estimate max line count
  int max_line_cnt = static_cast<int>((cracked_line_box->h /
      est_alef_hgt_) + 0.5);
  if (max_line_cnt < 2) {
    return NULL;
  }

  for (int line_cnt = 2; line_cnt < max_line_cnt; line_cnt++) {
    Pixa *lines = CrackLine(cracked_line_pix, cracked_line_box, line_cnt);
    if (lines != NULL) {
      return lines;
    }
  }

  return NULL;
}

// split a line continously until valid or fail
Pixa *CubeLineSegmenter::SplitLine(Pix *line_mask_pix, Box *line_box) {
  // clone the line mask
  Pix *line_pix = pixClone(line_mask_pix);

  if (line_pix == NULL) {
    return NULL;
  }

  // AND with the image to get the actual line
  pixRasterop(line_pix, 0, 0, line_pix->w, line_pix->h,
    PIX_SRC & PIX_DST, img_, line_box->x, line_box->y);

  // continue to do rasterop morphology on the line until
  // it splits to valid lines or we fail
  int morph_hgt = kLineSepMorphMinHgt - 1,
    best_threshold = kLineSepMorphMinHgt - 1,
    max_valid_portion = 0;

  Boxa *boxa;
  Pixa *pixac;

  do {
    pixac = VerticalClosing(line_pix, morph_hgt, &boxa);

    // add the box offset to all the lines
    // and check for the validity of each
    int line,
      valid_line_cnt = 0,
      valid_portion = 0;

    for (line = 0; line < pixac->n; line++) {
      boxa->box[line]->x += line_box->x;
      boxa->box[line]->y += line_box->y;

      if (ValidLine(pixac->pix[line], boxa->box[line]) == true) {
        // count valid lines
        valid_line_cnt++;

        // and the valid portions
        valid_portion += boxa->box[line]->h;
      }
    }

    // all the lines are valid
    if (valid_line_cnt == pixac->n) {
      boxaDestroy(&boxa);
      pixDestroy(&line_pix);
      return pixac;
    }

    // a larger valid portion
    if (valid_portion > max_valid_portion) {
      max_valid_portion = valid_portion;
      best_threshold = morph_hgt;
    }

    boxaDestroy(&boxa);
    pixaDestroy(&pixac);

    morph_hgt--;
  }
  while (morph_hgt > 0);

  // failed to break into valid lines
  // attempt to crack the line
  pixac = CrackLine(line_pix, line_box);
  if (pixac != NULL) {
    pixDestroy(&line_pix);
    return pixac;
  }

  // try to leverage any of the lines
  // did the best threshold yield a non zero valid portion
  if (max_valid_portion > 0) {
    // use this threshold to break lines
    pixac = VerticalClosing(line_pix, best_threshold, &boxa);

    // add the box offset to all the lines
    // and check for the validity of each
    for (int line = 0; line < pixac->n; line++) {
      boxa->box[line]->x += line_box->x;
      boxa->box[line]->y += line_box->y;

      // remove invalid lines from the pixa
      if (ValidLine(pixac->pix[line], boxa->box[line]) == false) {
        pixaRemovePix(pixac, line);
        line--;
      }
    }

    boxaDestroy(&boxa);
    pixDestroy(&line_pix);
    return pixac;
  }

  // last resort: attempt to crack the line
  pixDestroy(&line_pix);

  return NULL;
}

// Checks of a line is too small
bool CubeLineSegmenter::SmallLine(Box *line_box) {
  return line_box->h <= (kMinValidLineHgtRatio * est_dot_hgt_);
}

// Compute the connected components in a line
Boxa * CubeLineSegmenter::ComputeLineConComps(Pix *line_mask_pix,
                                              Box *line_box,
                                              Pixa **con_comps_pixa) {
  // clone the line mask
  Pix *line_pix = pixClone(line_mask_pix);

  if (line_pix == NULL) {
    return NULL;
  }

  // AND with the image to get the actual line
  pixRasterop(line_pix, 0, 0, line_pix->w, line_pix->h,
    PIX_SRC & PIX_DST, img_, line_box->x, line_box->y);

  // compute the connected components of the line to be merged
  Boxa *line_con_comps = pixConnComp(line_pix, con_comps_pixa, 8);

  pixDestroy(&line_pix);

  // offset boxes by the bbox of the line
  for (int con = 0; con < line_con_comps->n; con++) {
    line_con_comps->box[con]->x += line_box->x;
    line_con_comps->box[con]->y += line_box->y;
  }

  return line_con_comps;
}

// create a union of two arbitrary pix
Pix *CubeLineSegmenter::PixUnion(Pix *dest_pix, Box *dest_box,
    Pix *src_pix, Box *src_box) {
  // compute dimensions of union rect
  BOX *union_box = boxBoundingRegion(src_box, dest_box);

  // create the union pix
  Pix *union_pix = pixCreate(union_box->w, union_box->h, src_pix->d);
  if (union_pix == NULL) {
    return NULL;
  }

  // blt the src and dest pix
  pixRasterop(union_pix,
    src_box->x - union_box->x, src_box->y - union_box->y,
    src_box->w, src_box->h, PIX_SRC | PIX_DST, src_pix, 0, 0);

  pixRasterop(union_pix,
    dest_box->x - union_box->x, dest_box->y - union_box->y,
    dest_box->w, dest_box->h, PIX_SRC | PIX_DST, dest_pix, 0, 0);

  // replace the dest_box
  *dest_box = *union_box;

  boxDestroy(&union_box);

  return union_pix;
}

// create a union of a number of arbitrary pix
Pix *CubeLineSegmenter::Pixa2Pix(Pixa *pixa, Box **dest_box,
                                 int start_pix, int pix_cnt) {
  // compute union_box
  int min_x = INT_MAX,
    max_x = INT_MIN,
    min_y = INT_MAX,
    max_y = INT_MIN;

  for (int pix_idx = start_pix; pix_idx < (start_pix + pix_cnt); pix_idx++) {
    Box *pix_box = pixa->boxa->box[pix_idx];

    UpdateRange(pix_box->x, pix_box->x + pix_box->w, &min_x, &max_x);
    UpdateRange(pix_box->y, pix_box->y + pix_box->h, &min_y, &max_y);
  }

  (*dest_box) = boxCreate(min_x, min_y, max_x - min_x, max_y - min_y);
  if ((*dest_box) == NULL) {
    return false;
  }

  // create the union pix
  Pix *union_pix = pixCreate((*dest_box)->w, (*dest_box)->h, img_->d);
  if (union_pix == NULL) {
    boxDestroy(dest_box);
    return false;
  }

  // create a pix corresponding to the union of all pixs
  // blt the src and dest pix
  for (int pix_idx = start_pix; pix_idx < (start_pix + pix_cnt); pix_idx++) {
    Box *pix_box = pixa->boxa->box[pix_idx];
    Pix *con_pix = pixa->pix[pix_idx];

    pixRasterop(union_pix,
                pix_box->x - (*dest_box)->x, pix_box->y - (*dest_box)->y,
                pix_box->w, pix_box->h, PIX_SRC | PIX_DST, con_pix, 0, 0);
  }

  return union_pix;
}

// create a union of a number of arbitrary pix
Pix *CubeLineSegmenter::Pixa2Pix(Pixa *pixa, Box **dest_box) {
  return Pixa2Pix(pixa, dest_box, 0, pixa->n);
}

// merges a number of lines into one line given a bounding box and a mask
bool CubeLineSegmenter::MergeLine(Pix *line_mask_pix, Box *line_box,
                                  Pixa *lines, Boxaa *lines_con_comps) {
  // compute the connected components of the lines to be merged
  Pixa *small_con_comps_pix;
  Boxa *small_line_con_comps = ComputeLineConComps(line_mask_pix,
      line_box, &small_con_comps_pix);

  if (small_line_con_comps == NULL) {
    return false;
  }

  // for each connected component
  for (int con = 0; con < small_line_con_comps->n; con++) {
    Box *small_con_comp_box = small_line_con_comps->box[con];
    int best_line = -1,
      best_dist = INT_MAX,
      small_box_right = small_con_comp_box->x + small_con_comp_box->w,
      small_box_bottom = small_con_comp_box->y + small_con_comp_box->h;

    // for each valid line
    for (int line = 0; line < lines->n; line++) {
      if (SmallLine(lines->boxa->box[line]) == true) {
        continue;
      }

      // for all the connected components in the line
      Boxa *line_con_comps = lines_con_comps->boxa[line];

      for (int lcon = 0; lcon < line_con_comps->n; lcon++) {
        Box *con_comp_box = line_con_comps->box[lcon];
        int xdist,
          ydist,
          box_right = con_comp_box->x + con_comp_box->w,
          box_bottom = con_comp_box->y + con_comp_box->h;

        xdist = MAX(small_con_comp_box->x, con_comp_box->x) -
            MIN(small_box_right, box_right);

        ydist = MAX(small_con_comp_box->y, con_comp_box->y) -
            MIN(small_box_bottom, box_bottom);

        // if there is an overlap in x-direction
        if (xdist <= 0) {
          if (best_line == -1 || ydist < best_dist) {
            best_dist = ydist;
            best_line = line;
          }
        }
      }
    }

    // if the distance is too big, do not merged
    if (best_line != -1 && best_dist < est_alef_hgt_) {
      // add the pix to the best line
      Pix *new_line = PixUnion(lines->pix[best_line],
        lines->boxa->box[best_line],
        small_con_comps_pix->pix[con], small_con_comp_box);

      if (new_line == NULL) {
        return false;
      }

      pixDestroy(&lines->pix[best_line]);
      lines->pix[best_line] = new_line;
    }
  }

  pixaDestroy(&small_con_comps_pix);
  boxaDestroy(&small_line_con_comps);

  return true;
}

// Creates new set of lines from the computed columns
bool CubeLineSegmenter::AddLines(Pixa *lines) {
  // create an array that will hold the bounding boxes
  // of the concomps belonging to each line
  Boxaa *lines_con_comps = boxaaCreate(lines->n);
  if (lines_con_comps == NULL) {
    return false;
  }

  for (int line = 0; line < lines->n; line++) {
    // if the line is not valid
    if (ValidLine(lines->pix[line], lines->boxa->box[line]) == false) {
      // split it
      Pixa *split_lines = SplitLine(lines->pix[line],
          lines->boxa->box[line]);

      // remove the old line
      if (pixaRemovePix(lines, line) != 0) {
        return false;
      }

      line--;

      if (split_lines == NULL) {
        continue;
      }

      // add the split lines instead and move the pointer
      for (int s_line = 0; s_line < split_lines->n; s_line++) {
        Pix *sp_line = pixaGetPix(split_lines, s_line, L_CLONE);
        Box *sp_box = boxaGetBox(split_lines->boxa, s_line, L_CLONE);

        if (sp_line == NULL || sp_box == NULL) {
          return false;
        }

        // insert the new line
        if (pixaInsertPix(lines, ++line, sp_line, sp_box) != 0) {
          return false;
        }
      }

      // remove the split lines
      pixaDestroy(&split_lines);
    }
  }

  // compute the concomps bboxes of each line
  for (int line = 0; line < lines->n; line++) {
    Boxa *line_con_comps = ComputeLineConComps(lines->pix[line],
        lines->boxa->box[line], NULL);

    if (line_con_comps == NULL) {
      return false;
    }

    // insert it into the boxaa array
    if (boxaaAddBoxa(lines_con_comps, line_con_comps, L_INSERT) != 0) {
      return false;
    }
  }

  // post process the lines:
  // merge the contents of "small" lines info legitimate lines
  for (int line = 0; line < lines->n; line++) {
    // a small line detected
    if (SmallLine(lines->boxa->box[line]) == true) {
      // merge its components to one of the valid lines
      if (MergeLine(lines->pix[line], lines->boxa->box[line],
          lines, lines_con_comps) == true) {
        // remove the small line
        if (pixaRemovePix(lines, line) != 0) {
          return false;
        }

        if (boxaaRemoveBoxa(lines_con_comps, line) != 0) {
          return false;
        }

        line--;
      }
    }
  }

  boxaaDestroy(&lines_con_comps);

  // add the pix masks
  if (pixaaAddPixa(columns_, lines, L_INSERT) != 0) {
    return false;
  }

  return true;
}

// Index the specific pixa using RTL reading order
int *CubeLineSegmenter::IndexRTL(Pixa *pixa) {
  int *pix_index = new int[pixa->n];
  if (pix_index == NULL) {
    return NULL;
  }

  for (int pix = 0; pix < pixa->n; pix++) {
    pix_index[pix] = pix;
  }

  for (int ipix = 0; ipix < pixa->n; ipix++) {
    for (int jpix = ipix + 1; jpix < pixa->n; jpix++) {
      Box *ipix_box = pixa->boxa->box[pix_index[ipix]],
      *jpix_box = pixa->boxa->box[pix_index[jpix]];

      // swap?
      if ((ipix_box->x + ipix_box->w) < (jpix_box->x + jpix_box->w)) {
        int temp = pix_index[ipix];
        pix_index[ipix] = pix_index[jpix];
        pix_index[jpix] = temp;
      }
    }
  }

  return pix_index;
}

// Performs line segmentation
bool CubeLineSegmenter::LineSegment() {
  // Use full image morphology to find columns
  // This only works for simple layouts where each column
  // of text extends the full height of the input image.
  Pix *pix_temp1 = pixMorphCompSequence(img_, "c5.500", 0);
  if (pix_temp1 == NULL) {
    return false;
  }

  // Mask with a single component over each column
  Pixa *pixam;
  Boxa *boxa = pixConnComp(pix_temp1, &pixam, 8);

  if (boxa == NULL) {
    return false;
  }

  int init_morph_min_hgt = kLineSepMorphMinHgt;
  char sequence_str[16];
  sprintf(sequence_str, "c100.%d", init_morph_min_hgt);

  // Use selective region-based morphology to get the textline mask.
  Pixa *pixad = pixaMorphSequenceByRegion(img_, pixam, sequence_str, 0, 0);
  if (pixad == NULL) {
    return false;
  }

  // for all columns
  int col_cnt = boxaGetCount(boxa);

  // create columns
  columns_ = pixaaCreate(col_cnt);
  if (columns_ == NULL) {
    return false;
  }

  // index columns based on readind order (RTL)
  int *col_order = IndexRTL(pixad);
  if (col_order == NULL) {
    return false;
  }

  line_cnt_ = 0;

  for (int col_idx = 0; col_idx < col_cnt; col_idx++) {
    int col = col_order[col_idx];

    // get the pix and box corresponding to the column
    Pix *pixt3 = pixaGetPix(pixad, col, L_CLONE);
    if (pixt3 == NULL) {
      return false;
    }

    Box *col_box = pixad->boxa->box[col];

    Pixa *pixac;
    Boxa *boxa2 = pixConnComp(pixt3, &pixac, 8);
    if (boxa2 == NULL) {
      return false;
    }

    // offset the boxes by the column box
    for (int line = 0; line < pixac->n; line++) {
      pixac->boxa->box[line]->x += col_box->x;
      pixac->boxa->box[line]->y += col_box->y;
    }

    // add the lines
    if (AddLines(pixac) == true) {
      if (pixaaAddBox(columns_, col_box, L_CLONE) != 0) {
        return false;
      }
    }

    pixDestroy(&pixt3);
    boxaDestroy(&boxa2);

    line_cnt_ += columns_->pixa[col_idx]->n;
  }

  pixaDestroy(&pixam);
  pixaDestroy(&pixad);
  boxaDestroy(&boxa);

  delete []col_order;
  pixDestroy(&pix_temp1);

  return true;
}

// Estimate the paramters of the font(s) used in the page
bool CubeLineSegmenter::EstimateFontParams() {
  int hgt_hist[kHgtBins];
  int max_hgt;
  double mean_hgt;

  // init hgt histogram of concomps
  memset(hgt_hist, 0, sizeof(hgt_hist));

  // compute max hgt
  max_hgt = 0;

  for (int con = 0; con < con_comps_->n; con++) {
    // skip conn comps that are too long or too wide
    if (con_comps_->boxa->box[con]->h > kMaxConnCompHgt ||
        con_comps_->boxa->box[con]->w > kMaxConnCompWid) {
      continue;
    }

    max_hgt = MAX(max_hgt, con_comps_->boxa->box[con]->h);
  }

  if (max_hgt <= 0) {
    return false;
  }

  // init hgt histogram of concomps
  memset(hgt_hist, 0, sizeof(hgt_hist));

  // compute histogram
  mean_hgt = 0.0;
  for (int con = 0; con < con_comps_->n; con++) {
    // skip conn comps that are too long or too wide
    if (con_comps_->boxa->box[con]->h > kMaxConnCompHgt ||
        con_comps_->boxa->box[con]->w > kMaxConnCompWid) {
      continue;
    }

    int bin = static_cast<int>(kHgtBins * con_comps_->boxa->box[con]->h /
                               max_hgt);
    bin = MIN(bin, kHgtBins - 1);
    hgt_hist[bin]++;
    mean_hgt += con_comps_->boxa->box[con]->h;
  }

  mean_hgt /= con_comps_->n;

  // find the top 2 bins
  int idx[kHgtBins];

  for (int bin = 0; bin < kHgtBins; bin++) {
    idx[bin] = bin;
  }

  for (int ibin = 0; ibin < 2; ibin++) {
    for (int jbin = ibin + 1; jbin < kHgtBins; jbin++) {
      if (hgt_hist[idx[ibin]] < hgt_hist[idx[jbin]]) {
        int swap = idx[ibin];
        idx[ibin] = idx[jbin];
        idx[jbin] = swap;
      }
    }
  }

  // emperically, we found out that the 2 highest freq bins correspond
  // respectively to the dot and alef
  est_dot_hgt_ = (1.0 * (idx[0] + 1) * max_hgt / kHgtBins);
  est_alef_hgt_ = (1.0 * (idx[1] + 1) * max_hgt / kHgtBins);

  // as a sanity check the dot hgt must be significanly lower than alef
  if (est_alef_hgt_ < (est_dot_hgt_ * 2)) {
    // use max_hgt to estimate instead
    est_alef_hgt_ = mean_hgt * 1.5;
    est_dot_hgt_ = est_alef_hgt_ / 5.0;
  }

  est_alef_hgt_ = MAX(est_alef_hgt_, est_dot_hgt_ * 4.0);

  return true;
}

// clean up the image
Pix *CubeLineSegmenter::CleanUp(Pix *orig_img) {
  // get rid of long horizontal lines
  Pix *pix_temp0 = pixMorphCompSequence(orig_img, "o300.2", 0);
  pixXor(pix_temp0, pix_temp0, orig_img);

  // get rid of long vertical lines
  Pix *pix_temp1 = pixMorphCompSequence(pix_temp0, "o2.300", 0);
  pixXor(pix_temp1, pix_temp1, pix_temp0);

  pixDestroy(&pix_temp0);

  // detect connected components
  Pixa *con_comps;
  Boxa *boxa = pixConnComp(pix_temp1, &con_comps, 8);
  if (boxa == NULL) {
    return NULL;
  }

  // detect and remove suspicious conn comps
  for (int con = 0; con < con_comps->n; con++) {
    Box *box = boxa->box[con];

    // remove if suspc. conn comp
    if ((box->w > (box->h * kMaxHorzAspectRatio)) ||
         (box->h > (box->w * kMaxVertAspectRatio)) ||
         (box->w < kMinWid && box->h < kMinHgt)) {
      pixRasterop(pix_temp1, box->x, box->y, box->w, box->h,
        PIX_SRC ^ PIX_DST, con_comps->pix[con], 0, 0);
    }
  }

  pixaDestroy(&con_comps);
  boxaDestroy(&boxa);

  return pix_temp1;
}

// Init the page segmenter
bool CubeLineSegmenter::Init() {
  if (init_ == true) {
    return true;
  }

  if (orig_img_ == NULL) {
    return false;
  }

  // call the internal line segmentation
  return FindLines();
}

// return the pix mask and box of a specific line
Pix *CubeLineSegmenter::Line(int line, Box **line_box) {
  if (init_ == false && Init() == false) {
    return NULL;
  }

  if (line < 0 || line >= line_cnt_) {
    return NULL;
  }

  (*line_box) = lines_pixa_->boxa->box[line];
  return lines_pixa_->pix[line];
}

// Implements a basic rudimentary layout analysis based on Leptonica
// works OK for Arabic. For other languages, the function TesseractPageAnalysis
// should be called instead.
bool CubeLineSegmenter::FindLines() {
  // convert the image to gray scale if necessary
  Pix *gray_scale_img = NULL;
  if (orig_img_->d != 2 && orig_img_->d != 8) {
    gray_scale_img = pixConvertTo8(orig_img_, false);
    if (gray_scale_img == NULL) {
      return false;
    }
  } else {
    gray_scale_img = orig_img_;
  }

  // threshold image
  Pix *thresholded_img;
  thresholded_img = pixThresholdToBinary(gray_scale_img, 128);
  // free the gray scale image if necessary
  if (gray_scale_img != orig_img_) {
    pixDestroy(&gray_scale_img);
  }
  // bail-out if thresholding failed
  if (thresholded_img == NULL)  {
    return false;
  }

  // deskew
  Pix *deskew_img = pixDeskew(thresholded_img, 2);
  if (deskew_img == NULL) {
    return false;
  }

  pixDestroy(&thresholded_img);

  img_ = CleanUp(deskew_img);
  pixDestroy(&deskew_img);
  if (img_ == NULL) {
    return false;
  }

  pixDestroy(&deskew_img);

  // compute connected components
  Boxa *boxa = pixConnComp(img_, &con_comps_, 8);
  if (boxa == NULL) {
    return false;
  }

  boxaDestroy(&boxa);

  // estimate dot and alef hgts
  if (EstimateFontParams() == false) {
    return false;
  }

  // perform line segmentation
  if (LineSegment() == false) {
    return false;
  }

  // success
  init_ = true;
  return true;
}

}
