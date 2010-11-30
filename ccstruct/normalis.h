/**********************************************************************
 * File:        normalis.h  (Formerly denorm.h)
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

#ifndef           NORMALIS_H
#define           NORMALIS_H

#include <stdio.h>
#include "host.h"

const int kBlnCellHeight = 256;     // Full-height for baseline normalization.
const int kBlnXHeight = 128;        // x-height for baseline normalization.
const int kBlnBaselineOffset = 64;  // offset for baseline normalization.

struct Pix;
class ROW;                          // Forward decl
class BLOCK;
class FCOORD;
class TBLOB;
class TBOX;
class TPOINT;

class DENORM_SEG {
 public:
  DENORM_SEG() {}

  inT32 xstart;         // start of segment
  inT32 ycoord;         // y at segment
  float scale_factor;   // normalized_x/scale_factor + x_center == original_x
};

class DENORM {
 public:
  DENORM();
  DENORM(float x, float scaling, ROW *src);
  DENORM(float x,              // from same pieces
         float scaling,
         double line_m,        // default line: y = mx + c
         double line_c,
         inT16 seg_count,      // no of segments
         DENORM_SEG *seg_pts,  // actual segments
         BOOL8 using_row,      // as baseline
         ROW *src);
  // Copying a DENORM is allowed.
  DENORM(const DENORM &);
  DENORM& operator=(const DENORM&);
  ~DENORM();

  // Setup for a baseline normalization. If there are segs, then they
  // are used, otherwise, if there is a row, that is used, otherwise the
  // bottom of the word_box is used for the baseline.
  void SetupBLNormalize(const BLOCK* block, const ROW* row, float x_height,
                        const TBOX& word_box,
                        int num_segs, const DENORM_SEG* segs);

  // Setup the normalization transformation parameters.
  // The normalizations applied to a blob are as follows:
  // 1. An optional block layout rotation that was applied during layout
  // analysis to make the textlines horizontal.
  // 2. A normalization transformation (LocalNormTransform):
  // Subtract the "origin"
  // Apply an x,y scaling.
  // Apply an optional rotation.
  // Add back a final translation.
  // The origin is in the block-rotated space, and is usually something like
  // the x-middle of the word at the baseline.
  // 3. Zero or more further normalization transformations that are applied
  // in sequence, with a similar pattern to the first normalization transform.
  //
  // A DENORM holds the parameters of a single normalization, and can execute
  // both the LocalNormTransform (a forwards normalization), and the
  // LocalDenormTransform which is an inverse transform or de-normalization.
  // A DENORM may point to a predecessor DENORM, which is actually the earlier
  // normalization, so the full normalization sequence involves executing all
  // predecessors first and then the transform in "this".
  // Let x be image co-ordinates and that we have normalization classes A, B, C
  // where we first apply A then B then C to get normalized x':
  // x' = CBAx
  // Then the backwards (to original coordinates) would be:
  // x = A^-1 B^-1 C^-1 x'
  // and A = B->predecessor_ and B = C->predecessor_
  // NormTransform executes all predecessors recursively, and then this.
  // NormTransform would be used to transform an image-based feature to
  // normalized space for use in a classifier
  // DenormTransform inverts this and then all predecessors. It can be
  // used to get back to the original image coordinates from normalized space.
  // The LocalNormTransform member executes just the transformation
  // in "this" without the layout rotation or any predecessors. It would be
  // used to run each successive normalization, eg the word normalization,
  // and later the character normalization.

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
  // longer than this denorm, except rotation and segs, which are deep
  // copied on input.
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
  //
  // In theory, any of the commonly used normalizations can be setup here:
  // * Traditional baseline normalization on a word:
  // SetupNormalization(block, row, NULL, NULL, NULL, 0,
  //                    box.x_middle(), 0.0f,
  //                    kBlnXHeight / x_height, kBlnXHeight / x_height,
  //                    0, kBlnBaselineOffset);
  // * Numeric mode baseline normalization on a word:
  // SetupNormalization(block, NULL, NULL, NULL, segs, num_segs,
  //                    box.x_middle(), 0.0f,
  //                    kBlnXHeight / x_height, kBlnXHeight / x_height,
  //                    0, kBlnBaselineOffset);
  // * Anisotropic character normalization used by IntFx.
  // SetupNormalization(NULL, NULL, NULL, denorm, NULL, 0,
  //                    centroid_x, centroid_y,
  //                    51.2 / ry, 51.2 / rx, 128, 128);
  // * Normalize blob height to x-height (current OSD):
  // SetupNormalization(NULL, NULL, &rotation, NULL, NULL, 0,
  //                    box.rotational_x_middle(rotation),
  //                    box.rotational_y_middle(rotation),
  //                    kBlnXHeight / box.rotational_height(rotation),
  //                    kBlnXHeight / box.rotational_height(rotation),
  //                    0, kBlnBaselineOffset);
  // * Secondary normalization for classification rotation (current):
  // FCOORD rotation = block->classify_rotation();
  // float target_height = kBlnXHeight / CCStruct::kXHeightCapRatio;
  // SetupNormalization(NULL, NULL, &rotation, denorm, NULL, 0,
  //                    box.rotational_x_middle(rotation),
  //                    box.rotational_y_middle(rotation),
  //                    target_height / box.rotational_height(rotation),
  //                    target_height / box.rotational_height(rotation),
  //                    0, kBlnBaselineOffset);
  // * Proposed new normalizations for CJK: Between them there is then
  // no need for further normalization at all, and the character fills the cell.
  // ** Replacement for baseline normalization on a word:
  // Scales height and width independently so that modal height and pitch
  // fill the cell respectively.
  // float cap_height = x_height / CCStruct::kXHeightCapRatio;
  // SetupNormalization(block, row, NULL, NULL, NULL, 0,
  //                    box.x_middle(), cap_height / 2.0f,
  //                    kBlnCellHeight / fixed_pitch,
  //                    kBlnCellHeight / cap_height,
  //                    0, 0);
  // ** Secondary normalization for classification (with rotation) (proposed):
  // Requires a simple translation to the center of the appropriate character
  // cell, no further scaling and a simple rotation (or nothing) about the
  // cell center.
  // FCOORD rotation = block->classify_rotation();
  // SetupNormalization(NULL, NULL, &rotation, denorm, NULL, 0,
  //                    fixed_pitch_cell_center,
  //                    0.0f,
  //                    1.0f,
  //                    1.0f,
  //                    0, 0);
  void SetupNormalization(const BLOCK* block,
                          const ROW* row,
                          const FCOORD* rotation,
                          const DENORM* predecessor,
                          const DENORM_SEG* segs, int num_segs,
                          float x_origin, float y_origin,
                          float x_scale, float y_scale,
                          float final_xshift, float final_yshift);

  // Transforms the given coords one step forward to normalized space, without
  // using any block rotation or predecessor.
  void LocalNormTransform(const TPOINT& pt, TPOINT* transformed) const;
  void LocalNormTransform(const FCOORD& pt, FCOORD* transformed) const;
  // Transforms the given coords forward to normalized space using the
  // full transformation sequence defined by the block rotation, the
  // predecessors, deepest first, and finally this.
  void NormTransform(const TPOINT& pt, TPOINT* transformed) const;
  void NormTransform(const FCOORD& pt, FCOORD* transformed) const;
  // Transforms the given coords one step back to source space, without
  // using to any block rotation or predecessor.
  void LocalDenormTransform(const TPOINT& pt, TPOINT* original) const;
  void LocalDenormTransform(const FCOORD& pt, FCOORD* original) const;
  // Transforms the given coords all the way back to source image space using
  // the full transformation sequence defined by this and its predecesors
  // recursively, shallowest first, and finally any block re_rotation.
  void DenormTransform(const TPOINT& pt, TPOINT* original) const;
  void DenormTransform(const FCOORD& pt, FCOORD* original) const;

  // Normalize a blob using blob transformations. Less accurate, but
  // more accurately copies the old way.
  void LocalNormBlob(TBLOB* blob) const;

  Pix* pix() const {
    return pix_;
  }
  void set_pix(Pix* pix) {
    pix_ = pix;
  }
  bool inverse() const {
    return inverse_;
  }
  void set_inverse(bool value) {
    inverse_ = value;
  }
  const DENORM* RootDenorm() const {
    if (predecessor_ != NULL)
      return predecessor_->RootDenorm();
    return this;
  }
  // Accessors - perhaps should not be needed.
  float x_scale() const {
    return x_scale_;
  }
  float y_scale() const {
    return y_scale_;
  }
  const ROW *row() const {
    return row_;
  }
  void set_row(ROW* row) {
    row_ = row;
  }
  const BLOCK* block() const {
    return block_;
  }
  void set_block(const BLOCK* block) {
    block_ = block;
  }

 private:
  // Free allocated memory and clear pointers.
  void Clear();
  // Setup default values.
  void Init();

  // Returns the y-origin at the original (un-normalized) x.
  float YOriginAtOrigX(float orig_x) const;

  // Returns the y-scale at the original (un-normalized) x.
  float YScaleAtOrigX(float orig_x) const;

  // Deep copy the array of segments for use as a y_origin and y_scale.
  void SetSegments(const DENORM_SEG* new_segs, int seg_count);

  // Finds the appropriate segment for a given original x-coord
  const DENORM_SEG* BinarySearchSegment(float orig_x) const;

  // Best available image.
  Pix* pix_;
  // True if the source image is white-on-black.
  bool inverse_;
  // Block the word came from. If not null, block->re_rotation() takes the
  // "untransformed" coordinates even further back to the original image.
  const BLOCK* block_;
  // Row the word came from. If not null, row->baseline() is added to y_origin_.
  const ROW* row_;
  // Rotation to apply between translation to the origin and scaling.
  const FCOORD* rotation_;
  // Previous transformation in a chain.
  const DENORM* predecessor_;
  // Array of segments used to specify local y_origin_ and y_scale_.
  // Owned by the DENORM.
  DENORM_SEG *segs_;
  // Size of the segs_ array.
  int num_segs_;
  // x-coordinate to be mapped to final_xshift_ in the result.
  float x_origin_;
  // y-coordinate to be mapped to final_yshift_ in the result.
  float y_origin_;
  // Scale factors for x and y coords. Applied to pre-rotation system.
  float x_scale_;
  float y_scale_;
  // Destination coords of the x_origin_ and y_origin_.
  float final_xshift_;
  float final_yshift_;
};
#endif
