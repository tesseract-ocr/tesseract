///////////////////////////////////////////////////////////////////////
// File:        imagefind.cpp
// Description: Function to find image and drawing regions in an image
//              and create a corresponding list of empty blobs.
// Author:      Ray Smith
//
// (C) Copyright 2008, Google Inc.
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

#ifdef HAVE_CONFIG_H
#  include "config_auto.h"
#endif

#include "imagefind.h"

#include "colpartitiongrid.h"
#include "linlsq.h"
#include "params.h"
#include "statistc.h"

#include <allheaders.h>

#include <algorithm>

namespace tesseract {

static INT_VAR(textord_tabfind_show_images, false, "Show image blobs");

// Fraction of width or height of on pixels that can be discarded from a
// roughly rectangular image.
const double kMinRectangularFraction = 0.125;
// Fraction of width or height to consider image completely used.
const double kMaxRectangularFraction = 0.75;
// Fraction of width or height to allow transition from kMinRectangularFraction
// to kMaxRectangularFraction, equivalent to a dy/dx skew.
const double kMaxRectangularGradient = 0.1; // About 6 degrees.
// Minimum image size to be worth looking for images on.
const int kMinImageFindSize = 100;
// Pixel padding for noise blobs and partitions when rendering on the image
// mask to encourage them to join together. Make it too big and images
// will fatten out too much and have to be clipped to text.
const int kNoisePadding = 4;

// Scans horizontally on x=[x_start,x_end), starting with y=*y_start,
// stepping y+=y_step, until y=y_end. *ystart is input/output.
// If the number of black pixels in a row, pix_count fits this pattern:
// 0 or more rows with pix_count < min_count then
// <= mid_width rows with min_count <= pix_count <= max_count then
// a row with pix_count > max_count then
// true is returned, and *y_start = the first y with pix_count >= min_count.
static bool HScanForEdge(uint32_t *data, int wpl, int x_start, int x_end, int min_count,
                         int mid_width, int max_count, int y_end, int y_step, int *y_start) {
  int mid_rows = 0;
  for (int y = *y_start; y != y_end; y += y_step) {
    // Need pixCountPixelsInRow(pix, y, &pix_count, nullptr) to count in a
    // subset.
    int pix_count = 0;
    uint32_t *line = data + wpl * y;
    for (int x = x_start; x < x_end; ++x) {
      if (GET_DATA_BIT(line, x)) {
        ++pix_count;
      }
    }
    if (mid_rows == 0 && pix_count < min_count) {
      continue; // In the min phase.
    }
    if (mid_rows == 0) {
      *y_start = y; // Save the y_start where we came out of the min phase.
    }
    if (pix_count > max_count) {
      return true; // Found the pattern.
    }
    ++mid_rows;
    if (mid_rows > mid_width) {
      break; // Middle too big.
    }
  }
  return false; // Never found max_count.
}

// Scans vertically on y=[y_start,y_end), starting with x=*x_start,
// stepping x+=x_step, until x=x_end. *x_start is input/output.
// If the number of black pixels in a column, pix_count fits this pattern:
// 0 or more cols with pix_count < min_count then
// <= mid_width cols with min_count <= pix_count <= max_count then
// a column with pix_count > max_count then
// true is returned, and *x_start = the first x with pix_count >= min_count.
static bool VScanForEdge(uint32_t *data, int wpl, int y_start, int y_end, int min_count,
                         int mid_width, int max_count, int x_end, int x_step, int *x_start) {
  int mid_cols = 0;
  for (int x = *x_start; x != x_end; x += x_step) {
    int pix_count = 0;
    uint32_t *line = data + y_start * wpl;
    for (int y = y_start; y < y_end; ++y, line += wpl) {
      if (GET_DATA_BIT(line, x)) {
        ++pix_count;
      }
    }
    if (mid_cols == 0 && pix_count < min_count) {
      continue; // In the min phase.
    }
    if (mid_cols == 0) {
      *x_start = x; // Save the place where we came out of the min phase.
    }
    if (pix_count > max_count) {
      return true; // found the pattern.
    }
    ++mid_cols;
    if (mid_cols > mid_width) {
      break; // Middle too big.
    }
  }
  return false; // Never found max_count.
}

// Returns true if there is a rectangle in the source pix, such that all
// pixel rows and column slices outside of it have less than
// min_fraction of the pixels black, and within max_skew_gradient fraction
// of the pixels on the inside, there are at least max_fraction of the
// pixels black. In other words, the inside of the rectangle looks roughly
// rectangular, and the outside of it looks like extra bits.
// On return, the rectangle is defined by x_start, y_start, x_end and y_end.
// Note: the algorithm is iterative, allowing it to slice off pixels from
// one edge, allowing it to then slice off more pixels from another edge.
static bool pixNearlyRectangular(Image pix, double min_fraction, double max_fraction,
                                 double max_skew_gradient, int *x_start, int *y_start,
                                 int *x_end, int *y_end) {
  ASSERT_HOST(pix != nullptr);
  *x_start = 0;
  *x_end = pixGetWidth(pix);
  *y_start = 0;
  *y_end = pixGetHeight(pix);

  uint32_t *data = pixGetData(pix);
  int wpl = pixGetWpl(pix);
  bool any_cut = false;
  bool left_done = false;
  bool right_done = false;
  bool top_done = false;
  bool bottom_done = false;
  do {
    any_cut = false;
    // Find the top/bottom edges.
    int width = *x_end - *x_start;
    int min_count = static_cast<int>(width * min_fraction);
    int max_count = static_cast<int>(width * max_fraction);
    int edge_width = static_cast<int>(width * max_skew_gradient);
    if (HScanForEdge(data, wpl, *x_start, *x_end, min_count, edge_width, max_count, *y_end, 1,
                     y_start) &&
        !top_done) {
      top_done = true;
      any_cut = true;
    }
    --(*y_end);
    if (HScanForEdge(data, wpl, *x_start, *x_end, min_count, edge_width, max_count, *y_start, -1,
                     y_end) &&
        !bottom_done) {
      bottom_done = true;
      any_cut = true;
    }
    ++(*y_end);

    // Find the left/right edges.
    int height = *y_end - *y_start;
    min_count = static_cast<int>(height * min_fraction);
    max_count = static_cast<int>(height * max_fraction);
    edge_width = static_cast<int>(height * max_skew_gradient);
    if (VScanForEdge(data, wpl, *y_start, *y_end, min_count, edge_width, max_count, *x_end, 1,
                     x_start) &&
        !left_done) {
      left_done = true;
      any_cut = true;
    }
    --(*x_end);
    if (VScanForEdge(data, wpl, *y_start, *y_end, min_count, edge_width, max_count, *x_start, -1,
                     x_end) &&
        !right_done) {
      right_done = true;
      any_cut = true;
    }
    ++(*x_end);
  } while (any_cut);

  // All edges must satisfy the condition of sharp gradient in pixel density
  // in order for the full rectangle to be present.
  return left_done && right_done && top_done && bottom_done;
}

// Generates a Boxa, Pixa pair from the input binary (image mask) pix,
// analogous to pixConnComp, except that connected components which are nearly
// rectangular are replaced with solid rectangles.
// The returned boxa, pixa may be nullptr, meaning no images found.
// If not nullptr, they must be destroyed by the caller.
// Resolution of pix should match the source image (Tesseract::pix_binary_)
// so the output coordinate systems match.
static void ConnCompAndRectangularize(Image pix, DebugPixa *pixa_debug, Boxa **boxa,
                                      Pixa **pixa) {
  *boxa = nullptr;
  *pixa = nullptr;

  if (textord_tabfind_show_images && pixa_debug != nullptr) {
    pixa_debug->AddPix(pix, "Conncompimage");
  }
  // Find the individual image regions in the mask image.
  *boxa = pixConnComp(pix, pixa, 8);
  // Rectangularize the individual images. If a sharp edge in vertical and/or
  // horizontal occupancy can be found, it indicates a probably rectangular
  // image with unwanted bits merged on, so clip to the approximate rectangle.
  int npixes = 0;
  if (*boxa != nullptr && *pixa != nullptr) {
    npixes = pixaGetCount(*pixa);
  }
  for (int i = 0; i < npixes; ++i) {
    int x_start, x_end, y_start, y_end;
    Image img_pix = pixaGetPix(*pixa, i, L_CLONE);
    if (textord_tabfind_show_images && pixa_debug != nullptr) {
      pixa_debug->AddPix(img_pix, "A component");
    }
    if (pixNearlyRectangular(img_pix, kMinRectangularFraction, kMaxRectangularFraction,
                             kMaxRectangularGradient, &x_start, &y_start, &x_end, &y_end)) {
      Image simple_pix = pixCreate(x_end - x_start, y_end - y_start, 1);
      pixSetAll(simple_pix);
      img_pix.destroy();
      // pixaReplacePix takes ownership of the simple_pix.
      pixaReplacePix(*pixa, i, simple_pix, nullptr);
      img_pix = pixaGetPix(*pixa, i, L_CLONE);
      // Fix the box to match the new pix.
      l_int32 x, y, width, height;
      boxaGetBoxGeometry(*boxa, i, &x, &y, &width, &height);
      Box *simple_box = boxCreate(x + x_start, y + y_start, x_end - x_start, y_end - y_start);
      boxaReplaceBox(*boxa, i, simple_box);
    }
    img_pix.destroy();
  }
}

// Finds image regions within the BINARY source pix (page image) and returns
// the image regions as a mask image.
// The returned pix may be nullptr, meaning no images found.
// If not nullptr, it must be PixDestroyed by the caller.
// If textord_tabfind_show_images, debug images are appended to pixa_debug.
Image ImageFind::FindImages(Image pix, DebugPixa *pixa_debug) {
  // Not worth looking at small images.
  if (pixGetWidth(pix) < kMinImageFindSize || pixGetHeight(pix) < kMinImageFindSize) {
    return pixCreate(pixGetWidth(pix), pixGetHeight(pix), 1);
  }

  // Reduce by factor 2.
  Image pixr = pixReduceRankBinaryCascade(pix, 1, 0, 0, 0);
  if (textord_tabfind_show_images && pixa_debug != nullptr) {
    pixa_debug->AddPix(pixr, "CascadeReduced");
  }

  // Get the halftone mask directly from Leptonica.
  //
  // Leptonica will print an error message and return nullptr if we call
  // pixGenHalftoneMask(pixr, nullptr, ...) with too small image, so we
  // want to bypass that.
  if (pixGetWidth(pixr) < kMinImageFindSize || pixGetHeight(pixr) < kMinImageFindSize) {
    pixr.destroy();
    return pixCreate(pixGetWidth(pix), pixGetHeight(pix), 1);
  }
  // Get the halftone mask.
  l_int32 ht_found = 0;
  Pixa *pixadb = (textord_tabfind_show_images && pixa_debug != nullptr) ? pixaCreate(0) : nullptr;
  Image pixht2 = pixGenerateHalftoneMask(pixr, nullptr, &ht_found, pixadb);
  if (pixadb) {
    Image pixdb = pixaDisplayTiledInColumns(pixadb, 3, 1.0, 20, 2);
    if (textord_tabfind_show_images && pixa_debug != nullptr) {
      pixa_debug->AddPix(pixdb, "HalftoneMask");
    }
    pixdb.destroy();
    pixaDestroy(&pixadb);
  }
  pixr.destroy();
  if (!ht_found && pixht2 != nullptr) {
    pixht2.destroy();
  }
  if (pixht2 == nullptr) {
    return pixCreate(pixGetWidth(pix), pixGetHeight(pix), 1);
  }

  // Expand back up again.
  Image pixht = pixExpandReplicate(pixht2, 2);
  if (textord_tabfind_show_images && pixa_debug != nullptr) {
    pixa_debug->AddPix(pixht, "HalftoneReplicated");
  }
  pixht2.destroy();

  // Fill to capture pixels near the mask edges that were missed
  Image pixt = pixSeedfillBinary(nullptr, pixht, pix, 8);
  pixht |= pixt;
  pixt.destroy();

  // Eliminate lines and bars that may be joined to images.
  Image pixfinemask = pixReduceRankBinaryCascade(pixht, 1, 1, 3, 3);
  pixDilateBrick(pixfinemask, pixfinemask, 5, 5);
  if (textord_tabfind_show_images && pixa_debug != nullptr) {
    pixa_debug->AddPix(pixfinemask, "FineMask");
  }
  Image pixreduced = pixReduceRankBinaryCascade(pixht, 1, 1, 1, 1);
  Image pixreduced2 = pixReduceRankBinaryCascade(pixreduced, 3, 3, 3, 0);
  pixreduced.destroy();
  pixDilateBrick(pixreduced2, pixreduced2, 5, 5);
  Image pixcoarsemask = pixExpandReplicate(pixreduced2, 8);
  pixreduced2.destroy();
  if (textord_tabfind_show_images && pixa_debug != nullptr) {
    pixa_debug->AddPix(pixcoarsemask, "CoarseMask");
  }
  // Combine the coarse and fine image masks.
  pixcoarsemask &= pixfinemask;
  pixfinemask.destroy();
  // Dilate a bit to make sure we get everything.
  pixDilateBrick(pixcoarsemask, pixcoarsemask, 3, 3);
  Image pixmask = pixExpandReplicate(pixcoarsemask, 16);
  pixcoarsemask.destroy();
  if (textord_tabfind_show_images && pixa_debug != nullptr) {
    pixa_debug->AddPix(pixmask, "MaskDilated");
  }
  // And the image mask with the line and bar remover.
  pixht &= pixmask;
  pixmask.destroy();
  if (textord_tabfind_show_images && pixa_debug != nullptr) {
    pixa_debug->AddPix(pixht, "FinalMask");
  }
  // Make the result image the same size as the input.
  Image result = pixCreate(pixGetWidth(pix), pixGetHeight(pix), 1);
  result |= pixht;
  pixht.destroy();
  return result;
}

// Given an input pix, and a bounding rectangle, the sides of the rectangle
// are shrunk inwards until they bound any black pixels found within the
// original rectangle. Returns false if the rectangle contains no black
// pixels at all.
bool ImageFind::BoundsWithinRect(Image pix, int *x_start, int *y_start, int *x_end, int *y_end) {
  Box *input_box = boxCreate(*x_start, *y_start, *x_end - *x_start, *y_end - *y_start);
  Box *output_box = nullptr;
  pixClipBoxToForeground(pix, input_box, nullptr, &output_box);
  bool result = output_box != nullptr;
  if (result) {
    l_int32 x, y, width, height;
    boxGetGeometry(output_box, &x, &y, &width, &height);
    *x_start = x;
    *y_start = y;
    *x_end = x + width;
    *y_end = y + height;
    boxDestroy(&output_box);
  }
  boxDestroy(&input_box);
  return result;
}

// Given a point in 3-D (RGB) space, returns the squared Euclidean distance
// of the point from the given line, defined by a pair of points in the 3-D
// (RGB) space, line1 and line2.
double ImageFind::ColorDistanceFromLine(const uint8_t *line1, const uint8_t *line2,
                                        const uint8_t *point) {
  int line_vector[kRGBRMSColors];
  int point_vector[kRGBRMSColors];
  for (int i = 0; i < kRGBRMSColors; ++i) {
    line_vector[i] = static_cast<int>(line2[i]) - static_cast<int>(line1[i]);
    point_vector[i] = static_cast<int>(point[i]) - static_cast<int>(line1[i]);
  }
  line_vector[L_ALPHA_CHANNEL] = 0;
  // Now the cross product in 3d.
  int cross[kRGBRMSColors];
  cross[COLOR_RED] = line_vector[COLOR_GREEN] * point_vector[COLOR_BLUE] -
                     line_vector[COLOR_BLUE] * point_vector[COLOR_GREEN];
  cross[COLOR_GREEN] = line_vector[COLOR_BLUE] * point_vector[COLOR_RED] -
                       line_vector[COLOR_RED] * point_vector[COLOR_BLUE];
  cross[COLOR_BLUE] = line_vector[COLOR_RED] * point_vector[COLOR_GREEN] -
                      line_vector[COLOR_GREEN] * point_vector[COLOR_RED];
  cross[L_ALPHA_CHANNEL] = 0;
  // Now the sums of the squares.
  double cross_sq = 0.0;
  double line_sq = 0.0;
  for (int j = 0; j < kRGBRMSColors; ++j) {
    cross_sq += static_cast<double>(cross[j]) * cross[j];
    line_sq += static_cast<double>(line_vector[j]) * line_vector[j];
  }
  if (line_sq == 0.0) {
    return 0.0;
  }
  return cross_sq / line_sq; // This is the squared distance.
}

// ================ CUTTING POLYGONAL IMAGES FROM A RECTANGLE ================
// The following functions are responsible for cutting a polygonal image from
// a rectangle: CountPixelsInRotatedBox, AttemptToShrinkBox, CutChunkFromParts
// with DivideImageIntoParts as the master.
// Problem statement:
// We start with a single connected component from the image mask: we get
// a Pix of the component, and its location on the page (im_box).
// The objective of cutting a polygonal image from its rectangle is to avoid
// interfering text, but not text that completely overlaps the image.
//     ------------------------------      ------------------------------
//     |   Single input partition   |      | 1 Cut up output partitions |
//     |                            |      ------------------------------
//   Av|oid                         |    Avoid |                        |
//     |                            |          |________________________|
//  Int|erfering                    |   Interfering  |                  |
//     |                            |           _____|__________________|
//    T|ext                         |     Text |                        |
//     |        Text-on-image       |          |     Text-on-image      |
//     ------------------------------          --------------------------
// DivideImageIntoParts does this by building a ColPartition_LIST (not in the
// grid) with each ColPartition representing one of the rectangles needed,
// starting with a single rectangle for the whole image component, and cutting
// bits out of it with CutChunkFromParts as needed to avoid text. The output
// ColPartitions are supposed to be ordered from top to bottom.

// The problem is complicated by the fact that we have rotated the coordinate
// system to make text lines horizontal, so if we need to look at the component
// image, we have to rotate the coordinates. Throughout the functions in this
// section im_box is the rectangle representing the image component in the
// rotated page coordinates (where we are building our output ColPartitions),
// rotation is the rotation that we used to get there, and rerotation is the
// rotation required to get back to original page image coordinates.
// To get to coordinates in the component image, pix, we rotate the im_box,
// the point we want to locate, and subtract the rotated point from the top-left
// of the rotated im_box.
// im_box is therefore essential to calculating coordinates within the pix.

// Returns true if there are no black pixels in between the boxes.
// The im_box must represent the bounding box of the pix in tesseract
// coordinates, which may be negative, due to rotations to make the textlines
// horizontal. The boxes are rotated by rotation, which should undo such
// rotations, before mapping them onto the pix.
bool ImageFind::BlankImageInBetween(const TBOX &box1, const TBOX &box2, const TBOX &im_box,
                                    const FCOORD &rotation, Image pix) {
  TBOX search_box(box1);
  search_box += box2;
  if (box1.x_gap(box2) >= box1.y_gap(box2)) {
    if (box1.x_gap(box2) <= 0) {
      return true;
    }
    search_box.set_left(std::min(box1.right(), box2.right()));
    search_box.set_right(std::max(box1.left(), box2.left()));
  } else {
    if (box1.y_gap(box2) <= 0) {
      return true;
    }
    search_box.set_top(std::max(box1.bottom(), box2.bottom()));
    search_box.set_bottom(std::min(box1.top(), box2.top()));
  }
  return CountPixelsInRotatedBox(search_box, im_box, rotation, pix) == 0;
}

// Returns the number of pixels in box in the pix.
// rotation, pix and im_box are defined in the large comment above.
int ImageFind::CountPixelsInRotatedBox(TBOX box, const TBOX &im_box, const FCOORD &rotation,
                                       Image pix) {
  // Intersect it with the image box.
  box &= im_box; // This is in-place box intersection.
  if (box.null_box()) {
    return 0;
  }
  box.rotate(rotation);
  TBOX rotated_im_box(im_box);
  rotated_im_box.rotate(rotation);
  Image rect_pix = pixCreate(box.width(), box.height(), 1);
  pixRasterop(rect_pix, 0, 0, box.width(), box.height(), PIX_SRC, pix,
              box.left() - rotated_im_box.left(), rotated_im_box.top() - box.top());
  l_int32 result;
  pixCountPixels(rect_pix, &result, nullptr);
  rect_pix.destroy();
  return result;
}

// The box given by slice contains some black pixels, but not necessarily
// over the whole box. Shrink the x bounds of slice, but not the y bounds
// until there is at least one black pixel in the outermost columns.
// rotation, rerotation, pix and im_box are defined in the large comment above.
static void AttemptToShrinkBox(const FCOORD &rotation, const FCOORD &rerotation, const TBOX &im_box,
                               Image pix, TBOX *slice) {
  TBOX rotated_box(*slice);
  rotated_box.rotate(rerotation);
  TBOX rotated_im_box(im_box);
  rotated_im_box.rotate(rerotation);
  int left = rotated_box.left() - rotated_im_box.left();
  int right = rotated_box.right() - rotated_im_box.left();
  int top = rotated_im_box.top() - rotated_box.top();
  int bottom = rotated_im_box.top() - rotated_box.bottom();
  ImageFind::BoundsWithinRect(pix, &left, &top, &right, &bottom);
  top = rotated_im_box.top() - top;
  bottom = rotated_im_box.top() - bottom;
  left += rotated_im_box.left();
  right += rotated_im_box.left();
  rotated_box.set_to_given_coords(left, bottom, right, top);
  rotated_box.rotate(rotation);
  slice->set_left(rotated_box.left());
  slice->set_right(rotated_box.right());
}

// The meat of cutting a polygonal image around text.
// This function covers the general case of cutting a box out of a box
// as shown:
// Input                               Output
// ------------------------------      ------------------------------
// |   Single input partition   |      | 1 Cut up output partitions |
// |                            |      ------------------------------
// |         ----------         |      ---------           ----------
// |         |  box   |         |      |   2   |   box     |    3   |
// |         |        |         |      |       |  is cut   |        |
// |         ----------         |      ---------   out     ----------
// |                            |      ------------------------------
// |                            |      |   4                        |
// ------------------------------      ------------------------------
// In the context that this function is used, at most 3 of the above output
// boxes will be created, as the overlapping box is never contained by the
// input.
// The above cutting operation is executed for each element of part_list that
// is overlapped by the input box. Each modified ColPartition is replaced
// in place in the list by the output of the cutting operation in the order
// shown above, so iff no holes are ever created, the output will be in
// top-to-bottom order, but in extreme cases, hole creation is possible.
// In such cases, the output order may cause strange block polygons.
// rotation, rerotation, pix and im_box are defined in the large comment above.
static void CutChunkFromParts(const TBOX &box, const TBOX &im_box, const FCOORD &rotation,
                              const FCOORD &rerotation, Image pix, ColPartition_LIST *part_list) {
  ASSERT_HOST(!part_list->empty());
  ColPartition_IT part_it(part_list);
  do {
    ColPartition *part = part_it.data();
    TBOX part_box = part->bounding_box();
    if (part_box.overlap(box)) {
      // This part must be cut and replaced with the remains. There are
      // up to 4 pieces to be made. Start with the first one and use
      // add_before_stay_put. For each piece if it has no black pixels
      // left, just don't make the box.
      // Above box.
      if (box.top() < part_box.top()) {
        TBOX slice(part_box);
        slice.set_bottom(box.top());
        if (ImageFind::CountPixelsInRotatedBox(slice, im_box, rerotation, pix) > 0) {
          AttemptToShrinkBox(rotation, rerotation, im_box, pix, &slice);
          part_it.add_before_stay_put(
              ColPartition::FakePartition(slice, PT_UNKNOWN, BRT_POLYIMAGE, BTFT_NONTEXT));
        }
      }
      // Left of box.
      if (box.left() > part_box.left()) {
        TBOX slice(part_box);
        slice.set_right(box.left());
        if (box.top() < part_box.top()) {
          slice.set_top(box.top());
        }
        if (box.bottom() > part_box.bottom()) {
          slice.set_bottom(box.bottom());
        }
        if (ImageFind::CountPixelsInRotatedBox(slice, im_box, rerotation, pix) > 0) {
          AttemptToShrinkBox(rotation, rerotation, im_box, pix, &slice);
          part_it.add_before_stay_put(
              ColPartition::FakePartition(slice, PT_UNKNOWN, BRT_POLYIMAGE, BTFT_NONTEXT));
        }
      }
      // Right of box.
      if (box.right() < part_box.right()) {
        TBOX slice(part_box);
        slice.set_left(box.right());
        if (box.top() < part_box.top()) {
          slice.set_top(box.top());
        }
        if (box.bottom() > part_box.bottom()) {
          slice.set_bottom(box.bottom());
        }
        if (ImageFind::CountPixelsInRotatedBox(slice, im_box, rerotation, pix) > 0) {
          AttemptToShrinkBox(rotation, rerotation, im_box, pix, &slice);
          part_it.add_before_stay_put(
              ColPartition::FakePartition(slice, PT_UNKNOWN, BRT_POLYIMAGE, BTFT_NONTEXT));
        }
      }
      // Below box.
      if (box.bottom() > part_box.bottom()) {
        TBOX slice(part_box);
        slice.set_top(box.bottom());
        if (ImageFind::CountPixelsInRotatedBox(slice, im_box, rerotation, pix) > 0) {
          AttemptToShrinkBox(rotation, rerotation, im_box, pix, &slice);
          part_it.add_before_stay_put(
              ColPartition::FakePartition(slice, PT_UNKNOWN, BRT_POLYIMAGE, BTFT_NONTEXT));
        }
      }
      part->DeleteBoxes();
      delete part_it.extract();
    }
    part_it.forward();
  } while (!part_it.at_first());
}

// Starts with the bounding box of the image component and cuts it up
// so that it doesn't intersect text where possible.
// Strong fully contained horizontal text is marked as text on image,
// and does not cause a division of the image.
// For more detail see the large comment above on cutting polygonal images
// from a rectangle.
// rotation, rerotation, pix and im_box are defined in the large comment above.
static void DivideImageIntoParts(const TBOX &im_box, const FCOORD &rotation,
                                 const FCOORD &rerotation, Image pix,
                                 ColPartitionGridSearch *rectsearch, ColPartition_LIST *part_list) {
  // Add the full im_box partition to the list to begin with.
  ColPartition *pix_part =
      ColPartition::FakePartition(im_box, PT_UNKNOWN, BRT_RECTIMAGE, BTFT_NONTEXT);
  ColPartition_IT part_it(part_list);
  part_it.add_after_then_move(pix_part);

  rectsearch->StartRectSearch(im_box);
  ColPartition *part;
  while ((part = rectsearch->NextRectSearch()) != nullptr) {
    TBOX part_box = part->bounding_box();
    if (part_box.contains(im_box) && part->flow() >= BTFT_CHAIN) {
      // This image is completely covered by an existing text partition.
      for (part_it.move_to_first(); !part_it.empty(); part_it.forward()) {
        ColPartition *pix_part = part_it.extract();
        pix_part->DeleteBoxes();
        delete pix_part;
      }
    } else if (part->flow() == BTFT_STRONG_CHAIN) {
      // Text intersects the box.
      TBOX overlap_box = part_box.intersection(im_box);
      // Intersect it with the image box.
      int black_area = ImageFind::CountPixelsInRotatedBox(overlap_box, im_box, rerotation, pix);
      if (black_area * 2 < part_box.area() || !im_box.contains(part_box)) {
        // Eat a piece out of the image.
        // Pad it so that pieces eaten out look decent.
        int padding = part->blob_type() == BRT_VERT_TEXT ? part_box.width() : part_box.height();
        part_box.set_top(part_box.top() + padding / 2);
        part_box.set_bottom(part_box.bottom() - padding / 2);
        CutChunkFromParts(part_box, im_box, rotation, rerotation, pix, part_list);
      } else {
        // Strong overlap with the black area, so call it text on image.
        part->set_flow(BTFT_TEXT_ON_IMAGE);
      }
    }
    if (part_list->empty()) {
      break;
    }
  }
}

// Search for the rightmost text that overlaps vertically and is to the left
// of the given box, but within the given left limit.
static int ExpandImageLeft(const TBOX &box, int left_limit, ColPartitionGrid *part_grid) {
  ColPartitionGridSearch search(part_grid);
  ColPartition *part;
  // Search right to left for any text that overlaps.
  search.StartSideSearch(box.left(), box.bottom(), box.top());
  while ((part = search.NextSideSearch(true)) != nullptr) {
    if (part->flow() == BTFT_STRONG_CHAIN || part->flow() == BTFT_CHAIN) {
      const TBOX &part_box(part->bounding_box());
      if (part_box.y_gap(box) < 0) {
        if (part_box.right() > left_limit && part_box.right() < box.left()) {
          left_limit = part_box.right();
        }
        break;
      }
    }
  }
  if (part != nullptr) {
    // Search for the nearest text up to the one we already found.
    TBOX search_box(left_limit, box.bottom(), box.left(), box.top());
    search.StartRectSearch(search_box);
    while ((part = search.NextRectSearch()) != nullptr) {
      if (part->flow() == BTFT_STRONG_CHAIN || part->flow() == BTFT_CHAIN) {
        const TBOX &part_box(part->bounding_box());
        if (part_box.y_gap(box) < 0) {
          if (part_box.right() > left_limit && part_box.right() < box.left()) {
            left_limit = part_box.right();
          }
        }
      }
    }
  }
  return left_limit;
}

// Search for the leftmost text that overlaps vertically and is to the right
// of the given box, but within the given right limit.
static int ExpandImageRight(const TBOX &box, int right_limit, ColPartitionGrid *part_grid) {
  ColPartitionGridSearch search(part_grid);
  ColPartition *part;
  // Search left to right for any text that overlaps.
  search.StartSideSearch(box.right(), box.bottom(), box.top());
  while ((part = search.NextSideSearch(false)) != nullptr) {
    if (part->flow() == BTFT_STRONG_CHAIN || part->flow() == BTFT_CHAIN) {
      const TBOX &part_box(part->bounding_box());
      if (part_box.y_gap(box) < 0) {
        if (part_box.left() < right_limit && part_box.left() > box.right()) {
          right_limit = part_box.left();
        }
        break;
      }
    }
  }
  if (part != nullptr) {
    // Search for the nearest text up to the one we already found.
    TBOX search_box(box.left(), box.bottom(), right_limit, box.top());
    search.StartRectSearch(search_box);
    while ((part = search.NextRectSearch()) != nullptr) {
      if (part->flow() == BTFT_STRONG_CHAIN || part->flow() == BTFT_CHAIN) {
        const TBOX &part_box(part->bounding_box());
        if (part_box.y_gap(box) < 0) {
          if (part_box.left() < right_limit && part_box.left() > box.right()) {
            right_limit = part_box.left();
          }
        }
      }
    }
  }
  return right_limit;
}

// Search for the topmost text that overlaps horizontally and is below
// the given box, but within the given bottom limit.
static int ExpandImageBottom(const TBOX &box, int bottom_limit, ColPartitionGrid *part_grid) {
  ColPartitionGridSearch search(part_grid);
  ColPartition *part;
  // Search right to left for any text that overlaps.
  search.StartVerticalSearch(box.left(), box.right(), box.bottom());
  while ((part = search.NextVerticalSearch(true)) != nullptr) {
    if (part->flow() == BTFT_STRONG_CHAIN || part->flow() == BTFT_CHAIN) {
      const TBOX &part_box(part->bounding_box());
      if (part_box.x_gap(box) < 0) {
        if (part_box.top() > bottom_limit && part_box.top() < box.bottom()) {
          bottom_limit = part_box.top();
        }
        break;
      }
    }
  }
  if (part != nullptr) {
    // Search for the nearest text up to the one we already found.
    TBOX search_box(box.left(), bottom_limit, box.right(), box.bottom());
    search.StartRectSearch(search_box);
    while ((part = search.NextRectSearch()) != nullptr) {
      if (part->flow() == BTFT_STRONG_CHAIN || part->flow() == BTFT_CHAIN) {
        const TBOX &part_box(part->bounding_box());
        if (part_box.x_gap(box) < 0) {
          if (part_box.top() > bottom_limit && part_box.top() < box.bottom()) {
            bottom_limit = part_box.top();
          }
        }
      }
    }
  }
  return bottom_limit;
}

// Search for the bottommost text that overlaps horizontally and is above
// the given box, but within the given top limit.
static int ExpandImageTop(const TBOX &box, int top_limit, ColPartitionGrid *part_grid) {
  ColPartitionGridSearch search(part_grid);
  ColPartition *part;
  // Search right to left for any text that overlaps.
  search.StartVerticalSearch(box.left(), box.right(), box.top());
  while ((part = search.NextVerticalSearch(false)) != nullptr) {
    if (part->flow() == BTFT_STRONG_CHAIN || part->flow() == BTFT_CHAIN) {
      const TBOX &part_box(part->bounding_box());
      if (part_box.x_gap(box) < 0) {
        if (part_box.bottom() < top_limit && part_box.bottom() > box.top()) {
          top_limit = part_box.bottom();
        }
        break;
      }
    }
  }
  if (part != nullptr) {
    // Search for the nearest text up to the one we already found.
    TBOX search_box(box.left(), box.top(), box.right(), top_limit);
    search.StartRectSearch(search_box);
    while ((part = search.NextRectSearch()) != nullptr) {
      if (part->flow() == BTFT_STRONG_CHAIN || part->flow() == BTFT_CHAIN) {
        const TBOX &part_box(part->bounding_box());
        if (part_box.x_gap(box) < 0) {
          if (part_box.bottom() < top_limit && part_box.bottom() > box.top()) {
            top_limit = part_box.bottom();
          }
        }
      }
    }
  }
  return top_limit;
}

// Expands the image box in the given direction until it hits text,
// limiting the expansion to the given limit box, returning the result
// in the expanded box, and
// returning the increase in area resulting from the expansion.
static int ExpandImageDir(BlobNeighbourDir dir, const TBOX &im_box, const TBOX &limit_box,
                          ColPartitionGrid *part_grid, TBOX *expanded_box) {
  *expanded_box = im_box;
  switch (dir) {
    case BND_LEFT:
      expanded_box->set_left(ExpandImageLeft(im_box, limit_box.left(), part_grid));
      break;
    case BND_RIGHT:
      expanded_box->set_right(ExpandImageRight(im_box, limit_box.right(), part_grid));
      break;
    case BND_ABOVE:
      expanded_box->set_top(ExpandImageTop(im_box, limit_box.top(), part_grid));
      break;
    case BND_BELOW:
      expanded_box->set_bottom(ExpandImageBottom(im_box, limit_box.bottom(), part_grid));
      break;
    default:
      return 0;
  }
  return expanded_box->area() - im_box.area();
}

// Expands the image partition into any non-text until it touches text.
// The expansion proceeds in the order of increasing increase in area
// as a heuristic to find the best rectangle by expanding in the most
// constrained direction first.
static void MaximalImageBoundingBox(ColPartitionGrid *part_grid, TBOX *im_box) {
  bool dunnit[BND_COUNT];
  memset(dunnit, 0, sizeof(dunnit));
  TBOX limit_box(part_grid->bleft().x(), part_grid->bleft().y(), part_grid->tright().x(),
                 part_grid->tright().y());
  TBOX text_box(*im_box);
  for (int iteration = 0; iteration < BND_COUNT; ++iteration) {
    // Find the direction with least area increase.
    int best_delta = -1;
    BlobNeighbourDir best_dir = BND_LEFT;
    TBOX expanded_boxes[BND_COUNT];
    for (int dir = 0; dir < BND_COUNT; ++dir) {
      auto bnd = static_cast<BlobNeighbourDir>(dir);
      if (!dunnit[bnd]) {
        TBOX expanded_box;
        int area_delta = ExpandImageDir(bnd, text_box, limit_box, part_grid, &expanded_boxes[bnd]);
        if (best_delta < 0 || area_delta < best_delta) {
          best_delta = area_delta;
          best_dir = bnd;
        }
      }
    }
    // Run the best and remember the direction.
    dunnit[best_dir] = true;
    text_box = expanded_boxes[best_dir];
  }
  *im_box = text_box;
}

// Helper deletes the given partition but first marks up all the blobs as
// noise, so they get deleted later, and disowns them.
// If the initial type of the partition is image, then it actually deletes
// the blobs, as the partition owns them in that case.
static void DeletePartition(ColPartition *part) {
  BlobRegionType type = part->blob_type();
  if (type == BRT_RECTIMAGE || type == BRT_POLYIMAGE) {
    // The partition owns the boxes of these types, so just delete them.
    part->DeleteBoxes(); // From a previous iteration.
  } else {
    // Once marked, the blobs will be swept up by TidyBlobs.
    part->set_flow(BTFT_NONTEXT);
    part->set_blob_type(BRT_NOISE);
    part->SetBlobTypes();
    part->DisownBoxes(); // Created before FindImagePartitions.
  }
  delete part;
}

// The meat of joining fragmented images and consuming ColPartitions of
// uncertain type.
// *part_ptr is an input/output BRT_RECTIMAGE ColPartition that is to be
// expanded to consume overlapping and nearby ColPartitions of uncertain type
// and other BRT_RECTIMAGE partitions, but NOT to be expanded beyond
// max_image_box. *part_ptr is NOT in the part_grid.
// rectsearch is already constructed on the part_grid, and is used for
// searching for overlapping and nearby ColPartitions.
// ExpandImageIntoParts is called iteratively until it returns false. Each
// time it absorbs the nearest non-contained candidate, and everything that
// is fully contained within part_ptr's bounding box.
// TODO(rays) what if it just eats everything inside max_image_box in one go?
static bool ExpandImageIntoParts(const TBOX &max_image_box, ColPartitionGridSearch *rectsearch,
                                 ColPartitionGrid *part_grid, ColPartition **part_ptr) {
  ColPartition *image_part = *part_ptr;
  TBOX im_part_box = image_part->bounding_box();
  if (textord_tabfind_show_images > 1) {
    tprintf("Searching for merge with image part:");
    im_part_box.print();
    tprintf("Text box=");
    max_image_box.print();
  }
  rectsearch->StartRectSearch(max_image_box);
  ColPartition *part;
  ColPartition *best_part = nullptr;
  int best_dist = 0;
  while ((part = rectsearch->NextRectSearch()) != nullptr) {
    if (textord_tabfind_show_images > 1) {
      tprintf("Considering merge with part:");
      part->Print();
      if (im_part_box.contains(part->bounding_box())) {
        tprintf("Fully contained\n");
      } else if (!max_image_box.contains(part->bounding_box())) {
        tprintf("Not within text box\n");
      } else if (part->flow() == BTFT_STRONG_CHAIN) {
        tprintf("Too strong text\n");
      } else {
        tprintf("Real candidate\n");
      }
    }
    if (part->flow() == BTFT_STRONG_CHAIN || part->flow() == BTFT_TEXT_ON_IMAGE ||
        part->blob_type() == BRT_POLYIMAGE) {
      continue;
    }
    TBOX box = part->bounding_box();
    if (max_image_box.contains(box) && part->blob_type() != BRT_NOISE) {
      if (im_part_box.contains(box)) {
        // Eat it completely.
        rectsearch->RemoveBBox();
        DeletePartition(part);
        continue;
      }
      int x_dist = std::max(0, box.x_gap(im_part_box));
      int y_dist = std::max(0, box.y_gap(im_part_box));
      int dist = x_dist * x_dist + y_dist * y_dist;
      if (dist > box.area() || dist > im_part_box.area()) {
        continue; // Not close enough.
      }
      if (best_part == nullptr || dist < best_dist) {
        // We keep the nearest qualifier, which is not necessarily the nearest.
        best_part = part;
        best_dist = dist;
      }
    }
  }
  if (best_part != nullptr) {
    // It needs expanding. We can do it without touching text.
    TBOX box = best_part->bounding_box();
    if (textord_tabfind_show_images > 1) {
      tprintf("Merging image part:");
      im_part_box.print();
      tprintf("with part:");
      box.print();
    }
    im_part_box += box;
    *part_ptr = ColPartition::FakePartition(im_part_box, PT_UNKNOWN, BRT_RECTIMAGE, BTFT_NONTEXT);
    DeletePartition(image_part);
    part_grid->RemoveBBox(best_part);
    DeletePartition(best_part);
    rectsearch->RepositionIterator();
    return true;
  }
  return false;
}

// Helper function to compute the overlap area between the box and the
// given list of partitions.
static int IntersectArea(const TBOX &box, ColPartition_LIST *part_list) {
  int intersect_area = 0;
  ColPartition_IT part_it(part_list);
  // Iterate the parts and subtract intersecting area.
  for (part_it.mark_cycle_pt(); !part_it.cycled_list(); part_it.forward()) {
    ColPartition *image_part = part_it.data();
    TBOX intersect = box.intersection(image_part->bounding_box());
    intersect_area += intersect.area();
  }
  return intersect_area;
}

// part_list is a set of ColPartitions representing a polygonal image, and
// im_box is the union of the bounding boxes of all the parts in part_list.
// Tests whether part is to be consumed by the polygonal image.
// Returns true if part is weak text and more than half of its area is
// intersected by parts from the part_list, and it is contained within im_box.
static bool TestWeakIntersectedPart(const TBOX &im_box, ColPartition_LIST *part_list,
                                    ColPartition *part) {
  if (part->flow() < BTFT_STRONG_CHAIN) {
    // A weak partition intersects the box.
    const TBOX &part_box = part->bounding_box();
    if (im_box.contains(part_box)) {
      int area = part_box.area();
      int intersect_area = IntersectArea(part_box, part_list);
      if (area < 2 * intersect_area) {
        return true;
      }
    }
  }
  return false;
}

// A rectangular or polygonal image has been completed, in part_list, bounding
// box in im_box. We want to eliminate weak text or other uncertain partitions
// (basically anything that is not BRT_STRONG_CHAIN or better) from both the
// part_grid and the big_parts list that are contained within im_box and
// overlapped enough by the possibly polygonal image.
static void EliminateWeakParts(const TBOX &im_box, ColPartitionGrid *part_grid,
                               ColPartition_LIST *big_parts, ColPartition_LIST *part_list) {
  ColPartitionGridSearch rectsearch(part_grid);
  ColPartition *part;
  rectsearch.StartRectSearch(im_box);
  while ((part = rectsearch.NextRectSearch()) != nullptr) {
    if (TestWeakIntersectedPart(im_box, part_list, part)) {
      BlobRegionType type = part->blob_type();
      if (type == BRT_POLYIMAGE || type == BRT_RECTIMAGE) {
        rectsearch.RemoveBBox();
        DeletePartition(part);
      } else {
        // The part is mostly covered, so mark it. Non-image partitions are
        // kept hanging around to mark the image for pass2
        part->set_flow(BTFT_NONTEXT);
        part->set_blob_type(BRT_NOISE);
        part->SetBlobTypes();
      }
    }
  }
  ColPartition_IT big_it(big_parts);
  for (big_it.mark_cycle_pt(); !big_it.cycled_list(); big_it.forward()) {
    part = big_it.data();
    if (TestWeakIntersectedPart(im_box, part_list, part)) {
      // Once marked, the blobs will be swept up by TidyBlobs.
      DeletePartition(big_it.extract());
    }
  }
}

// Helper scans for good text partitions overlapping the given box.
// If there are no good text partitions overlapping an expanded box, then
// the box is expanded, otherwise, the original box is returned.
// If good text overlaps the box, true is returned.
static bool ScanForOverlappingText(ColPartitionGrid *part_grid, TBOX *box) {
  ColPartitionGridSearch rectsearch(part_grid);
  TBOX padded_box(*box);
  padded_box.pad(kNoisePadding, kNoisePadding);
  rectsearch.StartRectSearch(padded_box);
  ColPartition *part;
  bool any_text_in_padded_rect = false;
  while ((part = rectsearch.NextRectSearch()) != nullptr) {
    if (part->flow() == BTFT_CHAIN || part->flow() == BTFT_STRONG_CHAIN) {
      // Text intersects the box.
      any_text_in_padded_rect = true;
      const TBOX &part_box = part->bounding_box();
      if (box->overlap(part_box)) {
        return true;
      }
    }
  }
  if (!any_text_in_padded_rect) {
    *box = padded_box;
  }
  return false;
}

// Renders the boxes of image parts from the supplied list onto the image_pix,
// except where they interfere with existing strong text in the part_grid,
// and then deletes them.
// Box coordinates are rotated by rerotate to match the image.
static void MarkAndDeleteImageParts(const FCOORD &rerotate, ColPartitionGrid *part_grid,
                                    ColPartition_LIST *image_parts, Image image_pix) {
  if (image_pix == nullptr) {
    return;
  }
  int imageheight = pixGetHeight(image_pix);
  ColPartition_IT part_it(image_parts);
  for (; !part_it.empty(); part_it.forward()) {
    ColPartition *part = part_it.extract();
    TBOX part_box = part->bounding_box();
    BlobRegionType type = part->blob_type();
    if (!ScanForOverlappingText(part_grid, &part_box) || type == BRT_RECTIMAGE ||
        type == BRT_POLYIMAGE) {
      // Mark the box on the image.
      // All coords need to be rotated to match the image.
      part_box.rotate(rerotate);
      int left = part_box.left();
      int top = part_box.top();
      pixRasterop(image_pix, left, imageheight - top, part_box.width(), part_box.height(), PIX_SET,
                  nullptr, 0, 0);
    }
    DeletePartition(part);
  }
}

// Locates all the image partitions in the part_grid, that were found by a
// previous call to FindImagePartitions, marks them in the image_mask,
// removes them from the grid, and deletes them. This makes it possible to
// call FindImagePartitions again to produce less broken-up and less
// overlapping image partitions.
// rerotation specifies how to rotate the partition coords to match
// the image_mask, since this function is used after orientation correction.
void ImageFind::TransferImagePartsToImageMask(const FCOORD &rerotation, ColPartitionGrid *part_grid,
                                              Image image_mask) {
  // Extract the noise parts from the grid and put them on a temporary list.
  ColPartition_LIST parts_list;
  ColPartition_IT part_it(&parts_list);
  ColPartitionGridSearch gsearch(part_grid);
  gsearch.StartFullSearch();
  ColPartition *part;
  while ((part = gsearch.NextFullSearch()) != nullptr) {
    BlobRegionType type = part->blob_type();
    if (type == BRT_NOISE || type == BRT_RECTIMAGE || type == BRT_POLYIMAGE) {
      part_it.add_after_then_move(part);
      gsearch.RemoveBBox();
    }
  }
  // Render listed noise partitions to the image mask.
  MarkAndDeleteImageParts(rerotation, part_grid, &parts_list, image_mask);
}

// Removes and deletes all image partitions that are too small to be worth
// keeping. We have to do this as a separate phase after creating the image
// partitions as the small images are needed to join the larger ones together.
static void DeleteSmallImages(ColPartitionGrid *part_grid) {
  if (part_grid != nullptr) {
    return;
  }
  ColPartitionGridSearch gsearch(part_grid);
  gsearch.StartFullSearch();
  ColPartition *part;
  while ((part = gsearch.NextFullSearch()) != nullptr) {
    // Only delete rectangular images, since if it became a poly image, it
    // is more evidence that it is somehow important.
    if (part->blob_type() == BRT_RECTIMAGE) {
      const TBOX &part_box = part->bounding_box();
      if (part_box.width() < kMinImageFindSize || part_box.height() < kMinImageFindSize) {
        // It is too small to keep. Just make it disappear.
        gsearch.RemoveBBox();
        DeletePartition(part);
      }
    }
  }
}

// Runs a CC analysis on the image_pix mask image, and creates
// image partitions from them, cutting out strong text, and merging with
// nearby image regions such that they don't interfere with text.
// Rotation and rerotation specify how to rotate image coords to match
// the blob and partition coords and back again.
// The input/output part_grid owns all the created partitions, and
// the partitions own all the fake blobs that belong in the partitions.
// Since the other blobs in the other partitions will be owned by the block,
// ColPartitionGrid::ReTypeBlobs must be called afterwards to fix this
// situation and collect the image blobs.
void ImageFind::FindImagePartitions(Image image_pix, const FCOORD &rotation,
                                    const FCOORD &rerotation, TO_BLOCK *block, TabFind *tab_grid,
                                    DebugPixa *pixa_debug, ColPartitionGrid *part_grid,
                                    ColPartition_LIST *big_parts) {
  int imageheight = pixGetHeight(image_pix);
  Boxa *boxa;
  Pixa *pixa;
  ConnCompAndRectangularize(image_pix, pixa_debug, &boxa, &pixa);
  // Iterate the connected components in the image regions mask.
  int nboxes = 0;
  if (boxa != nullptr && pixa != nullptr) {
    nboxes = boxaGetCount(boxa);
  }
  for (int i = 0; i < nboxes; ++i) {
    l_int32 x, y, width, height;
    boxaGetBoxGeometry(boxa, i, &x, &y, &width, &height);
    Image pix = pixaGetPix(pixa, i, L_CLONE);
    TBOX im_box(x, imageheight - y - height, x + width, imageheight - y);
    im_box.rotate(rotation); // Now matches all partitions and blobs.
    ColPartitionGridSearch rectsearch(part_grid);
    rectsearch.SetUniqueMode(true);
    ColPartition_LIST part_list;
    DivideImageIntoParts(im_box, rotation, rerotation, pix, &rectsearch, &part_list);
    if (textord_tabfind_show_images && pixa_debug != nullptr) {
      pixa_debug->AddPix(pix, "ImageComponent");
      tprintf("Component has %d parts\n", part_list.length());
    }
    pix.destroy();
    if (!part_list.empty()) {
      ColPartition_IT part_it(&part_list);
      if (part_list.singleton()) {
        // We didn't have to chop it into a polygon to fit around text, so
        // try expanding it to merge fragmented image parts, as long as it
        // doesn't touch strong text.
        ColPartition *part = part_it.extract();
        TBOX text_box(im_box);
        MaximalImageBoundingBox(part_grid, &text_box);
        while (ExpandImageIntoParts(text_box, &rectsearch, part_grid, &part)) {
          ;
        }
        part_it.set_to_list(&part_list);
        part_it.add_after_then_move(part);
        im_box = part->bounding_box();
      }
      EliminateWeakParts(im_box, part_grid, big_parts, &part_list);
      // Iterate the part_list and put the parts into the grid.
      for (part_it.move_to_first(); !part_it.empty(); part_it.forward()) {
        ColPartition *image_part = part_it.extract();
        im_box = image_part->bounding_box();
        part_grid->InsertBBox(true, true, image_part);
        if (!part_it.at_last()) {
          ColPartition *neighbour = part_it.data_relative(1);
          image_part->AddPartner(false, neighbour);
          neighbour->AddPartner(true, image_part);
        }
      }
    }
  }
  boxaDestroy(&boxa);
  pixaDestroy(&pixa);
  DeleteSmallImages(part_grid);
#ifndef GRAPHICS_DISABLED
  if (textord_tabfind_show_images) {
    ScrollView *images_win_ = part_grid->MakeWindow(1000, 400, "With Images");
    part_grid->DisplayBoxes(images_win_);
  }
#endif
}

} // namespace tesseract.
