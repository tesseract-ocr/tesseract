///////////////////////////////////////////////////////////////////////
// File:        imagefind.cpp
// Description: Function to find image and drawing regions in an image
//              and create a corresponding list of empty blobs.
// Author:      Ray Smith
// Created:     Thu Mar 20 09:49:01 PDT 2008
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

#ifdef _MSC_VER
#pragma warning(disable:4244)  // Conversion warnings
#endif

#include "imagefind.h"
#include "varable.h"

// This entire file is dependent upon leptonica. If you don't have it,
// you don't get this functionality.
#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif
#ifdef HAVE_LIBLEPT
#include "allheaders.h"
#endif

BOOL_VAR(textord_tabfind_show_images, false, "Show image blobs");

namespace tesseract {

// Fraction of width or height of on pixels that can be discarded from a
// roughly rectangular image.
const double kMinRectangularFraction = 0.125;
// Fraction of width or height to consider image completely used.
const double kMaxRectangularFraction = 0.75;
// Fraction of width or height to allow transition from kMinRectangularFraction
// to kMaxRectangularFraction, equivalent to a dy/dx skew.
const double kMaxRectangularGradient = 0.1;  // About 6 degrees.
// Minimum image size to be worth looking for images on.
const int kMinImageFindSize = 100;

// Finds image regions within the source pix (page image) and returns
// the image regions as a Boxa, Pixa pair, analgous to pixConnComp.
// The returned boxa, pixa may be NULL, meaning no images found.
// If not NULL, they must be destroyed by the caller.
void ImageFinder::FindImages(Pix* pix, Boxa** boxa, Pixa** pixa) {
  *boxa = NULL;
  *pixa = NULL;

#ifdef HAVE_LIBLEPT
  if (pixGetWidth(pix) < kMinImageFindSize ||
      pixGetHeight(pix) < kMinImageFindSize)
    return;  // Not worth looking at small images.
  // Reduce by factor 2.
  Pix *pixr = pixReduceRankBinaryCascade(pix, 1, 0, 0, 0);
  pixDisplayWrite(pixr, textord_tabfind_show_images);

  // Get the halftone mask directly from Leptonica.
  Pix *pixht2 = pixGenHalftoneMask(pixr, NULL, NULL,
                                   textord_tabfind_show_images);
  pixDestroy(&pixr);
  if (pixht2 == NULL)
    return;

  // Expand back up again.
  Pix *pixht = pixExpandReplicate(pixht2, 2);
  pixDisplayWrite(pixht, textord_tabfind_show_images);
  pixDestroy(&pixht2);

  // Fill to capture pixels near the mask edges that were missed
  Pix *pixt = pixSeedfillBinary(NULL, pixht, pix, 8);
  pixOr(pixht, pixht, pixt);
  pixDestroy(&pixt);

  // Eliminate lines and bars that may be joined to images.
  Pix* pixfinemask = pixReduceRankBinaryCascade(pixht, 1, 1, 3, 3);
  pixDilateBrick(pixfinemask, pixfinemask, 5, 5);
  pixDisplayWrite(pixfinemask, textord_tabfind_show_images);
  Pix* pixreduced = pixReduceRankBinaryCascade(pixht, 1, 1, 1, 1);
  Pix* pixreduced2 = pixReduceRankBinaryCascade(pixreduced, 3, 3, 3, 0);
  pixDestroy(&pixreduced);
  pixDilateBrick(pixreduced2, pixreduced2, 5, 5);
  Pix* pixcoarsemask = pixExpandReplicate(pixreduced2, 8);
  pixDestroy(&pixreduced2);
  pixDisplayWrite(pixcoarsemask, textord_tabfind_show_images);
  // Combine the coarse and fine image masks.
  pixAnd(pixcoarsemask, pixcoarsemask, pixfinemask);
  pixDestroy(&pixfinemask);
  // Dilate a bit to make sure we get everything.
  pixDilateBrick(pixcoarsemask, pixcoarsemask, 3, 3);
  Pix* pixmask = pixExpandReplicate(pixcoarsemask, 16);
  pixDestroy(&pixcoarsemask);
  pixDisplayWrite(pixmask, textord_tabfind_show_images);
  // And the image mask with the line and bar remover.
  pixAnd(pixht, pixht, pixmask);
  pixDestroy(&pixmask);
  pixDisplayWrite(pixht, textord_tabfind_show_images);
  // Find the individual image regions in the mask image.
  *boxa = pixConnComp(pixht, pixa, 8);
  pixDestroy(&pixht);
  // Rectangularize the individual images. If a sharp edge in vertical and/or
  // horizontal occupancy can be found, it indicates a probably rectangular
  // image with unwanted bits merged on, so clip to the approximate rectangle.
  int npixes = pixaGetCount(*pixa);
  for (int i = 0; i < npixes; ++i) {
    int x_start, x_end, y_start, y_end;
    Pix* img_pix = pixaGetPix(*pixa, i, L_CLONE);
    pixDisplayWrite(img_pix, textord_tabfind_show_images);
    if (pixNearlyRectangular(img_pix, kMinRectangularFraction,
                             kMaxRectangularFraction,
                             kMaxRectangularGradient,
                             &x_start, &y_start, &x_end, &y_end)) {
      // Add 1 to the size as a kludgy flag to indicate to the later stages
      // of processing that it is a clipped rectangular image .
      Pix* simple_pix = pixCreate(pixGetWidth(img_pix) + 1,
                                  pixGetHeight(img_pix), 1);
      pixDestroy(&img_pix);
      pixRasterop(simple_pix, x_start, y_start, x_end - x_start,
                  y_end - y_start, PIX_SET, NULL, 0, 0);
      // pixaReplacePix takes ownership of the simple_pix.
      pixaReplacePix(*pixa, i, simple_pix, NULL);
      img_pix = pixaGetPix(*pixa, i, L_CLONE);
    }
    // Subtract the pix from the correct location in the master image.
    l_int32 x, y, width, height;
    pixDisplayWrite(img_pix, textord_tabfind_show_images);
    boxaGetBoxGeometry(*boxa, i, &x, &y, &width, &height);
    pixRasterop(pix, x, y, width, height, PIX_NOT(PIX_SRC) & PIX_DST,
                img_pix, 0, 0);
    pixDestroy(&img_pix);
  }
#endif
}

#ifdef HAVE_LIBLEPT
// Scans horizontally on x=[x_start,x_end), starting with y=*y_start,
// stepping y+=y_step, until y=y_end. *ystart is input/output.
// If the number of black pixels in a row, pix_count fits this pattern:
// 0 or more rows with pix_count < min_count then
// <= mid_width rows with min_count <= pix_count <= max_count then
// a row with pix_count > max_count then
// true is returned, and *y_start = the first y with pix_count >= min_count.
static bool HScanForEdge(uinT32* data, int wpl, int x_start, int x_end,
                         int min_count, int mid_width, int max_count,
                         int y_end, int y_step, int* y_start) {
  int mid_rows = 0;
  for (int y = *y_start; y != y_end; y += y_step) {
    // Need pixCountPixelsInRow(pix, y, &pix_count, NULL) to count in a subset.
    int pix_count = 0;
    uinT32* line = data + wpl * y;
    for (int x = x_start; x < x_end; ++x) {
      if (GET_DATA_BIT(line, x))
        ++pix_count;
    }
    if (mid_rows == 0 && pix_count < min_count)
      continue;      // In the min phase.
    if (mid_rows == 0)
      *y_start = y;  // Save the y_start where we came out of the min phase.
    if (pix_count > max_count)
      return true;   // Found the pattern.
    ++mid_rows;
    if (mid_rows > mid_width)
      break;         // Middle too big.
  }
  return false;      // Never found max_count.
}

// Scans vertically on y=[y_start,y_end), starting with x=*x_start,
// stepping x+=x_step, until x=x_end. *x_start is input/output.
// If the number of black pixels in a column, pix_count fits this pattern:
// 0 or more cols with pix_count < min_count then
// <= mid_width cols with min_count <= pix_count <= max_count then
// a column with pix_count > max_count then
// true is returned, and *x_start = the first x with pix_count >= min_count.
static bool VScanForEdge(uinT32* data, int wpl, int y_start, int y_end,
                         int min_count, int mid_width, int max_count,
                         int x_end, int x_step, int* x_start) {
  int mid_cols = 0;
  for (int x = *x_start; x != x_end; x += x_step) {
    int pix_count = 0;
    uinT32* line = data + y_start * wpl;
    for (int y = y_start; y < y_end; ++y, line += wpl) {
      if (GET_DATA_BIT(line, x))
        ++pix_count;
    }
    if (mid_cols == 0 && pix_count < min_count)
      continue;      // In the min phase.
    if (mid_cols == 0)
      *x_start = x;  // Save the place where we came out of the min phase.
    if (pix_count > max_count)
      return true;   // found the pattern.
    ++mid_cols;
    if (mid_cols > mid_width)
      break;         // Middle too big.
  }
  return false;      // Never found max_count.
}
#endif

// Returns true if there is a rectangle in the source pix, such that all
// pixel rows and column slices outside of it have less than
// min_fraction of the pixels black, and within max_skew_gradient fraction
// of the pixels on the inside, there are at least max_fraction of the
// pixels black. In other words, the inside of the rectangle looks roughly
// rectangular, and the outside of it looks like extra bits.
// On return, the rectangle is defined by x_start, y_start, x_end and y_end.
// Note: the algorithm is iterative, allowing it to slice off pixels from
// one edge, allowing it to then slice off more pixels from another edge.
bool ImageFinder::pixNearlyRectangular(Pix* pix,
                                       double min_fraction, double max_fraction,
                                       double max_skew_gradient,
                                       int* x_start, int* y_start,
                                       int* x_end, int* y_end) {
#ifdef HAVE_LIBLEPT
  *x_start = 0;
  *x_end = pixGetWidth(pix);
  *y_start = 0;
  *y_end = pixGetHeight(pix);

  uinT32* data = pixGetData(pix);
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
    if (!top_done && HScanForEdge(data, wpl, *x_start, *x_end,
                                  min_count, edge_width, max_count,
                                  *y_end, 1, y_start)) {
      top_done = true;
      any_cut = true;
    }
    --(*y_end);
    if (!bottom_done && HScanForEdge(data, wpl, *x_start, *x_end,
                                     min_count, edge_width, max_count,
                                     *y_start, -1, y_end)) {
      bottom_done = true;
      any_cut = true;
    }
    ++(*y_end);

    // Find the left/right edges.
    int height = *y_end - *y_start;
    min_count = static_cast<int>(height * min_fraction);
    max_count = static_cast<int>(height * max_fraction);
    edge_width = static_cast<int>(height * max_skew_gradient);
    if (!left_done && VScanForEdge(data, wpl, *y_start, *y_end,
                                   min_count, edge_width, max_count,
                                   *x_end, 1, x_start)) {
      left_done = true;
      any_cut = true;
    }
    --(*x_end);
    if (!right_done && VScanForEdge(data, wpl, *y_start, *y_end,
                                    min_count, edge_width, max_count,
                                    *x_start, -1, x_end)) {
      right_done = true;
      any_cut = true;
    }
    ++(*x_end);
  } while (any_cut);

  // All edges must satisfy the condition of sharp gradient in pixel density
  // in order for the full rectangle to be present.
  return left_done && right_done && top_done && bottom_done;
#else
  return false;
#endif
}

#ifdef HAVE_LIBLEPT
// Scanning rows horizontally on x=[x_start, x_end), returns the first y row
// starting at y_start, stepping by y_step to y_end in which there is
// any black pixel.
static int HScanForBlack(uinT32* data, int wpl, int x_start, int x_end,
                         int y_start, int y_end, int y_step) {
  for (int y = y_start; y != y_end; y += y_step) {
    uinT32* line = data + wpl * y;
    for (int x = x_start; x < x_end; ++x) {
      if (GET_DATA_BIT(line, x))
        return y;
    }
  }
  return y_end;
}

// Scanning columns vertically on y=[y_start, y_end), returns the first x
// colum starting at x_start, stepping by x_step to x_end in which there is
// any black pixel.
static int VScanForBlack(uinT32* data, int wpl, int x_start, int x_end,
                         int y_start, int y_end, int x_step) {
  for (int x = x_start; x != x_end; x += x_step) {
    uinT32* line = data + y_start * wpl;
    for (int y = y_start; y < y_end; ++y, line += wpl) {
      if (GET_DATA_BIT(line, x))
        return x;
    }
  }
  return x_end;
}
#endif

// Given an input pix, and a bounding rectangle, the sides of the rectangle
// are shrunk inwards until they bound any black pixels found within the
// original rectangle.
void ImageFinder::BoundsWithinRect(Pix* pix, int* x_start, int* y_start,
                                   int* x_end, int* y_end) {
#ifdef HAVE_LIBLEPT
  // This can probably be done with a lot less code using pixClipRect and
  // pixConnComp, but this code is probably a lot faster, given that most
  // uses will be applied to a solid black region.
  int width = pixGetWidth(pix);
  int height = pixGetHeight(pix);
  if (*x_start < 0) *x_start = 0;
  if (*x_end > width) *x_end = width;
  if (*y_start < 0) *y_start = 0;
  if (*y_end > height) *y_end = height;

  uinT32* data = pixGetData(pix);
  int wpl = pixGetWpl(pix);
  *y_start = HScanForBlack(data, wpl, *x_start, *x_end, *y_start, *y_end, 1);
  if (*y_end <= *y_start)
    return;
  *y_end = HScanForBlack(data, wpl, *x_start, *x_end,
                         *y_end - 1, *y_start - 1, -1) + 1;
  *x_start = VScanForBlack(data, wpl, *x_start, *x_end, *y_start, *y_end, 1);
  *x_end = VScanForBlack(data, wpl, *x_end - 1, *x_start - 1,
                         *y_start, *y_end, -1) + 1;
#endif
}

}  // namespace tesseract.

