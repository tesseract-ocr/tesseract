///////////////////////////////////////////////////////////////////////
// File:        baselinedetect.cpp
// Description: Initial Baseline Determination.
// Copyright 2012 Google Inc. All Rights Reserved.
// Author:      rays@google.com (Ray Smith)
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

#define _USE_MATH_DEFINES // for M_PI

#ifdef HAVE_CONFIG_H
#  include "config_auto.h"
#endif

#include "baselinedetect.h"

#include <allheaders.h>
#include <algorithm>
#include <cfloat> // for FLT_MAX
#include <cmath>  // for M_PI
#include "blobbox.h"
#include "detlinefit.h"
#include "drawtord.h"
#include "helpers.h"
#include "linlsq.h"
#include "makerow.h"
#include "textord.h"
#include "tprintf.h"
#include "underlin.h"

// Number of displacement modes kept in displacement_modes_;
const int kMaxDisplacementsModes = 3;
// Number of points to skip when retrying initial fit.
const int kNumSkipPoints = 3;
// Max angle deviation (in radians) allowed to keep the independent baseline.
const double kMaxSkewDeviation = 1.0 / 64;
// Fraction of line spacing estimate for quantization of blob displacements.
const double kOffsetQuantizationFactor = 3.0 / 64;
// Fraction of line spacing estimate for computing blob fit error.
const double kFitHalfrangeFactor = 6.0 / 64;
// Max fraction of line spacing allowed before a baseline counts as badly
// fitting.
const double kMaxBaselineError = 3.0 / 64;
// Multiple of linespacing that sets max_blob_size in TO_BLOCK.
// Copied from textord_excess_blobsize.
const double kMaxBlobSizeMultiple = 1.3;
// Min fraction of linespacing gaps that should be close to the model before
// we will force the linespacing model on all the lines.
const double kMinFittingLinespacings = 0.25;
// A y-coordinate within a textline that is to be debugged.
//#define kDebugYCoord 1525

namespace tesseract {

BaselineRow::BaselineRow(double line_spacing, TO_ROW *to_row)
    : blobs_(to_row->blob_list()),
      baseline_pt1_(0.0f, 0.0f),
      baseline_pt2_(0.0f, 0.0f),
      baseline_error_(0.0),
      good_baseline_(false) {
  ComputeBoundingBox();
  // Compute a scale factor for rounding to ints.
  disp_quant_factor_ = kOffsetQuantizationFactor * line_spacing;
  fit_halfrange_ = kFitHalfrangeFactor * line_spacing;
  max_baseline_error_ = kMaxBaselineError * line_spacing;
}

// Sets the TO_ROW with the output straight line.
void BaselineRow::SetupOldLineParameters(TO_ROW *row) const {
  // TODO(rays) get rid of this when m and c are no longer used.
  double gradient = tan(BaselineAngle());
  // para_c is the actual intercept of the baseline on the y-axis.
  float para_c = StraightYAtX(0.0);
  row->set_line(gradient, para_c, baseline_error_);
  row->set_parallel_line(gradient, para_c, baseline_error_);
}

// Outputs diagnostic information.
void BaselineRow::Print() const {
  tprintf("Baseline (%g,%g)->(%g,%g), angle=%g, intercept=%g\n",
          baseline_pt1_.x(), baseline_pt1_.y(), baseline_pt2_.x(),
          baseline_pt2_.y(), BaselineAngle(), StraightYAtX(0.0));
  tprintf("Quant factor=%g, error=%g, good=%d, box:", disp_quant_factor_,
          baseline_error_, good_baseline_);
  bounding_box_.print();
}

// Returns the skew angle (in radians) of the current baseline in [-pi,pi].
double BaselineRow::BaselineAngle() const {
  FCOORD baseline_dir(baseline_pt2_ - baseline_pt1_);
  double angle = baseline_dir.angle();
  // Baseline directions are only unique in a range of pi so constrain to
  // [-pi/2, pi/2].
  return fmod(angle + M_PI * 1.5, M_PI) - M_PI * 0.5;
}

// Computes and returns the linespacing at the middle of the overlap
// between this and other.
double BaselineRow::SpaceBetween(const BaselineRow &other) const {
  // Find the x-centre of overlap of the lines.
  float x = (std::max(bounding_box_.left(), other.bounding_box_.left()) +
             std::min(bounding_box_.right(), other.bounding_box_.right())) /
            2.0f;
  // Find the vertical centre between them.
  float y = (StraightYAtX(x) + other.StraightYAtX(x)) / 2.0f;
  // Find the perpendicular distance of (x,y) from each line.
  FCOORD pt(x, y);
  return PerpDistanceFromBaseline(pt) + other.PerpDistanceFromBaseline(pt);
}

// Computes and returns the displacement of the center of the line
// perpendicular to the given direction.
double BaselineRow::PerpDisp(const FCOORD &direction) const {
  float middle_x = (bounding_box_.left() + bounding_box_.right()) / 2.0f;
  FCOORD middle_pos(middle_x, StraightYAtX(middle_x));
  return direction * middle_pos / direction.length();
}

// Computes the y coordinate at the given x using the straight baseline
// defined by baseline_pt1_ and baseline_pt2__.
double BaselineRow::StraightYAtX(double x) const {
  double denominator = baseline_pt2_.x() - baseline_pt1_.x();
  if (denominator == 0.0) {
    return (baseline_pt1_.y() + baseline_pt2_.y()) / 2.0;
  }
  return baseline_pt1_.y() + (x - baseline_pt1_.x()) *
                                 (baseline_pt2_.y() - baseline_pt1_.y()) /
                                 denominator;
}

// Fits a straight baseline to the points. Returns true if it had enough
// points to be reasonably sure of the fitted baseline.
// If use_box_bottoms is false, baselines positions are formed by
// considering the outlines of the blobs.
bool BaselineRow::FitBaseline(bool use_box_bottoms) {
  // Deterministic fitting is used wherever possible.
  fitter_.Clear();
  // Linear least squares is a backup if the DetLineFit produces a bad line.
  LLSQ llsq;
  BLOBNBOX_IT blob_it(blobs_);

  for (blob_it.mark_cycle_pt(); !blob_it.cycled_list(); blob_it.forward()) {
    BLOBNBOX *blob = blob_it.data();
    if (!use_box_bottoms) {
      blob->EstimateBaselinePosition();
    }
    const TBOX &box = blob->bounding_box();
    int x_middle = (box.left() + box.right()) / 2;
#ifdef kDebugYCoord
    if (box.bottom() < kDebugYCoord && box.top() > kDebugYCoord) {
      tprintf("Box bottom = %d, baseline pos=%d for box at:", box.bottom(),
              blob->baseline_position());
      box.print();
    }
#endif
    fitter_.Add(ICOORD(x_middle, blob->baseline_position()), box.width() / 2);
    llsq.add(x_middle, blob->baseline_position());
  }
  // Fit the line.
  ICOORD pt1, pt2;
  baseline_error_ = fitter_.Fit(&pt1, &pt2);
  baseline_pt1_ = pt1;
  baseline_pt2_ = pt2;
  if (baseline_error_ > max_baseline_error_ &&
      fitter_.SufficientPointsForIndependentFit()) {
    // The fit was bad but there were plenty of points, so try skipping
    // the first and last few, and use the new line if it dramatically improves
    // the error of fit.
    double error = fitter_.Fit(kNumSkipPoints, kNumSkipPoints, &pt1, &pt2);
    if (error < baseline_error_ / 2.0) {
      baseline_error_ = error;
      baseline_pt1_ = pt1;
      baseline_pt2_ = pt2;
    }
  }
  int debug = 0;
#ifdef kDebugYCoord
  Print();
  debug = bounding_box_.bottom() < kDebugYCoord &&
                  bounding_box_.top() > kDebugYCoord
              ? 3
              : 2;
#endif
  // Now we obtained a direction from that fit, see if we can improve the
  // fit using the same direction and some other start point.
  FCOORD direction(pt2 - pt1);
  double target_offset = direction * pt1;
  good_baseline_ = false;
  FitConstrainedIfBetter(debug, direction, 0.0, target_offset);
  // Wild lines can be produced because DetLineFit allows vertical lines, but
  // vertical text has been rotated so angles over pi/4 should be disallowed.
  // Near vertical lines can still be produced by vertically aligned components
  // on very short lines.
  double angle = BaselineAngle();
  if (fabs(angle) > M_PI * 0.25) {
    // Use the llsq fit as a backup.
    baseline_pt1_ = llsq.mean_point();
    baseline_pt2_ = baseline_pt1_ + FCOORD(1.0f, llsq.m());
    // TODO(rays) get rid of this when m and c are no longer used.
    double m = llsq.m();
    double c = llsq.c(m);
    baseline_error_ = llsq.rms(m, c);
    good_baseline_ = false;
  }
  return good_baseline_;
}

// Modifies an existing result of FitBaseline to be parallel to the given
// direction vector if that produces a better result.
void BaselineRow::AdjustBaselineToParallel(int debug, const FCOORD &direction) {
  SetupBlobDisplacements(direction);
  if (displacement_modes_.empty()) {
    return;
  }
#ifdef kDebugYCoord
  if (bounding_box_.bottom() < kDebugYCoord &&
      bounding_box_.top() > kDebugYCoord && debug < 3)
    debug = 3;
#endif
  FitConstrainedIfBetter(debug, direction, 0.0, displacement_modes_[0]);
}

// Modifies the baseline to snap to the textline grid if the existing
// result is not good enough.
double BaselineRow::AdjustBaselineToGrid(int debug, const FCOORD &direction,
                                         double line_spacing,
                                         double line_offset) {
  if (blobs_->empty()) {
    if (debug > 1) {
      tprintf("Row empty at:");
      bounding_box_.print();
    }
    return line_offset;
  }
  // Find the displacement_modes_ entry nearest to the grid.
  double best_error = 0.0;
  int best_index = -1;
  for (unsigned i = 0; i < displacement_modes_.size(); ++i) {
    double blob_y = displacement_modes_[i];
    double error =
        BaselineBlock::SpacingModelError(blob_y, line_spacing, line_offset);
    if (debug > 1) {
      tprintf("Mode at %g has error %g from model \n", blob_y, error);
    }
    if (best_index < 0 || error < best_error) {
      best_error = error;
      best_index = i;
    }
  }
  // We will move the baseline only if the chosen mode is close enough to the
  // model.
  double model_margin = max_baseline_error_ - best_error;
  if (best_index >= 0 && model_margin > 0.0) {
    // But if the current baseline is already close to the mode there is no
    // point, and only the potential to damage accuracy by changing its angle.
    double perp_disp = PerpDisp(direction);
    double shift = displacement_modes_[best_index] - perp_disp;
    if (fabs(shift) > max_baseline_error_) {
      if (debug > 1) {
        tprintf("Attempting linespacing model fit with mode %g to row at:",
                displacement_modes_[best_index]);
        bounding_box_.print();
      }
      FitConstrainedIfBetter(debug, direction, model_margin,
                             displacement_modes_[best_index]);
    } else if (debug > 1) {
      tprintf("Linespacing model only moves current line by %g for row at:",
              shift);
      bounding_box_.print();
    }
  } else if (debug > 1) {
    tprintf("Linespacing model not close enough to any mode for row at:");
    bounding_box_.print();
  }
  return fmod(PerpDisp(direction), line_spacing);
}

// Sets up displacement_modes_ with the top few modes of the perpendicular
// distance of each blob from the given direction vector, after rounding.
void BaselineRow::SetupBlobDisplacements(const FCOORD &direction) {
  // Set of perpendicular displacements of the blob bottoms from the required
  // baseline direction.
  std::vector<double> perp_blob_dists;
  displacement_modes_.clear();
  // Gather the skew-corrected position of every blob.
  double min_dist = FLT_MAX;
  double max_dist = -FLT_MAX;
  BLOBNBOX_IT blob_it(blobs_);
#ifdef kDebugYCoord
  bool debug = false;
#endif
  for (blob_it.mark_cycle_pt(); !blob_it.cycled_list(); blob_it.forward()) {
    BLOBNBOX *blob = blob_it.data();
    const TBOX &box = blob->bounding_box();
#ifdef kDebugYCoord
    if (box.bottom() < kDebugYCoord && box.top() > kDebugYCoord)
      debug = true;
#endif
    FCOORD blob_pos((box.left() + box.right()) / 2.0f,
                    blob->baseline_position());
    double offset = direction * blob_pos;
    perp_blob_dists.push_back(offset);
#ifdef kDebugYCoord
    if (debug) {
      tprintf("Displacement %g for blob at:", offset);
      box.print();
    }
#endif
    UpdateRange(offset, &min_dist, &max_dist);
  }
  // Set up a histogram using disp_quant_factor_ as the bucket size.
  STATS dist_stats(IntCastRounded(min_dist / disp_quant_factor_),
                   IntCastRounded(max_dist / disp_quant_factor_));
  for (double perp_blob_dist : perp_blob_dists) {
    dist_stats.add(IntCastRounded(perp_blob_dist / disp_quant_factor_), 1);
  }
  std::vector<KDPairInc<float, int>> scaled_modes;
  dist_stats.top_n_modes(kMaxDisplacementsModes, scaled_modes);
#ifdef kDebugYCoord
  if (debug) {
    for (int i = 0; i < scaled_modes.size(); ++i) {
      tprintf("Top mode = %g * %d\n", scaled_modes[i].key * disp_quant_factor_,
              scaled_modes[i].data());
    }
  }
#endif
  for (auto &scaled_mode : scaled_modes) {
    displacement_modes_.push_back(disp_quant_factor_ * scaled_mode.key());
  }
}

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
void BaselineRow::FitConstrainedIfBetter(int debug, const FCOORD &direction,
                                         double cheat_allowance,
                                         double target_offset) {
  double halfrange = fit_halfrange_ * direction.length();
  double min_dist = target_offset - halfrange;
  double max_dist = target_offset + halfrange;
  ICOORD line_pt;
  double new_error = fitter_.ConstrainedFit(direction, min_dist, max_dist,
                                            debug > 2, &line_pt);
  // Allow cheat_allowance off the new error
  new_error -= cheat_allowance;
  double old_angle = BaselineAngle();
  double new_angle = direction.angle();
  if (debug > 1) {
    tprintf("Constrained error = %g, original = %g", new_error,
            baseline_error_);
    tprintf(" angles = %g, %g, delta=%g vs threshold %g\n", old_angle,
            new_angle, new_angle - old_angle, kMaxSkewDeviation);
  }
  bool new_good_baseline =
      new_error <= max_baseline_error_ &&
      (cheat_allowance > 0.0 || fitter_.SufficientPointsForIndependentFit());
  // The new will replace the old if any are true:
  // 1. the new error is better
  // 2. the old is NOT good, but the new is
  // 3. there is a wild angular difference between them (assuming that the new
  //    is a better guess at the angle.)
  if (new_error <= baseline_error_ || (!good_baseline_ && new_good_baseline) ||
      fabs(new_angle - old_angle) > kMaxSkewDeviation) {
    baseline_error_ = new_error;
    baseline_pt1_ = line_pt;
    baseline_pt2_ = baseline_pt1_ + direction;
    good_baseline_ = new_good_baseline;
    if (debug > 1) {
      tprintf("Replacing with constrained baseline, good = %d\n",
              good_baseline_);
    }
  } else if (debug > 1) {
    tprintf("Keeping old baseline\n");
  }
}

// Returns the perpendicular distance of the point from the straight
// baseline.
float BaselineRow::PerpDistanceFromBaseline(const FCOORD &pt) const {
  FCOORD baseline_vector(baseline_pt2_ - baseline_pt1_);
  FCOORD offset_vector(pt - baseline_pt1_);
  float distance = baseline_vector * offset_vector;
  float sqlength = baseline_vector.sqlength();
  if (sqlength == 0.0f) {
    tprintf("unexpected baseline vector (0,0)\n");
    return 0.0f;
  }
  return std::sqrt(distance * distance / sqlength);
}

// Computes the bounding box of the row.
void BaselineRow::ComputeBoundingBox() {
  BLOBNBOX_IT it(blobs_);
  TBOX box;
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    box += it.data()->bounding_box();
  }
  bounding_box_ = box;
}

BaselineBlock::BaselineBlock(int debug_level, bool non_text, TO_BLOCK *block)
    : block_(block),
      debug_level_(debug_level),
      non_text_block_(non_text),
      good_skew_angle_(false),
      skew_angle_(0.0),
      line_spacing_(block->line_spacing),
      line_offset_(0.0),
      model_error_(0.0) {
  TO_ROW_IT row_it(block_->get_rows());
  for (row_it.mark_cycle_pt(); !row_it.cycled_list(); row_it.forward()) {
    // Sort the blobs on the rows.
    row_it.data()->blob_list()->sort(blob_x_order);
    rows_.push_back(new BaselineRow(block->line_spacing, row_it.data()));
  }
}

// Computes and returns the absolute error of the given perp_disp from the
// given linespacing model.
double BaselineBlock::SpacingModelError(double perp_disp, double line_spacing,
                                        double line_offset) {
  // Round to the nearest multiple of line_spacing + line offset.
  int multiple = IntCastRounded((perp_disp - line_offset) / line_spacing);
  double model_y = line_spacing * multiple + line_offset;
  return fabs(perp_disp - model_y);
}

// Fits straight line baselines and computes the skew angle from the
// median angle. Returns true if a good angle is found.
// If use_box_bottoms is false, baseline positions are formed by
// considering the outlines of the blobs.
bool BaselineBlock::FitBaselinesAndFindSkew(bool use_box_bottoms) {
  if (non_text_block_) {
    return false;
  }
  std::vector<double> angles;
  for (auto row : rows_) {
    if (row->FitBaseline(use_box_bottoms)) {
      double angle = row->BaselineAngle();
      angles.push_back(angle);
    }
    if (debug_level_ > 1) {
      row->Print();
    }
  }

  if (!angles.empty()) {
    skew_angle_ = MedianOfCircularValues(M_PI, angles);
    good_skew_angle_ = true;
  } else {
    skew_angle_ = 0.0f;
    good_skew_angle_ = false;
  }
  if (debug_level_ > 0) {
    tprintf("Initial block skew angle = %g, good = %d\n", skew_angle_,
            good_skew_angle_);
  }
  return good_skew_angle_;
}

// Refits the baseline to a constrained angle, using the stored block
// skew if good enough, otherwise the supplied default skew.
void BaselineBlock::ParallelizeBaselines(double default_block_skew) {
  if (non_text_block_) {
    return;
  }
  if (!good_skew_angle_) {
    skew_angle_ = default_block_skew;
  }
  if (debug_level_ > 0) {
    tprintf("Adjusting block to skew angle %g\n", skew_angle_);
  }
  FCOORD direction(cos(skew_angle_), sin(skew_angle_));
  for (auto row : rows_) {
    row->AdjustBaselineToParallel(debug_level_, direction);
    if (debug_level_ > 1) {
      row->Print();
    }
  }
  if (rows_.size() < 3 || !ComputeLineSpacing()) {
    return;
  }
  // Enforce the line spacing model on all lines that don't yet have a good
  // baseline.
  // Start by finding the row that is best fitted to the model.
  unsigned best_row = 0;
  double best_error = SpacingModelError(rows_[0]->PerpDisp(direction),
                                        line_spacing_, line_offset_);
  for (unsigned r = 1; r < rows_.size(); ++r) {
    double error = SpacingModelError(rows_[r]->PerpDisp(direction),
                                     line_spacing_, line_offset_);
    if (error < best_error) {
      best_error = error;
      best_row = r;
    }
  }
  // Starting at the best fitting row, work outwards, syncing the offset.
  double offset = line_offset_;
  for (auto r = best_row + 1; r < rows_.size(); ++r) {
    offset = rows_[r]->AdjustBaselineToGrid(debug_level_, direction,
                                            line_spacing_, offset);
  }
  offset = line_offset_;
  for (int r = best_row - 1; r >= 0; --r) {
    offset = rows_[r]->AdjustBaselineToGrid(debug_level_, direction,
                                            line_spacing_, offset);
  }
}

// Sets the parameters in TO_BLOCK that are needed by subsequent processes.
void BaselineBlock::SetupBlockParameters() const {
  if (line_spacing_ > 0.0) {
    // Where was block_line_spacing set before?
    float min_spacing =
        std::min(block_->line_spacing, static_cast<float>(line_spacing_));
    if (min_spacing < block_->line_size) {
      block_->line_size = min_spacing;
    }
    block_->line_spacing = line_spacing_;
    block_->baseline_offset = line_offset_;
    block_->max_blob_size = line_spacing_ * kMaxBlobSizeMultiple;
  }
  // Setup the parameters on all the rows.
  TO_ROW_IT row_it(block_->get_rows());
  for (unsigned r = 0; r < rows_.size(); ++r, row_it.forward()) {
    BaselineRow *row = rows_[r];
    TO_ROW *to_row = row_it.data();
    row->SetupOldLineParameters(to_row);
  }
}

// Processing that is required before fitting baseline splines, but requires
// linear baselines in order to be successful:
//   Removes noise if required
//   Separates out underlines
//   Pre-associates blob fragments.
// TODO(rays/joeliu) This entire section of code is inherited from the past
// and could be improved/eliminated.
// page_tr is used to size a debug window.
void BaselineBlock::PrepareForSplineFitting(ICOORD page_tr, bool remove_noise) {
  if (non_text_block_) {
    return;
  }
  if (remove_noise) {
    vigorous_noise_removal(block_);
  }
  FCOORD rotation(1.0f, 0.0f);
  double gradient = tan(skew_angle_);
  separate_underlines(block_, gradient, rotation, true);
  pre_associate_blobs(page_tr, block_, rotation, true);
}

// Fits splines to the textlines, or creates fake QSPLINES from the straight
// baselines that are already on the TO_ROWs.
// As a side-effect, computes the xheights of the rows and the block.
// Although x-height estimation is conceptually separate, it is part of
// detecting perspective distortion and therefore baseline fitting.
void BaselineBlock::FitBaselineSplines(bool enable_splines,
                                       bool show_final_rows, Textord *textord) {
  double gradient = tan(skew_angle_);
  FCOORD rotation(1.0f, 0.0f);

  if (enable_splines) {
    textord->make_spline_rows(block_, gradient, show_final_rows);
  } else {
    // Make a fake spline from the existing line.
    TBOX block_box = block_->block->pdblk.bounding_box();
    TO_ROW_IT row_it = block_->get_rows();
    for (row_it.mark_cycle_pt(); !row_it.cycled_list(); row_it.forward()) {
      TO_ROW *row = row_it.data();
      int32_t xstarts[2] = {block_box.left(), block_box.right()};
      double coeffs[3] = {0.0, row->line_m(), row->line_c()};
      row->baseline = QSPLINE(1, xstarts, coeffs);
      textord->compute_row_xheight(row, block_->block->classify_rotation(),
                                   row->line_m(), block_->line_size);
    }
  }
  textord->compute_block_xheight(block_, gradient);
  block_->block->set_xheight(block_->xheight);
  if (textord_restore_underlines) { // fix underlines
    restore_underlined_blobs(block_);
  }
}

#ifndef GRAPHICS_DISABLED

// Draws the (straight) baselines and final blobs colored according to
// what was discarded as noise and what is associated with each row.
void BaselineBlock::DrawFinalRows(const ICOORD &page_tr) {
  if (non_text_block_) {
    return;
  }
  double gradient = tan(skew_angle_);
  FCOORD rotation(1.0f, 0.0f);
  int left_edge = block_->block->pdblk.bounding_box().left();
  ScrollView *win = create_to_win(page_tr);
  ScrollView::Color colour = ScrollView::RED;
  TO_ROW_IT row_it = block_->get_rows();
  for (row_it.mark_cycle_pt(); !row_it.cycled_list(); row_it.forward()) {
    plot_parallel_row(row_it.data(), gradient, left_edge, colour, rotation);
    colour = static_cast<ScrollView::Color>(colour + 1);
    if (colour > ScrollView::MAGENTA) {
      colour = ScrollView::RED;
    }
  }
  plot_blob_list(win, &block_->blobs, ScrollView::MAGENTA, ScrollView::WHITE);
  // Show discarded blobs.
  plot_blob_list(win, &block_->underlines, ScrollView::YELLOW,
                 ScrollView::CORAL);
  if (block_->blobs.length() > 0) {
    tprintf("%d blobs discarded as noise\n", block_->blobs.length());
  }
  draw_meanlines(block_, gradient, left_edge, ScrollView::WHITE, rotation);
}

#endif // !GRAPHICS_DISABLED

void BaselineBlock::DrawPixSpline(Image pix_in) {
  if (non_text_block_) {
    return;
  }
  TO_ROW_IT row_it = block_->get_rows();
  for (row_it.mark_cycle_pt(); !row_it.cycled_list(); row_it.forward()) {
    row_it.data()->baseline.plot(pix_in);
  }
}

// Top-level line-spacing calculation. Computes an estimate of the line-
// spacing, using the current baselines in the TO_ROWS of the block, and
// then refines it by fitting a regression line to the baseline positions
// as a function of their integer index.
// Returns true if it seems that the model is a reasonable fit to the
// observations.
bool BaselineBlock::ComputeLineSpacing() {
  FCOORD direction(cos(skew_angle_), sin(skew_angle_));
  std::vector<double> row_positions;
  ComputeBaselinePositions(direction, &row_positions);
  if (row_positions.size() < 2) {
    return false;
  }
  EstimateLineSpacing();
  RefineLineSpacing(row_positions);
  // Verify that the model is reasonable.
  double max_baseline_error = kMaxBaselineError * line_spacing_;
  int non_trivial_gaps = 0;
  int fitting_gaps = 0;
  for (unsigned i = 1; i < row_positions.size(); ++i) {
    double row_gap = fabs(row_positions[i - 1] - row_positions[i]);
    if (row_gap > max_baseline_error) {
      ++non_trivial_gaps;
      if (fabs(row_gap - line_spacing_) <= max_baseline_error) {
        ++fitting_gaps;
      }
    }
  }
  if (debug_level_ > 0) {
    tprintf("Spacing %g, in %zu rows, %d gaps fitted out of %d non-trivial\n",
            line_spacing_, row_positions.size(), fitting_gaps,
            non_trivial_gaps);
  }
  return fitting_gaps > non_trivial_gaps * kMinFittingLinespacings;
}

// Computes the deskewed vertical position of each baseline in the block and
// stores them in the given vector.
// This is calculated as the perpendicular distance of the middle of each
// baseline (in case it has a different skew angle) from the line passing
// through the origin parallel to the block baseline angle.
// NOTE that "distance" above is a signed quantity so we can tell which side
// of the block baseline a line sits, hence the function and argument name
// positions not distances.
void BaselineBlock::ComputeBaselinePositions(const FCOORD &direction,
                                             std::vector<double> *positions) {
  positions->clear();
  for (auto row : rows_) {
    const TBOX &row_box = row->bounding_box();
    float x_middle = (row_box.left() + row_box.right()) / 2.0f;
    FCOORD row_pos(x_middle, static_cast<float>(row->StraightYAtX(x_middle)));
    float offset = direction * row_pos;
    positions->push_back(offset);
  }
}

// Computes an estimate of the line spacing of the block from the median
// of the spacings between adjacent overlapping textlines.
void BaselineBlock::EstimateLineSpacing() {
  std::vector<float> spacings;
  for (unsigned r = 0; r < rows_.size(); ++r) {
    BaselineRow *row = rows_[r];
    // Exclude silly lines.
    if (fabs(row->BaselineAngle()) > M_PI * 0.25) {
      continue;
    }
    // Find the first row after row that overlaps it significantly.
    const TBOX &row_box = row->bounding_box();
    unsigned r2;
    for (r2 = r + 1; r2 < rows_.size() &&
                     !row_box.major_x_overlap(rows_[r2]->bounding_box());
         ++r2) {
      ;
    }
    if (r2 < rows_.size()) {
      BaselineRow *row2 = rows_[r2];
      // Exclude silly lines.
      if (fabs(row2->BaselineAngle()) > M_PI * 0.25) {
        continue;
      }
      float spacing = row->SpaceBetween(*row2);
      spacings.push_back(spacing);
    }
  }
  // If we have at least one value, use it, otherwise leave the previous
  // value unchanged.
  if (!spacings.empty()) {
    std::nth_element(spacings.begin(), spacings.begin() + spacings.size() / 2,
                     spacings.end());
    line_spacing_ = spacings[spacings.size() / 2];
    if (debug_level_ > 1) {
      tprintf("Estimate of linespacing = %g\n", line_spacing_);
    }
  }
}

// Refines the line spacing of the block by fitting a regression
// line to the deskewed y-position of each baseline as a function of its
// estimated line index, allowing for a small error in the initial linespacing
// and choosing the best available model.
void BaselineBlock::RefineLineSpacing(const std::vector<double> &positions) {
  double spacings[3], offsets[3], errors[3];
  int index_range;
  errors[0] = FitLineSpacingModel(positions, line_spacing_, &spacings[0],
                                  &offsets[0], &index_range);
  if (index_range > 1) {
    double spacing_plus = line_spacing_ / (1.0 + 1.0 / index_range);
    // Try the hypotheses that there might be index_range +/- 1 line spaces.
    errors[1] = FitLineSpacingModel(positions, spacing_plus, &spacings[1],
                                    &offsets[1], nullptr);
    double spacing_minus = line_spacing_ / (1.0 - 1.0 / index_range);
    errors[2] = FitLineSpacingModel(positions, spacing_minus, &spacings[2],
                                    &offsets[2], nullptr);
    for (int i = 1; i <= 2; ++i) {
      if (errors[i] < errors[0]) {
        spacings[0] = spacings[i];
        offsets[0] = offsets[i];
        errors[0] = errors[i];
      }
    }
  }
  if (spacings[0] > 0.0) {
    line_spacing_ = spacings[0];
    line_offset_ = offsets[0];
    model_error_ = errors[0];
    if (debug_level_ > 0) {
      tprintf("Final linespacing model = %g + offset %g, error %g\n",
              line_spacing_, line_offset_, model_error_);
    }
  }
}

// Given an initial estimate of line spacing (m_in) and the positions of each
// baseline, computes the line spacing of the block more accurately in m_out,
// and the corresponding intercept in c_out, and the number of spacings seen
// in index_delta. Returns the error of fit to the line spacing model.
// Uses a simple linear regression, but optimized the offset using the median.
double BaselineBlock::FitLineSpacingModel(const std::vector<double> &positions,
                                          double m_in, double *m_out,
                                          double *c_out, int *index_delta) {
  if (m_in == 0.0f || positions.size() < 2) {
    *m_out = m_in;
    *c_out = 0.0;
    if (index_delta != nullptr) {
      *index_delta = 0;
    }
    return 0.0;
  }
  std::vector<double> offsets;
  // Get the offset (remainder) linespacing for each line and choose the median.
  offsets.reserve(positions.size());
  for (double position : positions) {
    offsets.push_back(fmod(position, m_in));
  }
  // Get the median offset.
  double median_offset = MedianOfCircularValues(m_in, offsets);
  // Now fit a line to quantized line number and offset.
  LLSQ llsq;
  int min_index = INT32_MAX;
  int max_index = -INT32_MAX;
  for (double y_pos : positions) {
    int row_index = IntCastRounded((y_pos - median_offset) / m_in);
    UpdateRange(row_index, &min_index, &max_index);
    llsq.add(row_index, y_pos);
  }
  // Get the refined line spacing.
  *m_out = llsq.m();
  // Use the median offset rather than the mean.
  offsets.clear();
  if (*m_out != 0.0) {
    for (double position : positions) {
      offsets.push_back(fmod(position, *m_out));
    }
    // Get the median offset.
    if (debug_level_ > 2) {
      for (unsigned i = 0; i < offsets.size(); ++i) {
        tprintf("%u: %g\n", i, offsets[i]);
      }
    }
    *c_out = MedianOfCircularValues(*m_out, offsets);
  } else {
    *c_out = 0.0;
  }
  if (debug_level_ > 1) {
    tprintf("Median offset = %g, compared to mean of %g.\n", *c_out,
            llsq.c(*m_out));
  }
  // Index_delta is the number of hypothesized line gaps present.
  if (index_delta != nullptr) {
    *index_delta = max_index - min_index;
  }
  // Use the regression model's intercept to compute the error, as it may be
  // a full line-spacing in disagreement with the median.
  double rms_error = llsq.rms(*m_out, llsq.c(*m_out));
  if (debug_level_ > 1) {
    tprintf("Linespacing of y=%g x + %g improved to %g x + %g, rms=%g\n", m_in,
            median_offset, *m_out, *c_out, rms_error);
  }
  return rms_error;
}

BaselineDetect::BaselineDetect(int debug_level, const FCOORD &page_skew,
                               TO_BLOCK_LIST *blocks)
    : page_skew_(page_skew), debug_level_(debug_level) {
  TO_BLOCK_IT it(blocks);
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    TO_BLOCK *to_block = it.data();
    BLOCK *block = to_block->block;
    POLY_BLOCK *pb = block->pdblk.poly_block();
    // A note about non-text blocks.
    // On output, non-text blocks are supposed to contain a single empty word
    // in each incoming text line. These mark out the polygonal bounds of the
    // block. Ideally no baselines should be required, but currently
    // make_words crashes if a baseline and xheight are not provided, so we
    // include non-text blocks here, but flag them for special treatment.
    bool non_text = pb != nullptr && !pb->IsText();
    blocks_.push_back(new BaselineBlock(debug_level_, non_text, to_block));
  }
}

// Finds the initial baselines for each TO_ROW in each TO_BLOCK, gathers
// block-wise and page-wise data to smooth small blocks/rows, and applies
// smoothing based on block/page-level skew and block-level linespacing.
void BaselineDetect::ComputeStraightBaselines(bool use_box_bottoms) {
  std::vector<double> block_skew_angles;
  for (auto bl_block : blocks_) {
    if (debug_level_ > 0) {
      tprintf("Fitting initial baselines...\n");
    }
    if (bl_block->FitBaselinesAndFindSkew(use_box_bottoms)) {
      block_skew_angles.push_back(bl_block->skew_angle());
    }
  }
  // Compute a page-wide default skew for blocks with too little information.
  double default_block_skew = page_skew_.angle();
  if (!block_skew_angles.empty()) {
    default_block_skew = MedianOfCircularValues(M_PI, block_skew_angles);
  }
  if (debug_level_ > 0) {
    tprintf("Page skew angle = %g\n", default_block_skew);
  }
  // Set bad lines in each block to the default block skew and then force fit
  // a linespacing model where it makes sense to do so.
  for (auto bl_block : blocks_) {
    bl_block->ParallelizeBaselines(default_block_skew);
    bl_block->SetupBlockParameters(); // This replaced compute_row_stats.
  }
}

// Computes the baseline splines for each TO_ROW in each TO_BLOCK and
// other associated side-effects, including pre-associating blobs, computing
// x-heights and displaying debug information.
// NOTE that ComputeStraightBaselines must have been called first as this
// sets up data in the TO_ROWs upon which this function depends.
void BaselineDetect::ComputeBaselineSplinesAndXheights(const ICOORD &page_tr,
                                                       bool enable_splines,
                                                       bool remove_noise,
                                                       bool show_final_rows,
                                                       Textord *textord) {
  for (auto bl_block : blocks_) {
    if (enable_splines) {
      bl_block->PrepareForSplineFitting(page_tr, remove_noise);
    }
    bl_block->FitBaselineSplines(enable_splines, show_final_rows, textord);
#ifndef GRAPHICS_DISABLED
    if (show_final_rows) {
      bl_block->DrawFinalRows(page_tr);
    }
#endif
  }
}

} // namespace tesseract.
