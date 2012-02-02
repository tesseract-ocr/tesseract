/**********************************************************************
 * File:        normalis.cpp  (Formerly denorm.c)
 * Description: Code for the DENORM class.
 * Author:      Ray Smith
 * Created:     Thu Apr 23 09:22:43 BST 1992
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
#include "mfcpch.h"  // Precompiled header include must be first.

#include "normalis.h"

#include <stdlib.h>

#include "allheaders.h"
#include "blobs.h"
#include "helpers.h"
#include "ocrblock.h"
#include "unicharset.h"
#include "werd.h"


DENORM::DENORM() {
  Init();
}

// TODO(rays) Abolish all non-default constructors.
DENORM::DENORM(float x, float scaling, ROW *src) {
  Init();
  x_origin_ = x;              // just copy
  y_origin_ = 0.0f;
  x_scale_ = y_scale_ = scaling;
  row_ = src;
}

DENORM::DENORM(float x,              // from same pieces
               float scaling,
               double line_m,        // default line: y = mx + c
               double line_c,
               inT16 seg_count,      // no of segments
               DENORM_SEG *seg_pts,  // actual segments
               BOOL8 using_row,      // as baseline
               ROW *src) {
  Init();
  x_origin_ = x;                  // just copy
  y_origin_ = line_c;
  ASSERT_HOST(line_m == 0.0);
  x_scale_ = y_scale_ = scaling;
  SetSegments(seg_pts, seg_count);
}


DENORM::DENORM(const DENORM &src) {
  num_segs_ = 0;
  segs_ = NULL;
  rotation_ = NULL;
  *this = src;
}


DENORM & DENORM::operator=(const DENORM & src) {
  Clear();
  inverse_ = src.inverse_;
  pix_ = src.pix_;
  block_ = src.block_;
  row_ = src.row_;
  if (src.rotation_ == NULL)
    rotation_ = NULL;
  else
    rotation_ = new FCOORD(*src.rotation_);
  predecessor_ = src.predecessor_;
  SetSegments(src.segs_, src.num_segs_);
  x_origin_ = src.x_origin_;
  y_origin_ = src.y_origin_;
  x_scale_ = src.x_scale_;
  y_scale_ = src.y_scale_;
  final_xshift_ = src.final_xshift_;
  final_yshift_ = src.final_yshift_;
  return *this;
}

DENORM::~DENORM() {
  Clear();
}

// Setup for a baseline normalization. If there are segs, then they
// are used, otherwise, if there is a row, that is used, otherwise the
// bottom of the word_box is used for the baseline.
void DENORM::SetupBLNormalize(const BLOCK* block, const ROW* row,
                              float x_height, const TBOX& word_box,
                              int num_segs, const DENORM_SEG* segs) {
  float scale = kBlnXHeight / x_height;
  float x_origin = (word_box.left() + word_box.right()) / 2.0f;
  float y_origin = 0.0f;
  if (num_segs == 0 && row == NULL) {
    y_origin = word_box.bottom();
  }
  SetupNormalization(block, row, NULL, NULL, segs, num_segs,
                     x_origin, y_origin, scale, scale,
                     0.0f, static_cast<float>(kBlnBaselineOffset));
}

// Initializes the denorm for a transformation. For details see the large
// comment in normalis.h.
// Arguments:
// block: if not NULL, then this is the first transformation, and
//        block->re_rotation() needs to be used after the Denorm
//        transformation to get back to the image coords.
// row: if not NULL, then row->baseline(x) is added to the y_origin, unless
//      segs is not NULL and num_segs > 0, in which case they are used.
// rotation: if not NULL, apply this rotation after translation to the
//           origin and scaling. (Usually a classify rotation.)
// predecessor: if not NULL, then predecessor has been applied to the
//              input space and needs to be undone to complete the inverse.
// segs: if not NULL and num_segs > 0, then the segs provide the y_origin
//       and the y_scale at a given source x.
// num_segs: the number of segs.
// The above pointers are not owned by this DENORM and are assumed to live
// longer than this denorm, except rotation, which is deep copied on input.
//
// x_origin: The x origin which will be mapped to final_xshift in the result.
// y_origin: The y origin which will be mapped to final_yshift in the result.
//           Added to result of row->baseline(x) if not NULL.
//
// x_scale: scale factor for the x-coordinate.
// y_scale: scale factor for the y-coordinate. Ignored if segs is given.
// Note that these scale factors apply to the same x and y system as the
// x-origin and y-origin apply, ie after any block rotation, but before
// the rotation argument is applied.
//
// final_xshift: The x component of the final translation.
// final_yshift: The y component of the final translation.
void DENORM::SetupNormalization(const BLOCK* block,
                                const ROW* row,
                                const FCOORD* rotation,
                                const DENORM* predecessor,
                                const DENORM_SEG* segs, int num_segs,
                                float x_origin, float y_origin,
                                float x_scale, float y_scale,
                                float final_xshift, float final_yshift) {
  Clear();
  block_ = block;
  row_ = row;
  if (rotation == NULL)
    rotation_ = NULL;
  else
    rotation_ = new FCOORD(*rotation);
  predecessor_ = predecessor;
  SetSegments(segs, num_segs);
  x_origin_ = x_origin;
  y_origin_ = y_origin;
  x_scale_ = x_scale;
  y_scale_ = y_scale;
  final_xshift_ = final_xshift;
  final_yshift_ = final_yshift;
}

// Transforms the given coords one step forward to normalized space, without
// using any block rotation or predecessor.
void DENORM::LocalNormTransform(const TPOINT& pt, TPOINT* transformed) const {
  FCOORD src_pt(pt.x, pt.y);
  FCOORD float_result;
  LocalNormTransform(src_pt, &float_result);
  transformed->x = IntCastRounded(float_result.x());
  transformed->y = IntCastRounded(float_result.y());
}
void DENORM::LocalNormTransform(const FCOORD& pt, FCOORD* transformed) const {
  FCOORD translated(pt.x() - x_origin_, pt.y() - YOriginAtOrigX(pt.x()));
  translated.set_x(translated.x() * x_scale_);
  translated.set_y(translated.y() * YScaleAtOrigX(pt.x()));
  if (rotation_ != NULL)
    translated.rotate(*rotation_);
  transformed->set_x(translated.x() + final_xshift_);
  transformed->set_y(translated.y() + final_yshift_);
}

// Transforms the given coords forward to normalized space using the
// full transformation sequence defined by the block rotation, the
// predecessors, deepest first, and finally this.
void DENORM::NormTransform(const TPOINT& pt, TPOINT* transformed) const {
  FCOORD src_pt(pt.x, pt.y);
  FCOORD float_result;
  NormTransform(src_pt, &float_result);
  transformed->x = IntCastRounded(float_result.x());
  transformed->y = IntCastRounded(float_result.y());
}
void DENORM::NormTransform(const FCOORD& pt, FCOORD* transformed) const {
  FCOORD src_pt(pt);
  if (predecessor_ != NULL) {
    predecessor_->NormTransform(pt, &src_pt);
  } else if (block_ != NULL) {
    FCOORD fwd_rotation(block_->re_rotation().x(), -block_->re_rotation().y());
    src_pt.rotate(fwd_rotation);
  }
  LocalNormTransform(src_pt, transformed);
}

// Transforms the given coords one step back to source space, without
// using to any block rotation or predecessor.
void DENORM::LocalDenormTransform(const TPOINT& pt, TPOINT* original) const {
  FCOORD src_pt(pt.x, pt.y);
  FCOORD float_result;
  LocalDenormTransform(src_pt, &float_result);
  original->x = IntCastRounded(float_result.x());
  original->y = IntCastRounded(float_result.y());
}
void DENORM::LocalDenormTransform(const FCOORD& pt, FCOORD* original) const {
  FCOORD rotated(pt.x() - final_xshift_, pt.y() - final_yshift_);
  if (rotation_ != NULL) {
    FCOORD inverse_rotation(rotation_->x(), -rotation_->y());
    rotated.rotate(inverse_rotation);
  }
  original->set_x(rotated.x() / x_scale_ + x_origin_);
  float y_scale = y_scale_;
  if (num_segs_ > 0)
    y_scale = YScaleAtOrigX(original->x());
  original->set_y(rotated.y() / y_scale + YOriginAtOrigX(original->x()));
}

// Transforms the given coords all the way back to source image space using
// the full transformation sequence defined by this and its predecesors
// recursively, shallowest first, and finally any block re_rotation.
void DENORM::DenormTransform(const TPOINT& pt, TPOINT* original) const {
  FCOORD src_pt(pt.x, pt.y);
  FCOORD float_result;
  DenormTransform(src_pt, &float_result);
  original->x = IntCastRounded(float_result.x());
  original->y = IntCastRounded(float_result.y());
}
void DENORM::DenormTransform(const FCOORD& pt, FCOORD* original) const {
  LocalDenormTransform(pt, original);
  if (predecessor_ != NULL) {
    predecessor_->DenormTransform(*original, original);
  } else if (block_ != NULL) {
    original->rotate(block_->re_rotation());
  }
}

// Normalize a blob using blob transformations. Less accurate, but
// more accurately copies the old way.
void DENORM::LocalNormBlob(TBLOB* blob) const {
  TBOX blob_box = blob->bounding_box();
  float x_center = (blob_box.left() + blob_box.right()) / 2.0f;
  ICOORD translation(-IntCastRounded(x_origin_),
                     -IntCastRounded(YOriginAtOrigX(x_center)));
  blob->Move(translation);
  // Note that the old way of scaling only allowed for a single
  // scale factor.
  float scale = YScaleAtOrigX(x_center);
  if (scale != 1.0f)
    blob->Scale(scale);
  if (rotation_ != NULL)
    blob->Rotate(*rotation_);
  translation.set_x(IntCastRounded(final_xshift_));
  translation.set_y(IntCastRounded(final_yshift_));
  blob->Move(translation);
}

// Fills in the x-height range accepted by the given unichar_id, given its
// bounding box in the usual baseline-normalized coordinates, with some
// initial crude x-height estimate (such as word size) and this denoting the
// transformation that was used. Returns false, and an empty range if the
// bottom is a mis-fit. Returns true and empty [0, 0] range if the bottom
// fits, but the top is impossible.
bool DENORM::XHeightRange(int unichar_id, const UNICHARSET& unicharset,
                          const TBOX& bbox,
                          inT16* min_xht, inT16* max_xht) const {
  // Clip the top and bottom to the limit of normalized feature space.
  int top = ClipToRange<int>(bbox.top(), 0, kBlnCellHeight - 1);
  int bottom = ClipToRange<int>(bbox.bottom(), 0, kBlnCellHeight - 1);
  // A tolerance of yscale corresponds to 1 pixel in the image.
  double tolerance = y_scale();
  int min_bottom, max_bottom, min_top, max_top;
  unicharset.get_top_bottom(unichar_id, &min_bottom, &max_bottom,
                            &min_top, &max_top);
  // Default returns indicate a mis-fit.
  *min_xht = 0;
  *max_xht = 0;
  // Chars with a misfitting bottom might be sub/superscript/dropcap, or might
  // just be wrongly classified. Return an empty range so they have to be
  // good to be considered.
  if (bottom < min_bottom - tolerance || bottom > max_bottom + tolerance) {
    return false;
  }
  // To help very high cap/xheight ratio fonts accept the correct x-height,
  // and to allow the large caps in small caps to accept the xheight of the
  // small caps, add kBlnBaselineOffset to chars with a maximum max.
  if (max_top == kBlnCellHeight - 1)
    max_top += kBlnBaselineOffset;
  int height = top - kBlnBaselineOffset;
  double min_height = min_top - kBlnBaselineOffset - tolerance;
  double max_height = max_top - kBlnBaselineOffset + tolerance;
  if (min_height <= 0.0) {
    if (height <= 0 || max_height > 0)
      *max_xht = MAX_INT16;  // Anything will do.
  } else if (height > 0) {
    int result = IntCastRounded(height * kBlnXHeight / y_scale() / min_height);
    *max_xht = static_cast<inT16>(ClipToRange(result, 0, MAX_INT16));
  }
  if (max_height > 0.0 && height > 0) {
    int result = IntCastRounded(height * kBlnXHeight / y_scale() / max_height);
    *min_xht = static_cast<inT16>(ClipToRange(result, 0, MAX_INT16));
  }
  return true;
}

// ============== Private Code ======================

// Free allocated memory and clear pointers.
void DENORM::Clear() {
  if (segs_ != NULL) {
    delete [] segs_;
    segs_ = NULL;
    num_segs_ = 0;
  }
  if (rotation_ != NULL) {
    delete rotation_;
    rotation_ = NULL;
  }
}

// Setup default values.
void DENORM::Init() {
  inverse_ = false;
  pix_ = NULL;
  block_ = NULL;
  row_ = NULL;
  rotation_ = NULL;
  predecessor_ = NULL;
  segs_ = NULL;
  num_segs_ = 0;
  x_origin_ = 0.0f;
  y_origin_ = 0.0f;
  x_scale_ = 1.0f;
  y_scale_ = 1.0f;
  final_xshift_ = 0.0f;
  final_yshift_ = static_cast<float>(kBlnBaselineOffset);
}

// Returns the y-origin at the original (un-normalized) x.
float DENORM::YOriginAtOrigX(float orig_x) const {
  if (num_segs_ > 0) {
    const DENORM_SEG* seg = BinarySearchSegment(orig_x);
    if (seg->ycoord != -MAX_INT32) {
      return seg->ycoord;
    }
  }
  if (row_ != NULL)
    return row_->base_line(orig_x) + y_origin_;
  else
    return y_origin_;
}

// Returns the y-scale at the original (un-normalized) x.
float DENORM::YScaleAtOrigX(float orig_x) const {
  if (num_segs_ > 0) {
    const DENORM_SEG* seg = BinarySearchSegment(orig_x);
    if (seg->scale_factor > 0.0)
      return seg->scale_factor;
  }
  return y_scale_;
}


// Compare two segments by xstart for use with qsort(3) and bsearch(3)
static int CompareSegByXStart(const DENORM_SEG* a, const DENORM_SEG* b) {
  if (a->xstart < b->xstart)
    return -1;
  else if (a->xstart > b->xstart)
    return 1;
  return 0;
}

// Deep copy the array of segments for use as a y_origin and y_scale.
void DENORM::SetSegments(const DENORM_SEG* new_segs, int seg_count) {
  if (segs_ != NULL)
    delete [] segs_;
  if (seg_count > 0) {
    segs_ = new DENORM_SEG[seg_count];
    memcpy(segs_, new_segs, seg_count * sizeof(new_segs[0]));
    // It is possible, if infrequent that the segments may be out of order.
    // since we are searching with a binary search, keep them in order.
    qsort(segs_, num_segs_, sizeof(segs_[0]),
          reinterpret_cast<int(*)(const void*, const void*)>(
              &CompareSegByXStart));
  } else {
    num_segs_ = 0;
    segs_ = NULL;
  }
}

// Finds the appropriate segment for a given original x-coord
const DENORM_SEG* DENORM::BinarySearchSegment(float orig_x) const {
  int bottom, top, middle;       // binary search
  bottom = 0;
  top = num_segs_;
  do {
    middle = (bottom + top) / 2;
    if (segs_[middle].xstart > orig_x)
      top = middle;
    else
      bottom = middle;
  }
  while (top - bottom > 1);
  return &segs_[bottom];
}
