// Copyright 2011 Google Inc. All Rights Reserved.
// Author: rays@google.com (Ray Smith)
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

#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif

#include "textlineprojection.h"
#include "allheaders.h"
#include "bbgrid.h"         // Base class.
#include "blobbox.h"        // BlobNeighourDir.
#include "blobs.h"
#include "colpartition.h"
#include "normalis.h"

// Padding factor to use on definitely oriented blobs
const int kOrientedPadFactor = 8;
// Padding factor to use on not definitely oriented blobs.
const int kDefaultPadFactor = 2;
// Penalty factor for going away from the line center.
const int kWrongWayPenalty = 4;
// Ratio between parallel gap and perpendicular gap used to measure total
// distance of a box from a target box in curved textline space.
// parallel-gap is treated more favorably by this factor to allow catching
// quotes and elipsis at the end of textlines.
const int kParaPerpDistRatio = 4;
// Multiple of scale_factor_ that the inter-line gap must be before we start
// padding the increment box perpendicular to the text line.
const int kMinLineSpacingFactor = 4;
// Maximum tab-stop overrun for horizontal padding, in projection pixels.
const int kMaxTabStopOverrun = 6;

namespace tesseract {

TextlineProjection::TextlineProjection(int resolution)
  : x_origin_(0), y_origin_(0), pix_(NULL) {
  // The projection map should be about 100 ppi, whatever the input.
  scale_factor_ = IntCastRounded(resolution / 100.0);
  if (scale_factor_ < 1) scale_factor_ = 1;
}
TextlineProjection::~TextlineProjection() {
  pixDestroy(&pix_);
}

// Build the projection profile given the input_block containing lists of
// blobs, a rotation to convert to image coords,
// and a full-resolution nontext_map, marking out areas to avoid.
// During construction, we have the following assumptions:
// The rotation is a multiple of 90 degrees, ie no deskew yet.
// The blobs have had their left and right rules set to also limit
// the range of projection.
void TextlineProjection::ConstructProjection(TO_BLOCK* input_block,
                                             const FCOORD& rotation,
                                             Pix* nontext_map) {
  pixDestroy(&pix_);
  TBOX image_box(0, 0, pixGetWidth(nontext_map), pixGetHeight(nontext_map));
  x_origin_ = 0;
  y_origin_ = image_box.height();
  int width = (image_box.width() + scale_factor_ - 1) / scale_factor_;
  int height = (image_box.height() + scale_factor_ - 1) / scale_factor_;

  pix_ = pixCreate(width, height, 8);
  ProjectBlobs(&input_block->blobs, rotation, image_box, nontext_map);
  ProjectBlobs(&input_block->large_blobs, rotation, image_box, nontext_map);
  Pix* final_pix = pixBlockconv(pix_, 1, 1);
//  Pix* final_pix = pixBlockconv(pix_, 2, 2);
  pixDestroy(&pix_);
  pix_ = final_pix;
}

// Display the blobs in the window colored according to textline quality.
void TextlineProjection::PlotGradedBlobs(BLOBNBOX_LIST* blobs,
                                         ScrollView* win) {
#ifndef GRAPHICS_DISABLED
  BLOBNBOX_IT it(blobs);
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    BLOBNBOX* blob = it.data();
    const TBOX& box = blob->bounding_box();
    bool bad_box = BoxOutOfHTextline(box, NULL, false);
    if (blob->UniquelyVertical())
      win->Pen(ScrollView::YELLOW);
    else
      win->Pen(bad_box ? ScrollView::RED : ScrollView::BLUE);
    win->Rectangle(box.left(), box.bottom(), box.right(), box.top());
  }
  win->Update();
#endif  // GRAPHICS_DISABLED
}

// Moves blobs that look like they don't sit well on a textline from the
// input blobs list to the output small_blobs list.
// This gets them away from initial textline finding to stop diacritics
// from forming incorrect textlines. (Introduced mainly to fix Thai.)
void TextlineProjection::MoveNonTextlineBlobs(
    BLOBNBOX_LIST* blobs, BLOBNBOX_LIST* small_blobs) const {
  BLOBNBOX_IT it(blobs);
  BLOBNBOX_IT small_it(small_blobs);
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    BLOBNBOX* blob = it.data();
    const TBOX& box = blob->bounding_box();
    bool debug = AlignedBlob::WithinTestRegion(2, box.left(),
                                               box.bottom());
    if (BoxOutOfHTextline(box, NULL, debug) && !blob->UniquelyVertical()) {
      blob->ClearNeighbours();
      small_it.add_to_end(it.extract());
    }
  }
}

// Create a window and display the projection in it.
void TextlineProjection::DisplayProjection() const {
  int width = pixGetWidth(pix_);
  int height = pixGetHeight(pix_);
  Pix* pixc = pixCreate(width, height, 32);
  int src_wpl = pixGetWpl(pix_);
  int col_wpl = pixGetWpl(pixc);
  uinT32* src_data = pixGetData(pix_);
  uinT32* col_data = pixGetData(pixc);
  for (int y = 0; y < height; ++y, src_data += src_wpl, col_data += col_wpl) {
    for (int x = 0; x < width; ++x) {
      int pixel = GET_DATA_BYTE(src_data, x);
      l_uint32 result;
      if (pixel <= 17)
        composeRGBPixel(0, 0, pixel * 15, &result);
      else if (pixel <= 145)
        composeRGBPixel(0, (pixel - 17) * 2, 255, &result);
      else
        composeRGBPixel((pixel - 145) * 2, 255, 255, &result);
      col_data[x] = result;
    }
  }
#if 0
  // TODO(rays) uncomment when scrollview can display non-binary images.
  ScrollView* win = new ScrollView("Projection", 0, 0,
                                   width, height, width, height);
  win->Image(pixc, 0, 0);
  win->Update();
#else
  pixWrite("projection.png", pixc, IFF_PNG);
#endif
  pixDestroy(&pixc);
}

// Compute the distance of the box from the partition using curved projection
// space. As DistanceOfBoxFromBox, except that the direction is taken from
// the ColPartition and the median bounds of the ColPartition are used as
// the to_box.
int TextlineProjection::DistanceOfBoxFromPartition(const TBOX& box,
                                                   const ColPartition& part,
                                                   const DENORM* denorm,
                                                   bool debug) const {
  // Compute a partition box that uses the median top/bottom of the blobs
  // within and median left/right for vertical.
  TBOX part_box = part.bounding_box();
  if (part.IsHorizontalType()) {
    part_box.set_top(part.median_top());
    part_box.set_bottom(part.median_bottom());
  } else {
    part_box.set_left(part.median_left());
    part_box.set_right(part.median_right());
  }
  // Now use DistanceOfBoxFromBox to make the actual calculation.
  return DistanceOfBoxFromBox(box, part_box, part.IsHorizontalType(),
                              denorm, debug);
}

// Compute the distance from the from_box to the to_box using curved
// projection space. Separation that involves a decrease in projection
// density (moving from the from_box to the to_box) is weighted more heavily
// than constant density, and an increase is weighted less.
// If horizontal_textline is true, then curved space is used vertically,
// as for a diacritic on the edge of a textline.
// The projection uses original image coords, so denorm is used to get
// back to the image coords from box/part space.
// How the calculation works: Think of a diacritic near a textline.
// Distance is measured from the far side of the from_box to the near side of
// the to_box. Shown is the horizontal textline case.
//          |------^-----|
//          | from | box |
//          |------|-----|
//   perpendicular |
//          <------v-------->|--------------------|
//                  parallel |     to box         |
//                           |--------------------|
// Perpendicular distance uses "curved space" See VerticalDistance below.
// Parallel distance is linear.
// Result is perpendicular_gap + parallel_gap / kParaPerpDistRatio.
int TextlineProjection::DistanceOfBoxFromBox(const TBOX& from_box,
                                             const TBOX& to_box,
                                             bool horizontal_textline,
                                             const DENORM* denorm,
                                             bool debug) const {
  // The parallel_gap is the horizontal gap between a horizontal textline and
  // the box. Analogous for vertical.
  int parallel_gap = 0;
  // start_pt is the box end of the line to be modified for curved space.
  TPOINT start_pt;
  // end_pt is the partition end of the line to be modified for curved space.
  TPOINT end_pt;
  if (horizontal_textline) {
    parallel_gap = from_box.x_gap(to_box) + from_box.width();
    start_pt.x = (from_box.left() + from_box.right()) / 2;
    end_pt.x = start_pt.x;
    if (from_box.top() - to_box.top() >= to_box.bottom() - from_box.bottom()) {
      start_pt.y = from_box.top();
      end_pt.y = MIN(to_box.top(), start_pt.y);
    } else {
      start_pt.y = from_box.bottom();
      end_pt.y = MAX(to_box.bottom(), start_pt.y);
    }
  } else {
    parallel_gap = from_box.y_gap(to_box) + from_box.height();
    if (from_box.right() - to_box.right() >= to_box.left() - from_box.left()) {
      start_pt.x = from_box.right();
      end_pt.x = MIN(to_box.right(), start_pt.x);
    } else {
      start_pt.x = from_box.left();
      end_pt.x = MAX(to_box.left(), start_pt.x);
    }
    start_pt.y = (from_box.bottom() + from_box.top()) / 2;
    end_pt.y = start_pt.y;
  }
  // The perpendicular gap is the max vertical distance gap out of:
  // top of from_box to to_box top and bottom of from_box to to_box bottom.
  // This value is then modified for curved projection space.
  // Analogous for vertical.
  int perpendicular_gap = 0;
  // If start_pt == end_pt, then the from_box lies entirely within the to_box
  // (in the perpendicular direction), so we don't need to calculate the
  // perpendicular_gap.
  if (start_pt.x != end_pt.x || start_pt.y != end_pt.y) {
    if (denorm != NULL) {
      // Denormalize the start and end.
      denorm->DenormTransform(NULL, start_pt, &start_pt);
      denorm->DenormTransform(NULL, end_pt, &end_pt);
    }
    if (abs(start_pt.y - end_pt.y) >= abs(start_pt.x - end_pt.x)) {
      perpendicular_gap = VerticalDistance(debug, start_pt.x, start_pt.y,
                                           end_pt.y);
    } else {
      perpendicular_gap = HorizontalDistance(debug, start_pt.x, end_pt.x,
                                             start_pt.y);
    }
  }
  // The parallel_gap weighs less than the perpendicular_gap.
  return perpendicular_gap + parallel_gap / kParaPerpDistRatio;
}

// Compute the distance between (x, y1) and (x, y2) using the rule that
// a decrease in textline density is weighted more heavily than an increase.
// The coordinates are in source image space, ie processed by any denorm
// already, but not yet scaled by scale_factor_.
// Going from the outside of a textline to the inside should measure much
// less distance than going from the inside of a textline to the outside.
// How it works:
// An increase is cheap (getting closer to a textline).
// Constant costs unity.
// A decrease is expensive (getting further from a textline).
// Pixels in projection map Counted distance
//              2
//              3              1/x
//              3               1
//              2               x
//              5              1/x
//              7              1/x
// Total: 1 + x + 3/x where x = kWrongWayPenalty.
int TextlineProjection::VerticalDistance(bool debug, int x,
                                         int y1, int y2) const {
  x = ImageXToProjectionX(x);
  y1 = ImageYToProjectionY(y1);
  y2 = ImageYToProjectionY(y2);
  if (y1 == y2) return 0;
  int wpl = pixGetWpl(pix_);
  int step = y1 < y2 ? 1 : -1;
  uinT32* data = pixGetData(pix_) + y1 * wpl;
  wpl *= step;
  int prev_pixel = GET_DATA_BYTE(data, x);
  int distance = 0;
  int right_way_steps = 0;
  for (int y = y1; y != y2; y += step) {
    data += wpl;
    int pixel = GET_DATA_BYTE(data, x);
    if (debug)
      tprintf("At (%d,%d), pix = %d, prev=%d\n",
              x, y + step, pixel, prev_pixel);
    if (pixel < prev_pixel)
      distance += kWrongWayPenalty;
    else if (pixel > prev_pixel)
      ++right_way_steps;
    else
      ++distance;
    prev_pixel = pixel;
  }
  return distance * scale_factor_ +
      right_way_steps * scale_factor_ / kWrongWayPenalty;
}

// Compute the distance between (x1, y) and (x2, y) using the rule that
// a decrease in textline density is weighted more heavily than an increase.
int TextlineProjection::HorizontalDistance(bool debug, int x1, int x2,
                                           int y) const {
  x1 = ImageXToProjectionX(x1);
  x2 = ImageXToProjectionX(x2);
  y = ImageYToProjectionY(y);
  if (x1 == x2) return 0;
  int wpl = pixGetWpl(pix_);
  int step = x1 < x2 ? 1 : -1;
  uinT32* data = pixGetData(pix_) + y * wpl;
  int prev_pixel = GET_DATA_BYTE(data, x1);
  int distance = 0;
  int right_way_steps = 0;
  for (int x = x1; x != x2; x += step) {
    int pixel = GET_DATA_BYTE(data, x + step);
    if (debug)
      tprintf("At (%d,%d), pix = %d, prev=%d\n",
              x + step, y, pixel, prev_pixel);
    if (pixel < prev_pixel)
      distance += kWrongWayPenalty;
    else if (pixel > prev_pixel)
      ++right_way_steps;
    else
      ++distance;
    prev_pixel = pixel;
  }
  return distance * scale_factor_ +
      right_way_steps * scale_factor_ / kWrongWayPenalty;
}

// Returns true if the blob appears to be outside of a textline.
// Such blobs are potentially diacritics (even if large in Thai) and should
// be kept away from initial textline finding.
bool TextlineProjection::BoxOutOfHTextline(const TBOX& box,
                                          const DENORM* denorm,
                                          bool debug) const {
  int grad1 = 0;
  int grad2 = 0;
  EvaluateBoxInternal(box, denorm, debug, &grad1, &grad2, NULL, NULL);
  int worst_result = MIN(grad1, grad2);
  int total_result = grad1 + grad2;
  if (total_result >= 6) return false;  // Strongly in textline.
  // Medium strength: if either gradient is negative, it is likely outside
  // the body of the textline.
  if (worst_result < 0)
    return true;
  return false;
}

// Evaluates the textlineiness of a ColPartition. Uses EvaluateBox below,
// but uses the median top/bottom for horizontal and median left/right for
// vertical instead of the bounding box edges.
// Evaluates for both horizontal and vertical and returns the best result,
// with a positive value for horizontal and a negative value for vertical.
int TextlineProjection::EvaluateColPartition(const ColPartition& part,
                                             const DENORM* denorm,
                                             bool debug) const {
  if (part.IsSingleton())
    return EvaluateBox(part.bounding_box(), denorm, debug);
  // Test vertical orientation.
  TBOX box = part.bounding_box();
  // Use the partition median for left/right.
  box.set_left(part.median_left());
  box.set_right(part.median_right());
  int vresult = EvaluateBox(box, denorm, debug);

  // Test horizontal orientation.
  box = part.bounding_box();
  // Use the partition median for top/bottom.
  box.set_top(part.median_top());
  box.set_bottom(part.median_bottom());
  int hresult = EvaluateBox(box, denorm, debug);
  if (debug) {
    tprintf("Partition hresult=%d, vresult=%d from:", hresult, vresult);
    part.bounding_box().print();
    part.Print();
  }
  return hresult >= -vresult ? hresult : vresult;
}

// Computes the mean projection gradients over the horizontal and vertical
// edges of the box:
//   -h-h-h-h-h-h
//  |------------| mean=htop   -v|+v--------+v|-v
//  |+h+h+h+h+h+h|             -v|+v        +v|-v
//  |            |             -v|+v        +v|-v
//  |    box     |             -v|+v  box   +v|-v
//  |            |             -v|+v        +v|-v
//  |+h+h+h+h+h+h|             -v|+v        +v|-v
//  |------------| mean=hbot   -v|+v--------+v|-v
//   -h-h-h-h-h-h
//                           mean=vleft  mean=vright
//
// Returns MAX(htop,hbot) - MAX(vleft,vright), which is a positive number
// for a horizontal textline, a negative number for a vertical textline,
// and near zero for undecided. Undecided is most likely non-text.
// All the gradients are truncated to remain non-negative, since negative
// horizontal gradients don't give any indication of being vertical and
// vice versa.
// Additional complexity: The coordinates have to be transformed to original
// image coordinates with denorm (if not null), scaled to match the projection
// pix, and THEN step out 2 pixels each way from the edge to compute the
// gradient, and tries 3 positions, each measuring the gradient over a
// 4-pixel spread: (+3/-1), (+2/-2), (+1/-3).  This complexity is handled by
// several layers of helpers below.
int TextlineProjection::EvaluateBox(const TBOX& box, const DENORM* denorm,
                                    bool debug) const {
  return EvaluateBoxInternal(box, denorm, debug, NULL, NULL, NULL, NULL);
}

// Internal version of EvaluateBox returns the unclipped gradients as well
// as the result of EvaluateBox.
// hgrad1 and hgrad2 are the gradients for the horizontal textline.
int TextlineProjection::EvaluateBoxInternal(const TBOX& box,
                                            const DENORM* denorm, bool debug,
                                            int* hgrad1, int* hgrad2,
                                            int* vgrad1, int* vgrad2) const {
  int top_gradient = BestMeanGradientInRow(denorm, box.left(), box.right(),
                                           box.top(), true);
  int bottom_gradient = -BestMeanGradientInRow(denorm, box.left(), box.right(),
                                               box.bottom(), false);
  int left_gradient = BestMeanGradientInColumn(denorm, box.left(), box.bottom(),
                                               box.top(), true);
  int right_gradient = -BestMeanGradientInColumn(denorm, box.right(),
                                                 box.bottom(), box.top(),
                                                 false);
  int top_clipped = MAX(top_gradient, 0);
  int bottom_clipped = MAX(bottom_gradient, 0);
  int left_clipped = MAX(left_gradient, 0);
  int right_clipped = MAX(right_gradient, 0);
  if (debug) {
    tprintf("Gradients: top = %d, bottom = %d, left= %d, right= %d for box:",
            top_gradient, bottom_gradient, left_gradient, right_gradient);
    box.print();
  }
  int result = MAX(top_clipped, bottom_clipped) -
      MAX(left_clipped, right_clipped);
  if (hgrad1 != NULL && hgrad2 != NULL) {
    *hgrad1 = top_gradient;
    *hgrad2 = bottom_gradient;
  }
  if (vgrad1 != NULL && vgrad2 != NULL) {
    *vgrad1 = left_gradient;
    *vgrad2 = right_gradient;
  }
  return result;
}

// Helper returns the mean gradient value for the horizontal row at the given
// y, (in the external coordinates) by subtracting the mean of the transformed
// row 2 pixels above from the mean of the transformed row 2 pixels below.
// This gives a positive value for a good top edge and negative for bottom.
// Returns the best result out of +2/-2, +3/-1, +1/-3 pixels from the edge.
int TextlineProjection::BestMeanGradientInRow(const DENORM* denorm,
                                              inT16 min_x, inT16 max_x, inT16 y,
                                              bool best_is_max) const {
  TPOINT start_pt(min_x, y);
  TPOINT end_pt(max_x, y);
  int upper = MeanPixelsInLineSegment(denorm, -2, start_pt, end_pt);
  int lower = MeanPixelsInLineSegment(denorm, 2, start_pt, end_pt);
  int best_gradient = lower - upper;
  upper = MeanPixelsInLineSegment(denorm, -1, start_pt, end_pt);
  lower = MeanPixelsInLineSegment(denorm, 3, start_pt, end_pt);
  int gradient = lower - upper;
  if ((gradient > best_gradient) == best_is_max)
    best_gradient = gradient;
  upper = MeanPixelsInLineSegment(denorm, -3, start_pt, end_pt);
  lower = MeanPixelsInLineSegment(denorm, 1, start_pt, end_pt);
  gradient = lower - upper;
  if ((gradient > best_gradient) == best_is_max)
    best_gradient = gradient;
  return best_gradient;
}

// Helper returns the mean gradient value for the vertical column at the
// given x, (in the external coordinates) by subtracting the mean of the
// transformed column 2 pixels left from the mean of the transformed column
// 2 pixels to the right.
// This gives a positive value for a good left edge and negative for right.
// Returns the best result out of +2/-2, +3/-1, +1/-3 pixels from the edge.
int TextlineProjection::BestMeanGradientInColumn(const DENORM* denorm, inT16 x,
                                                 inT16 min_y, inT16 max_y,
                                                 bool best_is_max) const {
  TPOINT start_pt(x, min_y);
  TPOINT end_pt(x, max_y);
  int left = MeanPixelsInLineSegment(denorm, -2, start_pt, end_pt);
  int right = MeanPixelsInLineSegment(denorm, 2, start_pt, end_pt);
  int best_gradient = right - left;
  left = MeanPixelsInLineSegment(denorm, -1, start_pt, end_pt);
  right = MeanPixelsInLineSegment(denorm, 3, start_pt, end_pt);
  int gradient = right - left;
  if ((gradient > best_gradient) == best_is_max)
    best_gradient = gradient;
  left = MeanPixelsInLineSegment(denorm, -3, start_pt, end_pt);
  right = MeanPixelsInLineSegment(denorm, 1, start_pt, end_pt);
  gradient = right - left;
  if ((gradient > best_gradient) == best_is_max)
    best_gradient = gradient;
  return best_gradient;
}

// Helper returns the mean pixel value over the line between the start_pt and
// end_pt (inclusive), but shifted perpendicular to the line in the projection
// image by offset pixels. For simplicity, it is assumed that the vector is
// either nearly horizontal or nearly vertical. It works on skewed textlines!
// The end points are in external coordinates, and will be denormalized with
// the denorm if not NULL before further conversion to pix coordinates.
// After all the conversions, the offset is added to the direction
// perpendicular to the line direction. The offset is thus in projection image
// coordinates, which allows the caller to get a guaranteed displacement
// between pixels used to calculate gradients.
int TextlineProjection::MeanPixelsInLineSegment(const DENORM* denorm,
                                                int offset,
                                                TPOINT start_pt,
                                                TPOINT end_pt) const {
  TransformToPixCoords(denorm, &start_pt);
  TransformToPixCoords(denorm, &end_pt);
  TruncateToImageBounds(&start_pt);
  TruncateToImageBounds(&end_pt);
  int wpl = pixGetWpl(pix_);
  uinT32* data = pixGetData(pix_);
  int total = 0;
  int count = 0;
  int x_delta = end_pt.x - start_pt.x;
  int y_delta = end_pt.y - start_pt.y;
  if (abs(x_delta) >= abs(y_delta)) {
    if (x_delta == 0)
      return 0;
    // Horizontal line. Add the offset vertically.
    int x_step = x_delta > 0 ? 1 : -1;
    // Correct offset for rotation, keeping it anti-clockwise of the delta.
    offset *= x_step;
    start_pt.y += offset;
    end_pt.y += offset;
    TruncateToImageBounds(&start_pt);
    TruncateToImageBounds(&end_pt);
    x_delta = end_pt.x - start_pt.x;
    y_delta = end_pt.y - start_pt.y;
    count = x_delta * x_step + 1;
    for (int x = start_pt.x; x != end_pt.x; x += x_step) {
      int y = start_pt.y + DivRounded(y_delta * (x - start_pt.x), x_delta);
      total += GET_DATA_BYTE(data + wpl * y, x);
    }
  } else {
    // Vertical line. Add the offset horizontally.
    int y_step = y_delta > 0 ? 1 : -1;
    // Correct offset for rotation, keeping it anti-clockwise of the delta.
    // Pix holds the image with y=0 at the top, so the offset is negated.
    offset *= -y_step;
    start_pt.x += offset;
    end_pt.x += offset;
    TruncateToImageBounds(&start_pt);
    TruncateToImageBounds(&end_pt);
    x_delta = end_pt.x - start_pt.x;
    y_delta = end_pt.y - start_pt.y;
    count = y_delta * y_step + 1;
    for (int y = start_pt.y; y != end_pt.y; y += y_step) {
      int x = start_pt.x + DivRounded(x_delta * (y - start_pt.y), y_delta);
      total += GET_DATA_BYTE(data + wpl * y, x);
    }
  }
  return DivRounded(total, count);
}

// Given an input pix, and a box, the sides of the box are shrunk inwards until
// they bound any black pixels found within the original box.
// The function converts between tesseract coords and the pix coords assuming
// that this pix is full resolution equal in size to the original image.
// Returns an empty box if there are no black pixels in the source box.
static TBOX BoundsWithinBox(Pix* pix, const TBOX& box) {
  int im_height = pixGetHeight(pix);
  Box* input_box = boxCreate(box.left(), im_height - box.top(),
                             box.width(), box.height());
  Box* output_box = NULL;
  pixClipBoxToForeground(pix, input_box, NULL, &output_box);
  TBOX result_box;
  if (output_box != NULL) {
    l_int32 x, y, width, height;
    boxGetGeometry(output_box, &x, &y, &width, &height);
    result_box.set_left(x);
    result_box.set_right(x + width);
    result_box.set_top(im_height - y);
    result_box.set_bottom(result_box.top() - height);
    boxDestroy(&output_box);
  }
  boxDestroy(&input_box);
  return result_box;
}

// Splits the given box in half at x_middle or y_middle according to split_on_x
// and checks for nontext_map pixels in each half. Reduces the bbox so that it
// still includes the middle point, but does not touch any fg pixels in
// nontext_map. An empty box may be returned if there is no such box.
static void TruncateBoxToMissNonText(int x_middle, int y_middle,
                                     bool split_on_x, Pix* nontext_map,
                                     TBOX* bbox) {
  TBOX box1(*bbox);
  TBOX box2(*bbox);
  TBOX im_box;
  if (split_on_x) {
    box1.set_right(x_middle);
    im_box = BoundsWithinBox(nontext_map, box1);
    if (!im_box.null_box()) box1.set_left(im_box.right());
    box2.set_left(x_middle);
    im_box = BoundsWithinBox(nontext_map, box2);
    if (!im_box.null_box()) box2.set_right(im_box.left());
  } else {
    box1.set_bottom(y_middle);
    im_box = BoundsWithinBox(nontext_map, box1);
    if (!im_box.null_box()) box1.set_top(im_box.bottom());
    box2.set_top(y_middle);
    im_box = BoundsWithinBox(nontext_map, box2);
    if (!im_box.null_box()) box2.set_bottom(im_box.top());
  }
  box1 += box2;
  *bbox = box1;
}


// Helper function to add 1 to a rectangle in source image coords to the
// internal projection pix_.
void TextlineProjection::IncrementRectangle8Bit(const TBOX& box) {
  int scaled_left = ImageXToProjectionX(box.left());
  int scaled_top = ImageYToProjectionY(box.top());
  int scaled_right = ImageXToProjectionX(box.right());
  int scaled_bottom = ImageYToProjectionY(box.bottom());
  int wpl = pixGetWpl(pix_);
  uinT32* data = pixGetData(pix_) + scaled_top * wpl;
  for (int y = scaled_top; y <= scaled_bottom; ++y) {
    for (int x = scaled_left; x <= scaled_right; ++x) {
      int pixel = GET_DATA_BYTE(data, x);
      if (pixel < 255)
        SET_DATA_BYTE(data, x, pixel + 1);
    }
    data += wpl;
  }
}

// Inserts a list of blobs into the projection.
// Rotation is a multiple of 90 degrees to get from blob coords to
// nontext_map coords, nontext_map_box is the bounds of the nontext_map.
// Blobs are spread horizontally or vertically according to their internal
// flags, but the spreading is truncated by set pixels in the nontext_map
// and also by the horizontal rule line limits on the blobs.
void TextlineProjection::ProjectBlobs(BLOBNBOX_LIST* blobs,
                                      const FCOORD& rotation,
                                      const TBOX& nontext_map_box,
                                      Pix* nontext_map) {
  BLOBNBOX_IT blob_it(blobs);
  for (blob_it.mark_cycle_pt(); !blob_it.cycled_list(); blob_it.forward()) {
    BLOBNBOX* blob = blob_it.data();
    TBOX bbox = blob->bounding_box();
    ICOORD middle((bbox.left() + bbox.right()) / 2,
                  (bbox.bottom() + bbox.top()) / 2);
    bool spreading_horizontally = PadBlobBox(blob, &bbox);
    // Rotate to match the nontext_map.
    bbox.rotate(rotation);
    middle.rotate(rotation);
    if (rotation.x() == 0.0f)
      spreading_horizontally = !spreading_horizontally;
    // Clip to the image before applying the increments.
    bbox &= nontext_map_box;  // This is in-place box intersection.
    // Check for image pixels before spreading.
    TruncateBoxToMissNonText(middle.x(), middle.y(), spreading_horizontally,
                             nontext_map, &bbox);
    if (bbox.area() > 0) {
      IncrementRectangle8Bit(bbox);
    }
  }
}

// Pads the bounding box of the given blob according to whether it is on
// a horizontal or vertical text line, taking into account tab-stops near
// the blob. Returns true if padding was in the horizontal direction.
bool TextlineProjection::PadBlobBox(BLOBNBOX* blob, TBOX* bbox) {
  // Determine which direction to spread.
  // If text is well spaced out, it can be useful to pad perpendicular to
  // the textline direction, so as to ensure diacritics get absorbed
  // correctly, but if the text is tightly spaced, this will destroy the
  // blank space between textlines in the projection map, and that would
  // be very bad.
  int pad_limit = scale_factor_ * kMinLineSpacingFactor;
  int xpad = 0;
  int ypad = 0;
  bool padding_horizontally = false;
  if (blob->UniquelyHorizontal()) {
    xpad = bbox->height() * kOrientedPadFactor;
    padding_horizontally = true;
    // If the text appears to be very well spaced, pad the other direction by a
    // single pixel in the projection profile space to help join diacritics to
    // the textline.
    if ((blob->neighbour(BND_ABOVE) == NULL ||
        bbox->y_gap(blob->neighbour(BND_ABOVE)->bounding_box()) > pad_limit) &&
        (blob->neighbour(BND_BELOW) == NULL ||
        bbox->y_gap(blob->neighbour(BND_BELOW)->bounding_box()) > pad_limit)) {
      ypad = scale_factor_;
    }
  } else if (blob->UniquelyVertical()) {
    ypad = bbox->width() * kOrientedPadFactor;
    if ((blob->neighbour(BND_LEFT) == NULL ||
        bbox->x_gap(blob->neighbour(BND_LEFT)->bounding_box()) > pad_limit) &&
        (blob->neighbour(BND_RIGHT) == NULL ||
        bbox->x_gap(blob->neighbour(BND_RIGHT)->bounding_box()) > pad_limit)) {
      xpad = scale_factor_;
    }
  } else {
    if ((blob->neighbour(BND_ABOVE) != NULL &&
         blob->neighbour(BND_ABOVE)->neighbour(BND_BELOW) == blob) ||
        (blob->neighbour(BND_BELOW) != NULL &&
            blob->neighbour(BND_BELOW)->neighbour(BND_ABOVE) == blob)) {
      ypad = bbox->width() * kDefaultPadFactor;
    }
    if ((blob->neighbour(BND_RIGHT) != NULL &&
         blob->neighbour(BND_RIGHT)->neighbour(BND_LEFT) == blob) ||
        (blob->neighbour(BND_LEFT) != NULL &&
            blob->neighbour(BND_LEFT)->neighbour(BND_RIGHT) == blob)) {
      xpad = bbox->height() * kDefaultPadFactor;
      padding_horizontally = true;
    }
  }
  bbox->pad(xpad, ypad);
  pad_limit = scale_factor_ * kMaxTabStopOverrun;
  // Now shrink horizontally to avoid stepping more than pad_limit over a
  // tab-stop.
  if (bbox->left() < blob->left_rule() - pad_limit) {
    bbox->set_left(blob->left_rule() - pad_limit);
  }
  if (bbox->right() > blob->right_rule() + pad_limit) {
    bbox->set_right(blob->right_rule() + pad_limit);
  }
  return padding_horizontally;
}

// Helper denormalizes the TPOINT with the denorm if not NULL, then
// converts to pix_ coordinates.
void TextlineProjection::TransformToPixCoords(const DENORM* denorm,
                                              TPOINT* pt) const {
  if (denorm != NULL) {
    // Denormalize the point.
    denorm->DenormTransform(NULL, *pt, pt);
  }
  pt->x = ImageXToProjectionX(pt->x);
  pt->y = ImageYToProjectionY(pt->y);
}

#ifdef _MSC_VER
#pragma optimize("g", off)
#endif  // _MSC_VER
// Helper truncates the TPOINT to be within the pix_.
void TextlineProjection::TruncateToImageBounds(TPOINT* pt) const {
  pt->x = ClipToRange<int>(pt->x, 0, pixGetWidth(pix_) - 1);
  pt->y = ClipToRange<int>(pt->y, 0, pixGetHeight(pix_) - 1);
}
#ifdef _MSC_VER
#pragma optimize("", on)
#endif  // _MSC_VER

// Transform tesseract image coordinates to coordinates used in the projection.
int TextlineProjection::ImageXToProjectionX(int x) const {
  x = ClipToRange((x - x_origin_) / scale_factor_, 0, pixGetWidth(pix_) - 1);
  return x;
}
int TextlineProjection::ImageYToProjectionY(int y) const {
  y = ClipToRange((y_origin_ - y) / scale_factor_, 0, pixGetHeight(pix_) - 1);
  return y;
}

}  // namespace tesseract.
