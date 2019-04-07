/**********************************************************************
 * File:        blobbox.cpp  (Formerly blobnbox.c)
 * Description: Code for the textord blob class.
 * Author:      Ray Smith
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

// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif

#include "blobbox.h"
#include <algorithm>     // for max, min
#include <cstdint>       // for INT32_MAX, INT16_MAX
#include "allheaders.h"  // for pixGetHeight, pixGetPixel
#include "blobs.h"       // for TPOINT
#include "coutln.h"      // for C_OUTLINE_IT, C_OUTLINE, C_OUTLINE_LIST
#include "environ.h"     // for l_uint32
#include "helpers.h"     // for UpdateRange, IntCastRounded
#include "host.h"        // for NearlyEqual
#include "points.h"      // for operator+=, ICOORD::rotate

struct Pix;

#define PROJECTION_MARGIN 10     //arbitrary

ELISTIZE(BLOBNBOX)
ELIST2IZE(TO_ROW)
ELISTIZE(TO_BLOCK)

// Up to 30 degrees is allowed for rotations of diacritic blobs.
const double kCosSmallAngle = 0.866;
// Min aspect ratio for a joined word to indicate an obvious flow direction.
const double kDefiniteAspectRatio = 2.0;
// Multiple of short length in perimeter to make a joined word.
const double kComplexShapePerimeterRatio = 1.5;
// Min multiple of linesize for medium-sized blobs in ReFilterBlobs.
const double kMinMediumSizeRatio = 0.25;
// Max multiple of linesize for medium-sized blobs in ReFilterBlobs.
const double kMaxMediumSizeRatio = 4.0;

// Rotates the box and the underlying blob.
void BLOBNBOX::rotate(FCOORD rotation) {
  cblob_ptr->rotate(rotation);
  rotate_box(rotation);
  compute_bounding_box();
}

// Reflect the box in the y-axis, leaving the underlying blob untouched.
void BLOBNBOX::reflect_box_in_y_axis() {
  int left = -box.right();
  box.set_right(-box.left());
  box.set_left(left);
}

// Rotates the box by the angle given by rotation.
// If the blob is a diacritic, then only small rotations for skew
// correction can be applied.
void BLOBNBOX::rotate_box(FCOORD rotation) {
  if (IsDiacritic()) {
    ASSERT_HOST(rotation.x() >= kCosSmallAngle);
    ICOORD top_pt((box.left() + box.right()) / 2, base_char_top_);
    ICOORD bottom_pt(top_pt.x(), base_char_bottom_);
    top_pt.rotate(rotation);
    base_char_top_ = top_pt.y();
    bottom_pt.rotate(rotation);
    base_char_bottom_ = bottom_pt.y();
    box.rotate(rotation);
  } else {
    box.rotate(rotation);
    set_diacritic_box(box);
  }
}

/**********************************************************************
 * BLOBNBOX::merge
 *
 * Merge this blob with the given blob, which should be after this.
 **********************************************************************/
void BLOBNBOX::merge(                    //merge blobs
                     BLOBNBOX *nextblob  //blob to join with
                    ) {
  box += nextblob->box;          //merge boxes
  set_diacritic_box(box);
  nextblob->joined = true;
}


// Merge this with other, taking the outlines from other.
// Other is not deleted, but left for the caller to handle.
void BLOBNBOX::really_merge(BLOBNBOX* other) {
  if (other->cblob_ptr != nullptr) {
    C_OUTLINE_IT ol_it(cblob_ptr->out_list());
    ol_it.add_list_after(other->cblob_ptr->out_list());
  }
  compute_bounding_box();
}


/**********************************************************************
 * BLOBNBOX::chop
 *
 * Chop this blob into equal sized pieces using the x height as a guide.
 * The blob is not actually chopped. Instead, fake blobs are inserted
 * with the relevant bounding boxes.
 **********************************************************************/

void BLOBNBOX::chop(                        //chop blobs
                    BLOBNBOX_IT *start_it,  //location of this
                    BLOBNBOX_IT *end_it,    //iterator
                    FCOORD rotation,        //for landscape
                    float xheight           //of line
                   ) {
  int16_t blobcount;               //no of blobs
  BLOBNBOX *newblob;             //fake blob
  BLOBNBOX *blob;                //current blob
  int16_t blobindex;               //number of chop
  int16_t leftx;                   //left edge of blob
  float blobwidth;               //width of each
  float rightx;                  //right edge to scan
  float ymin, ymax;              //limits of new blob
  float test_ymin, test_ymax;    //limits of part blob
  ICOORD bl, tr;                 //corners of box
  BLOBNBOX_IT blob_it;           //blob iterator

                                 //get no of chops
  blobcount = static_cast<int16_t>(floor (box.width () / xheight));
  if (blobcount > 1 && cblob_ptr != nullptr) {
                                 //width of each
    blobwidth = static_cast<float>(box.width () + 1) / blobcount;
    for (blobindex = blobcount - 1, rightx = box.right ();
    blobindex >= 0; blobindex--, rightx -= blobwidth) {
      ymin = static_cast<float>(INT32_MAX);
      ymax = static_cast<float>(-INT32_MAX);
      blob_it = *start_it;
      do {
        blob = blob_it.data ();
        find_cblob_vlimits(blob->cblob_ptr, rightx - blobwidth,
                           rightx,
            /*rotation, */ test_ymin, test_ymax);
        blob_it.forward ();
        UpdateRange(test_ymin, test_ymax, &ymin, &ymax);
      }
      while (blob != end_it->data ());
      if (ymin < ymax) {
        leftx = static_cast<int16_t>(floor (rightx - blobwidth));
        if (leftx < box.left ())
          leftx = box.left ();   //clip to real box
        bl = ICOORD (leftx, static_cast<int16_t>(floor (ymin)));
        tr = ICOORD (static_cast<int16_t>(ceil (rightx)), static_cast<int16_t>(ceil (ymax)));
        if (blobindex == 0)
          box = TBOX (bl, tr);    //change box
        else {
          newblob = new BLOBNBOX;
                                 //box is all it has
          newblob->box = TBOX (bl, tr);
                                 //stay on current
          newblob->base_char_top_ = tr.y();
          newblob->base_char_bottom_ = bl.y();
          end_it->add_after_stay_put (newblob);
        }
      }
    }
  }
}

// Returns the box gaps between this and its neighbours_ in an array
// indexed by BlobNeighbourDir.
void BLOBNBOX::NeighbourGaps(int gaps[BND_COUNT]) const {
  for (int dir = 0; dir < BND_COUNT; ++dir) {
    gaps[dir] = INT16_MAX;
    BLOBNBOX* neighbour = neighbours_[dir];
    if (neighbour != nullptr) {
      const TBOX& n_box = neighbour->bounding_box();
      if (dir == BND_LEFT || dir == BND_RIGHT) {
        gaps[dir] = box.x_gap(n_box);
      } else {
        gaps[dir] = box.y_gap(n_box);
      }
    }
  }
}
// Returns the min and max horizontal and vertical gaps (from NeighbourGaps)
// modified so that if the max exceeds the max dimension of the blob, and
// the min is less, the max is replaced with the min.
// The objective is to catch cases where there is only a single neighbour
// and avoid reporting the other gap as a ridiculously large number
void BLOBNBOX::MinMaxGapsClipped(int* h_min, int* h_max,
                                 int* v_min, int* v_max) const {
  int max_dimension = std::max(box.width(), box.height());
  int gaps[BND_COUNT];
  NeighbourGaps(gaps);
  *h_min = std::min(gaps[BND_LEFT], gaps[BND_RIGHT]);
  *h_max = std::max(gaps[BND_LEFT], gaps[BND_RIGHT]);
  if (*h_max > max_dimension && *h_min < max_dimension) *h_max = *h_min;
  *v_min = std::min(gaps[BND_ABOVE], gaps[BND_BELOW]);
  *v_max = std::max(gaps[BND_ABOVE], gaps[BND_BELOW]);
  if (*v_max > max_dimension && *v_min < max_dimension) *v_max = *v_min;
}

// Nulls out any neighbours that are DeletableNoise to remove references.
void BLOBNBOX::CleanNeighbours() {
  for (int dir = 0; dir < BND_COUNT; ++dir) {
    BLOBNBOX* neighbour = neighbours_[dir];
    if (neighbour != nullptr && neighbour->DeletableNoise()) {
      neighbours_[dir] = nullptr;
      good_stroke_neighbours_[dir] = false;
    }
  }
}

// Returns positive if there is at least one side neighbour that has a similar
// stroke width and is not on the other side of a rule line.
int BLOBNBOX::GoodTextBlob() const {
  int score = 0;
  for (int dir = 0; dir < BND_COUNT; ++dir) {
    auto bnd = static_cast<BlobNeighbourDir>(dir);
    if (good_stroke_neighbour(bnd))
      ++score;
  }
  return score;
}

// Returns the number of side neighbours that are of type BRT_NOISE.
int BLOBNBOX::NoisyNeighbours() const {
  int count = 0;
  for (int dir = 0; dir < BND_COUNT; ++dir) {
    auto bnd = static_cast<BlobNeighbourDir>(dir);
    BLOBNBOX* blob = neighbour(bnd);
    if (blob != nullptr && blob->region_type() == BRT_NOISE)
      ++count;
  }
  return count;
}

// Returns true, and sets vert_possible/horz_possible if the blob has some
// feature that makes it individually appear to flow one way.
// eg if it has a high aspect ratio, yet has a complex shape, such as a
// joined word in Latin, Arabic, or Hindi, rather than being a -, I, l, 1 etc.
bool BLOBNBOX::DefiniteIndividualFlow() {
  if (cblob() == nullptr) return false;
  int box_perimeter = 2 * (box.height() + box.width());
  if (box.width() > box.height() * kDefiniteAspectRatio) {
    // Attempt to distinguish a wide joined word from a dash.
    // If it is a dash, then its perimeter is approximately
    // 2 * (box width + stroke width), but more if the outline is noisy,
    // so perimeter - 2*(box width + stroke width) should be close to zero.
    // A complex shape such as a joined word should have a much larger value.
    int perimeter = cblob()->perimeter();
    if (vert_stroke_width() > 0 || perimeter <= 0)
      perimeter -= 2 * vert_stroke_width();
    else
      perimeter -= 4 * cblob()->area() / perimeter;
    perimeter -= 2 * box.width();
    // Use a multiple of the box perimeter as a threshold.
    if (perimeter > kComplexShapePerimeterRatio * box_perimeter) {
      set_vert_possible(false);
      set_horz_possible(true);
      return true;
    }
  }
  if (box.height() > box.width() * kDefiniteAspectRatio) {
    // As above, but for a putative vertical word vs a I/1/l.
    int perimeter = cblob()->perimeter();
    if (horz_stroke_width() > 0 || perimeter <= 0)
      perimeter -= 2 * horz_stroke_width();
    else
      perimeter -= 4 * cblob()->area() / perimeter;
    perimeter -= 2 * box.height();
    if (perimeter > kComplexShapePerimeterRatio * box_perimeter) {
      set_vert_possible(true);
      set_horz_possible(false);
      return true;
    }
  }
  return false;
}

// Returns true if there is no tabstop violation in merging this and other.
bool BLOBNBOX::ConfirmNoTabViolation(const BLOBNBOX& other) const {
  if (box.left() < other.box.left() && box.left() < other.left_rule_)
    return false;
  if (other.box.left() < box.left() && other.box.left() < left_rule_)
    return false;
  if (box.right() > other.box.right() && box.right() > other.right_rule_)
    return false;
  if (other.box.right() > box.right() && other.box.right() > right_rule_)
    return false;
  return true;
}

// Returns true if other has a similar stroke width to this.
bool BLOBNBOX::MatchingStrokeWidth(const BLOBNBOX& other,
                                   double fractional_tolerance,
                                   double constant_tolerance) const {
  // The perimeter-based width is used as a backup in case there is
  // no information in the blob.
  double p_width = area_stroke_width();
  double n_p_width = other.area_stroke_width();
  float h_tolerance = horz_stroke_width_ * fractional_tolerance
                     + constant_tolerance;
  float v_tolerance = vert_stroke_width_ * fractional_tolerance
                     + constant_tolerance;
  double p_tolerance = p_width * fractional_tolerance
                     + constant_tolerance;
  bool h_zero = horz_stroke_width_ == 0.0f || other.horz_stroke_width_ == 0.0f;
  bool v_zero = vert_stroke_width_ == 0.0f || other.vert_stroke_width_ == 0.0f;
  bool h_ok = !h_zero && NearlyEqual(horz_stroke_width_,
                                     other.horz_stroke_width_, h_tolerance);
  bool v_ok = !v_zero && NearlyEqual(vert_stroke_width_,
                                     other.vert_stroke_width_, v_tolerance);
  bool p_ok = h_zero && v_zero && NearlyEqual(p_width, n_p_width, p_tolerance);
  // For a match, at least one of the horizontal and vertical widths
  // must match, and the other one must either match or be zero.
  // Only if both are zero will we look at the perimeter metric.
  return p_ok || ((v_ok || h_ok) && (h_ok || h_zero) && (v_ok || v_zero));
}

// Returns a bounding box of the outline contained within the
// given horizontal range.
TBOX BLOBNBOX::BoundsWithinLimits(int left, int right) {
  FCOORD no_rotation(1.0f, 0.0f);
  float top = box.top();
  float bottom = box.bottom();
  if (cblob_ptr != nullptr) {
    find_cblob_limits(cblob_ptr, static_cast<float>(left),
                      static_cast<float>(right), no_rotation,
                      bottom, top);
  }

  if (top < bottom) {
    top = box.top();
    bottom = box.bottom();
  }
  FCOORD bot_left(left, bottom);
  FCOORD top_right(right, top);
  TBOX shrunken_box(bot_left);
  TBOX shrunken_box2(top_right);
  shrunken_box += shrunken_box2;
  return shrunken_box;
}

// Estimates and stores the baseline position based on the shape of the
// outline.
void BLOBNBOX::EstimateBaselinePosition() {
  baseline_y_ = box.bottom();  // The default.
  if (cblob_ptr == nullptr) return;
  baseline_y_ = cblob_ptr->EstimateBaselinePosition();
}

// Helper to call CleanNeighbours on all blobs on the list.
void BLOBNBOX::CleanNeighbours(BLOBNBOX_LIST* blobs) {
  BLOBNBOX_IT blob_it(blobs);
  for (blob_it.mark_cycle_pt(); !blob_it.cycled_list(); blob_it.forward()) {
    blob_it.data()->CleanNeighbours();
  }
}

// Helper to delete all the deletable blobs on the list.
void BLOBNBOX::DeleteNoiseBlobs(BLOBNBOX_LIST* blobs) {
  BLOBNBOX_IT blob_it(blobs);
  for (blob_it.mark_cycle_pt(); !blob_it.cycled_list(); blob_it.forward()) {
    BLOBNBOX* blob = blob_it.data();
    if (blob->DeletableNoise()) {
      delete blob->cblob();
      delete blob_it.extract();
    }
  }
}

// Helper to compute edge offsets for  all the blobs on the list.
// See coutln.h for an explanation of edge offsets.
void BLOBNBOX::ComputeEdgeOffsets(Pix* thresholds, Pix* grey,
                                  BLOBNBOX_LIST* blobs) {
  int grey_height = 0;
  int thr_height = 0;
  int scale_factor = 1;
  if (thresholds != nullptr && grey != nullptr) {
    grey_height = pixGetHeight(grey);
    thr_height = pixGetHeight(thresholds);
    scale_factor =
        IntCastRounded(static_cast<double>(grey_height) / thr_height);
  }
  BLOBNBOX_IT blob_it(blobs);
  for (blob_it.mark_cycle_pt(); !blob_it.cycled_list(); blob_it.forward()) {
    BLOBNBOX* blob = blob_it.data();
    if (blob->cblob() != nullptr) {
      // Get the threshold that applies to this blob.
      l_uint32 threshold = 128;
      if (thresholds != nullptr && grey != nullptr) {
        const TBOX& box = blob->cblob()->bounding_box();
        // Transform the coordinates if required.
        TPOINT pt((box.left() + box.right()) / 2,
                  (box.top() + box.bottom()) / 2);
        pixGetPixel(thresholds, pt.x / scale_factor,
                    thr_height - 1 - pt.y / scale_factor, &threshold);
      }
      blob->cblob()->ComputeEdgeOffsets(threshold, grey);
    }
  }
}


#ifndef GRAPHICS_DISABLED
// Helper to draw all the blobs on the list in the given body_colour,
// with child outlines in the child_colour.
void BLOBNBOX::PlotBlobs(BLOBNBOX_LIST* list,
                         ScrollView::Color body_colour,
                         ScrollView::Color child_colour,
                         ScrollView* win) {
  BLOBNBOX_IT it(list);
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    it.data()->plot(win, body_colour, child_colour);
  }
}

// Helper to draw only DeletableNoise blobs (unowned, BRT_NOISE) on the
// given list in the given body_colour, with child outlines in the
// child_colour.
void BLOBNBOX::PlotNoiseBlobs(BLOBNBOX_LIST* list,
                              ScrollView::Color body_colour,
                              ScrollView::Color child_colour,
                              ScrollView* win) {
  BLOBNBOX_IT it(list);
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    BLOBNBOX* blob = it.data();
    if (blob->DeletableNoise())
      blob->plot(win, body_colour, child_colour);
  }
}

ScrollView::Color BLOBNBOX::TextlineColor(BlobRegionType region_type,
                                          BlobTextFlowType flow_type) {
  switch (region_type) {
    case BRT_HLINE:
      return ScrollView::BROWN;
    case BRT_VLINE:
      return ScrollView::DARK_GREEN;
    case BRT_RECTIMAGE:
      return ScrollView::RED;
    case BRT_POLYIMAGE:
      return ScrollView::ORANGE;
    case BRT_UNKNOWN:
      return flow_type == BTFT_NONTEXT ? ScrollView::CYAN : ScrollView::WHITE;
    case BRT_VERT_TEXT:
      if (flow_type == BTFT_STRONG_CHAIN || flow_type == BTFT_TEXT_ON_IMAGE)
        return ScrollView::GREEN;
      if (flow_type == BTFT_CHAIN)
        return ScrollView::LIME_GREEN;
      return ScrollView::YELLOW;
    case BRT_TEXT:
      if (flow_type == BTFT_STRONG_CHAIN)
        return ScrollView::BLUE;
      if (flow_type == BTFT_TEXT_ON_IMAGE)
        return ScrollView::LIGHT_BLUE;
      if (flow_type == BTFT_CHAIN)
        return ScrollView::MEDIUM_BLUE;
      if (flow_type == BTFT_LEADER)
        return ScrollView::WHEAT;
      if (flow_type == BTFT_NONTEXT)
        return ScrollView::PINK;
      return ScrollView::MAGENTA;
    default:
      return ScrollView::GREY;
  }
}

// Keep in sync with BlobRegionType.
ScrollView::Color BLOBNBOX::BoxColor() const {
  return TextlineColor(region_type_, flow_);
}

void BLOBNBOX::plot(ScrollView* window,                // window to draw in
                    ScrollView::Color blob_colour,     // for outer bits
                    ScrollView::Color child_colour) {  // for holes
  if (cblob_ptr != nullptr)
    cblob_ptr->plot(window, blob_colour, child_colour);
}
#endif
/**********************************************************************
 * find_cblob_limits
 *
 * Scan the outlines of the cblob to locate the y min and max
 * between the given x limits.
 **********************************************************************/

void find_cblob_limits(                  //get y limits
                       C_BLOB *blob,     //blob to search
                       float leftx,      //x limits
                       float rightx,
                       FCOORD rotation,  //for landscape
                       float &ymin,      //output y limits
                       float &ymax) {
  int16_t stepindex;               //current point
  ICOORD pos;                    //current coords
  ICOORD vec;                    //rotated step
  C_OUTLINE *outline;            //current outline
                                 //outlines
  C_OUTLINE_IT out_it = blob->out_list ();

  ymin = static_cast<float>(INT32_MAX);
  ymax = static_cast<float>(-INT32_MAX);
  for (out_it.mark_cycle_pt (); !out_it.cycled_list (); out_it.forward ()) {
    outline = out_it.data ();
    pos = outline->start_pos (); //get coords
    pos.rotate (rotation);
    for (stepindex = 0; stepindex < outline->pathlength (); stepindex++) {
                                 //inside
      if (pos.x () >= leftx && pos.x () <= rightx) {
        UpdateRange(pos.y(), &ymin, &ymax);
      }
      vec = outline->step (stepindex);
      vec.rotate (rotation);
      pos += vec;                //move to next
    }
  }
}


/**********************************************************************
 * find_cblob_vlimits
 *
 * Scan the outlines of the cblob to locate the y min and max
 * between the given x limits.
 **********************************************************************/

void find_cblob_vlimits(               //get y limits
                        C_BLOB *blob,  //blob to search
                        float leftx,   //x limits
                        float rightx,
                        float &ymin,   //output y limits
                        float &ymax) {
  int16_t stepindex;               //current point
  ICOORD pos;                    //current coords
  ICOORD vec;                    //rotated step
  C_OUTLINE *outline;            //current outline
                                 //outlines
  C_OUTLINE_IT out_it = blob->out_list ();

  ymin = static_cast<float>(INT32_MAX);
  ymax = static_cast<float>(-INT32_MAX);
  for (out_it.mark_cycle_pt (); !out_it.cycled_list (); out_it.forward ()) {
    outline = out_it.data ();
    pos = outline->start_pos (); //get coords
    for (stepindex = 0; stepindex < outline->pathlength (); stepindex++) {
                                 //inside
      if (pos.x () >= leftx && pos.x () <= rightx) {
        UpdateRange(pos.y(), &ymin, &ymax);
      }
      vec = outline->step (stepindex);
      pos += vec;                //move to next
    }
  }
}


/**********************************************************************
 * find_cblob_hlimits
 *
 * Scan the outlines of the cblob to locate the x min and max
 * between the given y limits.
 **********************************************************************/

void find_cblob_hlimits(                //get x limits
                        C_BLOB *blob,   //blob to search
                        float bottomy,  //y limits
                        float topy,
                        float &xmin,    //output x limits
                        float &xmax) {
  int16_t stepindex;               //current point
  ICOORD pos;                    //current coords
  ICOORD vec;                    //rotated step
  C_OUTLINE *outline;            //current outline
                                 //outlines
  C_OUTLINE_IT out_it = blob->out_list ();

  xmin = static_cast<float>(INT32_MAX);
  xmax = static_cast<float>(-INT32_MAX);
  for (out_it.mark_cycle_pt (); !out_it.cycled_list (); out_it.forward ()) {
    outline = out_it.data ();
    pos = outline->start_pos (); //get coords
    for (stepindex = 0; stepindex < outline->pathlength (); stepindex++) {
                                 //inside
      if (pos.y () >= bottomy && pos.y () <= topy) {
        UpdateRange(pos.x(), &xmin, &xmax);
      }
      vec = outline->step (stepindex);
      pos += vec;                //move to next
    }
  }
}

/**********************************************************************
 * crotate_cblob
 *
 * Rotate the copy by the given vector and return a C_BLOB.
 **********************************************************************/

C_BLOB *crotate_cblob(                 //rotate it
                      C_BLOB *blob,    //blob to search
                      FCOORD rotation  //for landscape
                     ) {
  C_OUTLINE_LIST out_list;       //output outlines
                                 //input outlines
  C_OUTLINE_IT in_it = blob->out_list ();
                                 //output outlines
  C_OUTLINE_IT out_it = &out_list;

  for (in_it.mark_cycle_pt (); !in_it.cycled_list (); in_it.forward ()) {
    out_it.add_after_then_move (new C_OUTLINE (in_it.data (), rotation));
  }
  return new C_BLOB (&out_list);
}


/**********************************************************************
 * box_next
 *
 * Compute the bounding box of this blob with merging of x overlaps
 * but no pre-chopping.
 * Then move the iterator on to the start of the next blob.
 **********************************************************************/

TBOX box_next(                 //get bounding box
             BLOBNBOX_IT *it  //iterator to blobds
            ) {
  BLOBNBOX *blob;                //current blob
  TBOX result;                    //total box

  blob = it->data ();
  result = blob->bounding_box ();
  do {
    it->forward ();
    blob = it->data ();
    if (blob->cblob() == nullptr)
                                 //was pre-chopped
      result += blob->bounding_box ();
  }
                                 //until next real blob
  while ((blob->cblob() == nullptr) || blob->joined_to_prev());
  return result;
}


/**********************************************************************
 * box_next_pre_chopped
 *
 * Compute the bounding box of this blob with merging of x overlaps
 * but WITH pre-chopping.
 * Then move the iterator on to the start of the next pre-chopped blob.
 **********************************************************************/

TBOX box_next_pre_chopped(                 //get bounding box
                         BLOBNBOX_IT *it  //iterator to blobds
                        ) {
  BLOBNBOX *blob;                //current blob
  TBOX result;                    //total box

  blob = it->data ();
  result = blob->bounding_box ();
  do {
    it->forward ();
    blob = it->data ();
  }
                                 //until next real blob
  while (blob->joined_to_prev ());
  return result;
}


/**********************************************************************
 * TO_ROW::TO_ROW
 *
 * Constructor to make a row from a blob.
 **********************************************************************/

TO_ROW::TO_ROW (                 //constructor
BLOBNBOX * blob,                 //first blob
float top,                       //corrected top
float bottom,                    //of row
float row_size                   //ideal
) {
  clear();
  y_min = bottom;
  y_max = top;
  initial_y_min = bottom;

  float diff;                    //in size
  BLOBNBOX_IT it = &blobs;       //list of blobs

  it.add_to_end (blob);
  diff = top - bottom - row_size;
  if (diff > 0) {
    y_max -= diff / 2;
    y_min += diff / 2;
  }
                                 //very small object
  else if ((top - bottom) * 3 < row_size) {
    diff = row_size / 3 + bottom - top;
    y_max += diff / 2;
    y_min -= diff / 2;
  }
}

void TO_ROW::print() const {
  tprintf("pitch=%d, fp=%g, fps=%g, fpns=%g, prs=%g, prns=%g,"
          " spacing=%g xh=%g y_origin=%g xev=%d, asc=%g, desc=%g,"
          " body=%g, minsp=%d maxnsp=%d, thr=%d kern=%g sp=%g\n",
          pitch_decision, fixed_pitch, fp_space, fp_nonsp, pr_space, pr_nonsp,
          spacing, xheight, y_origin, xheight_evidence, ascrise, descdrop,
          body_size, min_space, max_nonspace, space_threshold, kern_size,
          space_size);
}

/**********************************************************************
 * TO_ROW:add_blob
 *
 * Add the blob to the end of the row.
 **********************************************************************/

void TO_ROW::add_blob(                 //constructor
                      BLOBNBOX *blob,  //first blob
                      float top,       //corrected top
                      float bottom,    //of row
                      float row_size   //ideal
                     ) {
  float allowed;                 //allowed expansion
  float available;               //expansion
  BLOBNBOX_IT it = &blobs;       //list of blobs

  it.add_to_end (blob);
  allowed = row_size + y_min - y_max;
  if (allowed > 0) {
    available = top > y_max ? top - y_max : 0;
    if (bottom < y_min)
                                 //total available
        available += y_min - bottom;
    if (available > 0) {
      available += available;    //do it gradually
      if (available < allowed)
        available = allowed;
      if (bottom < y_min)
        y_min -= (y_min - bottom) * allowed / available;
      if (top > y_max)
        y_max += (top - y_max) * allowed / available;
    }
  }
}


/**********************************************************************
 * TO_ROW:insert_blob
 *
 * Add the blob to the row in the correct position.
 **********************************************************************/

void TO_ROW::insert_blob(                //constructor
                         BLOBNBOX *blob  //first blob
                        ) {
  BLOBNBOX_IT it = &blobs;       //list of blobs

  if (it.empty ())
    it.add_before_then_move (blob);
  else {
    it.mark_cycle_pt ();
    while (!it.cycled_list ()
      && it.data ()->bounding_box ().left () <=
      blob->bounding_box ().left ())
      it.forward ();
    if (it.cycled_list ())
      it.add_to_end (blob);
    else
      it.add_before_stay_put (blob);
  }
}


/**********************************************************************
 * TO_ROW::compute_vertical_projection
 *
 * Compute the vertical projection of a TO_ROW from its blobs.
 **********************************************************************/

void TO_ROW::compute_vertical_projection() {  //project whole row
  TBOX row_box;                   //bound of row
  BLOBNBOX *blob;                //current blob
  TBOX blob_box;                  //bounding box
  BLOBNBOX_IT blob_it = blob_list ();

  if (blob_it.empty ())
    return;
  row_box = blob_it.data ()->bounding_box ();
  for (blob_it.mark_cycle_pt (); !blob_it.cycled_list (); blob_it.forward ())
    row_box += blob_it.data ()->bounding_box ();

  projection.set_range (row_box.left () - PROJECTION_MARGIN,
    row_box.right () + PROJECTION_MARGIN);
  projection_left = row_box.left () - PROJECTION_MARGIN;
  projection_right = row_box.right () + PROJECTION_MARGIN;
  for (blob_it.mark_cycle_pt (); !blob_it.cycled_list (); blob_it.forward ()) {
    blob = blob_it.data();
    if (blob->cblob() != nullptr)
      vertical_cblob_projection(blob->cblob(), &projection);
  }
}


/**********************************************************************
 * TO_ROW::clear
 *
 * Zero out all scalar members.
 **********************************************************************/
void TO_ROW::clear() {
  all_caps = false;
  used_dm_model = false;
  projection_left = 0;
  projection_right = 0;
  pitch_decision = PITCH_DUNNO;
  fixed_pitch = 0.0;
  fp_space = 0.0;
  fp_nonsp = 0.0;
  pr_space = 0.0;
  pr_nonsp = 0.0;
  spacing = 0.0;
  xheight = 0.0;
  xheight_evidence = 0;
  body_size = 0.0;
  ascrise = 0.0;
  descdrop = 0.0;
  min_space = 0;
  max_nonspace = 0;
  space_threshold = 0;
  kern_size = 0.0;
  space_size = 0.0;
  y_min = 0.0;
  y_max = 0.0;
  initial_y_min = 0.0;
  m = 0.0;
  c = 0.0;
  error = 0.0;
  para_c = 0.0;
  para_error = 0.0;
  y_origin = 0.0;
  credibility = 0.0;
  num_repeated_sets_ = -1;
}


/**********************************************************************
 * vertical_cblob_projection
 *
 * Compute the vertical projection of a cblob from its outlines
 * and add to the given STATS.
 **********************************************************************/

void vertical_cblob_projection(               //project outlines
                               C_BLOB *blob,  //blob to project
                               STATS *stats   //output
                              ) {
                                 //outlines of blob
  C_OUTLINE_IT out_it = blob->out_list ();

  for (out_it.mark_cycle_pt (); !out_it.cycled_list (); out_it.forward ()) {
    vertical_coutline_projection (out_it.data (), stats);
  }
}


/**********************************************************************
 * vertical_coutline_projection
 *
 * Compute the vertical projection of a outline from its outlines
 * and add to the given STATS.
 **********************************************************************/

void vertical_coutline_projection(                     //project outlines
                                  C_OUTLINE *outline,  //outline to project
                                  STATS *stats         //output
                                 ) {
  ICOORD pos;                    //current point
  ICOORD step;                   //edge step
  int32_t length;                  //of outline
  int16_t stepindex;               //current step
  C_OUTLINE_IT out_it = outline->child ();

  pos = outline->start_pos ();
  length = outline->pathlength ();
  for (stepindex = 0; stepindex < length; stepindex++) {
    step = outline->step (stepindex);
    if (step.x () > 0) {
     stats->add (pos.x (), -pos.y ());
    } else if (step.x () < 0) {
      stats->add (pos.x () - 1, pos.y ());
    }
    pos += step;
  }

  for (out_it.mark_cycle_pt (); !out_it.cycled_list (); out_it.forward ()) {
    vertical_coutline_projection (out_it.data (), stats);
  }
}


/**********************************************************************
 * TO_BLOCK::TO_BLOCK
 *
 * Constructor to make a TO_BLOCK from a real block.
 **********************************************************************/

TO_BLOCK::TO_BLOCK(                  //make a block
                   BLOCK *src_block  //real block
                  ) {
  clear();
  block = src_block;
}

static void clear_blobnboxes(BLOBNBOX_LIST* boxes) {
  BLOBNBOX_IT it = boxes;
  // A BLOBNBOX generally doesn't own its blobs, so if they do, you
  // have to delete them explicitly.
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    BLOBNBOX* box = it.data();
    delete box->cblob();
  }
}

/**********************************************************************
 * TO_BLOCK::clear
 *
 * Zero out all scalar members.
 **********************************************************************/
void TO_BLOCK::clear() {
  block = nullptr;
  pitch_decision = PITCH_DUNNO;
  line_spacing = 0.0;
  line_size = 0.0;
  max_blob_size = 0.0;
  baseline_offset = 0.0;
  xheight = 0.0;
  fixed_pitch = 0.0;
  kern_size = 0.0;
  space_size = 0.0;
  min_space = 0;
  max_nonspace = 0;
  fp_space = 0.0;
  fp_nonsp = 0.0;
  pr_space = 0.0;
  pr_nonsp = 0.0;
  key_row = nullptr;
}


TO_BLOCK::~TO_BLOCK() {
  // Any residual BLOBNBOXes at this stage own their blobs, so delete them.
  clear_blobnboxes(&blobs);
  clear_blobnboxes(&underlines);
  clear_blobnboxes(&noise_blobs);
  clear_blobnboxes(&small_blobs);
  clear_blobnboxes(&large_blobs);
}

// Helper function to divide the input blobs over noise, small, medium
// and large lists. Blobs small in height and (small in width or large in width)
// go in the noise list. Dash (-) candidates go in the small list, and
// medium and large are by height.
// SIDE-EFFECT: reset all blobs to initial state by calling Init().
static void SizeFilterBlobs(int min_height, int max_height,
                            BLOBNBOX_LIST* src_list,
                            BLOBNBOX_LIST* noise_list,
                            BLOBNBOX_LIST* small_list,
                            BLOBNBOX_LIST* medium_list,
                            BLOBNBOX_LIST* large_list) {
  BLOBNBOX_IT noise_it(noise_list);
  BLOBNBOX_IT small_it(small_list);
  BLOBNBOX_IT medium_it(medium_list);
  BLOBNBOX_IT large_it(large_list);
  for (BLOBNBOX_IT src_it(src_list); !src_it.empty(); src_it.forward()) {
    BLOBNBOX* blob = src_it.extract();
    blob->ReInit();
    int width = blob->bounding_box().width();
    int height = blob->bounding_box().height();
    if (height < min_height  &&
        (width < min_height || width > max_height))
      noise_it.add_after_then_move(blob);
    else if (height > max_height)
      large_it.add_after_then_move(blob);
    else if (height < min_height)
      small_it.add_after_then_move(blob);
    else
      medium_it.add_after_then_move(blob);
  }
}

// Reorganize the blob lists with a different definition of small, medium
// and large, compared to the original definition.
// Height is still the primary filter key, but medium width blobs of small
// height become small, and very wide blobs of small height stay noise, along
// with small dot-shaped blobs.
void TO_BLOCK::ReSetAndReFilterBlobs() {
  int min_height = IntCastRounded(kMinMediumSizeRatio * line_size);
  int max_height = IntCastRounded(kMaxMediumSizeRatio * line_size);
  BLOBNBOX_LIST noise_list;
  BLOBNBOX_LIST small_list;
  BLOBNBOX_LIST medium_list;
  BLOBNBOX_LIST large_list;
  SizeFilterBlobs(min_height, max_height, &blobs,
                  &noise_list, &small_list, &medium_list, &large_list);
  SizeFilterBlobs(min_height, max_height, &large_blobs,
                  &noise_list, &small_list, &medium_list, &large_list);
  SizeFilterBlobs(min_height, max_height, &small_blobs,
                  &noise_list, &small_list, &medium_list, &large_list);
  SizeFilterBlobs(min_height, max_height, &noise_blobs,
                  &noise_list, &small_list, &medium_list, &large_list);
  BLOBNBOX_IT blob_it(&blobs);
  blob_it.add_list_after(&medium_list);
  blob_it.set_to_list(&large_blobs);
  blob_it.add_list_after(&large_list);
  blob_it.set_to_list(&small_blobs);
  blob_it.add_list_after(&small_list);
  blob_it.set_to_list(&noise_blobs);
  blob_it.add_list_after(&noise_list);
}

// Deletes noise blobs from all lists where not owned by a ColPartition.
void TO_BLOCK::DeleteUnownedNoise() {
  BLOBNBOX::CleanNeighbours(&blobs);
  BLOBNBOX::CleanNeighbours(&small_blobs);
  BLOBNBOX::CleanNeighbours(&noise_blobs);
  BLOBNBOX::CleanNeighbours(&large_blobs);
  BLOBNBOX::DeleteNoiseBlobs(&blobs);
  BLOBNBOX::DeleteNoiseBlobs(&small_blobs);
  BLOBNBOX::DeleteNoiseBlobs(&noise_blobs);
  BLOBNBOX::DeleteNoiseBlobs(&large_blobs);
}

// Computes and stores the edge offsets on each blob for use in feature
// extraction, using greyscale if the supplied grey and thresholds pixes
// are 8-bit or otherwise (if nullptr or not 8 bit) the original binary
// edge step outlines.
// Thresholds must either be the same size as grey or an integer down-scale
// of grey.
// See coutln.h for an explanation of edge offsets.
void TO_BLOCK::ComputeEdgeOffsets(Pix* thresholds, Pix* grey) {
  BLOBNBOX::ComputeEdgeOffsets(thresholds, grey, &blobs);
  BLOBNBOX::ComputeEdgeOffsets(thresholds, grey, &small_blobs);
  BLOBNBOX::ComputeEdgeOffsets(thresholds, grey, &noise_blobs);
}

#ifndef GRAPHICS_DISABLED
// Draw the noise blobs from all lists in red.
void TO_BLOCK::plot_noise_blobs(ScrollView* win) {
  BLOBNBOX::PlotNoiseBlobs(&noise_blobs, ScrollView::RED, ScrollView::RED, win);
  BLOBNBOX::PlotNoiseBlobs(&small_blobs, ScrollView::RED, ScrollView::RED, win);
  BLOBNBOX::PlotNoiseBlobs(&large_blobs, ScrollView::RED, ScrollView::RED, win);
  BLOBNBOX::PlotNoiseBlobs(&blobs, ScrollView::RED, ScrollView::RED, win);
}

// Draw the blobs on the various lists in the block in different colors.
void TO_BLOCK::plot_graded_blobs(ScrollView* win) {
  BLOBNBOX::PlotBlobs(&noise_blobs, ScrollView::CORAL, ScrollView::BLUE, win);
  BLOBNBOX::PlotBlobs(&small_blobs, ScrollView::GOLDENROD, ScrollView::YELLOW,
                      win);
  BLOBNBOX::PlotBlobs(&large_blobs, ScrollView::DARK_GREEN, ScrollView::YELLOW,
                      win);
  BLOBNBOX::PlotBlobs(&blobs, ScrollView::WHITE, ScrollView::BROWN, win);
}

/**********************************************************************
 * plot_blob_list
 *
 * Draw a list of blobs.
 **********************************************************************/

void plot_blob_list(ScrollView* win,                   // window to draw in
                    BLOBNBOX_LIST *list,               // blob list
                    ScrollView::Color body_colour,     // colour to draw
                    ScrollView::Color child_colour) {  // colour of child
  BLOBNBOX_IT it = list;
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    it.data()->plot(win, body_colour, child_colour);
  }
}
#endif  // GRAPHICS_DISABLED
