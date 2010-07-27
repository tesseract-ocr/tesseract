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

#ifdef _MSC_VER
#pragma warning(disable:4244)  // Conversion warnings
#endif

#include "linefind.h"
#include "alignedblob.h"
#include "tabvector.h"
#include "blobbox.h"
#include "edgblob.h"
// This entire file is dependent upon leptonica. If you don't have it,
// then the code doesn't do anything useful.
#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif
#ifdef HAVE_LIBLEPT
#include "allheaders.h"
#endif

BOOL_VAR(textord_tabfind_show_vlines, false, "Show vertical rule lines");

namespace tesseract {

/// Denominator of resolution makes max pixel width to allow thin lines.
const int kThinLineFraction = 30;
/// Denominator of resolution makes min pixels to demand line lengths to be.
const int kMinLineLengthFraction = 8;
/// Spacing of cracks across the page to break up tall vertical lines.
const int kCrackSpacing = 100;
/// Grid size used by line finder. Not very critical.
const int kLineFindGridSize = 50;

// Finds vertical line objects in the given pix.
// Uses the given resolution to determine size thresholds instead of any
// that may be present in the pix.
// The output vertical_x and vertical_y contain a sum of the output vectors,
// thereby giving the mean vertical direction.
// The output vectors are owned by the list and Frozen (cannot refit) by
// having no boxes, as there is no need to refit or merge separator lines.
void LineFinder::FindVerticalLines(int resolution,  Pix* pix,
                                   int* vertical_x, int* vertical_y,
                                   TabVector_LIST* vectors) {
#ifdef HAVE_LIBLEPT
  Pix* line_pix;
  Boxa* boxes = GetVLineBoxes(resolution, pix, &line_pix);
  C_BLOB_LIST line_cblobs;
  int width = pixGetWidth(pix);
  int height = pixGetHeight(pix);
  ConvertBoxaToBlobs(width, height, &boxes, &line_cblobs);
  // Make the BLOBNBOXes from the C_BLOBs.
  BLOBNBOX_LIST line_bblobs;
  C_BLOB_IT blob_it(&line_cblobs);
  BLOBNBOX_IT bbox_it(&line_bblobs);
  for (blob_it.mark_cycle_pt(); !blob_it.cycled_list(); blob_it.forward()) {
    C_BLOB* cblob = blob_it.data();
    BLOBNBOX* bblob = new BLOBNBOX(cblob);
    bbox_it.add_to_end(bblob);
  }
  ICOORD bleft(0, 0);
  ICOORD tright(width, height);
  FindLineVectors(bleft, tright, &line_bblobs, vertical_x, vertical_y, vectors);
  if (!vectors->empty()) {
    // Some lines were found, so erase the unused blobs from the line image
    // and then subtract the line image from the source.
    bbox_it.move_to_first();
    for (bbox_it.mark_cycle_pt(); !bbox_it.cycled_list(); bbox_it.forward()) {
      BLOBNBOX* blob = bbox_it.data();
      if (blob->left_tab_type() == TT_UNCONFIRMED) {
        const TBOX& box = blob->bounding_box();
        Box* pixbox = boxCreate(box.left(), height - box.top(),
                                box.width(), box.height());
        pixClearInRect(line_pix, pixbox);
        boxDestroy(&pixbox);
      }
    }
    pixDilateBrick(line_pix, line_pix, 1, 3);
    pixSubtract(pix, pix, line_pix);
    if (textord_tabfind_show_vlines)
      pixWrite("vlinesclean.png", line_pix, IFF_PNG);
    ICOORD vertical;
    vertical.set_with_shrink(*vertical_x, *vertical_y);
    TabVector::MergeSimilarTabVectors(vertical, vectors, NULL);
  }
  pixDestroy(&line_pix);
#endif
}

// Finds horizontal line objects in the given pix.
// Uses the given resolution to determine size thresholds instead of any
// that may be present in the pix.
// The output vectors are owned by the list and Frozen (cannot refit) by
// having no boxes, as there is no need to refit or merge separator lines.
void LineFinder::FindHorizontalLines(int resolution,  Pix* pix,
                                     TabVector_LIST* vectors) {
#ifdef HAVE_LIBLEPT
  Pix* line_pix;
  Boxa* boxes = GetHLineBoxes(resolution, pix, &line_pix);
  C_BLOB_LIST line_cblobs;
  int width = pixGetWidth(pix);
  int height = pixGetHeight(pix);
  ConvertBoxaToBlobs(height, width, &boxes, &line_cblobs);
  // Make the BLOBNBOXes from the C_BLOBs.
  BLOBNBOX_LIST line_bblobs;
  C_BLOB_IT blob_it(&line_cblobs);
  BLOBNBOX_IT bbox_it(&line_bblobs);
  for (blob_it.mark_cycle_pt(); !blob_it.cycled_list(); blob_it.forward()) {
    C_BLOB* cblob = blob_it.data();
    BLOBNBOX* bblob = new BLOBNBOX(cblob);
    bbox_it.add_to_end(bblob);
  }
  ICOORD bleft(0, 0);
  ICOORD tright(height, width);
  int vertical_x, vertical_y;
  FindLineVectors(bleft, tright, &line_bblobs, &vertical_x, &vertical_y,
                  vectors);
  if (!vectors->empty()) {
    // Some lines were found, so erase the unused blobs from the line image
    // and then subtract the line image from the source.
    bbox_it.move_to_first();
    for (bbox_it.mark_cycle_pt(); !bbox_it.cycled_list(); bbox_it.forward()) {
      BLOBNBOX* blob = bbox_it.data();
      if (blob->left_tab_type() == TT_UNCONFIRMED) {
        const TBOX& box = blob->bounding_box();
        // Coords are in tess format so filp x and y and then covert
        // to leptonica by height -y.
        Box* pixbox = boxCreate(box.bottom(), height - box.right(),
                                box.height(), box.width());
        pixClearInRect(line_pix, pixbox);
        boxDestroy(&pixbox);
      }
    }
    pixDilateBrick(line_pix, line_pix, 3, 1);
    pixSubtract(pix, pix, line_pix);
    if (textord_tabfind_show_vlines)
      pixWrite("hlinesclean.png", line_pix, IFF_PNG);
    ICOORD vertical;
    vertical.set_with_shrink(vertical_x, vertical_y);
    TabVector::MergeSimilarTabVectors(vertical, vectors, NULL);
    // Iterate the vectors to flip them.
    TabVector_IT h_it(vectors);
    for (h_it.mark_cycle_pt(); !h_it.cycled_list(); h_it.forward()) {
      h_it.data()->XYFlip();
    }
  }
  pixDestroy(&line_pix);
#endif
}

// Converts the Boxa array to a list of C_BLOB, getting rid of severely
// overlapping outlines and those that are children of a bigger one.
// The output is a list of C_BLOBs that are owned by the list.
// The C_OUTLINEs in the C_BLOBs contain no outline data - just empty
// bounding boxes. The Boxa is consumed and destroyed.
void LineFinder::ConvertBoxaToBlobs(int image_width, int image_height,
                                    Boxa** boxes, C_BLOB_LIST* blobs) {
#ifdef HAVE_LIBLEPT
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
    ICOORD top_left(x, image_height - y);
    ICOORD bot_right(x + width, image_height - (y + height));
    CRACKEDGE startpt;
    startpt.pos = top_left;
    C_OUTLINE* outline = new C_OUTLINE(&startpt, top_left, bot_right, 0);
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
#endif
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
    bblob->set_left_tab_type(TT_UNCONFIRMED);
    bblob->set_left_rule(bleft.x());
    bblob->set_right_rule(tright.x());
    bblob->set_left_crossing_rule(bleft.x());
    bblob->set_right_crossing_rule(tright.x());
    blob_grid.InsertBBox(false, true, bblob);
    ++b_count;
  }
  if (textord_debug_tabfind)
    tprintf("Inserted %d line blobs into grid\n", b_count);
  if (b_count == 0)
    return;

  // Search the entire grid, looking for vertical line vectors.
  GridSearch<BLOBNBOX, BLOBNBOX_CLIST, BLOBNBOX_C_IT> lsearch(&blob_grid);
  BLOBNBOX* bbox;
  TabVector_IT vector_it(vectors);
  *vertical_x = 0;
  *vertical_y = 1;
  lsearch.StartFullSearch();
  while ((bbox = lsearch.NextFullSearch()) != NULL) {
    if (bbox->left_tab_type() == TT_UNCONFIRMED) {
      const TBOX& box = bbox->bounding_box();
      if (AlignedBlob::WithinTestRegion(2, box.left(), box.bottom()))
        tprintf("Finding line vector starting at bbox (%d,%d)\n",
                box.left(), box.bottom());
      AlignedBlobParams align_params(*vertical_x, *vertical_y, box.width());
      TabVector* vector = blob_grid.FindVerticalAlignment(align_params, bbox,
                                                          vertical_x,
                                                          vertical_y);
      if (vector != NULL) {
        vector->Freeze();
        vector_it.add_to_end(vector);
      }
    }
  }
  ScrollView* line_win = NULL;
  if (textord_tabfind_show_vlines) {
    line_win = blob_grid.MakeWindow(0, 50, "Vlines");
    blob_grid.DisplayBoxes(line_win);
    line_win = blob_grid.DisplayTabs("Vlines", line_win);
  }
}

// Get a set of bounding boxes of possible vertical lines in the image.
// The input resolution overrides any resolution set in src_pix.
// The output line_pix contains just all the detected lines.
Boxa* LineFinder::GetVLineBoxes(int resolution, Pix* src_pix, Pix** line_pix) {
#ifdef HAVE_LIBLEPT
  // Remove any parts of 1 inch/kThinLineFraction wide or more, by opening
  // away the thin lines and subtracting what's left.
  // This is very generous and will leave in even quite wide lines.
  Pix* pixt1 = pixOpenBrick(NULL, src_pix, resolution / kThinLineFraction, 1);
  pixSubtract(pixt1, src_pix, pixt1);
  // Spread sideways to allow for some skew.
  Pix* pixt2 = pixDilateBrick(NULL, pixt1, 3, 1);
  // Now keep only tall stuff of height at least 1 inch/kMinLineLengthFraction.
  pixOpenBrick(pixt1, pixt2, 1, resolution / kMinLineLengthFraction);
  pixDestroy(&pixt2);
  // Put a single pixel crack in every line at an arbitrary spacing,
  // so they break up and the bounding boxes can be used to get the
  // direction accurately enough without needing outlines.
  int wpl = pixGetWpl(pixt1);
  int height = pixGetHeight(pixt1);
  l_uint32* data = pixGetData(pixt1);
  for (int y = kCrackSpacing; y < height; y += kCrackSpacing) {
    memset(data + wpl * y, 0, wpl * sizeof(*data));
  }
  if (textord_tabfind_show_vlines)
    pixWrite("vlines.png", pixt1, IFF_PNG);
  Boxa* boxa = pixConnComp(pixt1, NULL, 8);
  *line_pix = pixt1;
  return boxa;
#else
  return NULL;
#endif
}

// Get a set of bounding boxes of possible horizontal lines in the image.
// The input resolution overrides any resolution set in src_pix.
// The output line_pix contains just all the detected lines.
// The output boxes undergo the transformation (x,y)->(height-y,x) so the
// lines can be found with a vertical line finder afterwards.
// This transformation allows a simple x/y flip to reverse it in tesseract
// coordinates and it is faster to flip the lines than rotate the image.
Boxa* LineFinder::GetHLineBoxes(int resolution, Pix* src_pix, Pix** line_pix) {
#ifdef HAVE_LIBLEPT
  // Remove any parts of 1 inch/kThinLineFraction high or more, by opening
  // away the thin lines and subtracting what's left.
  // This is very generous and will leave in even quite wide lines.
  Pix* pixt1 = pixOpenBrick(NULL, src_pix, 1, resolution / kThinLineFraction);
  pixSubtract(pixt1, src_pix, pixt1);
  // Spread vertically to allow for some skew.
  Pix* pixt2 = pixDilateBrick(NULL, pixt1, 1, 3);
  // Now keep only wide stuff of width at least 1 inch/kMinLineLengthFraction.
  pixOpenBrick(pixt1, pixt2, resolution / kMinLineLengthFraction, 1);
  pixDestroy(&pixt2);
  // Put a single pixel crack in every line at an arbitrary spacing,
  // so they break up and the bounding boxes can be used to get the
  // direction accurately enough without needing outlines.
  int wpl = pixGetWpl(pixt1);
  int width = pixGetWidth(pixt1);
  int height = pixGetHeight(pixt1);
  l_uint32* data = pixGetData(pixt1);
  for (int y = 0; y < height; ++y, data += wpl) {
    for (int x = kCrackSpacing; x < width; x += kCrackSpacing) {
      CLEAR_DATA_BIT(data, x);
    }
  }
  if (textord_tabfind_show_vlines)
    pixWrite("hlines.png", pixt1, IFF_PNG);
  Boxa* boxa = pixConnComp(pixt1, NULL, 8);
  *line_pix = pixt1;

  // Iterate the boxes to flip x and y.
  int nboxes = boxaGetCount(boxa);
  for (int i = 0; i < nboxes; ++i) {
    l_int32 x, y, box_width, box_height;
    boxaGetBoxGeometry(boxa, i, &x, &y, &box_width, &box_height);
    Box* box = boxCreate(height - (y + box_height),
                         width - (x + box_width), box_height, box_width);
    boxaReplaceBox(boxa, i, box);
  }
  return boxa;
#else
  return NULL;
#endif
}

}  // namespace tesseract.


