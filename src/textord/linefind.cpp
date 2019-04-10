///////////////////////////////////////////////////////////////////////
// File:        linefind.cpp
// Description: Class to find vertical lines in an image and create
//              a corresponding list of empty blobs.
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

#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif

#include "linefind.h"
#include "alignedblob.h"
#include "tabvector.h"
#include "blobbox.h"
#include "edgblob.h"
#if defined(USE_OPENCL)
#include "openclwrapper.h" // for OpenclDevice
#endif

#include "allheaders.h"

#include <algorithm>

namespace tesseract {

/// Denominator of resolution makes max pixel width to allow thin lines.
const int kThinLineFraction = 20;
/// Denominator of resolution makes min pixels to demand line lengths to be.
const int kMinLineLengthFraction = 4;
/// Spacing of cracks across the page to break up tall vertical lines.
const int kCrackSpacing = 100;
/// Grid size used by line finder. Not very critical.
const int kLineFindGridSize = 50;
// Min width of a line in pixels to be considered thick.
const int kMinThickLineWidth = 12;
// Max size of line residue. (The pixels that fail the long thin opening, and
// therefore don't make it to the candidate line mask, but are nevertheless
// part of the line.)
const int kMaxLineResidue = 6;
// Min length in inches of a line segment that exceeds kMinThickLineWidth in
// thickness. (Such lines shouldn't break by simple image degradation.)
const double kThickLengthMultiple = 0.75;
// Max fraction of line box area that can be occupied by non-line pixels.
const double kMaxNonLineDensity = 0.25;
// Max height of a music stave in inches.
const double kMaxStaveHeight = 1.0;
// Minimum fraction of pixels in a music rectangle connected to the staves.
const double kMinMusicPixelFraction = 0.75;

// Erases the unused blobs from the line_pix image, taking into account
// whether this was a horizontal or vertical line set.
static void RemoveUnusedLineSegments(bool horizontal_lines,
                                     BLOBNBOX_LIST* line_bblobs,
                                     Pix* line_pix) {
  int height = pixGetHeight(line_pix);
  BLOBNBOX_IT bbox_it(line_bblobs);
  for (bbox_it.mark_cycle_pt(); !bbox_it.cycled_list(); bbox_it.forward()) {
    BLOBNBOX* blob = bbox_it.data();
    if (blob->left_tab_type() != TT_VLINE) {
      const TBOX& box = blob->bounding_box();
      Box* pixbox = nullptr;
      if (horizontal_lines) {
        // Horizontal lines are in tess format and also have x and y flipped
        // (to use FindVerticalAlignment) so we have to flip x and y and then
        // convert to Leptonica by height - flipped x (ie the right edge).
        // See GetLineBoxes for more explanation.
        pixbox = boxCreate(box.bottom(), height - box.right(),
                           box.height(), box.width());
      } else {
        // For vertical lines, just flip upside-down to convert to Leptonica.
        // The y position of the box in Leptonica terms is the distance from
        // the top of the image to the top of the box.
        pixbox = boxCreate(box.left(), height - box.top(),
                           box.width(), box.height());
      }
      pixClearInRect(line_pix, pixbox);
      boxDestroy(&pixbox);
    }
  }
}

// Helper subtracts the line_pix image from the src_pix, and removes residue
// as well by removing components that touch the line, but are not in the
// non_line_pix mask. It is assumed that the non_line_pix mask has already
// been prepared to required accuracy.
static void SubtractLinesAndResidue(Pix* line_pix, Pix* non_line_pix,
                                    int resolution, Pix* src_pix) {
  // First remove the lines themselves.
  pixSubtract(src_pix, src_pix, line_pix);
  // Subtract the non-lines from the image to get the residue.
  Pix* residue_pix = pixSubtract(nullptr, src_pix, non_line_pix);
  // Dilate the lines so they touch the residue.
  Pix* fat_line_pix = pixDilateBrick(nullptr, line_pix, 3, 3);
  // Seed fill the fat lines to get all the residue.
  pixSeedfillBinary(fat_line_pix, fat_line_pix, residue_pix, 8);
  // Subtract the residue from the original image.
  pixSubtract(src_pix, src_pix, fat_line_pix);
  pixDestroy(&fat_line_pix);
  pixDestroy(&residue_pix);
}

// Returns the maximum strokewidth in the given binary image by doubling
// the maximum of the distance function.
static int MaxStrokeWidth(Pix* pix) {
  Pix* dist_pix = pixDistanceFunction(pix, 4, 8, L_BOUNDARY_BG);
  int width = pixGetWidth(dist_pix);
  int height = pixGetHeight(dist_pix);
  int wpl = pixGetWpl(dist_pix);
  l_uint32* data = pixGetData(dist_pix);
  // Find the maximum value in the distance image.
  int max_dist = 0;
  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      int pixel = GET_DATA_BYTE(data, x);
      if (pixel > max_dist)
        max_dist = pixel;
    }
    data += wpl;
  }
  pixDestroy(&dist_pix);
  return max_dist * 2;
}

// Returns the number of components in the intersection_pix touched by line_box.
static int NumTouchingIntersections(Box* line_box, Pix* intersection_pix) {
  if (intersection_pix == nullptr) return 0;
  Pix* rect_pix = pixClipRectangle(intersection_pix, line_box, nullptr);
  Boxa* boxa = pixConnComp(rect_pix, nullptr, 8);
  pixDestroy(&rect_pix);
  if (boxa == nullptr) return false;
  int result = boxaGetCount(boxa);
  boxaDestroy(&boxa);
  return result;
}

// Returns the number of black pixels found in the box made by adding the line
// width to both sides of the line bounding box. (Increasing the smallest
// dimension of the bounding box.)
static int CountPixelsAdjacentToLine(int line_width, Box* line_box,
                                     Pix* nonline_pix) {
  l_int32 x, y, box_width, box_height;
  boxGetGeometry(line_box, &x, &y, &box_width, &box_height);
  if (box_width > box_height) {
    // horizontal line.
    int bottom = std::min(pixGetHeight(nonline_pix), y + box_height + line_width);
    y = std::max(0, y - line_width);
    box_height = bottom - y;
  } else {
    // Vertical line.
    int right = std::min(pixGetWidth(nonline_pix), x + box_width + line_width);
    x = std::max(0, x - line_width);
    box_width = right - x;
  }
  Box* box = boxCreate(x, y, box_width, box_height);
  Pix* rect_pix = pixClipRectangle(nonline_pix, box, nullptr);
  boxDestroy(&box);
  l_int32 result;
  pixCountPixels(rect_pix, &result, nullptr);
  pixDestroy(&rect_pix);
  return result;
}

// Helper erases false-positive line segments from the input/output line_pix.
// 1. Since thick lines shouldn't really break up, we can eliminate some false
//    positives by marking segments that are at least kMinThickLineWidth
//    thickness, yet have a length less than min_thick_length.
// 2. Lines that don't have at least 2 intersections with other lines and have
//    a lot of neighbouring non-lines are probably not lines (perhaps arabic
//    or Hindi words, or underlines.)
// Bad line components are erased from line_pix.
// Returns the number of remaining connected components.
static int FilterFalsePositives(int resolution, Pix* nonline_pix,
                                Pix* intersection_pix, Pix* line_pix) {
  int min_thick_length = static_cast<int>(resolution * kThickLengthMultiple);
  Pixa* pixa = nullptr;
  Boxa* boxa = pixConnComp(line_pix, &pixa, 8);
  // Iterate over the boxes to remove false positives.
  int nboxes = boxaGetCount(boxa);
  int remaining_boxes = nboxes;
  for (int i = 0; i < nboxes; ++i) {
    Box* box = boxaGetBox(boxa, i, L_CLONE);
    l_int32 x, y, box_width, box_height;
    boxGetGeometry(box, &x, &y, &box_width, &box_height);
    Pix* comp_pix = pixaGetPix(pixa, i, L_CLONE);
    int max_width = MaxStrokeWidth(comp_pix);
    pixDestroy(&comp_pix);
    bool bad_line = false;
    // If the length is too short to stand-alone as a line, and the box width
    // is thick enough, and the stroke width is thick enough it is bad.
    if (box_width >= kMinThickLineWidth && box_height >= kMinThickLineWidth &&
        box_width < min_thick_length && box_height < min_thick_length &&
        max_width > kMinThickLineWidth) {
      // Too thick for the length.
      bad_line = true;
    }
    if (!bad_line &&
        (intersection_pix == nullptr ||
        NumTouchingIntersections(box, intersection_pix) < 2)) {
      // Test non-line density near the line.
      int nonline_count = CountPixelsAdjacentToLine(max_width, box,
                                                    nonline_pix);
      if (nonline_count > box_height * box_width * kMaxNonLineDensity)
        bad_line = true;
    }
    if (bad_line) {
      // Not a good line.
      pixClearInRect(line_pix, box);
      --remaining_boxes;
    }
    boxDestroy(&box);
  }
  pixaDestroy(&pixa);
  boxaDestroy(&boxa);
  return remaining_boxes;
}

// Finds vertical and horizontal line objects in the given pix.
// Uses the given resolution to determine size thresholds instead of any
// that may be present in the pix.
// The output vertical_x and vertical_y contain a sum of the output vectors,
// thereby giving the mean vertical direction.
// If pix_music_mask != nullptr, and music is detected, a mask of the staves
// and anything that is connected (bars, notes etc.) will be returned in
// pix_music_mask, the mask subtracted from pix, and the lines will not
// appear in v_lines or h_lines.
// The output vectors are owned by the list and Frozen (cannot refit) by
// having no boxes, as there is no need to refit or merge separator lines.
// The detected lines are removed from the pix.
void LineFinder::FindAndRemoveLines(int resolution, bool debug, Pix* pix,
                                    int* vertical_x, int* vertical_y,
                                    Pix** pix_music_mask,
                                    TabVector_LIST* v_lines,
                                    TabVector_LIST* h_lines) {
  if (pix == nullptr || vertical_x == nullptr || vertical_y == nullptr) {
    tprintf("Error in parameters for LineFinder::FindAndRemoveLines\n");
    return;
  }
  Pix* pix_vline = nullptr;
  Pix* pix_non_vline = nullptr;
  Pix* pix_hline = nullptr;
  Pix* pix_non_hline = nullptr;
  Pix* pix_intersections = nullptr;
  Pixa* pixa_display = debug ? pixaCreate(0) : nullptr;
  GetLineMasks(resolution, pix, &pix_vline, &pix_non_vline, &pix_hline,
               &pix_non_hline, &pix_intersections, pix_music_mask,
               pixa_display);
  // Find lines, convert to TabVector_LIST and remove those that are used.
  FindAndRemoveVLines(resolution, pix_intersections, vertical_x, vertical_y,
                      &pix_vline, pix_non_vline, pix, v_lines);
  if (pix_hline != nullptr) {
    // Recompute intersections and re-filter false positive h-lines.
    if (pix_vline != nullptr)
      pixAnd(pix_intersections, pix_vline, pix_hline);
    else
      pixDestroy(&pix_intersections);
    if (!FilterFalsePositives(resolution, pix_non_hline, pix_intersections,
                              pix_hline)) {
      pixDestroy(&pix_hline);
    }
  }
  FindAndRemoveHLines(resolution, pix_intersections, *vertical_x, *vertical_y,
                      &pix_hline, pix_non_hline, pix, h_lines);
  if (pixa_display != nullptr && pix_vline != nullptr)
    pixaAddPix(pixa_display, pix_vline, L_CLONE);
  if (pixa_display != nullptr && pix_hline != nullptr)
    pixaAddPix(pixa_display, pix_hline, L_CLONE);
  if (pix_vline != nullptr && pix_hline != nullptr) {
    // Remove joins (intersections) where lines cross, and the residue.
    // Recalculate the intersections, since some lines have been deleted.
    pixAnd(pix_intersections, pix_vline, pix_hline);
    // Fatten up the intersections and seed-fill to get the intersection
    // residue.
    Pix* pix_join_residue = pixDilateBrick(nullptr, pix_intersections, 5, 5);
    pixSeedfillBinary(pix_join_residue, pix_join_residue, pix, 8);
    // Now remove the intersection residue.
    pixSubtract(pix, pix, pix_join_residue);
    pixDestroy(&pix_join_residue);
  }
  // Remove any detected music.
  if (pix_music_mask != nullptr && *pix_music_mask != nullptr) {
    if (pixa_display != nullptr)
      pixaAddPix(pixa_display, *pix_music_mask, L_CLONE);
    pixSubtract(pix, pix, *pix_music_mask);
  }
  if (pixa_display != nullptr)
    pixaAddPix(pixa_display, pix, L_CLONE);

  pixDestroy(&pix_vline);
  pixDestroy(&pix_non_vline);
  pixDestroy(&pix_hline);
  pixDestroy(&pix_non_hline);
  pixDestroy(&pix_intersections);
  if (pixa_display != nullptr) {
    pixaConvertToPdf(pixa_display, resolution, 1.0f, 0, 0, "LineFinding",
                     "vhlinefinding.pdf");
    pixaDestroy(&pixa_display);
  }
}

// Converts the Boxa array to a list of C_BLOB, getting rid of severely
// overlapping outlines and those that are children of a bigger one.
// The output is a list of C_BLOBs that are owned by the list.
// The C_OUTLINEs in the C_BLOBs contain no outline data - just empty
// bounding boxes. The Boxa is consumed and destroyed.
void LineFinder::ConvertBoxaToBlobs(int image_width, int image_height,
                                    Boxa** boxes, C_BLOB_LIST* blobs) {
  C_OUTLINE_LIST outlines;
  C_OUTLINE_IT ol_it = &outlines;
  // Iterate the boxes to convert to outlines.
  int nboxes = boxaGetCount(*boxes);
  for (int i = 0; i < nboxes; ++i) {
    l_int32 x, y, width, height;
    boxaGetBoxGeometry(*boxes, i, &x, &y, &width, &height);
    // Make a C_OUTLINE from the leptonica box. This is a bit of a hack,
    // as there is no outline, just a bounding box, but with some very
    // small changes to coutln.cpp, it works nicely.
    ICOORD top_left(x, y);
    ICOORD bot_right(x + width, y + height);
    CRACKEDGE startpt;
    startpt.pos = top_left;
    auto* outline = new C_OUTLINE(&startpt, top_left, bot_right, 0);
    ol_it.add_after_then_move(outline);
  }
  // Use outlines_to_blobs to convert the outlines to blobs and find
  // overlapping and contained objects. The output list of blobs in the block
  // has all the bad ones filtered out and deleted.
  BLOCK block;
  ICOORD page_tl(0, 0);
  ICOORD page_br(image_width, image_height);
  outlines_to_blobs(&block, page_tl, page_br, &outlines);
  // Transfer the created blobs to the output list.
  C_BLOB_IT blob_it(blobs);
  blob_it.add_list_after(block.blob_list());
  // The boxes aren't needed any more.
  boxaDestroy(boxes);
}

// Finds vertical line objects in pix_vline and removes the from src_pix.
// Uses the given resolution to determine size thresholds instead of any
// that may be present in the pix.
// The output vertical_x and vertical_y contain a sum of the output vectors,
// thereby giving the mean vertical direction.
// The output vectors are owned by the list and Frozen (cannot refit) by
// having no boxes, as there is no need to refit or merge separator lines.
// If no good lines are found, pix_vline is destroyed.
// None of the input pointers may be nullptr, and if *pix_vline is nullptr then
// the function does nothing.
void LineFinder::FindAndRemoveVLines(int resolution,
                                     Pix* pix_intersections,
                                     int* vertical_x, int* vertical_y,
                                     Pix** pix_vline, Pix* pix_non_vline,
                                     Pix* src_pix, TabVector_LIST* vectors) {
  if (pix_vline == nullptr || *pix_vline == nullptr) return;
  C_BLOB_LIST line_cblobs;
  BLOBNBOX_LIST line_bblobs;
  GetLineBoxes(false, *pix_vline, pix_intersections,
               &line_cblobs, &line_bblobs);
  int width = pixGetWidth(src_pix);
  int height = pixGetHeight(src_pix);
  ICOORD bleft(0, 0);
  ICOORD tright(width, height);
  FindLineVectors(bleft, tright, &line_bblobs, vertical_x, vertical_y, vectors);
  if (!vectors->empty()) {
    RemoveUnusedLineSegments(false, &line_bblobs, *pix_vline);
    SubtractLinesAndResidue(*pix_vline, pix_non_vline, resolution, src_pix);
    ICOORD vertical;
    vertical.set_with_shrink(*vertical_x, *vertical_y);
    TabVector::MergeSimilarTabVectors(vertical, vectors, nullptr);
  } else {
    pixDestroy(pix_vline);
  }
}

// Finds horizontal line objects in pix_hline and removes them from src_pix.
// Uses the given resolution to determine size thresholds instead of any
// that may be present in the pix.
// The output vertical_x and vertical_y contain a sum of the output vectors,
// thereby giving the mean vertical direction.
// The output vectors are owned by the list and Frozen (cannot refit) by
// having no boxes, as there is no need to refit or merge separator lines.
// If no good lines are found, pix_hline is destroyed.
// None of the input pointers may be nullptr, and if *pix_hline is nullptr then
// the function does nothing.
void LineFinder::FindAndRemoveHLines(int resolution,
                                     Pix* pix_intersections,
                                     int vertical_x, int vertical_y,
                                     Pix** pix_hline, Pix* pix_non_hline,
                                     Pix* src_pix, TabVector_LIST* vectors) {
  if (pix_hline == nullptr || *pix_hline == nullptr) return;
  C_BLOB_LIST line_cblobs;
  BLOBNBOX_LIST line_bblobs;
  GetLineBoxes(true, *pix_hline, pix_intersections, &line_cblobs, &line_bblobs);
  int width = pixGetWidth(src_pix);
  int height = pixGetHeight(src_pix);
  ICOORD bleft(0, 0);
  ICOORD tright(height, width);
  FindLineVectors(bleft, tright, &line_bblobs, &vertical_x, &vertical_y,
                  vectors);
  if (!vectors->empty()) {
    RemoveUnusedLineSegments(true, &line_bblobs, *pix_hline);
    SubtractLinesAndResidue(*pix_hline, pix_non_hline, resolution, src_pix);
    ICOORD vertical;
    vertical.set_with_shrink(vertical_x, vertical_y);
    TabVector::MergeSimilarTabVectors(vertical, vectors, nullptr);
    // Iterate the vectors to flip them. x and y were flipped for horizontal
    // lines, so FindLineVectors can work just with the vertical case.
    // See GetLineBoxes for more on the flip.
    TabVector_IT h_it(vectors);
    for (h_it.mark_cycle_pt(); !h_it.cycled_list(); h_it.forward()) {
      h_it.data()->XYFlip();
    }
  } else {
    pixDestroy(pix_hline);
  }
}

// Finds vertical lines in the given list of BLOBNBOXes. bleft and tright
// are the bounds of the image on which the input line_bblobs were found.
// The input line_bblobs list is const really.
// The output vertical_x and vertical_y are the total of all the vectors.
// The output list of TabVector makes no reference to the input BLOBNBOXes.
void LineFinder::FindLineVectors(const ICOORD& bleft, const ICOORD& tright,
                                 BLOBNBOX_LIST* line_bblobs,
                                 int* vertical_x, int* vertical_y,
                                 TabVector_LIST* vectors) {
  BLOBNBOX_IT bbox_it(line_bblobs);
  int b_count = 0;
  // Put all the blobs into the grid to find the lines, and move the blobs
  // to the output lists.
  AlignedBlob blob_grid(kLineFindGridSize, bleft, tright);
  for (bbox_it.mark_cycle_pt(); !bbox_it.cycled_list(); bbox_it.forward()) {
    BLOBNBOX* bblob = bbox_it.data();
    bblob->set_left_tab_type(TT_MAYBE_ALIGNED);
    bblob->set_left_rule(bleft.x());
    bblob->set_right_rule(tright.x());
    bblob->set_left_crossing_rule(bleft.x());
    bblob->set_right_crossing_rule(tright.x());
    blob_grid.InsertBBox(false, true, bblob);
    ++b_count;
  }
  if (b_count == 0)
    return;

  // Search the entire grid, looking for vertical line vectors.
  BlobGridSearch lsearch(&blob_grid);
  BLOBNBOX* bbox;
  TabVector_IT vector_it(vectors);
  *vertical_x = 0;
  *vertical_y = 1;
  lsearch.StartFullSearch();
  while ((bbox = lsearch.NextFullSearch()) != nullptr) {
    if (bbox->left_tab_type() == TT_MAYBE_ALIGNED) {
      const TBOX& box = bbox->bounding_box();
      if (AlignedBlob::WithinTestRegion(2, box.left(), box.bottom()))
        tprintf("Finding line vector starting at bbox (%d,%d)\n",
                box.left(), box.bottom());
      AlignedBlobParams align_params(*vertical_x, *vertical_y, box.width());
      TabVector* vector = blob_grid.FindVerticalAlignment(align_params, bbox,
                                                          vertical_x,
                                                          vertical_y);
      if (vector != nullptr) {
        vector->Freeze();
        vector_it.add_to_end(vector);
      }
    }
  }
}

// Returns a Pix music mask if music is detected.
// Any vertical line that has at least 5 intersections in sufficient density
// is taken to be a bar. Bars are used as a seed and the entire touching
// component is added to the output music mask and subtracted from the lines.
// Returns nullptr and does minimal work if no music is found.
static Pix* FilterMusic(int resolution, Pix* pix_closed,
                        Pix* pix_vline, Pix* pix_hline,
                        l_int32* v_empty, l_int32* h_empty) {
  int max_stave_height = static_cast<int>(resolution * kMaxStaveHeight);
  Pix* intersection_pix = pixAnd(nullptr, pix_vline, pix_hline);
  Boxa* boxa = pixConnComp(pix_vline, nullptr, 8);
  // Iterate over the boxes to find music bars.
  int nboxes = boxaGetCount(boxa);
  Pix* music_mask = nullptr;
  for (int i = 0; i < nboxes; ++i) {
    Box* box = boxaGetBox(boxa, i, L_CLONE);
    l_int32 x, y, box_width, box_height;
    boxGetGeometry(box, &x, &y, &box_width, &box_height);
    int joins = NumTouchingIntersections(box, intersection_pix);
    // Test for the join density being at least 5 per max_stave_height,
    // ie (joins-1)/box_height >= (5-1)/max_stave_height.
    if (joins >= 5 && (joins - 1) * max_stave_height >= 4 * box_height) {
      // This is a music bar. Add to the mask.
      if (music_mask == nullptr)
        music_mask = pixCreate(pixGetWidth(pix_vline), pixGetHeight(pix_vline),
                               1);
      pixSetInRect(music_mask, box);
    }
    boxDestroy(&box);
  }
  boxaDestroy(&boxa);
  pixDestroy(&intersection_pix);
  if (music_mask != nullptr) {
    // The mask currently contains just the bars. Use the mask as a seed
    // and the pix_closed as the mask for a seedfill to get all the
    // intersecting staves.
    pixSeedfillBinary(music_mask, music_mask, pix_closed, 8);
    // Filter out false positives. CCs in the music_mask should be the vast
    // majority of the pixels in their bounding boxes, as we expect just a
    // tiny amount of text, a few phrase marks, and crescendo etc left.
    Boxa* boxa = pixConnComp(music_mask, nullptr, 8);
    // Iterate over the boxes to find music components.
    int nboxes = boxaGetCount(boxa);
    for (int i = 0; i < nboxes; ++i) {
      Box* box = boxaGetBox(boxa, i, L_CLONE);
      Pix* rect_pix = pixClipRectangle(music_mask, box, nullptr);
      l_int32 music_pixels;
      pixCountPixels(rect_pix, &music_pixels, nullptr);
      pixDestroy(&rect_pix);
      rect_pix = pixClipRectangle(pix_closed, box, nullptr);
      l_int32 all_pixels;
      pixCountPixels(rect_pix, &all_pixels, nullptr);
      pixDestroy(&rect_pix);
      if (music_pixels < kMinMusicPixelFraction * all_pixels) {
        // False positive. Delete from the music mask.
        pixClearInRect(music_mask, box);
      }
      boxDestroy(&box);
    }
    l_int32 no_remaining_music;
    boxaDestroy(&boxa);
    pixZero(music_mask, &no_remaining_music);
    if (no_remaining_music) {
      pixDestroy(&music_mask);
    } else {
      pixSubtract(pix_vline, pix_vline, music_mask);
      pixSubtract(pix_hline, pix_hline, music_mask);
      // We may have deleted all the lines
      pixZero(pix_vline, v_empty);
      pixZero(pix_hline, h_empty);
    }
  }
  return music_mask;
}

// Most of the heavy lifting of line finding. Given src_pix and its separate
// resolution, returns image masks:
// pix_vline           candidate vertical lines.
// pix_non_vline       pixels that didn't look like vertical lines.
// pix_hline           candidate horizontal lines.
// pix_non_hline       pixels that didn't look like horizontal lines.
// pix_intersections   pixels where vertical and horizontal lines meet.
// pix_music_mask      candidate music staves.
// This function promises to initialize all the output (2nd level) pointers,
// but any of the returns that are empty will be nullptr on output.
// None of the input (1st level) pointers may be nullptr except pix_music_mask,
// which will disable music detection, and pixa_display.
void LineFinder::GetLineMasks(int resolution, Pix* src_pix,
                              Pix** pix_vline, Pix** pix_non_vline,
                              Pix** pix_hline, Pix** pix_non_hline,
                              Pix** pix_intersections, Pix** pix_music_mask,
                              Pixa* pixa_display) {
  Pix* pix_closed = nullptr;
  Pix* pix_hollow = nullptr;

  int max_line_width = resolution / kThinLineFraction;
  int min_line_length = resolution / kMinLineLengthFraction;
  if (pixa_display != nullptr) {
    tprintf("Image resolution = %d, max line width = %d, min length=%d\n",
            resolution, max_line_width, min_line_length);
  }
  int closing_brick = max_line_width / 3;

// only use opencl if compiled w/ OpenCL and selected device is opencl
#ifdef USE_OPENCL
  if (OpenclDevice::selectedDeviceIsOpenCL()) {
    // OpenCL pixGetLines Operation
    int clStatus = OpenclDevice::initMorphCLAllocations(pixGetWpl(src_pix),
                                                        pixGetHeight(src_pix),
                                                        src_pix);
    bool getpixclosed = pix_music_mask != nullptr;
    OpenclDevice::pixGetLinesCL(nullptr, src_pix, pix_vline, pix_hline,
                                &pix_closed, getpixclosed, closing_brick,
                                closing_brick, max_line_width, max_line_width,
                                min_line_length, min_line_length);
  } else {
#endif
  // Close up small holes, making it less likely that false alarms are found
  // in thickened text (as it will become more solid) and also smoothing over
  // some line breaks and nicks in the edges of the lines.
  pix_closed = pixCloseBrick(nullptr, src_pix, closing_brick, closing_brick);
  if (pixa_display != nullptr)
    pixaAddPix(pixa_display, pix_closed, L_CLONE);
  // Open up with a big box to detect solid areas, which can then be subtracted.
  // This is very generous and will leave in even quite wide lines.
  Pix* pix_solid = pixOpenBrick(nullptr, pix_closed, max_line_width,
                                max_line_width);
  if (pixa_display != nullptr)
    pixaAddPix(pixa_display, pix_solid, L_CLONE);
  pix_hollow = pixSubtract(nullptr, pix_closed, pix_solid);

  pixDestroy(&pix_solid);

  // Now open up in both directions independently to find lines of at least
  // 1 inch/kMinLineLengthFraction in length.
  if (pixa_display != nullptr)
    pixaAddPix(pixa_display, pix_hollow, L_CLONE);
  *pix_vline = pixOpenBrick(nullptr, pix_hollow, 1, min_line_length);
  *pix_hline = pixOpenBrick(nullptr, pix_hollow, min_line_length, 1);

  pixDestroy(&pix_hollow);
#ifdef USE_OPENCL
  }
#endif

  // Lines are sufficiently rare, that it is worth checking for a zero image.
  l_int32 v_empty = 0;
  l_int32 h_empty = 0;
  pixZero(*pix_vline, &v_empty);
  pixZero(*pix_hline, &h_empty);
  if (pix_music_mask != nullptr) {
    if (!v_empty && !h_empty) {
      *pix_music_mask = FilterMusic(resolution, pix_closed,
                                    *pix_vline, *pix_hline,
                                    &v_empty, &h_empty);
    } else {
      *pix_music_mask = nullptr;
    }
  }
  pixDestroy(&pix_closed);
  Pix* pix_nonlines = nullptr;
  *pix_intersections = nullptr;
  Pix* extra_non_hlines = nullptr;
  if (!v_empty) {
    // Subtract both line candidates from the source to get definite non-lines.
    pix_nonlines = pixSubtract(nullptr, src_pix, *pix_vline);
    if (!h_empty) {
      pixSubtract(pix_nonlines, pix_nonlines, *pix_hline);
      // Intersections are a useful indicator for likelihood of being a line.
      *pix_intersections = pixAnd(nullptr, *pix_vline, *pix_hline);
      // Candidate vlines are not hlines (apart from the intersections)
      // and vice versa.
      extra_non_hlines = pixSubtract(nullptr, *pix_vline, *pix_intersections);
    }
    *pix_non_vline = pixErodeBrick(nullptr, pix_nonlines, kMaxLineResidue, 1);
    pixSeedfillBinary(*pix_non_vline, *pix_non_vline, pix_nonlines, 8);
    if (!h_empty) {
      // Candidate hlines are not vlines.
      pixOr(*pix_non_vline, *pix_non_vline, *pix_hline);
      pixSubtract(*pix_non_vline, *pix_non_vline, *pix_intersections);
    }
    if (!FilterFalsePositives(resolution, *pix_non_vline, *pix_intersections,
                              *pix_vline))
      pixDestroy(pix_vline);  // No candidates left.
  } else {
    // No vertical lines.
    pixDestroy(pix_vline);
    *pix_non_vline = nullptr;
    if (!h_empty) {
      pix_nonlines = pixSubtract(nullptr, src_pix, *pix_hline);
    }
  }
  if (h_empty) {
    pixDestroy(pix_hline);
    *pix_non_hline = nullptr;
    if (v_empty) {
      return;
    }
  } else {
    *pix_non_hline = pixErodeBrick(nullptr, pix_nonlines, 1, kMaxLineResidue);
    pixSeedfillBinary(*pix_non_hline, *pix_non_hline, pix_nonlines, 8);
    if (extra_non_hlines != nullptr) {
      pixOr(*pix_non_hline, *pix_non_hline, extra_non_hlines);
      pixDestroy(&extra_non_hlines);
    }
    if (!FilterFalsePositives(resolution, *pix_non_hline, *pix_intersections,
                              *pix_hline))
      pixDestroy(pix_hline);  // No candidates left.
  }
  if (pixa_display != nullptr) {
    if (*pix_vline != nullptr) pixaAddPix(pixa_display, *pix_vline, L_CLONE);
    if (*pix_hline != nullptr) pixaAddPix(pixa_display, *pix_hline, L_CLONE);
    if (pix_nonlines != nullptr) pixaAddPix(pixa_display, pix_nonlines, L_CLONE);
    if (*pix_non_vline != nullptr)
      pixaAddPix(pixa_display, *pix_non_vline, L_CLONE);
    if (*pix_non_hline != nullptr)
      pixaAddPix(pixa_display, *pix_non_hline, L_CLONE);
    if (*pix_intersections != nullptr)
      pixaAddPix(pixa_display, *pix_intersections, L_CLONE);
    if (pix_music_mask != nullptr && *pix_music_mask != nullptr)
      pixaAddPix(pixa_display, *pix_music_mask, L_CLONE);
  }
  pixDestroy(&pix_nonlines);
}

// Returns a list of boxes corresponding to the candidate line segments. Sets
// the line_crossings member of the boxes so we can later determine the number
// of intersections touched by a full line.
void LineFinder::GetLineBoxes(bool horizontal_lines,
                              Pix* pix_lines, Pix* pix_intersections,
                              C_BLOB_LIST* line_cblobs,
                              BLOBNBOX_LIST* line_bblobs) {
  // Put a single pixel crack in every line at an arbitrary spacing,
  // so they break up and the bounding boxes can be used to get the
  // direction accurately enough without needing outlines.
  int wpl = pixGetWpl(pix_lines);
  int width = pixGetWidth(pix_lines);
  int height = pixGetHeight(pix_lines);
  l_uint32* data = pixGetData(pix_lines);
  if (horizontal_lines) {
    for (int y = 0; y < height; ++y, data += wpl) {
      for (int x = kCrackSpacing; x < width; x += kCrackSpacing) {
        CLEAR_DATA_BIT(data, x);
      }
    }
  } else {
    for (int y = kCrackSpacing; y < height; y += kCrackSpacing) {
      memset(data + wpl * y, 0, wpl * sizeof(*data));
    }
  }
  // Get the individual connected components
  Boxa* boxa = pixConnComp(pix_lines, nullptr, 8);
  ConvertBoxaToBlobs(width, height, &boxa, line_cblobs);
  // Make the BLOBNBOXes from the C_BLOBs.
  C_BLOB_IT blob_it(line_cblobs);
  BLOBNBOX_IT bbox_it(line_bblobs);
  for (blob_it.mark_cycle_pt(); !blob_it.cycled_list(); blob_it.forward()) {
    C_BLOB* cblob = blob_it.data();
    auto* bblob = new BLOBNBOX(cblob);
    bbox_it.add_to_end(bblob);
    // Determine whether the line segment touches two intersections.
    const TBOX& bbox = bblob->bounding_box();
    Box* box = boxCreate(bbox.left(), bbox.bottom(),
                         bbox.width(), bbox.height());
    bblob->set_line_crossings(NumTouchingIntersections(box, pix_intersections));
    boxDestroy(&box);
    // Transform the bounding box prior to finding lines. To save writing
    // two line finders, flip x and y for horizontal lines and re-use the
    // tab-stop detection code. For vertical lines we still have to flip the
    // y-coordinates to switch from leptonica coords to tesseract coords.
    if (horizontal_lines) {
      // Note that we have Leptonica coords stored in a Tesseract box, so that
      // bbox.bottom(), being the MIN y coord, is actually the top, so to get
      // back to Leptonica coords in RemoveUnusedLineSegments, we have to
      // use height - box.right() as the top, which looks very odd.
      TBOX new_box(height - bbox.top(), bbox.left(),
                   height - bbox.bottom(), bbox.right());
      bblob->set_bounding_box(new_box);
    } else {
      TBOX new_box(bbox.left(), height - bbox.top(),
                   bbox.right(), height - bbox.bottom());
      bblob->set_bounding_box(new_box);
    }
  }
}

}  // namespace tesseract.
