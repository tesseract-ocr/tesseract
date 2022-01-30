/******************************************************************************
 *
 * File:         blobs.cpp  (Formerly blobs.c)
 * Description:  Blob definition
 * Author:       Mark Seaman, OCR Technology
 *
 * (c) Copyright 1989, Hewlett-Packard Company.
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
 *****************************************************************************/

/*----------------------------------------------------------------------
              I n c l u d e s
----------------------------------------------------------------------*/
// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#  include "config_auto.h"
#endif

#include "blobs.h"

#include "ccstruct.h"
#include "clst.h"
#include "linlsq.h"
#include "normalis.h"
#include "ocrblock.h"
#include "ocrrow.h"
#include "points.h"
#include "polyaprx.h"
#include "werd.h"

#include "helpers.h"

#include <algorithm>

namespace tesseract {

// A Vector representing the "vertical" direction when measuring the
// divisiblity of blobs into multiple blobs just by separating outlines.
// See divisible_blob below for the use.
const TPOINT kDivisibleVerticalUpright(0, 1);
// A vector representing the "vertical" direction for italic text for use
// when separating outlines. Using it actually deteriorates final accuracy,
// so it is only used for ApplyBoxes chopping to get a better segmentation.
const TPOINT kDivisibleVerticalItalic(1, 5);

/*----------------------------------------------------------------------
              F u n c t i o n s
----------------------------------------------------------------------*/

// Returns true when the two line segments cross each other.
// (Moved from outlines.cpp).
// Finds where the projected lines would cross and then checks to see if the
// point of intersection lies on both of the line segments. If it does
// then these two segments cross.
/* static */
bool TPOINT::IsCrossed(const TPOINT &a0, const TPOINT &a1, const TPOINT &b0, const TPOINT &b1) {
  TPOINT b0a1, b0a0, a1b1, b0b1, a1a0;

  b0a1.x = a1.x - b0.x;
  b0a0.x = a0.x - b0.x;
  a1b1.x = b1.x - a1.x;
  b0b1.x = b1.x - b0.x;
  a1a0.x = a0.x - a1.x;
  b0a1.y = a1.y - b0.y;
  b0a0.y = a0.y - b0.y;
  a1b1.y = b1.y - a1.y;
  b0b1.y = b1.y - b0.y;
  a1a0.y = a0.y - a1.y;

  int b0a1xb0b1 = b0a1.cross(b0b1);
  int b0b1xb0a0 = b0b1.cross(b0a0);
  int a1b1xa1a0 = a1b1.cross(a1a0);
  // For clarity, we want a1a0.cross(a1b0) here but we have b0a1 instead of a1b0
  // so use -a1b0.cross(b0a1) instead, which is the same.
  int a1a0xa1b0 = -a1a0.cross(b0a1);

  return ((b0a1xb0b1 > 0 && b0b1xb0a0 > 0) || (b0a1xb0b1 < 0 && b0b1xb0a0 < 0)) &&
         ((a1b1xa1a0 > 0 && a1a0xa1b0 > 0) || (a1b1xa1a0 < 0 && a1a0xa1b0 < 0));
}

// Consume the circular list of EDGEPTs to make a TESSLINE.
TESSLINE *TESSLINE::BuildFromOutlineList(EDGEPT *outline) {
  auto *result = new TESSLINE;
  result->loop = outline;
  if (outline->src_outline != nullptr) {
    // ASSUMPTION: This function is only ever called from ApproximateOutline
    // and therefore either all points have a src_outline or all do not.
    // Just as SetupFromPos sets the vectors from the vertices, setup the
    // step_count members to indicate the (positive) number of original
    // C_OUTLINE steps to the next vertex.
    EDGEPT *pt = outline;
    do {
      pt->step_count = pt->next->start_step - pt->start_step;
      if (pt->step_count < 0) {
        pt->step_count += pt->src_outline->pathlength();
      }
      pt = pt->next;
    } while (pt != outline);
  }
  result->SetupFromPos();
  return result;
}

// Copies the data and the outline, but leaves next untouched.
void TESSLINE::CopyFrom(const TESSLINE &src) {
  Clear();
  topleft = src.topleft;
  botright = src.botright;
  start = src.start;
  is_hole = src.is_hole;
  if (src.loop != nullptr) {
    EDGEPT *prevpt = nullptr;
    EDGEPT *newpt = nullptr;
    EDGEPT *srcpt = src.loop;
    do {
      newpt = new EDGEPT(*srcpt);
      if (prevpt == nullptr) {
        loop = newpt;
      } else {
        newpt->prev = prevpt;
        prevpt->next = newpt;
      }
      prevpt = newpt;
      srcpt = srcpt->next;
    } while (srcpt != src.loop);
    loop->prev = newpt;
    newpt->next = loop;
  }
}

// Deletes owned data.
void TESSLINE::Clear() {
  if (loop == nullptr) {
    return;
  }

  EDGEPT *this_edge = loop;
  do {
    EDGEPT *next_edge = this_edge->next;
    delete this_edge;
    this_edge = next_edge;
  } while (this_edge != loop);
  loop = nullptr;
}

// Normalize in-place using the DENORM.
void TESSLINE::Normalize(const DENORM &denorm) {
  EDGEPT *pt = loop;
  do {
    denorm.LocalNormTransform(pt->pos, &pt->pos);
    pt = pt->next;
  } while (pt != loop);
  SetupFromPos();
}

// Rotates by the given rotation in place.
void TESSLINE::Rotate(const FCOORD rot) {
  EDGEPT *pt = loop;
  do {
    int tmp = static_cast<int>(floor(pt->pos.x * rot.x() - pt->pos.y * rot.y() + 0.5));
    pt->pos.y = static_cast<int>(floor(pt->pos.y * rot.x() + pt->pos.x * rot.y() + 0.5));
    pt->pos.x = tmp;
    pt = pt->next;
  } while (pt != loop);
  SetupFromPos();
}

// Moves by the given vec in place.
void TESSLINE::Move(const ICOORD vec) {
  EDGEPT *pt = loop;
  do {
    pt->pos.x += vec.x();
    pt->pos.y += vec.y();
    pt = pt->next;
  } while (pt != loop);
  SetupFromPos();
}

// Scales by the given factor in place.
void TESSLINE::Scale(float factor) {
  EDGEPT *pt = loop;
  do {
    pt->pos.x = static_cast<int>(floor(pt->pos.x * factor + 0.5));
    pt->pos.y = static_cast<int>(floor(pt->pos.y * factor + 0.5));
    pt = pt->next;
  } while (pt != loop);
  SetupFromPos();
}

// Sets up the start and vec members of the loop from the pos members.
void TESSLINE::SetupFromPos() {
  EDGEPT *pt = loop;
  do {
    pt->vec.x = pt->next->pos.x - pt->pos.x;
    pt->vec.y = pt->next->pos.y - pt->pos.y;
    pt = pt->next;
  } while (pt != loop);
  start = pt->pos;
  ComputeBoundingBox();
}

// Recomputes the bounding box from the points in the loop.
void TESSLINE::ComputeBoundingBox() {
  int minx = INT32_MAX;
  int miny = INT32_MAX;
  int maxx = -INT32_MAX;
  int maxy = -INT32_MAX;

  // Find boundaries.
  start = loop->pos;
  EDGEPT *this_edge = loop;
  do {
    if (!this_edge->IsHidden() || !this_edge->prev->IsHidden()) {
      if (this_edge->pos.x < minx) {
        minx = this_edge->pos.x;
      }
      if (this_edge->pos.y < miny) {
        miny = this_edge->pos.y;
      }
      if (this_edge->pos.x > maxx) {
        maxx = this_edge->pos.x;
      }
      if (this_edge->pos.y > maxy) {
        maxy = this_edge->pos.y;
      }
    }
    this_edge = this_edge->next;
  } while (this_edge != loop);
  // Reset bounds.
  topleft.x = minx;
  topleft.y = maxy;
  botright.x = maxx;
  botright.y = miny;
}

// Computes the min and max cross product of the outline points with the
// given vec and returns the results in min_xp and max_xp. Geometrically
// this is the left and right edge of the outline perpendicular to the
// given direction, but to get the distance units correct, you would
// have to divide by the modulus of vec.
void TESSLINE::MinMaxCrossProduct(const TPOINT vec, int *min_xp, int *max_xp) const {
  *min_xp = INT32_MAX;
  *max_xp = INT32_MIN;
  EDGEPT *this_edge = loop;
  do {
    if (!this_edge->IsHidden() || !this_edge->prev->IsHidden()) {
      int product = this_edge->pos.cross(vec);
      UpdateRange(product, min_xp, max_xp);
    }
    this_edge = this_edge->next;
  } while (this_edge != loop);
}

TBOX TESSLINE::bounding_box() const {
  return TBOX(topleft.x, botright.y, botright.x, topleft.y);
}

#ifndef GRAPHICS_DISABLED
void TESSLINE::plot(ScrollView *window, ScrollView::Color color, ScrollView::Color child_color) {
  if (is_hole) {
    window->Pen(child_color);
  } else {
    window->Pen(color);
  }
  window->SetCursor(start.x, start.y);
  EDGEPT *pt = loop;
  do {
    bool prev_hidden = pt->IsHidden();
    pt = pt->next;
    if (prev_hidden) {
      window->SetCursor(pt->pos.x, pt->pos.y);
    } else {
      window->DrawTo(pt->pos.x, pt->pos.y);
    }
  } while (pt != loop);
}
#endif // !GRAPHICS_DISABLED

// Returns the first non-hidden EDGEPT that has a different src_outline to
// its predecessor, or, if all the same, the lowest indexed point.
EDGEPT *TESSLINE::FindBestStartPt() const {
  EDGEPT *best_start = loop;
  int best_step = loop->start_step;
  // Iterate the polygon.
  EDGEPT *pt = loop;
  do {
    if (pt->IsHidden()) {
      continue;
    }
    if (pt->prev->IsHidden() || pt->prev->src_outline != pt->src_outline) {
      return pt; // Qualifies as the best.
    }
    if (pt->start_step < best_step) {
      best_step = pt->start_step;
      best_start = pt;
    }
  } while ((pt = pt->next) != loop);
  return best_start;
}

// Iterate the given list of outlines, converting to TESSLINE by polygonal
// approximation and recursively any children, returning the current tail
// of the resulting list of TESSLINEs.
static TESSLINE **ApproximateOutlineList(bool allow_detailed_fx, C_OUTLINE_LIST *outlines,
                                         bool children, TESSLINE **tail) {
  C_OUTLINE_IT ol_it(outlines);
  for (ol_it.mark_cycle_pt(); !ol_it.cycled_list(); ol_it.forward()) {
    C_OUTLINE *outline = ol_it.data();
    if (outline->pathlength() > 0) {
      TESSLINE *tessline = ApproximateOutline(allow_detailed_fx, outline);
      tessline->is_hole = children;
      *tail = tessline;
      tail = &tessline->next;
    }
    if (!outline->child()->empty()) {
      tail = ApproximateOutlineList(allow_detailed_fx, outline->child(), true, tail);
    }
  }
  return tail;
}

// Factory to build a TBLOB from a C_BLOB with polygonal approximation along
// the way. If allow_detailed_fx is true, the EDGEPTs in the returned TBLOB
// contain pointers to the input C_OUTLINEs that enable higher-resolution
// feature extraction that does not use the polygonal approximation.
TBLOB *TBLOB::PolygonalCopy(bool allow_detailed_fx, C_BLOB *src) {
  auto *tblob = new TBLOB;
  ApproximateOutlineList(allow_detailed_fx, src->out_list(), false, &tblob->outlines);
  return tblob;
}

// Factory builds a blob with no outlines, but copies the other member data.
TBLOB *TBLOB::ShallowCopy(const TBLOB &src) {
  auto *blob = new TBLOB;
  blob->denorm_ = src.denorm_;
  return blob;
}

// Normalizes the blob for classification only if needed.
// (Normally this means a non-zero classify rotation.)
// If no Normalization is needed, then nullptr is returned, and the input blob
// can be used directly. Otherwise a new TBLOB is returned which must be
// deleted after use.
TBLOB *TBLOB::ClassifyNormalizeIfNeeded() const {
  TBLOB *rotated_blob = nullptr;
  // If necessary, copy the blob and rotate it. The rotation is always
  // +/- 90 degrees, as 180 was already taken care of.
  if (denorm_.block() != nullptr && denorm_.block()->classify_rotation().y() != 0.0) {
    TBOX box = bounding_box();
    int x_middle = (box.left() + box.right()) / 2;
    int y_middle = (box.top() + box.bottom()) / 2;
    rotated_blob = new TBLOB(*this);
    const FCOORD &rotation = denorm_.block()->classify_rotation();
    // Move the rotated blob back to the same y-position so that we
    // can still distinguish similar glyphs with differeny y-position.
    float target_y =
        kBlnBaselineOffset + (rotation.y() > 0 ? x_middle - box.left() : box.right() - x_middle);
    rotated_blob->Normalize(nullptr, &rotation, &denorm_, x_middle, y_middle, 1.0f, 1.0f, 0.0f,
                            target_y, denorm_.inverse(), denorm_.pix());
  }
  return rotated_blob;
}

// Copies the data and the outline, but leaves next untouched.
void TBLOB::CopyFrom(const TBLOB &src) {
  Clear();
  TESSLINE *prev_outline = nullptr;
  for (TESSLINE *srcline = src.outlines; srcline != nullptr; srcline = srcline->next) {
    auto *new_outline = new TESSLINE(*srcline);
    if (outlines == nullptr) {
      outlines = new_outline;
    } else {
      prev_outline->next = new_outline;
    }
    prev_outline = new_outline;
  }
  denorm_ = src.denorm_;
}

// Deletes owned data.
void TBLOB::Clear() {
  for (TESSLINE *next_outline = nullptr; outlines != nullptr; outlines = next_outline) {
    next_outline = outlines->next;
    delete outlines;
  }
}

// Sets up the built-in DENORM and normalizes the blob in-place.
// For parameters see DENORM::SetupNormalization, plus the inverse flag for
// this blob and the Pix for the full image.
void TBLOB::Normalize(const BLOCK *block, const FCOORD *rotation, const DENORM *predecessor,
                      float x_origin, float y_origin, float x_scale, float y_scale,
                      float final_xshift, float final_yshift, bool inverse, Image pix) {
  denorm_.SetupNormalization(block, rotation, predecessor, x_origin, y_origin, x_scale, y_scale,
                             final_xshift, final_yshift);
  denorm_.set_inverse(inverse);
  denorm_.set_pix(pix);
  // TODO(rays) outline->Normalize is more accurate, but breaks tests due
  // the changes it makes. Reinstate this code with a retraining.
  // The reason this change is troublesome is that it normalizes for the
  // baseline value computed independently at each x-coord. If the baseline
  // is not horizontal, this introduces shear into the normalized blob, which
  // is useful on the rare occasions that the baseline is really curved, but
  // the baselines need to be stabilized the rest of the time.
#if 0
  for (TESSLINE* outline = outlines; outline != nullptr; outline = outline->next) {
    outline->Normalize(denorm_);
  }
#else
  denorm_.LocalNormBlob(this);
#endif
}

// Rotates by the given rotation in place.
void TBLOB::Rotate(const FCOORD rotation) {
  for (TESSLINE *outline = outlines; outline != nullptr; outline = outline->next) {
    outline->Rotate(rotation);
  }
}

// Moves by the given vec in place.
void TBLOB::Move(const ICOORD vec) {
  for (TESSLINE *outline = outlines; outline != nullptr; outline = outline->next) {
    outline->Move(vec);
  }
}

// Scales by the given factor in place.
void TBLOB::Scale(float factor) {
  for (TESSLINE *outline = outlines; outline != nullptr; outline = outline->next) {
    outline->Scale(factor);
  }
}

// Recomputes the bounding boxes of the outlines.
void TBLOB::ComputeBoundingBoxes() {
  for (TESSLINE *outline = outlines; outline != nullptr; outline = outline->next) {
    outline->ComputeBoundingBox();
  }
}

// Returns the number of outlines.
int TBLOB::NumOutlines() const {
  int result = 0;
  for (TESSLINE *outline = outlines; outline != nullptr; outline = outline->next) {
    ++result;
  }
  return result;
}

/**********************************************************************
 * TBLOB::bounding_box()
 *
 * Compute the bounding_box of a compound blob, defined to be the
 * bounding box of the union of all top-level outlines in the blob.
 **********************************************************************/
TBOX TBLOB::bounding_box() const {
  if (outlines == nullptr) {
    return TBOX(0, 0, 0, 0);
  }
  TESSLINE *outline = outlines;
  TBOX box = outline->bounding_box();
  for (outline = outline->next; outline != nullptr; outline = outline->next) {
    box += outline->bounding_box();
  }
  return box;
}

// Finds and deletes any duplicate outlines in this blob, without deleting
// their EDGEPTs.
void TBLOB::EliminateDuplicateOutlines() {
  for (TESSLINE *outline = outlines; outline != nullptr; outline = outline->next) {
    TESSLINE *last_outline = outline;
    for (TESSLINE *other_outline = outline->next; other_outline != nullptr;
         last_outline = other_outline, other_outline = other_outline->next) {
      if (outline->SameBox(*other_outline)) {
        last_outline->next = other_outline->next;
        // This doesn't leak - the outlines share the EDGEPTs.
        other_outline->loop = nullptr;
        delete other_outline;
        other_outline = last_outline;
        // If it is part of a cut, then it can't be a hole any more.
        outline->is_hole = false;
      }
    }
  }
}

// Swaps the outlines of *this and next if needed to keep the centers in
// increasing x.
void TBLOB::CorrectBlobOrder(TBLOB *next) {
  TBOX box = bounding_box();
  TBOX next_box = next->bounding_box();
  if (box.x_middle() > next_box.x_middle()) {
    std::swap(outlines, next->outlines);
  }
}

#ifndef GRAPHICS_DISABLED
void TBLOB::plot(ScrollView *window, ScrollView::Color color, ScrollView::Color child_color) {
  for (TESSLINE *outline = outlines; outline != nullptr; outline = outline->next) {
    outline->plot(window, color, child_color);
  }
}
#endif // !GRAPHICS_DISABLED

// Computes the center of mass and second moments for the old baseline and
// 2nd moment normalizations. Returns the outline length.
// The input denorm should be the normalizations that have been applied from
// the image to the current state of this TBLOB.
int TBLOB::ComputeMoments(FCOORD *center, FCOORD *second_moments) const {
  // Compute 1st and 2nd moments of the original outline.
  LLSQ accumulator;
  TBOX box = bounding_box();
  // Iterate the outlines, accumulating edges relative the box.botleft().
  CollectEdges(box, nullptr, &accumulator, nullptr, nullptr);
  *center = accumulator.mean_point() + box.botleft();
  // The 2nd moments are just the standard deviation of the point positions.
  double x2nd = sqrt(accumulator.x_variance());
  double y2nd = sqrt(accumulator.y_variance());
  if (x2nd < 1.0) {
    x2nd = 1.0;
  }
  if (y2nd < 1.0) {
    y2nd = 1.0;
  }
  second_moments->set_x(x2nd);
  second_moments->set_y(y2nd);
  return accumulator.count();
}

// Computes the precise bounding box of the coords that are generated by
// GetEdgeCoords. This may be different from the bounding box of the polygon.
void TBLOB::GetPreciseBoundingBox(TBOX *precise_box) const {
  TBOX box = bounding_box();
  *precise_box = TBOX();
  CollectEdges(box, precise_box, nullptr, nullptr, nullptr);
  precise_box->move(box.botleft());
}

// Adds edges to the given vectors.
// For all the edge steps in all the outlines, or polygonal approximation
// where there are no edge steps, collects the steps into x_coords/y_coords.
// x_coords is a collection of the x-coords of vertical edges for each
// y-coord starting at box.bottom().
// y_coords is a collection of the y-coords of horizontal edges for each
// x-coord starting at box.left().
// Eg x_coords[0] is a collection of the x-coords of edges at y=bottom.
// Eg x_coords[1] is a collection of the x-coords of edges at y=bottom + 1.
void TBLOB::GetEdgeCoords(const TBOX &box, std::vector<std::vector<int>> &x_coords,
                          std::vector<std::vector<int>> &y_coords) const {
  x_coords.clear();
  x_coords.resize(box.height());
  y_coords.clear();
  y_coords.resize(box.width());
  CollectEdges(box, nullptr, nullptr, &x_coords, &y_coords);
  // Sort the output vectors.
  for (auto &coord : x_coords) {
    std::sort(coord.begin(), coord.end());
  }
  for (auto &coord : y_coords) {
    std::sort(coord.begin(), coord.end());
  }
}

// Accumulates the segment between pt1 and pt2 in the LLSQ, quantizing over
// the integer coordinate grid to properly weight long vectors.
static void SegmentLLSQ(const FCOORD &pt1, const FCOORD &pt2, LLSQ *accumulator) {
  FCOORD step(pt2);
  step -= pt1;
  int xstart = IntCastRounded(std::min(pt1.x(), pt2.x()));
  int xend = IntCastRounded(std::max(pt1.x(), pt2.x()));
  int ystart = IntCastRounded(std::min(pt1.y(), pt2.y()));
  int yend = IntCastRounded(std::max(pt1.y(), pt2.y()));
  if (xstart == xend && ystart == yend) {
    return; // Nothing to do.
  }
  double weight = step.length() / (xend - xstart + yend - ystart);
  // Compute and save the y-position at the middle of each x-step.
  for (int x = xstart; x < xend; ++x) {
    double y = pt1.y() + step.y() * (x + 0.5 - pt1.x()) / step.x();
    accumulator->add(x + 0.5, y, weight);
  }
  // Compute and save the x-position at the middle of each y-step.
  for (int y = ystart; y < yend; ++y) {
    double x = pt1.x() + step.x() * (y + 0.5 - pt1.y()) / step.y();
    accumulator->add(x, y + 0.5, weight);
  }
}

// Adds any edges from a single segment of outline between pt1 and pt2 to
// the x_coords, y_coords vectors. pt1 and pt2 should be relative to the
// bottom-left of the bounding box, hence indices to x_coords, y_coords
// are clipped to ([0,x_limit], [0,y_limit]).
// See GetEdgeCoords above for a description of x_coords, y_coords.
static void SegmentCoords(const FCOORD &pt1, const FCOORD &pt2, int x_limit, int y_limit,
                          std::vector<std::vector<int>> *x_coords,
                          std::vector<std::vector<int>> *y_coords) {
  FCOORD step(pt2);
  step -= pt1;
  int start = ClipToRange(IntCastRounded(std::min(pt1.x(), pt2.x())), 0, x_limit);
  int end = ClipToRange(IntCastRounded(std::max(pt1.x(), pt2.x())), 0, x_limit);
  for (int x = start; x < end; ++x) {
    int y = IntCastRounded(pt1.y() + step.y() * (x + 0.5 - pt1.x()) / step.x());
    (*y_coords)[x].push_back(y);
  }
  start = ClipToRange(IntCastRounded(std::min(pt1.y(), pt2.y())), 0, y_limit);
  end = ClipToRange(IntCastRounded(std::max(pt1.y(), pt2.y())), 0, y_limit);
  for (int y = start; y < end; ++y) {
    int x = IntCastRounded(pt1.x() + step.x() * (y + 0.5 - pt1.y()) / step.y());
    (*x_coords)[y].push_back(x);
  }
}

// Adds any edges from a single segment of outline between pt1 and pt2 to
// the bbox such that it guarantees to contain anything produced by
// SegmentCoords.
static void SegmentBBox(const FCOORD &pt1, const FCOORD &pt2, TBOX *bbox) {
  FCOORD step(pt2);
  step -= pt1;
  int x1 = IntCastRounded(std::min(pt1.x(), pt2.x()));
  int x2 = IntCastRounded(std::max(pt1.x(), pt2.x()));
  if (x2 > x1) {
    int y1 = IntCastRounded(pt1.y() + step.y() * (x1 + 0.5 - pt1.x()) / step.x());
    int y2 = IntCastRounded(pt1.y() + step.y() * (x2 - 0.5 - pt1.x()) / step.x());
    TBOX point(x1, std::min(y1, y2), x2, std::max(y1, y2));
    *bbox += point;
  }
  int y1 = IntCastRounded(std::min(pt1.y(), pt2.y()));
  int y2 = IntCastRounded(std::max(pt1.y(), pt2.y()));
  if (y2 > y1) {
    int x1 = IntCastRounded(pt1.x() + step.x() * (y1 + 0.5 - pt1.y()) / step.y());
    int x2 = IntCastRounded(pt1.x() + step.x() * (y2 - 0.5 - pt1.y()) / step.y());
    TBOX point(std::min(x1, x2), y1, std::max(x1, x2), y2);
    *bbox += point;
  }
}

// Collects edges into the given bounding box, LLSQ accumulator and/or x_coords,
// y_coords vectors.
// For a description of x_coords/y_coords, see GetEdgeCoords above.
// Startpt to lastpt, inclusive, MUST have the same src_outline member,
// which may be nullptr. The vector from lastpt to its next is included in
// the accumulation. Hidden edges should be excluded by the caller.
// The input denorm should be the normalizations that have been applied from
// the image to the current state of the TBLOB from which startpt, lastpt come.
// box is the bounding box of the blob from which the EDGEPTs are taken and
// indices into x_coords, y_coords are offset by box.botleft().
static void CollectEdgesOfRun(const EDGEPT *startpt, const EDGEPT *lastpt, const DENORM &denorm,
                              const TBOX &box, TBOX *bounding_box, LLSQ *accumulator,
                              std::vector<std::vector<int>> *x_coords,
                              std::vector<std::vector<int>> *y_coords) {
  const C_OUTLINE *outline = startpt->src_outline;
  int x_limit = box.width() - 1;
  int y_limit = box.height() - 1;
  if (outline != nullptr) {
    // Use higher-resolution edge points stored on the outline.
    // The outline coordinates may not match the binary image because of the
    // rotation for vertical text lines, but the root_denorm IS the matching
    // start of the DENORM chain.
    const DENORM *root_denorm = denorm.RootDenorm();
    int step_length = outline->pathlength();
    int start_index = startpt->start_step;
    // Note that if this run straddles the wrap-around point of the outline,
    // that lastpt->start_step may have a lower index than startpt->start_step,
    // and we want to use an end_index that allows us to use a positive
    // increment, so we add step_length if necessary, but that may be beyond the
    // bounds of the outline steps/ due to wrap-around, so we use % step_length
    // everywhere, except for start_index.
    int end_index = lastpt->start_step + lastpt->step_count;
    if (end_index <= start_index) {
      end_index += step_length;
    }
    // pos is the integer coordinates of the binary image steps.
    ICOORD pos = outline->position_at_index(start_index);
    FCOORD origin(box.left(), box.bottom());
    // f_pos is a floating-point version of pos that offers improved edge
    // positioning using greyscale information or smoothing of edge steps.
    FCOORD f_pos = outline->sub_pixel_pos_at_index(pos, start_index);
    // pos_normed is f_pos after the appropriate normalization, and relative
    // to origin.
    // prev_normed is the previous value of pos_normed.
    FCOORD prev_normed;
    denorm.NormTransform(root_denorm, f_pos, &prev_normed);
    prev_normed -= origin;
    for (int index = start_index; index < end_index; ++index) {
      ICOORD step = outline->step(index % step_length);
      // Only use the point if its edge strength is positive. This excludes
      // points that don't provide useful information, eg
      // ___________
      //            |___________
      // The vertical step provides only noisy, damaging information, as even
      // with a greyscale image, the positioning of the edge there may be a
      // fictitious extrapolation, so previous processing has eliminated it.
      if (outline->edge_strength_at_index(index % step_length) > 0) {
        FCOORD f_pos = outline->sub_pixel_pos_at_index(pos, index % step_length);
        FCOORD pos_normed;
        denorm.NormTransform(root_denorm, f_pos, &pos_normed);
        pos_normed -= origin;
        // Accumulate the information that is selected by the caller.
        if (bounding_box != nullptr) {
          SegmentBBox(pos_normed, prev_normed, bounding_box);
        }
        if (accumulator != nullptr) {
          SegmentLLSQ(pos_normed, prev_normed, accumulator);
        }
        if (x_coords != nullptr && y_coords != nullptr) {
          SegmentCoords(pos_normed, prev_normed, x_limit, y_limit, x_coords, y_coords);
        }
        prev_normed = pos_normed;
      }
      pos += step;
    }
  } else {
    // There is no outline, so we are forced to use the polygonal approximation.
    const EDGEPT *endpt = lastpt->next;
    const EDGEPT *pt = startpt;
    do {
      FCOORD next_pos(pt->next->pos.x - box.left(), pt->next->pos.y - box.bottom());
      FCOORD pos(pt->pos.x - box.left(), pt->pos.y - box.bottom());
      if (bounding_box != nullptr) {
        SegmentBBox(next_pos, pos, bounding_box);
      }
      if (accumulator != nullptr) {
        SegmentLLSQ(next_pos, pos, accumulator);
      }
      if (x_coords != nullptr && y_coords != nullptr) {
        SegmentCoords(next_pos, pos, x_limit, y_limit, x_coords, y_coords);
      }
    } while ((pt = pt->next) != endpt);
  }
}

// For all the edge steps in all the outlines, or polygonal approximation
// where there are no edge steps, collects the steps into the bounding_box,
// llsq and/or the x_coords/y_coords. Both are used in different kinds of
// normalization.
// For a description of x_coords, y_coords, see GetEdgeCoords above.
void TBLOB::CollectEdges(const TBOX &box, TBOX *bounding_box, LLSQ *llsq,
                         std::vector<std::vector<int>> *x_coords,
                         std::vector<std::vector<int>> *y_coords) const {
  // Iterate the outlines.
  for (const TESSLINE *ol = outlines; ol != nullptr; ol = ol->next) {
    // Iterate the polygon.
    EDGEPT *loop_pt = ol->FindBestStartPt();
    EDGEPT *pt = loop_pt;
    if (pt == nullptr) {
      continue;
    }
    do {
      if (pt->IsHidden()) {
        continue;
      }
      // Find a run of equal src_outline.
      EDGEPT *last_pt = pt;
      do {
        last_pt = last_pt->next;
      } while (last_pt != loop_pt && !last_pt->IsHidden() &&
               last_pt->src_outline == pt->src_outline);
      last_pt = last_pt->prev;
      CollectEdgesOfRun(pt, last_pt, denorm_, box, bounding_box, llsq, x_coords, y_coords);
      pt = last_pt;
    } while ((pt = pt->next) != loop_pt);
  }
}

// Factory to build a TWERD from a (C_BLOB) WERD, with polygonal
// approximation along the way.
TWERD *TWERD::PolygonalCopy(bool allow_detailed_fx, WERD *src) {
  auto *tessword = new TWERD;
  tessword->latin_script = src->flag(W_SCRIPT_IS_LATIN);
  C_BLOB_IT b_it(src->cblob_list());
  for (b_it.mark_cycle_pt(); !b_it.cycled_list(); b_it.forward()) {
    C_BLOB *blob = b_it.data();
    TBLOB *tblob = TBLOB::PolygonalCopy(allow_detailed_fx, blob);
    tessword->blobs.push_back(tblob);
  }
  return tessword;
}

// Baseline normalizes the blobs in-place, recording the normalization in the
// DENORMs in the blobs.
void TWERD::BLNormalize(const BLOCK *block, const ROW *row, Image pix, bool inverse, float x_height,
                        float baseline_shift, bool numeric_mode, tesseract::OcrEngineMode hint,
                        const TBOX *norm_box, DENORM *word_denorm) {
  TBOX word_box = bounding_box();
  if (norm_box != nullptr) {
    word_box = *norm_box;
  }
  float word_middle = (word_box.left() + word_box.right()) / 2.0f;
  float input_y_offset = 0.0f;
  auto final_y_offset = static_cast<float>(kBlnBaselineOffset);
  float scale = kBlnXHeight / x_height;
  if (row == nullptr) {
    word_middle = word_box.left();
    input_y_offset = word_box.bottom();
    final_y_offset = 0.0f;
  } else {
    input_y_offset = row->base_line(word_middle) + baseline_shift;
  }
  for (auto blob : blobs) {
    TBOX blob_box = blob->bounding_box();
    float mid_x = (blob_box.left() + blob_box.right()) / 2.0f;
    float baseline = input_y_offset;
    float blob_scale = scale;
    if (numeric_mode) {
      baseline = blob_box.bottom();
      blob_scale = ClipToRange(kBlnXHeight * 4.0f / (3 * blob_box.height()), scale, scale * 1.5f);
    } else if (row != nullptr) {
      baseline = row->base_line(mid_x) + baseline_shift;
    }
    // The image will be 8-bit grey if the input was grey or color. Note that in
    // a grey image 0 is black and 255 is white. If the input was binary, then
    // the pix will be binary and 0 is white, with 1 being black.
    // To tell the difference pixGetDepth() will return 8 or 1.
    // The inverse flag will be true iff the word has been determined to be
    // white on black, and is independent of whether the pix is 8 bit or 1 bit.
    blob->Normalize(block, nullptr, nullptr, word_middle, baseline, blob_scale, blob_scale, 0.0f,
                    final_y_offset, inverse, pix);
  }
  if (word_denorm != nullptr) {
    word_denorm->SetupNormalization(block, nullptr, nullptr, word_middle, input_y_offset, scale,
                                    scale, 0.0f, final_y_offset);
    word_denorm->set_inverse(inverse);
    word_denorm->set_pix(pix);
  }
}

// Copies the data and the blobs, but leaves next untouched.
void TWERD::CopyFrom(const TWERD &src) {
  Clear();
  latin_script = src.latin_script;
  for (auto blob : src.blobs) {
    auto *new_blob = new TBLOB(*blob);
    blobs.push_back(new_blob);
  }
}

// Deletes owned data.
void TWERD::Clear() {
  for (auto blob : blobs) {
    delete blob;
  }
  blobs.clear();
}

// Recomputes the bounding boxes of the blobs.
void TWERD::ComputeBoundingBoxes() {
  for (auto &blob : blobs) {
    blob->ComputeBoundingBoxes();
  }
}

TBOX TWERD::bounding_box() const {
  TBOX result;
  for (auto blob : blobs) {
    TBOX box = blob->bounding_box();
    result += box;
  }
  return result;
}

// Merges the blobs from start to end, not including end, and deletes
// the blobs between start and end.
void TWERD::MergeBlobs(unsigned start, unsigned end) {
  if (end > blobs.size()) {
    end = blobs.size();
  }
  if (start >= end) {
    return; // Nothing to do.
  }
  TESSLINE *outline = blobs[start]->outlines;
  for (auto i = start + 1; i < end; ++i) {
    TBLOB *next_blob = blobs[i];
    // Take the outlines from the next blob.
    if (outline == nullptr) {
      blobs[start]->outlines = next_blob->outlines;
      outline = blobs[start]->outlines;
    } else {
      while (outline->next != nullptr) {
        outline = outline->next;
      }
      outline->next = next_blob->outlines;
      next_blob->outlines = nullptr;
    }
    // Delete the next blob and move on.
    delete next_blob;
    blobs[i] = nullptr;
  }
  // Remove dead blobs from the vector.
  // TODO: optimize.
  for (auto i = start + 1; i < end && start + 1 < blobs.size(); ++i) {
    blobs.erase(blobs.begin() + start + 1);
  }
}

#ifndef GRAPHICS_DISABLED
void TWERD::plot(ScrollView *window) {
  ScrollView::Color color = WERD::NextColor(ScrollView::BLACK);
  for (auto &blob : blobs) {
    blob->plot(window, color, ScrollView::BROWN);
    color = WERD::NextColor(color);
  }
}
#endif // !GRAPHICS_DISABLED

/**********************************************************************
 * divisible_blob
 *
 * Returns true if the blob contains multiple outlines than can be
 * separated using divide_blobs. Sets the location to be used in the
 * call to divide_blobs.
 **********************************************************************/
bool divisible_blob(TBLOB *blob, bool italic_blob, TPOINT *location) {
  if (blob->outlines == nullptr || blob->outlines->next == nullptr) {
    return false; // Need at least 2 outlines for it to be possible.
  }
  int max_gap = 0;
  TPOINT vertical = italic_blob ? kDivisibleVerticalItalic : kDivisibleVerticalUpright;
  for (TESSLINE *outline1 = blob->outlines; outline1 != nullptr; outline1 = outline1->next) {
    if (outline1->is_hole) {
      continue; // Holes do not count as separable.
    }
    TPOINT mid_pt1((outline1->topleft.x + outline1->botright.x) / 2,
                   (outline1->topleft.y + outline1->botright.y) / 2);
    int mid_prod1 = mid_pt1.cross(vertical);
    int min_prod1, max_prod1;
    outline1->MinMaxCrossProduct(vertical, &min_prod1, &max_prod1);
    for (TESSLINE *outline2 = outline1->next; outline2 != nullptr; outline2 = outline2->next) {
      if (outline2->is_hole) {
        continue; // Holes do not count as separable.
      }
      TPOINT mid_pt2((outline2->topleft.x + outline2->botright.x) / 2,
                     (outline2->topleft.y + outline2->botright.y) / 2);
      int mid_prod2 = mid_pt2.cross(vertical);
      int min_prod2, max_prod2;
      outline2->MinMaxCrossProduct(vertical, &min_prod2, &max_prod2);
      int mid_gap = abs(mid_prod2 - mid_prod1);
      int overlap = std::min(max_prod1, max_prod2) - std::max(min_prod1, min_prod2);
      if (mid_gap - overlap / 4 > max_gap) {
        max_gap = mid_gap - overlap / 4;
        *location = mid_pt1;
        *location += mid_pt2;
        *location /= 2;
      }
    }
  }
  // Use the y component of the vertical vector as an approximation to its
  // length.
  return max_gap > vertical.y;
}

/**********************************************************************
 * divide_blobs
 *
 * Create two blobs by grouping the outlines in the appropriate blob.
 * The outlines that are beyond the location point are moved to the
 * other blob.  The ones whose x location is less than that point are
 * retained in the original blob.
 **********************************************************************/
void divide_blobs(TBLOB *blob, TBLOB *other_blob, bool italic_blob, const TPOINT &location) {
  TPOINT vertical = italic_blob ? kDivisibleVerticalItalic : kDivisibleVerticalUpright;
  TESSLINE *outline1 = nullptr;
  TESSLINE *outline2 = nullptr;

  TESSLINE *outline = blob->outlines;
  blob->outlines = nullptr;
  int location_prod = location.cross(vertical);

  while (outline != nullptr) {
    TPOINT mid_pt((outline->topleft.x + outline->botright.x) / 2,
                  (outline->topleft.y + outline->botright.y) / 2);
    int mid_prod = mid_pt.cross(vertical);
    if (mid_prod < location_prod) {
      // Outline is in left blob.
      if (outline1) {
        outline1->next = outline;
      } else {
        blob->outlines = outline;
      }
      outline1 = outline;
    } else {
      // Outline is in right blob.
      if (outline2) {
        outline2->next = outline;
      } else {
        other_blob->outlines = outline;
      }
      outline2 = outline;
    }
    outline = outline->next;
  }

  if (outline1) {
    outline1->next = nullptr;
  }
  if (outline2) {
    outline2->next = nullptr;
  }
}

} // namespace tesseract
