///////////////////////////////////////////////////////////////////////
// File:        baselinedetect.h
// Description: Initial Baseline Determination.
// Copyright 2012 Google Inc. All Rights Reserved.
// Author:      rays@google.com (Ray Smith)
// Created:     Mon Apr 30 10:03:19 PDT 2012
//
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

#ifndef TESSERACT_TEXTORD_BASELINEDETECT_H_
#define TESSERACT_TEXTORD_BASELINEDETECT_H_

#include "detlinefit.h"
#include "genericvector.h"
#include "points.h"
#include "rect.h"
#include "strngs.h"

class BLOBNBOX_LIST;
class TO_BLOCK;
class TO_BLOCK_LIST;
class TO_ROW;
class Pix;

namespace tesseract {

class Textord;

// Class to compute and hold baseline data for a TO_ROW.
class BaselineRow {
 public:
  BaselineRow(double line_size, TO_ROW* to_row);

  const TBOX& bounding_box() const {
    return bounding_box_;
  }
  // Sets the TO_ROW with the output straight line.
  void SetupOldLineParameters(TO_ROW* row) const;

  // Outputs diagnostic information.
  void Print() const;

  // Returns the skew angle (in radians) of the current baseline in [-pi,pi].
  double BaselineAngle() const;
  // Computes and returns the linespacing at the middle of the overlap
  // between this and other.
  double SpaceBetween(const BaselineRow& other) const;
  // Computes and returns the displacement of the center of the line
  // perpendicular to the given direction.
  double PerpDisp(const FCOORD& direction) const;
  // Computes the y coordinate at the given x using the straight baseline
  // defined by baseline1_ and baseline2_.
  double StraightYAtX(double x) const;

  // Fits a straight baseline to the points. Returns true if it had enough
  // points to be reasonably sure of the fitted baseline.
  // If use_box_bottoms is false, baselines positions are formed by
  // considering the outlines of the blobs.
  bool FitBaseline(bool use_box_bottoms);
  // Modifies an existing result of FitBaseline to be parallel to the given
  // vector if that produces a better result.
  void AdjustBaselineToParallel(int debug, const FCOORD& direction);
  // Modifies the baseline to snap to the textline grid if the existing
  // result is not good enough.
  double AdjustBaselineToGrid(int debug, const FCOORD& direction,
                              double line_spacing, double line_offset);

 private:
  // Sets up displacement_modes_ with the top few modes of the perpendicular
  // distance of each blob from the given direction vector, after rounding.
  void SetupBlobDisplacements(const FCOORD& direction);

  // Fits a line in the given direction to blobs that are close to the given
  // target_offset perpendicular displacement from the direction. The fit
  // error is allowed to be cheat_allowance worse than the existing fit, and
  // will still be used.
  // If cheat_allowance > 0, the new fit will be good and replace the current
  // fit if it has better fit (with cheat) OR its error is below
  // max_baseline_error_ and the old fit is marked bad.
  // Otherwise the new fit will only replace the old if it is really better,
  // or the old fit is marked bad and the new fit has sufficient points, as
  // well as being within the max_baseline_error_.
  void FitConstrainedIfBetter(int debug, const FCOORD& direction,
                              double cheat_allowance,
                              double target_offset);
  // Returns the perpendicular distance of the point from the straight
  // baseline.
  double PerpDistanceFromBaseline(const FCOORD& pt) const;
  // Computes the bounding box of the row.
  void ComputeBoundingBox();

  // The blobs of the row to which this BaselineRow adds extra information
  // during baseline fitting. Note that blobs_ could easily come from either
  // a TO_ROW or a ColPartition.
  BLOBNBOX_LIST* blobs_;
  // Bounding box of all the blobs.
  TBOX bounding_box_;
  // Fitter used to fit lines to the blobs.
  DetLineFit fitter_;
  // 2 points on the straight baseline.
  FCOORD baseline_pt1_;
  FCOORD baseline_pt2_;
  // Set of modes of displacements. They indicate preferable baseline positions.
  GenericVector<double> displacement_modes_;
  // Quantization factor used for displacement_modes_.
  double disp_quant_factor_;
  // Half the acceptance range of blob displacements for computing the
  // error during a constrained fit.
  double fit_halfrange_;
  // Max baseline error before a line is regarded as fitting badly.
  double max_baseline_error_;
  // The error of fit of the baseline.
  double baseline_error_;
  // True if this row seems to have a good baseline.
  bool good_baseline_;
};

// Class to compute and hold baseline data for a TO_BLOCK.
class BaselineBlock {
 public:
  BaselineBlock(int debug_level, bool non_text, TO_BLOCK* block);

  TO_BLOCK* block() const {
    return block_;
  }
  double skew_angle() const {
    return skew_angle_;
  }

  // Computes and returns the absolute error of the given perp_disp from the
  // given linespacing model.
  static double SpacingModelError(double perp_disp, double line_spacing,
                                  double line_offset);

  // Fits straight line baselines and computes the skew angle from the
  // median angle. Returns true if a good angle is found.
  // If use_box_bottoms is false, baseline positions are formed by
  // considering the outlines of the blobs.
  bool FitBaselinesAndFindSkew(bool use_box_bottoms);

  // Refits the baseline to a constrained angle, using the stored block
  // skew if good enough, otherwise the supplied default skew.
  void ParallelizeBaselines(double default_block_skew);

  // Sets the parameters in TO_BLOCK that are needed by subsequent processes.
  void SetupBlockParameters() const;

  // Processing that is required before fitting baseline splines, but requires
  // linear baselines in order to be successful:
  //   Removes noise if required
  //   Separates out underlines
  //   Pre-associates blob fragments.
  // TODO(rays/joeliu) This entire section of code is inherited from the past
  // and could be improved/eliminated.
  // page_tr is used to size a debug window.
  void PrepareForSplineFitting(ICOORD page_tr, bool remove_noise);

  // Fits splines to the textlines, or creates fake QSPLINES from the straight
  // baselines that are already on the TO_ROWs.
  // As a side-effect, computes the xheights of the rows and the block.
  // Although x-height estimation is conceptually separate, it is part of
  // detecting perspective distortion and therefore baseline fitting.
  void FitBaselineSplines(bool enable_splines, bool show_final_rows,
                          Textord* textord);

  // Draws the (straight) baselines and final blobs colored according to
  // what was discarded as noise and what is associated with each row.
  void DrawFinalRows(const ICOORD& page_tr);

  // Render the generated spline baselines for this block on pix_in.
  void DrawPixSpline(Pix* pix_in);

 private:
  // Top-level line-spacing calculation. Computes an estimate of the line-
  // spacing, using the current baselines in the TO_ROWS of the block, and
  // then refines it by fitting a regression line to the baseline positions
  // as a function of their integer index.
  // Returns true if it seems that the model is a reasonable fit to the
  // observations.
  bool ComputeLineSpacing();

  // Computes the deskewed vertical position of each baseline in the block and
  // stores them in the given vector.
  void ComputeBaselinePositions(const FCOORD& direction,
                                GenericVector<double>* positions);

  // Computes an estimate of the line spacing of the block from the median
  // of the spacings between adjacent overlapping textlines.
  void EstimateLineSpacing();

  // Refines the line spacing of the block by fitting a regression
  // line to the deskewed y-position of each baseline as a function of its
  // estimated line index, allowing for a small error in the initial linespacing
  // and choosing the best available model.
  void RefineLineSpacing(const GenericVector<double>& positions);

  // Given an initial estimate of line spacing (m_in) and the positions of each
  // baseline, computes the line spacing of the block more accurately in m_out,
  // and the corresponding intercept in c_out, and the number of spacings seen
  // in index_delta. Returns the error of fit to the line spacing model.
  double FitLineSpacingModel(const GenericVector<double>& positions,
                             double m_in, double* m_out, double* c_out,
                             int* index_delta);


  // The block to which this class adds extra information used during baseline
  // calculation.
  TO_BLOCK* block_;
  // The rows in the block that we will be working with.
  PointerVector<BaselineRow> rows_;
  // Amount of debugging output to provide.
  int debug_level_;
  // True if the block is non-text (graphic).
  bool non_text_block_;
  // True if the block has at least one good enough baseline to compute the
  // skew angle and therefore skew_angle_ is valid.
  bool good_skew_angle_;
  // Angle of skew in radians using the conventional anticlockwise from x-axis.
  double skew_angle_;
  // Current best estimate line spacing in pixels perpendicular to skew_angle_.
  double line_spacing_;
  // Offset for baseline positions, in pixels. Each baseline is at
  // line_spacing_ * n + line_offset_ for integer n, which represents
  // [textline] line number in a line numbering system that has line 0 on or
  // at least near the x-axis. Not equal to the actual line number of a line
  // within a block as most blocks are not near the x-axis.
  double line_offset_;
  // The error of the line spacing model.
  double model_error_;
};

class BaselineDetect {
 public:
  BaselineDetect(int debug_level, const FCOORD& page_skew,
                 TO_BLOCK_LIST* blocks);

  ~BaselineDetect();

  // Finds the initial baselines for each TO_ROW in each TO_BLOCK, gathers
  // block-wise and page-wise data to smooth small blocks/rows, and applies
  // smoothing based on block/page-level skew and block-level linespacing.
  void ComputeStraightBaselines(bool use_box_bottoms);

  // Computes the baseline splines for each TO_ROW in each TO_BLOCK and
  // other associated side-effects, including pre-associating blobs, computing
  // x-heights and displaying debug information.
  // NOTE that ComputeStraightBaselines must have been called first as this
  // sets up data in the TO_ROWs upon which this function depends.
  void ComputeBaselineSplinesAndXheights(const ICOORD& page_tr,
                                         bool enable_splines,
                                         bool remove_noise,
                                         bool show_final_rows,
                                         Textord* textord);

  // Set up the image and filename, so that a debug image with the detected
  // baseline rendered will be saved.
  void SetDebugImage(Pix* pixIn, const STRING& output_path);

 private:
  // Average (median) skew of the blocks on the page among those that have
  // a good angle of their own.
  FCOORD page_skew_;
  // Amount of debug output to produce.
  int debug_level_;
  // The blocks that we are working with.
  PointerVector<BaselineBlock> blocks_;

  Pix* pix_debug_;
  STRING debug_file_prefix_;
};

}  // namespace tesseract

#endif  // TESSERACT_TEXTORD_BASELINEDETECT_H_
